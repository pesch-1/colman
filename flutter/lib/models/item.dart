// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
class Item {
  const Item({
    required this.id,
    required this.typeId,
    required this.name,
    this.barcode,
    this.place,
    this.description,
    this.values = const {},
    this.listValues = const {},
    this.thumbnailId,
    this.isSchoolItem = false,
    this.subject,
    this.academicYear = const [],
    this.typeName,
    this.dataCollectorId,
    this.signatureTop,
    this.signatureBottom,
    this.keywords = const [],
    this.allowReservations = false,
    this.reservationPeriodDays,
    this.etag,
  });

  final int id;
  final int typeId;
  final String name;
  final String? barcode;
  final String? place;
  final String? description;
  final Map<String, String> values;
  final Map<String, List<String>> listValues;
  final int? thumbnailId;
  final bool isSchoolItem;
  final String? subject;
  final List<List<int>> academicYear;
  final String? typeName;
  final int? dataCollectorId;
  final String? signatureTop;
  final String? signatureBottom;
  final List<String> keywords;
  final bool allowReservations;
  final int? reservationPeriodDays;
  final String? etag;

  factory Item.fromJson(Map<String, dynamic> json, {int? id, String? etag}) {
    return Item(
      id: id ?? json['id'] as int,
      typeId: json['type_id'] as int,
      name: json['name'] as String,
      barcode: json['barcode'] as String?,
      place: json['place'] as String?,
      description: json['description'] as String?,
      values: (json['values'] as Map<String, dynamic>?)
              ?.map((k, v) => MapEntry(k, v as String)) ??
          const {},
      listValues: (json['list_values'] as Map<String, dynamic>?)?.map(
              (k, v) => MapEntry(k, (v as List).cast<String>())) ??
          const {},
      thumbnailId: json['thumbnail_id'] as int?,
      isSchoolItem: json['is_school_item'] as bool? ?? false,
      subject: json['subject'] as String?,
      academicYear: (json['academic_year'] as List<dynamic>? ?? [])
          .map((e) => (e as List<dynamic>).cast<int>())
          .toList(),
      typeName: json['type_name'] as String?,
      dataCollectorId: json['data_collector_id'] as int?,
      signatureTop: json['signature_top'] as String?,
      signatureBottom: json['signature_bottom'] as String?,
      keywords: (json['keywords'] as List<dynamic>? ?? []).cast<String>(),
      allowReservations: (json['allow_reservations'] as int?) != 0,
      reservationPeriodDays: json['reservation_period_days'] as int?,
      etag: etag,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'type_id': typeId,
        'name': name,
        if (barcode != null) 'barcode': barcode,
        if (place != null) 'place': place,
        if (description != null) 'description': description,
        'values': values,
        'list_values': listValues,
        if (thumbnailId != null) 'thumbnail_id': thumbnailId,
        'is_school_item': isSchoolItem,
        if (subject != null) 'subject': subject,
        'academic_year': academicYear,
        if (typeName != null) 'type_name': typeName,
        if (dataCollectorId != null) 'data_collector_id': dataCollectorId,
        if (signatureTop != null) 'signature_top': signatureTop,
        if (signatureBottom != null) 'signature_bottom': signatureBottom,
        'keywords': keywords,
        'allow_reservations': allowReservations,
        if (reservationPeriodDays != null)
          'reservation_period_days': reservationPeriodDays,
      };
}

/// Search parameters for `POST /api/items/search`.
class SearchQuery {
  const SearchQuery({
    this.general = '',
    this.name = '',
    this.subject = '',
    this.description = '',
    this.keywords = '',
    this.typeId = -1,
  });

  final String general;
  final String name;
  final String subject;
  final String description;
  final String keywords;

  /// Item type filter; -1 means all types.
  final int typeId;

  bool get isEmpty =>
      general.isEmpty &&
      name.isEmpty &&
      subject.isEmpty &&
      description.isEmpty &&
      keywords.isEmpty &&
      typeId == -1;

  SearchQuery copyWith({
    String? general,
    String? name,
    String? subject,
    String? description,
    String? keywords,
    int? typeId,
  }) =>
      SearchQuery(
        general: general ?? this.general,
        name: name ?? this.name,
        subject: subject ?? this.subject,
        description: description ?? this.description,
        keywords: keywords ?? this.keywords,
        typeId: typeId ?? this.typeId,
      );
}

/// A compact result row returned by `POST /api/items/search`.
///
/// Contains only the fields needed to display a search result list.
/// Navigate to the full [Item] via [id].
class SearchItemRow {
  /// Creates a [SearchItemRow].
  const SearchItemRow({
    required this.id,
    required this.name,
    required this.typeLocale,
    required this.count,
    required this.available,
    required this.typeId,
  });

  /// Primary key of the item.
  final int id;

  /// Display name of the item.
  final String name;

  /// Localised type name (e.g. "Buch", "Zeitschrift").
  final String typeLocale;

  /// Total number of copies in stock.
  final int count;

  /// Number of copies currently available for lending.
  final int available;

  /// Foreign key of the item type.
  final int typeId;

  /// Deserialises a [SearchItemRow] from a JSON map.
  factory SearchItemRow.fromJson(Map<String, dynamic> json) {
    return SearchItemRow(
      id: json['id'] as int,
      name: json['name'] as String,
      typeLocale: json['type_locale'] as String,
      count: json['count'] as int,
      available: json['available'] as int,
      typeId: json['type_id'] as int,
    );
  }
}
