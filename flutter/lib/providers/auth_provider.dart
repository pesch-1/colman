// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../api/api_client.dart';
import '../api/api_config.dart';
import '../api/api_version.dart';

/// Base class for authentication states.
sealed class AuthState {
  const AuthState();
}

/// The user is not authenticated.
class Unauthenticated extends AuthState {
  const Unauthenticated();
}

/// The user is authenticated and holds the active [ApiConfig] as well as
/// the permission map fetched from `/api/me` at login time.
class Authenticated extends AuthState {
  const Authenticated({
    required this.config,
    required this.currentUserId,
    this.permissions = const {},
  });

  final ApiConfig config;

  /// Person ID of the logged-in user. 0 indicates the administrator.
  final int currentUserId;

  /// Permission map from the server (`edit_persons`, `view_history`, …).
  ///
  /// The administrator (person_id == 0) implicitly holds every permission,
  /// so [hasPermission] returns `true` regardless of this map for admin users.
  final Map<String, bool> permissions;

  /// Returns `true` when the user is the administrator (person_id == 0).
  bool get isAdmin => currentUserId == 0;

  /// Returns `true` when the user holds [perm].
  ///
  /// Administrators always have every permission. Regular users have only
  /// the permissions set in their database row.
  bool hasPermission(String perm) {
    if (isAdmin) return true;
    return permissions[perm] == true;
  }
}

/// Manages the application authentication lifecycle.
///
/// Persists credentials in [FlutterSecureStorage] and restores them on
/// startup. Provides [loginWithPassword], [loginWithToken], and [logout]
/// to change the authentication state.
class AuthNotifier extends AsyncNotifier<AuthState> {
  static const _storage = FlutterSecureStorage();

  // Secure-storage keys owned by this notifier. Only these are cleared on
  // logout; other keys (e.g. internet-service tokens) must survive.
  static const _keyBaseUrl = 'base_url';
  static const _keyToken = 'token';
  static const _keyUsername = 'username';
  static const _keyPassword = 'password';

  @override
  Future<AuthState> build() async {
    return _loadSavedAuth();
  }

  /// Queries the server's REST API version and throws an
  /// [IncompatibleServerException] when it is not compatible with this client.
  ///
  /// Runs before credentials are validated; `/api/version` needs no
  /// authentication, so an outdated server is reported clearly instead of
  /// failing later through missing endpoints or fields.
  Future<void> _verifyServerVersion(ApiClient client) async {
    final version = await client.fetchApiVersion();
    if (!version.isCompatible) {
      throw IncompatibleServerException(version);
    }
  }

  Future<AuthState> _loadSavedAuth() async {
    final baseUrl = await _storage.read(key: _keyBaseUrl);
    if (baseUrl == null) return const Unauthenticated();

    final token = await _storage.read(key: _keyToken);
    final username = await _storage.read(key: _keyUsername);
    final password = await _storage.read(key: _keyPassword);

    final ApiConfig config;
    if (token != null) {
      config = ApiConfig(baseUrl: baseUrl, token: token);
    } else if (username != null && password != null) {
      config = ApiConfig(
          baseUrl: baseUrl, username: username, password: password);
    } else {
      return const Unauthenticated();
    }

    try {
      final client = ApiClient(config);
      await _verifyServerVersion(client);
      final user = await client.fetchCurrentUser();
      return Authenticated(
        config: config,
        currentUserId: user.userId,
        permissions: user.permissions,
      );
    } catch (_) {
      return const Unauthenticated();
    }
  }

  /// Authenticates with username/password (HTTP Basic Auth).
  Future<void> loginWithPassword(
    String baseUrl,
    String username,
    String password,
  ) async {
    state = const AsyncValue.loading();
    state = await AsyncValue.guard(() async {
      final config =
          ApiConfig(baseUrl: baseUrl, username: username, password: password);
      final client = ApiClient(config);
      await _verifyServerVersion(client);
      final user = await client.fetchCurrentUser();

      await _storage.write(key: _keyBaseUrl, value: baseUrl);
      await _storage.write(key: _keyUsername, value: username);
      await _storage.write(key: _keyPassword, value: password);
      await _storage.delete(key: _keyToken);

      return Authenticated(
        config: config,
        currentUserId: user.userId,
        permissions: user.permissions,
      );
    });
  }

  /// Authenticates with a Bearer token.
  Future<void> loginWithToken(String baseUrl, String token) async {
    state = const AsyncValue.loading();
    state = await AsyncValue.guard(() async {
      final config = ApiConfig(baseUrl: baseUrl, token: token);
      final client = ApiClient(config);
      await _verifyServerVersion(client);
      final user = await client.fetchCurrentUser();

      await _storage.write(key: _keyBaseUrl, value: baseUrl);
      await _storage.write(key: _keyToken, value: token);
      await _storage.delete(key: _keyUsername);
      await _storage.delete(key: _keyPassword);

      return Authenticated(
        config: config,
        currentUserId: user.userId,
        permissions: user.permissions,
      );
    });
  }

  /// Clears the stored credentials and transitions to [Unauthenticated].
  ///
  /// Only the authentication keys are removed; other secure-storage entries
  /// such as the internet-service tokens are intentionally left untouched.
  Future<void> logout() async {
    await _storage.delete(key: _keyBaseUrl);
    await _storage.delete(key: _keyToken);
    await _storage.delete(key: _keyUsername);
    await _storage.delete(key: _keyPassword);
    state = const AsyncValue.data(Unauthenticated());
  }
}

final authProvider =
    AsyncNotifierProvider<AuthNotifier, AuthState>(AuthNotifier.new);
