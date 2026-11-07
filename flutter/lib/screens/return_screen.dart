// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/due_lending_row.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/extend_lending_dialog.dart';
import '../widgets/server_error_dialog.dart';

// ─── _LentItemRow ─────────────────────────────────────────────────────────────

/// A deduplicated item entry for the item-selection step.
///
/// Built from [DueLendingRow] records by grouping on [id] and summing
/// [totalOutstanding] across all open lendings for the same item.
class _LentItemRow {
  /// Creates a [_LentItemRow].
  const _LentItemRow({
    required this.id,
    required this.name,
    required this.totalOutstanding,
  });

  /// Primary key of the item.
  final int id;

  /// Display name of the item.
  final String name;

  /// Total number of copies outstanding across all lendings of this item.
  final int totalOutstanding;
}

// ─── ReturnInitialData ────────────────────────────────────────────────────────

/// Initial data to pre-configure a [ReturnScreen] from a barcode scan.
///
/// Set [itemId] to pre-select a specific item in step 0 and advance to
/// step 1 automatically.  Set [personId] to filter the open-lendings list
/// to show only that person's items.  Both can be combined.
class ReturnInitialData {
  const ReturnInitialData({this.itemId, this.personId});

  /// Pre-select this item in step 0; wizard advances to step 1 automatically.
  final int? itemId;

  /// Filter open lendings to show only items currently lent by this person.
  final int? personId;
}

// ─── ReturnScreen ─────────────────────────────────────────────────────────────

/// Main return screen with two tabs: a new-return wizard and the open-lendings
/// list.
///
/// The wizard follows the same flow as the Qt easy-return page:
/// item selection → lending (person) selection → details.
class ReturnScreen extends ConsumerStatefulWidget {
  /// Creates a [ReturnScreen].
  const ReturnScreen({super.key, this.initialData});

  /// Optional pre-configuration from a barcode scan dispatch.
  final ReturnInitialData? initialData;

  @override
  ConsumerState<ReturnScreen> createState() => _ReturnScreenState();
}

class _ReturnScreenState extends ConsumerState<ReturnScreen> {
  // ── Wizard state ──────────────────────────────────────────────────────────

  /// Current active step index (0 = item, 1 = lending, 2 = details).
  int _wizardStep = 0;

  /// Item selected in step 0.
  _LentItemRow? _selectedItem;

  /// Lending row selected in step 1; carries lendingId, personName, and
  /// outstandingCount.
  DueLendingRow? _selectedLending;

  /// Number of copies to return (1 .. outstandingCount).
  int _count = 1;

  final TextEditingController _remarksCtrl = TextEditingController();

  /// Filter text typed in the item search field (step 0).
  final TextEditingController _itemSearchCtrl = TextEditingController();

  /// True while submitting the return to the server.
  bool _submitting = false;

  /// Non-null after a successful return; drives the success card.
  int? _successId;

  // ── All-open-lendings cache ───────────────────────────────────────────────

  /// All currently open lendings, loaded once on init and reloaded after each
  /// successful return.  Both the item list (step 0) and the lending list
  /// (step 1) are derived from this cache.
  List<DueLendingRow> _allRows = [];

  /// True while [_loadAllLendings] is in flight.
  bool _allRowsLoading = false;

  /// Error message set when [_loadAllLendings] fails.
  Object? _allRowsError;

  @override
  void initState() {
    super.initState();
    _loadAllLendings();
  }

  @override
  void dispose() {
    _remarksCtrl.dispose();
    _itemSearchCtrl.dispose();
    super.dispose();
  }

  // ── Derived lists ─────────────────────────────────────────────────────────

  /// Returns the list of unique lent items from [_allRows], filtered by the
  /// current search text and sorted alphabetically by name.
  ///
  /// Outstanding counts are summed when the same item appears in multiple
  /// open lending records.
  List<_LentItemRow> get _filteredItems {
    final query = _itemSearchCtrl.text.toLowerCase();
    final Map<int, _LentItemRow> byId = {};
    for (final row in _allRows) {
      final existing = byId[row.itemId];
      if (existing == null) {
        byId[row.itemId] = _LentItemRow(
          id: row.itemId,
          name: row.itemName,
          totalOutstanding: row.outstandingCount,
        );
      } else {
        byId[row.itemId] = _LentItemRow(
          id: row.itemId,
          name: row.itemName,
          totalOutstanding: existing.totalOutstanding + row.outstandingCount,
        );
      }
    }
    var items = byId.values.toList()..sort((a, b) => a.name.compareTo(b.name));
    if (query.isNotEmpty) {
      items = items.where((i) => i.name.toLowerCase().contains(query)).toList();
    }
    return items;
  }

  /// Returns all open lendings from [_allRows] for the currently selected item.
  List<DueLendingRow> get _lendingRows =>
      _allRows.where((r) => r.itemId == _selectedItem?.id).toList();

  // ── Data loading ──────────────────────────────────────────────────────────

  /// Fetches all open lendings from the server and stores them in [_allRows].
  ///
  /// Called on init and after each successful return so that the item list and
  /// lending list always reflect the current database state.
  ///
  /// When [ReturnInitialData.personId] is set, rows are filtered to only
  /// include that person's outstanding lendings.  When [ReturnInitialData.itemId]
  /// is set, the matching item is auto-selected and the wizard advances to
  /// step 1.
  Future<void> _loadAllLendings() async {
    setState(() {
      _allRowsLoading = true;
      _allRowsError = null;
    });
    try {
      var rows = await ref.read(lendingsApiProvider).getDue();
      if (!mounted) return;
      final initial = widget.initialData;
      if (initial?.personId != null) {
        rows = rows.where((r) => r.personId == initial!.personId).toList();
      }
      _LentItemRow? autoItem;
      if (initial?.itemId != null) {
        final Map<int, _LentItemRow> byId = {};
        for (final r in rows) {
          final existing = byId[r.itemId];
          byId[r.itemId] = _LentItemRow(
            id: r.itemId,
            name: r.itemName,
            totalOutstanding:
                (existing?.totalOutstanding ?? 0) + r.outstandingCount,
          );
        }
        autoItem = byId[initial!.itemId];
      }
      setState(() {
        _allRows = rows;
        _allRowsLoading = false;
        if (autoItem != null) {
          _selectedItem = autoItem;
          _wizardStep = 1;
        }
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _allRowsError = e;
        _allRowsLoading = false;
      });
    }
  }

  // ── Wizard navigation ─────────────────────────────────────────────────────

  /// Validates the current step and advances to the next one.
  ///
  /// Step 0 → 1: if the selected item has exactly one open lending it is
  /// selected automatically and the wizard jumps directly to step 2.
  /// Step 1 → 2: resets the count to 1.
  /// Step 2: calls [_submit].
  void _onContinue() {
    final l10n = AppLocalizations.of(context)!;
    switch (_wizardStep) {
      case 0:
        if (_selectedItem == null) {
          _showSnack(l10n.returnStep1Title);
          return;
        }
        final lendings = _lendingRows;
        if (lendings.length == 1) {
          setState(() {
            _selectedLending = lendings.first;
            _count = 1;
            _wizardStep = 2;
          });
        } else {
          setState(() {
            _wizardStep = 1;
          });
        }
      case 1:
        if (_selectedLending == null) {
          _showSnack(l10n.returnStep2Title);
          return;
        }
        setState(() {
          _count = 1;
          _wizardStep = 2;
        });
      case 2:
        _submit();
    }
  }

  /// Goes back one step, clearing lending selection when leaving step 1.
  void _onCancel() {
    if (_wizardStep == 1) {
      setState(() {
        _wizardStep = 0;
        _selectedLending = null;
      });
    } else if (_wizardStep > 0) {
      setState(() {
        _wizardStep -= 1;
      });
    }
  }

  // ── Submission ────────────────────────────────────────────────────────────

  /// POSTs the return to the server.
  ///
  /// After a successful POST the open-lendings cache is reloaded so that the
  /// success-card "Same item" path shows the updated lending list.
  Future<void> _submit() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() {
      _submitting = true;
    });
    try {
      final data = <String, dynamic>{
        'lending_id': _selectedLending!.lendingId,
        'count': _count,
        'date_time': DateTime.now().toUtc().toIso8601String(),
      };
      if (_remarksCtrl.text.isNotEmpty) {
        data['remarks'] = _remarksCtrl.text;
      }
      final id = await ref.read(returnsApiProvider).create(data);
      if (!mounted) return;
      final updated = await ref.read(lendingsApiProvider).getDue();
      if (!mounted) return;
      setState(() {
        _allRows = updated;
        _successId = id;
        _submitting = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  // ── Extend due date ───────────────────────────────────────────────────────

  /// True when the current user may extend lending periods.
  bool _canExtendLendings() {
    final auth = ref.read(authProvider).value;
    return auth is Authenticated && auth.hasPermission('collect_lendings');
  }

  /// Extends exactly the lending chosen in step 2 (the dialog only asks for
  /// the new due date) and refreshes the lending rows afterwards so the new
  /// due date is shown in step 2.
  Future<void> _onExtend() async {
    final lending = _selectedLending;
    if (lending == null) return;
    final extended = await showExtendSelectedLendingsDialog(context, [lending]);
    if (!extended || !mounted) return;
    await _loadAllLendings();
    if (!mounted) return;
    setState(() {
      for (final r in _allRows) {
        if (r.lendingId == lending.lendingId) {
          _selectedLending = r;
        }
      }
    });
  }

  // ── Reset helpers ─────────────────────────────────────────────────────────

  /// Resets the wizard completely back to step 0 and reloads open lendings.
  void _resetWizard() {
    _remarksCtrl.clear();
    _itemSearchCtrl.clear();
    setState(() {
      _wizardStep = 0;
      _selectedItem = null;
      _selectedLending = null;
      _count = 1;
      _submitting = false;
      _successId = null;
    });
    _loadAllLendings();
  }

  /// Keeps the selected item and returns to step 1 to pick a different lending.
  ///
  /// [_allRows] was already refreshed by [_submit], so [_lendingRows] reflects
  /// the current state without a new network request.
  void _resetToSameItem() {
    _remarksCtrl.clear();
    setState(() {
      _selectedLending = null;
      _count = 1;
      _submitting = false;
      _successId = null;
      _wizardStep = 1;
    });
  }

  // ── Utility ───────────────────────────────────────────────────────────────

  /// Shows a brief SnackBar hint when the user tries to advance without a
  /// required selection.
  void _showSnack(String message) {
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(SnackBar(content: Text(message)));
  }

  // ── Build ─────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return DefaultTabController(
      length: 2,
      child: Scaffold(
        appBar: AppBar(
          title: AppBarTitle(
            icon: 'assets/icons/returns.jpeg',
            title: l10n.returnTitle,
          ),
          bottom: TabBar(
            tabs: [
              Tab(text: l10n.returnTabNew),
              Tab(text: l10n.returnTabOpen),
            ],
          ),
        ),
        body: TabBarView(
          children: [_buildNewTab(context, l10n), const _ReturnOpenTab()],
        ),
      ),
    );
  }

  /// Builds the "New Return" tab: either the success card or the wizard stepper.
  Widget _buildNewTab(BuildContext context, AppLocalizations l10n) {
    if (_successId != null) {
      return _buildSuccessCard(context, l10n);
    }
    return _buildStepper(context, l10n);
  }

  // ── Success card ──────────────────────────────────────────────────────────

  /// Shown after a return is successfully recorded.
  Widget _buildSuccessCard(BuildContext context, AppLocalizations l10n) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Icon(Icons.check_circle, color: Colors.green, size: 64),
            const SizedBox(height: 16),
            Text(
              l10n.returnSuccessId(_successId!),
              style: Theme.of(context).textTheme.headlineSmall,
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 32),
            ElevatedButton.icon(
              onPressed: _resetWizard,
              icon: const Icon(Icons.add),
              label: Text(l10n.returnNewReturn),
            ),
            const SizedBox(height: 12),
            OutlinedButton.icon(
              onPressed: _lendingRows.isEmpty ? null : _resetToSameItem,
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.returnSameItem),
            ),
          ],
        ),
      ),
    );
  }

  // ── Stepper wizard ────────────────────────────────────────────────────────

  /// Builds the three-step wizard for recording a new return.
  Widget _buildStepper(BuildContext context, AppLocalizations l10n) {
    return Stepper(
      type: StepperType.horizontal,
      currentStep: _wizardStep,
      onStepTapped: (i) {
        if (i < _wizardStep) {
          setState(() {
            _wizardStep = i;
          });
        }
      },
      onStepContinue: _submitting ? null : _onContinue,
      onStepCancel: _submitting ? null : _onCancel,
      controlsBuilder: _buildStepControls,
      steps: [
        _buildItemStep(l10n),
        _buildLendingStep(l10n),
        _buildDetailsStep(l10n),
      ],
    );
  }

  // ── Step controls ─────────────────────────────────────────────────────────

  /// Custom controls row showing a loading indicator while submitting.
  ///
  /// The final-step button is labelled with [AppLocalizations.historyActionReturn]
  /// (= "return" / "Rückgabe"); all earlier steps use
  /// [AppLocalizations.actionConfirm].
  Widget _buildStepControls(BuildContext ctx, ControlsDetails details) {
    final l10n = AppLocalizations.of(ctx)!;
    return Padding(
      padding: const EdgeInsets.only(top: 16),
      child: Row(
        children: [
          if (_submitting)
            const SizedBox(
              width: 24,
              height: 24,
              child: CircularProgressIndicator(strokeWidth: 2),
            )
          else
            ElevatedButton(
              onPressed: details.onStepContinue,
              child: Text(
                _wizardStep == 2
                    ? l10n.historyActionReturn
                    : l10n.actionConfirm,
              ),
            ),
          const SizedBox(width: 8),
          if (_wizardStep > 0 && !_submitting)
            TextButton(
              onPressed: details.onStepCancel,
              child: Text(MaterialLocalizations.of(ctx).cancelButtonLabel),
            ),
        ],
      ),
    );
  }

  // ── Step 0: Item ──────────────────────────────────────────────────────────

  /// Builds the item-selection step.
  ///
  /// Derives the list of currently lent items from [_allRows] (deduplicated
  /// and sorted).  A search field filters the list client-side.  Once an item
  /// is selected it is shown as a chip with a "change" button.
  Step _buildItemStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.lendingStepItem),
      subtitle: Text(l10n.returnStep1Title),
      isActive: _wizardStep >= 0,
      state: _selectedItem != null ? StepState.complete : StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          if (_selectedItem != null) ...[
            _buildInfoChip(Icons.book, _selectedItem!.name),
            const SizedBox(height: 8),
            TextButton.icon(
              onPressed: () {
                setState(() {
                  _selectedItem = null;
                  _selectedLending = null;
                });
              },
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.returnStep1Title),
            ),
          ] else if (_allRowsLoading)
            const Center(child: CircularProgressIndicator())
          else if (_allRowsError != null)
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 8),
              child: ErrorDisplay(error: _allRowsError!, l10n: l10n),
            )
          else ...[
            TextField(
              controller: _itemSearchCtrl,
              decoration: InputDecoration(
                hintText: l10n.lendingItemSearchHint,
                border: const OutlineInputBorder(),
                prefixIcon: const Icon(Icons.search),
                isDense: true,
              ),
              onChanged: (_) {
                setState(() {});
              },
            ),
            const SizedBox(height: 8),
            if (_filteredItems.isEmpty)
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 8),
                child: Text(
                  l10n.returnOpenNoResults,
                  style: TextStyle(
                    color: Theme.of(context).colorScheme.secondary,
                  ),
                ),
              )
            else
              // ConstrainedBox gives the ListView a bounded maxHeight so it
              // can scroll independently within the Stepper column.
              ConstrainedBox(
                constraints: const BoxConstraints(maxHeight: 400),
                child: ListView.separated(
                  physics: const ClampingScrollPhysics(),
                  itemCount: _filteredItems.length,
                  separatorBuilder: (_, __) => const Divider(height: 1),
                  itemBuilder: (ctx, i) {
                    final item = _filteredItems[i];
                    return ListTile(
                      dense: true,
                      leading: const Icon(Icons.book_outlined),
                      title: Text(item.name),
                      trailing: Text(
                        l10n.lendingDueOutstanding(item.totalOutstanding),
                        style: const TextStyle(
                          fontSize: 12,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      onTap: () {
                        setState(() {
                          _selectedItem = item;
                        });
                      },
                    );
                  },
                ),
              ),
          ],
        ],
      ),
    );
  }

  // ── Step 1: Lending ───────────────────────────────────────────────────────

  /// Builds the lending-selection step.
  ///
  /// Shows all open lendings for [_selectedItem] from [_lendingRows].  Each
  /// row identifies the borrowing person and how many copies are still
  /// outstanding.  If the selected item has only one open lending this step
  /// is skipped automatically by [_onContinue].
  Step _buildLendingStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.returnStepLending),
      subtitle: Text(l10n.returnStep2Title),
      isActive: _wizardStep >= 1,
      state: _selectedLending != null ? StepState.complete : StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          if (_selectedLending != null) ...[
            _buildInfoChip(Icons.person, _selectedLending!.personName),
            const SizedBox(height: 8),
            TextButton.icon(
              onPressed: () {
                setState(() {
                  _selectedLending = null;
                });
              },
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.returnStep2Title),
            ),
          ] else if (_lendingRows.isEmpty)
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 8),
              child: Text(
                l10n.returnStep2NoResults,
                style: TextStyle(
                  color: Theme.of(context).colorScheme.secondary,
                ),
              ),
            )
          else
            ConstrainedBox(
              constraints: const BoxConstraints(maxHeight: 400),
              child: ListView.separated(
                physics: const ClampingScrollPhysics(),
                itemCount: _lendingRows.length,
                separatorBuilder: (_, __) => const Divider(height: 1),
                itemBuilder: (ctx, i) {
                  final row = _lendingRows[i];
                  return ListTile(
                    dense: true,
                    leading: const Icon(Icons.person_outline),
                    title: Text(row.personName),
                    subtitle: Text(_fmtDate(row.duedate)),
                    trailing: Text(
                      l10n.lendingDueOutstanding(row.outstandingCount),
                      style: const TextStyle(
                        fontSize: 12,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    onTap: () {
                      setState(() {
                        _selectedLending = row;
                      });
                    },
                  );
                },
              ),
            ),
        ],
      ),
    );
  }

  // ── Step 2: Details ───────────────────────────────────────────────────────

  /// Builds the return-details step with a count stepper and a remarks field.
  Step _buildDetailsStep(AppLocalizations l10n) {
    final maxCount = _selectedLending?.outstandingCount ?? 1;
    return Step(
      title: Text(l10n.returnStepDetails),
      subtitle: Text(l10n.returnStep3Title),
      isActive: _wizardStep >= 2,
      state: StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // ── Count row ────────────────────────────────────────────────────
          Row(
            children: [
              Text(
                l10n.labelCount,
                style: Theme.of(context).textTheme.bodyMedium,
              ),
              const Spacer(),
              IconButton(
                icon: const Icon(Icons.remove),
                onPressed: _count > 1
                    ? () {
                        setState(() {
                          _count--;
                        });
                      }
                    : null,
              ),
              Text('$_count', style: Theme.of(context).textTheme.titleMedium),
              IconButton(
                icon: const Icon(Icons.add),
                onPressed: _count < maxCount
                    ? () {
                        setState(() {
                          _count++;
                        });
                      }
                    : null,
              ),
              Text('/ $maxCount', style: Theme.of(context).textTheme.bodySmall),
            ],
          ),

          // ── Remarks ───────────────────────────────────────────────────────
          const SizedBox(height: 12),
          TextField(
            controller: _remarksCtrl,
            decoration: InputDecoration(
              labelText: l10n.historyLabelRemarks,
              border: const OutlineInputBorder(),
              isDense: true,
            ),
            maxLines: 2,
          ),

          // ── Extend due date instead of returning ─────────────────────────
          if (_selectedLending != null && _canExtendLendings()) ...[
            const SizedBox(height: 8),
            Align(
              alignment: Alignment.centerLeft,
              child: TextButton.icon(
                onPressed: _submitting ? null : _onExtend,
                icon: const Icon(Icons.update),
                label: Text(l10n.extendDueDateButton),
              ),
            ),
          ],
        ],
      ),
    );
  }

  // ── Shared widget helpers ─────────────────────────────────────────────────

  /// A small coloured chip to confirm a selection (item or lending).
  Widget _buildInfoChip(IconData icon, String label) {
    return Chip(
      avatar: Icon(icon, size: 16),
      label: Text(label, overflow: TextOverflow.ellipsis),
    );
  }
}

// ─── _ReturnOpenTab ───────────────────────────────────────────────────────────

/// Tab displaying all currently open (unreturned) lendings.
///
/// Uses `GET /api/lendings/due` with no date filter to return every lending
/// that still has outstanding copies regardless of due date.  Supports
/// pull-to-refresh.
class _ReturnOpenTab extends ConsumerStatefulWidget {
  /// Creates a [_ReturnOpenTab].
  const _ReturnOpenTab();

  @override
  ConsumerState<_ReturnOpenTab> createState() => _ReturnOpenTabState();
}

class _ReturnOpenTabState extends ConsumerState<_ReturnOpenTab> {
  List<DueLendingRow> _rows = [];
  bool _loading = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    _load();
  }

  /// Fetches all open lendings from the server and updates the list.
  Future<void> _load() async {
    if (!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final rows = await ref.read(lendingsApiProvider).getDue();
      if (!mounted) return;
      setState(() {
        _rows = rows;
        _loading = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _error = e;
        _loading = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['lendings', 'returns', 'items', 'persons'], _load);
    final l10n = AppLocalizations.of(context)!;
    if (_loading && _rows.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null && _rows.isEmpty) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (_rows.isEmpty) {
      return Center(child: Text(l10n.returnOpenNoResults));
    }
    return RefreshIndicator(
      onRefresh: _load,
      child: ListView.separated(
        itemCount: _rows.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (ctx, i) {
          final row = _rows[i];
          return ListTile(
            leading: const Icon(Icons.book_outlined, color: Colors.blue),
            title: Text(row.itemName),
            subtitle: Text(
              '${row.personName}  ·  ${l10n.labelDueDate}: ${_fmtDate(row.duedate)}',
            ),
            trailing: Text(
              l10n.lendingDueOutstanding(row.outstandingCount),
              style: const TextStyle(fontSize: 12),
            ),
          );
        },
      ),
    );
  }
}

// ─── Pure helper functions ────────────────────────────────────────────────────

/// Extracts a concise, human-readable error message from any exception.
/// Formats [d] as an ISO 8601 date string (YYYY-MM-DD).
String _fmtDate(DateTime d) => d.toIso8601String().substring(0, 10);
