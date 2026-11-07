// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'providers/table_versions_provider.dart';
import 'router.dart';

class ColmanApp extends ConsumerWidget {
  const ColmanApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final router = ref.watch(routerProvider);
    return MaterialApp.router(
      title: 'Colman',
      builder: (context, child) =>
          _ReachabilityListener(child: child ?? const SizedBox.shrink()),
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFF4682D2),
          primary: const Color(0xFF4682D2),
          onPrimary: Colors.white,
          secondary: const Color(0xFF1E50AA),
        ),
        useMaterial3: true,
        appBarTheme: const AppBarTheme(
          backgroundColor: Color(0xFF4682D2),
          foregroundColor: Colors.white,
          elevation: 2,
        ),
        filledButtonTheme: FilledButtonThemeData(
          style: FilledButton.styleFrom(
            backgroundColor: const Color(0xFF4682D2),
            foregroundColor: Colors.white,
          ),
        ),
      ),
      localizationsDelegates: AppLocalizations.localizationsDelegates,
      supportedLocales: AppLocalizations.supportedLocales,
      routerConfig: router,
    );
  }
}

/// Shows a persistent [MaterialBanner] while the server is unreachable, driven
/// by [serverReachableProvider]. Wraps the routed page so it sits below the
/// [ScaffoldMessenger] provided by [MaterialApp].
class _ReachabilityListener extends ConsumerStatefulWidget {
  const _ReachabilityListener({required this.child});

  final Widget child;

  @override
  ConsumerState<_ReachabilityListener> createState() =>
      _ReachabilityListenerState();
}

class _ReachabilityListenerState extends ConsumerState<_ReachabilityListener> {
  @override
  Widget build(BuildContext context) {
    ref.listen<bool>(serverReachableProvider, (prev, reachable) {
      final messenger = ScaffoldMessenger.of(context);
      messenger.clearMaterialBanners();
      if (!reachable) {
        final l10n = AppLocalizations.of(context)!;
        messenger.showMaterialBanner(
          MaterialBanner(
            backgroundColor: Theme.of(context).colorScheme.errorContainer,
            leading: const Icon(Icons.cloud_off),
            content: Text(l10n.serverUnreachableBanner),
            actions: [
              TextButton(
                onPressed: messenger.clearMaterialBanners,
                child: Text(l10n.dismiss),
              ),
            ],
          ),
        );
      }
    });
    return widget.child;
  }
}
