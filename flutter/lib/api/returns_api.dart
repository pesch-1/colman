// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'api_client.dart';

/// REST client for the `/api/returns` endpoint.
class ReturnsApi {
  /// Creates a [ReturnsApi] backed by [_client].
  const ReturnsApi(this._client);

  final ApiClient _client;

  /// Records a new return and returns the new return id.
  ///
  /// [data] must contain `lending_id` (int) and `count` (int).
  /// `date_time` (ISO-8601 UTC) and `remarks` (string) are optional.
  /// The server responds with `{"id": N}` (HTTP 201).
  Future<int> create(Map<String, dynamic> data) async {
    final response = await _client.dio.post('/api/returns', data: data);
    return (response.data['id'] as num).toInt();
  }

  /// Deletes the return record with [id].
  Future<void> delete(int id) async {
    await _client.dio.delete('/api/returns/$id');
  }
}
