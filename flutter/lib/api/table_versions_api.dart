// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'api_client.dart';

class TableVersionsApi {
  const TableVersionsApi(this._client);

  final ApiClient _client;

  Future<Map<String, int>> getVersions() async {
    final response = await _client.dio.get('/api/table-versions');
    final data = response.data as Map<String, dynamic>;
    return data.map((k, v) => MapEntry(k, v as int));
  }
}
