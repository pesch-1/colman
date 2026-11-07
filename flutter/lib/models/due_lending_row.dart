// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// A single row from `GET /api/lendings/due`.
///
/// Represents a lending whose due date falls within the requested date range
/// and still has copies outstanding (not yet returned).
class DueLendingRow {
  /// Creates a [DueLendingRow].
  const DueLendingRow({
    required this.lendingId,
    required this.itemId,
    required this.personId,
    required this.duedate,
    required this.personName,
    required this.outstandingCount,
    required this.itemName,
  });

  /// Primary key of the lending record.
  final int lendingId;

  /// Primary key of the borrowed item.
  final int itemId;

  /// Primary key of the borrowing person.
  final int personId;

  /// Date by which the item is due for return.
  final DateTime duedate;

  /// Display name of the borrowing person.
  final String personName;

  /// Number of copies not yet returned.
  final int outstandingCount;

  /// Display name of the borrowed item.
  final String itemName;

  /// Deserialises one element of the `GET /api/lendings/due` JSON array.
  ///
  /// The server serialises a null due-date as `""`.  We map that to
  /// [DateTime.utc(0)] so callers can test `duedate.year > 0`.
  factory DueLendingRow.fromJson(Map<String, dynamic> json) {
    final duedateStr = json['duedate'] as String? ?? '';
    return DueLendingRow(
      lendingId: (json['lending_id'] as num).toInt(),
      itemId: (json['item_id'] as num).toInt(),
      personId: (json['person_id'] as num).toInt(),
      duedate: duedateStr.isNotEmpty
          ? DateTime.parse(duedateStr)
          : DateTime.utc(0),
      personName: json['person_name'] as String? ?? '',
      outstandingCount: (json['outstanding_count'] as num).toInt(),
      itemName: json['item_name'] as String? ?? '',
    );
  }
}
