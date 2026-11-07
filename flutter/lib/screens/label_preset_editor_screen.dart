// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/id_card_layout.dart';
import '../models/label_preset.dart';
import '../providers/api_client_provider.dart';
import '../widgets/app_bar_title.dart';
import 'layout_editor_screen.dart';
import 'printer_calibration_screen.dart';

/// Manages named, device-local label presets: create/rename/delete, choose the
/// printer type and its parameters (thermal vs. sheet) plus the on-label
/// arrangement. Edits are kept in memory and written to the device only when
/// the user taps Save.
class LabelPresetEditorScreen extends ConsumerStatefulWidget {
  const LabelPresetEditorScreen({super.key});

  @override
  ConsumerState<LabelPresetEditorScreen> createState() =>
      _LabelPresetEditorScreenState();
}

class _LabelPresetEditorScreenState
    extends ConsumerState<LabelPresetEditorScreen> {
  final _store = LabelPresetStore();
  List<LabelPreset> _presets = [];
  int _index = 0;
  bool _loading = true;
  bool _dirty = false;

  // Stable controllers, re-synced when the selected preset changes.
  final _gap = TextEditingController();
  final _xOffset = TextEditingController();
  final _columns = TextEditingController();
  final _rows = TextEditingController();
  final _left = TextEditingController();
  final _top = TextEditingController();
  final _hGap = TextEditingController();
  final _vGap = TextEditingController();
  final _bar = TextEditingController();
  final _unit = TextEditingController();

  @override
  void initState() {
    super.initState();
    _load();
  }

  @override
  void dispose() {
    for(final c in [_gap, _xOffset, _columns, _rows, _left, _top, _hGap, _vGap,
      _bar, _unit]) {
      c.dispose();
    }
    super.dispose();
  }

  Future<void> _load() async {
    final presets = await _store.loadAll();
    if(!mounted) return;
    setState(() {
      _presets = List.of(presets);
      _index = 0;
      _loading = false;
      _syncControllers();
    });
  }

  LabelPreset get _preset => _presets[_index];

  void _syncControllers() {
    final p = _preset;
    String mm(double v) =>
        v == v.roundToDouble() ? v.toStringAsFixed(0) : v.toStringAsFixed(1);
    _gap.text = mm(p.gapMm);
    _xOffset.text = mm(p.xOffsetMm);
    _columns.text = '${p.columns}';
    _rows.text = '${p.rows}';
    _left.text = mm(p.leftMarginMm);
    _top.text = mm(p.topMarginMm);
    _hGap.text = mm(p.hGapMm);
    _vGap.text = mm(p.vGapMm);
    _bar.text = mm(p.barWidthReductionMm);
    _unit.text = p.barcodeUnitMm.toStringAsFixed(2);
  }

  /// Reads the text controllers into the in-memory preset (without persisting).
  void _applyControllers() {
    double d(TextEditingController c, double fb) =>
        double.tryParse(c.text.trim().replaceAll(',', '.')) ?? fb;
    int i(TextEditingController c, int fb) {
      final v = int.tryParse(c.text.trim());
      return (v == null || v < 1) ? fb : v;
    }

    final p = _preset;
    _presets[_index] = p.copyWith(
      gapMm: d(_gap, p.gapMm),
      xOffsetMm: d(_xOffset, p.xOffsetMm),
      columns: i(_columns, p.columns),
      rows: i(_rows, p.rows),
      leftMarginMm: d(_left, p.leftMarginMm),
      topMarginMm: d(_top, p.topMarginMm),
      hGapMm: d(_hGap, p.hGapMm),
      vGapMm: d(_vGap, p.vGapMm),
      barWidthReductionMm: d(_bar, p.barWidthReductionMm),
      barcodeUnitMm: d(_unit, p.barcodeUnitMm).clamp(0.15, 1.0),
    );
  }

  /// Applies a non-text change (dropdown/slider/switch) and marks dirty.
  void _update(LabelPreset Function(LabelPreset) change) {
    _applyControllers();
    setState(() {
      _presets[_index] = change(_preset);
      _dirty = true;
    });
  }

  Future<void> _save(AppLocalizations l10n) async {
    _applyControllers();
    await _store.saveAll(_presets);
    if(!mounted) return;
    setState(() => _dirty = false);
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(l10n.labelPresetSaved)),
    );
  }

  void _selectIndex(int i) {
    _applyControllers();
    setState(() {
      _index = i;
      _syncControllers();
    });
  }

  Future<void> _newPreset(AppLocalizations l10n) async {
    final name = await _promptName(l10n, l10n.labelPresetNew, '');
    if(name == null || name.trim().isEmpty) return;
    _applyControllers();
    setState(() {
      _presets.add(LabelPreset(
        name: name.trim(),
        purpose: LabelPurpose.bookLabel,
        printerType: PrinterType.thermalUsb,
      ));
      _index = _presets.length - 1;
      _dirty = true;
      _syncControllers();
    });
  }

  Future<void> _rename(AppLocalizations l10n) async {
    final name = await _promptName(l10n, l10n.labelPresetRename, _preset.name);
    if(name == null || name.trim().isEmpty) return;
    _update((p) => p.copyWith(name: name.trim()));
  }

  void _delete() {
    if(_presets.length <= 1) return;
    setState(() {
      _presets.removeAt(_index);
      _index = 0;
      _dirty = true;
      _syncControllers();
    });
  }

  Future<String?> _promptName(
      AppLocalizations l10n, String title, String initial) {
    final ctrl = TextEditingController(text: initial);
    return showDialog<String>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(title),
        content: TextField(
          controller: ctrl,
          autofocus: true,
          decoration: InputDecoration(labelText: l10n.labelPresetName),
        ),
        actions: [
          TextButton(
              onPressed: () => Navigator.pop(ctx), child: Text(l10n.cancel)),
          FilledButton(
              onPressed: () => Navigator.pop(ctx, ctrl.text),
              child: Text(l10n.ok)),
        ],
      ),
    );
  }

  Future<void> _editArrangement(AppLocalizations l10n) async {
    _applyControllers();
    final purpose = _preset.purpose;
    if(purpose == LabelPurpose.signature) {
      final s = _preset.signatureLayout;
      final asCard = IdCardLayout(
        cardWidthMm: s.cardWidthMm,
        cardHeightMm: s.cardHeightMm,
        hasLogo: false,
        barcode: const IdCardElement(),
        text: s.text,
        logo: const IdCardElement(),
      );
      await Navigator.of(context).push(MaterialPageRoute(
        builder: (_) => LayoutEditorScreen(
          title: l10n.labelPresetArrangement,
          iconAsset: 'assets/icons/edit_label_layouts.jpeg',
          savedMessage: l10n.labelPresetSaved,
          sampleTextLines: [l10n.signatureSample],
          showBarcodeZone: false,
          showLogoToggle: false,
          marginStore:
              const IdCardMarginStore('label_preset_signature_margin_mm'),
          onLoad: (_) async => LayoutEditorData(layout: asCard),
          onSave: (_, layout) async {
            setState(() {
              _presets[_index] = _preset.copyWith(
                  signatureLayout: s.copyWith(
                      cardWidthMm: layout.cardWidthMm,
                      cardHeightMm: layout.cardHeightMm,
                      text: layout.text));
            });
            await _store.saveAll(_presets);
          },
        ),
      ));
      return;
    }
    final logo = await _logoBytes();
    if(!mounted) return;
    await Navigator.of(context).push(MaterialPageRoute(
      builder: (_) => LayoutEditorScreen(
        title: l10n.labelPresetArrangement,
        iconAsset: 'assets/icons/edit_label_layouts.jpeg',
        savedMessage: l10n.labelPresetSaved,
        sampleTextLines: [
          l10n.bookLabelSampleOrganization,
          l10n.bookLabelSampleTitle,
          l10n.bookLabelSampleAuthor,
        ],
        marginStore: const IdCardMarginStore('label_preset_card_margin_mm'),
        onLoad: (_) async =>
            LayoutEditorData(layout: _preset.cardLayout, logoBytes: logo),
        onSave: (_, layout) async {
          setState(() => _presets[_index] = _preset.copyWith(cardLayout: layout));
          await _store.saveAll(_presets);
        },
      ),
    ));
  }

  Future<Uint8List?> _logoBytes() async {
    try {
      final info = await ref.read(collectionApiProvider).get();
      return info.logoBytes;
    } catch(_) {
      return null;
    }
  }

  // ---- UI -----------------------------------------------------------------

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/edit_label_layouts.jpeg',
          title: l10n.labelPresetTitle,
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.add),
            tooltip: l10n.labelPresetNew,
            onPressed: _loading ? null : () => _newPreset(l10n),
          ),
          IconButton(
            icon: const Icon(Icons.save),
            tooltip: l10n.labelPresetSave,
            onPressed: (_loading || !_dirty) ? null : () => _save(l10n),
          ),
        ],
      ),
      body: _loading
          ? const Center(child: CircularProgressIndicator())
          : ListView(
              padding: const EdgeInsets.all(12),
              children: _buildFields(l10n),
            ),
    );
  }

  List<Widget> _buildFields(AppLocalizations l10n) {
    final p = _preset;
    return [
      Row(
        children: [
          Expanded(
            child: DropdownButtonFormField<int>(
              initialValue: _index,
              isExpanded: true,
              decoration: InputDecoration(
                labelText: l10n.labelPresetPreset,
                border: const OutlineInputBorder(),
                isDense: true,
              ),
              items: [
                for(var i = 0; i < _presets.length; i++)
                  DropdownMenuItem(value: i, child: Text(_presets[i].name)),
              ],
              onChanged: (i) => _selectIndex(i ?? 0),
            ),
          ),
          IconButton(
            icon: const Icon(Icons.drive_file_rename_outline),
            tooltip: l10n.labelPresetRename,
            onPressed: () => _rename(l10n),
          ),
          IconButton(
            icon: const Icon(Icons.delete_outline),
            tooltip: l10n.labelPresetDelete,
            onPressed: _presets.length <= 1 ? null : _delete,
          ),
        ],
      ),
      const SizedBox(height: 12),
      _dropdown<LabelPurpose>(
        label: l10n.labelPresetPurpose,
        value: p.purpose,
        items: {
          LabelPurpose.bookLabel: l10n.labelPurposeBookLabel,
          LabelPurpose.signature: l10n.labelPurposeSignature,
          LabelPurpose.idCard: l10n.labelPurposeIdCard,
        },
        onChanged: (v) => _update((pr) => pr.copyWith(purpose: v)),
      ),
      const SizedBox(height: 12),
      _dropdown<PrinterType>(
        label: l10n.labelPresetPrinter,
        value: p.printerType,
        items: {
          PrinterType.thermalUsb: l10n.labelPrinterThermal,
          PrinterType.systemSheet: l10n.labelPrinterSheet,
        },
        onChanged: (v) => _update((pr) => pr.copyWith(printerType: v)),
      ),
      const SizedBox(height: 12),
      if(p.printerType == PrinterType.thermalUsb)
        ..._thermalFields(l10n, p)
      else
        ..._sheetFields(l10n),
      const SizedBox(height: 8),
      Row(
        children: [
          Expanded(child: _numField(_bar, l10n.labelPresetBarWidthReduction)),
          const SizedBox(width: 12),
          Expanded(child: _numField(_unit, l10n.labelBarcodeUnit)),
        ],
      ),
      const SizedBox(height: 16),
      FilledButton.tonalIcon(
        onPressed: () => _editArrangement(l10n),
        icon: const Icon(Icons.dashboard_customize),
        label: Text(l10n.labelPresetEditArrangement),
      ),
    ];
  }

  List<Widget> _thermalFields(AppLocalizations l10n, LabelPreset p) {
    return [
      Row(
        children: [
          Text(l10n.labelPresetDarkness),
          Expanded(
            child: Slider(
              value: p.darkness.toDouble(),
              min: 0,
              max: LabelPreset.maxDarkness.toDouble(),
              divisions: LabelPreset.maxDarkness,
              label: '${p.darkness}',
              onChanged: (v) => _update((pr) => pr.copyWith(darkness: v.round())),
            ),
          ),
          SizedBox(width: 24, child: Text('${p.darkness}')),
        ],
      ),
      Row(
        children: [
          Expanded(child: _numField(_gap, l10n.labelPresetGap)),
          const SizedBox(width: 12),
          Expanded(child: _numField(_xOffset, l10n.labelPresetXOffset)),
        ],
      ),
    ];
  }

  List<Widget> _sheetFields(AppLocalizations l10n) {
    return [
      Row(
        children: [
          Expanded(child: _numField(_columns, l10n.labelPresetColumns)),
          const SizedBox(width: 12),
          Expanded(child: _numField(_rows, l10n.labelPresetRows)),
        ],
      ),
      const SizedBox(height: 8),
      Row(
        children: [
          Expanded(child: _numField(_left, l10n.labelPresetLeftMargin)),
          const SizedBox(width: 12),
          Expanded(child: _numField(_top, l10n.labelPresetTopMargin)),
        ],
      ),
      const SizedBox(height: 8),
      Row(
        children: [
          Expanded(child: _numField(_hGap, l10n.labelPresetHGap)),
          const SizedBox(width: 12),
          Expanded(child: _numField(_vGap, l10n.labelPresetVGap)),
        ],
      ),
      const SizedBox(height: 8),
      Row(
        children: [
          Expanded(
            child: _dropdown<PaperSize>(
              label: l10n.labelPresetPaperSize,
              value: _preset.paperSize,
              items: {
                for(final ps in PaperSize.values) ps: ps.name.toUpperCase(),
              },
              onChanged: (v) => _update((pr) => pr.copyWith(paperSize: v)),
            ),
          ),
          const SizedBox(width: 12),
          Expanded(
            child: SwitchListTile(
              contentPadding: EdgeInsets.zero,
              title: Text(l10n.labelPresetLandscape),
              value: _preset.landscape,
              onChanged: (v) => _update((pr) => pr.copyWith(landscape: v)),
            ),
          ),
        ],
      ),
      const SizedBox(height: 12),
      OutlinedButton.icon(
        onPressed: _calibrate,
        icon: const Icon(Icons.straighten),
        label: Text(l10n.calibrationButton),
      ),
    ];
  }

  /// Opens the guided calibration screen for the current sheet preset and
  /// stores the result in that preset.
  Future<void> _calibrate() async {
    _applyControllers();
    await Navigator.of(context).push(MaterialPageRoute(
      builder: (_) => PrinterCalibrationScreen(
        initial: _preset.calibration,
        onSave: (cal) async {
          setState(() => _presets[_index] = _preset.copyWith(calibration: cal));
          await _store.saveAll(_presets);
        },
      ),
    ));
  }

  Widget _dropdown<T>({
    required String label,
    required T value,
    required Map<T, String> items,
    required void Function(T) onChanged,
  }) {
    return DropdownButtonFormField<T>(
      initialValue: value,
      isExpanded: true,
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
        isDense: true,
      ),
      items: [
        for(final e in items.entries)
          DropdownMenuItem(value: e.key, child: Text(e.value)),
      ],
      onChanged: (v) {
        if(v != null) onChanged(v);
      },
    );
  }

  Widget _numField(TextEditingController controller, String label) {
    return TextField(
      controller: controller,
      keyboardType: const TextInputType.numberWithOptions(decimal: true),
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
        isDense: true,
      ),
      onChanged: (_) {
        if(!_dirty) setState(() => _dirty = true);
      },
    );
  }
}
