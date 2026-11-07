// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';

import '../models/item_type.dart';
import 'api_client.dart';

class TypesApi {
  const TypesApi(this._client);

  final ApiClient _client;

  Future<List<ItemType>> getAll() async {
    final response = await _client.dio.get('/api/types');
    final list = response.data as List;
    return list
        .map((e) => ItemType.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  Future<List<String>> getNames() async {
    final response = await _client.dio.get('/api/types/names');
    return (response.data as List).cast<String>();
  }

  Future<ItemType> getById(int id) async {
    final response = await _client.dio.get('/api/types/$id');
    final etag = response.headers.value('ETag');
    return ItemType.fromJson(
        response.data as Map<String, dynamic>, etag: etag);
  }

  /// Creates a new type and returns its database id.
  ///
  /// The server responds with `{"id": <int>}` only (not a full type object).
  Future<int> create(Map<String, dynamic> data) async {
    final response = await _client.dio.post('/api/types', data: data);
    final body = response.data;
    if (body is Map && body['id'] != null) {
      return (body['id'] as num).toInt();
    }
    return -1;
  }

  /// Updates an existing type. The server returns an empty `200 OK`.
  Future<void> update(
      int id, Map<String, dynamic> data, String etag) async {
    await _client.dio.put(
      '/api/types/$id',
      data: data,
      options: Options(headers: {'If-Match': etag}),
    );
  }

  Future<void> delete(int id) async {
    await _client.dio.delete('/api/types/$id');
  }

  /// Returns all types with localized names and full key definitions.
  ///
  /// [getAll] only returns internal (un-localized) names.  This method
  /// uses [getLocale] for the display names (server applies system locale)
  /// and [getById] for each type's key list.
  Future<List<ItemType>> getAllWithKeys() async {
    final pairs = await getLocale();
    final detailed = await Future.wait(pairs.map((p) => getById(p.id)));
    return List.generate(detailed.length, (i) => ItemType(
      id: detailed[i].id,
      name: pairs[i].name,
      keys: detailed[i].keys,
      etag: detailed[i].etag,
    ));
  }

  /// Returns id/name pairs for all types in the current locale.
  Future<List<({int id, String name})>> getLocale() async {
    final response = await _client.dio.get('/api/types/locale');
    final list = response.data as List;
    return list.map((e) {
      final m = e as Map<String, dynamic>;
      return (id: m['id'] as int, name: m['name'] as String);
    }).toList();
  }
}
