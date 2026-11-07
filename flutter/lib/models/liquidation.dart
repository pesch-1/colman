// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
class Liquidation {
  const Liquidation({
    required this.id,
    required this.itemId,
    required this.count,
    required this.dateTime,
    this.dataCollectorId,
  });

  final int id;
  final int itemId;
  final int count;
  final DateTime dateTime;
  final int? dataCollectorId;

  factory Liquidation.fromJson(Map<String, dynamic> json) {
    return Liquidation(
      id: json['id'] as int,
      itemId: json['item_id'] as int,
      count: json['count'] as int,
      dateTime: DateTime.parse(json['date_time'] as String),
      dataCollectorId: json['data_collector_id'] as int?,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'item_id': itemId,
        'count': count,
        'date_time': dateTime.toIso8601String(),
        if (dataCollectorId != null) 'data_collector_id': dataCollectorId,
      };
}
