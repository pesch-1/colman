// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart' show debugPrint;
import 'package:xml/xml.dart';

import '../models/internet_services_settings.dart';
import 'isbn_lookup.dart' show normalizeDate;
import 'marc_xml.dart';

// ---------------------------------------------------------------------------
// EanLookupResult
// ---------------------------------------------------------------------------

/// Metadata returned by an EAN internet lookup for AV media and board games.
///
/// All fields default to empty / null / false. Merge multiple results with
/// [merge]: scalars use first-non-empty-wins, lists concatenate, [isGame]
/// is OR'd, [publicationDate] keeps the longest (most specific) string.
class EanLookupResult {
  String name = '';
  String director = '';
  String participants = '';
  String publisher = '';
  String publisherNumber = '';
  String publicationDate = '';
  String language = '';
  String extent = '';
  String contentType = '';
  String mediaType = '';
  String carrierType = '';
  String description = '';
  String targetAudience = '';
  String minPlayers = '';
  String maxPlayers = '';
  String minAge = '';
  String playingTime = '';
  List<String> artists = [];
  List<String> designers = [];
  List<String> genres = [];
  bool isGame = false;
  String? coverUrl;

  bool get isEmpty =>
      name.isEmpty &&
      director.isEmpty &&
      participants.isEmpty &&
      publisher.isEmpty &&
      publisherNumber.isEmpty &&
      publicationDate.isEmpty &&
      language.isEmpty &&
      extent.isEmpty &&
      contentType.isEmpty &&
      mediaType.isEmpty &&
      carrierType.isEmpty &&
      description.isEmpty &&
      targetAudience.isEmpty &&
      minPlayers.isEmpty &&
      maxPlayers.isEmpty &&
      minAge.isEmpty &&
      playingTime.isEmpty &&
      artists.isEmpty &&
      designers.isEmpty &&
      genres.isEmpty &&
      !isGame &&
      coverUrl == null;

  /// Merges [other] into this result.
  ///
  /// Scalars: first non-empty value wins.
  /// [publicationDate]: longest (most specific) string wins.
  /// [isGame]: logical OR.
  /// Lists: entries from [other] are appended.
  /// [coverUrl]: first non-null wins.
  void merge(EanLookupResult other) {
    if(name.isEmpty && other.name.isNotEmpty) name = other.name;
    if(director.isEmpty && other.director.isNotEmpty) {
      director = other.director;
    }
    if(participants.isEmpty && other.participants.isNotEmpty) {
      participants = other.participants;
    }
    if(publisher.isEmpty && other.publisher.isNotEmpty) {
      publisher = other.publisher;
    }
    if(publisherNumber.isEmpty && other.publisherNumber.isNotEmpty) {
      publisherNumber = other.publisherNumber;
    }
    if(other.publicationDate.length > publicationDate.length) {
      publicationDate = other.publicationDate;
    }
    if(language.isEmpty && other.language.isNotEmpty) {
      language = other.language;
    }
    if(extent.isEmpty && other.extent.isNotEmpty) extent = other.extent;
    if(contentType.isEmpty && other.contentType.isNotEmpty) {
      contentType = other.contentType;
    }
    if(mediaType.isEmpty && other.mediaType.isNotEmpty) {
      mediaType = other.mediaType;
    }
    if(carrierType.isEmpty && other.carrierType.isNotEmpty) {
      carrierType = other.carrierType;
    }
    if(description.isEmpty && other.description.isNotEmpty) {
      description = other.description;
    }
    if(targetAudience.isEmpty && other.targetAudience.isNotEmpty) {
      targetAudience = other.targetAudience;
    }
    if(minPlayers.isEmpty && other.minPlayers.isNotEmpty) {
      minPlayers = other.minPlayers;
    }
    if(maxPlayers.isEmpty && other.maxPlayers.isNotEmpty) {
      maxPlayers = other.maxPlayers;
    }
    if(minAge.isEmpty && other.minAge.isNotEmpty) minAge = other.minAge;
    if(playingTime.isEmpty && other.playingTime.isNotEmpty) {
      playingTime = other.playingTime;
    }
    artists = [...artists, ...other.artists];
    designers = [...designers, ...other.designers];
    genres = [...genres, ...other.genres];
    isGame = isGame || other.isGame;
    coverUrl ??= other.coverUrl;
  }
}

// ---------------------------------------------------------------------------
// BggSearchResult
// ---------------------------------------------------------------------------

/// One result entry from a BoardGameGeek text search.
class BggSearchResult {
  const BggSearchResult({
    required this.id,
    required this.name,
    required this.year,
  });

  /// BGG game numeric identifier.
  final int id;

  /// Primary game name.
  final String name;

  /// Year published, or empty string when unknown.
  final String year;
}

// ---------------------------------------------------------------------------
// EAN / UPC validation
// ---------------------------------------------------------------------------

/// Returns true if [ean] is a valid EAN-13 product code (not an ISBN).
///
/// Validates the EAN-13 checksum and rejects the ISBN prefixes "978" and
/// "979". Returns false for strings that are not exactly 13 digits, contain
/// non-digit characters, fail the check digit, or start with an ISBN prefix.
bool isProductEan(String ean) {
  if(ean.length != 13) return false;
  var check = 0;
  for(var i = 0; i < 11; i += 2) {
    final d1 = int.tryParse(ean[i]);
    final d2 = int.tryParse(ean[i + 1]);
    if(d1 == null || d2 == null) return false;
    check += d1 + 3 * d2;
  }
  final last = int.tryParse(ean[12]);
  if(last == null) return false;
  if((check + last) % 10 != 0) return false;
  return !ean.startsWith('978') && !ean.startsWith('979');
}

/// Returns true if [upc] is a valid 12-digit UPC-A barcode.
///
/// Prepends "0" to [upc] and validates the result as a product EAN-13 via
/// [isProductEan].
bool isUpcA(String upc) {
  if(upc.length != 12) return false;
  return isProductEan('0$upc');
}

// ---------------------------------------------------------------------------
// String helpers (private)
// ---------------------------------------------------------------------------

/// Converts an inverted MARC21 personal name to natural order.
///
/// MARC21 stores personal names as "LastName, FirstName". This function
/// converts them to "FirstName LastName". Names without a comma are returned
/// unchanged.
String _invertedToNatural(String s) {
  final comma = s.indexOf(',');
  if(comma > 0) {
    return '${s.substring(comma + 1).trim()} ${s.substring(0, comma).trim()}';
  }
  return s;
}

/// Decodes common HTML entities and strips all HTML tags from [html].
String _stripHtml(String html) {
  var s = html
      .replaceAll('&amp;', '&')
      .replaceAll('&lt;', '<')
      .replaceAll('&gt;', '>')
      .replaceAll('&quot;', '"')
      .replaceAll('&#10;', '\n')
      .replaceAll('&mdash;', '—')
      .replaceAll('&ndash;', '–')
      .replaceAll('&rsquo;', '’')
      .replaceAll('&lsquo;', '‘')
      .replaceAll('&rdquo;', '”')
      .replaceAll('&ldquo;', '“');
  s = s.replaceAll(RegExp(r'<[^>]*>'), '');
  return s.trim();
}

// ---------------------------------------------------------------------------
// MusicBrainz JSON parser
// ---------------------------------------------------------------------------

/// Parses one MusicBrainz release object into AV media metadata.
///
/// Extracts title, artist credits (each name joined with its joinphrase),
/// label name, catalog number, publication date, and a Cover Art Archive
/// URL built from the release MBID.
EanLookupResult musicBrainzReleaseToResult(Map<String, dynamic> rel) {
  final result = EanLookupResult();

  final title = ((rel['title'] as String?) ?? '').trim();
  if(title.isNotEmpty) result.name = title;

  // Artist credits: concatenate each name with its joinphrase.
  final credits = (rel['artist-credit'] as List<dynamic>?) ?? [];
  final artistBuf = StringBuffer();
  for(final credit in credits) {
    final c = credit as Map<String, dynamic>;
    var creditName = ((c['name'] as String?) ?? '').trim();
    if(creditName.isEmpty) {
      creditName = (((c['artist'] as Map<String, dynamic>?)?['name'])
              as String? ??
          '').trim();
    }
    final join = (c['joinphrase'] as String?) ?? '';
    if(creditName.isNotEmpty) {
      artistBuf.write(creditName);
      artistBuf.write(join);
    }
  }
  final artistStr = artistBuf.toString().trim();
  if(artistStr.isNotEmpty) result.artists = [artistStr];

  // Label info: first entry provides publisher name and catalog number.
  final linfo = (rel['label-info'] as List<dynamic>?) ?? [];
  if(linfo.isNotEmpty) {
    final first = linfo[0] as Map<String, dynamic>;
    final label =
        (((first['label'] as Map<String, dynamic>?)?['name']) as String? ??
            '').trim();
    if(label.isNotEmpty) result.publisher = label;
    final catno = ((first['catalog-number'] as String?) ?? '').trim();
    if(catno.isNotEmpty) result.publisherNumber = catno;
  }

  final date = normalizeDate((rel['date'] as String?) ?? '');
  if(date.isNotEmpty) result.publicationDate = date;

  // Build a Cover Art Archive URL from the release MBID.
  final mbid = ((rel['id'] as String?) ?? '').trim();
  if(mbid.isNotEmpty) {
    result.coverUrl =
        'https://coverartarchive.org/release/$mbid/front-250';
  }
  return result;
}

/// Parses a MusicBrainz release search JSON response.
///
/// Delegates the first release in [json] to [musicBrainzReleaseToResult].
///
/// Returns an empty [EanLookupResult] on parse error or empty response.
EanLookupResult musicBrainzToResult(String json) {
  if(json.isEmpty) return EanLookupResult();
  try {
    final root = jsonDecode(json) as Map<String, dynamic>;
    final releases = (root['releases'] as List<dynamic>?) ?? [];
    if(releases.isEmpty) return EanLookupResult();
    return musicBrainzReleaseToResult(releases[0] as Map<String, dynamic>);
  } catch (_) {
    return EanLookupResult();
  }
}

// ---------------------------------------------------------------------------
// SRU MARC21 AV parser
// ---------------------------------------------------------------------------

/// Parses one MARC21 record element into AV media metadata.
///
/// Handles CDs, DVDs, Blu-rays, and board games. Board games are detected
/// when MARC21 field 653 contains the keyword "spiel" or "game"
/// (case-insensitive).
///
/// MARC21 personal names in MARC21 inverted order ("Last, First") are
/// converted to natural order ("First Last") by [_invertedToNatural].
EanLookupResult marc21RecordToAvResult(XmlElement marc) {
  final result = EanLookupResult();
  try {
    for(final df in marc.childElements) {
      if(localName(df) != 'datafield') continue;
      final tag = df.getAttribute('tag') ?? '';

      if(tag == '028') {
        final num = stripMarc(subfield(df, 'a'));
        if(num.isNotEmpty && result.publisherNumber.isEmpty) {
          result.publisherNumber = num;
        }
        final pub = stripMarc(subfield(df, 'b'));
        if(pub.isNotEmpty && result.publisher.isEmpty) {
          result.publisher = pub;
        }
      } else if(tag == '041') {
        final lang = stripMarc(subfield(df, 'a'));
        if(lang.isNotEmpty && result.language.isEmpty) {
          result.language = lang;
        }
      } else if(tag == '100') {
        if(result.artists.isEmpty) {
          final n = _invertedToNatural(stripMarc(subfield(df, 'a')));
          if(n.isNotEmpty) result.artists = [n];
        }
      } else if(tag == '245') {
        var title = stripMarc(subfield(df, 'a'));
        var part = stripMarc(subfield(df, 'n'));
        final slashIdx = part.indexOf(' /');
        if(slashIdx >= 0) part = stripMarc(part.substring(0, slashIdx));
        if(part.isNotEmpty) title += ': $part';
        if(title.isNotEmpty && result.name.isEmpty) result.name = title;
      } else if(tag == '264' && df.getAttribute('ind2') == '1') {
        final pub = stripMarc(subfield(df, 'b'));
        if(pub.isNotEmpty && result.publisher.isEmpty) {
          result.publisher = pub;
        }
        final date = stripMarc(subfield(df, 'c'));
        if(date.isNotEmpty) result.publicationDate = normalizeDate(date);
      } else if(tag == '260') {
        if(result.publisher.isEmpty) {
          final pub = stripMarc(subfield(df, 'b'));
          if(pub.isNotEmpty) result.publisher = pub;
        }
        if(result.publicationDate.isEmpty) {
          final date = stripMarc(subfield(df, 'c'));
          if(date.isNotEmpty) result.publicationDate = normalizeDate(date);
        }
      } else if(tag == '300') {
        final ext = stripMarc(subfield(df, 'a'));
        if(ext.isNotEmpty && result.extent.isEmpty) result.extent = ext;
      } else if(tag == '336') {
        final ct = stripMarc(subfield(df, 'a'));
        if(ct.isNotEmpty && result.contentType.isEmpty) {
          result.contentType = ct;
        }
      } else if(tag == '337') {
        final mt = stripMarc(subfield(df, 'a'));
        if(mt.isNotEmpty && result.mediaType.isEmpty) {
          result.mediaType = mt;
        }
      } else if(tag == '338') {
        final ct = stripMarc(subfield(df, 'a'));
        if(ct.isNotEmpty && result.carrierType.isEmpty) {
          result.carrierType = ct;
        }
      } else if(tag == '500') {
        final note = stripMarc(subfield(df, 'a'));
        if(note.isNotEmpty && result.description.isEmpty) {
          result.description = note;
        }
      } else if(tag == '508') {
        final credits = stripMarc(subfield(df, 'a'));
        if(credits.isNotEmpty && result.director.isEmpty) {
          result.director = credits;
        }
      } else if(tag == '511') {
        final cast = stripMarc(subfield(df, 'a'));
        if(cast.isNotEmpty) {
          result.participants = result.participants.isEmpty
              ? cast
              : '${result.participants}; $cast';
        }
      } else if(tag == '520') {
        final summary = stripMarc(subfield(df, 'a'));
        if(summary.isNotEmpty && result.description.isEmpty) {
          result.description = summary;
        }
      } else if(tag == '521') {
        final audience = stripMarc(subfield(df, 'a'));
        if(audience.isNotEmpty && result.targetAudience.isEmpty) {
          result.targetAudience = audience;
        }
      } else if(tag == '653') {
        if(!result.isGame) {
          final kw = subfield(df, 'a').toLowerCase();
          if(kw.contains('spiel') || kw.contains('game')) {
            result.isGame = true;
          }
        }
      } else if(tag == '700') {
        final n = _invertedToNatural(stripMarc(subfield(df, 'a')));
        final role = stripMarc(subfield(df, 'e')).toLowerCase();
        if(n.isEmpty) continue;
        if(role.contains('regisseur')) {
          result.director = result.director.isEmpty
              ? n
              : '${result.director}; $n';
        } else if(role.contains('schauspieler') ||
                  role.contains('darsteller') ||
                  role.contains('sprecher')) {
          result.participants = result.participants.isEmpty
              ? n
              : '${result.participants}; $n';
        } else {
          result.artists.add(n);
        }
      }
    }
  } catch (_) {}
  return result;
}

/// Parses a MARC21-xml or marcxml SRU response and returns AV media metadata.
///
/// Delegates the first MARC21 record to [marc21RecordToAvResult]. Both
/// MARC21-xml (DNB) and marcxml (K10plus, LoC) namespace conventions are
/// supported via [localName].
///
/// Returns an empty [EanLookupResult] on parse error or missing records.
EanLookupResult sruMarcToAvResult(String xml) {
  final records = sruMarcRecords(xml);
  if(records.isEmpty) return EanLookupResult();
  return marc21RecordToAvResult(records.first);
}

// ---------------------------------------------------------------------------
// OpenGTINDB text/plain parser
// ---------------------------------------------------------------------------

/// Parses an opengtindb.org text/plain key=value response.
///
/// [data] must be the raw response string decoded as ISO-8859-1 (Latin-1)
/// since opengtindb.org returns accented characters in that encoding. Use
/// [dart:convert]'s [latin1] codec to decode raw HTTP response bytes.
///
/// Returns an empty [EanLookupResult] when [data] is empty or the `error`
/// field is non-zero.
EanLookupResult openGtinDbToResult(String data) {
  final result = EanLookupResult();
  if(data.isEmpty) return result;

  final fields = <String, String>{};
  for(final line in data.split('\n')) {
    final eq = line.indexOf('=');
    if(eq > 0) {
      fields[line.substring(0, eq).trim()] =
          line.substring(eq + 1).trim();
    }
  }

  final err = fields['error'] ?? '';
  if(err.isNotEmpty && err != '0') return result;

  var name = (fields['detailname'] ?? '').trim();
  if(name.isEmpty) name = (fields['name'] ?? '').trim();
  // Strip trailing parenthetical manufacturer / article codes.
  name = name.replaceAll(RegExp(r'\s*\([^)]*\)\s*$'), '');
  if(name.isNotEmpty) result.name = name;

  var descr = (fields['descr'] ?? '').trim();
  if(descr.isEmpty) descr = (fields['descr_en'] ?? '').trim();
  if(descr.isNotEmpty) result.description = descr;

  final vendor = (fields['vendor'] ?? '').trim();
  if(vendor.isNotEmpty) result.publisher = vendor;

  return result;
}

// ---------------------------------------------------------------------------
// BGG XML parsers
// ---------------------------------------------------------------------------

/// Parses a BGG XML API2 search response.
///
/// Extracts game ID, primary name, and year from each `item` element.
/// Items without a primary name or valid numeric ID are skipped. Returns
/// results sorted alphabetically by name. Returns an empty list on parse
/// error or empty response.
List<BggSearchResult> bggSearchToResults(String xml) {
  final results = <BggSearchResult>[];
  if(xml.isEmpty) return results;
  try {
    final doc = XmlDocument.parse(xml);
    final root = doc.rootElement;
    for(final item in root.childElements) {
      if(item.name.local != 'item') continue;
      final idStr = item.getAttribute('id') ?? '';
      final id = int.tryParse(idStr);
      if(id == null || id <= 0) continue;

      String gameName = '';
      for(final nameEl in item.childElements) {
        if(nameEl.name.local == 'name' &&
           nameEl.getAttribute('type') == 'primary') {
          gameName = (nameEl.getAttribute('value') ?? '').trim();
          break;
        }
      }
      if(gameName.isEmpty) continue;

      String year = '';
      for(final el in item.childElements) {
        if(el.name.local == 'yearpublished') {
          year = (el.getAttribute('value') ?? '').trim();
          break;
        }
      }

      results.add(BggSearchResult(id: id, name: gameName, year: year));
    }
    results.sort((a, b) => a.name.compareTo(b.name));
  } catch (_) {}
  return results;
}

/// Parses a BGG XML API2 thing response and returns board game metadata.
///
/// Prefers a German alternate name (detected by the presence of ä, ö, ü, ß,
/// Ä, Ö, or Ü) over the primary English name. HTML entities in the
/// description field are decoded and HTML tags are stripped.
///
/// Returns an empty [EanLookupResult] on parse error or when no `item`
/// element is found.
EanLookupResult bggThingToResult(String xml) {
  final result = EanLookupResult();
  if(xml.isEmpty) return result;
  try {
    final doc = XmlDocument.parse(xml);
    final root = doc.rootElement;
    XmlElement? thing;
    for(final c in root.childElements) {
      if(c.name.local == 'item') {
        thing = c;
        break;
      }
    }
    if(thing == null) return result;
    final t = thing;

    // Helper: reads the value= attribute of the first child with [tagName].
    String intAttr(String tagName) {
      for(final el in t.childElements) {
        if(el.name.local == tagName) {
          return (el.getAttribute('value') ?? '').trim();
        }
      }
      return '';
    }

    // Names: prefer a German alternate (contains ä/ö/ü/ß/Ä/Ö/Ü).
    String primaryName = '';
    String germanName = '';
    for(final el in t.childElements) {
      if(el.name.local != 'name') continue;
      final val = (el.getAttribute('value') ?? '').trim();
      final type = el.getAttribute('type') ?? '';
      if(type == 'primary') {
        primaryName = val;
      } else if(type == 'alternate' &&
                val.isNotEmpty &&
                germanName.isEmpty &&
                RegExp(r'[äöüßÄÖÜ]').hasMatch(val)) {
        germanName = val;
      }
    }
    final title = germanName.isNotEmpty ? germanName : primaryName;
    if(title.isNotEmpty) result.name = title;

    final year = intAttr('yearpublished');
    if(year.isNotEmpty) result.publicationDate = year;
    final minP = intAttr('minplayers');
    if(minP.isNotEmpty) result.minPlayers = minP;
    final maxP = intAttr('maxplayers');
    if(maxP.isNotEmpty) result.maxPlayers = maxP;
    final age = intAttr('minage');
    if(age.isNotEmpty) result.minAge = age;
    final time = intAttr('playingtime');
    if(time.isNotEmpty) result.playingTime = time;

    // Description: decode HTML entities and strip HTML markup.
    for(final el in t.childElements) {
      if(el.name.local == 'description') {
        final desc = _stripHtml(el.innerText);
        if(desc.isNotEmpty) result.description = desc;
        break;
      }
    }

    // Cover image: prefer the small thumbnail over the full image.
    String thumbnail = '';
    String image = '';
    for(final el in t.childElements) {
      if(el.name.local == 'thumbnail') {
        thumbnail = el.innerText.trim();
      } else if(el.name.local == 'image') {
        image = el.innerText.trim();
      }
    }
    final cover = thumbnail.isNotEmpty ? thumbnail : image;
    if(cover.isNotEmpty) result.coverUrl = cover;

    // Links: first publisher wins; designers, artists, genres accumulate.
    var publisherSet = false;
    for(final link in t.childElements) {
      if(link.name.local != 'link') continue;
      final type = link.getAttribute('type') ?? '';
      final val = stripMarc((link.getAttribute('value') ?? '').trim());
      if(val.isEmpty) continue;
      if(type == 'boardgamepublisher' && !publisherSet) {
        result.publisher = val;
        publisherSet = true;
      } else if(type == 'boardgamedesigner') {
        result.designers.add(val);
      } else if(type == 'boardgameartist') {
        result.artists.add(val);
      } else if(type == 'boardgamecategory') {
        result.genres.add(val);
      }
    }
  } catch (_) {}
  return result;
}

// ---------------------------------------------------------------------------
// Internal HTTP helpers
// ---------------------------------------------------------------------------

Dio _makeDio(int timeoutMs) => Dio(BaseOptions(
      connectTimeout: Duration(milliseconds: timeoutMs),
      receiveTimeout: Duration(milliseconds: timeoutMs),
    ));

/// Builds a SRU query URL for an EAN lookup using [SruServer.eanField].
String _eanSruQueryUrl(SruServer server, String ean) =>
    '${server.endpoint}'
    '?version=1.1&operation=searchRetrieve'
    '&maximumRecords=1'
    '&query=${server.eanField}%3D$ean'
    '&recordSchema=${server.recordSchema}';

Future<EanLookupResult> _fetchMusicBrainz(String ean, Dio dio) async {
  debugPrint('[ean] → MusicBrainz barcode:$ean');
  try {
    final response = await dio.get<String>(
      'https://musicbrainz.org/ws/2/release',
      queryParameters: {
        'query': 'barcode:$ean',
        'fmt': 'json',
        'limit': '5',
      },
      options: Options(headers: {
        'User-Agent': 'Colman/1.0 (school library management software)',
      }),
    );
    final result = musicBrainzToResult(response.data ?? '');
    debugPrint('[ean] ← MusicBrainz: '
        '${result.isEmpty ? "empty" : "'${result.name}'"}');
    return result;
  } catch (e) {
    debugPrint('[ean] ← MusicBrainz: ERROR $e');
    return EanLookupResult();
  }
}

Future<EanLookupResult> _fetchSruEan(
  SruServer server,
  String ean,
  Dio dio,
) async {
  try {
    final url = _eanSruQueryUrl(server, ean);
    debugPrint('[ean] → SRU "${server.name}" $url');
    final response = await dio.get<String>(url);
    final data = response.data ?? '';
    if(data.isEmpty) {
      debugPrint('[ean] ← SRU "${server.name}": empty response');
      return EanLookupResult();
    }
    final result = sruMarcToAvResult(data);
    debugPrint('[ean] ← SRU "${server.name}": '
        '${result.isEmpty ? "no record" : "'${result.name}'"}');
    return result;
  } catch (e) {
    debugPrint('[ean] ← SRU "${server.name}": ERROR $e');
    return EanLookupResult();
  }
}

Future<EanLookupResult> _fetchOpenGtindb(
  String ean,
  String userid,
  Dio dio,
) async {
  debugPrint('[ean] → OpenGTINDB ean=$ean userid=$userid');
  try {
    final response = await dio.get<List<int>>(
      'https://opengtindb.org/',
      queryParameters: {'ean': ean, 'cmd': 'query', 'queryid': userid},
      // Request raw bytes to allow manual Latin-1 decoding.
      options: Options(responseType: ResponseType.bytes),
    );
    final bytes = response.data;
    if(bytes == null || bytes.isEmpty) {
      debugPrint('[ean] ← OpenGTINDB: empty response');
      return EanLookupResult();
    }
    final text = latin1.decode(bytes, allowInvalid: true);
    final result = openGtinDbToResult(text);
    debugPrint('[ean] ← OpenGTINDB: '
        '${result.isEmpty ? "empty" : "'${result.name}'"}');
    return result;
  } catch (e) {
    debugPrint('[ean] ← OpenGTINDB: ERROR $e');
    return EanLookupResult();
  }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Performs parallel EAN lookups against all enabled internet services.
///
/// Queries MusicBrainz (when [InternetServicesSettings.useMusicbrainz] is
/// true) and all enabled SRU servers with a non-empty [SruServer.eanField]
/// in parallel via [Future.wait]. Results from all sources are merged; the
/// first non-empty value wins for scalars.
///
/// Returns an empty [EanLookupResult] when no service is enabled or all
/// queries fail.
Future<EanLookupResult> queryEan(
  String ean,
  InternetServicesSettings settings,
) async {
  final dio = _makeDio(settings.timeoutMs);
  final futures = <Future<EanLookupResult>>[];

  if(settings.useMusicbrainz) {
    futures.add(_fetchMusicBrainz(ean, dio));
  }
  for(final srv in settings.sruServers) {
    if(srv.enabled && srv.eanField.isNotEmpty) {
      futures.add(_fetchSruEan(srv, ean, dio));
    }
  }

  debugPrint('[ean] queryEan $ean — ${futures.length} service(s)');
  if(futures.isEmpty) return EanLookupResult();

  final partials = await Future.wait(futures);
  final merged = EanLookupResult();
  for(final p in partials) {
    merged.merge(p);
  }
  debugPrint('[ean] queryEan merged: '
      '${merged.isEmpty ? "empty" : "'${merged.name}'"}');
  return merged;
}

/// Queries opengtindb.org for product metadata associated with [ean].
///
/// [userid] is the opengtindb.org query ID required for API access.
/// [timeoutMs] controls the network timeout in milliseconds. The returned
/// [EanLookupResult.name] is useful as a search term for a subsequent BGG
/// board-game lookup via [searchBgg].
///
/// Returns an empty [EanLookupResult] on network error or invalid response.
Future<EanLookupResult> queryOpenGtindb(
  String ean,
  String userid,
  int timeoutMs,
) async {
  final dio = _makeDio(timeoutMs);
  return _fetchOpenGtindb(ean, userid, dio);
}

/// Searches BoardGameGeek for board games matching [name].
///
/// [token] is an optional Bearer authentication token; pass an empty string
/// for unauthenticated access. [timeoutMs] controls the network timeout in
/// milliseconds. Returns a list of matches sorted alphabetically by name.
///
/// Returns an empty list on network error or parse failure.
Future<List<BggSearchResult>> searchBgg(
  String name,
  String token,
  int timeoutMs,
) async {
  debugPrint('[ean] → BGG search: "$name"');
  final dio = _makeDio(timeoutMs);
  try {
    final opts = token.isNotEmpty
        ? Options(headers: {'Authorization': 'Bearer $token'})
        : null;
    final response = await dio.get<String>(
      // No "www." — the redirect to the apex domain would make Dart's
      // HTTP client strip the Authorization header (cross-host redirect).
      'https://boardgamegeek.com/xmlapi2/search',
      queryParameters: {'query': name, 'type': 'boardgame'},
      options: opts,
    );
    final results = bggSearchToResults(response.data ?? '');
    debugPrint('[ean] ← BGG search: ${results.length} result(s)');
    return results;
  } catch (e) {
    debugPrint('[ean] ← BGG search: ERROR $e');
    return [];
  }
}

/// Fetches and parses a BoardGameGeek game detail record for game [id].
///
/// Retries up to three times with a two-second delay when the response
/// begins with `<message>`, which BGG returns while still processing the
/// request (HTTP 202-style). [token] is an optional Bearer authentication
/// token; pass an empty string for unauthenticated access.
///
/// Returns an empty [EanLookupResult] on network error or exhausted retries.
Future<EanLookupResult> queryBggThing(
  int id,
  String token,
  int timeoutMs,
) async {
  debugPrint('[ean] → BGG thing id=$id');
  final dio = _makeDio(timeoutMs);
  final opts = token.isNotEmpty
      ? Options(headers: {'Authorization': 'Bearer $token'})
      : null;
  for(var attempt = 0; attempt < 3; attempt++) {
    try {
      final response = await dio.get<String>(
        // No "www." — see searchBgg.
        'https://boardgamegeek.com/xmlapi2/thing',
        queryParameters: {'id': '$id'},
        options: opts,
      );
      final data = response.data ?? '';
      if(!data.trimLeft().startsWith('<message>')) {
        final result = bggThingToResult(data);
        debugPrint('[ean] ← BGG thing id=$id: '
            '${result.isEmpty ? "empty" : "'${result.name}'"}');
        return result;
      }
      debugPrint('[ean] BGG thing id=$id: <message> response, '
          'retry ${attempt + 1}/3');
      // BGG is still processing — wait before retrying.
      await Future.delayed(const Duration(seconds: 2));
    } catch (e) {
      debugPrint('[ean] ← BGG thing id=$id: ERROR $e');
      return EanLookupResult();
    }
  }
  debugPrint('[ean] ← BGG thing id=$id: retries exhausted');
  return EanLookupResult();
}
