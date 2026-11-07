// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'barcode_scan_screen.dart';
import '../api/barcode_lookup.dart' show normalizeBarcode;
import '../api/ean_lookup.dart' show isProductEan, isUpcA;
import '../api/isbn_lookup.dart'
    show isIsbn10, isIsbn13, isbn10ToIsbn13;
import '../models/item.dart';
import '../models/item_type.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/item_detail_body.dart';
import '../widgets/server_error_dialog.dart';

// ─── LiquidationInitialData ───────────────────────────────────────────────────

/// Initial data to pre-configure a [LiquidationScreen] from a barcode scan.
///
/// Exactly one of [item] or [barcode] is typically set:
/// - [item]: pre-select an existing item and jump to step 1 (details).
/// - [barcode]: pre-fill the barcode field and trigger the lookup flow.
class LiquidationInitialData {
  const LiquidationInitialData({this.item, this.barcode});

  /// Pre-selected existing item; wizard starts at step 1 (liquidation details).
  final SearchItemRow? item;

  /// Barcode string to pre-fill in the barcode field.
  final String? barcode;
}

// ─── LiquidationScreen ────────────────────────────────────────────────────────

/// Liquidation wizard: records that copies of an item were removed from stock.
///
/// Two-step horizontal stepper: item selection → liquidation details.
/// After a successful POST the screen shows a success card with options to
/// start a new liquidation or liquidate the same item again.
class LiquidationScreen extends ConsumerStatefulWidget {
  /// Creates a [LiquidationScreen].
  const LiquidationScreen({super.key, this.initialData});

  /// Optional pre-configuration from a barcode scan dispatch.
  final LiquidationInitialData? initialData;

  @override
  ConsumerState<LiquidationScreen> createState() =>
      _LiquidationScreenState();
}

class _LiquidationScreenState
    extends ConsumerState<LiquidationScreen> {
  // ── Wizard state ──────────────────────────────────────────────────────────

  /// Current active step index (0 = item, 1 = details).
  int _wizardStep = 0;

  /// Item selected in step 0.
  SearchItemRow? _selectedItem;

  /// Full item fetched for the inline detail preview.
  Item? _fullItem;

  /// Item type fetched in parallel with [_fullItem]; needed for custom fields.
  ItemType? _fullItemType;

  /// True while [_fetchFullItemForPreview] is in flight.
  bool _fullItemLoading = false;

  /// Number of copies to liquidate (at least 1, at most available).
  int _count = 1;

  /// Date part of the liquidation timestamp.
  DateTime _liquidationDate = DateTime.now();

  /// Time part of the liquidation timestamp.
  TimeOfDay _liquidationTime = TimeOfDay.now();

  final TextEditingController _itemSearchCtrl = TextEditingController();
  final TextEditingController _barcodeCtrl = TextEditingController();
  Timer? _itemDebounce;

  /// True while a barcode lookup is in flight.
  bool _barcodeLoading = false;

  /// Status message shown below the barcode field (empty = hidden).
  String _barcodeStatus = '';

  /// True when [_barcodeStatus] describes an error (shown in error colour).
  bool _barcodeIsError = false;

  /// Current item search results shown in step 0.
  List<SearchItemRow> _itemResults = [];

  /// True while the item search request is in flight.
  bool _itemLoading = false;

  /// True while submitting the liquidation to the server.
  bool _submitting = false;

  /// Non-null after a successful liquidation; drives the success card.
  int? _successId;

  @override
  void initState() {
    super.initState();
    final initial = widget.initialData;
    if(initial?.item != null) {
      _selectedItem = initial!.item;
      _wizardStep = 1;
    }
    if(initial?.barcode != null) {
      _barcodeCtrl.text = initial!.barcode!;
    }
  }

  @override
  void dispose() {
    _itemDebounce?.cancel();
    _itemSearchCtrl.dispose();
    _barcodeCtrl.dispose();
    super.dispose();
  }

  // ── Item preview fetch ────────────────────────────────────────────────────

  /// Fetches the full [Item] and its [ItemType] for the inline detail preview.
  ///
  /// Runs in the background after an item is selected.  Errors are silently
  /// swallowed — the preview degrades gracefully to a name chip.
  Future<void> _fetchFullItemForPreview(int id) async {
    setState(() {
      _fullItemLoading = true;
      _fullItem = null;
      _fullItemType = null;
    });
    try {
      final item = await ref.read(itemsApiProvider).getById(id);
      if(!mounted) return;
      final type = await ref.read(typesApiProvider).getById(item.typeId);
      if(!mounted) return;
      setState(() {
        _fullItem = item;
        _fullItemType = type;
        _fullItemLoading = false;
      });
    } catch(_) {
      if(!mounted) return;
      setState(() { _fullItemLoading = false; });
    }
  }

  // ── Item search (step 0) ──────────────────────────────────────────────────

  /// Debounces item search by 300 ms; ignores queries shorter than 2 chars.
  void _scheduleItemSearch(String query) {
    _itemDebounce?.cancel();
    if(query.length < 2) {
      setState(() { _itemResults = []; });
      return;
    }
    _itemDebounce = Timer(
      const Duration(milliseconds: 300),
      () => _runItemSearch(query),
    );
  }

  Future<void> _runItemSearch(String query) async {
    setState(() { _itemLoading = true; });
    try {
      final results = await ref
          .read(itemsApiProvider)
          .search(SearchQuery(general: query));
      if(!mounted) return;
      setState(() {
        _itemResults = results;
        _itemLoading = false;
      });
    } catch(_) {
      if(!mounted) return;
      setState(() { _itemLoading = false; });
    }
  }

  /// Selects [item] and starts the background detail fetch.
  void _selectItem(SearchItemRow item) {
    setState(() {
      _selectedItem = item;
      _count = 1;
    });
    _fetchFullItemForPreview(item.id);
  }

  /// Looks up [code] via a two-step cascade and updates wizard state.
  ///
  /// 1. colman item barcode → select existing item, advance to step 1.
  /// 2. DB lookup by ISBN/EAN → select existing item, advance to step 1.
  /// 3. Nothing found → show status message below the barcode field.
  ///
  /// No internet lookup: liquidation only works on items already in stock.
  Future<void> _onBarcodeScanned(String code) async {
    final clean = normalizeBarcode(code);
    if(clean.isEmpty) return;
    final l10n = AppLocalizations.of(context)!;
    setState(() {
      _barcodeLoading = true;
      _barcodeStatus = '';
      _barcodeIsError = false;
    });
    try {
      // Step 1: colman item barcode
      final parsed =
          await ref.read(itemsApiProvider).parseItemBarcode(clean);
      if(!mounted) return;
      if(parsed != null) {
        final item = await ref.read(itemsApiProvider).getById(parsed.itemId);
        if(!mounted) return;
        ItemType? type;
        try {
          type = await ref.read(typesApiProvider).getById(item.typeId);
        } catch(_) {}
        if(!mounted) return;
        setState(() {
          _barcodeLoading = false;
          _fullItem = item;
          _fullItemType = type;
          _fullItemLoading = false;
          _selectedItem = SearchItemRow(
            id: item.id,
            name: item.name,
            typeLocale: item.typeName ?? '',
            count: 0,
            available: 0,
            typeId: item.typeId,
          );
          _count = 1;
          _wizardStep = 1;
        });
        return;
      }
      // Step 2: DB lookup by ISBN or EAN
      final looksLikeIsbn = isIsbn10(clean) ||
          (isIsbn13(clean) &&
              (clean.startsWith('978') || clean.startsWith('979')));
      Item? dbItem;
      if(looksLikeIsbn) {
        final isbn13 = isIsbn10(clean) ? isbn10ToIsbn13(clean) : clean;
        dbItem = await ref.read(itemsApiProvider).getByIsbn(isbn13);
      } else if(isProductEan(clean)) {
        dbItem = await ref.read(itemsApiProvider).getByEan(clean);
      } else if(isUpcA(clean)) {
        dbItem = await ref.read(itemsApiProvider).getByEan('0$clean');
      }
      if(!mounted) return;
      if(dbItem != null) {
        ItemType? type;
        try {
          type = await ref.read(typesApiProvider).getById(dbItem.typeId);
        } catch(_) {}
        if(!mounted) return;
        setState(() {
          _barcodeLoading = false;
          _fullItem = dbItem;
          _fullItemType = type;
          _fullItemLoading = false;
          _selectedItem = SearchItemRow(
            id: dbItem!.id,
            name: dbItem.name,
            typeLocale: dbItem.typeName ?? '',
            count: 0,
            available: 0,
            typeId: dbItem.typeId,
          );
          _count = 1;
          _wizardStep = 1;
        });
        return;
      }
      setState(() {
        _barcodeLoading = false;
        _barcodeStatus = l10n.acquisitionBarcodeNotFound;
        _barcodeIsError = true;
      });
    } catch(e) {
      if(!mounted) return;
      setState(() {
        _barcodeLoading = false;
        _barcodeStatus = extractHttpDetail(e);
        _barcodeIsError = true;
      });
    }
  }

  /// Opens the camera scanner and feeds the detected barcode into
  /// [_onBarcodeScanned].
  Future<void> _openCameraScanner() async {
    final code = await Navigator.of(context).push<String>(
      MaterialPageRoute(builder: (_) => const BarcodeScanScreen()),
    );
    if(code != null && mounted) {
      _barcodeCtrl.text = code;
      _onBarcodeScanned(code);
    }
  }

  // ── Wizard navigation ─────────────────────────────────────────────────────

  /// Validates the current step and advances to the next one.
  void _onContinue() {
    final l10n = AppLocalizations.of(context)!;
    switch(_wizardStep) {
      case 0:
        if(_selectedItem == null) {
          _showSnack(l10n.liquidationStep1Title);
          return;
        }
        setState(() { _wizardStep = 1; });
      case 1:
        _submit();
    }
  }

  /// Goes back one step, or does nothing at step 0.
  void _onCancel() {
    if(_wizardStep > 0) {
      setState(() { _wizardStep -= 1; });
    }
  }

  // ── Submission ────────────────────────────────────────────────────────────

  /// POSTs the liquidation to the server and transitions to the success card.
  Future<void> _submit() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() { _submitting = true; });
    try {
      final dt = DateTime(
        _liquidationDate.year,
        _liquidationDate.month,
        _liquidationDate.day,
        _liquidationTime.hour,
        _liquidationTime.minute,
      );
      final data = <String, dynamic>{
        'item_id': _selectedItem!.id,
        'count': _count,
        'date_time': dt.toUtc().toIso8601String(),
      };
      final auth = ref.read(authProvider).value;
      if(auth is Authenticated && !auth.isAdmin) {
        data['data_collector_id'] = auth.currentUserId;
      }
      final id = await ref.read(liquidationsApiProvider).create(data);
      if(!mounted) return;
      setState(() {
        _successId = id;
        _submitting = false;
      });
    } catch(e) {
      if(!mounted) return;
      setState(() { _submitting = false; });
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  // ── Reset helpers ─────────────────────────────────────────────────────────

  /// Resets the wizard completely back to step 0.
  void _resetWizard() {
    _itemDebounce?.cancel();
    _itemSearchCtrl.clear();
    _barcodeCtrl.clear();
    final now = DateTime.now();
    setState(() {
      _wizardStep = 0;
      _selectedItem = null;
      _fullItem = null;
      _fullItemType = null;
      _fullItemLoading = false;
      _count = 1;
      _liquidationDate = now;
      _liquidationTime = TimeOfDay.fromDateTime(now);
      _itemResults = [];
      _itemLoading = false;
      _barcodeLoading = false;
      _barcodeStatus = '';
      _barcodeIsError = false;
      _submitting = false;
      _successId = null;
    });
  }

  /// Keeps the selected item and returns to step 1 to enter new details.
  void _resetToSameItem() {
    final now = DateTime.now();
    setState(() {
      _wizardStep = 1;
      _count = 1;
      _liquidationDate = now;
      _liquidationTime = TimeOfDay.fromDateTime(now);
      _submitting = false;
      _successId = null;
    });
  }

  // ── Date/time pickers ─────────────────────────────────────────────────────

  Future<void> _pickDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _liquidationDate,
      firstDate: DateTime(2000),
      lastDate: DateTime.now().add(const Duration(days: 1)),
    );
    if(picked != null && mounted) {
      setState(() { _liquidationDate = picked; });
    }
  }

  Future<void> _pickTime() async {
    final picked = await showTimePicker(
      context: context,
      initialTime: _liquidationTime,
    );
    if(picked != null && mounted) {
      setState(() { _liquidationTime = picked; });
    }
  }

  // ── Utility ───────────────────────────────────────────────────────────────

  void _showSnack(String message) {
    ScaffoldMessenger.of(context)
        .showSnackBar(SnackBar(content: Text(message)));
  }

  // ── Build ─────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/remove_book.jpeg',
          title: l10n.liquidationTitle,
        ),
      ),
      body: _successId != null
          ? _buildSuccessCard(context, l10n)
          : _buildStepper(context, l10n),
    );
  }

  // ── Success card ──────────────────────────────────────────────────────────

  /// Shown after a liquidation is successfully recorded.
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
              l10n.liquidationSuccessId(_successId!),
              style: Theme.of(context).textTheme.headlineSmall,
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 32),
            ElevatedButton.icon(
              onPressed: _resetWizard,
              icon: const Icon(Icons.add),
              label: Text(l10n.liquidationNewLiquidation),
            ),
            const SizedBox(height: 12),
            OutlinedButton.icon(
              onPressed: _resetToSameItem,
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.returnSameItem),
            ),
          ],
        ),
      ),
    );
  }

  // ── Stepper wizard ────────────────────────────────────────────────────────

  /// Builds the two-step wizard for recording a new liquidation.
  Widget _buildStepper(BuildContext context, AppLocalizations l10n) {
    return Stepper(
      type: StepperType.horizontal,
      currentStep: _wizardStep,
      onStepTapped: (i) {
        if(i < _wizardStep) {
          setState(() { _wizardStep = i; });
        }
      },
      onStepContinue: _submitting ? null : _onContinue,
      onStepCancel: _submitting ? null : _onCancel,
      controlsBuilder: _buildStepControls,
      steps: [
        _buildItemStep(l10n),
        _buildDetailsStep(l10n),
      ],
    );
  }

  /// Custom controls row with a loading indicator while submitting.
  Widget _buildStepControls(BuildContext ctx, ControlsDetails details) {
    final l10n = AppLocalizations.of(ctx)!;
    return Padding(
      padding: const EdgeInsets.only(top: 16),
      child: Row(
        children: [
          if(_submitting)
            const SizedBox(
              width: 24,
              height: 24,
              child: CircularProgressIndicator(strokeWidth: 2),
            )
          else
            ElevatedButton(
              onPressed: details.onStepContinue,
              child: Text(_wizardStep == 1
                  ? l10n.navLiquidate
                  : l10n.actionConfirm),
            ),
          const SizedBox(width: 8),
          if(_wizardStep > 0 && !_submitting)
            TextButton(
              onPressed: details.onStepCancel,
              child: Text(
                  MaterialLocalizations.of(ctx).cancelButtonLabel),
            ),
        ],
      ),
    );
  }

  // ── Step 0: Item ──────────────────────────────────────────────────────────

  /// Builds the item-selection step.
  ///
  /// Shows a barcode input field and a name search field.
  Step _buildItemStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.lendingStepItem),
      subtitle: Text(l10n.liquidationStep1Title),
      isActive: _wizardStep >= 0,
      state: _selectedItem != null
          ? StepState.complete
          : StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          if(_selectedItem != null) ...[
            if(_fullItemLoading)
              const Center(child: CircularProgressIndicator())
            else if(_fullItem != null)
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
              label: Text(l10n.liquidationStep1Title),
            ),
          ]
          else ...[
            Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Expanded(
                  child: TextField(
                    controller: _barcodeCtrl,
                    decoration: InputDecoration(
                      hintText: l10n.acquisitionBarcodeHint,
                      border: const OutlineInputBorder(),
                      isDense: true,
                      prefixIcon: const Icon(Icons.qr_code_scanner),
                      suffixIcon: _barcodeLoading
                          ? const Padding(
                              padding: EdgeInsets.all(10),
                              child: SizedBox(
                                width: 16,
                                height: 16,
                                child: CircularProgressIndicator(
                                    strokeWidth: 2),
                              ),
                            )
                          : IconButton(
                              icon: const Icon(Icons.search),
                              onPressed: () =>
                                  _onBarcodeScanned(_barcodeCtrl.text),
                            ),
                    ),
                    onSubmitted: _onBarcodeScanned,
                  ),
                ),
                const SizedBox(width: 4),
                IconButton(
                  icon: const Icon(Icons.camera_alt),
                  tooltip: l10n.barcodeScanTooltip,
                  onPressed:
                      _barcodeLoading ? null : _openCameraScanner,
                ),
              ],
            ),
            if(_barcodeStatus.isNotEmpty) ...[
              const SizedBox(height: 4),
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 4),
                child: Text(
                  _barcodeStatus,
                  style: TextStyle(
                    color: _barcodeIsError
                        ? Theme.of(context).colorScheme.error
                        : Theme.of(context).colorScheme.secondary,
                  ),
                ),
              ),
            ],
            const SizedBox(height: 8),
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
            if(_itemLoading)
              const Center(child: CircularProgressIndicator())
            else if(_itemSearchCtrl.text.isNotEmpty &&
                _itemResults.isEmpty)
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 8),
                child: Text(
                  l10n.lendingItemNotFound,
                  style: TextStyle(
                    color: Theme.of(context).colorScheme.secondary,
                  ),
                ),
              )
            else if(_itemSearchCtrl.text.isEmpty)
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
              ConstrainedBox(
                constraints: const BoxConstraints(maxHeight: 400),
                child: ListView.separated(
                  physics: const ClampingScrollPhysics(),
                  itemCount: _itemResults.length,
                  separatorBuilder: (_, __) => const Divider(height: 1),
                  itemBuilder: (ctx, i) {
                    final row = _itemResults[i];
                    return ListTile(
                      dense: true,
                      title: Text(row.name),
                      subtitle: Text(row.typeLocale),
                      trailing: Text(
                        '${row.available}',
                        style: const TextStyle(fontSize: 12),
                      ),
                      onTap: () => _selectItem(row),
                    );
                  },
                ),
              ),
          ],
        ],
      ),
    );
  }

  // ── Step 1: Details ───────────────────────────────────────────────────────

  /// Builds the liquidation-details step.
  ///
  /// Count is bounded by the available copies of the selected item.
  /// Date and time default to now; both can be changed via pickers.
  Step _buildDetailsStep(AppLocalizations l10n) {
    final maxCount = _selectedItem?.available ?? 0;
    return Step(
      title: Text(l10n.lendingStepDetails),
      subtitle: Text(l10n.liquidationStep2Title),
      isActive: _wizardStep >= 1,
      state: StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // ── Selected item details ─────────────────────────────────────────
          if(_fullItemLoading)
            const Center(child: CircularProgressIndicator()),
          if(_fullItem != null) ...[
            ItemDetailBody(item: _fullItem!, itemType: _fullItemType),
            const SizedBox(height: 16),
          ] else if(_selectedItem != null) ...[
            _buildInfoChip(Icons.book, _selectedItem!.name),
            const SizedBox(height: 12),
          ],
          // ── Count ─────────────────────────────────────────────────────────
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
                    ? () { setState(() { _count--; }); }
                    : null,
              ),
              Text(
                '$_count',
                style: Theme.of(context).textTheme.titleMedium,
              ),
              IconButton(
                icon: const Icon(Icons.add),
                onPressed: (maxCount <= 0 || _count < maxCount)
                    ? () { setState(() { _count++; }); }
                    : null,
              ),
            ],
          ),

          // ── Date + Time ───────────────────────────────────────────────────
          const SizedBox(height: 12),
          Row(
            children: [
              Expanded(
                child: _DateField(
                  label: l10n.historyLabelDateTime,
                  date: _liquidationDate,
                  onTap: _pickDate,
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: _TimeField(
                  time: _liquidationTime,
                  onTap: _pickTime,
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  // ── Shared widget helpers ─────────────────────────────────────────────────

  /// A small chip confirming an item selection.
  Widget _buildInfoChip(IconData icon, String label) {
    return Chip(
      avatar: Icon(icon, size: 16),
      label: Text(label, overflow: TextOverflow.ellipsis),
    );
  }
}

// ─── _DateField ───────────────────────────────────────────────────────────────

/// A tappable read-only date display styled as a form field.
class _DateField extends StatelessWidget {
  const _DateField({
    required this.label,
    required this.date,
    required this.onTap,
  });

  final String label;
  final DateTime date;
  final VoidCallback onTap;

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
          suffixIcon: const Icon(Icons.calendar_today, size: 16),
        ),
        child: Text(_fmtDate(date)),
      ),
    );
  }
}

// ─── _TimeField ───────────────────────────────────────────────────────────────

/// A tappable read-only time display styled as a form field.
class _TimeField extends StatelessWidget {
  const _TimeField({
    required this.time,
    required this.onTap,
  });

  final TimeOfDay time;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final h = time.hour.toString().padLeft(2, '0');
    final m = time.minute.toString().padLeft(2, '0');
    return InkWell(
      onTap: onTap,
      borderRadius: BorderRadius.circular(4),
      child: InputDecorator(
        decoration: const InputDecoration(
          border: OutlineInputBorder(),
          isDense: true,
          contentPadding: EdgeInsets.fromLTRB(12, 8, 8, 8),
          suffixIcon: Icon(Icons.access_time, size: 16),
        ),
        child: Text('$h:$m'),
      ),
    );
  }
}

// ─── Pure helper functions ────────────────────────────────────────────────────

/// Formats [d] as an ISO 8601 date string (YYYY-MM-DD).
String _fmtDate(DateTime d) => d.toIso8601String().substring(0, 10);
