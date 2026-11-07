// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/models/printer_calibration.dart';

void main() {
  group('PrinterCalibration', () {
    test('exact measurements yield the identity transform', () {
      final cal = PrinterCalibration.fromMeasurements(
        aXMm: PrinterCalibration.refXMm,
        aYMm: PrinterCalibration.refYMm,
        bXMm: PrinterCalibration.refXMm + PrinterCalibration.spanXMm,
        cYMm: PrinterCalibration.refYMm + PrinterCalibration.spanYMm,
      );
      expect(cal.offsetXMm, closeTo(0.0, 1e-9));
      expect(cal.offsetYMm, closeTo(0.0, 1e-9));
      expect(cal.scaleX, closeTo(1.0, 1e-9));
      expect(cal.scaleY, closeTo(1.0, 1e-9));
    });

    test('pure translation is inverted', () {
      // Everything printed 5 mm too far right and 3 mm too low.
      final cal = PrinterCalibration.fromMeasurements(
        aXMm: PrinterCalibration.refXMm + 5,
        aYMm: PrinterCalibration.refYMm + 3,
        bXMm: PrinterCalibration.refXMm + PrinterCalibration.spanXMm + 5,
        cYMm: PrinterCalibration.refYMm + PrinterCalibration.spanYMm + 3,
      );
      // Unit scale (span unchanged), offset compensates the shift.
      expect(cal.scaleX, closeTo(1.0, 1e-9));
      expect(cal.scaleY, closeTo(1.0, 1e-9));
      expect(cal.offsetXMm, closeTo(-5.0, 1e-9));
      expect(cal.offsetYMm, closeTo(-3.0, 1e-9));
    });

    test('correcting a scale+shift error round-trips to the nominal positions',
        () {
      // Forward model of the printer error: printed = scaleP * nominal + transP
      // (X enlarged 10 % and shifted, Y shrunk 5 % and shifted).
      const scalePx = 1.1, transPx = -3.0;
      const scalePy = 0.95, transPy = 2.0;
      double printedX(double n) => scalePx * n + transPx;
      double printedY(double n) => scalePy * n + transPy;

      final cal = PrinterCalibration.fromMeasurements(
        aXMm: printedX(PrinterCalibration.refXMm),
        aYMm: printedY(PrinterCalibration.refYMm),
        bXMm: printedX(PrinterCalibration.refXMm + PrinterCalibration.spanXMm),
        cYMm: printedY(PrinterCalibration.refYMm + PrinterCalibration.spanYMm),
      );
      expect(cal.scaleX, closeTo(1 / scalePx, 1e-9));
      expect(cal.scaleY, closeTo(1 / scalePy, 1e-9));

      // Applying the correction and then the printer error must reproduce the
      // nominal coordinate for every point.
      for (final x in [
        PrinterCalibration.refXMm,
        PrinterCalibration.refXMm + PrinterCalibration.spanXMm,
      ]) {
        expect(printedX(cal.scaleX * x + cal.offsetXMm), closeTo(x, 1e-6));
      }
      for (final y in [
        PrinterCalibration.refYMm,
        PrinterCalibration.refYMm + PrinterCalibration.spanYMm,
      ]) {
        expect(printedY(cal.scaleY * y + cal.offsetYMm), closeTo(y, 1e-6));
      }
    });

    test('degenerate span falls back to identity on that axis', () {
      final cal = PrinterCalibration.fromMeasurements(
        aXMm: PrinterCalibration.refXMm,
        aYMm: PrinterCalibration.refYMm,
        bXMm: PrinterCalibration.refXMm, // zero X span
        cYMm: PrinterCalibration.refYMm + PrinterCalibration.spanYMm,
      );
      expect(cal.scaleX, 1.0);
      expect(cal.offsetXMm, 0.0);
    });

    test('JSON round-trips', () {
      const cal = PrinterCalibration(
          offsetXMm: 2.5, offsetYMm: -1.0, scaleX: 0.98, scaleY: 1.02);
      final back = PrinterCalibration.fromJson(cal.toJson());
      expect(back.offsetXMm, 2.5);
      expect(back.offsetYMm, -1.0);
      expect(back.scaleX, 0.98);
      expect(back.scaleY, 1.02);
    });
  });
}
