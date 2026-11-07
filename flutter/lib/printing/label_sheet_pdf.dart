// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';

import 'package:pdf/pdf.dart';
import 'package:pdf/widgets.dart' as pw;

import '../models/label_preset.dart';

/// Maps a preset [PaperSize] (+ landscape flag) to a `PdfPageFormat`.
PdfPageFormat sheetPageFormat(LabelPreset preset) {
  final PdfPageFormat base;
  switch (preset.paperSize) {
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
  return preset.landscape ? base.landscape : base.portrait;
}

/// Builds a print-ready PDF that tiles the rendered label PNGs onto sheets at
/// their **real size** (columns × rows with the preset's margins and gaps), so
/// an ordinary/network printer prints them 1:1 instead of scaling one label to
/// fill the page.
///
/// [cellOrder] lists the label-cell indices (row-major, 0-based) to fill on the
/// first sheet — letting the user skip already-used positions on a partly used
/// sheet. Overflow continues on fresh, fully populated sheets. When null, the
/// first sheet is filled in natural order.
Future<Uint8List> buildLabelSheetPdf({
  required List<Uint8List> labelPngs,
  required LabelPreset preset,
  List<int>? cellOrder,
}) {
  final pageFormat = sheetPageFormat(preset);
  final columns = preset.columns < 1 ? 1 : preset.columns;
  final rows = preset.rows < 1 ? 1 : preset.rows;
  final perPage = columns * rows;
  // Print-position correction for the chosen sheet printer: the offset shifts
  // every label by the same physical amount on the page, the scale resizes each
  // label (identity by default leaves the layout unchanged).
  final cal = preset.calibration;
  final labelW = preset.labelWidthMm * cal.scaleX * PdfPageFormat.mm;
  final labelH = preset.labelHeightMm * cal.scaleY * PdfPageFormat.mm;
  final hStep = (preset.labelWidthMm + preset.hGapMm) * PdfPageFormat.mm;
  final vStep = (preset.labelHeightMm + preset.vGapMm) * PdfPageFormat.mm;
  final left = (preset.leftMarginMm + cal.offsetXMm) * PdfPageFormat.mm;
  final top = (preset.topMarginMm + cal.offsetYMm) * PdfPageFormat.mm;

  // Assign labels to (page, cell) slots.
  final firstCells = (cellOrder == null || cellOrder.isEmpty)
      ? List<int>.generate(perPage, (i) => i)
      : cellOrder.where((c) => c >= 0 && c < perPage).toList();
  final pages = <List<MapEntry<int, Uint8List>>>[];
  var labelIdx = 0;
  final firstPage = <MapEntry<int, Uint8List>>[];
  for (final cell in firstCells) {
    if (labelIdx >= labelPngs.length) break;
    firstPage.add(MapEntry(cell, labelPngs[labelIdx++]));
  }
  pages.add(firstPage);
  while (labelIdx < labelPngs.length) {
    final page = <MapEntry<int, Uint8List>>[];
    for (var c = 0; c < perPage && labelIdx < labelPngs.length; c++) {
      page.add(MapEntry(c, labelPngs[labelIdx++]));
    }
    pages.add(page);
  }

  final doc = pw.Document();
  for (final page in pages) {
    if (page.isEmpty) continue;
    doc.addPage(pw.Page(
      pageFormat: pageFormat,
      margin: pw.EdgeInsets.zero,
      build: (_) => pw.Stack(
        children: [
          for (final slot in page)
            pw.Positioned(
              left: left + (slot.key % columns) * hStep,
              top: top + (slot.key ~/ columns) * vStep,
              child: pw.SizedBox(
                width: labelW,
                height: labelH,
                child: pw.Image(pw.MemoryImage(slot.value), fit: pw.BoxFit.fill),
              ),
            ),
        ],
      ),
    ));
  }
  return doc.save();
}
