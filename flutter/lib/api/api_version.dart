// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// REST API version constants and the compatibility check for this client.
///
/// Mirrors the server-side `colman_rest_api_version.hpp`: the version is a
/// `{major, minor}` pair. The major number changes on backwards-incompatible
/// changes, the minor number on backwards-compatible additions.
library;

/// Major REST API version this client requires.
///
/// Must equal the server's major version; a mismatch means the server speaks
/// an incompatible protocol.
const int clientApiMajor = 2;

/// Minimum REST API minor version this client requires.
///
/// Raise this only when the client starts relying on an endpoint or field that
/// was added in a newer server minor version. A server reporting a lower minor
/// is missing features this client build depends on.
///
/// Minor 2: the client computes person barcodes in format 2, which older
/// servers cannot parse.
///
/// Minor 3: the client extends lending periods (`POST /api/lendings/extend`)
/// and filters the history by action type (`GET /api/history?action=`).
const int clientApiMinor = 3;

/// Version reported by the server via `GET /api/version`.
class ServerApiVersion {
  const ServerApiVersion({
    required this.major,
    required this.minor,
    this.name = '',
  });

  /// Parses the JSON object returned by `GET /api/version`.
  factory ServerApiVersion.fromJson(Map<String, dynamic> json) {
    return ServerApiVersion(
      major: (json['major'] as num?)?.toInt() ?? 0,
      minor: (json['minor'] as num?)?.toInt() ?? 0,
      name: json['name'] as String? ?? '',
    );
  }

  final int major;
  final int minor;

  /// Server display name (present from minor 1 onwards), may be empty.
  final String name;

  /// `"major.minor"`, e.g. `"1.18"`.
  String get label => '$major.$minor';

  /// `true` when this server version is compatible with the built-in client
  /// version: same major number and server minor ≥ the client's minimum minor.
  bool get isCompatible =>
      major == clientApiMajor && minor >= clientApiMinor;
}

/// Thrown during login when the server's REST API version is incompatible with
/// this client build.
class IncompatibleServerException implements Exception {
  const IncompatibleServerException(this.server);

  /// The version reported by the server.
  final ServerApiVersion server;

  /// `"major.minor"` the client requires (its minimum acceptable version).
  String get requiredLabel => '$clientApiMajor.$clientApiMinor';

  @override
  String toString() =>
      'IncompatibleServerException: server ${server.label}, '
      'client requires $requiredLabel';
}
