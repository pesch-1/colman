// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// A row in the admin "change passwords" person list.
///
/// Mirrors the JSON objects returned by `GET /api/persons/change-passwords`:
/// the minimal fields needed to identify a person and show their current
/// username when resetting credentials.
class ChangePasswordPerson {
  const ChangePasswordPerson({
    required this.id,
    required this.lastname,
    required this.firstname,
    this.title = '',
    this.username = '',
  });

  /// Parses one object from the `GET /api/persons/change-passwords` array.
  factory ChangePasswordPerson.fromJson(Map<String, dynamic> json) {
    return ChangePasswordPerson(
      id: (json['id'] as num).toInt(),
      lastname: json['lastname'] as String? ?? '',
      firstname: json['firstname'] as String? ?? '',
      title: json['title'] as String? ?? '',
      username: json['username'] as String? ?? '',
    );
  }

  /// Database id of the person (0 = administrator).
  final int id;
  final String lastname;
  final String firstname;
  final String title;

  /// Current login username, or empty when none is set.
  final String username;

  /// Display name, e.g. `"Dr. Smith, John"`.
  String get displayName {
    final name = [lastname, firstname]
        .where((s) => s.isNotEmpty)
        .join(', ');
    return title.isEmpty ? name : '$title $name';
  }
}
