// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';

import 'package:pdf/pdf.dart';
import 'package:pdf/widgets.dart' as pw;

import '../models/printer_calibration.dart';

/// Half-length of each reference cross arm, in millimetres.
const double _crossArmMm = 10.0;

/// Builds an A4 calibration sheet with three reference crosses at the nominal
/// positions A(50,50), B(150,50), C(50,150) mm from the top-left corner.
///
/// The user prints this at 100 % (no fit-to-page), measures the actual cross
/// centres with a ruler and feeds the four values into
/// [PrinterCalibration.fromMeasurements]. The sheet mirrors the desktop
/// calibration sheet so the same measuring procedure applies.
Future<Uint8List> buildCalibrationSheetPdf() {
  final doc = pw.Document();
  doc.addPage(
    pw.Page(
      pageFormat: PdfPageFormat.a4,
      margin: pw.EdgeInsets.zero,
      build: (_) => pw.Stack(
        children: [
          ..._cross('A', PrinterCalibration.refXMm, PrinterCalibration.refYMm),
          ..._cross('B', PrinterCalibration.refXMm + PrinterCalibration.spanXMm,
              PrinterCalibration.refYMm),
          ..._cross('C', PrinterCalibration.refXMm,
              PrinterCalibration.refYMm + PrinterCalibration.spanYMm),
        ],
      ),
    ),
  );
  return doc.save();
}

/// Positioned widgets drawing a single labelled cross centred on ([xMm], [yMm])
/// (millimetres from the corner).
List<pw.Widget> _cross(String label, double xMm, double yMm) {
  final x = xMm * PdfPageFormat.mm;
  final y = yMm * PdfPageFormat.mm;
  final arm = _crossArmMm * PdfPageFormat.mm;
  final stroke = 0.3 * PdfPageFormat.mm;
  return [
    // Horizontal arm.
    pw.Positioned(
      left: x - arm,
      top: y - stroke / 2,
      child:
          pw.Container(width: arm * 2, height: stroke, color: PdfColors.black),
    ),
    // Vertical arm.
    pw.Positioned(
      left: x - stroke / 2,
      top: y - arm,
      child:
          pw.Container(width: stroke, height: arm * 2, color: PdfColors.black),
    ),
    // Label just above-right of the cross.
    pw.Positioned(
      left: x + 2 * PdfPageFormat.mm,
      top: y - arm - 4 * PdfPageFormat.mm,
      child: pw.Text(label,
          style: pw.TextStyle(fontSize: 10, fontWeight: pw.FontWeight.bold)),
    ),
  ];
}
