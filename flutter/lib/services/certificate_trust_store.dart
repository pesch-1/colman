// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

/// Persists trusted TLS certificate fingerprints per hostname.
///
/// Fingerprints are SHA-256 digests of the DER-encoded certificate, stored as
/// lowercase hex bytes separated by colons (e.g. `"aa:bb:cc:..."`).
///
/// Call [initialize] once at app startup before creating any [ApiClient].
class CertificateTrustStore {
  static const _storage = FlutterSecureStorage();
  static const _keyPrefix = 'trusted_cert_';

  final Map<String, String> _cache = {};

  /// Loads all previously trusted fingerprints from secure storage.
  Future<void> initialize() async {
    final all = await _storage.readAll();
    for (final entry in all.entries) {
      if (entry.key.startsWith(_keyPrefix)) {
        final host = entry.key.substring(_keyPrefix.length);
        _cache[host] = entry.value;
      }
    }
  }

  /// Returns true when [fingerprint] was previously accepted for [host].
  bool isTrusted(String host, String fingerprint) =>
      _cache[host] == fingerprint;

  /// Persists [fingerprint] as trusted for [host].
  Future<void> trust(String host, String fingerprint) async {
    _cache[host] = fingerprint;
    await _storage.write(key: '$_keyPrefix$host', value: fingerprint);
  }
}

/// Singleton instance initialised in [main].
final certificateTrustStore = CertificateTrustStore();
