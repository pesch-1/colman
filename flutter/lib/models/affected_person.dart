// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// A person affected by a school-year class transition.
class AffectedPerson {
  const AffectedPerson({
    required this.personId,
    required this.displayName,
    required this.oldClass,
    required this.newClass,
  });

  final int personId;
  final String displayName;
  final String oldClass;

  /// Target class for next school year; empty string means no new class.
  final String newClass;

  factory AffectedPerson.fromJson(Map<String, dynamic> json) {
    return AffectedPerson(
      personId: json['person_id'] as int,
      displayName: json['display_name'] as String,
      oldClass: json['old_class'] as String,
      newClass: json['new_class'] as String,
    );
  }

  AffectedPerson copyWith({String? newClass}) {
    return AffectedPerson(
      personId: personId,
      displayName: displayName,
      oldClass: oldClass,
      newClass: newClass ?? this.newClass,
    );
  }
}
