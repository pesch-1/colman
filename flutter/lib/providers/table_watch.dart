// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/widgets.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'table_versions_provider.dart';

/// Calls [onChanged] whenever the server-side change counter of any table in
/// [tables] advances, as reported by periodic `/api/table-versions` polling.
///
/// Call this from a `ConsumerState.build` (or `ConsumerWidget.build`) of a
/// data-bearing screen, passing the table names the screen depends on and its
/// existing reload method, e.g.:
///
/// ```dart
/// watchTables(ref, const ['lendings', 'items', 'persons'], _load);
/// ```
///
/// Watching keeps the background poller alive while the screen is shown. The
/// initial load is ignored (the screen loads itself in `initState`); only
/// genuine subsequent changes trigger [onChanged].
void watchTables(WidgetRef ref, List<String> tables, VoidCallback onChanged) {
  ref.listen<AsyncValue<Map<String, int>>>(tableVersionsProvider, (prev, next) {
    final n = next.value;
    final p = prev?.value;
    // Ignore the initial loading -> data transition; react only to real
    // change events that arrive while the screen is mounted.
    if (n == null || p == null) return;
    final changed = tables.any((t) => (p[t] ?? -1) < (n[t] ?? -1));
    if (changed) onChanged();
  });
}
