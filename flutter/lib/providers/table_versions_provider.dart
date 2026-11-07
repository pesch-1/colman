// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'api_client_provider.dart';
import 'auth_provider.dart';
import 'poll_interval_provider.dart';

/// Whether the most recent poll of `/api/table-versions` reached the server.
///
/// Flipped to `false` when a poll throws (server down / network error) and back
/// to `true` on the next successful poll. Screens / the app shell can watch this
/// to surface a "server unreachable" banner. Defaults to `true` (optimistic).
class ServerReachableNotifier extends Notifier<bool> {
  @override
  bool build() => true;

  /// Updates the reachability flag (no-op when unchanged).
  void set(bool reachable) {
    if (state != reachable) state = reachable;
  }
}

final serverReachableProvider = NotifierProvider<ServerReachableNotifier, bool>(
  ServerReachableNotifier.new,
);

/// Polls `/api/table-versions` at the configured interval and publishes the
/// full version map whenever any table's counter advances.
///
/// Screens observe this (via `watchTables`) to auto-refresh the data they show
/// when it changes on the server. Watching the provider also keeps the poller
/// alive; it auto-disposes once no screen observes it.
class TableVersionsNotifier extends AsyncNotifier<Map<String, int>> {
  Timer? _timer;
  Map<String, int> _lastKnown = {};

  @override
  Future<Map<String, int>> build() async {
    final auth = ref.watch(authProvider).value;
    if (auth is! Authenticated) return {};

    final intervalSec =
        ref.watch(pollIntervalProvider).value ??
        PollIntervalNotifier.defaultSeconds;

    _timer?.cancel();
    ref.onDispose(() => _timer?.cancel());

    Map<String, int> versions;
    try {
      versions = await _fetchVersions();
      _setReachable(true);
    } catch (_) {
      versions = {};
      _setReachable(false);
    }
    _lastKnown = versions;

    _timer = Timer.periodic(Duration(seconds: intervalSec), (_) async {
      try {
        final next = await _fetchVersions();
        _setReachable(true);
        final changed = next.entries.any(
          (e) => (_lastKnown[e.key] ?? -1) < e.value,
        );
        if (changed) {
          _lastKnown = next;
          state = AsyncValue.data(next);
        }
      } catch (_) {
        _setReachable(false);
      }
    });

    return versions;
  }

  void _setReachable(bool reachable) {
    ref.read(serverReachableProvider.notifier).set(reachable);
  }

  Future<Map<String, int>> _fetchVersions() async {
    return ref.read(tableVersionsApiProvider).getVersions();
  }
}

final tableVersionsProvider =
    AsyncNotifierProvider.autoDispose<TableVersionsNotifier, Map<String, int>>(
      TableVersionsNotifier.new,
    );
