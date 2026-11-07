// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/reservation.dart';
import '../providers/api_client_provider.dart';
import '../providers/table_watch.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';

/// Screen listing the currently logged-in user's own active reservations.
class MyReservationsScreen extends ConsumerStatefulWidget {
  const MyReservationsScreen({super.key});

  @override
  ConsumerState<MyReservationsScreen> createState() =>
      _MyReservationsScreenState();
}

class _MyReservationsScreenState extends ConsumerState<MyReservationsScreen> {
  List<Reservation> _rows = [];
  Map<int, String> _itemNames = {};
  bool _loading = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    if (!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final rows = await ref.read(reservationsApiProvider).getMine();
      final itemIds = rows.map((r) => r.itemId).toSet();
      final names = <int, String>{};
      await Future.wait(
        itemIds.map((id) async {
          try {
            final item = await ref.read(itemsApiProvider).getById(id);
            names[id] = item.name;
          } catch (_) {
            names[id] = '#$id';
          }
        }),
      );
      if (!mounted) return;
      setState(() {
        _rows = rows;
        _itemNames = names;
        _loading = false;
      });
    } catch (e) {
      if (!mounted) return;
      final l10n = AppLocalizations.of(context)!;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(friendlyHttpError(e, l10n)),
          action: SnackBarAction(
            label: l10n.errorDetails,
            onPressed: () {
              if (!mounted) return;
              ServerErrorDialog.show(context, e, l10n);
            },
          ),
        ),
      );
      setState(() {
        _rows = [];
        _loading = false;
        _error = e;
      });
    }
  }

  /// Asks for confirmation and cancels the reservation [r].
  Future<void> _cancelReservation(Reservation r) async {
    final l10n = AppLocalizations.of(context)!;
    final name = _itemNames[r.itemId] ?? '#${r.itemId}';
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(l10n.cancelReservationDialogTitle),
        content: Text(
          '$name\n'
          '${l10n.labelCount}: ${r.count} · '
          '${l10n.labelExpiryDate}: '
          '${r.expiresAt.toIso8601String().substring(0, 10)}',
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: Text(MaterialLocalizations.of(ctx).cancelButtonLabel),
          ),
          TextButton(
            onPressed: () => Navigator.pop(ctx, true),
            child: Text(l10n.actionConfirm),
          ),
        ],
      ),
    );
    if (!mounted || confirmed != true) return;
    try {
      await ref.read(reservationsApiProvider).cancel(r.id);
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.reservationCancelled)));
      _load();
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(friendlyHttpError(e, l10n)),
          action: SnackBarAction(
            label: l10n.errorDetails,
            onPressed: () {
              if (!mounted) return;
              ServerErrorDialog.show(context, e, l10n);
            },
          ),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['reservations', 'items', 'lendings'], _load);
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/reserve2.jpeg',
          title: l10n.navMyReservations,
        ),
      ),
      body: _buildBody(l10n),
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if (_loading) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null && _rows.isEmpty) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (_rows.isEmpty) {
      return Center(child: Text(l10n.myReservationsEmpty));
    }
    return RefreshIndicator(
      onRefresh: _load,
      child: ListView.separated(
        itemCount: _rows.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (ctx, i) {
          final r = _rows[i];
          final name = _itemNames[r.itemId] ?? '#${r.itemId}';
          final expires = r.expiresAt.toIso8601String().substring(0, 10);
          return ListTile(
            leading: const Icon(Icons.bookmark_outline, color: Colors.blue),
            title: Text(name),
            subtitle: Text(
              '${l10n.myReservationsColExpires}: $expires'
              '  ·  ${l10n.labelCount}: ${r.count}',
            ),
            trailing: IconButton(
              icon: const Icon(Icons.cancel_outlined),
              tooltip: l10n.actionCancelReservation,
              onPressed: () => _cancelReservation(r),
            ),
            onTap: () => _cancelReservation(r),
          );
        },
      ),
    );
  }
}
