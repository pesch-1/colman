// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'api_client.dart';

class LiquidationsApi {
  const LiquidationsApi(this._client);

  final ApiClient _client;

  Future<int> create(Map<String, dynamic> data) async {
    final response = await _client.dio.post('/api/liquidations', data: data);
    return (response.data as Map<String, dynamic>)['id'] as int;
  }

  Future<void> delete(int id) async {
    await _client.dio.delete('/api/liquidations/$id');
  }
}
