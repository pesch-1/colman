// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/models/id_card_layout.dart';
import 'package:colman_client/models/label_preset.dart';
import 'package:colman_client/models/printer_calibration.dart';
import 'package:colman_client/models/signature_layout.dart';

void main() {
  group('LabelPreset', () {
    test('JSON round-trips all fields', () {
      const preset = LabelPreset(
        name: 'Sheet book label',
        purpose: LabelPurpose.bookLabel,
        printerType: PrinterType.systemSheet,
        barWidthReductionMm: 0.1,
        barcodeUnitMm: 0.5,
        dpi: 300,
        darkness: 12,
        xOffsetMm: 9,
        gapMm: 3,
        columns: 3,
        rows: 8,
        leftMarginMm: 6,
        topMarginMm: 11,
        hGapMm: 2,
        vGapMm: 1,
        paperSize: PaperSize.letter,
        landscape: true,
        calibration: PrinterCalibration(
            offsetXMm: 1.5, offsetYMm: -2.0, scaleX: 1.01, scaleY: 0.99),
      );
      final back = LabelPreset.fromJson(preset.toJson());
      expect(back.name, preset.name);
      expect(back.purpose, LabelPurpose.bookLabel);
      expect(back.printerType, PrinterType.systemSheet);
      expect(back.barWidthReductionMm, 0.1);
      expect(back.barcodeUnitMm, 0.5);
      expect(back.dpi, 300);
      expect(back.darkness, 12);
      expect(back.columns, 3);
      expect(back.rows, 8);
      expect(back.paperSize, PaperSize.letter);
      expect(back.landscape, true);
      expect(back.calibration.offsetXMm, 1.5);
      expect(back.calibration.offsetYMm, -2.0);
      expect(back.calibration.scaleX, 1.01);
      expect(back.calibration.scaleY, 0.99);
    });

    test('label size comes from the active arrangement', () {
      const book = LabelPreset(
        name: 'b',
        purpose: LabelPurpose.bookLabel,
        printerType: PrinterType.thermalUsb,
        cardLayout: IdCardLayout(cardWidthMm: 70, cardHeightMm: 40),
      );
      expect(book.labelWidthMm, 70);
      expect(book.labelHeightMm, 40);

      const sig = LabelPreset(
        name: 's',
        purpose: LabelPurpose.signature,
        printerType: PrinterType.thermalUsb,
        signatureLayout: SignatureLayout(cardWidthMm: 50, cardHeightMm: 18),
      );
      expect(sig.labelWidthMm, 50);
      expect(sig.labelHeightMm, 18);
    });

    test('barcode unit is clamped on load and defaults to 0.33', () {
      final wide = LabelPreset.fromJson({
        'name': 'x',
        'purpose': 'bookLabel',
        'printer_type': 'thermalUsb',
        'barcode_unit_mm': 5.0,
      });
      expect(wide.barcodeUnitMm, 1.0);
      final missing = LabelPreset.fromJson({
        'name': 'x',
        'purpose': 'bookLabel',
        'printer_type': 'thermalUsb',
      });
      expect(missing.barcodeUnitMm, 0.33);
    });

    test('darkness is clamped on load', () {
      final p = LabelPreset.fromJson({
        'name': 'x',
        'purpose': 'bookLabel',
        'printer_type': 'thermalUsb',
        'darkness': 99,
      });
      expect(p.darkness, LabelPreset.maxDarkness);
    });

    test('defaults provide presets per purpose', () {
      final names = LabelPresetStore.defaults.map((p) => p.purpose).toSet();
      expect(names.contains(LabelPurpose.bookLabel), true);
      expect(names.contains(LabelPurpose.signature), true);
    });
  });
}
