// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// One row of the combined history view.
class HistoryRow {
  const HistoryRow({
    required this.id,
    required this.datetime,
    required this.action,
    required this.itemName,
    required this.count,
    required this.userName,
    required this.collector,
    required this.itemId,
    required this.personId,
    required this.remarks,
    required this.collectorId,
    this.duedate,
  });

  final int id;
  final DateTime datetime;
  final String action;
  final String itemName;
  final int count;
  final String userName;
  final String collector;
  final int itemId;
  final int personId;
  final String remarks;
  final int collectorId;

  /// Due date — set for lending rows only.
  final DateTime? duedate;

  factory HistoryRow.fromJson(Map<String, dynamic> json) {
    final duedateStr = json['duedate'] as String? ?? '';
    return HistoryRow(
      id: (json['id'] as num?)?.toInt() ?? 0,
      datetime: DateTime.parse(json['datetime'] as String),
      action: json['action'] as String? ?? '',
      itemName: json['item_name'] as String? ?? '',
      count: (json['count'] as num?)?.toInt() ?? 0,
      userName: json['user_name'] as String? ?? '',
      collector: json['collector'] as String? ?? '',
      itemId: (json['item_id'] as num?)?.toInt() ?? 0,
      personId: (json['person_id'] as num?)?.toInt() ?? 0,
      remarks: json['remarks'] as String? ?? '',
      collectorId: (json['collector_id'] as num?)?.toInt() ?? 0,
      duedate: duedateStr.isNotEmpty ? DateTime.parse(duedateStr) : null,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'datetime': datetime.toIso8601String(),
        'action': action,
        'item_name': itemName,
        'count': count,
        'user_name': userName,
        'collector': collector,
        'item_id': itemId,
        'person_id': personId,
        'remarks': remarks,
        'collector_id': collectorId,
        'duedate': duedate != null
            ? duedate!.toIso8601String().substring(0, 10)
            : '',
      };

  HistoryRow copyWith({
    int? id,
    DateTime? datetime,
    String? action,
    String? itemName,
    int? count,
    String? userName,
    String? collector,
    int? itemId,
    int? personId,
    String? remarks,
    int? collectorId,
    DateTime? duedate,
    bool clearDuedate = false,
  }) {
    return HistoryRow(
      id: id ?? this.id,
      datetime: datetime ?? this.datetime,
      action: action ?? this.action,
      itemName: itemName ?? this.itemName,
      count: count ?? this.count,
      userName: userName ?? this.userName,
      collector: collector ?? this.collector,
      itemId: itemId ?? this.itemId,
      personId: personId ?? this.personId,
      remarks: remarks ?? this.remarks,
      collectorId: collectorId ?? this.collectorId,
      duedate: clearDuedate ? null : (duedate ?? this.duedate),
    );
  }
}
