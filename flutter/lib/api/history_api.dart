// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';

import '../models/history_row.dart';
import 'api_client.dart';

class HistoryApi {
  const HistoryApi(this._client);

  final ApiClient _client;

  /// Returns history rows; [action] restricts them to one action type
  /// (`acquisition`, `liquidation`, `lending`, `return`, `reservation`).
  Future<List<HistoryRow>> getHistory({
    required bool isAdmin,
    required DateTime from,
    required DateTime to,
    required bool showAll,
    required int limit,
    required int offset,
    String? action,
  }) async {
    final response = await _client.dio.get(
      '/api/history',
      queryParameters: {
        'is_admin': isAdmin,
        'from': _fmtDate(from),
        'to': _fmtDate(to),
        'show_all': showAll,
        'limit': limit,
        'offset': offset,
        if (action != null && action.isNotEmpty) 'action': action,
      },
    );
    final list = response.data as List;
    return list
        .map((e) => HistoryRow.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Returns all actions that depend on [row] and would be broken by deleting it.
  Future<List<HistoryRow>> getConflicts(HistoryRow row) async {
    final response = await _client.dio.post(
      '/api/history/conflicts',
      data: row.toJson(),
    );
    final list = response.data as List;
    return list
        .map((e) => HistoryRow.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Returns all actions that would be affected by changing [oldRow] to [newRow].
  Future<List<HistoryRow>> getChangeConflicts(
      HistoryRow oldRow, HistoryRow newRow) async {
    final response = await _client.dio.post(
      '/api/history/change-conflicts',
      data: {'old': oldRow.toJson(), 'new': newRow.toJson()},
    );
    final list = response.data as List;
    return list
        .map((e) => HistoryRow.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  Future<void> changeAction(HistoryRow oldRow, HistoryRow newRow) async {
    await _client.dio.put(
      '/api/history',
      data: {'old': oldRow.toJson(), 'new': newRow.toJson()},
    );
  }

  Future<void> deleteActions(List<HistoryRow> rows) async {
    await _client.dio.delete(
      '/api/history',
      data: rows.map((r) => r.toJson()).toList(),
      options: Options(headers: {'Content-Type': 'application/json'}),
    );
  }
}

String _fmtDate(DateTime d) =>
    '${d.year.toString().padLeft(4, '0')}-'
    '${d.month.toString().padLeft(2, '0')}-'
    '${d.day.toString().padLeft(2, '0')}';
