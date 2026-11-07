// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/isbn_lookup.dart';
import 'package:colman_client/models/internet_services_settings.dart';

// ---------------------------------------------------------------------------
// Static test fixtures
// ---------------------------------------------------------------------------

// Minimal MARC21-xml SRU response (DNB-style namespace).
const String _marc21XmlSample = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim" type="Bibliographic">
          <datafield tag="020" ind1=" " ind2=" ">
            <subfield code="a">9780596007126</subfield>
          </datafield>
          <datafield tag="082" ind1="0" ind2=" ">
            <subfield code="a">005.133</subfield>
          </datafield>
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Programming Python</subfield>
            <subfield code="c">Mark Lutz</subfield>
          </datafield>
          <datafield tag="264" ind1=" " ind2="1">
            <subfield code="b">O'Reilly</subfield>
            <subfield code="c">2006</subfield>
          </datafield>
          <datafield tag="300" ind1=" " ind2=" ">
            <subfield code="a">1564 Seiten</subfield>
          </datafield>
          <datafield tag="520" ind1=" " ind2=" ">
            <subfield code="a">A comprehensive guide to Python programming.</subfield>
          </datafield>
          <datafield tag="655" ind1=" " ind2="7">
            <subfield code="a">Programmiersprache</subfield>
          </datafield>
          <datafield tag="655" ind1=" " ind2="7">
            <subfield code="a">Python</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// MARC21 XML with a 260 field instead of 264 (older record style).
const String _marc21XmlWith260 = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records><record><recordData>
    <record xmlns="http://www.loc.gov/MARC21/slim">
      <datafield tag="245" ind1="1" ind2="0">
        <subfield code="a">Old Style Book</subfield>
      </datafield>
      <datafield tag="260" ind1=" " ind2=" ">
        <subfield code="b">Old Publisher</subfield>
        <subfield code="c">1999</subfield>
      </datafield>
    </record>
  </recordData></record></records>
</searchRetrieveResponse>
''';

// Minimal UNIMARC-xml SRU response (BnF-style).
const String _unimarcXmlSample = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="010" ind1=" " ind2=" ">
            <subfield code="a">978-0-596-00712-6</subfield>
          </datafield>
          <datafield tag="200" ind1="1" ind2=" ">
            <subfield code="a">Programming Python</subfield>
            <subfield code="f">Mark Lutz</subfield>
          </datafield>
          <datafield tag="210" ind1=" " ind2=" ">
            <subfield code="c">O'Reilly</subfield>
            <subfield code="d">2006</subfield>
          </datafield>
          <datafield tag="215" ind1=" " ind2=" ">
            <subfield code="a">1564 p.</subfield>
          </datafield>
          <datafield tag="330" ind1=" " ind2=" ">
            <subfield code="a">A comprehensive guide.</subfield>
          </datafield>
          <datafield tag="606" ind1=" " ind2=" ">
            <subfield code="a">Python</subfield>
          </datafield>
          <datafield tag="700" ind1=" " ind2=" ">
            <subfield code="a">Lutz</subfield>
            <subfield code="b">Mark</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// Minimal Open Library API response (format=json&jscmd=data).
const Map<String, dynamic> _openlibraryJson = {
  'ISBN:9780596007126': {
    'title': 'Programming Python',
    'authors': [
      {'name': 'Mark Lutz'},
    ],
    'publishers': [
      {'name': "O'Reilly"},
    ],
    'publish_date': 'August 2006',
    'number_of_pages': 1564,
    'cover': {'medium': 'https://covers.openlibrary.org/b/id/123-M.jpg'},
    'identifiers': {
      'isbn_10': ['0596007124'],
      'isbn_13': ['9780596007126'],
    },
    'dewey_decimal_class': ['005.133'],
    'description': 'A comprehensive Python guide.',
    'genres': ['Programming', 'Python'],
  },
};

// Open Library response where description is an object instead of a string.
const Map<String, dynamic> _openlibraryJsonObjectDesc = {
  'ISBN:0596007124': {
    'title': 'Some Book',
    'description': {'type': '/type/text', 'value': 'Object description.'},
  },
};

// Minimal Google Books API response.
const Map<String, dynamic> _googlebooksJson = {
  'items': [
    {
      'volumeInfo': {
        'title': 'Programming Python',
        'subtitle': 'Powerful Object-Oriented Programming',
        'authors': ['Mark Lutz'],
        'publisher': "O'Reilly",
        'publishedDate': '2006-08-01',
        'pageCount': 1564,
        'categories': ['Computers'],
        'description': 'A comprehensive Python guide.',
        'industryIdentifiers': [
          {'type': 'ISBN_10', 'identifier': '0596007124'},
          {'type': 'ISBN_13', 'identifier': '9780596007126'},
        ],
        'imageLinks': {
          'thumbnail': 'https://books.google.com/thumbnail.jpg',
        },
      },
    },
  ],
};

void main() {
  // -------------------------------------------------------------------------
  // isIsbn13
  // -------------------------------------------------------------------------
  group('isIsbn13', () {
    test('valid 978 prefix', () {
      expect(isIsbn13('9780596007126'), isTrue);
    });

    test('valid 979 prefix', () {
      expect(isIsbn13('9791032349014'), isTrue);
    });

    test('wrong check digit', () {
      expect(isIsbn13('9780596007125'), isFalse);
    });

    test('too short', () {
      expect(isIsbn13('978059600712'), isFalse);
    });

    test('too long', () {
      expect(isIsbn13('97805960071261'), isFalse);
    });

    test('empty', () {
      expect(isIsbn13(''), isFalse);
    });

    test('non-digit character', () {
      expect(isIsbn13('978059600712X'), isFalse);
    });
  });

  // -------------------------------------------------------------------------
  // isIsbn10
  // -------------------------------------------------------------------------
  group('isIsbn10', () {
    test('valid with digit check', () {
      // "Programming Python" 3rd ed.
      expect(isIsbn10('0596007124'), isTrue);
    });

    test('valid with X check digit', () {
      // "The Pragmatic Programmer"
      expect(isIsbn10('020161622X'), isTrue);
    });

    test('wrong check digit', () {
      expect(isIsbn10('0596007123'), isFalse);
    });

    test('too short', () {
      expect(isIsbn10('059600712'), isFalse);
    });

    test('too long', () {
      expect(isIsbn10('05960071241'), isFalse);
    });

    test('empty', () {
      expect(isIsbn10(''), isFalse);
    });

    test('non-digit non-X character', () {
      expect(isIsbn10('059600712A'), isFalse);
    });

    test('X in non-final position', () {
      expect(isIsbn10('059X007124'), isFalse);
    });
  });

  // -------------------------------------------------------------------------
  // isbn10ToIsbn13
  // -------------------------------------------------------------------------
  group('isbn10ToIsbn13', () {
    test('digit check digit', () {
      expect(isbn10ToIsbn13('0596007124'), equals('9780596007126'));
    });

    test('X check digit', () {
      expect(isbn10ToIsbn13('020161622X'), equals('9780201616224'));
    });

    test('result is a valid ISBN-13', () {
      expect(isIsbn13(isbn10ToIsbn13('0596007124')), isTrue);
      expect(isIsbn13(isbn10ToIsbn13('020161622X')), isTrue);
    });
  });

  // -------------------------------------------------------------------------
  // isbn13ToIsbn10
  // -------------------------------------------------------------------------
  group('isbn13ToIsbn10', () {
    test('digit check digit', () {
      expect(isbn13ToIsbn10('9780596007126'), equals('0596007124'));
    });

    test('X check digit', () {
      expect(isbn13ToIsbn10('9780201616224'), equals('020161622X'));
    });

    test('979 prefix returns empty string', () {
      expect(isbn13ToIsbn10('9791032349014'), isEmpty);
    });

    test('result is a valid ISBN-10', () {
      expect(isIsbn10(isbn13ToIsbn10('9780596007126')), isTrue);
      expect(isIsbn10(isbn13ToIsbn10('9780201616224')), isTrue);
    });
  });

  // -------------------------------------------------------------------------
  // Round-trips
  // -------------------------------------------------------------------------
  group('ISBN round-trip', () {
    test('isbn10 → isbn13 → isbn10', () {
      const isbn10 = '0596007124';
      expect(isbn13ToIsbn10(isbn10ToIsbn13(isbn10)), equals(isbn10));
    });

    test('isbn10 (X) → isbn13 → isbn10', () {
      const isbn10 = '020161622X';
      expect(isbn13ToIsbn10(isbn10ToIsbn13(isbn10)), equals(isbn10));
    });

    test('isbn13 → isbn10 → isbn13', () {
      const isbn13 = '9780596007126';
      expect(isbn10ToIsbn13(isbn13ToIsbn10(isbn13)), equals(isbn13));
    });
  });

  // -------------------------------------------------------------------------
  // normalizeDate
  // -------------------------------------------------------------------------
  group('normalizeDate', () {
    test('ISO year only', () {
      expect(normalizeDate('2006'), equals('2006'));
    });

    test('ISO year-month', () {
      expect(normalizeDate('2006-08'), equals('8.2006'));
    });

    test('ISO year-month-day', () {
      expect(normalizeDate('2006-08-01'), equals('1.8.2006'));
    });

    test('ISO leading zeros stripped', () {
      expect(normalizeDate('2006-01-05'), equals('5.1.2006'));
    });

    test('English long form "MMMM d, yyyy"', () {
      expect(normalizeDate('August 1, 2006'), equals('1.8.2006'));
    });

    test('English long form "MMMM yyyy"', () {
      expect(normalizeDate('August 2006'), equals('8.2006'));
    });

    test('English short form "MMM yyyy"', () {
      expect(normalizeDate('Aug 2006'), equals('8.2006'));
    });

    test('MARC21 annotation with copyright symbol', () {
      expect(normalizeDate('©2006'), equals('2006'));
    });

    test('MARC21 annotation "c2006."', () {
      expect(normalizeDate('c2006.'), equals('2006'));
    });

    test('MARC21 annotation "[2006]"', () {
      expect(normalizeDate('[2006]'), equals('2006'));
    });

    test('empty string returns empty', () {
      expect(normalizeDate(''), isEmpty);
    });

    test('unrecognisable string returns empty', () {
      expect(normalizeDate('no date here'), isEmpty);
    });
  });

  // -------------------------------------------------------------------------
  // BookLookupResult.mergeFrom
  // -------------------------------------------------------------------------
  group('BookLookupResult.mergeFrom', () {
    test('first non-empty name wins', () {
      final a = BookLookupResult()..name = 'First';
      final b = BookLookupResult()..name = 'Second';
      a.mergeFrom(b);
      expect(a.name, equals('First'));
    });

    test('empty field filled from other', () {
      final a = BookLookupResult();
      final b = BookLookupResult()..name = 'From B';
      a.mergeFrom(b);
      expect(a.name, equals('From B'));
    });

    test('longer publicationDate wins', () {
      final a = BookLookupResult()..publicationDate = '2006';
      final b = BookLookupResult()..publicationDate = '8.2006';
      a.mergeFrom(b);
      expect(a.publicationDate, equals('8.2006'));
    });

    test('shorter publicationDate does not overwrite longer', () {
      final a = BookLookupResult()..publicationDate = '1.8.2006';
      final b = BookLookupResult()..publicationDate = '2006';
      a.mergeFrom(b);
      expect(a.publicationDate, equals('1.8.2006'));
    });

    test('empty authors filled from other', () {
      final a = BookLookupResult();
      final b = BookLookupResult()..authors = ['Author A'];
      a.mergeFrom(b);
      expect(a.authors, equals(['Author A']));
    });

    test('non-empty authors not overwritten', () {
      final a = BookLookupResult()..authors = ['Author A'];
      final b = BookLookupResult()..authors = ['Author B'];
      a.mergeFrom(b);
      expect(a.authors, equals(['Author A']));
    });

    test('coverUrl filled from other when null', () {
      final a = BookLookupResult();
      final b = BookLookupResult()..coverUrl = 'https://example.com/cover.jpg';
      a.mergeFrom(b);
      expect(a.coverUrl, equals('https://example.com/cover.jpg'));
    });

    test('coverUrl not overwritten when already set', () {
      final a = BookLookupResult()..coverUrl = 'https://a.com/cover.jpg';
      final b = BookLookupResult()..coverUrl = 'https://b.com/cover.jpg';
      a.mergeFrom(b);
      expect(a.coverUrl, equals('https://a.com/cover.jpg'));
    });

    test('isEmpty true for default instance', () {
      expect(BookLookupResult().isEmpty, isTrue);
    });

    test('isEmpty false after setting name', () {
      final r = BookLookupResult()..name = 'Test';
      expect(r.isEmpty, isFalse);
    });
  });

  // -------------------------------------------------------------------------
  // openlibraryToResult
  // -------------------------------------------------------------------------
  group('openlibraryToResult', () {
    test('null input returns empty result', () {
      expect(openlibraryToResult(null).isEmpty, isTrue);
    });

    test('empty map returns empty result', () {
      expect(openlibraryToResult({}).isEmpty, isTrue);
    });

    test('parses title', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.name, equals('Programming Python'));
    });

    test('parses authors', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.authors, equals(['Mark Lutz']));
    });

    test('parses publisher', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.publisher, equals("O'Reilly"));
    });

    test('parses publication date (English long form)', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.publicationDate, equals('8.2006'));
    });

    test('parses ISBN-10 and ISBN-13', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.isbn10, equals('0596007124'));
      expect(r.isbn13, equals('9780596007126'));
    });

    test('parses page count', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.pages, equals('1564'));
    });

    test('parses cover URL', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.coverUrl,
          equals('https://covers.openlibrary.org/b/id/123-M.jpg'));
    });

    test('parses DDC', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.ddc, equals('005.133'));
    });

    test('parses description as plain string', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.description, equals('A comprehensive Python guide.'));
    });

    test('parses description when it is an object', () {
      final r = openlibraryToResult(_openlibraryJsonObjectDesc);
      expect(r.description, equals('Object description.'));
    });

    test('parses genres joined with semicolons', () {
      final r = openlibraryToResult(_openlibraryJson);
      expect(r.genre, equals('Programming; Python'));
    });
  });

  // -------------------------------------------------------------------------
  // googlebooksToResult
  // -------------------------------------------------------------------------
  group('googlebooksToResult', () {
    test('null input returns empty result', () {
      expect(googlebooksToResult('9780596007126', null).isEmpty, isTrue);
    });

    test('no matching ISBN returns empty result', () {
      final r = googlebooksToResult('0000000000000', _googlebooksJson);
      expect(r.isEmpty, isTrue);
    });

    test('matches by ISBN-13', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.name, startsWith('Programming Python'));
    });

    test('matches by ISBN-10', () {
      final r = googlebooksToResult('0596007124', _googlebooksJson);
      expect(r.name, startsWith('Programming Python'));
    });

    test('appends subtitle with " : "', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.name,
          equals('Programming Python : Powerful Object-Oriented Programming'));
    });

    test('parses authors', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.authors, equals(['Mark Lutz']));
    });

    test('parses publisher', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.publisher, equals("O'Reilly"));
    });

    test('parses publication date from ISO', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.publicationDate, equals('1.8.2006'));
    });

    test('parses ISBN-10 and ISBN-13', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.isbn10, equals('0596007124'));
      expect(r.isbn13, equals('9780596007126'));
    });

    test('parses page count', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.pages, equals('1564'));
    });

    test('parses categories as genre', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.genre, equals('Computers'));
    });

    test('parses description', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.description, equals('A comprehensive Python guide.'));
    });

    test('parses thumbnail URL', () {
      final r = googlebooksToResult('9780596007126', _googlebooksJson);
      expect(r.coverUrl,
          equals('https://books.google.com/thumbnail.jpg'));
    });
  });

  // -------------------------------------------------------------------------
  // parseMarc21Xml
  // -------------------------------------------------------------------------
  group('parseMarc21Xml', () {
    test('empty string returns empty result', () {
      expect(parseMarc21Xml('', '').isEmpty, isTrue);
    });

    test('malformed XML returns empty result', () {
      expect(parseMarc21Xml('<not valid xml', '').isEmpty, isTrue);
    });

    test('parses title from field 245 \$a', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.name, equals('Programming Python'));
    });

    test('parses authors from field 245 \$c', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.authors, equals(['Mark Lutz']));
    });

    test('parses publisher from field 264 ind2=1 \$b', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.publisher, equals("O'Reilly"));
    });

    test('parses publication date from field 264 \$c', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.publicationDate, equals('2006'));
    });

    test('parses publisher from field 260 when 264 absent', () {
      final r = parseMarc21Xml(_marc21XmlWith260, '');
      expect(r.publisher, equals('Old Publisher'));
      expect(r.publicationDate, equals('1999'));
    });

    test('parses ISBN-13 from field 020', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.isbn13, equals('9780596007126'));
    });

    test('parses DDC from field 082', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.ddc, equals('005.133'));
    });

    test('parses page count from field 300', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.pages, equals('1564'));
    });

    test('parses description from field 520 (trailing dot stripped)', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.description,
          equals('A comprehensive guide to Python programming'));
    });

    test('merges multiple field 655 genre values with semicolons', () {
      final r = parseMarc21Xml(_marc21XmlSample, '9780596007126');
      expect(r.genre, equals('Programmiersprache; Python'));
    });

    test('falls back to query ISBN when record has no ISBN field', () {
      final r = parseMarc21Xml(_marc21XmlWith260, '9780596007126');
      expect(r.isbn13, equals('9780596007126'));
      expect(r.isbn10, equals('0596007124'));
    });
  });

  // -------------------------------------------------------------------------
  // parseUnimarcXml
  // -------------------------------------------------------------------------
  group('parseUnimarcXml', () {
    test('empty string returns empty result', () {
      expect(parseUnimarcXml('').isEmpty, isTrue);
    });

    test('malformed XML returns empty result', () {
      expect(parseUnimarcXml('<broken').isEmpty, isTrue);
    });

    test('parses title from field 200 \$a', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.name, equals('Programming Python'));
    });

    test('parses author from field 700 forename + family', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.authors, contains('Mark Lutz'));
    });

    test('parses publisher from field 210 \$c', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.publisher, equals("O'Reilly"));
    });

    test('parses date from field 210 \$d', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.publicationDate, equals('2006'));
    });

    test('parses ISBN from field 010 (hyphens stripped)', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.isbn13, equals('9780596007126'));
    });

    test('parses page count from field 215', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.pages, equals('1564'));
    });

    test('parses description from field 330 (trailing dot stripped)', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.description, equals('A comprehensive guide'));
    });

    test('parses genre from field 606', () {
      final r = parseUnimarcXml(_unimarcXmlSample);
      expect(r.genre, equals('Python'));
    });
  });

  // -------------------------------------------------------------------------
  // queryIsbn — no services enabled
  // -------------------------------------------------------------------------
  group('queryIsbn (no network)', () {
    test('returns empty result when no service is enabled', () async {
      final settings = InternetServicesSettings(
        useOpenlibrary: false,
        useGoogleapis: false,
        googleapisKey: '',
        useMusicbrainz: false,
        useBgg: false,
        bggToken: '',
        useOpengtindb: false,
        opengtindbUserid: '',
        timeoutMs: 100,
        sruServers: const [],
        downloadThumbnails: false,
      );
      final r = await queryIsbn('9780596007126', settings);
      expect(r.isEmpty, isTrue);
    });

    test('fills ISBN fields from query parameter even without network',
        () async {
      // All SRU servers enabled but unreachable in 1 ms → timeout immediately.
      // ISBN fields must still be set from the query string.
      final settings = InternetServicesSettings(
        useOpenlibrary: false,
        useGoogleapis: false,
        googleapisKey: '',
        useMusicbrainz: false,
        useBgg: false,
        bggToken: '',
        useOpengtindb: false,
        opengtindbUserid: '',
        timeoutMs: 1,
        sruServers: [
          SruServer(
            name: 'Dummy',
            endpoint: 'http://0.0.0.0',
            isbnField: 'isbn',
            eanField: '',
            recordSchema: 'marcxml',
            enabled: true,
          ),
        ],
        downloadThumbnails: false,
      );
      final r = await queryIsbn('9780596007126', settings);
      // Network fails, but _fillIsbn must have set the ISBN fields.
      expect(r.isbn13, equals('9780596007126'));
      expect(r.isbn10, equals('0596007124'));
    });
  });

  // -------------------------------------------------------------------------
  // Live network tests (opt-in via COLMAN_RUN_NETWORK_TESTS=1)
  // -------------------------------------------------------------------------
  group('queryIsbn (live network)', () {
    const isbn13 = '9780596007126'; // "Programming Python" 3rd ed.
    const isbn10 = '0596007124';

    test('DNB SRU returns title and publisher', () async {
      if(Platform.environment['COLMAN_RUN_NETWORK_TESTS'] != '1') return;
      final settings = InternetServicesSettings(
        useOpenlibrary: false,
        useGoogleapis: false,
        googleapisKey: '',
        useMusicbrainz: false,
        useBgg: false,
        bggToken: '',
        useOpengtindb: false,
        opengtindbUserid: '',
        timeoutMs: 10000,
        sruServers: InternetServicesSettings.defaults().sruServers
            .where((s) => s.name.contains('Nationalbibliothek'))
            .map((s) => s.copyWith(enabled: true))
            .toList(),
        downloadThumbnails: false,
      );
      final r = await queryIsbn(isbn13, settings);
      expect(r.name, isNotEmpty);
      expect(r.publisher, isNotEmpty);
      expect(r.isbn13, equals(isbn13));
      expect(r.isbn10, equals(isbn10));
    }, timeout: const Timeout(Duration(seconds: 30)));

    test('Open Library returns title, authors, and cover URL', () async {
      if(Platform.environment['COLMAN_RUN_NETWORK_TESTS'] != '1') return;
      final settings = InternetServicesSettings(
        useOpenlibrary: true,
        useGoogleapis: false,
        googleapisKey: '',
        useMusicbrainz: false,
        useBgg: false,
        bggToken: '',
        useOpengtindb: false,
        opengtindbUserid: '',
        timeoutMs: 10000,
        sruServers: const [],
        downloadThumbnails: false,
      );
      final r = await queryIsbn(isbn13, settings);
      expect(r.name, isNotEmpty);
      expect(r.authors, isNotEmpty);
      expect(r.coverUrl, isNotNull);
      expect(r.isbn13, equals(isbn13));
    }, timeout: const Timeout(Duration(seconds: 30)));
  });

  // -------------------------------------------------------------------------
  group('multi-record responses keep first-record behavior', () {
    const marc21MultiRecord = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse>
<records>
<record><recordData><record>
<datafield tag="245"><subfield code="a">First Title</subfield></datafield>
</record></recordData></record>
<record><recordData><record>
<datafield tag="245"><subfield code="a">Second Title</subfield></datafield>
</record></recordData></record>
</records>
</searchRetrieveResponse>
''';

    const unimarcMultiRecord = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse>
<records>
<record><recordData><record>
<datafield tag="200"><subfield code="a">Premier Titre</subfield></datafield>
</record></recordData></record>
<record><recordData><record>
<datafield tag="200"><subfield code="a">Deuxieme Titre</subfield></datafield>
</record></recordData></record>
</records>
</searchRetrieveResponse>
''';

    test('parseMarc21Xml still returns the first record only', () {
      expect(parseMarc21Xml(marc21MultiRecord, '').name, 'First Title');
    });

    test('parseUnimarcXml still returns the first record only', () {
      expect(parseUnimarcXml(unimarcMultiRecord).name, 'Premier Titre');
    });
  });
}
