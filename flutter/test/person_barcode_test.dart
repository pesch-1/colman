// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/isbn_lookup.dart';
import 'package:colman_client/models/id_card_layout.dart';

void main() {
  group('computePersonBarcode', () {
    test('produces barcode format 2 like the server', () {
      expect(computePersonBarcode('54861', 227), '5486100032279');
    });

    test('is never a valid ISBN/EAN', () {
      for(var id = 1; id < 100000; id += 7) {
        final barcode = computePersonBarcode('54861', id);
        expect(barcode.length, greaterThanOrEqualTo(11));
        expect(isIsbn13(barcode), isFalse, reason: barcode);
        expect(isIsbn10(barcode), isFalse, reason: barcode);
      }
    });
  });
}
