// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
class Lending {
  const Lending({
    required this.id,
    required this.itemId,
    required this.personId,
    required this.count,
    required this.dateTime,
    this.duedate,
    this.remarks,
    this.dataCollectorId,
    this.etag,
  });

  final int id;
  final int itemId;
  final int personId;
  final int count;
  final DateTime dateTime;
  final DateTime? duedate;
  final String? remarks;
  final int? dataCollectorId;
  final String? etag;

  factory Lending.fromJson(Map<String, dynamic> json, {String? etag}) {
    return Lending(
      id: json['id'] as int,
      itemId: json['item_id'] as int,
      personId: json['person_id'] as int,
      count: json['count'] as int,
      dateTime: DateTime.parse(json['date_time'] as String),
      duedate: json['duedate'] != null
          ? DateTime.parse(json['duedate'] as String)
          : null,
      remarks: json['remarks'] as String?,
      dataCollectorId: json['data_collector_id'] as int?,
      etag: etag,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'item_id': itemId,
        'person_id': personId,
        'count': count,
        'date_time': dateTime.toIso8601String(),
        if (duedate != null) 'duedate': duedate!.toIso8601String(),
        if (remarks != null) 'remarks': remarks,
        if (dataCollectorId != null) 'data_collector_id': dataCollectorId,
      };
}
