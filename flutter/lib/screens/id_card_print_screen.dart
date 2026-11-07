// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:pdf/pdf.dart';
import 'package:pdf/widgets.dart' as pw;
import 'package:printing/printing.dart';

import '../api/collection_api.dart';
import '../models/id_card_layout.dart';
import '../models/label_grid_settings.dart';
import '../models/person.dart';
import '../printing/label_bitmap.dart';
import '../providers/api_client_provider.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';
import 'printer_calibration_screen.dart';

/// Screen to select persons and print their ID cards.
///
/// Available to any user holding the `print_ids` permission (enforced by the
/// router). Cards are rendered client-side into a PDF (tiled on A4) and handed
/// to the platform print/preview dialog. Person barcodes are computed locally
/// from the collection prefix, so no extra (`view_persons`) permission is
/// required.
class IdCardPrintScreen extends ConsumerStatefulWidget {
  const IdCardPrintScreen({super.key});

  @override
  ConsumerState<IdCardPrintScreen> createState() =>
      _IdCardPrintScreenState();
}

class _IdCardPrintScreenState extends ConsumerState<IdCardPrintScreen> {
  List<Person> _persons = [];
  final Set<int> _selected = {};
  CollectionInfo? _info;
  // The person ID-card layout is kept locally per client (seeded once from the
  // server); it is no longer taken from the fetched collection.
  final _layoutStore = const IdCardLayoutStore();
  IdCardLayout? _layout;
  bool _loading = false;
  bool _printing = false;
  Object? _error;
  String _search = '';

  final _searchCtrl = TextEditingController();

  // Label-sheet arrangement (columns/rows/margins/gaps), persisted on device.
  final _gridStore = LabelGridStore();
  LabelGridSettings _grid = LabelGridSettings.defaults;
  final _columnsCtrl = TextEditingController();
  final _rowsCtrl = TextEditingController();
  final _leftCtrl = TextEditingController();
  final _topCtrl = TextEditingController();
  final _hGapCtrl = TextEditingController();
  final _vGapCtrl = TextEditingController();
  final _unitCtrl = TextEditingController();

  @override
  void initState() {
    super.initState();
    _syncGridControllers();
    _loadGrid();
    _load();
  }

  @override
  void dispose() {
    _searchCtrl.dispose();
    _columnsCtrl.dispose();
    _rowsCtrl.dispose();
    _leftCtrl.dispose();
    _topCtrl.dispose();
    _hGapCtrl.dispose();
    _vGapCtrl.dispose();
    _unitCtrl.dispose();
    super.dispose();
  }

  Future<void> _loadGrid() async {
    final g = await _gridStore.load();
    if(!mounted) return;
    setState(() {
      _grid = g;
      _syncGridControllers();
    });
  }

  void _syncGridControllers() {
    _columnsCtrl.text = _grid.columns.toString();
    _rowsCtrl.text = _grid.rows.toString();
    _leftCtrl.text = _fmtMm(_grid.leftMarginMm);
    _topCtrl.text = _fmtMm(_grid.topMarginMm);
    _hGapCtrl.text = _fmtMm(_grid.hGapMm);
    _vGapCtrl.text = _fmtMm(_grid.vGapMm);
    _unitCtrl.text = _grid.barcodeUnitMm.toStringAsFixed(2);
  }

  void _applyGridFromControllers() {
    int parseInt(TextEditingController c, int fallback) =>
        int.tryParse(c.text.trim()) ?? fallback;
    double parseD(TextEditingController c, double fallback) =>
        double.tryParse(c.text.trim().replaceAll(',', '.')) ?? fallback;
    _grid = _grid.copyWith(
      columns: math.max(1, parseInt(_columnsCtrl, _grid.columns)),
      rows: math.max(1, parseInt(_rowsCtrl, _grid.rows)),
      leftMarginMm: math.max(0.0, parseD(_leftCtrl, _grid.leftMarginMm)),
      topMarginMm: math.max(0.0, parseD(_topCtrl, _grid.topMarginMm)),
      hGapMm: math.max(0.0, parseD(_hGapCtrl, _grid.hGapMm)),
      vGapMm: math.max(0.0, parseD(_vGapCtrl, _grid.vGapMm)),
      barcodeUnitMm:
          parseD(_unitCtrl, _grid.barcodeUnitMm).clamp(0.15, 1.0),
    );
  }

  /// Maps the configured paper size + orientation to a `PdfPageFormat`.
  PdfPageFormat _pageFormat() {
    final PdfPageFormat base;
    switch(_grid.paperSize) {
      case PaperSize.a3:
        base = PdfPageFormat.a3;
        break;
      case PaperSize.a4:
        base = PdfPageFormat.a4;
        break;
      case PaperSize.a5:
        base = PdfPageFormat.a5;
        break;
      case PaperSize.letter:
        base = PdfPageFormat.letter;
        break;
      case PaperSize.legal:
        base = PdfPageFormat.legal;
        break;
    }
    return _grid.landscape ? base.landscape : base.portrait;
  }

  Future<void> _load() async {
    if(!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final personsFuture = ref.read(personsApiProvider).search(_search);
      // The collection only needs to be fetched once.
      final infoFuture =
          _info == null ? ref.read(collectionApiProvider).get() : null;
      final persons = await personsFuture;
      final info = infoFuture == null ? _info : await infoFuture;
      // Load the device-local layout, seeding it once from the server's.
      final layout = await _layoutStore
          .loadOrSeed(() async => info!.idCardLayout);
      if(!mounted) return;
      setState(() {
        _persons = persons;
        _info = info;
        _layout = layout;
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

  void _selectAll() {
    setState(() => _selected.addAll(_persons.map((p) => p.id)));
  }

  void _deselectAll() {
    setState(_selected.clear);
  }

  Future<void> _print(AppLocalizations l10n) async {
    final info = _info;
    if(info == null) return;
    final layout = _layout;
    if(layout == null || layout.cardWidthMm <= 0 || layout.cardHeightMm <= 0) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(l10n.idCardPrintNoLayout)),
      );
      return;
    }
    final selectedPersons =
        _persons.where((p) => _selected.contains(p.id)).toList();
    if(selectedPersons.isEmpty) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(l10n.idCardPrintNoSelection)),
      );
      return;
    }
    _applyGridFromControllers();
    await _gridStore.save(_grid);
    if(!mounted) return;
    setState(() { _printing = true; });
    try {
      final doc = await _buildPdf(l10n, info, selectedPersons);
      // Pass the configured page format so the platform print dialog adopts the
      // chosen paper size and orientation; without it the dialog defaults to
      // portrait regardless of the PDF page dimensions.
      final pageFormat = _pageFormat();
      await Printing.layoutPdf(
        format: pageFormat,
        onLayout: (_) async => doc.save(),
      );
      if(!mounted) return;
      setState(() { _printing = false; });
    } catch(e) {
      if(!mounted) return;
      setState(() { _printing = false; });
      ServerErrorDialog.show(context, e, l10n);
    }
  }

  Future<pw.Document> _buildPdf(
    AppLocalizations l10n,
    CollectionInfo info,
    List<Person> persons,
  ) async {
    final font = await PdfGoogleFonts.notoSansRegular();
    final fontBold = await PdfGoogleFonts.notoSansBold();
    final logo =
        info.logoBytes != null ? pw.MemoryImage(info.logoBytes!) : null;
    final today = DateTime.now().toIso8601String().substring(0, 10);
    final layout = _layout!;
    final theme = pw.ThemeData.withFont(base: font, bold: fontBold);

    // Print-position correction for the chosen sheet printer: offset shifts the
    // whole grid via the page margin, scale resizes each card (identity by
    // default leaves the layout unchanged).
    final cal = _grid.calibration;
    final cardW = layout.cardWidthMm * cal.scaleX * PdfPageFormat.mm;
    final cardH = layout.cardHeightMm * cal.scaleY * PdfPageFormat.mm;
    final hGap = _grid.hGapMm * PdfPageFormat.mm;
    final vGap = _grid.vGapMm * PdfPageFormat.mm;
    final columns = math.max(1, _grid.columns);
    final rowsPerPage = math.max(1, _grid.rows);
    final perPage = columns * rowsPerPage;
    final margin = pw.EdgeInsets.only(
      left: (_grid.leftMarginMm + cal.offsetXMm) * PdfPageFormat.mm,
      top: (_grid.topMarginMm + cal.offsetYMm) * PdfPageFormat.mm,
    );
    final pageFormat = _pageFormat();

    final doc = pw.Document();
    // One A4 page per chunk of columns×rows cards, laid out at the configured
    // margins and inter-label gaps.
    for(var start = 0; start < persons.length; start += perPage) {
      final pagePersons =
          persons.sublist(start, math.min(start + perPage, persons.length));
      final gridRows = <List<Person>>[];
      for(var i = 0; i < pagePersons.length; i += columns) {
        gridRows.add(pagePersons.sublist(
            i, math.min(i + columns, pagePersons.length)));
      }
      doc.addPage(
        pw.Page(
          pageFormat: pageFormat,
          margin: margin,
          theme: theme,
          build: (ctx) => pw.Column(
            crossAxisAlignment: pw.CrossAxisAlignment.start,
            mainAxisSize: pw.MainAxisSize.min,
            children: [
              for(var r = 0; r < gridRows.length; ++r) ...[
                if(r > 0) pw.SizedBox(height: vGap),
                pw.Row(
                  crossAxisAlignment: pw.CrossAxisAlignment.start,
                  children: [
                    for(var c = 0; c < gridRows[r].length; ++c) ...[
                      if(c > 0) pw.SizedBox(width: hGap),
                      _buildCard(
                          info, gridRows[r][c], cardW, cardH, today, logo),
                    ],
                  ],
                ),
              ],
            ],
          ),
        ),
      );
    }
    return doc;
  }

  pw.Widget _buildCard(
    CollectionInfo info,
    Person person,
    double cardW,
    double cardH,
    String today,
    pw.ImageProvider? logo,
  ) {
    final l10n = AppLocalizations.of(context)!;
    final layout = _layout!;
    final barcode = computePersonBarcode(info.barcodePrefix, person.id);
    final zones = <pw.Widget>[
      _placeZone(
        layout.barcode,
        _barcodeContent(barcode),
      ),
      _placeZone(
        layout.text,
        _textContent(l10n, info, person, today),
      ),
      if(layout.hasLogo && logo != null)
        _placeZone(layout.logo, pw.Image(logo, fit: pw.BoxFit.contain)),
    ];
    return pw.Container(
      width: cardW,
      height: cardH,
      decoration: _grid.drawBorder
          ? pw.BoxDecoration(border: pw.Border.all(width: 0.5))
          : null,
      child: pw.Stack(children: zones),
    );
  }

  /// The card's Code128 widget at the configured X-dimension, centred in its
  /// zone instead of stretched to the zone width — matching the desktop and
  /// the label printing path. Stretched modules leave the range scanners
  /// expect; the spare zone width stays as quiet zone. When the zone is
  /// narrower than the configured width, the enclosing zone box clamps the
  /// barcode back to the zone width.
  pw.Widget _barcodeContent(String data) {
    final modules = code128Modules(data);
    final code = pw.BarcodeWidget(
      barcode: pw.Barcode.code128(),
      data: data,
      drawText: false,
      width: modules > 0
          ? modules * _grid.barcodeUnitMm * PdfPageFormat.mm
          : null,
    );
    return modules > 0 ? pw.Center(child: code) : code;
  }

  /// Positions [content] inside the card according to [e], applying the zone
  /// rotation while keeping its on-card footprint.
  ///
  /// The content is laid out at its natural (pre-rotation) dimensions — swapped
  /// for odd quarter turns — and handed to [pw.Transform.rotateBox], the
  /// layout-aware rotation that resizes its own bounding box to the rotated
  /// content. That box equals the footprint, so it can be placed directly at
  /// the zone's top-left corner. This mirrors the layout editor's [RotatedBox]
  /// and avoids the manual centring offsets that previously pushed rotated
  /// zones (e.g. the logo) outside the card frame.
  pw.Widget _placeZone(IdCardElement e, pw.Widget content) {
    final footW = e.widthMm * PdfPageFormat.mm;
    final footH = e.heightMm * PdfPageFormat.mm;
    final odd = (e.orientation ~/ 90) % 2 != 0;
    final natW = odd ? footH : footW;
    final natH = odd ? footW : footH;
    return pw.Positioned(
      left: e.xMm * PdfPageFormat.mm,
      top: e.yMm * PdfPageFormat.mm,
      child: pw.Transform.rotateBox(
        // Negated to rotate clockwise, matching the layout editor's RotatedBox;
        // the underlying Matrix4.rotationZ otherwise turns the other way,
        // leaving 90/270 zones 180 degrees off the preview.
        angle: -e.orientation * math.pi / 180.0,
        child: pw.SizedBox(width: natW, height: natH, child: content),
      ),
    );
  }

  pw.Widget _textContent(
    AppLocalizations l10n,
    CollectionInfo info,
    Person person,
    String today,
  ) {
    final lines = <(String, bool)>[
      if(info.name.isNotEmpty) (info.name, false),
      if(info.organization.isNotEmpty) (info.organization, false),
      (l10n.idCardIssuedOn(today), false),
      (person.displayName, true),
    ];
    return pw.FittedBox(
      fit: pw.BoxFit.contain,
      alignment: pw.Alignment.centerLeft,
      child: pw.Column(
        mainAxisSize: pw.MainAxisSize.min,
        crossAxisAlignment: pw.CrossAxisAlignment.start,
        children: [
          for(final (text, bold) in lines)
            pw.Text(
              text,
              style: pw.TextStyle(
                fontWeight: bold ? pw.FontWeight.bold : pw.FontWeight.normal,
              ),
            ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/print_id.jpeg',
          title: l10n.idCardPrintTitle,
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.print),
            tooltip: l10n.idCardPrintButton,
            onPressed:
                (_printing || _selected.isEmpty) ? null : () => _print(l10n),
          ),
        ],
      ),
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 8, 12, 4),
            child: TextField(
              controller: _searchCtrl,
              decoration: InputDecoration(
                labelText: l10n.idCardPrintSearchHint,
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
                Text(l10n.idCardPrintSelectedCount(_selected.length)),
                const Spacer(),
                TextButton(
                  onPressed: _persons.isEmpty ? null : _selectAll,
                  child: Text(l10n.idCardPrintSelectAll),
                ),
                TextButton(
                  onPressed: _selected.isEmpty ? null : _deselectAll,
                  child: Text(l10n.idCardPrintDeselectAll),
                ),
              ],
            ),
          ),
          _buildArrangement(l10n),
          if(_printing || _loading) const LinearProgressIndicator(),
          const Divider(height: 1),
          Expanded(child: _buildList(l10n)),
        ],
      ),
    );
  }

  /// Collapsible editor for the label-sheet arrangement.
  Widget _buildArrangement(AppLocalizations l10n) {
    return ExpansionTile(
      title: Text(l10n.idCardPrintArrangement),
      tilePadding: const EdgeInsets.symmetric(horizontal: 12),
      childrenPadding: const EdgeInsets.fromLTRB(12, 0, 12, 12),
      children: [
        Row(
          children: [
            Expanded(
              child: DropdownButtonFormField<PaperSize>(
                initialValue: _grid.paperSize,
                decoration: InputDecoration(
                  labelText: l10n.idCardPrintPaperSize,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
                items: [
                  for(final p in PaperSize.values)
                    DropdownMenuItem(value: p, child: Text(_paperLabel(p))),
                ],
                onChanged: (p) {
                  if(p != null) {
                    setState(() => _grid = _grid.copyWith(paperSize: p));
                  }
                },
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: DropdownButtonFormField<bool>(
                initialValue: _grid.landscape,
                decoration: InputDecoration(
                  labelText: l10n.idCardPrintOrientation,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
                items: [
                  DropdownMenuItem(
                      value: false, child: Text(l10n.idCardPrintPortrait)),
                  DropdownMenuItem(
                      value: true, child: Text(l10n.idCardPrintLandscape)),
                ],
                onChanged: (v) {
                  if(v != null) {
                    setState(() => _grid = _grid.copyWith(landscape: v));
                  }
                },
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        Row(
          children: [
            Expanded(child: _numField(_columnsCtrl, l10n.idCardPrintColumns)),
            const SizedBox(width: 12),
            Expanded(child: _numField(_rowsCtrl, l10n.idCardPrintRows)),
          ],
        ),
        const SizedBox(height: 8),
        Row(
          children: [
            Expanded(child: _numField(_leftCtrl, l10n.idCardPrintMarginLeft)),
            const SizedBox(width: 12),
            Expanded(child: _numField(_topCtrl, l10n.idCardPrintMarginTop)),
          ],
        ),
        const SizedBox(height: 8),
        Row(
          children: [
            Expanded(child: _numField(_hGapCtrl, l10n.idCardPrintGapH)),
            const SizedBox(width: 12),
            Expanded(child: _numField(_vGapCtrl, l10n.idCardPrintGapV)),
          ],
        ),
        const SizedBox(height: 8),
        Row(
          children: [
            Expanded(child: _numField(_unitCtrl, l10n.labelBarcodeUnit)),
            const SizedBox(width: 12),
            const Expanded(child: SizedBox()),
          ],
        ),
        SwitchListTile(
          contentPadding: EdgeInsets.zero,
          title: Text(l10n.idCardPrintDrawBorder),
          value: _grid.drawBorder,
          onChanged: (v) =>
              setState(() => _grid = _grid.copyWith(drawBorder: v)),
        ),
        const SizedBox(height: 8),
        Align(
          alignment: Alignment.centerLeft,
          child: OutlinedButton.icon(
            onPressed: _calibrate,
            icon: const Icon(Icons.straighten),
            label: Text(l10n.calibrationButton),
          ),
        ),
      ],
    );
  }

  /// Opens the guided calibration screen for the ID-card sheet printer and
  /// stores the result in the device-local grid settings.
  Future<void> _calibrate() async {
    _applyGridFromControllers();
    await Navigator.of(context).push(MaterialPageRoute(
      builder: (_) => PrinterCalibrationScreen(
        initial: _grid.calibration,
        onSave: (cal) async {
          setState(() => _grid = _grid.copyWith(calibration: cal));
          await _gridStore.save(_grid);
        },
      ),
    ));
  }

  String _paperLabel(PaperSize p) {
    switch(p) {
      case PaperSize.a3:
        return 'A3';
      case PaperSize.a4:
        return 'A4';
      case PaperSize.a5:
        return 'A5';
      case PaperSize.letter:
        return 'Letter';
      case PaperSize.legal:
        return 'Legal';
    }
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
    );
  }

  Widget _buildList(AppLocalizations l10n) {
    if(_loading && _persons.isEmpty) {
      return const SizedBox.shrink();
    }
    if(_error != null && _persons.isEmpty) {
      return Center(
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: ErrorDisplay(error: _error!, l10n: l10n),
        ),
      );
    }
    if(_persons.isEmpty) {
      return Center(child: Text(l10n.idCardPrintNoSelection));
    }
    return ListView.separated(
      itemCount: _persons.length,
      separatorBuilder: (_, __) => const Divider(height: 1),
      itemBuilder: (ctx, i) {
        final p = _persons[i];
        return CheckboxListTile(
          value: _selected.contains(p.id),
          onChanged: (on) => _toggle(p.id, on),
          title: Text(p.displayName),
          subtitle: p.schoolClass != null && p.schoolClass!.isNotEmpty
              ? Text(p.schoolClass!)
              : null,
        );
      },
    );
  }
}

String _fmtMm(double v) =>
    v == v.roundToDouble() ? v.toStringAsFixed(0) : v.toStringAsFixed(1);
