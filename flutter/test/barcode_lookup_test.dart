// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/barcode_lookup.dart';
import 'package:colman_client/api/ean_lookup.dart' show EanLookupResult;
import 'package:colman_client/api/isbn_lookup.dart' show BookLookupResult;
import 'package:colman_client/models/internet_services_settings.dart';

// All queries below use this settings object. All services are disabled so
// the tests exercise dispatch and normalisation without network access.
const InternetServicesSettings _noServices = InternetServicesSettings(
  useOpenlibrary: false,
  useGoogleapis: false,
  googleapisKey: '',
  useMusicbrainz: false,
  useBgg: false,
  bggToken: '',
  useOpengtindb: false,
  opengtindbUserid: '',
  timeoutMs: 100,
  sruServers: [],
  downloadThumbnails: false,
);

void main() {
  // -------------------------------------------------------------------------
  group('normalizeBarcode', () {
    test('strips hyphens', () {
      expect(normalizeBarcode('978-0-596-00712-3'), '9780596007123');
    });

    test('strips spaces', () {
      expect(normalizeBarcode('978 0 596 00712 3'), '9780596007123');
    });

    test('uppercases x to X', () {
      expect(normalizeBarcode('047-0-12345-x'), '047012345X');
    });

    test('removes other non-digit non-X characters', () {
      expect(normalizeBarcode('isbn: 978.0.596.00712.3'), '9780596007123');
    });

    test('pure digit string passes through unchanged', () {
      expect(normalizeBarcode('4002051683320'), '4002051683320');
    });

    test('empty string returns empty string', () {
      expect(normalizeBarcode(''), '');
    });
  });

  // -------------------------------------------------------------------------
  group('queryBarcode — dispatch', () {
    test('ISBN-13 returns BookSearchResult', () async {
      final r = await queryBarcode('9780596007126', _noServices);
      expect(r, isA<BookSearchResult>());
    });

    test('ISBN-10 returns BookSearchResult', () async {
      final r = await queryBarcode('0596007124', _noServices);
      expect(r, isA<BookSearchResult>());
    });

    test('EAN-13 returns AvSearchResult', () async {
      final r = await queryBarcode('4002051683320', _noServices);
      expect(r, isA<AvSearchResult>());
    });

    test('UPC-A (12 digits) returns AvSearchResult', () async {
      final r = await queryBarcode('012345678905', _noServices);
      expect(r, isA<AvSearchResult>());
    });

    test('too short returns NoSearchResult', () async {
      final r = await queryBarcode('12345', _noServices);
      expect(r, isA<NoSearchResult>());
    });

    test('all-letters input returns NoSearchResult', () async {
      final r = await queryBarcode('ABCDEFG', _noServices);
      expect(r, isA<NoSearchResult>());
    });

    test('empty string returns NoSearchResult', () async {
      final r = await queryBarcode('', _noServices);
      expect(r, isA<NoSearchResult>());
    });

    test('13 digits with wrong check digit returns NoSearchResult', () async {
      // 4002051683321: last digit wrong (should be 0)
      final r = await queryBarcode('4002051683321', _noServices);
      expect(r, isA<NoSearchResult>());
    });
  });

  // -------------------------------------------------------------------------
  group('queryBarcode — barcode normalisation before dispatch', () {
    test('ISBN-13 with hyphens returns BookSearchResult', () async {
      final r = await queryBarcode('978-0-596-00712-6', _noServices);
      expect(r, isA<BookSearchResult>());
    });

    test('ISBN-13 with spaces returns BookSearchResult', () async {
      final r = await queryBarcode('978 0 596 00712 6', _noServices);
      expect(r, isA<BookSearchResult>());
    });

    test('EAN-13 with spaces returns AvSearchResult', () async {
      final r = await queryBarcode('4 002051 683320', _noServices);
      expect(r, isA<AvSearchResult>());
    });
  });

  // -------------------------------------------------------------------------
  group('queryBarcode — ISBN-10 to ISBN-13 conversion', () {
    test('ISBN-10 result is BookSearchResult', () async {
      // ISBN-10: 0596007124 → ISBN-13: 9780596007126
      final r = await queryBarcode('0596007124', _noServices);
      expect(r, isA<BookSearchResult>());
    });

    test('ISBN-10 with X check digit dispatches correctly', () async {
      // 0-201-63361-2 is a valid ISBN-10 without X;
      // Use 0-8044-2957-X which ends in X.
      final r = await queryBarcode('080442957X', _noServices);
      expect(r, isA<BookSearchResult>());
    });

    test('ISBN-10 with lowercase x is accepted', () async {
      final r = await queryBarcode('080442957x', _noServices);
      expect(r, isA<BookSearchResult>());
    });
  });

  // -------------------------------------------------------------------------
  group('queryBarcode — UPC-A to EAN-13 padding', () {
    test('12-digit UPC-A dispatches to EAN query', () async {
      final r = await queryBarcode('012345678905', _noServices);
      expect(r, isA<AvSearchResult>());
    });
  });

  // -------------------------------------------------------------------------
  group('BarcodeSearchResult — data access', () {
    test('BookSearchResult.data is a BookLookupResult', () {
      final book = BookLookupResult()..name = 'Test Book';
      final r = BookSearchResult(book);
      expect(r.data, isA<BookLookupResult>());
      expect(r.data.name, 'Test Book');
    });

    test('AvSearchResult.data is an EanLookupResult', () {
      final av = EanLookupResult()..name = 'Test CD';
      final r = AvSearchResult(av);
      expect(r.data, isA<EanLookupResult>());
      expect(r.data.name, 'Test CD');
    });

    test('BookSearchResult with empty data is still BookSearchResult', () async {
      final r = await queryBarcode('9780596007126', _noServices);
      final book = r as BookSearchResult;
      expect(book.data.isEmpty, isTrue);
    });

    test('AvSearchResult with empty data is still AvSearchResult', () async {
      final r = await queryBarcode('4002051683320', _noServices);
      final av = r as AvSearchResult;
      expect(av.data.isEmpty, isTrue);
    });
  });

  // -------------------------------------------------------------------------
  group('BarcodeSearchResult — pattern matching', () {
    test('sealed class covers all cases exhaustively', () async {
      Future<String> describe(String barcode) async {
        final r = await queryBarcode(barcode, _noServices);
        return switch(r) {
          BookSearchResult() => 'book',
          AvSearchResult()   => 'av',
          NoSearchResult()   => 'none',
        };
      }

      expect(await describe('9780596007126'), 'book');
      expect(await describe('4002051683320'), 'av');
      expect(await describe('12345'),         'none');
    });
  });
}
