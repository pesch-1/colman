// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Maps the server-side `DataType` integer enum to Flutter-side values.
///
/// Order must match the C++ `colman::DataType` enum exactly:
/// String=0, ShortString=1, Date=2, Time=3, Integer=4,
/// Float=5, Duration=6, Table=7, IntegerRanges=8.
enum TypeKeyDataType {
  string,       // 0 – multi-line text
  shortString,  // 1 – single-line text (most common)
  date,         // 2 – date
  time,         // 3 – time-of-day
  integer,      // 4 – integer
  float,        // 5 – floating-point
  duration,     // 6 – duration string
  table,        // 7 – list of strings  (isList = true)
  integerRanges // 8 – integer ranges  (isList = true)
}

/// Maps the server-side `MediaKind` integer enum (colman_media_kind.hpp).
///
/// Order must match exactly: Unknown=0, Book=1, Cd=2, Dvd=3, BluRay=4,
/// BoardGame=5.
enum MediaKind {
  unknown,   // 0
  book,      // 1
  cd,        // 2
  dvd,       // 3
  bluRay,    // 4
  boardGame, // 5
}

class TypeKey {
  const TypeKey({
    required this.name,
    required this.dataType,
    String? displayName,
    this.isList = false,
    this.info = const {},
  }) : displayName = displayName ?? name;

  /// Internal column name (e.g. "authors"). Immutable once a field exists —
  /// renaming the column would drop and recreate it (data loss).
  final String name;

  /// Localized display name (e.g. "Autoren"), falls back to [name].
  final String displayName;

  final TypeKeyDataType dataType;

  /// True when the field accepts multiple values (info.is_repeatable = true).
  final bool isList;

  /// Raw `info` map from the server (is_repeatable, with_minimum, minimum,
  /// with_maximum, maximum, marc21_tag, …). Preserved verbatim so that
  /// round-tripping a type through PUT does not discard constraints or MARC21
  /// tags of fields the mobile editor does not expose.
  final Map<String, dynamic> info;

  TypeKey copyWith({
    String? name,
    String? displayName,
    TypeKeyDataType? dataType,
    bool? isList,
  }) {
    return TypeKey(
      name: name ?? this.name,
      displayName: displayName ?? this.displayName,
      dataType: dataType ?? this.dataType,
      isList: isList ?? this.isList,
      info: info,
    );
  }

  factory TypeKey.fromJson(Map<String, dynamic> json) {
    final dt = (json['data_type'] as num).toInt();
    final dataType = (dt >= 0 && dt < TypeKeyDataType.values.length)
        ? TypeKeyDataType.values[dt]
        : TypeKeyDataType.shortString;
    final column = json['column'] as String;
    final label = json['label'] as String?;
    final info = (json['info'] as Map<String, dynamic>?) ?? {};
    return TypeKey(
      name: column,
      displayName: (label != null && label.isNotEmpty) ? label : column,
      dataType: dataType,
      isList: info['is_repeatable'] == true,
      info: Map<String, dynamic>.from(info),
    );
  }

  /// Server-compatible key object for POST/PUT `/api/types`.
  ///
  /// Keeps the existing [info] map intact, only overriding `is_repeatable`
  /// from [isList].
  Map<String, dynamic> toServerJson() => {
        'column': name,
        'data_type': dataType.index,
        'info': {
          ...info,
          'is_repeatable': isList,
        },
      };
}

class ItemType {
  const ItemType({
    required this.id,
    required this.name,
    this.keys = const [],
    this.mediaKind = MediaKind.unknown,
    this.etag,
  });

  final int id;
  final String name;
  final List<TypeKey> keys;
  final MediaKind mediaKind;
  final String? etag;

  factory ItemType.fromJson(Map<String, dynamic> json, {String? etag}) {
    final mk = (json['media_kind'] as num?)?.toInt() ?? 0;
    return ItemType(
      id: json['id'] as int,
      name: json['name'] as String,
      keys: (json['keys'] as List?)
              ?.map((k) => TypeKey.fromJson(k as Map<String, dynamic>))
              .toList() ??
          const [],
      mediaKind: (mk >= 0 && mk < MediaKind.values.length)
          ? MediaKind.values[mk]
          : MediaKind.unknown,
      etag: etag,
    );
  }

  /// Body for `POST /api/types` (new type). [tablename] must be the internal
  /// SQLite table name (`"_" + stringToColumnName(name)`).
  Map<String, dynamic> toCreateJson(String tablename) => {
        'name': name,
        'media_kind': mediaKind.index,
        'tablename': tablename,
        'keys': keys.map((k) => k.toServerJson()).toList(),
      };

  /// Body for `PUT /api/types/<id>` (existing type). Sends the full key list
  /// (the server rebuilds the type table from it) plus the display-name map.
  Map<String, dynamic> toUpdateJson() => {
        'name': name,
        'media_kind': mediaKind.index,
        'keys': keys.map((k) => k.toServerJson()).toList(),
        'display_names': {
          for (final k in keys) k.name: k.displayName,
        },
      };
}
