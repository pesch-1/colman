// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../api/barcode_lookup.dart'
    show queryBarcode, normalizeBarcode, BookSearchResult, AvSearchResult;
import '../api/ean_lookup.dart' show isProductEan, isUpcA;
import '../api/isbn_lookup.dart'
    show isIsbn10, isIsbn13, isbn10ToIsbn13;
import '../models/due_lending_row.dart';
import '../models/item.dart';
import '../models/item_prefill.dart';
import '../models/person.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/internet_services_provider.dart'
    show internetServicesProvider;
import 'acquisition_screen.dart' show AcquisitionInitialData;
import 'lending_screen.dart' show LendingInitialData;
import 'liquidation_screen.dart' show LiquidationInitialData;
import 'return_screen.dart' show ReturnInitialData;
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';

// ─── _DispatchResult — sealed hierarchy ──────────────────────────────────────

/// Result of dispatching a scanned barcode, discriminated by what the barcode
/// refers to.
///
/// Produced by [_BarcodeDispatchScreenState._dispatch] and consumed by the
/// build method to render the correct action menu.
sealed class _DispatchResult {
  const _DispatchResult();
}

/// Barcode was identified as a Colman person barcode.
///
/// [openLendings] contains all currently outstanding lendings for this person
/// (already filtered from the full `getDue()` response).
final class _PersonResult extends _DispatchResult {
  const _PersonResult(this.person, {required this.openLendings});

  /// The person whose barcode was scanned.
  final Person person;

  /// Open (outstanding) lendings belonging to [person].
  final List<DueLendingRow> openLendings;
}

/// Barcode was identified as a Colman item barcode, or as an ISBN/EAN that
/// already has entries in the database.
///
/// [available] and [acquired] are fetched in parallel so the action menu can
/// show or hide the "lend" and "liquidate" buttons based on stock levels.
final class _ItemResult extends _DispatchResult {
  const _ItemResult({
    required this.item,
    required this.available,
    required this.acquired,
  });

  /// The identified item.
  final Item item;

  /// Number of copies currently available for lending.
  final int available;

  /// Total number of copies ever acquired (lentCount = acquired - available).
  final int acquired;

  /// Number of copies currently lent out.
  int get lentCount => acquired - available;
}

/// Barcode was identified as an ISBN or EAN that is NOT in the database.
///
/// An internet lookup was attempted:
/// - [prefill] is non-null when internet data was found.
/// - [prefill] is null when the internet search returned no results.
///
/// In both cases the app navigates to the acquisition screen; [barcode] is
/// passed so the barcode field can be pre-filled.
final class _NewItemResult extends _DispatchResult {
  const _NewItemResult({required this.barcode, this.prefill});

  /// Normalised barcode string (ISBN-13 or EAN-13) to pre-fill.
  final String barcode;

  /// Pre-filled metadata from an internet lookup, or null if none was found.
  final ItemPrefill? prefill;
}

/// Barcode could not be classified as a person barcode, item barcode, or
/// valid ISBN/EAN.
final class _UnknownResult extends _DispatchResult {
  const _UnknownResult();
}

// ─── _Action — typed action descriptor ───────────────────────────────────────

/// A single action button shown in the dispatch menu.
class _Action {
  const _Action({
    required this.label,
    required this.icon,
    required this.onTap,
  });

  final String label;
  final IconData icon;
  final VoidCallback onTap;
}

// ─── BarcodeDispatchScreen ────────────────────────────────────────────────────

/// Dispatches a scanned [barcode] to the appropriate workflow.
///
/// Shows a loading indicator while performing API lookups, then presents a
/// context-sensitive action menu (up to four buttons).  When only one action
/// is available the screen navigates directly without showing the menu.
///
/// Three cases are handled:
/// - **Fall 1** — Colman person barcode: view lendings, edit person, return
///   items, lend to person.
/// - **Fall 2** — Colman item barcode or known ISBN/EAN: return, lend,
///   acquire/edit, liquidate.
/// - **Fall 3** — Unknown ISBN/EAN: internet search then navigate to
///   acquisition with pre-filled fields (or just the barcode if no results).
class BarcodeDispatchScreen extends ConsumerStatefulWidget {
  /// Creates a [BarcodeDispatchScreen] for the given raw [barcode] string.
  const BarcodeDispatchScreen({super.key, required this.barcode});

  /// Raw barcode string as returned by the camera scanner.
  final String barcode;

  @override
  ConsumerState<BarcodeDispatchScreen> createState() =>
      _BarcodeDispatchScreenState();
}

class _BarcodeDispatchScreenState
    extends ConsumerState<BarcodeDispatchScreen> {
  /// Dispatch result, null while the lookup is still in flight.
  _DispatchResult? _result;

  /// Error message, non-null when the lookup threw an exception.
  Object? _error;

  @override
  void initState() {
    super.initState();
    _runDispatch();
  }

  // ── Dispatch logic ────────────────────────────────────────────────────────

  /// Runs all API lookups and stores the result in [_result].
  ///
  /// The cascade follows the spec priority:
  /// 1. Try Colman person barcode.
  /// 2. Try Colman item barcode.
  /// 3. Classify as ISBN/EAN, then try DB lookup.
  /// 4. If classified but not in DB: internet search → [_NewItemResult].
  /// 5. Unclassifiable → [_UnknownResult].
  Future<void> _runDispatch() async {
    try {
      final result = await _dispatch(normalizeBarcode(widget.barcode));
      if(!mounted) return;
      setState(() { _result = result; });
      _maybeAutoNavigate(result);
    } catch(e) {
      if(!mounted) return;
      setState(() { _error = e; });
    }
  }

  Future<_DispatchResult> _dispatch(String clean) async {
    if(clean.isEmpty) return const _UnknownResult();

    final auth = ref.read(authProvider).value;
    if(auth is! Authenticated) return const _UnknownResult();

    // ── Step 1: Colman person barcode ─────────────────────────────────────
    // Only attempt when the user holds at least one person-related permission;
    // otherwise /api/persons/by-barcode returns 403 before we can fall through.
    final canHandlePersons = auth.hasPermission('view_persons') ||
        auth.hasPermission('edit_persons') ||
        auth.hasPermission('collect_lendings') ||
        auth.hasPermission('collect_returns');
    if(canHandlePersons) {
      try {
        final person =
            await ref.read(personsApiProvider).getByBarcode(clean);
        if(person != null) {
          List<DueLendingRow> personDue;
          try {
            personDue = await ref
                .read(lendingsApiProvider)
                .getDue(personId: person.id);
          } on DioException catch(e) {
            // 403 when user lacks view_history/collect_returns — show
            // available person actions without open-lending data.
            if(e.response?.statusCode != 403) rethrow;
            personDue = const [];
          }
          return _PersonResult(person, openLendings: personDue);
        }
      } on DioException catch(e) {
        // 404 means not a person barcode — fall through
        if(e.response?.statusCode != 404) rethrow;
      }
    }

    // ── Step 2: Colman item barcode ───────────────────────────────────────
    final parsed =
        await ref.read(itemsApiProvider).parseItemBarcode(clean);
    if(parsed != null) {
      return _itemResultFor(parsed.itemId);
    }

    // ── Step 3: Classify as ISBN/EAN and try DB lookup ────────────────────
    final looksLikeIsbn = isIsbn10(clean) ||
        (isIsbn13(clean) &&
            (clean.startsWith('978') || clean.startsWith('979')));
    String? normalizedCode;
    Item? dbItem;
    if(looksLikeIsbn) {
      normalizedCode = isIsbn10(clean) ? isbn10ToIsbn13(clean) : clean;
      dbItem = await ref.read(itemsApiProvider).getByIsbn(normalizedCode);
    } else if(isProductEan(clean)) {
      normalizedCode = clean;
      dbItem = await ref.read(itemsApiProvider).getByEan(clean);
    } else if(isUpcA(clean)) {
      normalizedCode = '0$clean';
      dbItem = await ref.read(itemsApiProvider).getByEan('0$clean');
    } else {
      return const _UnknownResult();
    }

    if(dbItem != null) {
      return _itemResultFor(dbItem.id);
    }

    // ── Step 4: Internet search (Fall 3) ──────────────────────────────────
    final settings = await ref.read(internetServicesProvider.future);
    final searchResult = await queryBarcode(clean, settings);
    // normalizedCode is guaranteed non-null here: the else branch above returns.
    final code = normalizedCode;
    ItemPrefill? prefill;
    if(searchResult is BookSearchResult && !searchResult.data.isEmpty) {
      prefill = bookToItemPrefill(searchResult.data, code);
    } else if(searchResult is AvSearchResult && !searchResult.data.isEmpty) {
      prefill = avToItemPrefill(searchResult.data, code);
    }
    return _NewItemResult(barcode: code, prefill: prefill);
  }

  /// Fetches [Item], available count, and acquired count in parallel and
  /// wraps them in an [_ItemResult].
  Future<_ItemResult> _itemResultFor(int itemId) async {
    final results = await Future.wait([
      ref.read(itemsApiProvider).getById(itemId),
      ref.read(itemsApiProvider).getAvailable(itemId),
      ref.read(itemsApiProvider).getAcquired(itemId),
    ]);
    return _ItemResult(
      item: results[0] as Item,
      available: results[1] as int,
      acquired: results[2] as int,
    );
  }

  /// Auto-navigates when only one action is available, or immediately for
  /// [_NewItemResult] (no action menu needed).
  void _maybeAutoNavigate(_DispatchResult result) {
    final auth = ref.read(authProvider).value;
    if(auth is! Authenticated) return;
    final l10n = AppLocalizations.of(context)!;

    final List<_Action> actions;
    if(result is _PersonResult) {
      actions = _buildPersonActions(result, auth, l10n);
    } else if(result is _ItemResult) {
      actions = _buildItemActions(result, auth, l10n);
    } else if(result is _NewItemResult) {
      // Fall 3: auto-navigate to acquisition only when permitted
      if(!auth.hasPermission('collect_acquisitions')) return;
      WidgetsBinding.instance.addPostFrameCallback((_) {
        if(mounted) _goToAcquisitionNewItem(result);
      });
      return;
    } else {
      return;
    }

    if(actions.length == 1) {
      WidgetsBinding.instance.addPostFrameCallback((_) {
        if(mounted) actions.first.onTap();
      });
    }
  }

  // ── Action builders ───────────────────────────────────────────────────────

  /// Builds the list of available actions for a person barcode (Fall 1).
  List<_Action> _buildPersonActions(
    _PersonResult result,
    Authenticated auth,
    AppLocalizations l10n,
  ) {
    final actions = <_Action>[];
    if(auth.hasPermission('view_persons')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionViewLendings,
        icon: Icons.list_alt,
        onTap: () => _showLendingsSheet(result, l10n),
      ));
    }
    if(auth.hasPermission('edit_persons')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionEditPerson,
        icon: Icons.edit,
        onTap: () =>
            context.pushReplacement('/persons/${result.person.id}'),
      ));
    }
    if(result.openLendings.isNotEmpty &&
        auth.hasPermission('collect_returns')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionReturnPersonItems,
        icon: Icons.assignment_return,
        onTap: () => context.pushReplacement(
          '/return',
          extra: ReturnInitialData(personId: result.person.id),
        ),
      ));
    }
    if(auth.hasPermission('collect_lendings')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionLendToPerson,
        icon: Icons.add_circle_outline,
        onTap: () => context.pushReplacement(
          '/lending',
          extra: LendingInitialData(person: result.person),
        ),
      ));
    }
    return actions;
  }

  /// Builds the list of available actions for an item barcode (Fall 2).
  List<_Action> _buildItemActions(
    _ItemResult result,
    Authenticated auth,
    AppLocalizations l10n,
  ) {
    final actions = <_Action>[];
    if(result.lentCount > 0 && auth.hasPermission('collect_returns')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionReturnItem,
        icon: Icons.assignment_return,
        onTap: () => context.pushReplacement(
          '/return',
          extra: ReturnInitialData(itemId: result.item.id),
        ),
      ));
    }
    if(result.available > 0 && auth.hasPermission('collect_lendings')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionLendItem,
        icon: Icons.add_circle_outline,
        onTap: () => context.pushReplacement(
          '/lending',
          extra: LendingInitialData(
            item: SearchItemRow(
              id: result.item.id,
              name: result.item.name,
              typeLocale: result.item.typeName ?? '',
              count: result.acquired,
              available: result.available,
              typeId: result.item.typeId,
            ),
          ),
        ),
      ));
    }
    if(auth.hasPermission('collect_acquisitions')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionAcquireItem,
        icon: Icons.add_box,
        onTap: () => context.pushReplacement(
          '/acquisition',
          extra: AcquisitionInitialData(
            item: SearchItemRow(
              id: result.item.id,
              name: result.item.name,
              typeLocale: result.item.typeName ?? '',
              count: result.acquired,
              available: result.available,
              typeId: result.item.typeId,
            ),
          ),
        ),
      ));
    }
    if(result.available > 0 && auth.hasPermission('collect_liquidations')) {
      actions.add(_Action(
        label: l10n.barcodeDispatchActionLiquidateItem,
        icon: Icons.delete_sweep,
        onTap: () => context.pushReplacement(
          '/liquidation',
          extra: LiquidationInitialData(
            item: SearchItemRow(
              id: result.item.id,
              name: result.item.name,
              typeLocale: result.item.typeName ?? '',
              count: result.acquired,
              available: result.available,
              typeId: result.item.typeId,
            ),
          ),
        ),
      ));
    }
    return actions;
  }

  // ── Navigation helpers ────────────────────────────────────────────────────

  void _goToAcquisitionNewItem(_NewItemResult result) {
    context.pushReplacement(
      '/acquisition',
      extra: AcquisitionInitialData(
        prefill: result.prefill ?? ItemPrefill(barcode: result.barcode),
      ),
    );
  }

  /// Shows a modal bottom sheet listing [result.person]'s open lendings.
  void _showLendingsSheet(_PersonResult result, AppLocalizations l10n) {
    showModalBottomSheet<void>(
      context: context,
      builder: (ctx) {
        return SafeArea(
          child: Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  result.person.displayName,
                  style: Theme.of(ctx).textTheme.titleMedium,
                ),
                const SizedBox(height: 8),
                if(result.openLendings.isEmpty)
                  Padding(
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    child: Text(
                      l10n.barcodeDispatchNoOpenLendings,
                      style: TextStyle(
                        color: Theme.of(ctx).colorScheme.secondary,
                      ),
                    ),
                  )
                else
                  ...result.openLendings.map(
                    (row) => ListTile(
                      dense: true,
                      leading: const Icon(Icons.book_outlined),
                      title: Text(row.itemName),
                      trailing: Text(
                        '${row.outstandingCount}×',
                        style: Theme.of(ctx).textTheme.bodySmall,
                      ),
                    ),
                  ),
              ],
            ),
          ),
        );
      },
    );
  }

  // ── Build ─────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/scan_barcode.jpeg',
          title: l10n.barcodeDispatchTitle,
        ),
      ),
      body: _buildBody(l10n),
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    final error = _error;
    if(error != null) {
      return Center(
        child: Padding(
          padding: const EdgeInsets.all(32),
          child: ErrorDisplay(error: error, l10n: l10n),
        ),
      );
    }
    final result = _result;
    if(result == null) {
      return _buildLoading(l10n);
    }
    final auth = ref.read(authProvider).value;
    if(auth is! Authenticated) {
      return _buildMessage(Icons.lock_outline, '');
    }

    if(result is _PersonResult) {
      final actions = _buildPersonActions(result, auth, l10n);
      if(actions.isEmpty) {
        return _buildMessage(Icons.block, l10n.barcodeDispatchNotRecognized);
      }
      if(actions.length == 1) {
        return _buildLoading(l10n);
      }
      return _buildActionMenu(
        subtitle: l10n.barcodeDispatchPersonSubtitle(result.person.displayName),
        actions: actions,
      );
    }

    if(result is _ItemResult) {
      final actions = _buildItemActions(result, auth, l10n);
      if(actions.isEmpty) {
        return _buildMessage(Icons.block, l10n.barcodeDispatchNotRecognized);
      }
      if(actions.length == 1) {
        return _buildLoading(l10n);
      }
      return _buildActionMenu(
        subtitle:
            l10n.barcodeDispatchItemSubtitle(result.item.name),
        actions: actions,
      );
    }

    if(result is _NewItemResult) {
      if(!auth.hasPermission('collect_acquisitions')) {
        return _buildMessage(Icons.block, l10n.barcodeDispatchNotRecognized);
      }
      // Auto-navigate handled in _maybeAutoNavigate; show spinner while
      // the postFrameCallback fires.
      return _buildLoading(l10n);
    }

    // _UnknownResult
    return _buildMessage(
        Icons.qr_code, l10n.barcodeDispatchNotRecognized);
  }

  Widget _buildLoading(AppLocalizations l10n) {
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          const CircularProgressIndicator(),
          const SizedBox(height: 16),
          Text(l10n.barcodeDispatchLoading),
        ],
      ),
    );
  }

  Widget _buildMessage(IconData icon, String text, {Color? color}) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(icon, size: 64, color: color),
            if(text.isNotEmpty) ...[
              const SizedBox(height: 16),
              Text(text, textAlign: TextAlign.center),
            ],
          ],
        ),
      ),
    );
  }

  Widget _buildActionMenu({
    required String subtitle,
    required List<_Action> actions,
  }) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Padding(
          padding: const EdgeInsets.only(bottom: 16),
          child: Text(
            subtitle,
            style: Theme.of(context).textTheme.titleMedium,
          ),
        ),
        ...actions.map(
          (a) => Card(
            child: ListTile(
              leading: Icon(a.icon),
              title: Text(a.label),
              trailing: const Icon(Icons.chevron_right),
              onTap: a.onTap,
            ),
          ),
        ),
      ],
    );
  }
}
