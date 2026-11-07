// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import 'barcode_scan_screen.dart';
import 'label_print_screen.dart' show LabelPrintInitialData;
import '../api/barcode_lookup.dart'
    show queryBarcode, normalizeBarcode, BookSearchResult, AvSearchResult;
import '../api/ean_lookup.dart' show isProductEan, isUpcA;
import '../api/isbn_lookup.dart'
    show isIsbn10, isIsbn13, isbn10ToIsbn13;
import '../api/title_lookup.dart' show titleSearchAvailable;
import '../models/item.dart';
import '../models/item_prefill.dart';
import '../models/item_type.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/internet_services_provider.dart'
    show internetServicesProvider;
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/server_error_dialog.dart';
import '../widgets/thumbnail_widget.dart';
import '../widgets/title_search_dialog.dart';

/// Downloads an image from [url] and converts it to PNG bytes.
///
/// Returns null when the download fails or decoding is not possible.
Future<Uint8List?> _fetchPngBytes(String url, int timeoutMs) async {
  try {
    final dio = Dio(BaseOptions(
      connectTimeout: Duration(milliseconds: timeoutMs),
      receiveTimeout: Duration(milliseconds: timeoutMs),
    ));
    final response = await dio.get<List<int>>(
      url.replaceFirst('http://', 'https://'),
      options: Options(responseType: ResponseType.bytes),
    );
    final bytes = response.data;
    if(bytes == null || bytes.isEmpty) return null;
    final raw = Uint8List.fromList(bytes);
    final codec = await ui.instantiateImageCodec(raw);
    final frame = await codec.getNextFrame();
    final byteData =
        await frame.image.toByteData(format: ui.ImageByteFormat.png);
    return byteData?.buffer.asUint8List();
  } catch(_) {
    return null;
  }
}

// ─── AcquisitionInitialData ───────────────────────────────────────────────────

/// Initial data to pre-configure an [AcquisitionScreen] from a barcode scan.
///
/// Typically one of the following combinations is used:
/// - [item]: pre-select an existing item and jump to step 1 (details).
/// - [prefill]: open the "Create new item" dialog pre-filled from an internet
///   search result (Fall 3 of the barcode dispatch flow).  When the internet
///   search found nothing, a minimal [ItemPrefill] with just the scanned
///   barcode is passed so the dialog still opens and the user can fill in
///   all fields manually.
/// - [barcode]: pre-fill the barcode search field (rarely used directly).
class AcquisitionInitialData {
  const AcquisitionInitialData({this.item, this.barcode, this.prefill});

  /// Pre-selected existing item; wizard starts at step 1 (acquisition details).
  final SearchItemRow? item;

  /// Barcode string to pre-fill in the barcode field.
  final String? barcode;

  /// Pre-filled data for the "Create new item" dialog from an internet lookup.
  final ItemPrefill? prefill;
}

// ─── AcquisitionScreen ────────────────────────────────────────────────────────

/// Acquisition wizard: records that copies of an item were added to stock.
///
/// Two-step horizontal stepper: item selection → acquisition details.
/// After a successful POST the screen shows a success card with options to
/// start a new acquisition or acquire the same item again.
class AcquisitionScreen extends ConsumerStatefulWidget {
  /// Creates an [AcquisitionScreen].
  const AcquisitionScreen({super.key, this.initialData});

  /// Optional pre-configuration from a barcode scan dispatch.
  final AcquisitionInitialData? initialData;

  @override
  ConsumerState<AcquisitionScreen> createState() =>
      _AcquisitionScreenState();
}

class _AcquisitionScreenState
    extends ConsumerState<AcquisitionScreen> {
  // ── Wizard state ──────────────────────────────────────────────────────────

  /// All item types, loaded once on init for the "Create new item" dialog.
  List<ItemType> _types = [];

  /// True while loading types.
  bool _typesLoading = false;

  /// Current active step index (0 = item, 1 = details).
  int _wizardStep = 0;

  /// Item selected in step 0.
  SearchItemRow? _selectedItem;

  /// Number of copies to acquire (no upper limit).
  int _count = 1;

  /// Item data from the "Create new item" dialog, not yet written to the DB.
  /// Cleared once the item is actually created during [_submit].
  Map<String, dynamic>? _pendingNewItemData;

  /// Date part of the acquisition timestamp.
  DateTime _acquisitionDate = DateTime.now();

  /// Time part of the acquisition timestamp.
  TimeOfDay _acquisitionTime = TimeOfDay.now();

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

  /// True while submitting the acquisition to the server.
  bool _submitting = false;

  /// Non-null after a successful acquisition; drives the success card.
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
    _loadTypes();
  }

  @override
  void dispose() {
    _itemDebounce?.cancel();
    _itemSearchCtrl.dispose();
    _barcodeCtrl.dispose();
    super.dispose();
  }

  // ── Types loading ─────────────────────────────────────────────────────────

  Future<void> _loadTypes() async {
    setState(() { _typesLoading = true; });
    try {
      final types = await ref.read(typesApiProvider).getAllWithKeys();
      if(!mounted) return;
      setState(() {
        _types = types;
        _typesLoading = false;
      });
      final prefill = widget.initialData?.prefill;
      if(prefill != null && mounted) {
        WidgetsBinding.instance.addPostFrameCallback(
          (_) => _showCreateItemDialog(prefill: prefill),
        );
      }
    } catch(_) {
      if(!mounted) return;
      setState(() { _typesLoading = false; });
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

  /// Selects [item] and advances the wizard to step 1.
  void _selectItem(SearchItemRow item) {
    setState(() { _selectedItem = item; });
  }

  /// Looks up [code] via a three-step cascade and updates wizard state.
  ///
  /// 1. colman item barcode → select existing item, advance to step 1.
  /// 2. DB lookup by ISBN/EAN → select existing item, advance to step 1.
  /// 3. Internet search → open "Create new item" dialog pre-filled.
  /// 4. Nothing found → show status message below the barcode field.
  Future<void> _onBarcodeScanned(String code) async {
    final clean = normalizeBarcode(code);
    if(clean.isEmpty) return;
    final l10n = AppLocalizations.of(context)!;
    debugPrint('[barcode] scan: $clean');
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
      debugPrint('[barcode] parseItemBarcode → '
          '${parsed != null ? "itemId=${parsed.itemId}" : "null"}');
      if(parsed != null) {
        final item = await ref.read(itemsApiProvider).getById(parsed.itemId);
        if(!mounted) return;
        debugPrint('[barcode] getById ${parsed.itemId} → "${item.name}"');
        final typeName = _types
            .where((t) => t.id == item.typeId)
            .map((t) => t.name)
            .firstOrNull ?? '';
        setState(() {
          _barcodeLoading = false;
          _selectedItem = SearchItemRow(
            id: item.id, name: item.name,
            typeLocale: typeName, count: 0,
            available: 0, typeId: item.typeId,
          );
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
        debugPrint('[barcode] getByIsbn $isbn13');
        dbItem = await ref.read(itemsApiProvider).getByIsbn(isbn13);
      } else if(isProductEan(clean)) {
        debugPrint('[barcode] getByEan $clean');
        dbItem = await ref.read(itemsApiProvider).getByEan(clean);
      } else if(isUpcA(clean)) {
        debugPrint('[barcode] getByEan 0$clean (UPC-A padded)');
        dbItem = await ref.read(itemsApiProvider).getByEan('0$clean');
      } else {
        debugPrint('[barcode] unclassifiable — skip DB lookup');
      }
      if(!mounted) return;
      if(dbItem != null) {
        debugPrint('[barcode] DB lookup → id=${dbItem.id} "${dbItem.name}"');
      } else {
        debugPrint('[barcode] DB lookup → null');
      }
      if(dbItem != null) {
        final typeName = _types
            .where((t) => t.id == dbItem!.typeId)
            .map((t) => t.name)
            .firstOrNull ?? '';
        setState(() {
          _barcodeLoading = false;
          _selectedItem = SearchItemRow(
            id: dbItem!.id, name: dbItem.name,
            typeLocale: typeName, count: 0,
            available: 0, typeId: dbItem.typeId,
          );
          _wizardStep = 1;
        });
        return;
      }
      // Step 3: internet search
      debugPrint('[barcode] not in DB — querying internet');
      setState(() { _barcodeStatus = l10n.acquisitionBarcodeSearchingInternet; });
      final settings = await ref.read(internetServicesProvider.future);
      if(!mounted) return;
      final result = await queryBarcode(clean, settings);
      if(!mounted) return;
      debugPrint('[barcode] internet → ${result.runtimeType}');
      ItemPrefill? prefill;
      if(result is BookSearchResult && !result.data.isEmpty) {
        debugPrint('[barcode] book: "${result.data.name}"');
        prefill = bookToItemPrefill(result.data, clean);
      } else if(result is AvSearchResult && !result.data.isEmpty) {
        debugPrint('[barcode] av: "${result.data.name}"');
        prefill = avToItemPrefill(result.data, clean);
      } else {
        debugPrint('[barcode] internet: no data');
      }
      if(!mounted) return;
      if(prefill != null) {
        setState(() { _barcodeLoading = false; _barcodeStatus = ''; });
        await _showCreateItemDialog(prefill: prefill);
      } else {
        setState(() {
          _barcodeLoading = false;
          _barcodeStatus = l10n.acquisitionBarcodeNotFound;
        });
      }
    } catch(e) {
      if(!mounted) return;
      debugPrint('[barcode] ERROR: $e');
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

  /// True when the internet title search has at least one enabled service.
  ///
  /// Only valid during build (watches the internet services provider).
  bool get _titleSearchEnabled {
    final settings = ref.watch(internetServicesProvider).value;
    return settings != null && titleSearchAvailable(settings);
  }

  /// Opens the internet title search dialog pre-filled with [initialQuery]
  /// and, when a result is adopted, hands the prefill to the "Create new
  /// item" dialog.
  Future<void> _openTitleSearch(String initialQuery) async {
    final settings = await ref.read(internetServicesProvider.future);
    if(!mounted) return;
    final prefill = await showDialog<ItemPrefill>(
      context: context,
      builder: (_) => TitleSearchDialog(
        settings: settings,
        initialQuery: initialQuery,
      ),
    );
    if(prefill != null && mounted) {
      await _showCreateItemDialog(prefill: prefill);
    }
  }

  /// Opens the "Create new item" dialog and, if the user completes it,
  /// selects the newly created item and advances to step 1.
  ///
  /// Item creation is intentionally deferred to [_submit] so that no orphan
  /// item is written to the DB when the user abandons step 1.
  Future<void> _showCreateItemDialog({ItemPrefill? prefill}) async {
    final settings = await ref.read(internetServicesProvider.future);
    final itemsApi = ref.read(itemsApiProvider);
    if(!mounted) return;
    final result = await showDialog<SearchItemRow>(
      context: context,
      builder: (ctx) => _CreateItemDialog(
        types: _types,
        prefill: prefill,
        onCreate: (data) async {
          setState(() { _pendingNewItemData = data; });
          return 0;
        },
        onStoreThumbnailBytes: settings.downloadThumbnails
            ? (bytes, key, url) async =>
                itemsApi.storeThumbnail(bytes, key, url)
            : null,
        timeoutMs: settings.timeoutMs,
        onPickFile: (pngBytes) {
          final key =
              'file_${DateTime.now().millisecondsSinceEpoch}';
          return itemsApi.storeThumbnail(pngBytes, key, '');
        },
      ),
    );
    if(result != null && mounted) {
      setState(() {
        _selectedItem = result;
        _wizardStep = 1;
      });
    }
  }

  // ── Wizard navigation ─────────────────────────────────────────────────────

  /// Validates the current step and advances to the next one.
  void _onContinue() {
    final l10n = AppLocalizations.of(context)!;
    switch(_wizardStep) {
      case 0:
        if(_selectedItem == null) {
          _showSnack(l10n.acquisitionStep1Title);
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

  /// POSTs the acquisition to the server and transitions to the success card.
  ///
  /// If [_pendingNewItemData] is set, the item is created first so that no
  /// item exists in the DB without a corresponding acquisition record.
  Future<void> _submit() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() { _submitting = true; });
    try {
      final dt = DateTime(
        _acquisitionDate.year,
        _acquisitionDate.month,
        _acquisitionDate.day,
        _acquisitionTime.hour,
        _acquisitionTime.minute,
      );
      int itemId = _selectedItem!.id;
      if(_pendingNewItemData != null) {
        itemId =
            await ref.read(itemsApiProvider).create(_pendingNewItemData!);
        if(!mounted) return;
        _pendingNewItemData = null;
        _selectedItem = SearchItemRow(
          id: itemId,
          name: _selectedItem!.name,
          typeLocale: _selectedItem!.typeLocale,
          count: 0,
          available: 0,
          typeId: _selectedItem!.typeId,
        );
      }
      final data = <String, dynamic>{
        'item_id': itemId,
        'count': _count,
        'date_time': dt.toUtc().toIso8601String(),
      };
      final auth = ref.read(authProvider).value;
      if(auth is Authenticated && !auth.isAdmin) {
        data['data_collector_id'] = auth.currentUserId;
      }
      final id = await ref.read(acquisitionsApiProvider).create(data);
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
      _pendingNewItemData = null;
      _count = 1;
      _acquisitionDate = now;
      _acquisitionTime = TimeOfDay.fromDateTime(now);
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
      _acquisitionDate = now;
      _acquisitionTime = TimeOfDay.fromDateTime(now);
      _submitting = false;
      _successId = null;
    });
  }

  // ── Date/time pickers ─────────────────────────────────────────────────────

  Future<void> _pickDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _acquisitionDate,
      firstDate: DateTime(2000),
      lastDate: DateTime.now().add(const Duration(days: 1)),
    );
    if(picked != null && mounted) {
      setState(() { _acquisitionDate = picked; });
    }
  }

  Future<void> _pickTime() async {
    final picked = await showTimePicker(
      context: context,
      initialTime: _acquisitionTime,
    );
    if(picked != null && mounted) {
      setState(() { _acquisitionTime = picked; });
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
          icon: 'assets/icons/add_book.jpeg',
          title: l10n.acquisitionTitle,
        ),
      ),
      body: _successId != null
          ? _buildSuccessCard(context, l10n)
          : _buildStepper(context, l10n),
    );
  }

  // ── Success card ──────────────────────────────────────────────────────────

  /// Shown after an acquisition is successfully recorded.
  Widget _buildSuccessCard(BuildContext context, AppLocalizations l10n) {
    final auth = ref.read(authProvider).value;
    final canPrintLabels =
        auth is Authenticated && auth.hasPermission('view_items');
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Icon(Icons.check_circle, color: Colors.green, size: 64),
            const SizedBox(height: 16),
            Text(
              l10n.acquisitionSuccessId(_successId!),
              style: Theme.of(context).textTheme.headlineSmall,
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 32),
            ElevatedButton.icon(
              onPressed: _resetWizard,
              icon: const Icon(Icons.add),
              label: Text(l10n.acquisitionNewAcquisition),
            ),
            const SizedBox(height: 12),
            OutlinedButton.icon(
              onPressed: _resetToSameItem,
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.returnSameItem),
            ),
            if(canPrintLabels) ...[
              const SizedBox(height: 12),
              OutlinedButton.icon(
                onPressed: () => context.push(
                  '/label-print',
                  extra: LabelPrintInitialData(
                    itemId: _selectedItem!.id,
                    itemName: _selectedItem!.name,
                    copies: _count,
                  ),
                ),
                icon: const Icon(Icons.print_outlined),
                label: Text(l10n.labelPrintTitle),
              ),
            ],
          ],
        ),
      ),
    );
  }

  // ── Stepper wizard ────────────────────────────────────────────────────────

  /// Builds the two-step wizard for recording a new acquisition.
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
                  ? l10n.navAcquire
                  : l10n.actionConfirm),
            ),
          const SizedBox(width: 8),
          if(_wizardStep > 0 && !_submitting)
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
  /// Shows a barcode input field (triggers [_onBarcodeScanned] on submit) and
  /// a name search field. Searches all items regardless of availability.
  Step _buildItemStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.lendingStepItem),
      subtitle: Text(l10n.acquisitionStep1Title),
      isActive: _wizardStep >= 0,
      state: _selectedItem != null
          ? StepState.complete
          : StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          if(_selectedItem != null) ...[
            _buildInfoChip(Icons.book, _selectedItem!.name),
            const SizedBox(height: 8),
            TextButton.icon(
              onPressed: () {
                setState(() { _selectedItem = null; });
              },
              icon: const Icon(Icons.swap_horiz),
              label: Text(l10n.acquisitionStep1Title),
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
                                width: 16, height: 16,
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
                  onPressed: _barcodeLoading ? null : _openCameraScanner,
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
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      l10n.lendingItemNotFound,
                      style: TextStyle(
                        color: Theme.of(context).colorScheme.secondary,
                      ),
                    ),
                    if(_titleSearchEnabled) ...[
                      const SizedBox(height: 8),
                      FilledButton.tonalIcon(
                        onPressed: _typesLoading
                            ? null
                            : () => _openTitleSearch(
                                _itemSearchCtrl.text.trim()),
                        icon: const Icon(Icons.travel_explore),
                        label: Text(l10n.titleSearchOfferButton(
                            _itemSearchCtrl.text.trim())),
                      ),
                    ],
                  ],
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
              // ConstrainedBox gives the ListView a bounded maxHeight so it
              // can scroll independently within the Stepper column.
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
                        '${row.count}',
                        style: const TextStyle(fontSize: 12),
                      ),
                      onTap: () => _selectItem(row),
                    );
                  },
                ),
              ),
            const SizedBox(height: 4),
            Wrap(
              spacing: 8,
              children: [
                TextButton.icon(
                  onPressed: _typesLoading ? null : _showCreateItemDialog,
                  icon: const Icon(Icons.add),
                  label: Text(l10n.acquisitionCreateItemButton),
                ),
                TextButton.icon(
                  onPressed: _typesLoading || !_titleSearchEnabled
                      ? null
                      : () => _openTitleSearch(
                          _itemSearchCtrl.text.trim()),
                  icon: const Icon(Icons.travel_explore),
                  label: Text(l10n.titleSearchButton),
                ),
              ],
            ),
          ],
        ],
      ),
    );
  }

  // ── Step 1: Details ───────────────────────────────────────────────────────

  /// Builds the acquisition-details step.
  ///
  /// Count has no upper limit because we are adding new copies to stock.
  /// Date and time default to now; both can be changed via pickers.
  /// The data collector is optional.
  Step _buildDetailsStep(AppLocalizations l10n) {
    return Step(
      title: Text(l10n.lendingStepDetails),
      subtitle: Text(l10n.acquisitionStep2Title),
      isActive: _wizardStep >= 1,
      state: StepState.indexed,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
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
                onPressed: () { setState(() { _count++; }); },
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
                  date: _acquisitionDate,
                  onTap: _pickDate,
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: _TimeField(
                  time: _acquisitionTime,
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

  /// A small chip confirming a selection (item or person).
  Widget _buildInfoChip(IconData icon, String label) {
    return Chip(
      avatar: Icon(icon, size: 16),
      label: Text(label, overflow: TextOverflow.ellipsis),
    );
  }
}

// ─── _ItemPrefill ─────────────────────────────────────────────────────────────

// _ItemPrefill removed — replaced by the public ItemPrefill in models/item_prefill.dart

// ─── _CreateItemDialog ────────────────────────────────────────────────────────

/// Dialog for creating a new item during the acquisition workflow.
///
/// Shows the type dropdown, the required name field, all type-specific
/// attribute fields (derived from [ItemType.keys]), and the optional
/// barcode/place/description fields.  On success pops with a [SearchItemRow]
/// so the caller can continue the acquisition wizard.
class _CreateItemDialog extends StatefulWidget {
  const _CreateItemDialog({
    required this.types,
    required this.onCreate,
    this.prefill,
    this.onStoreThumbnailBytes,
    this.onPickFile,
    this.timeoutMs = 10000,
  });

  /// All available item types including their field definitions.
  final List<ItemType> types;

  /// Called with the POST body; must return the server-assigned item id.
  final Future<int> Function(Map<String, dynamic>) onCreate;

  /// Optional pre-fill data from an internet barcode search.
  final ItemPrefill? prefill;

  /// Called with already-downloaded PNG bytes, the cache key, and the source
  /// URL to upload the thumbnail to the server.
  ///
  /// Returns the server-assigned thumbnail id, or null when upload fails or
  /// the setting is disabled.
  final Future<int?> Function(Uint8List bytes, String key, String url)?
      onStoreThumbnailBytes;

  /// Called when the user picks a local image file.
  ///
  /// Must store the thumbnail and return the server-assigned id, or null on
  /// failure.
  final Future<int?> Function(Uint8List pngBytes)? onPickFile;

  /// Network timeout in milliseconds used when downloading the cover preview.
  final int timeoutMs;

  @override
  State<_CreateItemDialog> createState() => _CreateItemDialogState();
}

class _CreateItemDialogState extends State<_CreateItemDialog> {
  final _nameCtrl = TextEditingController();
  final _barcodeCtrl = TextEditingController();
  final _placeCtrl = TextEditingController();
  final _descCtrl = TextEditingController();

  /// Currently selected type (drives the dynamic field list).
  ItemType? _selectedType;

  // ── Type-specific field state ─────────────────────────────────────────────

  /// Controllers for non-list scalar fields (text, integer, float, etc.).
  Map<String, TextEditingController> _valueCtrl = {};

  /// Values for date fields (null = not set).
  Map<String, DateTime?> _dateValues = {};

  /// Controllers for list fields (one controller per list entry).
  Map<String, List<TextEditingController>> _listCtrl = {};

  bool _submitting = false;
  String? _error;

  /// Thumbnail id set by the user via file picker; overrides internet download.
  int? _manualThumbnailId;

  /// Cover image bytes downloaded from the internet lookup URL.
  Uint8List? _previewBytes;

  /// True while the cover image is being downloaded.
  bool _loadingPreview = false;

  @override
  void initState() {
    super.initState();
    if(widget.types.isNotEmpty) {
      _selectedType = _pickType(
        widget.types,
        widget.prefill?.preferredTypeHints ?? const [],
      );
      _initFieldControllers(_selectedType!);
    }
    final prefill = widget.prefill;
    if(prefill != null) {
      _nameCtrl.text = prefill.name;
      _barcodeCtrl.text = prefill.barcode;
      _descCtrl.text = prefill.description;
      _applyPrefillToTypeFields();
      if(prefill.coverUrl != null && widget.onStoreThumbnailBytes != null) {
        _loadingPreview = true;
        _downloadCoverPreview(prefill.coverUrl!);
      }
    }
  }

  Future<void> _downloadCoverPreview(String url) async {
    final bytes = await _fetchPngBytes(url, widget.timeoutMs);
    if(!mounted) return;
    setState(() {
      _previewBytes = bytes;
      _loadingPreview = false;
    });
  }

  @override
  void dispose() {
    _nameCtrl.dispose();
    _barcodeCtrl.dispose();
    _placeCtrl.dispose();
    _descCtrl.dispose();
    _disposeFieldControllers();
    super.dispose();
  }

  /// Returns the first type whose name (lowercased) contains any hint, or
  /// [types.first] when nothing matches.
  ItemType _pickType(List<ItemType> types, List<String> hints) {
    for(final hint in hints) {
      for(final type in types) {
        if(type.name.toLowerCase().contains(hint)) return type;
      }
    }
    return types.first;
  }

  void _disposeFieldControllers() {
    for (final ctrl in _valueCtrl.values) {
      ctrl.dispose();
    }
    for (final ctrls in _listCtrl.values) {
      for (final ctrl in ctrls) {
        ctrl.dispose();
      }
    }
  }

  /// Replaces all type-specific field controllers with ones for [type].
  ///
  /// Call [_applyPrefillToTypeFields] afterwards to restore any prefill data.
  void _initFieldControllers(ItemType type) {
    _disposeFieldControllers();
    _valueCtrl = {};
    _dateValues = {};
    _listCtrl = {};
    for (final key in type.keys) {
      if(key.isList) {
        _listCtrl[key.name] = [TextEditingController()];
      } else if(key.dataType == TypeKeyDataType.date) {
        _dateValues[key.name] = null;
      } else {
        _valueCtrl[key.name] = TextEditingController();
      }
    }
  }

  /// Copies [widget.prefill] data into the type-specific controllers.
  ///
  /// Safe to call whenever the type changes: keys not present in the current
  /// type's controllers are silently ignored.
  void _applyPrefillToTypeFields() {
    final prefill = widget.prefill;
    if(prefill == null) return;
    for(final e in prefill.typeSpecific.entries) {
      _valueCtrl[e.key]?.text = e.value;
    }
    for(final e in prefill.typeListSpecific.entries) {
      final ctrls = _listCtrl[e.key];
      if(ctrls == null || e.value.isEmpty) continue;
      for(final c in ctrls) { c.dispose(); }
      _listCtrl[e.key] = e.value
          .map((v) => TextEditingController(text: v))
          .toList();
    }
  }

  /// Switches to [newType], preserving any values the user has already entered.
  ///
  /// Saves the current scalar and list controller contents before rebuilding
  /// the field set, then restores matching keys in the new type. Prefill data
  /// fills only fields that are still empty after restoration.
  void _onTypeChanged(ItemType newType) {
    // Snapshot current user-entered values.
    final savedScalar = <String, String>{
      for(final e in _valueCtrl.entries) e.key: e.value.text,
    };
    final savedList = <String, List<String>>{
      for(final e in _listCtrl.entries)
        e.key: e.value.map((c) => c.text).toList(),
    };
    final savedDates = Map<String, DateTime?>.from(_dateValues);

    _initFieldControllers(newType);

    // Restore prefill first so user edits can override it below.
    _applyPrefillToTypeFields();

    // Overlay saved user input, overwriting prefill for non-empty entries.
    for(final e in savedScalar.entries) {
      if(e.value.isNotEmpty) {
        _valueCtrl[e.key]?.text = e.value;
      }
    }
    for(final e in savedList.entries) {
      final ctrls = _listCtrl[e.key];
      if(ctrls != null && e.value.any((v) => v.isNotEmpty)) {
        for(final c in ctrls) { c.dispose(); }
        _listCtrl[e.key] =
            e.value.map((v) => TextEditingController(text: v)).toList();
      }
    }
    for(final e in savedDates.entries) {
      if(_dateValues.containsKey(e.key)) {
        _dateValues[e.key] = e.value;
      }
    }

    _selectedType = newType;
  }

  Future<void> _onFilePicked(Uint8List pngBytes) async {
    final id = await widget.onPickFile!(pngBytes);
    if(!mounted || id == null) return;
    setState(() { _manualThumbnailId = id; });
  }

  /// Amazon search term: ISBN/EAN from the prefill when available, otherwise
  /// the item name as currently typed.
  String _amazonSearchTerm() {
    final term = widget.prefill?.amazonSearchTerm;
    if(term != null && term.isNotEmpty) return term;
    return _nameCtrl.text.trim();
  }

  /// Discards both the manually picked thumbnail and the downloaded cover
  /// preview so the item is created without a thumbnail.
  void _removeThumbnail() {
    setState(() {
      _manualThumbnailId = null;
      _previewBytes = null;
    });
  }

  // ── Submission ────────────────────────────────────────────────────────────

  Future<void> _submit() async {
    final l10n = AppLocalizations.of(context)!;
    if(_selectedType == null) return;
    final name = _nameCtrl.text.trim();
    if(name.isEmpty) {
      setState(() { _error = l10n.loginRequired; });
      return;
    }
    setState(() {
      _submitting = true;
      _error = null;
    });
    try {
      final data = <String, dynamic>{
        'type_id': _selectedType!.id,
        'name': name,
      };
      if(_barcodeCtrl.text.isNotEmpty) {
        data['barcode'] = _barcodeCtrl.text.trim();
      }
      if(_placeCtrl.text.isNotEmpty) {
        data['place'] = _placeCtrl.text.trim();
      }
      if(_descCtrl.text.isNotEmpty) {
        data['description'] = _descCtrl.text.trim();
      }
      // Collect type-specific single-value fields → "values" map
      final Map<String, String> values = {};
      // Collect type-specific list fields → "list_values" map
      final Map<String, List<String>> listValues = {};
      for (final key in _selectedType!.keys) {
        if(key.isList) {
          final vals = (_listCtrl[key.name] ?? [])
              .map((c) => c.text.trim())
              .where((s) => s.isNotEmpty)
              .toList();
          if(vals.isNotEmpty) listValues[key.name] = vals;
        } else if(key.dataType == TypeKeyDataType.date) {
          final dt = _dateValues[key.name];
          if(dt != null) values[key.name] = _fmtDate(dt);
        } else {
          final text = _valueCtrl[key.name]?.text.trim() ?? '';
          if(text.isNotEmpty) values[key.name] = text;
        }
      }
      if(values.isNotEmpty) data['values'] = values;
      if(listValues.isNotEmpty) data['list_values'] = listValues;

      if(_manualThumbnailId != null) {
        data['thumbnail_id'] = _manualThumbnailId;
      } else {
        final coverKey = widget.prefill?.coverKey;
        if(_previewBytes != null && coverKey != null &&
           widget.onStoreThumbnailBytes != null) {
          final tid = await widget.onStoreThumbnailBytes!(
              _previewBytes!, coverKey, widget.prefill?.coverUrl ?? '');
          if(tid != null) data['thumbnail_id'] = tid;
        }
      }

      final id = await widget.onCreate(data);
      if(!mounted) return;
      Navigator.of(context).pop(SearchItemRow(
        id: id,
        name: name,
        typeLocale: _selectedType!.name,
        count: 0,
        available: 0,
        typeId: _selectedType!.id,
      ));
    } catch(e) {
      if(!mounted) return;
      setState(() { _submitting = false; });
      await ServerErrorDialog.show(context, e, AppLocalizations.of(context)!);
    }
  }

  // ── Build ─────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return AlertDialog(
      title: Text(l10n.acquisitionCreateItemTitle),
      content: SingleChildScrollView(
        child: SizedBox(
          width: double.maxFinite,
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // ── Thumbnail ────────────────────────────────────────────────
              ThumbnailWidget(
                thumbnailId: _manualThumbnailId,
                previewBytes:
                    _manualThumbnailId == null ? _previewBytes : null,
                loading: _loadingPreview,
                amazonSearchTerm: _amazonSearchTerm,
                onPickFile: widget.onPickFile != null
                    ? _onFilePicked
                    : null,
                onRemove: _submitting ? null : _removeThumbnail,
              ),
              // ── Type (required) ──────────────────────────────────────────
              InputDecorator(
                decoration: InputDecoration(
                  labelText: l10n.labelType,
                  border: const OutlineInputBorder(),
                  isDense: true,
                  contentPadding: const EdgeInsets.fromLTRB(12, 8, 8, 8),
                ),
                child: DropdownButton<int>(
                  value: _selectedType?.id,
                  isExpanded: true,
                  underline: const SizedBox.shrink(),
                  isDense: true,
                  items: widget.types
                      .map((t) => DropdownMenuItem(
                            value: t.id,
                            child: Text(t.name),
                          ))
                      .toList(),
                  onChanged: _submitting
                      ? null
                      : (v) {
                          if(v == null) return;
                          final t = widget.types
                              .firstWhere((t) => t.id == v);
                          setState(() {
                            _onTypeChanged(t);
                          });
                        },
                ),
              ),
              const SizedBox(height: 12),
              // ── Name (required) ──────────────────────────────────────────
              TextField(
                controller: _nameCtrl,
                enabled: !_submitting,
                autofocus: true,
                decoration: InputDecoration(
                  labelText: l10n.labelName,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
                onSubmitted: (_) => _submit(),
              ),
              // ── Type-specific fields (dynamic) ───────────────────────────
              if(_selectedType != null)
                for (final key in _selectedType!.keys) ...[
                  const SizedBox(height: 12),
                  _buildKeyField(key),
                ],
              const SizedBox(height: 12),
              // ── Barcode (optional) ───────────────────────────────────────
              TextField(
                controller: _barcodeCtrl,
                enabled: !_submitting,
                decoration: InputDecoration(
                  labelText: l10n.labelBarcode,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
              ),
              const SizedBox(height: 12),
              // ── Place (optional) ─────────────────────────────────────────
              TextField(
                controller: _placeCtrl,
                enabled: !_submitting,
                decoration: InputDecoration(
                  labelText: l10n.labelPlace,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
              ),
              const SizedBox(height: 12),
              // ── Description (optional) ───────────────────────────────────
              TextField(
                controller: _descCtrl,
                enabled: !_submitting,
                maxLines: 3,
                decoration: InputDecoration(
                  labelText: l10n.labelDescription,
                  border: const OutlineInputBorder(),
                  isDense: true,
                ),
              ),
              if(_error != null) ...[
                const SizedBox(height: 8),
                Text(
                  _error!,
                  style: TextStyle(
                    color: Theme.of(context).colorScheme.error,
                  ),
                ),
              ],
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: _submitting
              ? null
              : () => Navigator.of(context).pop(),
          child: Text(
            MaterialLocalizations.of(context).cancelButtonLabel,
          ),
        ),
        ElevatedButton(
          onPressed: _submitting ? null : _submit,
          child: _submitting
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child: CircularProgressIndicator(strokeWidth: 2),
                )
              : Text(l10n.personSave),
        ),
      ],
    );
  }

  // ── Field builders ────────────────────────────────────────────────────────

  Widget _buildKeyField(TypeKey key) {
    if(key.isList) return _buildListKeyField(key);
    return switch(key.dataType) {
      TypeKeyDataType.date => _buildDateKeyField(key),
      _ => _buildTextKeyField(key),
    };
  }

  Widget _buildTextKeyField(TypeKey key) {
    final isNumeric = key.dataType == TypeKeyDataType.integer ||
        key.dataType == TypeKeyDataType.float;
    return TextField(
      controller: _valueCtrl[key.name],
      enabled: !_submitting,
      decoration: InputDecoration(
        labelText: key.displayName,
        border: const OutlineInputBorder(),
        isDense: true,
      ),
      keyboardType: isNumeric
          ? const TextInputType.numberWithOptions(decimal: true)
          : null,
      maxLines: key.dataType == TypeKeyDataType.string ? 3 : 1,
    );
  }

  Widget _buildDateKeyField(TypeKey key) {
    final dt = _dateValues[key.name];
    return InkWell(
      onTap: _submitting
          ? null
          : () async {
              final picked = await showDatePicker(
                context: context,
                initialDate: dt ?? DateTime.now(),
                firstDate: DateTime(1900),
                lastDate: DateTime(2100),
              );
              if(picked != null && mounted) {
                setState(() { _dateValues[key.name] = picked; });
              }
            },
      borderRadius: BorderRadius.circular(4),
      child: InputDecorator(
        decoration: InputDecoration(
          labelText: key.displayName,
          border: const OutlineInputBorder(),
          isDense: true,
          contentPadding: const EdgeInsets.fromLTRB(12, 8, 8, 8),
          suffixIcon: const Icon(Icons.calendar_today, size: 16),
        ),
        child: Text(
          dt != null ? _fmtDate(dt) : '—',
          style: dt == null
              ? TextStyle(color: Theme.of(context).hintColor)
              : null,
        ),
      ),
    );
  }

  /// Renders a repeatable list field with add/remove entry buttons.
  Widget _buildListKeyField(TypeKey key) {
    final ctrls = _listCtrl[key.name] ?? [];
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Row(
          children: [
            Expanded(
              child: Text(
                key.displayName,
                style: Theme.of(context).textTheme.bodySmall,
              ),
            ),
            IconButton(
              icon: const Icon(Icons.add_circle_outline, size: 20),
              padding: EdgeInsets.zero,
              constraints: const BoxConstraints(),
              visualDensity: VisualDensity.compact,
              onPressed: _submitting
                  ? null
                  : () => setState(() {
                        ctrls.add(TextEditingController());
                      }),
            ),
          ],
        ),
        for (int i = 0; i < ctrls.length; i++) ...[
          const SizedBox(height: 4),
          Row(
            children: [
              Expanded(
                child: TextField(
                  controller: ctrls[i],
                  enabled: !_submitting,
                  decoration: const InputDecoration(
                    border: OutlineInputBorder(),
                    isDense: true,
                  ),
                ),
              ),
              IconButton(
                icon: const Icon(Icons.remove_circle_outline, size: 20),
                padding: EdgeInsets.zero,
                constraints: const BoxConstraints(),
                visualDensity: VisualDensity.compact,
                onPressed: (ctrls.length > 1 && !_submitting)
                    ? () {
                        ctrls[i].dispose();
                        setState(() { ctrls.removeAt(i); });
                      }
                    : null,
              ),
            ],
          ),
        ],
      ],
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
