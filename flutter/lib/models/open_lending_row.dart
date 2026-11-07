// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// A single row from `GET /api/lendings`.
///
/// Represents one open (unreturned or partially-returned) lending.
/// The server returns a list of these when querying active lendings.
class OpenLendingRow {
  /// Creates an [OpenLendingRow].
  const OpenLendingRow({
    required this.id,
    required this.lentCount,
    required this.returnedCount,
    required this.datetime,
    this.duedate,
    required this.remarks,
    required this.personName,
    required this.itemName,
  });

  /// Primary key of the lending record.
  final int id;

  /// Total number of copies lent in this transaction.
  final int lentCount;

  /// Number of copies returned so far.
  final int returnedCount;

  /// Date and time the lending was recorded.
  final DateTime datetime;

  /// Optional due date for return; null when the server returns an empty string.
  final DateTime? duedate;

  /// Optional free-text remarks attached to the lending.
  final String remarks;

  /// Display name of the borrowing person.
  final String personName;

  /// Display name of the borrowed item.
  final String itemName;

  /// Deserialises one element of the `GET /api/lendings` JSON array.
  factory OpenLendingRow.fromJson(Map<String, dynamic> json) {
    final duedateStr = json['duedate'] as String? ?? '';
    return OpenLendingRow(
      id: (json['id'] as num).toInt(),
      lentCount: (json['lent_count'] as num).toInt(),
      returnedCount: (json['returned_count'] as num).toInt(),
      datetime: DateTime.parse(json['datetime'] as String),
      duedate: duedateStr.isNotEmpty ? DateTime.parse(duedateStr) : null,
      remarks: json['remarks'] as String? ?? '',
      personName: json['person_name'] as String? ?? '',
      itemName: json['item_name'] as String? ?? '',
    );
  }
}
