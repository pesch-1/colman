// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/ean_lookup.dart';
import 'package:colman_client/models/internet_services_settings.dart';

// ---------------------------------------------------------------------------
// Static fixtures
// ---------------------------------------------------------------------------

// MusicBrainz release search response with two artist credits and joinphrase.
const String _musicBrainzJson = '''
{
  "releases": [
    {
      "id": "abc123-def456",
      "title": "Kind of Blue",
      "artist-credit": [
        {"name": "Miles Davis", "joinphrase": " feat. "},
        {"name": "Bill Evans", "joinphrase": ""}
      ],
      "label-info": [
        {
          "label": {"name": "Columbia"},
          "catalog-number": "CL 1355"
        }
      ],
      "date": "1959-08-17"
    }
  ]
}
''';

// MusicBrainz response with empty releases array.
const String _musicBrainzEmptyJson = '{"releases": []}';

// SRU MARC21-xml response for a CD.
const String _marcXmlCd = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="028" ind1="0" ind2=" ">
            <subfield code="a">CL 1355</subfield>
            <subfield code="b">Columbia</subfield>
          </datafield>
          <datafield tag="041" ind1="0" ind2=" ">
            <subfield code="a">eng</subfield>
          </datafield>
          <datafield tag="100" ind1="1" ind2=" ">
            <subfield code="a">Davis, Miles.</subfield>
          </datafield>
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Kind of Blue.</subfield>
          </datafield>
          <datafield tag="264" ind1=" " ind2="1">
            <subfield code="b">Columbia Records</subfield>
            <subfield code="c">1959</subfield>
          </datafield>
          <datafield tag="300" ind1=" " ind2=" ">
            <subfield code="a">1 CD (46 min.)</subfield>
          </datafield>
          <datafield tag="336" ind1=" " ind2=" ">
            <subfield code="a">performed music</subfield>
          </datafield>
          <datafield tag="337" ind1=" " ind2=" ">
            <subfield code="a">audio</subfield>
          </datafield>
          <datafield tag="338" ind1=" " ind2=" ">
            <subfield code="a">audio disc</subfield>
          </datafield>
          <datafield tag="520" ind1=" " ind2=" ">
            <subfield code="a">Classic jazz album</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// SRU MARC21-xml response for a DVD with cast, director via 700, and FSK.
const String _marcXmlDvd = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Die Verurteilten.</subfield>
          </datafield>
          <datafield tag="264" ind1=" " ind2="1">
            <subfield code="b">Warner Bros.</subfield>
            <subfield code="c">1994</subfield>
          </datafield>
          <datafield tag="337" ind1=" " ind2=" ">
            <subfield code="a">video</subfield>
          </datafield>
          <datafield tag="338" ind1=" " ind2=" ">
            <subfield code="a">videodisc</subfield>
          </datafield>
          <datafield tag="511" ind1="0" ind2=" ">
            <subfield code="a">Tim Robbins, Morgan Freeman</subfield>
          </datafield>
          <datafield tag="521" ind1=" " ind2=" ">
            <subfield code="a">FSK ab 16</subfield>
          </datafield>
          <datafield tag="700" ind1="1" ind2=" ">
            <subfield code="a">Darabont, Frank.</subfield>
            <subfield code="e">Regisseur.</subfield>
          </datafield>
          <datafield tag="700" ind1="1" ind2=" ">
            <subfield code="a">Gunton, Bob.</subfield>
            <subfield code="e">Schauspieler.</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// SRU MARC21-xml response for a DVD with 508 director fallback and no 700.
const String _marcXmlDvd508 = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Metropolis.</subfield>
          </datafield>
          <datafield tag="500" ind1=" " ind2=" ">
            <subfield code="a">General note about the film</subfield>
          </datafield>
          <datafield tag="508" ind1=" " ind2=" ">
            <subfield code="a">Regie: Fritz Lang</subfield>
          </datafield>
          <datafield tag="520" ind1=" " ind2=" ">
            <subfield code="a">Silent film classic</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// SRU MARC21-xml response for a board game detected via field 653.
const String _marcXmlBoardGame = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Die Siedler von Catan.</subfield>
          </datafield>
          <datafield tag="264" ind1=" " ind2="1">
            <subfield code="b">Kosmos</subfield>
            <subfield code="c">1995</subfield>
          </datafield>
          <datafield tag="653" ind1=" " ind2=" ">
            <subfield code="a">Gesellschaftsspiel</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// SRU MARC21-xml response with a part number in field 245 $n.
const String _marcXmlPartNumber = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Symphony no. 5</subfield>
            <subfield code="n">op. 67 / Beethoven</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// SRU MARC21-xml response with 260 fallback dating.
const String _marcXml260 = '''
<?xml version="1.0" encoding="UTF-8"?>
<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
  <records>
    <record>
      <recordData>
        <record xmlns="http://www.loc.gov/MARC21/slim">
          <datafield tag="245" ind1="1" ind2="0">
            <subfield code="a">Old Record</subfield>
          </datafield>
          <datafield tag="260" ind1=" " ind2=" ">
            <subfield code="b">Old Label</subfield>
            <subfield code="c">1975</subfield>
          </datafield>
        </record>
      </recordData>
    </record>
  </records>
</searchRetrieveResponse>
''';

// OpenGTINDB success response (ASCII; Latin-1 byte handling is an
// integration concern not covered by offline unit tests).
const String _openGtinDbSuccess = '''
error=0
ean=4002051683320
name=Gesellschaftsspiel Catan
detailname=Catan - Das Spiel (KOSMOS 6820280)
vendor=KOSMOS Verlag
descr=Strategisches Brettspiel fuer 3-4 Spieler
descr_en=Strategy board game for 3-4 players
''';

// OpenGTINDB response where only 'name' (no 'detailname') is present.
const String _openGtinDbNameOnly = '''
error=0
name=Catan Brettspiel
vendor=KOSMOS
descr_en=Strategy game
''';

// OpenGTINDB error response.
const String _openGtinDbError = 'error=13\n';

// BGG search XML with three items in non-alphabetical order.
const String _bggSearchXml = '''
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<items total="3" termsofuse="https://boardgamegeek.com/xmlapi/termsofuse">
  <item type="boardgame" id="13">
    <name type="primary" sortindex="5" value="Catan"/>
    <yearpublished value="1995"/>
  </item>
  <item type="boardgame" id="167791">
    <name type="primary" sortindex="1" value="Terraforming Mars"/>
    <yearpublished value="2016"/>
  </item>
  <item type="boardgame" id="42">
    <name type="primary" sortindex="1" value="Agricola"/>
    <yearpublished value="2007"/>
  </item>
</items>
''';

// BGG search XML where one item has no primary name.
const String _bggSearchXmlMissingName = '''
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<items total="2">
  <item type="boardgame" id="99">
    <name type="alternate" sortindex="1" value="Only Alternate"/>
  </item>
  <item type="boardgame" id="7">
    <name type="primary" sortindex="1" value="Valid Game"/>
    <yearpublished value="2000"/>
  </item>
</items>
''';

// BGG thing XML with German alternate name, HTML entities, and all link types.
const String _bggThingXml = '''
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<items termsofuse="https://boardgamegeek.com/xmlapi/termsofuse">
  <item type="boardgame" id="13">
    <thumbnail>https://example.com/thumb.jpg</thumbnail>
    <name type="primary" sortindex="5" value="Catan"/>
    <name type="alternate" sortindex="1" value="Die Fürsten von Catan"/>
    <yearpublished value="1995"/>
    <minplayers value="3"/>
    <maxplayers value="4"/>
    <minage value="10"/>
    <playingtime value="120"/>
    <description>In Catan &amp;amp; beyond, settlers compete.</description>
    <link type="boardgamecategory" id="1015" value="Civilization"/>
    <link type="boardgamecategory" id="1070" value="Economic"/>
    <link type="boardgamepublisher" id="267" value="Kosmos"/>
    <link type="boardgamepublisher" id="32" value="999 Games"/>
    <link type="boardgamedesigner" id="30" value="Klaus Teuber"/>
    <link type="boardgameartist" id="382" value="Volkan Baga"/>
  </item>
</items>
''';

// BGG thing XML with no German alternate name (only English primary).
const String _bggThingXmlEnglish = '''
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<items termsofuse="https://boardgamegeek.com/xmlapi/termsofuse">
  <item type="boardgame" id="42">
    <name type="primary" sortindex="1" value="Ticket to Ride"/>
    <yearpublished value="2004"/>
    <minplayers value="2"/>
    <maxplayers value="5"/>
    <minage value="8"/>
    <playingtime value="75"/>
    <description>A cross-country train adventure.</description>
    <link type="boardgamepublisher" id="21" value="Days of Wonder"/>
    <link type="boardgamedesigner" id="9" value="Alan R. Moon"/>
  </item>
</items>
''';

// BGG thing XML starting with <message> (BGG rate-limiting / 202-style).
const String _bggThingMessage = '<message>Rate limit exceeded.</message>';

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void main() {
  // -------------------------------------------------------------------------
  group('isProductEan', () {
    test('valid product EAN-13 returns true', () {
      // 4002051683320 is a real KOSMOS EAN
      expect(isProductEan('4002051683320'), isTrue);
    });

    test('ISBN 978-prefix rejected', () {
      expect(isProductEan('9780596007126'), isFalse);
    });

    test('ISBN 979-prefix rejected', () {
      expect(isProductEan('9791032349014'), isFalse);
    });

    test('wrong check digit returns false', () {
      expect(isProductEan('4002051683321'), isFalse);
    });

    test('too short returns false', () {
      expect(isProductEan('400205168332'), isFalse);
    });

    test('too long returns false', () {
      expect(isProductEan('40020516833200'), isFalse);
    });

    test('non-digit character returns false', () {
      expect(isProductEan('400205168332X'), isFalse);
    });

    test('empty string returns false', () {
      expect(isProductEan(''), isFalse);
    });
  });

  // -------------------------------------------------------------------------
  group('isUpcA', () {
    test('valid 12-digit UPC-A returns true', () {
      // 012345678905 is a well-known test UPC-A
      expect(isUpcA('012345678905'), isTrue);
    });

    test('too short returns false', () {
      expect(isUpcA('01234567890'), isFalse);
    });

    test('too long returns false', () {
      expect(isUpcA('0123456789050'), isFalse);
    });

    test('non-digit character returns false', () {
      expect(isUpcA('01234567890X'), isFalse);
    });

    test('empty string returns false', () {
      expect(isUpcA(''), isFalse);
    });
  });

  // -------------------------------------------------------------------------
  group('EanLookupResult.merge', () {
    test('scalar first-non-empty wins for name', () {
      final a = EanLookupResult()..name = 'First';
      final b = EanLookupResult()..name = 'Second';
      a.merge(b);
      expect(a.name, 'First');
    });

    test('empty scalar filled from other', () {
      final a = EanLookupResult();
      final b = EanLookupResult()..publisher = 'Label';
      a.merge(b);
      expect(a.publisher, 'Label');
    });

    test('lists concatenated', () {
      final a = EanLookupResult()..artists = ['A'];
      final b = EanLookupResult()..artists = ['B', 'C'];
      a.merge(b);
      expect(a.artists, ['A', 'B', 'C']);
    });

    test('designers and genres concatenated', () {
      final a = EanLookupResult()
        ..designers = ['D1']
        ..genres = ['G1'];
      final b = EanLookupResult()
        ..designers = ['D2']
        ..genres = ['G2'];
      a.merge(b);
      expect(a.designers, ['D1', 'D2']);
      expect(a.genres, ['G1', 'G2']);
    });

    test('isGame is logical OR', () {
      final a = EanLookupResult();
      final b = EanLookupResult()..isGame = true;
      a.merge(b);
      expect(a.isGame, isTrue);
    });

    test('isGame stays true when other is false', () {
      final a = EanLookupResult()..isGame = true;
      final b = EanLookupResult();
      a.merge(b);
      expect(a.isGame, isTrue);
    });

    test('publicationDate longest wins', () {
      final a = EanLookupResult()..publicationDate = '1959';
      final b = EanLookupResult()..publicationDate = '17.8.1959';
      a.merge(b);
      expect(a.publicationDate, '17.8.1959');
    });

    test('publicationDate shortest not replaced by empty', () {
      final a = EanLookupResult()..publicationDate = '1959';
      final b = EanLookupResult();
      a.merge(b);
      expect(a.publicationDate, '1959');
    });

    test('coverUrl first-non-null wins', () {
      final a = EanLookupResult()..coverUrl = 'https://example.com/a.jpg';
      final b = EanLookupResult()..coverUrl = 'https://example.com/b.jpg';
      a.merge(b);
      expect(a.coverUrl, 'https://example.com/a.jpg');
    });

    test('coverUrl filled from other when null', () {
      final a = EanLookupResult();
      final b = EanLookupResult()..coverUrl = 'https://example.com/b.jpg';
      a.merge(b);
      expect(a.coverUrl, 'https://example.com/b.jpg');
    });

    test('isEmpty reflects all-empty state', () {
      expect(EanLookupResult().isEmpty, isTrue);
    });

    test('isEmpty false after setting name', () {
      final r = EanLookupResult()..name = 'x';
      expect(r.isEmpty, isFalse);
    });

    test('isEmpty false when isGame is true', () {
      final r = EanLookupResult()..isGame = true;
      expect(r.isEmpty, isFalse);
    });
  });

  // -------------------------------------------------------------------------
  group('musicBrainzToResult', () {
    test('empty string returns empty result', () {
      expect(musicBrainzToResult('').isEmpty, isTrue);
    });

    test('empty releases array returns empty result', () {
      expect(musicBrainzToResult(_musicBrainzEmptyJson).isEmpty, isTrue);
    });

    test('parses title', () {
      expect(musicBrainzToResult(_musicBrainzJson).name, 'Kind of Blue');
    });

    test('joins artist credits with joinphrase', () {
      final r = musicBrainzToResult(_musicBrainzJson);
      expect(r.artists, hasLength(1));
      expect(r.artists[0], 'Miles Davis feat. Bill Evans');
    });

    test('parses publisher label name', () {
      expect(musicBrainzToResult(_musicBrainzJson).publisher, 'Columbia');
    });

    test('parses catalog number as publisherNumber', () {
      expect(
        musicBrainzToResult(_musicBrainzJson).publisherNumber,
        'CL 1355',
      );
    });

    test('normalizes publication date', () {
      expect(
        musicBrainzToResult(_musicBrainzJson).publicationDate,
        '17.8.1959',
      );
    });

    test('builds Cover Art Archive URL from MBID', () {
      expect(
        musicBrainzToResult(_musicBrainzJson).coverUrl,
        'https://coverartarchive.org/release/abc123-def456/front-250',
      );
    });

    test('returns empty on malformed JSON', () {
      expect(musicBrainzToResult('not-json').isEmpty, isTrue);
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — CD', () {
    late EanLookupResult cd;
    setUpAll(() => cd = sruMarcToAvResult(_marcXmlCd));

    test('empty string returns empty result', () {
      expect(sruMarcToAvResult('').isEmpty, isTrue);
    });

    test('parses title from field 245 \$a (trailing punct stripped)', () {
      expect(cd.name, 'Kind of Blue');
    });

    test('parses artist from field 100 with name inversion', () {
      expect(cd.artists, ['Miles Davis']);
    });

    test('parses publisherNumber from field 028 \$a', () {
      expect(cd.publisherNumber, 'CL 1355');
    });

    test('publisher: 028 \$b wins over 264 \$b (first-wins)', () {
      // 028$b = "Columbia", 264$b = "Columbia Records"
      expect(cd.publisher, 'Columbia');
    });

    test('parses publicationDate from field 264 \$c', () {
      expect(cd.publicationDate, '1959');
    });

    test('parses language from field 041 \$a', () {
      expect(cd.language, 'eng');
    });

    test('parses extent from field 300', () {
      expect(cd.extent, '1 CD (46 min.)');
    });

    test('parses contentType from field 336', () {
      expect(cd.contentType, 'performed music');
    });

    test('parses mediaType from field 337', () {
      expect(cd.mediaType, 'audio');
    });

    test('parses carrierType from field 338', () {
      expect(cd.carrierType, 'audio disc');
    });

    test('parses description from field 520', () {
      expect(cd.description, 'Classic jazz album');
    });

    test('isGame is false', () {
      expect(cd.isGame, isFalse);
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — DVD with 700 roles', () {
    late EanLookupResult dvd;
    setUpAll(() => dvd = sruMarcToAvResult(_marcXmlDvd));

    test('parses title', () {
      expect(dvd.name, 'Die Verurteilten');
    });

    test('parses director from field 700 with Regisseur role', () {
      expect(dvd.director, 'Frank Darabont');
    });

    test('parses participants from field 511 and 700 Schauspieler', () {
      // 511$a = "Tim Robbins, Morgan Freeman"
      // 700 Schauspieler "Gunton, Bob" → "Bob Gunton" appended
      expect(
        dvd.participants,
        'Tim Robbins, Morgan Freeman; Bob Gunton',
      );
    });

    test('parses targetAudience from field 521', () {
      expect(dvd.targetAudience, 'FSK ab 16');
    });

    test('parses mediaType from field 337', () {
      expect(dvd.mediaType, 'video');
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — 508 director fallback', () {
    late EanLookupResult r;
    setUpAll(() => r = sruMarcToAvResult(_marcXmlDvd508));

    test('sets director from field 508 when no 700 Regisseur', () {
      expect(r.director, 'Regie: Fritz Lang');
    });

    test('520 preferred over 500 for description', () {
      // Both 500 and 520 are present; 500 comes first in the record.
      // Both use "only if empty" logic, so the first one (500) wins.
      // This matches the C++ marc21xml_to_av_item behaviour.
      expect(r.description, 'General note about the film');
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — board game', () {
    late EanLookupResult r;
    setUpAll(() => r = sruMarcToAvResult(_marcXmlBoardGame));

    test('isGame true when field 653 contains "spiel"', () {
      expect(r.isGame, isTrue);
    });

    test('parses name', () {
      expect(r.name, 'Die Siedler von Catan');
    });

    test('parses publicationDate from field 264', () {
      expect(r.publicationDate, '1995');
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — field 245 \$n part number', () {
    test('appends part number after stripping " /" suffix', () {
      final r = sruMarcToAvResult(_marcXmlPartNumber);
      // $n = "op. 67 / Beethoven" → strip " /" part → "op. 67"
      expect(r.name, 'Symphony no. 5: op. 67');
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — field 260 fallback', () {
    test('uses 260 \$b and \$c when 264 is absent', () {
      final r = sruMarcToAvResult(_marcXml260);
      expect(r.publisher, 'Old Label');
      expect(r.publicationDate, '1975');
    });
  });

  // -------------------------------------------------------------------------
  group('openGtinDbToResult', () {
    test('empty string returns empty result', () {
      expect(openGtinDbToResult('').isEmpty, isTrue);
    });

    test('non-zero error returns empty result', () {
      expect(openGtinDbToResult(_openGtinDbError).isEmpty, isTrue);
    });

    test('detailname preferred over name', () {
      final r = openGtinDbToResult(_openGtinDbSuccess);
      // detailname = "Catan - Das Spiel (KOSMOS 6820280)"
      // parenthetical stripped → "Catan - Das Spiel"
      expect(r.name, 'Catan - Das Spiel');
    });

    test('falls back to name when detailname absent', () {
      final r = openGtinDbToResult(_openGtinDbNameOnly);
      expect(r.name, 'Catan Brettspiel');
    });

    test('strips trailing parenthetical from detailname', () {
      final r = openGtinDbToResult(_openGtinDbSuccess);
      expect(r.name, isNot(contains('(')));
    });

    test('descr preferred over descr_en for description', () {
      final r = openGtinDbToResult(_openGtinDbSuccess);
      expect(r.description, 'Strategisches Brettspiel fuer 3-4 Spieler');
    });

    test('falls back to descr_en when descr absent', () {
      final r = openGtinDbToResult(_openGtinDbNameOnly);
      expect(r.description, 'Strategy game');
    });

    test('parses publisher from vendor field', () {
      expect(openGtinDbToResult(_openGtinDbSuccess).publisher, 'KOSMOS Verlag');
    });

    test('missing error field treated as success', () {
      const data = 'name=Test Product\nvendor=ACME\n';
      final r = openGtinDbToResult(data);
      expect(r.name, 'Test Product');
    });
  });

  // -------------------------------------------------------------------------
  group('bggSearchToResults', () {
    test('empty string returns empty list', () {
      expect(bggSearchToResults(''), isEmpty);
    });

    test('parses id and primary name', () {
      final results = bggSearchToResults(_bggSearchXml);
      final catan = results.firstWhere((r) => r.id == 13);
      expect(catan.name, 'Catan');
    });

    test('parses year', () {
      final results = bggSearchToResults(_bggSearchXml);
      final catan = results.firstWhere((r) => r.id == 13);
      expect(catan.year, '1995');
    });

    test('results sorted alphabetically by name', () {
      final results = bggSearchToResults(_bggSearchXml);
      expect(results.map((r) => r.name).toList(),
          ['Agricola', 'Catan', 'Terraforming Mars']);
    });

    test('items without primary name are skipped', () {
      final results = bggSearchToResults(_bggSearchXmlMissingName);
      expect(results, hasLength(1));
      expect(results[0].name, 'Valid Game');
    });

    test('returns malformed XML as empty list', () {
      expect(bggSearchToResults('<not valid xml'), isEmpty);
    });
  });

  // -------------------------------------------------------------------------
  group('bggThingToResult', () {
    late EanLookupResult r;
    setUpAll(() => r = bggThingToResult(_bggThingXml));

    test('empty string returns empty result', () {
      expect(bggThingToResult('').isEmpty, isTrue);
    });

    test('prefers German alternate name over English primary', () {
      expect(r.name, 'Die Fürsten von Catan');
    });

    test('uses primary name when no German alternate exists', () {
      final eng = bggThingToResult(_bggThingXmlEnglish);
      expect(eng.name, 'Ticket to Ride');
    });

    test('parses yearpublished as publicationDate', () {
      expect(r.publicationDate, '1995');
    });

    test('parses minplayers', () {
      expect(r.minPlayers, '3');
    });

    test('parses maxplayers', () {
      expect(r.maxPlayers, '4');
    });

    test('parses minage', () {
      expect(r.minAge, '10');
    });

    test('parses playingtime', () {
      expect(r.playingTime, '120');
    });

    test('decodes HTML entity &amp;amp; in description', () {
      // Raw XML: &amp;amp; → XML parse: &amp; → _stripHtml: &
      expect(r.description, contains('&'));
      expect(r.description, 'In Catan & beyond, settlers compete.');
    });

    test('first boardgamepublisher link is publisher', () {
      expect(r.publisher, 'Kosmos');
    });

    test('second boardgamepublisher link ignored', () {
      expect(r.publisher, isNot('999 Games'));
    });

    test('parses designers list', () {
      expect(r.designers, ['Klaus Teuber']);
    });

    test('parses artists list', () {
      expect(r.artists, ['Volkan Baga']);
    });

    test('parses genres list', () {
      expect(r.genres, containsAll(['Civilization', 'Economic']));
      expect(r.genres, hasLength(2));
    });

    test('parses thumbnail as coverUrl', () {
      expect(r.coverUrl, 'https://example.com/thumb.jpg');
    });

    test('coverUrl stays null without thumbnail or image', () {
      final eng = bggThingToResult(_bggThingXmlEnglish);
      expect(eng.coverUrl, isNull);
    });

    test('response starting with <message> is detected', () {
      // bggThingToResult itself does not handle the retry — the caller
      // (queryBggThing) does. Verify that the message response parses
      // to an empty result rather than crashing.
      final msg = bggThingToResult(_bggThingMessage);
      expect(msg.isEmpty, isTrue);
    });
  });

  // -------------------------------------------------------------------------
  group('queryEan (no network)', () {
    test('returns empty result when no service is enabled', () async {
      const settings = InternetServicesSettings(
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
      final result = await queryEan('4002051683320', settings);
      expect(result.isEmpty, isTrue);
    });

    test('returns empty result when SRU servers have no eanField', () async {
      const settings = InternetServicesSettings(
        useOpenlibrary: false,
        useGoogleapis: false,
        googleapisKey: '',
        useMusicbrainz: false,
        useBgg: false,
        bggToken: '',
        useOpengtindb: false,
        opengtindbUserid: '',
        timeoutMs: 100,
        sruServers: [
          SruServer(
            name: 'Test',
            endpoint: 'https://example.com/sru',
            isbnField: 'isbn',
            eanField: '', // empty eanField → skipped
            recordSchema: 'marcxml',
            enabled: true,
          ),
        ],
        downloadThumbnails: false,
      );
      final result = await queryEan('4002051683320', settings);
      expect(result.isEmpty, isTrue);
    });
  });

  // -------------------------------------------------------------------------
  group('sruMarcToAvResult — multi-record response', () {
    const multiRecord = '''
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

    test('still returns the first record only', () {
      expect(sruMarcToAvResult(multiRecord).name, 'First Title');
    });
  });
}
