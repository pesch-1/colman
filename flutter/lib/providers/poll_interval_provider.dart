// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

/// Client-local polling interval (in seconds) for server change detection.
///
/// Mirrors the Qt client's `poll_interval_s` preference. Stored locally per
/// device (not on the server), defaulting to [defaultSeconds] and clamped to
/// [minSeconds]..[maxSeconds].
class PollIntervalNotifier extends AsyncNotifier<int> {
  static const _storage = FlutterSecureStorage();
  static const _key = 'poll_interval_s';

  /// Default polling interval when none has been configured.
  static const defaultSeconds = 10;

  /// Smallest allowed polling interval.
  static const minSeconds = 5;

  /// Largest allowed polling interval.
  static const maxSeconds = 300;

  @override
  Future<int> build() async {
    final raw = await _storage.read(key: _key);
    final value = int.tryParse(raw ?? '') ?? defaultSeconds;
    return value.clamp(minSeconds, maxSeconds);
  }

  /// Persists a new polling interval (clamped) and updates the state.
  Future<void> set(int seconds) async {
    final clamped = seconds.clamp(minSeconds, maxSeconds);
    await _storage.write(key: _key, value: clamped.toString());
    state = AsyncValue.data(clamped);
  }
}

final pollIntervalProvider = AsyncNotifierProvider<PollIntervalNotifier, int>(
  PollIntervalNotifier.new,
);
