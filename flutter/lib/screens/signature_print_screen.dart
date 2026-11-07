// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
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

/// Prints signature (spine call-number) labels. Mirrors the desktop signature
/// print page: optionally pick one item to prefill the top and bottom signature
/// fields, then enter or choose a known signature in each field and print a
/// chosen number of copies — each field has its own count and Print button.
class SignaturePrintScreen extends ConsumerStatefulWidget {
  const SignaturePrintScreen({super.key});

  @override
  ConsumerState<SignaturePrintScreen> createState() =>
      _SignaturePrintScreenState();
}

class _SignaturePrintScreenState extends ConsumerState<SignaturePrintScreen> {
  final _printer = UsbLabelPrinter();
  final _presetStore = LabelPresetStore();

  List<LabelPreset> _presets = [];
  LabelPreset? _preset;
  List<SearchItemRow> _rows = [];
  List<String> _topOptions = [];
  List<String> _bottomOptions = [];
  UsbDeviceInfo? _device;
  bool _loading = false;
  bool _printing = false;
  Object? _error;
  String _search = '';

  final _searchCtrl = TextEditingController();
  final _top = TextEditingController();
  final _bottom = TextEditingController();
  final _topCount = TextEditingController(text: '1');
  final _bottomCount = TextEditingController(text: '1');

  @override
  void initState() {
    super.initState();
    _loadPresets();
    _loadSignatures();
    _refreshDevice();
    _load();
  }

  @override
  void dispose() {
    _searchCtrl.dispose();
    _top.dispose();
    _bottom.dispose();
    _topCount.dispose();
    _bottomCount.dispose();
    super.dispose();
  }

  Future<void> _loadPresets() async {
    final all = await _presetStore.loadAll();
    final sig =
        all.where((p) => p.purpose == LabelPurpose.signature).toList();
    final lastName = await _presetStore.lastPresetName('signature');
    if(!mounted) return;
    setState(() {
      _presets = sig;
      LabelPreset? sel;
      for(final p in sig) {
        if(p.name == lastName) sel = p;
      }
      _preset = sel ?? (sig.isNotEmpty ? sig.first : null);
    });
  }

  Future<void> _loadSignatures() async {
    try {
      final api = ref.read(signaturesApiProvider);
      final top = await api.top();
      final bottom = await api.bottom();
      if(!mounted) return;
      setState(() {
        _topOptions = top;
        _bottomOptions = bottom;
      });
    } catch(_) {
      // Suggestions are optional; free text still works.
    }
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

  Future<void> _prefillFromItem(int id) async {
    try {
      final item = await ref.read(itemsApiProvider).getById(id);
      if(!mounted) return;
      setState(() {
        _top.text = item.signatureTop ?? '';
        _bottom.text = item.signatureBottom ?? '';
      });
    } catch(e) {
      if(!mounted) return;
      ServerErrorDialog.show(context, e, AppLocalizations.of(context)!);
    }
  }

  IdCardLayout _signatureLayoutFor(LabelPreset preset) {
    final s = preset.signatureLayout;
    return IdCardLayout(
      cardWidthMm: s.cardWidthMm,
      cardHeightMm: s.cardHeightMm,
      hasLogo: false,
      barcode: const IdCardElement(),
      text: s.text,
      logo: const IdCardElement(),
    );
  }

  Future<void> _print(AppLocalizations l10n, String text, int count) async {
    final preset = _preset;
    if(preset == null) return;
    if(text.trim().isEmpty) {
      _snack(l10n.signaturePrintEmpty);
      return;
    }
    final layout = _signatureLayoutFor(preset);
    final lines = [LabelTextLine(text.trim(), center: true)];
    if(preset.printerType == PrinterType.thermalUsb) {
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
        final bmp = await renderBookLabel(
          layout: layout,
          barcode: '',
          textLines: lines,
          dpi: preset.dpi,
          barWidthReductionMm: preset.barWidthReductionMm,
        );
        final bytes = labelsToTsplBitmap(
          List.filled(count, bmp),
          dpi: preset.dpi,
          gapMm: preset.gapMm,
          darkness: preset.darkness,
          xOffsetMm: preset.xOffsetMm,
          profile: TsplProfile.forDevice(device.vendorId, device.productId),
        );
        await _printer.printBytes((_device ?? device).name, bytes);
        if(!mounted) return;
        setState(() { _printing = false; });
        _snack(l10n.labelPrintSent(count));
      } catch(e) {
        if(!mounted) return;
        setState(() { _printing = false; });
        ServerErrorDialog.show(context, e, l10n);
      }
      return;
    }
    // Sheet printer: render once and reuse for every copy.
    setState(() { _printing = true; });
    try {
      final png = await renderBookLabelPng(
        layout: layout,
        barcode: '',
        textLines: lines,
        dpi: preset.dpi,
        barWidthReductionMm: preset.barWidthReductionMm,
      );
      if(!mounted) return;
      setState(() { _printing = false; });
      await Navigator.of(context).push(MaterialPageRoute(
        builder: (_) => SheetPositionsScreen(
          preset: preset,
          itemIds: List.generate(count, (i) => i),
          renderPng: (_) async => png,
        ),
      ));
    } catch(e) {
      if(!mounted) return;
      setState(() { _printing = false; });
      ServerErrorDialog.show(context, e, l10n);
    }
  }

  void _snack(String text) {
    ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(text)));
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final isThermal = _preset?.printerType == PrinterType.thermalUsb;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/print_signatures.jpeg',
          title: l10n.signaturePrintTitle,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.all(12),
        children: [
          DropdownButtonFormField<LabelPreset>(
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
                _presetStore.setLastPresetName('signature', p.name);
              }
            },
          ),
          if(isThermal && _printer.isSupported)
            ListTile(
              contentPadding: EdgeInsets.zero,
              leading: Icon(_device == null ? Icons.usb_off : Icons.usb,
                  color: _device == null ? Colors.grey : Colors.green),
              title: Text(_device?.label ?? l10n.labelPrintNoPrinter),
              trailing: IconButton(
                icon: const Icon(Icons.refresh),
                tooltip: l10n.labelPrintRefresh,
                onPressed: _refreshDevice,
              ),
            ),
          const SizedBox(height: 8),
          _signatureField(l10n, l10n.signaturePrintTop, _top, _topOptions,
              _topCount, () => _print(l10n, _top.text, _parseCount(_topCount))),
          const SizedBox(height: 12),
          _signatureField(
              l10n,
              l10n.signaturePrintBottom,
              _bottom,
              _bottomOptions,
              _bottomCount,
              () => _print(l10n, _bottom.text, _parseCount(_bottomCount))),
          const SizedBox(height: 16),
          if(_printing) const LinearProgressIndicator(),
          const Divider(),
          Padding(
            padding: const EdgeInsets.symmetric(vertical: 8),
            child: Text(l10n.signaturePrintPickItem,
                style: Theme.of(context).textTheme.titleSmall),
          ),
          TextField(
            controller: _searchCtrl,
            decoration: InputDecoration(
              labelText: l10n.labelPrintSearchHint,
              prefixIcon: const Icon(Icons.search),
              border: const OutlineInputBorder(),
              isDense: true,
            ),
            onChanged: _onSearchChanged,
          ),
          const SizedBox(height: 8),
          if(_loading) const LinearProgressIndicator(),
          if(_error != null)
            ErrorDisplay(error: _error!, l10n: l10n)
          else
            ..._rows.map((r) => ListTile(
                  dense: true,
                  title: Text(r.name),
                  subtitle: Text(r.typeLocale),
                  onTap: () => _prefillFromItem(r.id),
                )),
        ],
      ),
    );
  }

  int _parseCount(TextEditingController c) {
    final v = int.tryParse(c.text.trim()) ?? 1;
    return v < 1 ? 1 : v;
  }

  Widget _signatureField(
    AppLocalizations l10n,
    String label,
    TextEditingController controller,
    List<String> options,
    TextEditingController countCtrl,
    VoidCallback onPrint,
  ) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Autocomplete<String>(
          initialValue: controller.value,
          optionsBuilder: (v) {
            final q = v.text.toLowerCase();
            if(q.isEmpty) return options;
            return options.where((o) => o.toLowerCase().contains(q));
          },
          onSelected: (s) => controller.text = s,
          fieldViewBuilder: (ctx, textCtrl, focus, onSubmit) {
            // Keep our controller in sync with the autocomplete's field.
            textCtrl.text = controller.text;
            return TextField(
              controller: textCtrl,
              focusNode: focus,
              decoration: InputDecoration(
                labelText: label,
                border: const OutlineInputBorder(),
                isDense: true,
              ),
              onChanged: (v) => controller.text = v,
            );
          },
        ),
        const SizedBox(height: 6),
        Row(
          children: [
            SizedBox(
              width: 90,
              child: TextField(
                keyboardType: TextInputType.number,
                decoration: InputDecoration(
                  labelText: l10n.signaturePrintCount,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
                controller: countCtrl,
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: FilledButton.icon(
                onPressed: _printing ? null : onPrint,
                icon: const Icon(Icons.print),
                label: Text(l10n.signaturePrintButton),
              ),
            ),
          ],
        ),
      ],
    );
  }
}
