// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart' show debugPrint;
import 'package:xml/xml.dart';

import '../models/internet_services_settings.dart';
import 'ean_lookup.dart';
import 'isbn_lookup.dart';
import 'marc_xml.dart';

// ---------------------------------------------------------------------------
// Media kinds
// ---------------------------------------------------------------------------

/// Media kind selectable in the title search dialog.
enum MediaKind { book, cd, dvd, bluray, boardGame }

/// Media kind detected in a single MARC21 record.
///
/// Richer than [MediaKind]: [video] stands for video records that cannot
/// be narrowed down to DVD or Blu-ray.
enum DetectedMediaKind { unknown, book, cd, video, dvd, bluray }

// ---------------------------------------------------------------------------
// TitleSearchResult
// ---------------------------------------------------------------------------

/// One entry from a multi-source title search.
sealed class TitleSearchResult {
  const TitleSearchResult({
    required this.name,
    required this.year,
    required this.source,
    this.coverUrl,
  });

  /// Result title shown in the list.
  final String name;

  /// Four-digit publication year, or empty string when unknown.
  final String year;

  /// Source label, e.g. "Open Library", "MusicBrainz", or a server name.
  final String source;

  /// Cover image URL, or null when no cover is available.
  final String? coverUrl;
}

/// A book hit (Open Library, Google Books, or an SRU book record).
final class BookTitleResult extends TitleSearchResult {
  const BookTitleResult({
    required super.name,
    required super.year,
    required super.source,
    super.coverUrl,
    required this.data,
  });

  /// Full book metadata for prefilling the item form.
  final BookLookupResult data;
}

/// An AV media hit (MusicBrainz release or an SRU AV record).
final class AvTitleResult extends TitleSearchResult {
  const AvTitleResult({
    required super.name,
    required super.year,
    required super.source,
    super.coverUrl,
    required this.data,
  });

  /// Full AV media metadata for prefilling the item form.
  final EanLookupResult data;
}

/// A board game hit from the BoardGameGeek text search.
///
/// Carries only the BGG ID; callers must fetch the full record with
/// [queryBggThing] before adopting the result (two-stage flow).
final class BggTitleResult extends TitleSearchResult {
  const BggTitleResult({
    required super.name,
    required super.year,
    required super.source,
    required this.bggId,
  });

  /// BGG game identifier for the detail fetch.
  final int bggId;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Extracts the four-digit year from a normalised publication date.
///
/// [date] is a string as produced by [normalizeDate] ("yyyy", "M.yyyy",
/// or "d.M.yyyy"). Returns an empty string when no year is present.
String yearFromDate(String date) {
  final m = RegExp(r'(\d{4})\s*$').firstMatch(date);
  return m?.group(1) ?? '';
}

// ---------------------------------------------------------------------------
// Media kind detection (MARC21)
// ---------------------------------------------------------------------------

/// Detects the media kind of one MARC21 record.
///
/// Uses the leader (position 06), controlfield 007 (positions 00 and 04),
/// and the carrier/extent text of datafields 300 $a and 338 $a.
/// Returns [DetectedMediaKind.unknown] when no signal is present.
DetectedMediaKind detectMarc21Kind(XmlElement marc) {
  var kind = DetectedMediaKind.unknown;
  var carrierText = '';
  for(final el in marc.childElements) {
    final local = localName(el);
    if(local == 'leader') {
      final ldr = el.innerText;
      if(ldr.length > 6 && kind == DetectedMediaKind.unknown) {
        final t = ldr[6];
        if(t == 'a' || t == 't') {
          kind = DetectedMediaKind.book;
        } else if(t == 'j' || t == 'i') {
          kind = DetectedMediaKind.cd;
        } else if(t == 'g') {
          kind = DetectedMediaKind.video;
        }
      }
    } else if(local == 'controlfield' && el.getAttribute('tag') == '007') {
      final cf = el.innerText;
      if(cf.isEmpty) continue;
      if(cf[0] == 'v') {
        if(kind == DetectedMediaKind.unknown) {
          kind = DetectedMediaKind.video;
        }
        if(cf.length > 4 && kind == DetectedMediaKind.video) {
          // Videorecording format: 'v' = DVD, 's' = Blu-ray.
          if(cf[4] == 'v') {
            kind = DetectedMediaKind.dvd;
          } else if(cf[4] == 's') {
            kind = DetectedMediaKind.bluray;
          }
        }
      } else if(cf[0] == 's' && kind == DetectedMediaKind.unknown) {
        kind = DetectedMediaKind.cd;
      }
    } else if(local == 'datafield') {
      final tag = el.getAttribute('tag') ?? '';
      if(tag == '300' || tag == '338') {
        carrierText += ' ${subfield(el, 'a').toLowerCase()}';
      }
    }
  }
  if(kind == DetectedMediaKind.video) {
    if(carrierText.contains('blu-ray') || carrierText.contains('bluray')) {
      kind = DetectedMediaKind.bluray;
    } else if(carrierText.contains('dvd')) {
      kind = DetectedMediaKind.dvd;
    }
  }
  return kind;
}

/// Returns true when a record of [detected] kind matches [requested].
///
/// Undetectable records match every request; generic video records match
/// both DVD and Blu-ray requests. [MediaKind.boardGame] never matches
/// MARC records (board games are searched on BoardGameGeek only).
bool kindMatches(DetectedMediaKind detected, MediaKind requested) {
  switch(detected) {
    case DetectedMediaKind.unknown:
      return true;
    case DetectedMediaKind.book:
      return requested == MediaKind.book;
    case DetectedMediaKind.cd:
      return requested == MediaKind.cd;
    case DetectedMediaKind.video:
      return requested == MediaKind.dvd || requested == MediaKind.bluray;
    case DetectedMediaKind.dvd:
      return requested == MediaKind.dvd;
    case DetectedMediaKind.bluray:
      return requested == MediaKind.bluray;
  }
}

// ---------------------------------------------------------------------------
// Per-source search parsers
// ---------------------------------------------------------------------------

/// Parses a decoded Open Library search.json response into title results.
///
/// Builds one shallow result per `docs[]` entry: title, authors, first
/// publication year, first publisher, and cover URL. Deliberately stores
/// no ISBN, because the search documents mix the ISBNs of all editions.
/// Untitled entries are skipped.
List<BookTitleResult> openlibrarySearchToResults(Map<String, dynamic>? root) {
  final results = <BookTitleResult>[];
  if(root == null) return results;
  for(final entry in (root['docs'] as List<dynamic>?) ?? []) {
    final doc = entry as Map<String, dynamic>;
    final title = ((doc['title'] as String?) ?? '').trim();
    if(title.isEmpty) continue;

    final data = BookLookupResult();
    data.name = title;
    data.authors = ((doc['author_name'] as List<dynamic>?) ?? [])
        .cast<String>()
        .where((n) => n.trim().isNotEmpty)
        .toList();
    final year = (doc['first_publish_year'] as int?) ?? 0;
    if(year > 0) data.publicationDate = '$year';
    final publishers = (doc['publisher'] as List<dynamic>?) ?? [];
    if(publishers.isNotEmpty) {
      data.publisher = (publishers.first as String).trim();
    }
    final coverId = (doc['cover_i'] as int?) ?? 0;
    if(coverId > 0) {
      data.coverUrl = 'https://covers.openlibrary.org/b/id/$coverId-M.jpg';
    }

    results.add(BookTitleResult(
      name: title,
      year: year > 0 ? '$year' : '',
      source: 'Open Library',
      coverUrl: data.coverUrl,
      data: data,
    ));
  }
  return results;
}

/// Parses a decoded Google Books volumes response into title results.
///
/// Builds one result per volume via [googlebooksVolumeToResult];
/// untitled volumes are skipped.
List<BookTitleResult> googlebooksSearchToResults(Map<String, dynamic>? root) {
  final results = <BookTitleResult>[];
  if(root == null) return results;
  for(final it in (root['items'] as List<dynamic>?) ?? []) {
    final vol =
        (it as Map<String, dynamic>)['volumeInfo'] as Map<String, dynamic>?;
    if(vol == null) continue;
    final data = googlebooksVolumeToResult(vol);
    if(data.name.isEmpty) continue;
    results.add(BookTitleResult(
      name: data.name,
      year: yearFromDate(data.publicationDate),
      source: 'Google Books',
      coverUrl: data.coverUrl,
      data: data,
    ));
  }
  return results;
}

/// Parses a MusicBrainz release search JSON response into title results.
///
/// Builds one result per release via [musicBrainzReleaseToResult];
/// untitled releases are skipped.
List<AvTitleResult> musicbrainzSearchToResults(String json) {
  final results = <AvTitleResult>[];
  if(json.isEmpty) return results;
  try {
    final root = jsonDecode(json) as Map<String, dynamic>;
    for(final entry in (root['releases'] as List<dynamic>?) ?? []) {
      final data =
          musicBrainzReleaseToResult(entry as Map<String, dynamic>);
      if(data.name.isEmpty) continue;
      results.add(AvTitleResult(
        name: data.name,
        year: yearFromDate(data.publicationDate),
        source: 'MusicBrainz',
        coverUrl: data.coverUrl,
        data: data,
      ));
    }
  } catch (_) {}
  return results;
}

/// Parses a multi-record SRU title response, filtered by media kind.
///
/// Keeps only records whose detected media kind matches [kind]; records
/// whose kind cannot be detected are kept as well, and generic video
/// records match both [MediaKind.dvd] and [MediaKind.bluray]. Untitled
/// records are dropped. For UNIMARC servers only [MediaKind.book] is
/// supported; other kinds yield an empty result.
List<TitleSearchResult> sruTitleToResults(
  SruServer server,
  String xml,
  MediaKind kind,
) {
  final results = <TitleSearchResult>[];
  if(server.isUnimarc) {
    if(kind != MediaKind.book) return results;
    for(final marc in sruMarcRecords(xml)) {
      final data = unimarcRecordToBookResult(marc);
      if(data.name.isEmpty) continue;
      results.add(BookTitleResult(
        name: data.name,
        year: yearFromDate(data.publicationDate),
        source: server.name,
        data: data,
      ));
    }
    return results;
  }
  for(final marc in sruMarcRecords(xml)) {
    if(!kindMatches(detectMarc21Kind(marc), kind)) continue;
    if(kind == MediaKind.book) {
      final data = marc21RecordToBookResult(marc);
      if(data.name.isEmpty) continue;
      results.add(BookTitleResult(
        name: data.name,
        year: yearFromDate(data.publicationDate),
        source: server.name,
        data: data,
      ));
    } else {
      final data = marc21RecordToAvResult(marc);
      if(data.name.isEmpty) continue;
      results.add(AvTitleResult(
        name: data.name,
        year: yearFromDate(data.publicationDate),
        source: server.name,
        data: data,
      ));
    }
  }
  return results;
}

// ---------------------------------------------------------------------------
// SRU title query URL
// ---------------------------------------------------------------------------

/// Builds the SRU title-search URL for [server].
///
/// The title is sent as a percent-encoded, quoted CQL phrase so that
/// spaces and non-ASCII characters cannot break the CQL query. UNIMARC
/// servers (BnF) reject SRU version 1.1 and are queried with version
/// 1.2; all other servers (e.g. DNB) only accept version 1.1.
String sruTitleQueryUrl(SruServer server, String title,
    {int maxRecords = 10}) {
  final version = server.isUnimarc ? '1.2' : '1.1';
  return '${server.endpoint}'
      '?version=$version&operation=searchRetrieve'
      '&maximumRecords=$maxRecords'
      '&query=${server.titleField}%3D%22'
      '${Uri.encodeComponent(title.trim())}%22'
      '&recordSchema=${server.recordSchema}';
}

// ---------------------------------------------------------------------------
// Availability
// ---------------------------------------------------------------------------

bool _sruTitleSearchAvailable(InternetServicesSettings s, MediaKind kind) {
  for(final srv in s.sruServers) {
    if(srv.enabled &&
       srv.titleField.isNotEmpty &&
       (kind == MediaKind.book || !srv.isUnimarc)) {
      return true;
    }
  }
  return false;
}

/// Returns true when a title-search service is enabled for [kind].
bool titleSearchAvailableFor(MediaKind kind, InternetServicesSettings s) {
  switch(kind) {
    case MediaKind.book:
      return s.useOpenlibrary ||
          s.useGoogleapis ||
          _sruTitleSearchAvailable(s, kind);
    case MediaKind.cd:
      return s.useMusicbrainz || _sruTitleSearchAvailable(s, kind);
    case MediaKind.dvd:
    case MediaKind.bluray:
      return _sruTitleSearchAvailable(s, kind);
    case MediaKind.boardGame:
      return s.useBgg;
  }
}

/// Returns true when the title search is available for at least one kind.
bool titleSearchAvailable(InternetServicesSettings s) =>
    MediaKind.values.any((kind) => titleSearchAvailableFor(kind, s));

// ---------------------------------------------------------------------------
// Internal HTTP helpers
// ---------------------------------------------------------------------------

Dio _makeDio(int timeoutMs) => Dio(BaseOptions(
      connectTimeout: Duration(milliseconds: timeoutMs),
      receiveTimeout: Duration(milliseconds: timeoutMs),
    ));

Future<List<TitleSearchResult>> _fetchTitleOpenlibrary(
  String title,
  Dio dio,
) async {
  debugPrint('[title] → Open Library "$title"');
  try {
    final response = await dio.get<Map<String, dynamic>>(
      'https://openlibrary.org/search.json',
      queryParameters: {
        'title': title,
        'limit': '10',
        'fields': 'title,author_name,first_publish_year,cover_i,publisher',
      },
    );
    final results = openlibrarySearchToResults(response.data);
    debugPrint('[title] ← Open Library: ${results.length} hit(s)');
    return results;
  } catch (e) {
    debugPrint('[title] ← Open Library: ERROR $e');
    return [];
  }
}

Future<List<TitleSearchResult>> _fetchTitleGooglebooks(
  String title,
  String key,
  Dio dio,
) async {
  debugPrint('[title] → Google Books intitle:"$title"');
  try {
    final params = <String, String>{
      'q': 'intitle:"$title"',
      'maxResults': '10',
    };
    if(key.isNotEmpty) params['key'] = key;
    final response = await dio.get<Map<String, dynamic>>(
      'https://www.googleapis.com/books/v1/volumes',
      queryParameters: params,
    );
    final results = googlebooksSearchToResults(response.data);
    debugPrint('[title] ← Google Books: ${results.length} hit(s)');
    return results;
  } catch (e) {
    debugPrint('[title] ← Google Books: ERROR $e');
    return [];
  }
}

Future<List<TitleSearchResult>> _fetchTitleMusicbrainz(
  String title,
  Dio dio,
) async {
  debugPrint('[title] → MusicBrainz release:"$title"');
  try {
    final response = await dio.get<String>(
      'https://musicbrainz.org/ws/2/release',
      queryParameters: {
        'query': 'release:"$title"',
        'fmt': 'json',
        'limit': '10',
      },
      options: Options(headers: {
        'User-Agent': 'Colman/1.0 (school library management software)',
      }),
    );
    final results = musicbrainzSearchToResults(response.data ?? '');
    debugPrint('[title] ← MusicBrainz: ${results.length} hit(s)');
    return results;
  } catch (e) {
    debugPrint('[title] ← MusicBrainz: ERROR $e');
    return [];
  }
}

Future<List<TitleSearchResult>> _fetchTitleSru(
  SruServer server,
  String title,
  MediaKind kind,
  Dio dio,
) async {
  try {
    final url = sruTitleQueryUrl(server, title);
    debugPrint('[title] → SRU "${server.name}" $url');
    final response = await dio.get<String>(url);
    final data = response.data ?? '';
    if(data.isEmpty) {
      debugPrint('[title] ← SRU "${server.name}": empty response');
      return [];
    }
    final results = sruTitleToResults(server, data, kind);
    debugPrint('[title] ← SRU "${server.name}": ${results.length} hit(s)');
    return results;
  } catch (e) {
    debugPrint('[title] ← SRU "${server.name}": ERROR $e');
    return [];
  }
}

Future<List<TitleSearchResult>> _fetchTitleBgg(
  String title,
  InternetServicesSettings settings,
) async {
  final hits = await searchBgg(title, settings.bggToken, settings.timeoutMs);
  return hits
      .map((h) => BggTitleResult(
            name: h.name,
            year: h.year,
            source: 'BoardGameGeek',
            bggId: h.id,
          ))
      .toList();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Performs parallel title searches against all services for [kind].
///
/// Sources per kind: books → Open Library, Google Books, and SRU servers
/// with a title field (UNIMARC servers for books only); CDs → MusicBrainz
/// and SRU servers; DVDs/Blu-rays → SRU servers; board games →
/// BoardGameGeek. Per-source network errors are swallowed and logged.
///
/// The combined results keep the dedicated services first, followed by
/// the SRU servers in configured order. Returns an empty list when the
/// title is blank or no applicable service is enabled.
Future<List<TitleSearchResult>> queryTitle(
  String title,
  MediaKind kind,
  InternetServicesSettings settings,
) async {
  final query = title.trim();
  if(query.isEmpty) return [];

  if(kind == MediaKind.boardGame) {
    if(!settings.useBgg) return [];
    return _fetchTitleBgg(query, settings);
  }

  final dio = _makeDio(settings.timeoutMs);
  final futures = <Future<List<TitleSearchResult>>>[];

  if(kind == MediaKind.book && settings.useOpenlibrary) {
    futures.add(_fetchTitleOpenlibrary(query, dio));
  }
  if(kind == MediaKind.book && settings.useGoogleapis) {
    futures.add(_fetchTitleGooglebooks(query, settings.googleapisKey, dio));
  }
  if(kind == MediaKind.cd && settings.useMusicbrainz) {
    futures.add(_fetchTitleMusicbrainz(query, dio));
  }
  for(final srv in settings.sruServers) {
    if(srv.enabled &&
       srv.titleField.isNotEmpty &&
       (kind == MediaKind.book || !srv.isUnimarc)) {
      futures.add(_fetchTitleSru(srv, query, kind, dio));
    }
  }

  debugPrint('[title] queryTitle "$query" kind=$kind — '
      '${futures.length} service(s)');
  if(futures.isEmpty) return [];

  final partials = await Future.wait(futures);
  return [for(final p in partials) ...p];
}
