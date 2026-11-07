// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:math' as math;
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/id_card_layout.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';

/// The three placeable zones of a card/label layout.
enum _Zone { barcode, text, logo }

/// Alignment of a zone within its free space.
enum _Align { left, hCenter, right, top, vCenter, bottom }

/// The data a [LayoutEditorScreen] loads: the stored layout plus the logo image
/// bytes to preview (if any).
class LayoutEditorData {
  const LayoutEditorData({required this.layout, this.logoBytes});
  final IdCardLayout layout;
  final Uint8List? logoBytes;
}

/// Loads the layout and logo for the editor.
typedef LayoutLoad = Future<LayoutEditorData> Function(WidgetRef ref);

/// Persists the edited [IdCardLayout].
typedef LayoutSave = Future<void> Function(WidgetRef ref, IdCardLayout layout);

/// Reusable visual editor for an [IdCardLayout]-shaped layout (ID card or book
/// label). Zones (barcode, text, optional logo) can be dragged, resized from
/// any handle, rotated in 90° steps, maximised and aligned. Loading and saving
/// are injected so the same editor serves the server-stored ID-card layout and
/// device-local label layouts.
class LayoutEditorScreen extends ConsumerStatefulWidget {
  const LayoutEditorScreen({
    super.key,
    required this.title,
    required this.iconAsset,
    required this.onLoad,
    required this.onSave,
    required this.savedMessage,
    required this.sampleTextLines,
    this.marginStore = const IdCardMarginStore(),
    this.showBarcodeZone = true,
    this.showLogoToggle = true,
  });

  final String title;
  final String iconAsset;
  final LayoutLoad onLoad;
  final LayoutSave onSave;
  final String savedMessage;

  /// Representative lines shown in the text zone's preview (e.g. issue date +
  /// name for ID cards, or title + author for book labels).
  final List<String> sampleTextLines;
  final IdCardMarginStore marginStore;

  /// Whether the barcode zone is shown (false for signature labels).
  final bool showBarcodeZone;

  /// Whether the logo toggle/zone is shown (false for signature labels).
  final bool showLogoToggle;

  @override
  ConsumerState<LayoutEditorScreen> createState() =>
      _LayoutEditorScreenState();
}

class _LayoutEditorScreenState extends ConsumerState<LayoutEditorScreen> {
  Uint8List? _logoBytes;
  IdCardLayout? _draft;
  IdCardLayout? _loaded;
  _Zone? _selected;
  bool _loading = false;
  bool _saving = false;
  Object? _error;

  final _widthCtrl = TextEditingController();
  final _heightCtrl = TextEditingController();
  final _marginCtrl = TextEditingController();

  // Free margin around the card edge, in millimetres. A per-device placement
  // preference (mirroring the Qt client); zones are kept within it.
  double _marginMm = 0.0;

  // Natural width/height ratio of the collection logo image, or null when no
  // logo is stored or it could not be decoded. The logo zone is constrained to
  // this ratio (adjusted for its orientation) so the logo is never distorted.
  double? _logoRatio;

  @override
  void initState() {
    super.initState();
    _marginCtrl.text = _fmtMm(_marginMm);
    _load();
  }

  @override
  void dispose() {
    _widthCtrl.dispose();
    _heightCtrl.dispose();
    _marginCtrl.dispose();
    super.dispose();
  }

  /// The configured margin clamped so that, on both axes, a minimum-size zone
  /// still fits between the opposing margins.
  double _effectiveMargin(IdCardLayout d) {
    final limit =
        (math.min(d.cardWidthMm, d.cardHeightMm) - _minZoneMm) / 2.0;
    return _marginMm.clamp(0.0, math.max(0.0, limit));
  }

  /// Decodes the natural width/height ratio of the image in [bytes], or null on
  /// failure or a degenerate size.
  Future<double?> _decodeImageRatio(Uint8List bytes) async {
    try {
      final codec = await ui.instantiateImageCodec(bytes);
      final frame = await codec.getNextFrame();
      final w = frame.image.width;
      final h = frame.image.height;
      frame.image.dispose();
      if(w <= 0 || h <= 0) return null;
      return w / h;
    } catch(_) {
      return null;
    }
  }

  /// The width/height ratio the logo zone [e]'s footprint must keep, accounting
  /// for its orientation (a quarter turn swaps the footprint's axes). Null when
  /// no logo ratio is known.
  double? _logoFootprintRatio(IdCardElement e) {
    final r = _logoRatio;
    if(r == null || r <= 0) return null;
    final odd = (e.orientation ~/ 90) % 2 != 0;
    return odd ? 1.0 / r : r;
  }

  /// Whether zone [z] (with element [e]) keeps a fixed aspect ratio — true only
  /// for the logo when its natural ratio is known.
  bool _aspectLocked(_Zone z, IdCardElement e) =>
      z == _Zone.logo && _logoFootprintRatio(e) != null;

  Future<void> _load() async {
    if(!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final dataFuture = widget.onLoad(ref);
      final margin = await widget.marginStore.load();
      final data = await dataFuture;
      final ratio = data.logoBytes != null
          ? await _decodeImageRatio(data.logoBytes!)
          : null;
      if(!mounted) return;
      setState(() {
        _logoBytes = data.logoBytes;
        _marginMm = margin;
        _logoRatio = ratio;
        _loaded = data.layout;
        // Bring any zones placed outside the local margin back inside it.
        _draft = _clampLayout(data.layout);
        _loading = false;
        _syncSizeControllers();
      });
    } catch(e) {
      if(!mounted) return;
      setState(() {
        _error = e;
        _loading = false;
      });
    }
  }

  void _syncSizeControllers() {
    final d = _draft;
    if(d == null) return;
    _widthCtrl.text = _fmtMm(d.cardWidthMm);
    _heightCtrl.text = _fmtMm(d.cardHeightMm);
    _marginCtrl.text = _fmtMm(_marginMm);
  }

  /// Applies the margin entered in [_marginCtrl] (clamped to 0..max), persists
  /// it on the device, and re-clamps all zones into the new safe area.
  void _applyMargin() {
    final v = double.tryParse(_marginCtrl.text.replaceAll(',', '.'));
    if(v == null) {
      _marginCtrl.text = _fmtMm(_marginMm);
      return;
    }
    final m = v.clamp(0.0, IdCardMarginStore.maxMm);
    setState(() {
      _marginMm = m;
      _marginCtrl.text = _fmtMm(m);
      if(_draft != null) _draft = _clampLayout(_draft!);
    });
    widget.marginStore.save(m);
  }

  IdCardElement _element(_Zone z) {
    final d = _draft!;
    switch(z) {
      case _Zone.barcode:
        return d.barcode;
      case _Zone.text:
        return d.text;
      case _Zone.logo:
        return d.logo;
    }
  }

  void _setElement(_Zone z, IdCardElement e) {
    final d = _draft!;
    switch(z) {
      case _Zone.barcode:
        _draft = d.copyWith(barcode: e);
        break;
      case _Zone.text:
        _draft = d.copyWith(text: e);
        break;
      case _Zone.logo:
        _draft = d.copyWith(logo: e);
        break;
    }
  }

  /// Clamps [e] so it stays within the card bounds of the current draft,
  /// inset by the configured free margin.
  IdCardElement _clamp(IdCardElement e) => _clampIn(e, _draft!);

  /// Clamps [e] against the bounds of [d], inset by the free margin.
  IdCardElement _clampIn(IdCardElement e, IdCardLayout d) {
    final m = _effectiveMargin(d);
    final maxW = math.max(_minZoneMm, d.cardWidthMm - 2 * m);
    final maxH = math.max(_minZoneMm, d.cardHeightMm - 2 * m);
    final w = e.widthMm.clamp(_minZoneMm, maxW);
    final h = e.heightMm.clamp(_minZoneMm, maxH);
    final x = e.xMm.clamp(m, d.cardWidthMm - m - w);
    final y = e.yMm.clamp(m, d.cardHeightMm - m - h);
    return e.copyWith(xMm: x, yMm: y, widthMm: w, heightMm: h);
  }

  /// Returns a copy of [d] with every zone re-clamped to the current bounds.
  IdCardLayout _clampLayout(IdCardLayout d) {
    return d.copyWith(
      barcode: _clampIn(d.barcode, d),
      text: _clampIn(d.text, d),
      logo: _clampIn(d.logo, d),
    );
  }

  void _applyCardSize() {
    final w = double.tryParse(_widthCtrl.text.replaceAll(',', '.'));
    final h = double.tryParse(_heightCtrl.text.replaceAll(',', '.'));
    if(w == null || h == null || w < _minCardMm || h < _minCardMm) {
      return;
    }
    setState(() {
      _draft = _draft!.copyWith(cardWidthMm: w, cardHeightMm: h);
      // Re-clamp all zones to the new card size.
      for(final z in _Zone.values) {
        _setElement(z, _clamp(_element(z)));
      }
    });
  }

  void _toggleLogo(bool on) {
    setState(() {
      var d = _draft!;
      if(on && d.logo.widthMm < _minZoneMm) {
        // Give the logo a sensible default box in the top-left corner, sized to
        // the logo's natural aspect ratio when known.
        final side = math.min(d.cardWidthMm, d.cardHeightMm) / 3.0;
        final ratio = _logoRatio;
        final w = ratio != null && ratio >= 1.0 ? side : side * (ratio ?? 1.0);
        final h = ratio != null && ratio >= 1.0 ? side / ratio : side;
        d = d.copyWith(
          logo: _clampIn(
            IdCardElement(xMm: 0.0, yMm: 0.0, widthMm: w, heightMm: h),
            d,
          ),
        );
      }
      _draft = d.copyWith(hasLogo: on);
      if(!on && _selected == _Zone.logo) _selected = null;
    });
  }

  void _rotateSelected() {
    final z = _selected;
    if(z == null) return;
    final e = _element(z);
    final d = _draft!;
    // Mirror the Qt rotate_90: swap dimensions, advance orientation.
    final newW = e.heightMm;
    final newH = e.widthMm;
    if(newW > d.cardWidthMm || newH > d.cardHeightMm) {
      final l10n = AppLocalizations.of(context)!;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(l10n.idCardLayoutRotateBlocked)),
      );
      return;
    }
    setState(() {
      _setElement(
        z,
        _clamp(e.copyWith(
          widthMm: newW,
          heightMm: newH,
          orientation: (e.orientation + 90) % 360,
        )),
      );
    });
  }

  void _dragZone(_Zone z, Offset deltaMm) {
    setState(() {
      final e = _element(z);
      _setElement(
        z,
        _clamp(e.copyWith(xMm: e.xMm + deltaMm.dx, yMm: e.yMm + deltaMm.dy)),
      );
    });
  }

  /// Resizes [z] by dragging one or more of its edges by [deltaMm], keeping the
  /// opposite (un-dragged) edges anchored. Each moved edge is clamped to the
  /// free margin and to a minimum zone size.
  void _resizeZoneEdges(
    _Zone z, {
    bool left = false,
    bool top = false,
    bool right = false,
    bool bottom = false,
    required Offset deltaMm,
  }) {
    setState(() {
      final d = _draft!;
      final m = _effectiveMargin(d);
      final e = _element(z);
      var l = e.xMm;
      var t = e.yMm;
      var r = e.xMm + e.widthMm;
      var b = e.yMm + e.heightMm;
      if(left) {
        l = (l + deltaMm.dx).clamp(m, r - _minZoneMm);
      }
      if(right) {
        r = (r + deltaMm.dx).clamp(l + _minZoneMm, d.cardWidthMm - m);
      }
      if(top) {
        t = (t + deltaMm.dy).clamp(m, b - _minZoneMm);
      }
      if(bottom) {
        b = (b + deltaMm.dy).clamp(t + _minZoneMm, d.cardHeightMm - m);
      }
      final ratio = z == _Zone.logo ? _logoFootprintRatio(e) : null;
      if(ratio != null) {
        // The logo keeps its aspect ratio: width drives height, but cap the
        // height to the room left by the anchored (un-dragged) vertical edge,
        // shrinking width to match. Only corner handles are active for the
        // logo, so exactly one horizontal and one vertical edge moved.
        var width = r - l;
        final vRoom = top ? (b - m) : (d.cardHeightMm - m - t);
        var height = width / ratio;
        if(height > vRoom) {
          height = vRoom;
          width = height * ratio;
          if(left) {
            l = r - width;
          } else {
            r = l + width;
          }
        }
        if(width < _minZoneMm || height < _minZoneMm) return;
        if(top) {
          t = b - height;
        } else {
          b = t + height;
        }
      }
      _setElement(
        z,
        e.copyWith(xMm: l, yMm: t, widthMm: r - l, heightMm: b - t),
      );
    });
  }

  /// Grows [z] to fill the free space around its centre — the largest rectangle
  /// bounded by the margin-inset card edges and the nearest edges of the other
  /// zones. Mirrors the Qt client's double-click "maximize".
  void _maximizeZone(_Zone z) {
    final fs = _freeSpaceRect(z);
    if(fs.width < _minZoneMm || fs.height < _minZoneMm) return;
    setState(() {
      final e = _element(z);
      var w = fs.width;
      var h = fs.height;
      // Fit the largest ratio-preserving box inside the free space for the logo.
      final ratio = z == _Zone.logo ? _logoFootprintRatio(e) : null;
      if(ratio != null) {
        if(w / h > ratio) {
          w = h * ratio;
        } else {
          h = w / ratio;
        }
      }
      _setElement(
        z,
        e.copyWith(xMm: fs.left, yMm: fs.top, widthMm: w, heightMm: h),
      );
    });
  }

  /// Aligns [z] within its free space to [a], keeping its current size.
  /// Mirrors the Qt client's align_left/right/center actions.
  void _alignZone(_Zone z, _Align a) {
    final fs = _freeSpaceRect(z);
    setState(() {
      final e = _element(z);
      var x = e.xMm;
      var y = e.yMm;
      switch(a) {
        case _Align.left:
          x = fs.left;
          break;
        case _Align.hCenter:
          x = fs.left + (fs.width - e.widthMm) / 2.0;
          break;
        case _Align.right:
          x = fs.right - e.widthMm;
          break;
        case _Align.top:
          y = fs.top;
          break;
        case _Align.vCenter:
          y = fs.top + (fs.height - e.heightMm) / 2.0;
          break;
        case _Align.bottom:
          y = fs.bottom - e.heightMm;
          break;
      }
      _setElement(z, _clamp(e.copyWith(xMm: x, yMm: y)));
    });
  }

  /// The largest free rectangle around [z]'s centre, bounded by the
  /// margin-inset card edges and the nearest edges of the other zones, in
  /// millimetres. Mirrors the Qt client's `free_space_rect`.
  Rect _freeSpaceRect(_Zone z) {
    final d = _draft!;
    final m = _effectiveMargin(d);
    final e = _element(z);
    var left = m;
    var top = m;
    var right = d.cardWidthMm - m;
    var bottom = d.cardHeightMm - m;
    final cx = e.xMm + e.widthMm / 2.0;
    final cy = e.yMm + e.heightMm / 2.0;
    for(final o in _Zone.values) {
      if(o == z) continue;
      if(o == _Zone.logo && !d.hasLogo) continue;
      final oe = _element(o);
      final ol = oe.xMm;
      final ot = oe.yMm;
      final or = oe.xMm + oe.widthMm;
      final ob = oe.yMm + oe.heightMm;
      // Only zones overlapping this zone's row/column bound its free space.
      if(ob > e.yMm && ot < e.yMm + e.heightMm) {
        if(or <= cx) left = math.max(left, or);
        if(ol >= cx) right = math.min(right, ol);
      }
      if(or > e.xMm && ol < e.xMm + e.widthMm) {
        if(ob <= cy) top = math.max(top, ob);
        if(ot >= cy) bottom = math.min(bottom, ot);
      }
    }
    return Rect.fromLTRB(left, top, right, bottom);
  }

  Future<void> _save() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() { _saving = true; });
    try {
      await widget.onSave(ref, _draft!);
      if(!mounted) return;
      setState(() {
        _saving = false;
        _loaded = _draft;
      });
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(widget.savedMessage)),
      );
    } catch(e) {
      if(!mounted) return;
      setState(() { _saving = false; });
      ServerErrorDialog.show(context, e, l10n);
    }
  }

  void _reset() {
    setState(() {
      _draft = _loaded;
      _selected = null;
      _syncSizeControllers();
    });
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: widget.iconAsset,
          title: widget.title,
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            tooltip: l10n.idCardLayoutReset,
            onPressed: (_draft == null || _saving) ? null : _reset,
          ),
          IconButton(
            icon: const Icon(Icons.save),
            tooltip: l10n.idCardLayoutSave,
            onPressed: (_draft == null || _saving) ? null : _save,
          ),
        ],
      ),
      body: _buildBody(l10n),
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if(_loading) {
      return const Center(child: CircularProgressIndicator());
    }
    if(_error != null) {
      return Center(
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: ErrorDisplay(error: _error!, l10n: l10n),
        ),
      );
    }
    final d = _draft;
    if(d == null) return const SizedBox.shrink();
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        if(_saving) const LinearProgressIndicator(),
        Padding(
          padding: const EdgeInsets.fromLTRB(12, 12, 12, 0),
          child: _buildControls(l10n, d),
        ),
        Expanded(
          child: Padding(
            padding: const EdgeInsets.all(16),
            child: Center(child: _buildCanvas(d)),
          ),
        ),
      ],
    );
  }

  Widget _buildControls(AppLocalizations l10n, IdCardLayout d) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Row(
          children: [
            Expanded(
              child: TextField(
                controller: _widthCtrl,
                keyboardType:
                    const TextInputType.numberWithOptions(decimal: true),
                decoration: InputDecoration(
                  labelText: l10n.idCardLayoutWidthMm,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
                onEditingComplete: _applyCardSize,
                onTapOutside: (_) => _applyCardSize(),
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: TextField(
                controller: _heightCtrl,
                keyboardType:
                    const TextInputType.numberWithOptions(decimal: true),
                decoration: InputDecoration(
                  labelText: l10n.idCardLayoutHeightMm,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
                onEditingComplete: _applyCardSize,
                onTapOutside: (_) => _applyCardSize(),
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        TextField(
          controller: _marginCtrl,
          keyboardType: const TextInputType.numberWithOptions(decimal: true),
          decoration: InputDecoration(
            labelText: l10n.idCardLayoutMarginMm,
            border: const OutlineInputBorder(),
            isDense: true,
          ),
          onEditingComplete: _applyMargin,
          onTapOutside: (_) => _applyMargin(),
        ),
        if(widget.showLogoToggle)
          Row(
            children: [
              Switch(value: d.hasLogo, onChanged: _toggleLogo),
              Text(l10n.idCardLayoutUseLogo),
            ],
          ),
        // Zone actions as compact icon buttons; a Wrap keeps them from being
        // clipped on narrow (phone) screens. All act on the selected zone.
        Wrap(
          spacing: 4,
          runSpacing: 4,
          children: [
            _zoneActionButton(
              icon: Icons.align_horizontal_left,
              tooltip: l10n.idCardLayoutAlignLeft,
              onPressed: () => _alignZone(_selected!, _Align.left),
            ),
            _zoneActionButton(
              icon: Icons.align_horizontal_center,
              tooltip: l10n.idCardLayoutAlignHCenter,
              onPressed: () => _alignZone(_selected!, _Align.hCenter),
            ),
            _zoneActionButton(
              icon: Icons.align_horizontal_right,
              tooltip: l10n.idCardLayoutAlignRight,
              onPressed: () => _alignZone(_selected!, _Align.right),
            ),
            _zoneActionButton(
              icon: Icons.align_vertical_top,
              tooltip: l10n.idCardLayoutAlignTop,
              onPressed: () => _alignZone(_selected!, _Align.top),
            ),
            _zoneActionButton(
              icon: Icons.align_vertical_center,
              tooltip: l10n.idCardLayoutAlignVCenter,
              onPressed: () => _alignZone(_selected!, _Align.vCenter),
            ),
            _zoneActionButton(
              icon: Icons.align_vertical_bottom,
              tooltip: l10n.idCardLayoutAlignBottom,
              onPressed: () => _alignZone(_selected!, _Align.bottom),
            ),
            _zoneActionButton(
              icon: Icons.fit_screen,
              tooltip: l10n.idCardLayoutMaximize,
              onPressed: () => _maximizeZone(_selected!),
            ),
            _zoneActionButton(
              icon: Icons.rotate_90_degrees_cw,
              tooltip: l10n.idCardLayoutRotate,
              onPressed: _rotateSelected,
            ),
          ],
        ),
      ],
    );
  }

  /// A compact icon button for a zone action; disabled when no zone is
  /// selected. [onPressed] may assume `_selected` is non-null.
  Widget _zoneActionButton({
    required IconData icon,
    required String tooltip,
    required VoidCallback onPressed,
  }) {
    return IconButton.filledTonal(
      icon: Icon(icon),
      tooltip: tooltip,
      visualDensity: VisualDensity.compact,
      onPressed: _selected == null ? null : onPressed,
    );
  }

  Widget _buildCanvas(IdCardLayout d) {
    if(d.cardWidthMm <= 0 || d.cardHeightMm <= 0) {
      return const SizedBox.shrink();
    }
    return AspectRatio(
      aspectRatio: d.cardWidthMm / d.cardHeightMm,
      child: LayoutBuilder(
        builder: (ctx, constraints) {
          final scale = constraints.maxWidth / d.cardWidthMm;
          final zones = <_Zone>[
            if(widget.showBarcodeZone) _Zone.barcode,
            _Zone.text,
            if(d.hasLogo && widget.showLogoToggle) _Zone.logo,
          ];
          return GestureDetector(
            onTap: () => setState(() => _selected = null),
            child: DecoratedBox(
              decoration: BoxDecoration(
                color: Colors.white,
                border: Border.all(color: Colors.black54),
              ),
              child: Stack(
                children: [
                  if(_effectiveMargin(d) > 0)
                    Positioned.fill(
                      child: IgnorePointer(
                        child: CustomPaint(
                          painter:
                              _MarginGuidePainter(_effectiveMargin(d) * scale),
                        ),
                      ),
                    ),
                  for(final z in zones) _buildZone(z, scale),
                ],
              ),
            ),
          );
        },
      ),
    );
  }

  Widget _buildZone(_Zone z, double scale) {
    final e = _element(z);
    final selected = _selected == z;
    final wPx = e.widthMm * scale;
    final hPx = e.heightMm * scale;
    // Centre offsets for the edge-midpoint handles, never negative.
    final cx = math.max(0.0, (wPx - _handleSize) / 2.0);
    final cy = math.max(0.0, (hPx - _handleSize) / 2.0);
    return Positioned(
      left: e.xMm * scale,
      top: e.yMm * scale,
      width: wPx,
      height: hPx,
      child: GestureDetector(
        onTap: () => setState(() => _selected = z),
        // Double-tap maximizes the zone into the free space, like the Qt client.
        onDoubleTap: () {
          setState(() => _selected = z);
          _maximizeZone(z);
        },
        onPanUpdate: (det) =>
            _dragZone(z, Offset(det.delta.dx / scale, det.delta.dy / scale)),
        child: Stack(
          clipBehavior: Clip.none,
          children: [
            Positioned.fill(
              child: DecoratedBox(
                decoration: BoxDecoration(
                  border: Border.all(
                    color: selected ? Colors.blue : Colors.grey,
                    width: selected ? 2 : 1,
                  ),
                  color: Colors.black.withValues(alpha: 0.03),
                ),
                child: _zoneContent(z, e),
              ),
            ),
            // Resize handles, shown only while the zone is selected. Each drags
            // its own edge(s), anchoring the opposite side so any corner — not
            // just bottom-right — can be moved. The aspect-locked logo offers
            // only corner handles (edge handles would change one side alone and
            // break the ratio), mirroring the Qt client.
            if(selected) ...[
              _resizeHandle(z, scale, left: 0, top: 0,
                  dragLeft: true, dragTop: true),
              _resizeHandle(z, scale, right: 0, top: 0,
                  dragRight: true, dragTop: true),
              _resizeHandle(z, scale, right: 0, bottom: 0,
                  dragRight: true, dragBottom: true),
              _resizeHandle(z, scale, left: 0, bottom: 0,
                  dragLeft: true, dragBottom: true),
              if(!_aspectLocked(z, e)) ...[
                _resizeHandle(z, scale, left: cx, top: 0, dragTop: true),
                _resizeHandle(z, scale, right: 0, top: cy, dragRight: true),
                _resizeHandle(z, scale, left: cx, bottom: 0, dragBottom: true),
                _resizeHandle(z, scale, left: 0, top: cy, dragLeft: true),
              ],
            ],
          ],
        ),
      ),
    );
  }

  /// A small square resize handle positioned within the selected zone. The
  /// `drag*` flags select which edge(s) the handle moves.
  Widget _resizeHandle(
    _Zone z,
    double scale, {
    double? left,
    double? top,
    double? right,
    double? bottom,
    bool dragLeft = false,
    bool dragTop = false,
    bool dragRight = false,
    bool dragBottom = false,
  }) {
    return Positioned(
      left: left,
      top: top,
      right: right,
      bottom: bottom,
      child: GestureDetector(
        onPanUpdate: (det) => _resizeZoneEdges(
          z,
          left: dragLeft,
          top: dragTop,
          right: dragRight,
          bottom: dragBottom,
          deltaMm: Offset(det.delta.dx / scale, det.delta.dy / scale),
        ),
        child: Container(
          width: _handleSize,
          height: _handleSize,
          decoration: BoxDecoration(
            color: Colors.blue,
            border: Border.all(color: Colors.white, width: 1),
          ),
        ),
      ),
    );
  }

  /// Builds the rotated preview content of a zone within its footprint.
  ///
  /// [RotatedBox] is layout-aware: for odd quarter turns it lays the child out
  /// with swapped width/height constraints, so the content fills the footprint
  /// instead of being clamped to the unrotated bounds (which left wide margins).
  Widget _zoneContent(_Zone z, IdCardElement e) {
    return RotatedBox(
      quarterTurns: (e.orientation ~/ 90) % 4,
      child: _zoneNaturalContent(z),
    );
  }

  Widget _zoneNaturalContent(_Zone z) {
    switch(z) {
      case _Zone.barcode:
        return const Padding(
          padding: EdgeInsets.all(2),
          child: CustomPaint(painter: _BarcodePreviewPainter()),
        );
      case _Zone.text:
        final lines = widget.sampleTextLines;
        return Padding(
          padding: const EdgeInsets.all(2),
          child: FittedBox(
            fit: BoxFit.contain,
            alignment: Alignment.centerLeft,
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                for(var i = 0; i < lines.length; ++i)
                  Text(
                    lines[i],
                    style: TextStyle(
                      fontWeight: i == lines.length - 1
                          ? FontWeight.bold
                          : FontWeight.normal,
                    ),
                  ),
              ],
            ),
          ),
        );
      case _Zone.logo:
        if(_logoBytes != null) {
          return Padding(
            padding: const EdgeInsets.all(2),
            child: Image.memory(_logoBytes!, fit: BoxFit.contain),
          );
        }
        return const Center(child: Icon(Icons.image_outlined));
    }
  }
}

/// Minimum card / zone dimensions in millimetres, to avoid degenerate boxes.
const double _minCardMm = 10.0;
const double _minZoneMm = 3.0;

/// Side length, in logical pixels, of the zone resize handles.
const double _handleSize = 14.0;

String _fmtMm(double v) {
  if(v == v.roundToDouble()) return v.toStringAsFixed(0);
  return v.toStringAsFixed(1);
}


/// Paints a simple representative barcode (alternating bars) for the preview.
class _BarcodePreviewPainter extends CustomPainter {
  const _BarcodePreviewPainter();

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()..color = Colors.black;
    // A fixed pseudo-pattern of bar widths so the preview looks like a barcode.
    const pattern = [1, 1, 2, 1, 3, 1, 1, 2, 2, 1, 1, 3, 1, 1, 2, 1];
    final unit = size.width / pattern.fold<int>(0, (a, b) => a + b);
    var x = 0.0;
    var draw = true;
    for(final w in pattern) {
      final bw = w * unit;
      if(draw) {
        canvas.drawRect(Rect.fromLTWH(x, 0, bw, size.height), paint);
      }
      x += bw;
      draw = !draw;
    }
  }

  @override
  bool shouldRepaint(covariant _BarcodePreviewPainter oldDelegate) => false;
}

/// Paints the dashed free-margin guide rectangle, inset by [insetPx] on every
/// side, matching the Qt editor's dashed margin indicator.
class _MarginGuidePainter extends CustomPainter {
  const _MarginGuidePainter(this.insetPx);

  /// Inset of the guide from each card edge, in logical pixels.
  final double insetPx;

  @override
  void paint(Canvas canvas, Size size) {
    final rect = Rect.fromLTWH(insetPx, insetPx,
        size.width - 2 * insetPx, size.height - 2 * insetPx);
    if(rect.width <= 0 || rect.height <= 0) return;
    final paint = Paint()
      ..color = Colors.grey
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1.0;
    const dash = 4.0;
    const gap = 3.0;
    // Trace the rectangle perimeter as short dashes.
    for(final seg in [
      (rect.topLeft, rect.topRight),
      (rect.topRight, rect.bottomRight),
      (rect.bottomRight, rect.bottomLeft),
      (rect.bottomLeft, rect.topLeft),
    ]) {
      final (start, end) = seg;
      final total = (end - start).distance;
      final dir = (end - start) / total;
      var pos = 0.0;
      while(pos < total) {
        final a = start + dir * pos;
        final b = start + dir * math.min(pos + dash, total);
        canvas.drawLine(a, b, paint);
        pos += dash + gap;
      }
    }
  }

  @override
  bool shouldRepaint(covariant _MarginGuidePainter oldDelegate) =>
      oldDelegate.insetPx != insetPx;
}
