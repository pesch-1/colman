// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';

import '../models/change_password_person.dart';
import '../models/person.dart';
import 'api_client.dart';

/// REST client for the `/api/persons` family of endpoints.
///
/// Covers CRUD for persons as well as username management, password
/// management, and per-person permission management.
class PersonsApi {
  const PersonsApi(this._client);

  final ApiClient _client;

  // ── Person CRUD ────────────────────────────────────────────────────────────

  /// Searches for persons whose name matches [query].
  ///
  /// Uses `GET /api/persons/start-page?search=…`.
  Future<List<Person>> search(String query) async {
    final response = await _client.dio
        .get('/api/persons/start-page', queryParameters: {'search': query});
    final list = response.data as List;
    return list
        .map((e) => Person.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Fetches a single person by [id], including the current ETag for
  /// optimistic concurrency control.
  ///
  /// Uses `GET /api/persons/{id}`.
  Future<Person> getById(int id) async {
    final response = await _client.dio.get('/api/persons/$id');
    final etag = response.headers.value('ETag');
    return Person.fromJson(
        response.data as Map<String, dynamic>, etag: etag);
  }

  /// Creates a new person from [data] and returns the created record.
  ///
  /// The server responds with `{"id": N}` on success. This method fetches
  /// the full person via [getById] so the caller gets a complete [Person]
  /// including its initial ETag.
  ///
  /// Uses `POST /api/persons`. The server automatically creates default
  /// permissions for the new person.
  Future<Person> create(Map<String, dynamic> data) async {
    final response = await _client.dio.post('/api/persons', data: data);
    final id = (response.data as Map<String, dynamic>)['id'] as int;
    return getById(id);
  }

  /// Updates an existing person identified by [id] with [data].
  ///
  /// The [etag] value obtained from [getById] is sent as `If-Match` to
  /// prevent overwriting concurrent changes (optimistic locking). Throws
  /// a [DioException] with status 412 on conflict.
  ///
  /// The server returns 200 with an empty body on success, so this method
  /// re-fetches the person via [getById] to obtain the updated record and
  /// its new ETag for subsequent saves.
  ///
  /// Uses `PUT /api/persons/{id}`.
  Future<Person> update(int id, Map<String, dynamic> data, String etag) async {
    await _client.dio.put(
      '/api/persons/$id',
      data: data,
      options: Options(headers: {'If-Match': etag}),
    );
    return getById(id);
  }

  /// Deletes the person with [id].
  ///
  /// The server rejects the request if the person has open lendings.
  /// Uses `DELETE /api/persons/{id}`.
  Future<void> delete(int id) async {
    await _client.dio.delete('/api/persons/$id');
  }

  /// Looks up a person by their barcode.
  ///
  /// Returns `null` when no person with [barcode] exists.
  Future<Person?> getByBarcode(String barcode) async {
    final response =
        await _client.dio.get('/api/persons/by-barcode/$barcode');
    if (response.statusCode == 404) return null;
    return Person.fromJson(response.data as Map<String, dynamic>);
  }

  /// Looks up a person by their external UUID (Authentik account link).
  ///
  /// Returns `null` when no person with [uuid] exists.
  Future<Person?> getByExternalId(String uuid) async {
    final response =
        await _client.dio.get('/api/persons/by-external-id/$uuid');
    if (response.statusCode == 404) return null;
    return Person.fromJson(response.data as Map<String, dynamic>);
  }

  /// Returns auto-completion suggestions for person [field] starting with
  /// [prefix].
  ///
  /// Uses `GET /api/persons/completions/{field}?prefix=…`.
  Future<List<String>> getCompletions(String field, String prefix) async {
    final response = await _client.dio.get(
      '/api/persons/completions/$field',
      queryParameters: {'prefix': prefix},
    );
    return (response.data as List).cast<String>();
  }

  /// Searches for persons usable as borrowers in the lending wizard.
  ///
  /// Uses `GET /api/persons/lending-query?q=…`.
  Future<List<Person>> getLendingQuery(String query) async {
    final response = await _client.dio
        .get('/api/persons/lending-query', queryParameters: {'q': query});
    final list = response.data as List;
    return list
        .map((e) => Person.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  // ── Username ───────────────────────────────────────────────────────────────

  /// Returns the current username of person [id], or `null` if none is set.
  ///
  /// Uses `GET /api/persons/{id}/username`.
  Future<String?> getUsername(int id) async {
    try {
      final response = await _client.dio.get('/api/persons/$id/username');
      final data = response.data as Map<String, dynamic>;
      final u = data['username'];
      return u is String && u.isNotEmpty ? u : null;
    } on DioException catch (e) {
      if (e.response?.statusCode == 404) return null;
      rethrow;
    }
  }

  /// Sets the username for person [id] to [username].
  ///
  /// Throws on conflict (username taken) with a server-side error response.
  /// Uses `PUT /api/persons/{id}/username`.
  Future<void> setUsername(int id, String username) async {
    await _client.dio.put(
      '/api/persons/$id/username',
      data: {'username': username},
    );
  }

  /// Removes the username of person [id].
  ///
  /// Uses `DELETE /api/persons/{id}/username`.
  Future<void> removeUsername(int id) async {
    await _client.dio.delete('/api/persons/$id/username');
  }

  // ── Password ───────────────────────────────────────────────────────────────

  /// Returns `true` when a password hash is stored for person [id].
  ///
  /// Uses `GET /api/persons/{id}/password`.
  Future<bool> hasPassword(int id) async {
    final response = await _client.dio.get('/api/persons/$id/password');
    final data = response.data as Map<String, dynamic>;
    return data['has_password'] == true;
  }

  /// Stores an Argon2id-encoded [passwordHash] for person [id].
  ///
  /// The hash must be pre-computed client-side using the same parameters
  /// as `colman_passwords.cpp` (argon2id, t=2, m=19456 KB, p=1, len=32).
  /// Uses `POST /api/persons/{id}/password`.
  Future<void> setPassword(int id, String passwordHash) async {
    await _client.dio.post(
      '/api/persons/$id/password',
      data: {'password': passwordHash},
    );
  }

  /// Removes the stored password hash for person [id].
  ///
  /// Uses `DELETE /api/persons/{id}/password`.
  Future<void> removePassword(int id) async {
    await _client.dio.delete('/api/persons/$id/password');
  }

  /// Changes the logged-in user's own password.
  ///
  /// Sends the current password ([oldPassword], clear text over HTTPS) and the
  /// already Argon2id-hashed [newPasswordHash]. The server verifies the current
  /// password against the stored hash and rejects the request with HTTP 403 if
  /// it does not match. No special permission is required — the server
  /// identifies the caller from the authentication header.
  ///
  /// Uses `POST /api/me/password`.
  Future<void> changeOwnPassword(
      String oldPassword, String newPasswordHash) async {
    await _client.dio.post(
      '/api/me/password',
      data: {'old_password': oldPassword, 'new_password': newPasswordHash},
    );
  }

  /// Returns the list of all persons for the admin password-management screen.
  ///
  /// Each entry carries the id, name parts and current username. Uses
  /// `GET /api/persons/change-passwords` (requires `view_persons`).
  Future<List<ChangePasswordPerson>> queryChangePasswords() async {
    final response =
        await _client.dio.get('/api/persons/change-passwords');
    final list = response.data as List;
    return list
        .map((e) =>
            ChangePasswordPerson.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  // ── Permissions ────────────────────────────────────────────────────────────

  /// Returns the list of all known permission column names from the server.
  ///
  /// Uses `GET /api/permissions`.
  Future<List<String>> getPermissionNames() async {
    final response = await _client.dio.get('/api/permissions');
    return (response.data as List).cast<String>();
  }

  /// Returns the permission map for person [id].
  ///
  /// Fetches the permission column names and the boolean values in
  /// parallel and combines them into a single map.
  Future<Map<String, bool>> getPersonPermissions(int id) async {
    final namesFuture = getPermissionNames();
    final valuesFuture =
        _client.dio.get('/api/persons/$id/permissions');
    final names = await namesFuture;
    final valuesResp = await valuesFuture;
    final values = (valuesResp.data as List).cast<bool>();
    final result = <String, bool>{};
    for (var i = 0; i < names.length && i < values.length; i++) {
      result[names[i]] = values[i];
    }
    return result;
  }

  /// Sets the boolean [value] for a single permission [perm] of person [id].
  ///
  /// Uses `PUT /api/persons/{id}/permissions/{perm}`.
  Future<void> setPermission(int id, String perm, bool value) async {
    await _client.dio.put(
      '/api/persons/$id/permissions/$perm',
      data: {'value': value},
    );
  }

  /// Removes all permission entries for person [id].
  ///
  /// Uses `DELETE /api/persons/{id}/permissions`.
  Future<void> removePermissions(int id) async {
    await _client.dio.delete('/api/persons/$id/permissions');
  }
}
