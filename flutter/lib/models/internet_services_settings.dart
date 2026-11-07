// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

/// Configuration for a single SRU (Search/Retrieve via URL) library server.
class SruServer {
  const SruServer({
    required this.name,
    required this.endpoint,
    required this.isbnField,
    required this.eanField,
    this.titleField = '',
    required this.recordSchema,
    required this.enabled,
  });

  final String name;
  final String endpoint;
  final String isbnField;
  final String eanField;

  /// CQL index for title searches; empty = title search not supported.
  final String titleField;

  final String recordSchema;
  final bool enabled;

  /// True when the record schema is a UNIMARC variant (e.g. BnF).
  bool get isUnimarc => recordSchema.toLowerCase().contains('unimarc');

  SruServer copyWith({
    String? name,
    String? endpoint,
    String? isbnField,
    String? eanField,
    String? titleField,
    String? recordSchema,
    bool? enabled,
  }) {
    return SruServer(
      name: name ?? this.name,
      endpoint: endpoint ?? this.endpoint,
      isbnField: isbnField ?? this.isbnField,
      eanField: eanField ?? this.eanField,
      titleField: titleField ?? this.titleField,
      recordSchema: recordSchema ?? this.recordSchema,
      enabled: enabled ?? this.enabled,
    );
  }

  Map<String, dynamic> toJson() => {
    'name': name,
    'endpoint': endpoint,
    'isbn_field': isbnField,
    'ean_field': eanField,
    'title_field': titleField,
    'record_schema': recordSchema,
    'enabled': enabled,
  };

  /// Deserializes a server entry.
  ///
  /// Entries saved by versions without title-search support lack the
  /// `title_field` key; for those the title field of the default server
  /// with the same endpoint is used (empty for unknown endpoints).
  factory SruServer.fromJson(Map<String, dynamic> json) => SruServer(
    name: json['name'] as String? ?? '',
    endpoint: json['endpoint'] as String? ?? '',
    isbnField: json['isbn_field'] as String? ?? '',
    eanField: json['ean_field'] as String? ?? '',
    titleField: json.containsKey('title_field')
        ? (json['title_field'] as String? ?? '')
        : _defaultTitleFieldFor(json['endpoint'] as String? ?? ''),
    recordSchema: json['record_schema'] as String? ?? 'marcxml',
    enabled: json['enabled'] as bool? ?? false,
  );

  @override
  bool operator ==(Object other) =>
      other is SruServer &&
      name == other.name &&
      endpoint == other.endpoint &&
      isbnField == other.isbnField &&
      eanField == other.eanField &&
      titleField == other.titleField &&
      recordSchema == other.recordSchema &&
      enabled == other.enabled;

  @override
  int get hashCode => Object.hash(
    name, endpoint, isbnField, eanField, titleField, recordSchema, enabled,
  );
}

/// Returns the default CQL title index for the server at [endpoint].
///
/// Used to migrate SRU server lists saved by versions that did not yet
/// know the title field; unknown endpoints yield an empty string.
String _defaultTitleFieldFor(String endpoint) {
  for (final srv in _defaultSruServers) {
    if (srv.endpoint == endpoint) return srv.titleField;
  }
  return '';
}

/// All internet service settings persisted by the app.
class InternetServicesSettings {
  const InternetServicesSettings({
    required this.useOpenlibrary,
    required this.useGoogleapis,
    required this.googleapisKey,
    required this.useMusicbrainz,
    required this.useBgg,
    required this.bggToken,
    required this.useOpengtindb,
    required this.opengtindbUserid,
    required this.timeoutMs,
    required this.sruServers,
    required this.downloadThumbnails,
  });

  /// Returns the default configuration, matching the Qt desktop defaults.
  factory InternetServicesSettings.defaults() => InternetServicesSettings(
    useOpenlibrary: false,
    useGoogleapis: false,
    googleapisKey: '',
    useMusicbrainz: false,
    useBgg: false,
    bggToken: '',
    useOpengtindb: false,
    opengtindbUserid: '',
    timeoutMs: 5000,
    sruServers: _defaultSruServers,
    downloadThumbnails: false,
  );

  final bool useOpenlibrary;
  final bool useGoogleapis;
  final String googleapisKey;
  final bool useMusicbrainz;
  final bool useBgg;
  final String bggToken;
  final bool useOpengtindb;
  final String opengtindbUserid;

  /// Network timeout in milliseconds.
  final int timeoutMs;

  final List<SruServer> sruServers;

  /// Whether to download and store cover images during barcode lookup.
  final bool downloadThumbnails;

  InternetServicesSettings copyWith({
    bool? useOpenlibrary,
    bool? useGoogleapis,
    String? googleapisKey,
    bool? useMusicbrainz,
    bool? useBgg,
    String? bggToken,
    bool? useOpengtindb,
    String? opengtindbUserid,
    int? timeoutMs,
    List<SruServer>? sruServers,
    bool? downloadThumbnails,
  }) {
    return InternetServicesSettings(
      useOpenlibrary: useOpenlibrary ?? this.useOpenlibrary,
      useGoogleapis: useGoogleapis ?? this.useGoogleapis,
      googleapisKey: googleapisKey ?? this.googleapisKey,
      useMusicbrainz: useMusicbrainz ?? this.useMusicbrainz,
      useBgg: useBgg ?? this.useBgg,
      bggToken: bggToken ?? this.bggToken,
      useOpengtindb: useOpengtindb ?? this.useOpengtindb,
      opengtindbUserid: opengtindbUserid ?? this.opengtindbUserid,
      timeoutMs: timeoutMs ?? this.timeoutMs,
      sruServers: sruServers ?? this.sruServers,
      downloadThumbnails: downloadThumbnails ?? this.downloadThumbnails,
    );
  }

  @override
  bool operator ==(Object other) =>
      other is InternetServicesSettings &&
      useOpenlibrary == other.useOpenlibrary &&
      useGoogleapis == other.useGoogleapis &&
      googleapisKey == other.googleapisKey &&
      useMusicbrainz == other.useMusicbrainz &&
      useBgg == other.useBgg &&
      bggToken == other.bggToken &&
      useOpengtindb == other.useOpengtindb &&
      opengtindbUserid == other.opengtindbUserid &&
      timeoutMs == other.timeoutMs &&
      _listEquals(sruServers, other.sruServers) &&
      downloadThumbnails == other.downloadThumbnails;

  @override
  int get hashCode => Object.hash(
    useOpenlibrary,
    useGoogleapis,
    googleapisKey,
    useMusicbrainz,
    useBgg,
    bggToken,
    useOpengtindb,
    opengtindbUserid,
    timeoutMs,
    Object.hashAll(sruServers),
    downloadThumbnails,
  );

  static bool _listEquals(List<SruServer> a, List<SruServer> b) {
    if (a.length != b.length) return false;
    for (var i = 0; i < a.length; i++) {
      if (a[i] != b[i]) return false;
    }
    return true;
  }
}

/// Serializes a list of [SruServer] objects to a JSON string.
String sruServersToJson(List<SruServer> servers) =>
    jsonEncode(servers.map((s) => s.toJson()).toList());

/// Deserializes a list of [SruServer] objects from a JSON string.
///
/// Returns [_defaultSruServers] on any parse error.
List<SruServer> sruServersFromJson(String json) {
  try {
    final list = jsonDecode(json) as List<dynamic>;
    return list
        .whereType<Map<String, dynamic>>()
        .map(SruServer.fromJson)
        .toList();
  } catch (_) {
    return List.of(_defaultSruServers);
  }
}

const List<SruServer> _defaultSruServers = [
  SruServer(
    name: 'Deutsche Nationalbibliothek (DNB)',
    endpoint: 'https://services.dnb.de/sru/dnb',
    isbnField: 'num',
    eanField: '',
    titleField: 'tit',
    recordSchema: 'MARC21-xml',
    enabled: true,
  ),
  SruServer(
    name: 'K10plus',
    endpoint: 'https://sru.k10plus.de/opac-de-627',
    isbnField: 'pica.isb',
    eanField: 'pica.num',
    titleField: 'pica.tit',
    recordSchema: 'marcxml',
    enabled: true,
  ),
  SruServer(
    name: 'Library of Congress',
    endpoint: 'https://lx2.loc.gov/sru/lcdb',
    isbnField: 'bath.isbn',
    eanField: '',
    titleField: 'dc.title',
    recordSchema: 'marcxml',
    enabled: true,
  ),
  SruServer(
    name: 'Österreichische Nationalbibliothek (ÖNB)',
    endpoint: 'https://obv-at-oenb.alma.exlibrisgroup.com/view/sru/43ACC_ONB',
    isbnField: 'alma.isbn',
    eanField: '',
    titleField: 'alma.title',
    recordSchema: 'marcxml',
    enabled: false,
  ),
  SruServer(
    name: 'Swisscovery',
    endpoint: 'https://swisscovery.slsp.ch/view/sru/41SLSP_NETWORK',
    isbnField: 'isbn',
    eanField: '',
    titleField: 'alma.title',
    recordSchema: 'marcxml',
    enabled: false,
  ),
  SruServer(
    name: 'Bibliothèque nationale de France (BnF)',
    endpoint: 'http://catalogue.bnf.fr/api/SRU',
    isbnField: 'bib.isbn',
    eanField: '',
    titleField: 'bib.title',
    recordSchema: 'unimarcxchange',
    enabled: false,
  ),
];
