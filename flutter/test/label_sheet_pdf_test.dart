// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/models/label_preset.dart';
import 'package:colman_client/printing/label_sheet_pdf.dart';

void main() {
  // A 1x1 transparent PNG.
  final png = Uint8List.fromList([
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, //
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
    0x89, 0x00, 0x00, 0x00, 0x0A, 0x49, 0x44, 0x41,
    0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
    0x42, 0x60, 0x82,
  ]);

  const sheet = LabelPreset(
    name: 'Sheet',
    purpose: LabelPurpose.bookLabel,
    printerType: PrinterType.systemSheet,
    columns: 2,
    rows: 5,
  );

  test('buildLabelSheetPdf produces a valid PDF', () async {
    final bytes = await buildLabelSheetPdf(
        labelPngs: List.filled(3, png), preset: sheet);
    expect(String.fromCharCodes(bytes.sublist(0, 5)), '%PDF-');
  });

  test('overflow spills onto a second page', () async {
    // 2x5 = 10 per page; 11 labels need two pages.
    final two = await buildLabelSheetPdf(
        labelPngs: List.filled(11, png), preset: sheet);
    final one = await buildLabelSheetPdf(
        labelPngs: List.filled(10, png), preset: sheet);
    expect(two.length, greaterThan(one.length));
  });
}
