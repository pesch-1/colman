// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../api/lendings_api.dart';
import '../api/reservations_api.dart';
import '../models/due_lending_row.dart';
import '../models/item.dart';
import '../models/item_type.dart';
import '../models/person.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/extend_lending_dialog.dart';
import '../widgets/item_detail_body.dart';
import '../widgets/person_search_field.dart';
import '../widgets/server_error_dialog.dart';

// ─── LendingInitialData ───────────────────────────────────────────────────────

/// Initial data to pre-configure a [LendingScreen] from a barcode scan.
///
/// Set [person] to pre-select the borrower (wizard starts at step 1).
/// Set [item] to pre-select the item; the wizard still starts at step 0
/// (person selection) but skips step 1 after the person is confirmed.
class LendingInitialData {
  const LendingInitialData({this.person, this.item, this.initialTabIndex = 0});

  /// Pre-selected borrower; wizard starts at step 1 (item selection).
  final Person? person;

  /// Pre-selected item; wizard starts at step 0 but jumps from step 0 to
  /// step 2 (details) after the person is confirmed.
  final SearchItemRow? item;

  /// Tab to open on entry: 0 = New, 1 = Open, 2 = Due.
  final int initialTabIndex;
}

// ─── LendingScreen ────────────────────────────────────────────────────────────

/// Main lending screen with three tabs: new lending wizard, open lendings
/// list, and due-date filtered lendings list.
class LendingScreen extends ConsumerStatefulWidget {
  const LendingScreen({super.key, this.initialData});

  /// Optional pre-configuration from a barcode scan dispatch.
  final LendingInitialData? initialData;

  @override
  ConsumerState<LendingScreen> createState() => _LendingScreenState();
}

class _LendingScreenState extends ConsumerState<LendingScreen> {
  // ── Wizard state ─────────────────────────────────────────────────────────

  /// Current active step index (0 = person, 1 = item, 2 = details).
  int _wizardStep = 0;

  /// Person selected in step 0.
  Person? _selectedPerson;

  /// Item row selected in step 1.
  SearchItemRow? _selectedItem;

  /// Full item fetched when advancing from step 1; needed for allowReservations.
  Item? _fullItem;

  /// Item type fetched in parallel with [_fullItem]; needed for custom fields.
  ItemType? _fullItemType;

  /// True while [_fetchFullItemForPreview] is in flight.
  bool _fullItemLoading = false;

  /// Number of copies to lend/reserve.
  int _count = 1;

  /// True when the action is "reserve" rather than "lend".
  bool _isReserve = false;

  /// Whether the due-date checkbox is checked (lending only).
  bool _dueDateEnabled = false;

  /// Due date for lending (valid when _dueDateEnabled is true).
  DateTime _duedate = DateTime.now().add(const Duration(days: 14));

  /// Today plus the collection's default lending period, or `null` when the
  /// collection has none (or it could not be loaded yet).
  DateTime? _defaultDueDate;

  /// Whether the expiry-date checkbox is checked (reservation only).
  bool _expiryEnabled = false;

  /// Expiry date for reservation (valid when _expiryEnabled is true).
  DateTime _expiresAt = DateTime.now().add(const Duration(days: 14));

  final TextEditingController _remarksCtrl = TextEditingController();
  final TextEditingController _itemSearchCtrl = TextEditingController();
  Timer? _itemDebounce;

  /// Current item search results shown in step 1.
  List<SearchItemRow> _itemResults = [];

  /// True while the item search request is in flight.
  bool _itemLoading = false;

  /// True while fetching the full item or submitting the form.
  bool _submitting = false;

  /// Non-null after a successful lending or reservation; drives success card.
  int? _successId;

  /// True when _successId refers to a reservation (false = lending).
  bool _successIsReserve = false;

  @override
  void initState() {
    super.initState();
    final initial = widget.initialData;
    if (initial?.person != null) {
      _selectedPerson = initial!.person;
      _wizardStep = 1;
    }
    if (initial?.item != null) {
      _selectedItem = initial!.item;
    }
    _loadDefaultDueDate();
  }

  /// Loads the collection's default lending period and presets the due date.
  ///
  /// Failures are ignored: the due date then simply stays unchecked, as it
  /// was before the collection setting existed.
  Future<void> _loadDefaultDueDate() async {
    try {
      final info = await ref.read(collectionApiProvider).get();
      if (!mounted) return;
      setState(() {
        _defaultDueDate = info.defaultDueDate();
        _resetDueDate();
      });
    } catch (_) {
      // keep the built-in default
    }
  }

  /// Resets the due date to the collection default (call inside setState).
  void _resetDueDate() {
    _dueDateEnabled = _defaultDueDate != null;
    _duedate =
        _defaultDueDate ?? DateTime.now().add(const Duration(days: 14));
  }

  @override
  void dispose() {
    _itemDebounce?.cancel();
    _remarksCtrl.dispose();
    _itemSearchCtrl.dispose();
    super.dispose();
  }

  // ── Item preview fetch ────────────────────────────────────────────────────

  /// Fetches the full [Item] and its [ItemType] for the inline detail preview.
  ///
  /// Runs in the background after an item is selected in step 1.  Errors are
  /// silently swallowed — the preview degrades gracefully to a name chip.
  Future<void> _fetchFullItemForPreview(int id) async {
    setState(() {
      _fullItemLoading = true;
      _fullItem = null;
      _fullItemType = null;
    });
    try {
      final itemsApi = ref.read(itemsApiProvider);
      final item = await itemsApi.getById(id);
      if (!mounted) return;
      final type = await ref.read(typesApiProvider).getById(item.typeId);
      if (!mounted) return;
      setState(() {
        _fullItem = item;
        _fullItemType = type;
        _fullItemLoading = false;
      });
    } catch (_) {
      if (!mounted) return;
      setState(() {
        _fullItemLoading = false;
      });
    }
  }

  // ── Item search (step 1) ──────────────────────────────────────────────────

  /// Called on every keystroke in the item search field; debounces by 300 ms.
  void _scheduleItemSearch(String query) {
    _itemDebounce?.cancel();
    if (query.length < 2) {
      setState(() {
        _itemResults = [];
      });
      return;
    }
    _itemDebounce = Timer(
      const Duration(milliseconds: 300),
      () => _runItemSearch(query),
    );
  }

  Future<void> _runItemSearch(String query) async {
    setState(() {
      _itemLoading = true;
    });
    try {
      final results = await ref
          .read(itemsApiProvider)
          .search(SearchQuery(general: query));
      if (!mounted) return;
      setState(() {
        _itemResults = results;
        _itemLoading = false;
      });
    } catch (_) {
      if (!mounted) return;
      setState(() {
        _itemLoading = false;
      });
    }
  }

  // ── Wizard navigation ─────────────────────────────────────────────────────

  /// Validates the current step and advances to the next one.
  ///
  /// Step 1 → 2 triggers a full item fetch to read allowReservations.
  void _onContinue() {
    final l10n = AppLocalizations.of(context)!;
    switch (_wizardStep) {
      case 0:
        if (_selectedPerson == null) {
          _showSnack(l10n.labelPersonSearch);
          return;
        }
        // Skip item selection when an item was pre-selected by a barcode scan.
        if (widget.initialData?.item != null) {
          _loadFullItem();
        } else {
          setState(() {
            _wizardStep = 1;
          });
        }
      case 1:
        if (_selectedItem == null) {
          _showSnack(l10n.lendingStep2Title);
          return;
        }
        _loadFullItem();
      case 2:
        _submit();
    }
  }

  /// Goes back one step (or does nothing at step 0).
  void _onCancel() {
    if (_wizardStep > 0) {
      setState(() {
        _wizardStep -= 1;
      });
    }
  }

  /// Advances the wizard to step 2, fetching the full [Item] if not yet loaded.
  ///
  /// When [_fetchFullItemForPreview] has already run successfully (the common
  /// case after the user taps a search result), the network call is skipped and
  /// the wizard advances immediately.
  Future<void> _loadFullItem() async {
    if (_fullItem != null && !_fullItemLoading) {
      final days = _fullItem!.reservationPeriodDays ?? 14;
      setState(() {
        _expiresAt = DateTime.now().add(Duration(days: days));
        _wizardStep = 2;
      });
      return;
    }
    setState(() {
      _submitting = true;
    });
    try {
      final item = await ref.read(itemsApiProvider).getById(_selectedItem!.id);
      if (!mounted) return;
      ItemType? type;
      try {
        type = await ref.read(typesApiProvider).getById(item.typeId);
      } catch (_) {
        // type is supplementary — proceed without it
      }
      if (!mounted) return;
      final days = item.reservationPeriodDays ?? 14;
      setState(() {
        _fullItem = item;
        _fullItemType = type;
        _expiresAt = DateTime.now().add(Duration(days: days));
        _wizardStep = 2;
        _submitting = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      await ServerErrorDialog.show(context, e, AppLocalizations.of(context)!);
    }
  }

  // ── Submission ────────────────────────────────────────────────────────────

  /// Submits the lending or reservation and transitions to the success card.
  Future<void> _submit() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() {
      _submitting = true;
    });
    try {
      if (_isReserve) {
        final id = await ref
            .read(reservationsApiProvider)
            .create(
              itemId: _selectedItem!.id,
              personId: _selectedPerson!.id,
              count: _count,
              expiresAt: _expiryEnabled
                  ? _expiresAt
                  : DateTime.now().add(
                      Duration(days: _fullItem?.reservationPeriodDays ?? 14),
                    ),
            );
        if (!mounted) return;
        setState(() {
          _successId = id;
          _successIsReserve = true;
          _submitting = false;
        });
      } else {
        final data = <String, dynamic>{
          'item_id': _selectedItem!.id,
          'person_id': _selectedPerson!.id,
          'count': _count,
        };
        if (_dueDateEnabled) {
          data['duedate'] = _fmtDate(_duedate);
        }
        if (_remarksCtrl.text.isNotEmpty) {
          data['remarks'] = _remarksCtrl.text;
        }
        final id = await ref.read(lendingsApiProvider).create(data);
        if (!mounted) return;
        setState(() {
          _successId = id;
          _successIsReserve = false;
          _submitting = false;
        });
      }
    } on LendingConflictException catch (e) {
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      await _showErrorDialog(l10n.lendingConflictReserved(e.notReservedCount));
    } on LendingUnavailableException catch (_) {
      // The item was lent out by someone else in the meantime. Discard the
      // now-stale selection so the user re-picks from fresh data.
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      await _showErrorDialog(l10n.errorNotEnoughAvailable);
      if (mounted) _resetWizard();
    } on ReservationException catch (e) {
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      final msg = switch (e.error) {
        ReservationError.alreadyExists => l10n.errorAlreadyReserved,
        ReservationError.notEnoughAvailable => l10n.errorNotEnoughAvailable,
        ReservationError.disabled => l10n.errorReservationsDisabled,
        _ => l10n.errorRequestFailed,
      };
      await _showErrorDialog(msg);
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  // ── Reset helpers ─────────────────────────────────────────────────────────

  /// Resets the wizard completely back to step 0.
  void _resetWizard() {
    _itemDebounce?.cancel();
    _remarksCtrl.clear();
    _itemSearchCtrl.clear();
    setState(() {
      _wizardStep = 0;
      _selectedPerson = null;
      _selectedItem = null;
      _fullItem = null;
      _fullItemType = null;
      _fullItemLoading = false;
      _count = 1;
      _isReserve = false;
      _resetDueDate();
      _expiryEnabled = false;
      _expiresAt = DateTime.now().add(const Duration(days: 14));
      _itemResults = [];
      _itemLoading = false;
      _submitting = false;
      _successId = null;
      _successIsReserve = false;
    });
  }

  /// Keeps the selected person and resets back to step 1 for a new item.
  void _resetToStep1() {
    _itemDebounce?.cancel();
    _remarksCtrl.clear();
    _itemSearchCtrl.clear();
    setState(() {
      _wizardStep = 1;
      _selectedItem = null;
      _fullItem = null;
      _fullItemType = null;
      _fullItemLoading = false;
      _count = 1;
      _isReserve = false;
      _resetDueDate();
      _expiryEnabled = false;
      _expiresAt = DateTime.now().add(const Duration(days: 14));
      _itemResults = [];
      _itemLoading = false;
      _submitting = false;
      _successId = null;
      _successIsReserve = false;
    });
  }

  // ── Date pickers ──────────────────────────────────────────────────────────

  Future<void> _pickDueDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _duedate,
      firstDate: DateTime.now(),
      lastDate: DateTime.now().add(const Duration(days: 365 * 5)),
    );
    if (picked != null && mounted) {
      setState(() {
        _duedate = picked;
      });
    }
  }

  Future<void> _pickExpiryDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _expiresAt,
      firstDate: DateTime.now(),
      lastDate: DateTime.now().add(const Duration(days: 365 * 5)),
    );
    if (picked != null && mounted) {
      setState(() {
        _expiresAt = picked;
      });
    }
  }

  // ── Utility ───────────────────────────────────────────────────────────────

  /// Shows a brief SnackBar hint (e.g. "please select a person first").
  ///
  /// Use [_showErrorDialog] instead for errors that result from a failed
  /// server operation.
  void _showSnack(String message) {
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(SnackBar(content: Text(message)));
  }

  /// Shows a modal error dialog that the user must explicitly dismiss.
  ///
  /// Prefer this over SnackBar for errors that block the user's action —
  /// a dialog cannot be accidentally missed or auto-dismissed.
  Future<void> _showErrorDialog(String message) {
    return showDialog<void>(
      context: context,
      builder: (ctx) => AlertDialog(
        icon: Icon(
          Icons.error_outline,
          color: Theme.of(ctx).colorScheme.error,
          size: 40,
        ),
        content: Text(message),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(ctx).pop(),
            child: Text(MaterialLocalizations.of(ctx).okButtonLabel),
          ),
        ],
      ),
    );
  }

  // ── Build ─────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canViewHistory =
        auth is Authenticated && auth.hasPermission('view_history');
    final tabs = [
      Tab(text: l10n.lendingTabNew),
      if (canViewHistory) Tab(text: l10n.lendingTabOpen),
      if (canViewHistory) Tab(text: l10n.lendingTabDue),
    ];
    final tabViews = [
      _buildNewTab(context, l10n),
      if (canViewHistory) const _OpenTab(),
      if (canViewHistory) const _DueTab(),
    ];
    final initialIndex = canViewHistory
        ? (widget.initialData?.initialTabIndex ?? 0)
        : 0;
    return DefaultTabController(
      length: tabs.length,
      initialIndex: initialIndex,
      child: Scaffold(
        appBar: AppBar(
          title: AppBarTitle(
            icon: 'assets/icons/lending.jpeg',
            title: l10n.lendingTitle,
          ),
          bottom: TabBar(tabs: tabs),
        ),
        body: TabBarView(children: tabViews),
      ),
    );
  }

  /// Builds the "New Lending" tab: either the wizard Stepper or the success card.
  Widget _buildNewTab(BuildContext context, AppLocalizations l10n) {
    if (_successId != null) {
      return _buildSuccessCard(context, l10n);
    }
    return _buildStepper(context, l10n);
  }

  // ── Success card ──────────────────────────────────────────────────────────

  /// Shown after a lending or reservation is successfully created.
  Widget _buildSuccessCard(BuildContext context, AppLocalizations l10n) {
    final label = _successIsReserve
        ? l10n.lendingSuccessReserveId(_successId!)
        : l10n.lendingSuccessLendId(_successId!);
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Icon(Icons.check_circle, color: Colors.green, size: 64),
            const SizedBox(height: 16),
            Text(
              label,
              style: Theme.of(context).textTheme.headlineSmall,
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 32),
            ElevatedButton.icon(
              onPressed: _resetWizard,
              icon: const Icon(Icons.add),
              label: Text(l10n.lendingNewLending),
            ),
            const SizedBox(height: 12),
            OutlinedButton.icon(
              onPressed: _resetToStep1,
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.lendingNewItemSamePerson),
            ),
          ],
        ),
      ),
    );
  }

  // ── Stepper wizard ────────────────────────────────────────────────────────

  /// Builds the three-step wizard for creating a new lending or reservation.
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
        _buildPersonStep(l10n),
        _buildItemStep(l10n),
        _buildDetailsStep(l10n),
      ],
    );
  }

  /// Custom controls row with loading indicator while submitting.
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
                    ? (_isReserve ? l10n.actionReserve : l10n.actionLend)
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

  // ── Step 0: Person ────────────────────────────────────────────────────────

  /// True when the current user may extend lending periods.
  bool _canExtendLendings() {
    final auth = ref.read(authProvider).value;
    return auth is Authenticated && auth.hasPermission('collect_lendings');
  }

  Step _buildPersonStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.lendingStepPerson),
      subtitle: Text(l10n.lendingStep1Title),
      isActive: _wizardStep >= 0,
      state: _selectedPerson != null ? StepState.complete : StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          PersonSearchField(
            personsApi: ref.read(personsApiProvider),
            labelText: l10n.labelPersonSearch,
            onSelected: (p) {
              setState(() {
                _selectedPerson = p;
              });
            },
            initialText: _selectedPerson != null
                ? '${_selectedPerson!.firstname} ${_selectedPerson!.lastname}'
                : null,
          ),
          if (_selectedPerson != null) ...[
            const SizedBox(height: 12),
            _buildInfoChip(
              Icons.person,
              '${_selectedPerson!.firstname} ${_selectedPerson!.lastname}',
            ),
            if (_canExtendLendings()) ...[
              const SizedBox(height: 8),
              Align(
                alignment: Alignment.centerLeft,
                child: TextButton.icon(
                  onPressed: () => showExtendLendingDialog(
                    context,
                    personId: _selectedPerson!.id,
                  ),
                  icon: const Icon(Icons.update),
                  label: Text(l10n.extendDueDateButton),
                ),
              ),
            ],
          ],
        ],
      ),
    );
  }

  // ── Step 1: Item ──────────────────────────────────────────────────────────

  Step _buildItemStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.lendingStepItem),
      subtitle: Text(l10n.lendingStep2Title),
      isActive: _wizardStep >= 1,
      state: _selectedItem != null ? StepState.complete : StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // Show item details when one is selected.
          if (_selectedItem != null) ...[
            if (_fullItemLoading)
              const Center(child: CircularProgressIndicator())
            else if (_fullItem != null)
              ItemDetailBody(item: _fullItem!, itemType: _fullItemType)
            else
              _buildInfoChip(Icons.book, _selectedItem!.name),
            const SizedBox(height: 8),
            TextButton.icon(
              onPressed: () {
                setState(() {
                  _selectedItem = null;
                  _fullItem = null;
                  _fullItemType = null;
                });
              },
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.lendingStep2Title),
            ),
          ] else ...[
            TextField(
              controller: _itemSearchCtrl,
              decoration: InputDecoration(
                hintText: l10n.lendingItemSearchHint,
                border: const OutlineInputBorder(),
                prefixIcon: const Icon(Icons.search),
                isDense: true,
              ),
              onChanged: _scheduleItemSearch,
            ),
            const SizedBox(height: 8),
            if (_itemLoading)
              const Center(child: CircularProgressIndicator())
            else if (_itemSearchCtrl.text.isNotEmpty && _itemResults.isEmpty)
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 8),
                child: Text(
                  l10n.lendingItemNotFound,
                  style: TextStyle(
                    color: Theme.of(context).colorScheme.secondary,
                  ),
                ),
              )
            else if (_itemSearchCtrl.text.isEmpty)
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 8),
                child: Text(
                  l10n.lendingItemSearchPrompt,
                  style: TextStyle(
                    color: Theme.of(context).colorScheme.secondary,
                  ),
                ),
              )
            else
              // ConstrainedBox gives the ListView a bounded maxHeight so it
              // can scroll independently within the Stepper column.
              // shrinkWrap must NOT be set here: it collapses the scroll
              // extent to the visible height, making scrolling impossible.
              ConstrainedBox(
                constraints: const BoxConstraints(maxHeight: 400),
                child: ListView.separated(
                  physics: const ClampingScrollPhysics(),
                  itemCount: _itemResults.length,
                  separatorBuilder: (_, __) => const Divider(height: 1),
                  itemBuilder: (ctx, i) {
                    final row = _itemResults[i];
                    final available = row.available > 0;
                    return ListTile(
                      dense: true,
                      enabled: available,
                      title: Text(row.name),
                      subtitle: Text(row.typeLocale),
                      trailing: Text(
                        '${row.available}/${row.count}',
                        style: TextStyle(
                          color: available ? Colors.green : Colors.red,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      onTap: available
                          ? () {
                              setState(() {
                                _selectedItem = row;
                              });
                              _fetchFullItemForPreview(row.id);
                            }
                          : null,
                    );
                  },
                ),
              ),
          ],
        ],
      ),
    );
  }

  // ── Step 2: Details ───────────────────────────────────────────────────────

  Step _buildDetailsStep(AppLocalizations l10n) {
    final maxCount = _selectedItem?.available ?? 1;
    return Step(
      title: Text(l10n.lendingStepDetails),
      subtitle: Text(l10n.lendingStep3Title),
      isActive: _wizardStep >= 2,
      state: StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // ── Selected item reference ───────────────────────────────────────
          if (_selectedItem != null) ...[
            _buildInfoChip(Icons.book, _selectedItem!.name),
            const SizedBox(height: 12),
          ],
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

          // ── Lend / Reserve toggle (only when allowReservations is true) ──
          if (_fullItem?.allowReservations == true) ...[
            const SizedBox(height: 12),
            SegmentedButton<bool>(
              segments: [
                ButtonSegment(
                  value: false,
                  label: Text(l10n.actionLend),
                  icon: const Icon(Icons.arrow_forward),
                ),
                ButtonSegment(
                  value: true,
                  label: Text(l10n.actionReserve),
                  icon: const Icon(Icons.bookmark_outline),
                ),
              ],
              selected: {_isReserve},
              onSelectionChanged: (selection) {
                setState(() {
                  _isReserve = selection.first;
                });
              },
            ),
          ],

          // ── Due date (lending only) ───────────────────────────────────────
          if (!_isReserve) ...[
            const SizedBox(height: 8),
            CheckboxListTile(
              dense: true,
              title: Text(l10n.labelDueDate),
              value: _dueDateEnabled,
              onChanged: (v) {
                setState(() {
                  _dueDateEnabled = v ?? false;
                });
              },
              controlAffinity: ListTileControlAffinity.leading,
              contentPadding: EdgeInsets.zero,
            ),
            if (_dueDateEnabled)
              _DateField(
                label: l10n.labelDueDate,
                date: _duedate,
                enabled: true,
                onTap: _pickDueDate,
              ),
          ],

          // ── Expiry date (reservation only) ────────────────────────────────
          if (_isReserve) ...[
            const SizedBox(height: 8),
            CheckboxListTile(
              dense: true,
              title: Text(l10n.labelExpiryDate),
              value: _expiryEnabled,
              onChanged: (v) {
                setState(() {
                  _expiryEnabled = v ?? false;
                });
              },
              controlAffinity: ListTileControlAffinity.leading,
              contentPadding: EdgeInsets.zero,
            ),
            if (_expiryEnabled)
              _DateField(
                label: l10n.labelExpiryDate,
                date: _expiresAt,
                enabled: true,
                onTap: _pickExpiryDate,
              ),
          ],

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
        ],
      ),
    );
  }

  // ── Shared widget helpers ─────────────────────────────────────────────────

  /// A small coloured chip to confirm a selection (person or item).
  Widget _buildInfoChip(IconData icon, String label) {
    return Chip(
      avatar: Icon(icon, size: 16),
      label: Text(label, overflow: TextOverflow.ellipsis),
    );
  }
}

// ─── _OpenTab ─────────────────────────────────────────────────────────────────

/// Tab displaying all currently open (unreturned) lendings.
///
/// Uses `GET /api/lendings/due` with no date filter — the same approach as the
/// Qt REST client's "Open Lendings" page — which returns every lending that
/// still has outstanding copies, regardless of due date, and includes the
/// item name via the SQL join in `query_due_lendings`.
///
/// Loads the list on mount and supports pull-to-refresh.
class _OpenTab extends ConsumerStatefulWidget {
  const _OpenTab();

  @override
  ConsumerState<_OpenTab> createState() => _OpenTabState();
}

class _OpenTabState extends ConsumerState<_OpenTab> {
  List<DueLendingRow> _rows = [];
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
      // No date arguments → server skips the date WHERE clause and returns
      // all open lendings, including those without a due date.
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
    watchTables(ref, const ['lendings', 'items', 'persons'], _load);
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
      return Center(child: Text(l10n.lendingOpenNoResults));
    }
    return RefreshIndicator(
      onRefresh: _load,
      child: ListView.separated(
        itemCount: _rows.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (ctx, i) {
          final row = _rows[i];
          final duePart = '  ·  ${l10n.labelDueDate}: ${_fmtDate(row.duedate)}';
          return ListTile(
            leading: const Icon(Icons.book_outlined, color: Colors.blue),
            title: Text(row.itemName),
            subtitle: Text('${row.personName}$duePart'),
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

// ─── _DueTab ──────────────────────────────────────────────────────────────────

/// Tab displaying lendings with due dates in a user-selectable date range.
///
/// Defaults to today ± 30 days.  The user can adjust the range and tap Apply.
class _DueTab extends ConsumerStatefulWidget {
  const _DueTab();

  @override
  ConsumerState<_DueTab> createState() => _DueTabState();
}

class _DueTabState extends ConsumerState<_DueTab> {
  late DateTime _fromDate;
  late DateTime _toDate;
  List<DueLendingRow> _rows = [];
  bool _loading = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    final now = DateTime.now();
    _fromDate = DateTime(
      now.year,
      now.month,
      now.day,
    ).subtract(const Duration(days: 30));
    _toDate = DateTime(
      now.year,
      now.month,
      now.day,
    ).add(const Duration(days: 30));
    _load();
  }

  Future<void> _load() async {
    if (!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final rows = await ref
          .read(lendingsApiProvider)
          .getDue(from: _fromDate, to: _toDate);
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

  Future<void> _pickFromDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _fromDate,
      firstDate: DateTime(2000),
      lastDate: _toDate,
    );
    if (picked != null && mounted) {
      setState(() {
        _fromDate = picked;
      });
    }
  }

  Future<void> _pickToDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _toDate,
      firstDate: _fromDate,
      lastDate: DateTime.now().add(const Duration(days: 365 * 5)),
    );
    if (picked != null && mounted) {
      setState(() {
        _toDate = picked;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['lendings', 'items', 'persons'], _load);
    final l10n = AppLocalizations.of(context)!;
    return Column(
      children: [
        // ── Filter bar ──────────────────────────────────────────────────────
        Padding(
          padding: const EdgeInsets.all(8),
          child: Row(
            children: [
              Expanded(
                child: _DateField(
                  label: l10n.historyFromDate,
                  date: _fromDate,
                  enabled: true,
                  onTap: _pickFromDate,
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: _DateField(
                  label: l10n.historyToDate,
                  date: _toDate,
                  enabled: true,
                  onTap: _pickToDate,
                ),
              ),
              const SizedBox(width: 8),
              TextButton(
                onPressed: _loading ? null : _load,
                child: Text(l10n.historyApply),
              ),
            ],
          ),
        ),
        // ── Results ─────────────────────────────────────────────────────────
        Expanded(child: _buildBody(l10n)),
      ],
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if (_loading && _rows.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null && _rows.isEmpty) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (_rows.isEmpty) {
      return Center(child: Text(l10n.lendingDueNoResults));
    }
    return RefreshIndicator(
      onRefresh: _load,
      child: ListView.separated(
        itemCount: _rows.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (ctx, i) {
          final row = _rows[i];
          final isOverdue = row.duedate.isBefore(DateTime.now());
          return ListTile(
            leading: CircleAvatar(
              backgroundColor: isOverdue
                  ? Colors.red.shade100
                  : Colors.orange.shade100,
              child: Icon(
                Icons.warning_amber,
                color: isOverdue ? Colors.red : Colors.orange,
                size: 20,
              ),
            ),
            title: Text(row.itemName),
            subtitle: Text('${row.personName}  ·  ${_fmtDate(row.duedate)}'),
            trailing: Text(
              l10n.lendingDueOutstanding(row.outstandingCount),
              style: TextStyle(
                color: isOverdue ? Colors.red : null,
                fontSize: 12,
              ),
            ),
          );
        },
      ),
    );
  }
}

// ─── _DateField ───────────────────────────────────────────────────────────────

/// A tappable read-only date display styled as a form field.
///
/// Copied from history_screen.dart (which keeps it file-private).
class _DateField extends StatelessWidget {
  const _DateField({
    required this.label,
    required this.date,
    required this.enabled,
    required this.onTap,
  });

  final String label;
  final DateTime date;
  final bool enabled;
  final VoidCallback? onTap;

  @override
  Widget build(BuildContext context) {
    return InkWell(
      onTap: onTap,
      borderRadius: BorderRadius.circular(4),
      child: InputDecorator(
        decoration: InputDecoration(
          labelText: label,
          border: const OutlineInputBorder(),
          isDense: true,
          contentPadding: const EdgeInsets.fromLTRB(12, 8, 8, 8),
          suffixIcon: Icon(
            Icons.calendar_today,
            size: 16,
            color: enabled
                ? Theme.of(context).iconTheme.color
                : Theme.of(context).disabledColor,
          ),
        ),
        child: Text(
          _fmtDate(date),
          style: TextStyle(
            color: enabled ? null : Theme.of(context).disabledColor,
          ),
        ),
      ),
    );
  }
}

// ─── Pure helper functions ────────────────────────────────────────────────────

/// Extracts a concise, human-readable error message from any exception.
/// Formats [d] as an ISO 8601 date string (YYYY-MM-DD).
String _fmtDate(DateTime d) => d.toIso8601String().substring(0, 10);
