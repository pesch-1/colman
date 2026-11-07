// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// One reservation row returned by the server.
class Reservation {
  const Reservation({
    required this.id,
    required this.itemId,
    required this.personId,
    required this.count,
    required this.reservedAt,
    required this.expiresAt,
    required this.version,
    required this.dataCollectorId,
  });

  final int id;
  final int itemId;
  final int personId;
  final int count;
  final DateTime reservedAt;
  final DateTime expiresAt;
  final int version;
  final int dataCollectorId;

  factory Reservation.fromJson(Map<String, dynamic> json) {
    return Reservation(
      id: json['id'] as int,
      itemId: json['item_id'] as int,
      personId: json['person_id'] as int,
      count: json['count'] as int,
      reservedAt: DateTime.parse(json['reserved_at'] as String),
      expiresAt: DateTime.parse(json['expires_at'] as String),
      version: json['version'] as int? ?? 1,
      dataCollectorId: json['data_collector_id'] as int? ?? 0,
    );
  }
}
