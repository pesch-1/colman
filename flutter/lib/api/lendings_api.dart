// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';

import '../models/due_lending_row.dart';
import '../models/lending.dart';
import 'api_client.dart';

/// Thrown when `POST /api/lendings` returns HTTP 409 because some or all
/// copies of the requested item are blocked by another person's reservation.
///
/// [notReservedCount] indicates how many copies could still be lent because
/// they are not covered by any reservation.
class LendingConflictException implements Exception {
  /// Creates a [LendingConflictException] with the given available count.
  const LendingConflictException(this.notReservedCount);

  /// Number of copies available for lending despite the conflict.
  final int notReservedCount;
}

/// Thrown when `POST /api/lendings` returns HTTP 409 because no (or not enough)
/// copies of the requested item are available — e.g. it has meanwhile been
/// lent out by someone else.
///
/// [availableCount] is the number of copies currently available.
class LendingUnavailableException implements Exception {
  /// Creates a [LendingUnavailableException] with the available count.
  const LendingUnavailableException(this.availableCount);

  /// Number of copies currently available (0 when fully lent out).
  final int availableCount;
}

/// REST client for the `/api/lendings` and `/api/lendings/due` endpoints.
class LendingsApi {
  /// Creates a [LendingsApi] backed by [_client].
  const LendingsApi(this._client);

  final ApiClient _client;

  /// Returns the full lending record for [id], including its ETag.
  Future<Lending> getById(int id) async {
    final response = await _client.dio.get('/api/lendings/$id');
    final etag = response.headers.value('ETag');
    return Lending.fromJson(response.data as Map<String, dynamic>, etag: etag);
  }

  /// Creates a new lending and returns the new lending id.
  ///
  /// The server responds with `{"id": N}` (HTTP 201); the full record is not
  /// returned, so only the id is extracted — matching the reservations pattern.
  ///
  /// Throws [LendingConflictException] (HTTP 409) when some copies are blocked
  /// by another user's reservation; [notReservedCount] then tells how many
  /// unreserved copies remain.
  Future<int> create(Map<String, dynamic> data) async {
    try {
      final response = await _client.dio.post('/api/lendings', data: data);
      return (response.data['id'] as num).toInt();
    } on DioException catch (e) {
      if (e.response?.statusCode == 409) {
        final body = e.response?.data;
        final map = body is Map ? body : const {};
        if (map['error'] == 'not_available') {
          final available = (map['available_count'] as num?)?.toInt() ?? 0;
          throw LendingUnavailableException(available);
        }
        final notReserved = (map['not_reserved_count'] as num?)?.toInt() ?? 0;
        throw LendingConflictException(notReserved);
      }
      rethrow;
    }
  }

  /// Updates lending [id] with [data], using [etag] for optimistic concurrency.
  ///
  /// The server responds with an empty body on success, so the updated
  /// lending is re-fetched to obtain the new ETag.
  Future<Lending> update(int id, Map<String, dynamic> data, String etag) async {
    await _client.dio.put(
      '/api/lendings/$id',
      data: data,
      options: Options(headers: {'If-Match': etag}),
    );
    return getById(id);
  }

  /// Deletes lending [id].
  Future<void> delete(int id) async {
    await _client.dio.delete('/api/lendings/$id');
  }

  /// Sets [duedate] as the new due date of all lendings in [lendingIds].
  ///
  /// The server changes all lendings in one transaction: if one of them does
  /// not exist, none is changed (HTTP 404).  Requires `collect_lendings`.
  ///
  /// Calls `POST /api/lendings/extend` (REST API 2.3).
  Future<void> extend(List<int> lendingIds, DateTime duedate) async {
    await _client.dio.post(
      '/api/lendings/extend',
      data: {'lending_ids': lendingIds, 'duedate': _fmtDate(duedate)},
    );
  }

  /// Returns all lendings for person [personId] as compact info records.
  Future<List<Lending>> getByPerson(int personId) async {
    final response = await _client.dio.get('/api/lendings/person/$personId');
    final list = response.data as List;
    return list
        .map((e) => Lending.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Returns open lendings optionally filtered by date range, person, or class.
  ///
  /// When [from] and [to] are both omitted (null), the server skips the date
  /// filter and returns **all** currently open lendings — this is how the Qt
  /// REST client populates its "Open Lendings" view.
  ///
  /// Pass [personId] (>= 0) to restrict results to one borrower.
  /// Pass [schoolClass] (non-empty) to restrict results to one class.
  ///
  /// Calls `GET /api/lendings/due`.
  Future<List<DueLendingRow>> getDue({
    DateTime? from,
    DateTime? to,
    int? personId,
    String? schoolClass,
  }) async {
    final params = <String, dynamic>{};
    if (from != null) params['from'] = _fmtDate(from);
    if (to != null) params['to'] = _fmtDate(to);
    if (personId != null && personId >= 0) params['person_id'] = personId;
    if (schoolClass != null && schoolClass.isNotEmpty) {
      params['school_class'] = schoolClass;
    }
    final response = await _client.dio.get(
      '/api/lendings/due',
      queryParameters: params.isEmpty ? null : params,
    );
    final list = response.data as List;
    return list
        .map((e) => DueLendingRow.fromJson(e as Map<String, dynamic>))
        .toList();
  }
}

/// Formats [d] as an ISO 8601 date string (YYYY-MM-DD).
String _fmtDate(DateTime d) => d.toIso8601String().substring(0, 10);
