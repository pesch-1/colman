// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:dio/dio.dart';
import 'package:dio/io.dart';
import 'package:hashlib/hashlib.dart';

import '../services/certificate_trust_store.dart';
import 'api_config.dart';
import 'api_version.dart';

/// Thrown when the server presents a certificate that is not yet trusted.
///
/// The caller should show a confirmation dialog and, if the user accepts, call
/// [CertificateTrustStore.trust] and retry the request.
class CertificateException implements Exception {
  const CertificateException({
    required this.host,
    required this.fingerprint,
    required this.subject,
    required this.validFrom,
    required this.validUntil,
  });

  /// Hostname of the server.
  final String host;

  /// SHA-256 fingerprint of the DER-encoded certificate as colon-separated
  /// lowercase hex bytes (e.g. `"aa:bb:cc:..."`).
  final String fingerprint;

  /// Subject field from the certificate (e.g. `"CN=colman-nas"`).
  final String subject;

  final DateTime validFrom;
  final DateTime validUntil;

  @override
  String toString() => 'CertificateException: untrusted certificate for $host';
}

/// Result of fetching the current authenticated user from `/api/me`.
typedef CurrentUser = ({int userId, Map<String, bool> permissions});

/// Central HTTP client with automatic authentication headers.
///
/// Configures a [Dio] instance with the given [ApiConfig] and attaches
/// an interceptor that injects the appropriate `Authorization` header
/// (Bearer token or HTTP Basic) on every request.
///
/// When the server URL uses HTTPS, certificate trust is handled via
/// [CertificateTrustStore].  On the first connection to an untrusted
/// certificate a [CertificateException] is thrown so the caller can show a
/// confirmation dialog and retry after calling [CertificateTrustStore.trust].
class ApiClient {
  ApiClient(this.config) {
    _dio = Dio(BaseOptions(
      baseUrl: config.baseUrl,
      connectTimeout: Duration(milliseconds: config.timeoutMs),
      receiveTimeout: Duration(milliseconds: config.timeoutMs),
      headers: {'Content-Type': 'application/json'},
    ));

    _dio.interceptors.add(InterceptorsWrapper(
      onRequest: (options, handler) {
        if (config.token != null) {
          options.headers['Authorization'] = 'Bearer ${config.token}';
        } else {
          final credentials =
              base64Encode(utf8.encode('${config.username}:${config.password}'));
          options.headers['Authorization'] = 'Basic $credentials';
        }
        options.headers['Accept-Language'] =
            ui.PlatformDispatcher.instance.locale.languageCode;
        handler.next(options);
      },
    ));

    if (config.baseUrl.startsWith('https')) {
      _configureTlsHandling();
    }
  }

  final ApiConfig config;
  late final Dio _dio;
  CertificateException? _pendingCertException;

  Dio get dio => _dio;

  void _configureTlsHandling() {
    (_dio.httpClientAdapter as IOHttpClientAdapter).createHttpClient = () {
      final client = HttpClient();
      client.badCertificateCallback = (cert, host, port) {
        final fp = _fingerprint(cert.der);
        if (certificateTrustStore.isTrusted(host, fp)) {
          return true;
        }
        _pendingCertException = CertificateException(
          host: host,
          fingerprint: fp,
          subject: cert.subject,
          validFrom: cert.startValidity,
          validUntil: cert.endValidity,
        );
        return false;
      };
      return client;
    };
  }

  static String _fingerprint(Uint8List der) {
    final hash = sha256.convert(der);
    return hash.bytes
        .map((b) => b.toRadixString(16).padLeft(2, '0'))
        .join(':');
  }

  /// Wraps [call] and rethrows a [CertificateException] if the connection
  /// failed because of an untrusted certificate.
  Future<T> _withCertCheck<T>(Future<T> Function() call) async {
    _pendingCertException = null;
    try {
      return await call();
    } catch (_) {
      if (_pendingCertException != null) throw _pendingCertException!;
      rethrow;
    }
  }

  /// Fetches the current user's person ID and permission map from `/api/me`.
  Future<CurrentUser> fetchCurrentUser() => _withCertCheck(() async {
        final response = await _dio.get('/api/me');
        final data = response.data as Map<String, dynamic>;
        final userId = data['person_id'] as int;
        final permObj =
            (data['permissions'] as Map<String, dynamic>?) ?? const {};
        final permissions = permObj.map((k, v) => MapEntry(k, v as bool));
        return (userId: userId, permissions: permissions);
      });

  /// Fetches the server's REST API version from `/api/version`.
  ///
  /// This endpoint requires no authentication, so it can be queried before
  /// credentials are validated to detect an incompatible server early.
  Future<ServerApiVersion> fetchApiVersion() => _withCertCheck(() async {
        final response = await _dio.get('/api/version');
        return ServerApiVersion.fromJson(
            response.data as Map<String, dynamic>);
      });

  /// Verifies the server is reachable and credentials are valid.
  Future<void> testConnection() =>
      _withCertCheck(() => _dio.get('/api/collection'));
}
