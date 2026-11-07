// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import 'permission_policy.dart';
import 'providers/auth_provider.dart';
import 'screens/acquisition_screen.dart';
import 'screens/barcode_dispatch_screen.dart';
import 'screens/categories_screen.dart';
import 'screens/change_own_password_screen.dart';
import 'screens/change_passwords_screen.dart';
import 'screens/classes_screen.dart';
import 'screens/history_screen.dart';
import 'screens/home_screen.dart';
import 'screens/id_card_layout_screen.dart';
import 'screens/id_card_print_screen.dart';
import 'screens/my_reservations_screen.dart';
import 'screens/item_detail_screen.dart';
import 'screens/item_edit_screen.dart';
import 'screens/items_screen.dart';
import 'screens/lending_screen.dart';
import 'screens/open_lendings_screen.dart';
import 'screens/liquidation_screen.dart'
    show LiquidationScreen, LiquidationInitialData;
import 'screens/login_screen.dart';
import 'screens/person_detail_screen.dart';
import 'screens/persons_screen.dart';
import 'screens/label_preset_editor_screen.dart';
import 'screens/label_print_screen.dart';
import 'screens/return_screen.dart';
import 'screens/settings_screen.dart';
import 'screens/signature_print_screen.dart';

class RouterNotifier extends ChangeNotifier {
  RouterNotifier(this._ref) {
    _ref.listen(authProvider, (_, __) => notifyListeners());
  }

  final Ref _ref;

  String? redirect(BuildContext context, GoRouterState state) {
    final auth = _ref.read(authProvider);
    if (auth.isLoading) return null;

    final authValue = auth.value;
    final isAuthenticated = authValue is Authenticated;
    final isLoginRoute = state.matchedLocation == '/login';

    if (!isAuthenticated && !isLoginRoute) return '/login';
    if (isAuthenticated && isLoginRoute) return '/';

    // Redirect to home when the user may not manage persons at all
    // (needs edit_persons, create_lenders or create_data_collectors).
    if (authValue is Authenticated &&
        state.matchedLocation.startsWith('/persons') &&
        !mayManagePersons(authValue.permissions,
            isAdmin: authValue.isAdmin)) {
      return '/';
    }
    // Redirect to home when the user lacks both settings permissions.
    if (authValue is Authenticated &&
        state.matchedLocation == '/settings' &&
        !authValue.hasPermission('edit_preferences') &&
        !authValue.hasPermission('edit_program_settings')) {
      return '/';
    }
    // Redirect to home when the user lacks the view_history permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/history' &&
        !authValue.hasPermission('view_history')) {
      return '/';
    }
    // Redirect to home when the user lacks the collect_lendings permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/lending' &&
        !authValue.hasPermission('collect_lendings')) {
      return '/';
    }
    // Redirect to home when the user lacks the collect_returns permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/return' &&
        !authValue.hasPermission('collect_returns')) {
      return '/';
    }
    // Redirect to home when the user lacks the collect_acquisitions permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/acquisition' &&
        !authValue.hasPermission('collect_acquisitions')) {
      return '/';
    }
    // Redirect to home when the user lacks the collect_liquidations permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/liquidation' &&
        !authValue.hasPermission('collect_liquidations')) {
      return '/';
    }
    // Redirect to home when the user lacks the collect_acquisitions
    // permission for item editing (checked before the broader view_items
    // guard below).
    if (authValue is Authenticated &&
        state.matchedLocation.startsWith('/items/') &&
        state.matchedLocation.endsWith('/edit') &&
        !authValue.hasPermission('collect_acquisitions')) {
      return '/';
    }
    // Redirect to home when the user lacks the view_items permission.
    if (authValue is Authenticated &&
        state.matchedLocation.startsWith('/items') &&
        !authValue.hasPermission('view_items')) {
      return '/';
    }
    // Redirect to home when the user lacks the manage_classes permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/classes' &&
        !authValue.hasPermission('manage_classes')) {
      return '/';
    }
    // Redirect to home when the user lacks the edit_categories permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/categories' &&
        !authValue.hasPermission('edit_categories')) {
      return '/';
    }
    // Redirect to home when the user lacks the edit_id_layout permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/id-card-layout' &&
        !authValue.hasPermission('edit_id_layout')) {
      return '/';
    }
    // Redirect to home when the user lacks the print_ids permission.
    if (authValue is Authenticated &&
        state.matchedLocation == '/id-card-print' &&
        !authValue.hasPermission('print_ids')) {
      return '/';
    }
    // Redirect to home when the user lacks the view_items permission
    // (label printing and the label-preset editor).
    if (authValue is Authenticated &&
        (state.matchedLocation == '/label-print' ||
            state.matchedLocation == '/label-presets' ||
            state.matchedLocation == '/signature-print') &&
        !authValue.hasPermission('view_items')) {
      return '/';
    }
    // The admin password-management screen is administrator-only.
    if (authValue is Authenticated &&
        state.matchedLocation == '/change-passwords' &&
        !authValue.isAdmin) {
      return '/';
    }
    return null;
  }
}

final routerProvider = Provider<GoRouter>((ref) {
  final notifier = RouterNotifier(ref);
  return GoRouter(
    initialLocation: '/',
    refreshListenable: notifier,
    redirect: notifier.redirect,
    routes: [
      GoRoute(
        path: '/login',
        builder: (_, __) => const LoginScreen(),
      ),
      GoRoute(
        path: '/',
        builder: (_, __) => const HomeScreen(),
      ),
      GoRoute(
        path: '/barcode-dispatch',
        builder: (_, state) => BarcodeDispatchScreen(
          barcode: state.extra as String? ?? '',
        ),
      ),
      GoRoute(
        path: '/lending',
        builder: (_, state) => LendingScreen(
          initialData: state.extra as LendingInitialData?,
        ),
      ),
      GoRoute(
        path: '/return',
        builder: (_, state) => ReturnScreen(
          initialData: state.extra as ReturnInitialData?,
        ),
      ),
      GoRoute(
        path: '/acquisition',
        builder: (_, state) => AcquisitionScreen(
          initialData: state.extra as AcquisitionInitialData?,
        ),
      ),
      GoRoute(
        path: '/liquidation',
        builder: (_, state) => LiquidationScreen(
          initialData: state.extra as LiquidationInitialData?,
        ),
      ),
      GoRoute(
        path: '/persons',
        builder: (_, __) => const PersonsScreen(),
        routes: [
          GoRoute(
            path: ':id',
            builder: (_, state) => PersonDetailScreen(
              personId: int.parse(state.pathParameters['id']!),
            ),
          ),
        ],
      ),
      GoRoute(
        path: '/items',
        builder: (_, __) => const ItemsScreen(),
        routes: [
          GoRoute(
            path: ':id',
            builder: (_, state) => ItemDetailScreen(
              itemId: int.parse(state.pathParameters['id']!),
            ),
            routes: [
              GoRoute(
                path: 'edit',
                builder: (_, state) => ItemEditScreen(
                  itemId: int.parse(state.pathParameters['id']!),
                ),
              ),
            ],
          ),
        ],
      ),
      GoRoute(
        path: '/open-lendings',
        builder: (_, __) => const OpenLendingsScreen(),
      ),
      GoRoute(
        path: '/my-lendings',
        builder: (_, __) {
          final auth = ref.read(authProvider).value;
          final personId =
              auth is Authenticated ? auth.currentUserId : -1;
          return OpenLendingsScreen(lockedPersonId: personId);
        },
      ),
      GoRoute(
        path: '/my-reservations',
        builder: (_, __) => const MyReservationsScreen(),
      ),
      GoRoute(
        path: '/history',
        builder: (_, __) => const HistoryScreen(),
      ),
      GoRoute(
        path: '/classes',
        builder: (_, __) => const ClassesScreen(),
      ),
      GoRoute(
        path: '/categories',
        builder: (_, __) => const CategoriesScreen(),
      ),
      GoRoute(
        path: '/id-card-layout',
        builder: (_, __) => const IdCardLayoutScreen(),
      ),
      GoRoute(
        path: '/id-card-print',
        builder: (_, __) => const IdCardPrintScreen(),
      ),
      GoRoute(
        path: '/label-print',
        builder: (_, state) => LabelPrintScreen(
          initialData: state.extra as LabelPrintInitialData?,
        ),
      ),
      GoRoute(
        path: '/label-presets',
        builder: (_, __) => const LabelPresetEditorScreen(),
      ),
      GoRoute(
        path: '/signature-print',
        builder: (_, __) => const SignaturePrintScreen(),
      ),
      GoRoute(
        path: '/settings',
        builder: (_, __) => const SettingsScreen(),
      ),
      GoRoute(
        path: '/change-password',
        builder: (_, __) => const ChangeOwnPasswordScreen(),
      ),
      GoRoute(
        path: '/change-passwords',
        builder: (_, __) => const ChangePasswordsScreen(),
      ),
    ],
  );
});
