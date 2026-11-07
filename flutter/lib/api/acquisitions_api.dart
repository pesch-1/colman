// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'api_client.dart';

/// REST client for the `/api/acquisitions` endpoints.
class AcquisitionsApi {
  /// Creates an [AcquisitionsApi] backed by [_client].
  const AcquisitionsApi(this._client);

  final ApiClient _client;

  /// Creates a new acquisition and returns the new acquisition id.
  ///
  /// The server responds with `{"id": N}` (HTTP 201).
  Future<int> create(Map<String, dynamic> data) async {
    final response = await _client.dio.post('/api/acquisitions', data: data);
    return (response.data['id'] as num).toInt();
  }

  /// Deletes acquisition [id].
  Future<void> delete(int id) async {
    await _client.dio.delete('/api/acquisitions/$id');
  }
}
