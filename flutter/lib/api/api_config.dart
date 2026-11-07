// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
class ApiConfig {
  const ApiConfig({
    required this.baseUrl,
    this.token,
    this.username,
    this.password,
    this.timeoutMs = 30000,
  }) : assert(
          token != null || (username != null && password != null),
          'Either token or username+password must be provided',
        );

  final String baseUrl;
  final String? token;
  final String? username;
  final String? password;
  final int timeoutMs;
}
