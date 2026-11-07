// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/marc_xml.dart';
import 'package:colman_client/api/title_lookup.dart';
import 'package:colman_client/models/internet_services_settings.dart';

const SruServer _marc21Server = SruServer(
  name: 'Test Server',
  endpoint: 'https://example.org/sru',
  isbnField: 'isbn',
  eanField: '',
  titleField: 'tit',
  recordSchema: 'MARC21-xml',
  enabled: true,
);

const SruServer _unimarcServer = SruServer(
  name: 'BnF Test',
  endpoint: 'https://example.org/sru-bnf',
  isbnField: 'bib.isbn',
  eanField: '',
  titleField: 'bib.title',
  recordSchema: 'unimarcxchange',
  enabled: true,
);

/// A seven-record MARC21 SRU response with one record per media kind.
const String _marc21MixedFixture = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse>
<numberOfRecords>7</numberOfRecords>
<records>

<record><recordData><record>
<leader>00000cam a2200000 c 4500</leader>
<datafield tag="245">
<subfield code="a">Title Book</subfield>
<subfield code="c">Some Author</subfield>
</datafield>
</record></recordData></record>

<record><recordData><record>
<leader>00000cjm a2200000 c 4500</leader>
<datafield tag="245">
<subfield code="a">Title CD</subfield>
</datafield>
</record></recordData></record>

<record><recordData><record>
<leader>00000cgm a2200000 c 4500</leader>
<controlfield tag="007">vd cvaizq</controlfield>
<datafield tag="245">
<subfield code="a">Title DVD</subfield>
</datafield>
</record></recordData></record>

<record><recordData><record>
<leader>00000cgm a2200000 c 4500</leader>
<controlfield tag="007">vd csaizs</controlfield>
<datafield tag="245">
<subfield code="a">Title BluRay</subfield>
</datafield>
</record></recordData></record>

<record><recordData><record>
<leader>00000cgm a2200000 c 4500</leader>
<datafield tag="300">
<subfield code="a">1 Blu-ray Disc (90 min)</subfield>
</datafield>
<datafield tag="245">
<subfield code="a">Title BluRayText</subfield>
</datafield>
</record></recordData></record>

<record><recordData><record>
<leader>00000cgm a2200000 c 4500</leader>
<datafield tag="245">
<subfield code="a">Title Video</subfield>
</datafield>
</record></recordData></record>

<record><recordData><record>
<datafield tag="245">
<subfield code="a">Title Unknown</subfield>
</datafield>
</record></recordData></record>

</records>
</searchRetrieveResponse>
''';

/// A two-record UNIMARC SRU response (BnF style); second record untitled.
const String _unimarcFixture = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse>
<records>
<record><recordData><record>
<datafield tag="200">
<subfield code="a">Titre Un</subfield>
</datafield>
<datafield tag="700">
<subfield code="a">Hugo</subfield>
<subfield code="b">Victor</subfield>
</datafield>
</record></recordData></record>
<record><recordData><record>
<datafield tag="210">
<subfield code="c">Editeur</subfield>
</datafield>
</record></recordData></record>
</records>
</searchRetrieveResponse>
''';

List<String> _names(List<TitleSearchResult> results) =>
    results.map((r) => r.name).toList();

void main() {
  group('yearFromDate', () {
    test('extracts year from normalised dates', () {
      expect(yearFromDate('1937'), '1937');
      expect(yearFromDate('3.1999'), '1999');
      expect(yearFromDate('30.3.1999'), '1999');
      expect(yearFromDate(''), '');
      expect(yearFromDate('unknown'), '');
    });
  });

  group('openlibrarySearchToResults', () {
    test('parses docs and skips untitled entries', () {
      final root = jsonDecode('''
        {"numFound":3,"docs":[
          {"title":"Der Hobbit",
           "author_name":["J. R. R. Tolkien"],
           "first_publish_year":1937,
           "cover_i":12345,
           "publisher":["Klett","Other"]},
          {"title":""},
          {"author_name":["No Title"]}
        ]}''') as Map<String, dynamic>;
      final results = openlibrarySearchToResults(root);
      expect(results, hasLength(1));
      final r = results.first;
      expect(r.name, 'Der Hobbit');
      expect(r.year, '1937');
      expect(r.source, 'Open Library');
      expect(r.coverUrl, 'https://covers.openlibrary.org/b/id/12345-M.jpg');
      expect(r.data.authors, ['J. R. R. Tolkien']);
      expect(r.data.publisher, 'Klett');
      expect(r.data.publicationDate, '1937');
      // Deliberately no ISBN: search docs mix all editions.
      expect(r.data.isbn10, '');
      expect(r.data.isbn13, '');
    });

    test('returns empty list for null or empty input', () {
      expect(openlibrarySearchToResults(null), isEmpty);
      expect(openlibrarySearchToResults({'docs': []}), isEmpty);
    });
  });

  group('googlebooksSearchToResults', () {
    test('parses volumes with subtitle and per-edition ISBNs', () {
      final root = jsonDecode('''
        {"items":[
          {"volumeInfo":{"title":"Buch Eins","subtitle":"Untertitel",
            "authors":["A1"],
            "industryIdentifiers":[
              {"type":"ISBN_13","identifier":"9780596007126"},
              {"type":"ISBN_10","identifier":"0596007124"}],
            "publishedDate":"2020-05-01",
            "publisher":"Verlag",
            "pageCount":300,
            "imageLinks":{"thumbnail":"http://img/1.jpg"}}},
          {"volumeInfo":{"title":"Buch Zwei"}},
          {"volumeInfo":{}}
        ]}''') as Map<String, dynamic>;
      final results = googlebooksSearchToResults(root);
      expect(results, hasLength(2));
      expect(results[0].name, 'Buch Eins : Untertitel');
      expect(results[0].year, '2020');
      expect(results[0].source, 'Google Books');
      expect(results[0].coverUrl, 'http://img/1.jpg');
      expect(results[0].data.isbn13, '9780596007126');
      expect(results[0].data.isbn10, '0596007124');
      expect(results[1].name, 'Buch Zwei');
      expect(results[1].year, '');
    });

    test('returns empty list for null input', () {
      expect(googlebooksSearchToResults(null), isEmpty);
    });
  });

  group('musicbrainzSearchToResults', () {
    test('parses every release and skips untitled ones', () {
      const json = '''
        {"releases":[
          {"id":"mbid-1","title":"Album Eins",
           "artist-credit":[
             {"name":"Artist A","joinphrase":" & "},
             {"name":"Artist B"}],
           "label-info":[{"label":{"name":"Label X"},
             "catalog-number":"LX-1"}],
           "date":"2001-09-10"},
          {"id":"mbid-2","title":"Album Zwei"},
          {"title":""}
        ]}''';
      final results = musicbrainzSearchToResults(json);
      expect(results, hasLength(2));
      final r = results.first;
      expect(r.name, 'Album Eins');
      expect(r.year, '2001');
      expect(r.source, 'MusicBrainz');
      expect(r.coverUrl,
          'https://coverartarchive.org/release/mbid-1/front-250');
      expect(r.data.artists, ['Artist A & Artist B']);
      expect(r.data.publisher, 'Label X');
      expect(r.data.publisherNumber, 'LX-1');
      expect(results[1].name, 'Album Zwei');
    });

    test('returns empty list for empty or invalid input', () {
      expect(musicbrainzSearchToResults(''), isEmpty);
      expect(musicbrainzSearchToResults('not json'), isEmpty);
      expect(musicbrainzSearchToResults('{"releases":[]}'), isEmpty);
    });
  });

  group('detectMarc21Kind / kindMatches', () {
    DetectedMediaKind kindOf(int index) {
      final records = sruMarcRecords(_marc21MixedFixture);
      return detectMarc21Kind(records[index]);
    }

    test('detects kinds from leader, 007, and carrier text', () {
      expect(kindOf(0), DetectedMediaKind.book);
      expect(kindOf(1), DetectedMediaKind.cd);
      expect(kindOf(2), DetectedMediaKind.dvd);
      expect(kindOf(3), DetectedMediaKind.bluray);
      // Generic video refined to Blu-ray via the 300 \$a carrier text.
      expect(kindOf(4), DetectedMediaKind.bluray);
      expect(kindOf(5), DetectedMediaKind.video);
      expect(kindOf(6), DetectedMediaKind.unknown);
    });

    test('unknown matches everything, video matches dvd and bluray', () {
      expect(kindMatches(DetectedMediaKind.unknown, MediaKind.book), isTrue);
      expect(kindMatches(DetectedMediaKind.unknown, MediaKind.bluray), isTrue);
      expect(kindMatches(DetectedMediaKind.video, MediaKind.dvd), isTrue);
      expect(kindMatches(DetectedMediaKind.video, MediaKind.bluray), isTrue);
      expect(kindMatches(DetectedMediaKind.video, MediaKind.cd), isFalse);
      expect(kindMatches(DetectedMediaKind.dvd, MediaKind.bluray), isFalse);
      expect(kindMatches(DetectedMediaKind.bluray, MediaKind.dvd), isFalse);
      expect(kindMatches(DetectedMediaKind.book, MediaKind.boardGame),
          isFalse);
    });
  });

  group('sruTitleToResults', () {
    test('book request keeps book and undetectable records', () {
      final results = sruTitleToResults(
          _marc21Server, _marc21MixedFixture, MediaKind.book);
      expect(_names(results), ['Title Book', 'Title Unknown']);
      expect(results.first, isA<BookTitleResult>());
      expect((results.first as BookTitleResult).data.authors,
          ['Some Author']);
      expect(results.first.source, 'Test Server');
    });

    test('cd request keeps cd and undetectable records', () {
      final results = sruTitleToResults(
          _marc21Server, _marc21MixedFixture, MediaKind.cd);
      expect(_names(results), ['Title CD', 'Title Unknown']);
      expect(results.first, isA<AvTitleResult>());
    });

    test('dvd request excludes explicit Blu-ray records', () {
      final results = sruTitleToResults(
          _marc21Server, _marc21MixedFixture, MediaKind.dvd);
      expect(_names(results),
          ['Title DVD', 'Title Video', 'Title Unknown']);
    });

    test('bluray request keeps explicit and text-detected Blu-rays', () {
      final results = sruTitleToResults(
          _marc21Server, _marc21MixedFixture, MediaKind.bluray);
      expect(_names(results),
          ['Title BluRay', 'Title BluRayText', 'Title Video',
           'Title Unknown']);
    });

    test('unimarc server supports book searches only', () {
      final books = sruTitleToResults(
          _unimarcServer, _unimarcFixture, MediaKind.book);
      expect(_names(books), ['Titre Un']);
      expect((books.first as BookTitleResult).data.authors,
          ['Victor Hugo']);
      expect(
          sruTitleToResults(_unimarcServer, _unimarcFixture, MediaKind.cd),
          isEmpty);
      expect(
          sruTitleToResults(
              _unimarcServer, _unimarcFixture, MediaKind.bluray),
          isEmpty);
    });

    test('returns empty list for garbage input', () {
      expect(sruTitleToResults(_marc21Server, '', MediaKind.book), isEmpty);
      expect(sruTitleToResults(_marc21Server, 'not xml', MediaKind.book),
          isEmpty);
    });
  });

  group('sruTitleQueryUrl', () {
    test('uses version 1.1 for MARC21 servers', () {
      final url = sruTitleQueryUrl(_marc21Server, 'Der Hobbit');
      expect(url, contains('version=1.1'));
      expect(url, contains('maximumRecords=10'));
      expect(url, contains('query=tit%3D%22Der%20Hobbit%22'));
      expect(url, contains('recordSchema=MARC21-xml'));
    });

    test('uses version 1.2 for UNIMARC servers', () {
      final url = sruTitleQueryUrl(_unimarcServer, 'Le Hobbit');
      expect(url, contains('version=1.2'));
      expect(url, contains('query=bib.title%3D%22Le%20Hobbit%22'));
    });

    test('percent-encodes umlauts and trims the title', () {
      final url = sruTitleQueryUrl(_marc21Server, '  Käpt\'n Blaubär  ');
      expect(url, contains('%C3%A4'));
      expect(url, isNot(contains('%20%20')));
    });
  });

  group('queryTitle offline guards', () {
    InternetServicesSettings noServices() =>
        InternetServicesSettings.defaults().copyWith(
          sruServers: [],
        );

    test('blank title yields no results', () async {
      expect(await queryTitle('   ', MediaKind.book, noServices()), isEmpty);
    });

    test('no enabled services yields no results without network', () async {
      for(final kind in MediaKind.values) {
        expect(await queryTitle('Der Hobbit', kind, noServices()), isEmpty,
            reason: 'kind $kind');
      }
    });
  });

  group('titleSearchAvailableFor', () {
    test('per-kind availability follows the source matrix', () {
      final none = InternetServicesSettings.defaults().copyWith(
        sruServers: [],
      );
      for(final kind in MediaKind.values) {
        expect(titleSearchAvailableFor(kind, none), isFalse,
            reason: 'kind $kind');
      }
      expect(titleSearchAvailable(none), isFalse);

      final ol = none.copyWith(useOpenlibrary: true);
      expect(titleSearchAvailableFor(MediaKind.book, ol), isTrue);
      expect(titleSearchAvailableFor(MediaKind.cd, ol), isFalse);
      expect(titleSearchAvailable(ol), isTrue);

      final mb = none.copyWith(useMusicbrainz: true);
      expect(titleSearchAvailableFor(MediaKind.cd, mb), isTrue);
      expect(titleSearchAvailableFor(MediaKind.dvd, mb), isFalse);

      final bgg = none.copyWith(useBgg: true);
      expect(titleSearchAvailableFor(MediaKind.boardGame, bgg), isTrue);
      expect(titleSearchAvailableFor(MediaKind.book, bgg), isFalse);

      final sru = none.copyWith(sruServers: [_marc21Server]);
      for(final kind in
          [MediaKind.book, MediaKind.cd, MediaKind.dvd, MediaKind.bluray]) {
        expect(titleSearchAvailableFor(kind, sru), isTrue,
            reason: 'kind $kind');
      }
      expect(titleSearchAvailableFor(MediaKind.boardGame, sru), isFalse);

      // A UNIMARC-only configuration supports book searches only.
      final unimarcOnly = none.copyWith(sruServers: [_unimarcServer]);
      expect(titleSearchAvailableFor(MediaKind.book, unimarcOnly), isTrue);
      expect(titleSearchAvailableFor(MediaKind.dvd, unimarcOnly), isFalse);
    });
  });
}
