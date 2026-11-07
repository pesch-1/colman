// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'api_client.dart';
import '../models/affected_person.dart';

/// REST client for `/api/classes` and the grade-range setting in `/api/collection`.
class ClassesApi {
  const ClassesApi(this._client);

  final ApiClient _client;

  /// Returns the current list of class names (sorted by the server).
  Future<List<String>> loadClassNames() async {
    final response = await _client.dio.get('/api/classes');
    return (response.data as List).cast<String>();
  }

  /// Adds a new class with the given [name].
  Future<void> addClass(String name) async {
    await _client.dio.post('/api/classes', data: {'name': name});
  }

  /// Removes the class with the given [name].
  Future<void> removeClass(String name) async {
    await _client.dio.delete('/api/classes/$name');
  }

  /// Renames [oldName] to [newName] (also updates persons table atomically).
  Future<void> renameClass(String oldName, String newName) async {
    await _client.dio.put(
      '/api/classes/$oldName',
      data: {'new_name': newName},
    );
  }

  /// Fetches the grade range (min, max) from the collection settings.
  Future<({int min, int max})> fetchGradeRange() async {
    final response = await _client.dio.get('/api/collection');
    final data = response.data as Map<String, dynamic>;
    return (
      min: (data['grade_range_min'] as int?) ?? 1,
      max: (data['grade_range_max'] as int?) ?? 13,
    );
  }

  /// Saves the grade range to the collection settings.
  Future<void> saveGradeRange(int min, int max) async {
    await _client.dio.put('/api/collection', data: {
      'grade_range_min': min,
      'grade_range_max': max,
    });
  }

  /// Returns persons whose current class appears as a key in [oldToNew].
  ///
  /// The map value is the suggested new class name (empty = dissolved).
  Future<List<AffectedPerson>> getAffectedPersons(
    Map<String, String> oldToNew,
  ) async {
    final response = await _client.dio.post(
      '/api/classes/affected-persons',
      data: oldToNew,
    );
    return (response.data as List)
        .map((e) => AffectedPerson.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Applies school-year changes atomically:
  /// - updates each person's class according to [personToClass],
  /// - deletes persons in [toDelete],
  /// - replaces the class list with [newClassList].
  Future<void> applyYearChanges({
    required Map<int, String> personToClass,
    required List<int> toDelete,
    required List<String> newClassList,
  }) async {
    await _client.dio.post('/api/classes/year-changes', data: {
      'person_id_to_new_class':
          personToClass.map((k, v) => MapEntry(k.toString(), v)),
      'persons_to_delete': toDelete,
      'new_class_list': newClassList,
    });
  }
}
