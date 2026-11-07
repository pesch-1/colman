// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
class Person {
  const Person({
    required this.id,
    required this.lastname,
    required this.firstname,
    this.title,
    this.schoolClass,
    this.barcode,
    this.emails = const [],
    this.telephone,
    this.street,
    this.zip,
    this.city,
    this.country,
    this.remarks,
    this.thumbnailId,
    this.dataCollectorId,
    this.externalId,
    this.username,
    this.etag,
  });

  final int id;
  final String lastname;
  final String firstname;
  final String? title;
  final String? schoolClass;
  final String? barcode;
  final List<String> emails;
  final String? telephone;
  final String? street;
  final String? zip;
  final String? city;
  final String? country;
  final String? remarks;
  final int? thumbnailId;
  final int? dataCollectorId;
  final String? externalId;
  final String? username;
  final String? etag;

  String get displayName {
    final t = (title != null && title!.isNotEmpty) ? '$title ' : '';
    return '$t$firstname $lastname';
  }

  factory Person.fromJson(Map<String, dynamic> json, {String? etag}) {
    return Person(
      id: json['id'] as int,
      lastname: json['lastname'] as String,
      firstname: json['firstname'] as String,
      title: json['title'] as String?,
      schoolClass: json['school_class'] as String?,
      barcode: json['barcode'] as String?,
      // The server serialises emails as a comma-separated plain String.
      // Handle String, List (forward-compat), and null defensively.
      emails: _parseEmails(json['emails']),
      telephone: json['telephone'] as String?,
      street: json['street'] as String?,
      zip: json['zip'] as String?,
      city: json['city'] as String?,
      country: json['country'] as String?,
      remarks: json['remarks'] as String?,
      thumbnailId: json['thumbnail_id'] as int?,
      dataCollectorId: json['data_collector_id'] as int?,
      externalId: json['external_id'] as String?,
      username: json['username'] as String?,
      etag: etag,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'lastname': lastname,
        'firstname': firstname,
        if (title != null) 'title': title,
        if (schoolClass != null) 'school_class': schoolClass,
        if (barcode != null) 'barcode': barcode,
        // The server stores emails as a comma-separated String.
        'emails': emails.join(', '),
        if (telephone != null) 'telephone': telephone,
        if (street != null) 'street': street,
        if (zip != null) 'zip': zip,
        if (city != null) 'city': city,
        if (country != null) 'country': country,
        if (remarks != null) 'remarks': remarks,
        if (thumbnailId != null) 'thumbnail_id': thumbnailId,
        if (dataCollectorId != null) 'data_collector_id': dataCollectorId,
        if (externalId != null) 'external_id': externalId,
        if (username != null) 'username': username,
      };
}

/// Parses the `emails` JSON field.
///
/// The server serialises emails as a comma-separated [String]. Accepts
/// a plain [String], a [List] (defensive, for forward-compatibility),
/// or `null`, and always returns a non-null [List<String>].
List<String> _parseEmails(dynamic value) {
  if (value is String) {
    return value
        .split(',')
        .map((e) => e.trim())
        .where((e) => e.isNotEmpty)
        .toList();
  }
  if (value is List) {
    return value
        .map((e) => '$e'.trim())
        .where((e) => e.isNotEmpty)
        .toList();
  }
  return const <String>[];
}
