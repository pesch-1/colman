// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'api_client.dart';

/// Reads the known signature categories from the server, used to suggest
/// signatures in the signature-print screen.
class SignaturesApi {
  SignaturesApi(this._client);

  final ApiClient _client;

  /// Known top (upper) signature categories.
  Future<List<String>> top() async {
    final response = await _client.dio.get('/api/signatures/top');
    return (response.data as List).map((e) => '$e').toList();
  }

  /// Known bottom (lower) signature categories.
  Future<List<String>> bottom() async {
    final response = await _client.dio.get('/api/signatures/bottom');
    return (response.data as List).map((e) => '$e').toList();
  }
}
