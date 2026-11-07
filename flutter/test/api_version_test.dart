// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/api_version.dart';

void main() {
  group('ServerApiVersion.fromJson', () {
    test('parses major, minor and name', () {
      final v = ServerApiVersion.fromJson(
          {'major': 1, 'minor': 18, 'name': 'School library'});
      expect(v.major, 1);
      expect(v.minor, 18);
      expect(v.name, 'School library');
      expect(v.label, '1.18');
    });

    test('tolerates missing name and fields', () {
      final v = ServerApiVersion.fromJson({'major': 1});
      expect(v.major, 1);
      expect(v.minor, 0);
      expect(v.name, '');
    });
  });

  group('ServerApiVersion.isCompatible', () {
    test('same major and equal minor is compatible', () {
      final v = ServerApiVersion(major: clientApiMajor, minor: clientApiMinor);
      expect(v.isCompatible, isTrue);
    });

    test('same major and newer minor is compatible', () {
      final v =
          ServerApiVersion(major: clientApiMajor, minor: clientApiMinor + 5);
      expect(v.isCompatible, isTrue);
    });

    test('same major but older minor is incompatible', () {
      final v =
          ServerApiVersion(major: clientApiMajor, minor: clientApiMinor - 1);
      expect(v.isCompatible, isFalse);
    });

    test('different major is incompatible even with a higher minor', () {
      final v = ServerApiVersion(
          major: clientApiMajor + 1, minor: clientApiMinor + 99);
      expect(v.isCompatible, isFalse);
    });
  });
}
