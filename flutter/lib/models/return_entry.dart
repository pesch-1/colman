// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
class ReturnEntry {
  const ReturnEntry({
    required this.id,
    required this.lendingId,
    required this.count,
    required this.dateTime,
    this.remarks,
    this.dataCollectorId,
  });

  final int id;
  final int lendingId;
  final int count;
  final DateTime dateTime;
  final String? remarks;
  final int? dataCollectorId;

  factory ReturnEntry.fromJson(Map<String, dynamic> json) {
    return ReturnEntry(
      id: json['id'] as int,
      lendingId: json['lending_id'] as int,
      count: json['count'] as int,
      dateTime: DateTime.parse(json['date_time'] as String),
      remarks: json['remarks'] as String?,
      dataCollectorId: json['data_collector_id'] as int?,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'lending_id': lendingId,
        'count': count,
        'date_time': dateTime.toIso8601String(),
        if (remarks != null) 'remarks': remarks,
        if (dataCollectorId != null) 'data_collector_id': dataCollectorId,
      };
}
