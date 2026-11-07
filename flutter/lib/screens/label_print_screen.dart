// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/id_card_layout.dart';
import '../models/item.dart';
import '../models/label_preset.dart';
import '../printing/label_bitmap.dart';
import '../printing/usb_label_printer.dart';
import '../providers/api_client_provider.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';
import 'sheet_positions_screen.dart';

/// Initial data to pre-configure a [LabelPrintScreen], e.g. from the
/// acquisition success card: pre-selects [itemId], pre-fills the search
/// field with [itemName] and sets the per-item copy count to [copies].
class LabelPrintInitialData {
  const LabelPrintInitialData({
    required this.itemId,
    required this.itemName,
    this.copies = 1,
  });

  /// Item to pre-select for printing.
  final int itemId;

  /// Item name, pre-filled into the search field so the selection is visible.
  final String itemName;

  /// Initial number of identical labels per item.
  final int copies;
}

/// Prints labels for items by first selecting items, then a named [LabelPreset]
/// (which carries the printer type, label size, labels-per-sheet and the
/// on-label arrangement). Thermal presets print over USB (TSPL); sheet presets
/// tile the labels at real size and print through the system dialog, where the
/// user first chooses which sheet positions to use.
class LabelPrintScreen extends ConsumerStatefulWidget {
  const LabelPrintScreen({super.key, this.initialData});

  /// Optional pre-configuration, e.g. from the acquisition success card.
  final LabelPrintInitialData? initialData;

  @override
  ConsumerState<LabelPrintScreen> createState() => _LabelPrintScreenState();
}

class _LabelPrintScreenState extends ConsumerState<LabelPrintScreen> {
  final _presetStore = LabelPresetStore();
  final _printer = UsbLabelPrinter();

  List<LabelPreset> _presets = [];
  LabelPreset? _preset;
  List<SearchItemRow> _rows = [];
  final Set<int> _selected = {};
  UsbDeviceInfo? _device;
  ui.Image? _logo;
  String _org = '';
  bool _loading = false;
  bool _printing = false;
  Object? _error;
  String _search = '';

  /// Number of identical labels printed per selected item.
  int _copies = 1;

  final _searchCtrl = TextEditingController();

  @override
  void initState() {
    super.initState();
    final init = widget.initialData;
    if(init != null) {
      _selected.add(init.itemId);
      _copies = init.copies.clamp(1, 99);
      _search = init.itemName;
      _searchCtrl.text = init.itemName;
    }
    _loadPresets();
    _loadCollection();
    _refreshDevice();
    _load();
  }

  @override
  void dispose() {
    _searchCtrl.dispose();
    super.dispose();
  }

  Future<void> _loadPresets() async {
    final all = await _presetStore.loadAll();
    final usable =
        all.where((p) => p.purpose == LabelPurpose.bookLabel).toList();
    final lastName = await _presetStore.lastPresetName('bookLabel');
    if(!mounted) return;
    setState(() {
      _presets = usable;
      _preset = _presetFor(usable, lastName);
    });
  }

  /// The remembered preset (by [name]) if it still exists, else the first.
  LabelPreset? _presetFor(List<LabelPreset> presets, String? name) {
    for(final p in presets) {
      if(p.name == name) return p;
    }
    return presets.isNotEmpty ? presets.first : null;
  }

  Future<void> _loadCollection() async {
    ui.Image? logo;
    var org = '';
    try {
      final info = await ref.read(collectionApiProvider).get();
      org = info.organization;
      final bytes = info.logoBytes;
      if(bytes != null && bytes.isNotEmpty) {
        final codec = await ui.instantiateImageCodec(bytes);
        logo = (await codec.getNextFrame()).image;
      }
    } catch(_) {
      // Missing collection/logo only affects the logo zone.
    }
    if(!mounted) return;
    setState(() {
      _logo = logo;
      _org = org;
    });
  }

  Future<void> _refreshDevice() async {
    final device = await _printer.findPrinter();
    if(!mounted) return;
    setState(() => _device = device);
  }

  Future<void> _load() async {
    if(!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final rows = await ref
          .read(itemsApiProvider)
          .search(SearchQuery(general: _search));
      if(!mounted) return;
      setState(() {
        _rows = rows;
        _loading = false;
      });
    } catch(e) {
      if(!mounted) return;
      setState(() {
        _error = e;
        _loading = false;
      });
    }
  }

  void _onSearchChanged(String value) {
    _search = value;
    _load();
  }

  void _toggle(int id, bool? on) {
    setState(() {
      if(on == true) {
        _selected.add(id);
      } else {
        _selected.remove(id);
      }
    });
  }

  // ---- content mapping ----------------------------------------------------

  /// The arrangement, barcode and text lines to render for [item] under [p].
  ({IdCardLayout layout, String barcode, List<LabelTextLine> text, bool useLogo})
      _content(Item item, LabelPreset p) {
    if(p.purpose == LabelPurpose.signature) {
      final s = p.signatureLayout;
      final sig = _signatureText(item);
      return (
        layout: IdCardLayout(
          cardWidthMm: s.cardWidthMm,
          cardHeightMm: s.cardHeightMm,
          hasLogo: false,
          barcode: const IdCardElement(),
          text: s.text,
          logo: const IdCardElement(),
        ),
        barcode: '',
        text: sig.isEmpty ? const [] : [LabelTextLine(sig)],
        useLogo: false,
      );
    }
    return (
      layout: p.cardLayout,
      barcode: item.barcode ?? '',
      text: _labelTextLines(item),
      useLogo: p.cardLayout.hasLogo,
    );
  }

  String _signatureText(Item item) => [item.signatureTop, item.signatureBottom]
      .where((s) => (s ?? '').trim().isNotEmpty)
      .map((s) => s!.trim())
      .join(' ');

  /// Book-label text lines, mirroring the Qt client: organisation, title (up to
  /// two word-wrapped lines), author, ISBN and the combined spine signature.
  List<LabelTextLine> _labelTextLines(Item item) {
    String? listVal(List<String> keys) {
      for(final k in keys) {
        final v = item.listValues[k];
        if(v != null) {
          final parts = v.where((s) => s.trim().isNotEmpty).toList();
          if(parts.isNotEmpty) return parts.join(', ');
        }
      }
      return null;
    }

    String? mapVal(List<String> keys) {
      for(final k in keys) {
        final v = item.values[k];
        if(v != null && v.trim().isNotEmpty) return v.trim();
      }
      return null;
    }

    final title = mapVal(['title']) ?? item.name;
    final author = listVal(['authors', 'author', 'Autoren', 'Autor', 'Author']) ??
        mapVal(['author', 'authors']);
    final isbn = listVal(['ISBN13', 'ISBN10']);
    final signature = _signatureText(item);

    return [
      if(_org.isNotEmpty) LabelTextLine(_org),
      LabelTextLine(title, maxLines: 2, truncate: true),
      if(author != null) LabelTextLine(author, truncate: true),
      if(isbn != null) LabelTextLine('ISBN: $isbn'),
      if(signature.isNotEmpty) LabelTextLine(signature),
    ];
  }

  // ---- printing -----------------------------------------------------------

  Future<List<Item>> _fetchSelected() async {
    final itemsApi = ref.read(itemsApiProvider);
    final items = <Item>[];
    for(final id in _selected) {
      items.add(await itemsApi.getById(id));
    }
    return items;
  }

  Future<void> _print(AppLocalizations l10n) async {
    final preset = _preset;
    if(preset == null) return;
    if(_selected.isEmpty) {
      _snack(l10n.labelPrintNoSelection);
      return;
    }
    if(preset.printerType == PrinterType.thermalUsb) {
      await _printThermal(l10n, preset);
    } else {
      await _printSheet(preset);
    }
  }

  Future<void> _printThermal(AppLocalizations l10n, LabelPreset preset) async {
    final device = _device;
    if(device == null) {
      _snack(l10n.labelPrintNoPrinter);
      return;
    }
    setState(() { _printing = true; });
    try {
      if(!device.hasPermission) {
        final granted = await _printer.requestPermission(device.name);
        if(!granted) {
          if(!mounted) return;
          setState(() { _printing = false; });
          _snack(l10n.labelPrintPermissionDenied);
          return;
        }
        await _refreshDevice();
      }
      final items = await _fetchSelected();
      final bitmaps = <MonoBitmap>[];
      for(final item in items) {
        final c = _content(item, preset);
        final bitmap = await renderBookLabel(
          layout: c.layout,
          barcode: c.barcode,
          textLines: c.text,
          logo: c.useLogo ? _logo : null,
          dpi: preset.dpi,
          barWidthReductionMm: preset.barWidthReductionMm,
          barcodeUnitMm: preset.barcodeUnitMm,
        );
        for(var i = 0; i < _copies; i++) {
          bitmaps.add(bitmap);
        }
      }
      final bytes = labelsToTsplBitmap(
        bitmaps,
        dpi: preset.dpi,
        gapMm: preset.gapMm,
        darkness: preset.darkness,
        xOffsetMm: preset.xOffsetMm,
        profile: TsplProfile.forDevice(device.vendorId, device.productId),
      );
      await _printer.printBytes((_device ?? device).name, bytes);
      if(!mounted) return;
      setState(() { _printing = false; });
      _snack(l10n.labelPrintSent(bitmaps.length));
    } catch(e) {
      if(!mounted) return;
      setState(() { _printing = false; });
      ServerErrorDialog.show(context, e, l10n);
    }
  }

  /// Sheet printing: hand the chosen items and a render callback to the
  /// position-picker screen, which prints one sheet at a time. Each selected
  /// item appears [_copies] times in the list; renders are cached per id so
  /// copies do not refetch and rerender the same label.
  Future<void> _printSheet(LabelPreset preset) async {
    final ids = [for(final id in _selected) ...List.filled(_copies, id)];
    final itemsApi = ref.read(itemsApiProvider);
    final cache = <int, Uint8List>{};
    Future<Uint8List> renderPng(int id) async {
      final cached = cache[id];
      if(cached != null) return cached;
      final item = await itemsApi.getById(id);
      final c = _content(item, preset);
      final png = await renderBookLabelPng(
        layout: c.layout,
        barcode: c.barcode,
        textLines: c.text,
        logo: c.useLogo ? _logo : null,
        dpi: preset.dpi,
        barWidthReductionMm: preset.barWidthReductionMm,
        barcodeUnitMm: preset.barcodeUnitMm,
      );
      return cache[id] = png;
    }

    await Navigator.of(context).push(MaterialPageRoute(
      builder: (_) => SheetPositionsScreen(
        preset: preset,
        itemIds: ids,
        renderPng: renderPng,
      ),
    ));
  }

  void _snack(String text) {
    ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(text)));
  }

  // ---- UI -----------------------------------------------------------------

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final preset = _preset;
    final isThermal = preset?.printerType == PrinterType.thermalUsb;
    final canPrint = !_printing &&
        _selected.isNotEmpty &&
        preset != null &&
        (!isThermal || _device != null);
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/print_label.jpeg',
          title: l10n.labelPrintTitle,
        ),
        actions: [
          IconButton(
            icon: Icon(isThermal ? Icons.usb : Icons.navigate_next),
            tooltip: isThermal ? l10n.labelPrintButton : l10n.labelPrintNext,
            onPressed: canPrint ? () => _print(l10n) : null,
          ),
        ],
      ),
      body: CustomScrollView(
        slivers: [
          SliverToBoxAdapter(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                _buildPresetSelector(l10n),
                if(isThermal && _printer.isSupported)
                  _buildPrinterStatus(l10n),
                Padding(
                  padding: const EdgeInsets.fromLTRB(12, 8, 12, 4),
                  child: TextField(
                    controller: _searchCtrl,
                    decoration: InputDecoration(
                      labelText: l10n.labelPrintSearchHint,
                      prefixIcon: const Icon(Icons.search),
                      border: const OutlineInputBorder(),
                      isDense: true,
                    ),
                    onChanged: _onSearchChanged,
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 12),
                  child: Row(
                    children: [
                      Text(l10n.labelPrintCopies),
                      const Spacer(),
                      IconButton(
                        icon: const Icon(Icons.remove),
                        onPressed: _copies > 1
                            ? () { setState(() { _copies--; }); }
                            : null,
                      ),
                      Text(
                        '$_copies',
                        style: Theme.of(context).textTheme.titleMedium,
                      ),
                      IconButton(
                        icon: const Icon(Icons.add),
                        onPressed: _copies < 99
                            ? () { setState(() { _copies++; }); }
                            : null,
                      ),
                    ],
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 12),
                  child: Row(
                    children: [
                      Text(l10n.labelPrintSelectedCount(_selected.length)),
                      const Spacer(),
                      TextButton(
                        onPressed: _rows.isEmpty
                            ? null
                            : () => setState(
                                () => _selected.addAll(_rows.map((r) => r.id))),
                        child: Text(l10n.labelPrintSelectAll),
                      ),
                      TextButton(
                        onPressed: _selected.isEmpty
                            ? null
                            : () => setState(_selected.clear),
                        child: Text(l10n.labelPrintDeselectAll),
                      ),
                    ],
                  ),
                ),
                if(_printing || _loading) const LinearProgressIndicator(),
                const Divider(height: 1),
              ],
            ),
          ),
          _buildListSliver(l10n),
        ],
      ),
    );
  }

  Widget _buildPresetSelector(AppLocalizations l10n) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(12, 12, 12, 0),
      child: DropdownButtonFormField<LabelPreset>(
        initialValue: _preset,
        isExpanded: true,
        decoration: InputDecoration(
          labelText: l10n.labelPrintPreset,
          border: const OutlineInputBorder(),
          isDense: true,
        ),
        items: [
          for(final p in _presets)
            DropdownMenuItem(value: p, child: Text(p.name)),
        ],
        onChanged: (p) {
          if(p != null) {
            setState(() => _preset = p);
            _presetStore.setLastPresetName('bookLabel', p.name);
          }
        },
      ),
    );
  }

  Widget _buildPrinterStatus(AppLocalizations l10n) {
    final device = _device;
    return ListTile(
      leading: Icon(device == null ? Icons.usb_off : Icons.usb,
          color: device == null ? Colors.grey : Colors.green),
      title: Text(device?.label ?? l10n.labelPrintNoPrinter),
      subtitle: device != null ? Text(l10n.labelPrintPrinterConnected) : null,
      trailing: IconButton(
        icon: const Icon(Icons.refresh),
        tooltip: l10n.labelPrintRefresh,
        onPressed: _refreshDevice,
      ),
    );
  }

  Widget _buildListSliver(AppLocalizations l10n) {
    if(_loading && _rows.isEmpty) {
      return const SliverToBoxAdapter(child: SizedBox.shrink());
    }
    if(_error != null && _rows.isEmpty) {
      return SliverToBoxAdapter(
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: ErrorDisplay(error: _error!, l10n: l10n),
        ),
      );
    }
    if(_rows.isEmpty) {
      return SliverToBoxAdapter(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Center(child: Text(l10n.labelPrintNoItems)),
        ),
      );
    }
    return SliverList.separated(
      itemCount: _rows.length,
      separatorBuilder: (_, __) => const Divider(height: 1),
      itemBuilder: (ctx, i) {
        final r = _rows[i];
        return CheckboxListTile(
          value: _selected.contains(r.id),
          onChanged: (on) => _toggle(r.id, on),
          title: Text(r.name),
          subtitle: Text(r.typeLocale),
        );
      },
    );
  }
}
