// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/models/id_card_layout.dart';
import 'package:colman_client/printing/label_bitmap.dart';

void main() {
  Uint8List tspl(List<MonoBitmap> b) => labelsToTsplBitmap(b,
      dpi: 203, gapMm: 4, darkness: 8, xOffsetMm: 11);

  group('labelsToTsplBitmap', () {
    test('wraps each bitmap in a CLS/BITMAP/PRINT block', () {
      final bmp = MonoBitmap(16, 8, Uint8List(2 * 8)..fillRange(0, 16, 0xFF));
      final out = latin1.decode(tspl([bmp, bmp]));
      expect('SIZE'.allMatches(out).length, 1);
      expect('DIRECTION 0'.allMatches(out).length, 1);
      expect('REFERENCE'.allMatches(out).length, 1);
      expect('CLS'.allMatches(out).length, 2);
      expect('BITMAP'.allMatches(out).length, 2);
      expect('PRINT 1'.allMatches(out).length, 2);
      expect(out, contains('DENSITY 8'));
    });

    test('uses the bitmap label size for SIZE', () {
      final bmp = MonoBitmap(599, 400, Uint8List(75 * 400),
          widthMm: 60, heightMm: 40);
      final out = latin1.decode(tspl([bmp]));
      expect(out, contains('SIZE 60 mm,40 mm'));
    });

    test('applies the per-model placement profile', () {
      final bmp = MonoBitmap(16, 8, Uint8List(2 * 8)..fillRange(0, 16, 0xFF));

      // Vevor Y486: shifted origin, bitmap indented 11 mm (≈ 88 dots at
      // 203 dpi with the neutral 11 mm x-offset), and the darkness set via
      // the EPL2 density register — the firmware ignores TSPL DENSITY.
      final y486 = latin1.decode(labelsToTsplBitmap([bmp],
          dpi: 203, gapMm: 4, darkness: 8, xOffsetMm: 11,
          profile: TsplProfile.forDevice(0x5958, 0x0486)));
      expect(y486, startsWith('\nN\nD8\n'));
      expect(y486, contains('REFERENCE -64,0'));
      expect(y486, contains('BITMAP 88,0,'));

      // Munbyn RW403B (and unknown models): standard TSPL origin, no EPL2
      // prefix (the RW403B discards jobs preceded by foreign commands).
      final rw403b = latin1.decode(labelsToTsplBitmap([bmp],
          dpi: 203, gapMm: 4, darkness: 8, xOffsetMm: 11,
          profile: TsplProfile.forDevice(0x0d28, 0xccdd)));
      expect(rw403b, startsWith('SIZE '));
      expect(rw403b, contains('REFERENCE 0,0'));
      expect(rw403b, contains('BITMAP 0,0,'));
    });

    test('draws the bitmap in mode 1 (OR)', () {
      final bmp = MonoBitmap(16, 8, Uint8List(2 * 8)..fillRange(0, 16, 0xFF));
      final out = latin1.decode(tspl([bmp]));
      expect(out, contains(',${bmp.widthBytes},${bmp.heightDots},1,'));
    });
  });

  group('code128Modules', () {
    test('returns the symbol module count', () {
      // A Code128 symbol is 11 modules per character plus the 13-module stop
      // pattern, so the total is always ≡ 2 (mod 11).
      expect(code128Modules('1234567') % 11, 2);
      expect(code128Modules('X0001234') % 11, 2);
      expect(code128Modules('X0001234'),
          greaterThan(code128Modules('1234567')));
      expect(code128Modules(''), 0);
    });
  });

  group('renderBookLabel', () {
    test('produces a 1-bit bitmap of the expected byte size', () async {
      const layout = IdCardLayout(
        cardWidthMm: 75,
        cardHeightMm: 50,
        hasLogo: false,
        barcode: IdCardElement(xMm: 3, yMm: 24, widthMm: 50, heightMm: 20),
        text: IdCardElement(xMm: 3, yMm: 3, widthMm: 69, heightMm: 18),
        logo: IdCardElement(),
      );
      final bmp = await renderBookLabel(
        layout: layout,
        barcode: '1234567',
        textLines: const [LabelTextLine('Title')],
        dpi: 203,
      );
      expect(bmp.widthDots, 599);
      expect(bmp.heightDots, 400);
      expect(bmp.widthBytes, (599 + 7) ~/ 8);
      expect(bmp.data.length, bmp.widthBytes * bmp.heightDots);
    });

    // Renders a barcode-only label and measures the narrowest black run of a
    // row through the bars (well below the zone top, above the human-readable
    // text): one module, expected at the X-dimension snapped to whole dots.
    Future<int> minBarRun({double? unitMm}) async {
      // Barcode-only layout; the zone is far wider than the symbol needs, so
      // a stretched render would produce much wider modules.
      const layout = IdCardLayout(
        cardWidthMm: 75,
        cardHeightMm: 50,
        hasLogo: false,
        barcode: IdCardElement(xMm: 3, yMm: 3, widthMm: 69, heightMm: 30),
        text: IdCardElement(),
        logo: IdCardElement(),
      );
      final bmp = await renderBookLabel(
        layout: layout,
        barcode: '1234567',
        textLines: const [],
        dpi: 203,
        rotate180: false,
        barcodeUnitMm: unitMm ?? defaultBarcodeUnitMm,
      );
      final dotsPerMm = 203 / 25.4;
      final y = ((3 + 5) * dotsPerMm).round();
      bool black(int x) =>
          (bmp.data[y * bmp.widthBytes + (x >> 3)] & (0x80 >> (x & 7))) == 0;
      var minRun = bmp.widthDots;
      var run = 0;
      for (var x = 0; x < bmp.widthDots; x++) {
        if (black(x)) {
          run++;
        } else if (run > 0) {
          if (run < minRun) minRun = run;
          run = 0;
        }
      }
      return minRun;
    }

    test('draws barcode modules at the default X-dimension', () async {
      // 0.33 mm snapped to whole dots is 3 dots at 203 dpi.
      expect(await minBarRun(), 3);
    });

    test('honours a configured X-dimension', () async {
      // 0.5 mm snapped to whole dots is 4 dots at 203 dpi.
      expect(await minBarRun(unitMm: 0.5), 4);
    });

    test('renderBookLabelPng returns a PNG image', () async {
      const layout = IdCardLayout(
        cardWidthMm: 75,
        cardHeightMm: 50,
        hasLogo: false,
        barcode: IdCardElement(xMm: 3, yMm: 24, widthMm: 50, heightMm: 20),
        text: IdCardElement(xMm: 3, yMm: 3, widthMm: 69, heightMm: 18),
        logo: IdCardElement(),
      );
      final png = await renderBookLabelPng(
        layout: layout,
        barcode: '1234567',
        textLines: const [LabelTextLine('Title'), LabelTextLine('Author')],
        dpi: 203,
      );
      // PNG magic number.
      expect(png.sublist(0, 4), [0x89, 0x50, 0x4E, 0x47]);
    });
  });
}
