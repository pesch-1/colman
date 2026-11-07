// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';

import '../models/reservation.dart';
import 'api_client.dart';

/// Reasons why creating a reservation can fail.
enum ReservationError {
  alreadyExists,
  notEnoughAvailable,
  disabled,
  other,
}

/// Thrown when a reservation cannot be created.
class ReservationException implements Exception {
  const ReservationException(this.error);

  final ReservationError error;
}

/// REST client for `/api/reservations` and `/api/items/<id>/reservations`.
class ReservationsApi {
  const ReservationsApi(this._client);

  final ApiClient _client;

  /// Returns all active reservations of the currently logged-in user.
  Future<List<Reservation>> getMine() async {
    final response = await _client.dio.get('/api/reservations');
    final list = response.data as List;
    return list
        .map((e) => Reservation.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Returns the collection-wide default reservation period in days,
  /// or `null` when the collection does not define one.
  Future<int?> defaultPeriodDays() async {
    final response = await _client.dio.get('/api/collection');
    final data = response.data as Map<String, dynamic>;
    return data['reservation_period_days'] as int?;
  }

  /// Returns all active reservations for item [itemId].
  Future<List<Reservation>> getForItem(int itemId) async {
    final response =
        await _client.dio.get('/api/items/$itemId/reservations');
    final list = response.data as List;
    return list
        .map((e) => Reservation.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Creates a reservation and returns the new reservation id.
  ///
  /// Throws [ReservationException] for known server-side failures
  /// (already exists, not enough copies, reservations disabled).
  Future<int> create({
    required int itemId,
    required int personId,
    required int count,
    required DateTime expiresAt,
  }) async {
    try {
      final response = await _client.dio.post(
        '/api/reservations',
        data: {
          'item_id': itemId,
          'person_id': personId,
          'count': count,
          'expires_at': expiresAt.toIso8601String().substring(0, 10),
        },
      );
      return response.data['id'] as int;
    } on DioException catch(e) {
      final status = e.response?.statusCode;
      if(status == 409) {
        throw const ReservationException(ReservationError.alreadyExists);
      }
      if(status == 422) {
        throw const ReservationException(ReservationError.notEnoughAvailable);
      }
      if(status == 403) {
        final body = e.response?.data;
        if(body is Map &&
            body['error'] == 'reservations_disabled_for_item') {
          throw const ReservationException(ReservationError.disabled);
        }
      }
      throw const ReservationException(ReservationError.other);
    }
  }

  /// Cancels the reservation with [reservationId].
  Future<void> cancel(int reservationId) async {
    await _client.dio.delete('/api/reservations/$reservationId');
  }
}
