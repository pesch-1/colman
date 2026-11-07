// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../api/lendings_api.dart';
import '../api/persons_api.dart';
import '../api/reservations_api.dart';
import '../models/item.dart';
import '../models/item_type.dart';
import '../models/person.dart';
import '../models/reservation.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/item_detail_body.dart';
import '../widgets/person_search_field.dart';
import '../widgets/server_error_dialog.dart';
import 'label_print_screen.dart' show LabelPrintInitialData;

/// Detail screen for a single library item.
///
/// Loads the [Item] by [itemId] on mount and displays its fields.
/// Availability and reservations are loaded in parallel via separate requests.
/// Action buttons (Reserve, Lend, Cancel Reservation) are shown at the bottom.
class ItemDetailScreen extends ConsumerStatefulWidget {
  /// Creates an [ItemDetailScreen] for item [itemId].
  const ItemDetailScreen({super.key, required this.itemId});

  /// Primary key of the item to display.
  final int itemId;

  @override
  ConsumerState<ItemDetailScreen> createState() => _ItemDetailScreenState();
}

class _ItemDetailScreenState extends ConsumerState<ItemDetailScreen> {
  Item? _item;
  ItemType? _type;
  bool _loading = true;
  Object? _error;
  List<Reservation> _reservations = [];
  int? _available;
  int? _stock;

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    try {
      final item = await ref.read(itemsApiProvider).getById(widget.itemId);
      final type = await ref.read(typesApiProvider).getById(item.typeId);
      if (!mounted) return;
      setState(() {
        _item = item;
        _type = type;
        _loading = false;
      });
      _loadReservations();
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _error = e;
        _loading = false;
      });
    }
  }

  Future<void> _loadReservations() async {
    try {
      final reservationsFuture = ref
          .read(reservationsApiProvider)
          .getForItem(widget.itemId);
      final availableFuture = ref
          .read(itemsApiProvider)
          .getAvailable(widget.itemId);
      final stockFuture = ref.read(itemsApiProvider).getStock(widget.itemId);
      final reservations = await reservationsFuture;
      final available = await availableFuture;
      final stock = await stockFuture;
      if (!mounted) return;
      setState(() {
        _reservations = reservations;
        _available = available;
        _stock = stock;
      });
    } catch (_) {
      // reservations, availability and stock are supplementary — ignore errors
    }
  }

  /// Returns the number of copies that can still be reserved: available
  /// copies minus those reserved by anyone (own reservations included),
  /// or `null` while the availability has not been loaded yet.
  int? _reservableCount() {
    final available = _available;
    if (available == null) return null;
    var reserved = 0;
    for (final r in _reservations) {
      reserved += r.count;
    }
    return available - reserved;
  }

  /// Whether the reserve button should be shown: the item allows
  /// reservations and there are copies left that nobody has reserved.
  bool _canShowReserve() {
    final auth = ref.read(authProvider).value;
    if (auth is! Authenticated || !auth.hasPermission('reserve')) {
      return false;
    }
    if (!(_item?.allowReservations ?? false)) return false;
    return (_reservableCount() ?? 0) > 0;
  }

  Future<void> _showReserveDialog() async {
    final item = _item!;
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.read(authProvider).value;
    // Users without edit_reservations may only reserve for themselves;
    // the dialog then skips the person search (which would require the
    // view_persons permission anyway).
    final fixedPersonId =
        (auth is Authenticated && !auth.hasPermission('edit_reservations'))
        ? auth.currentUserId
        : null;
    final reservationsApi = ref.read(reservationsApiProvider);
    final personsApi = ref.read(personsApiProvider);
    final success = await showDialog<bool>(
      context: context,
      builder: (_) => _ReserveDialog(
        item: item,
        reservationsApi: reservationsApi,
        personsApi: personsApi,
        l10n: l10n,
        fixedPersonId: fixedPersonId,
        maxCount: _reservableCount(),
      ),
    );
    if (!mounted) return;
    if (success == true) {
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.reservationCreated)));
      _loadReservations();
    }
  }

  Future<void> _showLendDialog() async {
    final item = _item!;
    final l10n = AppLocalizations.of(context)!;
    final lendingsApi = ref.read(lendingsApiProvider);
    final personsApi = ref.read(personsApiProvider);
    final success = await showDialog<bool>(
      context: context,
      builder: (_) => _LendDialog(
        item: item,
        lendingsApi: lendingsApi,
        personsApi: personsApi,
        l10n: l10n,
      ),
    );
    if (!mounted) return;
    if (success == true) {
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.lendingCreated)));
    }
  }

  Future<void> _editItem() async {
    final result = await context.push('/items/${widget.itemId}/edit');
    if (result == true && mounted) {
      _load();
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(AppLocalizations.of(context)!.itemSaved)),
      );
    }
  }

  /// Whether the delete button should be shown: the user may collect
  /// acquisitions and the item currently has no copies left (e.g. after all
  /// of them were liquidated).
  bool _canShowDelete() {
    final auth = ref.read(authProvider).value;
    if (auth is! Authenticated || !auth.hasPermission('collect_acquisitions')) {
      return false;
    }
    return _stock == 0;
  }

  Future<void> _deleteItem() async {
    final item = _item;
    if (item == null) return;
    final l10n = AppLocalizations.of(context)!;
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(l10n.actionDeleteItem),
        content: Text(l10n.deleteItemConfirmMessage(item.name)),
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
    if (confirmed != true || !mounted) return;
    try {
      await ref.read(itemsApiProvider).delete(widget.itemId);
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.itemDeleted)));
      // Pop back to the result list, signalling that a refresh is needed.
      context.pop(true);
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

  /// Returns the reservations the current user may cancel: their own,
  /// or all of the item's reservations with the edit_reservations
  /// permission.
  List<Reservation> _cancellableReservations() {
    final auth = ref.read(authProvider).value;
    if (auth is! Authenticated) return const [];
    if (auth.hasPermission('edit_reservations')) return _reservations;
    return _reservations
        .where((r) => r.personId == auth.currentUserId)
        .toList();
  }

  Future<void> _onCancelReservation() async {
    final l10n = AppLocalizations.of(context)!;
    final cancellable = _cancellableReservations();
    if (cancellable.isEmpty) {
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.noActiveReservations)));
      return;
    }
    Reservation? toCancel;
    if (cancellable.length == 1) {
      final r = cancellable.first;
      final confirmed = await showDialog<bool>(
        context: context,
        builder: (ctx) => AlertDialog(
          title: Text(l10n.cancelReservationDialogTitle),
          content: Text(
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
      if (confirmed == true) toCancel = r;
    } else {
      toCancel = await showModalBottomSheet<Reservation>(
        context: context,
        builder: (ctx) => Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Padding(
              padding: const EdgeInsets.all(16),
              child: Text(
                l10n.selectReservationToCancel,
                style: Theme.of(ctx).textTheme.titleMedium,
              ),
            ),
            ...cancellable.map(
              (r) => ListTile(
                title: Text('${l10n.labelCount}: ${r.count}'),
                subtitle: Text(
                  '${l10n.labelExpiryDate}: '
                  '${r.expiresAt.toIso8601String().substring(0, 10)}',
                ),
                onTap: () => Navigator.pop(ctx, r),
              ),
            ),
            const SizedBox(height: 8),
          ],
        ),
      );
    }
    if (!mounted || toCancel == null) return;
    try {
      await ref.read(reservationsApiProvider).cancel(toCancel.id);
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.reservationCancelled)));
      _loadReservations();
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
    watchTables(ref, const [
      'items',
      'lendings',
      'returns',
      'acquisitions',
      'liquidations',
      'reservations',
    ], _load);
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canCollectAcquisitions =
        auth is Authenticated && auth.hasPermission('collect_acquisitions');
    final canCollectLendings =
        auth is Authenticated && auth.hasPermission('collect_lendings');
    final canReserve = auth is Authenticated && auth.hasPermission('reserve');
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/search.jpeg',
          title: _item?.name ?? l10n.itemDetailFallbackTitle,
        ),
      ),
      body: _buildBody(
        l10n,
        canCollectAcquisitions,
        canCollectLendings,
        canReserve,
      ),
    );
  }

  Widget _buildBody(
    AppLocalizations l10n,
    bool canCollectAcquisitions,
    bool canCollectLendings,
    bool canReserve,
  ) {
    if (_loading) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    final item = _item!;
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        ItemDetailBody(item: item, itemType: _type),
        const SizedBox(height: 16),
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: [
            if (_canShowReserve())
              OutlinedButton.icon(
                icon: const Icon(Icons.bookmark_border),
                label: Text(l10n.actionReserve),
                onPressed: _showReserveDialog,
              ),
            if (canCollectLendings)
              OutlinedButton.icon(
                icon: const Icon(Icons.assignment_return_outlined),
                label: Text(l10n.actionLend),
                onPressed: _showLendDialog,
              ),
            if (canReserve && _cancellableReservations().isNotEmpty)
              OutlinedButton.icon(
                icon: const Icon(Icons.cancel_outlined),
                label: Text(l10n.actionCancelReservation),
                onPressed: _onCancelReservation,
              ),
            if (canCollectAcquisitions)
              OutlinedButton.icon(
                icon: const Icon(Icons.edit),
                label: Text(l10n.actionEditItem),
                onPressed: _item != null ? _editItem : null,
              ),
            if (_canShowDelete())
              OutlinedButton.icon(
                icon: const Icon(Icons.delete_outline),
                label: Text(l10n.actionDeleteItem),
                onPressed: _deleteItem,
              ),
            OutlinedButton.icon(
              icon: const Icon(Icons.print_outlined),
              label: Text(l10n.labelPrintTitle),
              onPressed: () => context.push(
                '/label-print',
                extra: LabelPrintInitialData(
                  itemId: item.id,
                  itemName: item.name,
                ),
              ),
            ),
          ],
        ),
      ],
    );
  }
}

/// Dialog for reserving a library item.
///
/// When [fixedPersonId] is set, the reservation is made for that person
/// (the logged-in user) and the person search is hidden.  [maxCount]
/// caps the count selector at the number of still reservable copies.
class _ReserveDialog extends StatefulWidget {
  const _ReserveDialog({
    required this.item,
    required this.reservationsApi,
    required this.personsApi,
    required this.l10n,
    this.fixedPersonId,
    this.maxCount,
  });

  final Item item;
  final ReservationsApi reservationsApi;
  final PersonsApi personsApi;
  final AppLocalizations l10n;
  final int? fixedPersonId;
  final int? maxCount;

  @override
  State<_ReserveDialog> createState() => _ReserveDialogState();
}

class _ReserveDialogState extends State<_ReserveDialog> {
  Person? _person;
  int _count = 1;
  DateTime? _expiresAt;
  bool _busy = false;
  String? _errorMessage;

  bool get _canSubmit =>
      (widget.fixedPersonId != null || _person != null) &&
      _count > 0 &&
      _expiresAt != null &&
      !_busy;

  void _onPersonSelected(Person p) => setState(() {
    _person = p;
  });

  @override
  void initState() {
    super.initState();
    _initExpiryDefault();
  }

  /// Prefills the expiry date from the item's reservation period or,
  /// when the item uses the default, from the collection setting.
  Future<void> _initExpiryDefault() async {
    var days = widget.item.reservationPeriodDays;
    if (days == null) {
      try {
        days = await widget.reservationsApi.defaultPeriodDays();
      } catch (_) {
        return; // the date can still be picked manually
      }
    }
    if (!mounted || _expiresAt != null || days == null) return;
    setState(() {
      _expiresAt = DateTime.now().add(Duration(days: days!));
    });
  }

  Future<void> _pickDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: DateTime.now().add(const Duration(days: 7)),
      firstDate: DateTime.now().add(const Duration(days: 1)),
      lastDate: DateTime.now().add(const Duration(days: 365)),
    );
    if (picked != null) {
      setState(() {
        _expiresAt = picked;
      });
    }
  }

  Future<void> _submit() async {
    setState(() {
      _busy = true;
      _errorMessage = null;
    });
    try {
      await widget.reservationsApi.create(
        itemId: widget.item.id,
        personId: widget.fixedPersonId ?? _person!.id,
        count: _count,
        expiresAt: _expiresAt!,
      );
      if (!mounted) return;
      Navigator.pop(context, true);
    } on ReservationException catch (e) {
      if (!mounted) return;
      final String msg;
      switch (e.error) {
        case ReservationError.alreadyExists:
          msg = widget.l10n.errorAlreadyReserved;
        case ReservationError.notEnoughAvailable:
          msg = widget.l10n.errorNotEnoughAvailable;
        case ReservationError.disabled:
          msg = widget.l10n.errorReservationsDisabled;
        case ReservationError.other:
          msg = widget.l10n.errorRequestFailed;
      }
      setState(() {
        _busy = false;
        _errorMessage = msg;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
      });
      await ServerErrorDialog.show(context, e, widget.l10n);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = widget.l10n;
    final expiresText = _expiresAt != null
        ? _expiresAt!.toIso8601String().substring(0, 10)
        : '—';
    return AlertDialog(
      title: Text(l10n.reserveDialogTitle),
      content: SingleChildScrollView(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            if (widget.fixedPersonId == null)
              PersonSearchField(
                personsApi: widget.personsApi,
                labelText: l10n.labelPersonSearch,
                onSelected: _onPersonSelected,
              )
            else
              Text(l10n.reserveSelfHint),
            const SizedBox(height: 12),
            Row(
              children: [
                Text(l10n.labelCount),
                const Spacer(),
                IconButton(
                  onPressed: _count > 1
                      ? () => setState(() {
                          _count--;
                        })
                      : null,
                  icon: const Icon(Icons.remove),
                ),
                Text('$_count'),
                IconButton(
                  onPressed:
                      (widget.maxCount == null || _count < widget.maxCount!)
                      ? () => setState(() {
                          _count++;
                        })
                      : null,
                  icon: const Icon(Icons.add),
                ),
              ],
            ),
            ListTile(
              contentPadding: EdgeInsets.zero,
              title: Text(l10n.labelExpiryDate),
              subtitle: Text(expiresText),
              trailing: const Icon(Icons.calendar_today),
              onTap: _busy ? null : _pickDate,
            ),
            if (_errorMessage != null)
              Text(
                _errorMessage!,
                style: TextStyle(color: Theme.of(context).colorScheme.error),
              ),
          ],
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: _canSubmit ? _submit : null,
          child: _busy
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child: CircularProgressIndicator.adaptive(strokeWidth: 2),
                )
              : Text(l10n.actionConfirm),
        ),
      ],
    );
  }
}

/// Dialog for recording a lending for a library item.
class _LendDialog extends StatefulWidget {
  const _LendDialog({
    required this.item,
    required this.lendingsApi,
    required this.personsApi,
    required this.l10n,
  });

  final Item item;
  final LendingsApi lendingsApi;
  final PersonsApi personsApi;
  final AppLocalizations l10n;

  @override
  State<_LendDialog> createState() => _LendDialogState();
}

class _LendDialogState extends State<_LendDialog> {
  Person? _person;
  int _count = 1;
  DateTime? _duedate;
  bool _busy = false;
  String? _errorMessage;

  bool get _canSubmit => _person != null && _count > 0 && !_busy;

  void _onPersonSelected(Person p) => setState(() {
    _person = p;
  });

  Future<void> _pickDueDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: DateTime.now().add(const Duration(days: 14)),
      firstDate: DateTime.now().add(const Duration(days: 1)),
      lastDate: DateTime.now().add(const Duration(days: 365)),
    );
    if (picked != null) {
      setState(() {
        _duedate = picked;
      });
    }
  }

  Future<void> _submit() async {
    setState(() {
      _busy = true;
      _errorMessage = null;
    });
    try {
      final now = DateTime.now();
      await widget.lendingsApi.create({
        'item_id': widget.item.id,
        'person_id': _person!.id,
        'count': _count,
        'date_time': now.toIso8601String(),
        if (_duedate != null)
          'duedate': _duedate!.toIso8601String().substring(0, 10),
      });
      if (!mounted) return;
      Navigator.pop(context, true);
    } on LendingUnavailableException catch (_) {
      if (!mounted) return;
      setState(() {
        _busy = false;
        _errorMessage = widget.l10n.errorNotEnoughAvailable;
      });
    } on LendingConflictException catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
        _errorMessage = widget.l10n.lendingConflictReserved(e.notReservedCount);
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
      });
      await ServerErrorDialog.show(context, e, widget.l10n);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = widget.l10n;
    final duedateText = _duedate != null
        ? _duedate!.toIso8601String().substring(0, 10)
        : '—';
    return AlertDialog(
      title: Text(l10n.lendDialogTitle),
      content: SingleChildScrollView(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            PersonSearchField(
              personsApi: widget.personsApi,
              labelText: l10n.labelPersonSearch,
              onSelected: _onPersonSelected,
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Text(l10n.labelCount),
                const Spacer(),
                IconButton(
                  onPressed: _count > 1
                      ? () => setState(() {
                          _count--;
                        })
                      : null,
                  icon: const Icon(Icons.remove),
                ),
                Text('$_count'),
                IconButton(
                  onPressed: () => setState(() {
                    _count++;
                  }),
                  icon: const Icon(Icons.add),
                ),
              ],
            ),
            ListTile(
              contentPadding: EdgeInsets.zero,
              title: Text(l10n.labelDueDate),
              subtitle: Text(duedateText),
              trailing: const Icon(Icons.calendar_today),
              onTap: _busy ? null : _pickDueDate,
            ),
            if (_errorMessage != null)
              Text(
                _errorMessage!,
                style: TextStyle(color: Theme.of(context).colorScheme.error),
              ),
          ],
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: _canSubmit ? _submit : null,
          child: _busy
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child: CircularProgressIndicator.adaptive(strokeWidth: 2),
                )
              : Text(l10n.actionConfirm),
        ),
      ],
    );
  }
}
