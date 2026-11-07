// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart' show debugPrint;
import 'package:intl/intl.dart';
import 'package:xml/xml.dart';

import '../models/internet_services_settings.dart';
import 'marc_xml.dart';

// ---------------------------------------------------------------------------
// BookLookupResult
// ---------------------------------------------------------------------------

/// Metadata returned by an ISBN internet lookup.
///
/// All fields default to empty / null. Merge multiple results with
/// [mergeFrom]: the first non-empty value wins, except for
/// [publicationDate] where the longest (most specific) string wins.
class BookLookupResult {
  String name = '';
  List<String> authors = [];
  String publisher = '';
  String publicationDate = '';
  String isbn10 = '';
  String isbn13 = '';
  String pages = '';
  String genre = '';
  String description = '';
  String ddc = '';
  String? coverUrl;

  bool get isEmpty =>
      name.isEmpty &&
      authors.isEmpty &&
      publisher.isEmpty &&
      publicationDate.isEmpty &&
      isbn10.isEmpty &&
      isbn13.isEmpty &&
      pages.isEmpty &&
      genre.isEmpty &&
      description.isEmpty &&
      ddc.isEmpty &&
      coverUrl == null;

  /// Merges [other] into this result: first non-empty value wins.
  ///
  /// Exception: [publicationDate] keeps the longest (most specific) string
  /// across both sources.
  void mergeFrom(BookLookupResult other) {
    if(name.isEmpty && other.name.isNotEmpty) name = other.name;
    if(authors.isEmpty && other.authors.isNotEmpty) {
      authors = List.of(other.authors);
    }
    if(publisher.isEmpty && other.publisher.isNotEmpty) {
      publisher = other.publisher;
    }
    if(other.publicationDate.length > publicationDate.length) {
      publicationDate = other.publicationDate;
    }
    if(isbn10.isEmpty && other.isbn10.isNotEmpty) isbn10 = other.isbn10;
    if(isbn13.isEmpty && other.isbn13.isNotEmpty) isbn13 = other.isbn13;
    if(pages.isEmpty && other.pages.isNotEmpty) pages = other.pages;
    if(genre.isEmpty && other.genre.isNotEmpty) genre = other.genre;
    if(description.isEmpty && other.description.isNotEmpty) {
      description = other.description;
    }
    if(ddc.isEmpty && other.ddc.isNotEmpty) ddc = other.ddc;
    coverUrl ??= other.coverUrl;
  }
}

// ---------------------------------------------------------------------------
// ISBN utility functions
// ---------------------------------------------------------------------------

/// Returns true if [s] is a syntactically valid ISBN-13.
bool isIsbn13(String s) {
  if(s.length != 13) return false;
  var check = 0;
  for(var i = 0; i < 11; i += 2) {
    final d1 = int.tryParse(s[i]);
    final d2 = int.tryParse(s[i + 1]);
    if(d1 == null || d2 == null) return false;
    check += d1 + 3 * d2;
  }
  final last = int.tryParse(s[12]);
  if(last == null) return false;
  return (check + last) % 10 == 0;
}

/// Returns true if [s] is a syntactically valid ISBN-10.
bool isIsbn10(String s) {
  if(s.length != 10) return false;
  var check = 0;
  for(var i = 0; i < 10; i++) {
    final isX = i == 9 && s[i] == 'X';
    final d = isX ? 10 : int.tryParse(s[i]);
    if(d == null) return false;
    check += d * (10 - i);
  }
  return check % 11 == 0;
}

/// Converts a valid ISBN-10 to its ISBN-13 equivalent.
String isbn10ToIsbn13(String isbn10) {
  assert(isIsbn10(isbn10));
  final buf = StringBuffer('978');
  buf.write(isbn10.substring(0, 9));
  final s = buf.toString();
  var check = 0;
  for(var i = 0; i < 11; i += 2) {
    check += int.parse(s[i]) + 3 * int.parse(s[i + 1]);
  }
  final digit = (10 - (check % 10)) % 10;
  return '$s$digit';
}

/// Converts a valid ISBN-13 with "978" prefix to its ISBN-10 equivalent.
///
/// Returns an empty string for non-978 prefixes.
String isbn13ToIsbn10(String isbn13) {
  assert(isIsbn13(isbn13));
  if(!isbn13.startsWith('978')) return '';
  final core = isbn13.substring(3, 12);
  var check = 0;
  for(var i = 0; i < 9; i++) {
    check += int.parse(core[i]) * (10 - i);
  }
  final digit = (11 - (check % 11)) % 11;
  final checkChar = digit == 10 ? 'X' : '$digit';
  return '$core$checkChar';
}

void _fillIsbn(String isbn, BookLookupResult result) {
  if(isIsbn13(isbn)) {
    if(result.isbn13.isEmpty) result.isbn13 = isbn;
    if(result.isbn10.isEmpty) result.isbn10 = isbn13ToIsbn10(isbn);
  } else if(isIsbn10(isbn)) {
    if(result.isbn10.isEmpty) result.isbn10 = isbn;
    if(result.isbn13.isEmpty) result.isbn13 = isbn10ToIsbn13(isbn);
  }
}

// ---------------------------------------------------------------------------
// Date normalisation
// ---------------------------------------------------------------------------

/// Normalises a raw publication-date string to "yyyy", "M.yyyy", or
/// "d.M.yyyy".
///
/// Handles ISO partial dates, common English long-form dates (via intl),
/// and MARC21 annotations such as "c2020.", "[2020]", "©2020".
String normalizeDate(String raw) {
  final s = raw.trim();
  if(s.isEmpty) return '';

  // ISO-like: yyyy[-M[-d]]
  final iso = RegExp(r'^(\d{4})(?:-(\d{1,2})(?:-(\d{1,2}))?)?$');
  final m = iso.firstMatch(s);
  if(m != null) {
    final year = m.group(1)!;
    final rawMonth = m.group(2);
    final rawDay = m.group(3);
    if(rawMonth == null) return year;
    final month = rawMonth.replaceFirst(RegExp(r'^0'), '');
    if(rawDay == null) return '$month.$year';
    final day = rawDay.replaceFirst(RegExp(r'^0'), '');
    return '$day.$month.$year';
  }

  // English long-form dates via intl (locale en_US)
  const patterns = [
    'MMMM d, yyyy',
    'd MMMM yyyy',
    'MMM d, yyyy',
    'd MMM yyyy',
    'MMMM yyyy',
    'MMM yyyy',
  ];
  for(final fmt in patterns) {
    try {
      final d = DateFormat(fmt, 'en_US').parseStrict(s);
      if(fmt == 'MMMM yyyy' || fmt == 'MMM yyyy') {
        return '${d.month}.${d.year}';
      }
      return '${d.day}.${d.month}.${d.year}';
    } catch (_) {}
  }

  // Fallback: first four-digit sequence
  final yearOnly = RegExp(r'(\d{4})');
  final ym = yearOnly.firstMatch(s);
  if(ym != null) return ym.group(1)!;

  return '';
}

// ---------------------------------------------------------------------------
// Open Library JSON parser
// ---------------------------------------------------------------------------

/// Parses a decoded Open Library Books API response.
///
/// [root] is the top-level JSON object returned by the API
/// (`bibkeys=ISBN:…&format=json&jscmd=data`).  Returns an empty
/// [BookLookupResult] when no matching entry is found or [root] is null.
BookLookupResult openlibraryToResult(Map<String, dynamic>? root) {
  final result = BookLookupResult();
  if(root == null || root.isEmpty) return result;
  final book = (root.values.first as Map<String, dynamic>?) ?? {};

  result.name = (book['title'] as String?) ?? '';

  final jauthors = (book['authors'] as List<dynamic>?) ?? [];
  result.authors = jauthors
      .map((a) => (a as Map<String, dynamic>)['name'] as String? ?? '')
      .where((n) => n.isNotEmpty)
      .toList();

  final jpubs = (book['publishers'] as List<dynamic>?) ?? [];
  result.publisher = jpubs
      .map((p) => (p as Map<String, dynamic>)['name'] as String? ?? '')
      .join(', ');

  final published = (book['publish_date'] as String?) ?? '';
  result.publicationDate = normalizeDate(published);

  final ids = (book['identifiers'] as Map<String, dynamic>?) ?? {};
  final isbn10List = (ids['isbn_10'] as List<dynamic>?) ?? [];
  final isbn13List = (ids['isbn_13'] as List<dynamic>?) ?? [];
  result.isbn10 = isbn10List.isNotEmpty ? isbn10List.first as String : '';
  result.isbn13 = isbn13List.isNotEmpty ? isbn13List.first as String : '';

  final pages = (book['number_of_pages'] as int?) ?? 0;
  if(pages > 0) result.pages = '$pages';

  final coverMedium =
      (book['cover'] as Map<String, dynamic>?)?['medium'] as String?;
  result.coverUrl = (coverMedium != null && coverMedium.isNotEmpty)
      ? coverMedium
      : null;

  final jdesc = book['description'];
  if(jdesc is String && jdesc.isNotEmpty) {
    result.description = jdesc;
  } else if(jdesc is Map<String, dynamic>) {
    final v = jdesc['value'] as String? ?? '';
    if(v.isNotEmpty) result.description = v;
  }

  final jddc = (book['dewey_decimal_class'] as List<dynamic>?) ?? [];
  if(jddc.isNotEmpty) result.ddc = jddc.first as String;

  final genres = (book['genres'] as List<dynamic>?) ?? [];
  result.genre = genres.cast<String>().join('; ');

  return result;
}

// ---------------------------------------------------------------------------
// Google Books JSON parser
// ---------------------------------------------------------------------------

/// Parses one Google Books volumeInfo object into book metadata.
///
/// Extracts title (with subtitle), authors, publisher, publication date,
/// per-edition ISBNs, page count, genre, description, and thumbnail URL
/// from a single volumeInfo object [vol].
BookLookupResult googlebooksVolumeToResult(Map<String, dynamic> vol) {
  final result = BookLookupResult();

  for(final id in (vol['industryIdentifiers'] as List<dynamic>?) ?? []) {
    final idMap = id as Map<String, dynamic>;
    final type = idMap['type'] as String? ?? '';
    final identifier = idMap['identifier'] as String? ?? '';
    if(type == 'ISBN_10') result.isbn10 = identifier;
    if(type == 'ISBN_13') result.isbn13 = identifier;
  }

  var title = (vol['title'] as String?) ?? '';
  final subtitle = (vol['subtitle'] as String?) ?? '';
  if(subtitle.isNotEmpty) title += ' : $subtitle';
  result.name = title;

  result.authors = ((vol['authors'] as List<dynamic>?) ?? [])
      .cast<String>()
      .where((n) => n.isNotEmpty)
      .toList();

  result.publisher = (vol['publisher'] as String?) ?? '';
  result.publicationDate =
      normalizeDate((vol['publishedDate'] as String?) ?? '');

  final pageCount = (vol['pageCount'] as int?) ?? 0;
  if(pageCount > 0) result.pages = '$pageCount';

  final cats = (vol['categories'] as List<dynamic>?) ?? [];
  result.genre = cats.cast<String>().join('; ');

  result.description = (vol['description'] as String?) ?? '';

  final thumb =
      (vol['imageLinks'] as Map<String, dynamic>?)?['thumbnail'] as String?;
  result.coverUrl = (thumb != null && thumb.isNotEmpty) ? thumb : null;

  return result;
}

/// Parses a decoded Google Books API volumes response.
///
/// [isbn] is the ISBN that was queried; the matching volume entry is
/// identified by its `industryIdentifiers`.  [root] is the top-level JSON
/// object.  Returns an empty [BookLookupResult] when no matching volume
/// is found or [root] is null.
BookLookupResult googlebooksToResult(String isbn, Map<String, dynamic>? root) {
  if(root == null) return BookLookupResult();
  final items = (root['items'] as List<dynamic>?) ?? [];
  for(final it in items) {
    final vol =
        (it as Map<String, dynamic>)['volumeInfo'] as Map<String, dynamic>?;
    if(vol == null) continue;
    final result = googlebooksVolumeToResult(vol);
    if(result.isbn10 == isbn || result.isbn13 == isbn) return result;
  }
  return BookLookupResult();
}

// ---------------------------------------------------------------------------
// Internal HTTP helpers
// ---------------------------------------------------------------------------

Dio _makeDio(int timeoutMs) => Dio(BaseOptions(
      connectTimeout: Duration(milliseconds: timeoutMs),
      receiveTimeout: Duration(milliseconds: timeoutMs),
    ));

Future<BookLookupResult> _fetchOpenlibrary(String isbn, Dio dio) async {
  debugPrint('[isbn] → Open Library ISBN:$isbn');
  try {
    final response = await dio.get<Map<String, dynamic>>(
      'https://openlibrary.org/api/books',
      queryParameters: {
        'bibkeys': 'ISBN:$isbn',
        'format': 'json',
        'jscmd': 'data',
      },
    );
    final result = openlibraryToResult(response.data);
    debugPrint('[isbn] ← Open Library: '
        '${result.isEmpty ? "empty" : "'${result.name}'"}');
    return result;
  } catch (e) {
    debugPrint('[isbn] ← Open Library: ERROR $e');
    return BookLookupResult();
  }
}

Future<BookLookupResult> _fetchGooglebooks(
  String isbn,
  String key,
  Dio dio,
) async {
  debugPrint('[isbn] → Google Books isbn:$isbn');
  try {
    final params = <String, String>{'q': 'isbn:$isbn'};
    if(key.isNotEmpty) params['key'] = key;
    final response = await dio.get<Map<String, dynamic>>(
      'https://www.googleapis.com/books/v1/volumes',
      queryParameters: params,
    );
    final result = googlebooksToResult(isbn, response.data);
    debugPrint('[isbn] ← Google Books: '
        '${result.isEmpty ? "empty" : "'${result.name}'"}');
    return result;
  } catch (e) {
    debugPrint('[isbn] ← Google Books: ERROR $e');
    return BookLookupResult();
  }
}

// ---------------------------------------------------------------------------
// SRU fetcher
// ---------------------------------------------------------------------------

Future<BookLookupResult> _fetchSru(
  SruServer server,
  String isbn,
  Dio dio,
) async {
  try {
    final url = '${server.endpoint}'
        '?version=1.1&operation=searchRetrieve'
        '&maximumRecords=1'
        '&query=${server.isbnField}%3D$isbn'
        '&recordSchema=${server.recordSchema}';
    debugPrint('[isbn] → SRU "${server.name}" $url');
    final response = await dio.get<String>(url);
    final data = response.data ?? '';
    if(data.isEmpty) {
      debugPrint('[isbn] ← SRU "${server.name}": empty response');
      return BookLookupResult();
    }
    final result = server.recordSchema.toLowerCase().contains('unimarc')
        ? parseUnimarcXml(data)
        : parseMarc21Xml(data, isbn);
    debugPrint('[isbn] ← SRU "${server.name}": '
        '${result.isEmpty ? "no record" : "'${result.name}'"}');
    return result;
  } catch (e) {
    debugPrint('[isbn] ← SRU "${server.name}": ERROR $e');
    return BookLookupResult();
  }
}

// ---------------------------------------------------------------------------
// MARC21 XML parser
// ---------------------------------------------------------------------------

/// Parses one MARC21 record element into book metadata.
///
/// Extracts title, authors, publisher, publication date, ISBNs, DDC,
/// page count, description, and genre from the MARC record [marc].
BookLookupResult marc21RecordToBookResult(XmlElement marc) {
  final result = BookLookupResult();
  var authors = '';
  for(final df in marc.childElements) {
    if(localName(df) != 'datafield') continue;
    final tag = df.getAttribute('tag') ?? '';

    if(tag == '020') {
      var clean = '';
      for(final c in subfield(df, 'a').split('')) {
        if(RegExp(r'\d').hasMatch(c) || c.toUpperCase() == 'X') {
          clean += c.toUpperCase();
        } else if(clean.isNotEmpty) {
          break;
        }
      }
      if(isIsbn13(clean) && result.isbn13.isEmpty) result.isbn13 = clean;
      if(isIsbn10(clean) && result.isbn10.isEmpty) result.isbn10 = clean;
    } else if(tag == '082') {
      final ddc = stripMarc(subfield(df, 'a'));
      if(ddc.isNotEmpty && result.ddc.isEmpty) result.ddc = ddc;
    } else if(tag == '245') {
      final title = stripMarc(subfield(df, 'a'));
      if(title.isNotEmpty) result.name = title;
      final resp = stripMarc(subfield(df, 'c'));
      if(resp.isNotEmpty) authors = resp;
    } else if(tag == '264' && df.getAttribute('ind2') == '1') {
      final pub = stripMarc(subfield(df, 'b'));
      final date = stripMarc(subfield(df, 'c'));
      if(pub.isNotEmpty && result.publisher.isEmpty) result.publisher = pub;
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
      final extent = subfield(df, 'a');
      var pages = '';
      var j = extent.length - 1;
      while(j >= 0) {
        if(RegExp(r'\d').hasMatch(extent[j])) {
          final end = j;
          while(j > 0 && RegExp(r'\d').hasMatch(extent[j - 1])) {
            j--;
          }
          pages = extent.substring(j, end + 1);
          break;
        }
        j--;
      }
      if(pages.isNotEmpty) result.pages = pages;
    } else if(tag == '520') {
      final summary = stripMarc(subfield(df, 'a'));
      if(summary.isNotEmpty && result.description.isEmpty) {
        result.description = summary;
      }
    } else if(tag == '655') {
      final genre = stripMarc(subfield(df, 'a'));
      if(genre.isNotEmpty) {
        result.genre =
            result.genre.isEmpty ? genre : '${result.genre}; $genre';
      }
    }
  }
  if(authors.isNotEmpty) result.authors = [authors];
  return result;
}

/// Parses a MARC21-xml SRU response and returns book metadata.
///
/// Delegates the first MARC21 record to [marc21RecordToBookResult].
/// [isbn] is used as a fallback when no ISBN field is found in the record.
/// Returns an empty [BookLookupResult] on parse error or missing records.
BookLookupResult parseMarc21Xml(String data, String isbn) {
  final records = sruMarcRecords(data);
  if(records.isEmpty) return BookLookupResult();
  final result = marc21RecordToBookResult(records.first);
  if(result.isbn13.isEmpty && result.isbn10.isEmpty) {
    _fillIsbn(isbn, result);
  }
  return result;
}

// ---------------------------------------------------------------------------
// UNIMARC XML parser
// ---------------------------------------------------------------------------

/// Parses one UNIMARC record element into book metadata.
///
/// Extracts title, authors, publisher, publication date, ISBNs, page
/// count, description, and genre from the UNIMARC record [marc].
BookLookupResult unimarcRecordToBookResult(XmlElement marc) {
  final result = BookLookupResult();
  final authors = <String>[];
  for(final df in marc.childElements) {
    if(localName(df) != 'datafield') continue;
    final tag = df.getAttribute('tag') ?? '';

    if(tag == '010') {
      var clean = '';
      for(final c in subfield(df, 'a').split('')) {
        if(RegExp(r'\d').hasMatch(c) || c.toUpperCase() == 'X') {
          clean += c.toUpperCase();
        }
      }
      if(isIsbn13(clean) && result.isbn13.isEmpty) result.isbn13 = clean;
      if(isIsbn10(clean) && result.isbn10.isEmpty) result.isbn10 = clean;
    } else if(tag == '200') {
      final title = stripMarc(subfield(df, 'a'));
      if(title.isNotEmpty) result.name = title;
      final authorStmt = stripMarc(subfield(df, 'f'));
      if(authorStmt.isNotEmpty) authors.add(authorStmt);
    } else if(tag == '210') {
      final pub = stripMarc(subfield(df, 'c'));
      final date = stripMarc(subfield(df, 'd'));
      if(pub.isNotEmpty && result.publisher.isEmpty) result.publisher = pub;
      if(date.isNotEmpty && result.publicationDate.isEmpty) {
        result.publicationDate = normalizeDate(date);
      }
    } else if(tag == '215') {
      final extent = subfield(df, 'a');
      var pages = '';
      var j = extent.length - 1;
      while(j >= 0) {
        if(RegExp(r'\d').hasMatch(extent[j])) {
          final end = j;
          while(j > 0 && RegExp(r'\d').hasMatch(extent[j - 1])) {
            j--;
          }
          pages = extent.substring(j, end + 1);
          break;
        }
        j--;
      }
      if(pages.isNotEmpty && result.pages.isEmpty) result.pages = pages;
    } else if(tag == '330') {
      final summary = stripMarc(subfield(df, 'a'));
      if(summary.isNotEmpty && result.description.isEmpty) {
        result.description = summary;
      }
    } else if(tag == '606') {
      final subject = stripMarc(subfield(df, 'a'));
      if(subject.isNotEmpty) {
        result.genre = result.genre.isEmpty
            ? subject
            : '${result.genre}; $subject';
      }
    } else if(tag == '700' || tag == '701') {
      final family = stripMarc(subfield(df, 'a'));
      final forename = stripMarc(subfield(df, 'b'));
      if(family.isNotEmpty) {
        final name =
            forename.isNotEmpty ? '$forename $family' : family;
        authors.add(name);
      }
    }
  }
  if(authors.isNotEmpty) result.authors = authors;
  return result;
}

/// Parses a UNIMARC-xml SRU response (e.g. BnF) and returns book metadata.
///
/// Delegates the first UNIMARC record to [unimarcRecordToBookResult].
/// Returns an empty [BookLookupResult] on parse error or missing records.
BookLookupResult parseUnimarcXml(String data) {
  final records = sruMarcRecords(data);
  if(records.isEmpty) return BookLookupResult();
  return unimarcRecordToBookResult(records.first);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Performs parallel ISBN lookups against all enabled internet services.
///
/// Uses [settings] to determine which services (Open Library, Google Books,
/// SRU servers) to query and what timeout to apply.
/// Returns an empty [BookLookupResult] when no service is enabled.
Future<BookLookupResult> queryIsbn(
  String isbn,
  InternetServicesSettings settings,
) async {
  final dio = _makeDio(settings.timeoutMs);
  final futures = <Future<BookLookupResult>>[];

  if(settings.useOpenlibrary) {
    futures.add(_fetchOpenlibrary(isbn, dio));
  }
  if(settings.useGoogleapis) {
    futures.add(_fetchGooglebooks(isbn, settings.googleapisKey, dio));
  }
  for(final srv in settings.sruServers) {
    if(srv.enabled && srv.isbnField.isNotEmpty) {
      futures.add(_fetchSru(srv, isbn, dio));
    }
  }

  debugPrint('[isbn] queryIsbn $isbn — ${futures.length} service(s)');
  if(futures.isEmpty) return BookLookupResult();

  final partials = await Future.wait(futures);
  final merged = BookLookupResult();
  for(final p in partials) {
    merged.mergeFrom(p);
  }

  _fillIsbn(isbn, merged);
  debugPrint('[isbn] queryIsbn merged: '
      '${merged.isEmpty ? "empty" : "'${merged.name}'"}');
  return merged;
}
