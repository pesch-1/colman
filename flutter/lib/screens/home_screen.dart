// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../providers/auth_provider.dart';
import 'barcode_scan_screen.dart';

/// Application home screen showing a grid of navigation cards.
///
/// Cards for privileged operations (e.g. person management) are only
/// shown when the logged-in user holds the corresponding permission.
class HomeScreen extends ConsumerWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canEditPersons =
        auth is Authenticated && auth.hasPermission('edit_persons');
    final canViewHistory =
        auth is Authenticated && auth.hasPermission('view_history');
    final canReserve =
        auth is Authenticated && auth.hasPermission('reserve');
    final canEditPreferences =
        auth is Authenticated &&
        (auth.hasPermission('edit_preferences') ||
            auth.hasPermission('edit_program_settings'));
    final canCollectLendings =
        auth is Authenticated && auth.hasPermission('collect_lendings');
    final canCollectReturns =
        auth is Authenticated && auth.hasPermission('collect_returns');
    final canCollectAcquisitions =
        auth is Authenticated && auth.hasPermission('collect_acquisitions');
    final canCollectLiquidations =
        auth is Authenticated && auth.hasPermission('collect_liquidations');
    final canViewItems =
        auth is Authenticated && auth.hasPermission('view_items');
    final canManageClasses =
        auth is Authenticated && auth.hasPermission('manage_classes');
    final canEditCategories =
        auth is Authenticated && auth.hasPermission('edit_categories');
    final canEditIdLayout =
        auth is Authenticated && auth.hasPermission('edit_id_layout');
    final canPrintIds =
        auth is Authenticated && auth.hasPermission('print_ids');
    final isAdmin = auth is Authenticated && auth.isAdmin;
    final isAuthenticated = auth is Authenticated;
    return Scaffold(
      appBar: AppBar(
        title: Row(
          children: [
            Image.asset('assets/icons/colman_logo.jpeg', height: 32),
            const SizedBox(width: 10),
            Text(l10n.appTitle),
          ],
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.logout),
            tooltip: l10n.signOut,
            onPressed: () => ref.read(authProvider.notifier).logout(),
          ),
        ],
      ),
      body: GridView(
        padding: const EdgeInsets.all(12),
        gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(
          maxCrossAxisExtent: 140,
          mainAxisExtent: 120,
          mainAxisSpacing: 10,
          crossAxisSpacing: 10,
        ),
        children: [
          _HomeCard(
            icon: 'assets/icons/scan_barcode.jpeg',
            label: l10n.navScan,
            onTap: () async {
              final code = await Navigator.of(context).push<String>(
                MaterialPageRoute(
                  builder: (_) => const BarcodeScanScreen(),
                ),
              );
              if(code != null && context.mounted) {
                context.push('/barcode-dispatch', extra: code);
              }
            },
          ),
          if(canCollectLendings)
            _HomeCard(
              icon: 'assets/icons/lending.jpeg',
              label: l10n.navLend,
              onTap: () => context.push('/lending'),
            ),
          if(canCollectReturns)
            _HomeCard(
              icon: 'assets/icons/returns.jpeg',
              label: l10n.navReturn,
              onTap: () => context.push('/return'),
            ),
          if(canViewHistory)
            _HomeCard(
              icon: 'assets/icons/waiting.jpeg',
              label: l10n.navOpenLendings,
              onTap: () => context.push('/open-lendings'),
            ),
          if(!canViewHistory)
            _HomeCard(
              icon: 'assets/icons/waiting.jpeg',
              label: l10n.navMyLendings,
              onTap: () => context.push('/my-lendings'),
            ),
          if(!canViewHistory && canReserve)
            _HomeCard(
              icon: 'assets/icons/reserve2.jpeg',
              label: l10n.navMyReservations,
              onTap: () => context.push('/my-reservations'),
            ),
          if(canViewHistory)
            _HomeCard(
              icon: 'assets/icons/list_history.jpeg',
              label: l10n.navHistory,
              onTap: () => context.push('/history'),
            ),
          if(canEditPersons)
            _HomeCard(
              icon: 'assets/icons/edit_persons.jpeg',
              label: l10n.navPersons,
              onTap: () => context.push('/persons'),
            ),
          if(canCollectAcquisitions)
            _HomeCard(
              icon: 'assets/icons/add_book.jpeg',
              label: l10n.navAcquire,
              onTap: () => context.push('/acquisition'),
            ),
          if(canCollectLiquidations)
            _HomeCard(
              icon: 'assets/icons/remove_book.jpeg',
              label: l10n.navLiquidate,
              onTap: () => context.push('/liquidation'),
            ),
          if(canViewItems)
            _HomeCard(
              icon: 'assets/icons/search.jpeg',
              label: l10n.navSearch,
              onTap: () => context.push('/items'),
            ),
          if(canManageClasses)
            _HomeCard(
              icon: 'assets/icons/pupils.jpeg',
              label: l10n.navClasses,
              onTap: () => context.push('/classes'),
            ),
          if(canEditCategories)
            _HomeCard(
              icon: 'assets/icons/edit_types.jpeg',
              label: l10n.navCategories,
              onTap: () => context.push('/categories'),
            ),
          if(canPrintIds)
            _HomeCard(
              icon: 'assets/icons/print_id.jpeg',
              label: l10n.navIdCardPrint,
              onTap: () => context.push('/id-card-print'),
            ),
          if(canViewItems)
            _HomeCard(
              icon: 'assets/icons/print_label.jpeg',
              label: l10n.navLabelPrint,
              onTap: () => context.push('/label-print'),
            ),
          if(canViewItems)
            _HomeCard(
              icon: 'assets/icons/print_signatures.jpeg',
              label: l10n.navSignaturePrint,
              onTap: () => context.push('/signature-print'),
            ),
          if(canViewItems)
            _HomeCard(
              icon: 'assets/icons/edit_label_layouts.jpeg',
              label: l10n.navLabelPresets,
              onTap: () => context.push('/label-presets'),
            ),
          if(canEditIdLayout)
            _HomeCard(
              icon: 'assets/icons/edit_id.jpeg',
              label: l10n.navIdCardLayout,
              onTap: () => context.push('/id-card-layout'),
            ),
          if(canEditPreferences)
            _HomeCard(
              icon: 'assets/icons/preferences.jpeg',
              label: l10n.navSettings,
              onTap: () => context.push('/settings'),
            ),
          if(isAdmin)
            _HomeCard(
              icon: 'assets/icons/edit_passwords.jpeg',
              label: l10n.navChangePasswords,
              onTap: () => context.push('/change-passwords'),
            ),
          if(isAuthenticated && !isAdmin)
            _HomeCard(
              icon: 'assets/icons/change_password.jpeg',
              label: l10n.navChangePassword,
              onTap: () => context.push('/change-password'),
            ),
        ],
      ),
    );
  }
}

/// A single tile in the home-screen navigation grid.
class _HomeCard extends StatelessWidget {
  const _HomeCard({
    required this.icon,
    required this.label,
    required this.onTap,
  });

  final String icon;
  final String label;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return Card(
      clipBehavior: Clip.antiAlias,
      elevation: 2,
      child: InkWell(
        onTap: onTap,
        child: Padding(
          padding: const EdgeInsets.all(8),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Flexible(
                child: Image.asset(icon, height: 80, fit: BoxFit.contain),
              ),
              const SizedBox(height: 6),
              Text(
                label,
                style: Theme.of(context).textTheme.labelLarge,
                textAlign: TextAlign.center,
              ),
            ],
          ),
        ),
      ),
    );
  }
}
