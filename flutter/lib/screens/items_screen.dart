// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../api/barcode_lookup.dart' show normalizeBarcode;
import '../api/ean_lookup.dart' show isProductEan, isUpcA;
import '../api/isbn_lookup.dart' show isIsbn10, isIsbn13, isbn10ToIsbn13;
import '../models/item.dart';
import '../providers/api_client_provider.dart';
import '../providers/table_watch.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import 'barcode_scan_screen.dart';

/// Search screen for library items.
///
/// Displays a general search field and an expandable advanced-search panel
/// with per-field filters (name, subject, description, keywords, type).
/// Results are fetched with a 300 ms debounce after any field change.
/// Tapping a result navigates to `/items/:id`.
class ItemsScreen extends ConsumerStatefulWidget {
  /// Creates an [ItemsScreen].
  const ItemsScreen({super.key});

  @override
  ConsumerState<ItemsScreen> createState() => _ItemsScreenState();
}

class _ItemsScreenState extends ConsumerState<ItemsScreen> {
  final _generalController = TextEditingController();
  final _nameController = TextEditingController();
  final _subjectController = TextEditingController();
  final _descriptionController = TextEditingController();
  final _keywordsController = TextEditingController();
  int _selectedTypeId = -1;
  List<({int id, String name})> _typeLocales = [];
  bool _typesLoaded = false;

  Timer? _debounce;
  List<SearchItemRow> _results = [];
  bool _loading = false;
  Object? _error;

  /// True once the user has triggered at least one search.
  bool _searched = false;

  @override
  void dispose() {
    _debounce?.cancel();
    _generalController.dispose();
    _nameController.dispose();
    _subjectController.dispose();
    _descriptionController.dispose();
    _keywordsController.dispose();
    super.dispose();
  }

  SearchQuery _buildQuery() => SearchQuery(
    general: _generalController.text.trim(),
    name: _nameController.text.trim(),
    subject: _subjectController.text.trim(),
    description: _descriptionController.text.trim(),
    keywords: _keywordsController.text.trim(),
    typeId: _selectedTypeId,
  );

  /// Resets the debounce timer on any field change.
  void _scheduleSearch() {
    _debounce?.cancel();
    final q = _buildQuery();
    if (q.isEmpty) {
      setState(() {
        _results = [];
        _error = null;
        _loading = false;
        _searched = false;
      });
      return;
    }
    _debounce = Timer(const Duration(milliseconds: 300), () => _search(q));
  }

  Future<void> _scanBarcode() async {
    final l10n = AppLocalizations.of(context)!;
    final raw = await Navigator.push<String>(
      context,
      MaterialPageRoute(builder: (_) => const BarcodeScanScreen()),
    );
    if (!mounted || raw == null) return;
    final clean = normalizeBarcode(raw);
    if (clean.isEmpty) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final api = ref.read(itemsApiProvider);
      final parsed = await api.parseItemBarcode(clean);
      if (!mounted) return;
      if (parsed != null) {
        setState(() {
          _loading = false;
        });
        context.push('/items/${parsed.itemId}');
        return;
      }
      final bool looksLikeIsbn =
          isIsbn10(clean) ||
          (isIsbn13(clean) &&
              (clean.startsWith('978') || clean.startsWith('979')));
      int? foundId;
      if (looksLikeIsbn) {
        final normalized = isIsbn10(clean) ? isbn10ToIsbn13(clean) : clean;
        final item = await api.getByIsbn(normalized);
        foundId = item?.id;
      } else if (isProductEan(clean)) {
        final item = await api.getByEan(clean);
        foundId = item?.id;
      } else if (isUpcA(clean)) {
        final item = await api.getByEan('0$clean');
        foundId = item?.id;
      }
      if (!mounted) return;
      setState(() {
        _loading = false;
      });
      if (foundId != null) {
        context.push('/items/$foundId');
      } else {
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(SnackBar(content: Text(l10n.itemsScanNotFound)));
      }
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _loading = false;
        _error = e;
      });
    }
  }

  Future<void> _search(SearchQuery q) async {
    setState(() {
      _loading = true;
      _error = null;
      _searched = true;
    });
    try {
      final results = await ref.read(itemsApiProvider).search(q);
      if (!mounted) return;
      setState(() {
        _results = results;
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

  Future<void> _loadTypeLocales() async {
    if (_typesLoaded) return;
    try {
      final locales = await ref.read(typesApiProvider).getLocale();
      if (!mounted) return;
      setState(() {
        _typeLocales = locales;
        _typesLoaded = true;
      });
    } catch (_) {
      // type locale loading is supplementary; ignore errors
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['items', 'acquisitions', 'liquidations'], () {
      if (_searched) {
        _search(_buildQuery());
      }
    });
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/search.jpeg',
          title: l10n.itemsTitle,
        ),
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 12, 12, 0),
            child: Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: _generalController,
                    autofocus: true,
                    decoration: InputDecoration(
                      hintText: l10n.itemsSearchHint,
                      prefixIcon: const Icon(Icons.search),
                      border: const OutlineInputBorder(),
                      suffixIcon: _generalController.text.isNotEmpty
                          ? IconButton(
                              icon: const Icon(Icons.clear),
                              onPressed: () {
                                _generalController.clear();
                                _scheduleSearch();
                              },
                            )
                          : null,
                    ),
                    onChanged: (_) => _scheduleSearch(),
                  ),
                ),
                const SizedBox(width: 8),
                Tooltip(
                  message: l10n.barcodeScanTooltip,
                  child: InkWell(
                    onTap: _scanBarcode,
                    borderRadius: BorderRadius.circular(8),
                    child: ClipRRect(
                      borderRadius: BorderRadius.circular(8),
                      child: Image.asset(
                        'assets/icons/scan_barcode.jpeg',
                        width: 48,
                        height: 48,
                        fit: BoxFit.cover,
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),
          ExpansionTile(
            title: Text(l10n.itemsAdvancedSearch),
            tilePadding: const EdgeInsets.symmetric(horizontal: 12),
            onExpansionChanged: (expanded) {
              if (expanded) _loadTypeLocales();
            },
            children: [
              Padding(
                padding: const EdgeInsets.fromLTRB(12, 0, 12, 12),
                child: Column(
                  children: [
                    Row(
                      children: [
                        Expanded(
                          child: _AdvancedField(
                            controller: _nameController,
                            label: l10n.labelName,
                            onChanged: (_) => _scheduleSearch(),
                          ),
                        ),
                        const SizedBox(width: 8),
                        Expanded(
                          child: _AdvancedField(
                            controller: _subjectController,
                            label: l10n.labelSubject,
                            onChanged: (_) => _scheduleSearch(),
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 8),
                    Row(
                      children: [
                        Expanded(
                          child: _AdvancedField(
                            controller: _descriptionController,
                            label: l10n.labelDescription,
                            onChanged: (_) => _scheduleSearch(),
                          ),
                        ),
                        const SizedBox(width: 8),
                        Expanded(
                          child: _AdvancedField(
                            controller: _keywordsController,
                            label: l10n.labelKeywords,
                            onChanged: (_) => _scheduleSearch(),
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 8),
                    InputDecorator(
                      decoration: InputDecoration(
                        labelText: l10n.labelType,
                        border: const OutlineInputBorder(),
                        isDense: true,
                        contentPadding: const EdgeInsets.fromLTRB(12, 8, 12, 8),
                      ),
                      child: DropdownButtonHideUnderline(
                        child: DropdownButton<int>(
                          value: _selectedTypeId,
                          isDense: true,
                          items: [
                            DropdownMenuItem(
                              value: -1,
                              child: Text(l10n.labelAllTypes),
                            ),
                            ..._typeLocales.map(
                              (t) => DropdownMenuItem(
                                value: t.id,
                                child: Text(t.name),
                              ),
                            ),
                          ],
                          onChanged: (val) {
                            setState(() {
                              _selectedTypeId = val ?? -1;
                            });
                            _scheduleSearch();
                          },
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
          Expanded(child: _buildBody(l10n)),
        ],
      ),
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if (_loading) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (!_searched) {
      return Center(child: Text(l10n.itemsEmptyPrompt));
    }
    if (_results.isEmpty) {
      return Center(child: Text(l10n.itemsNotFound));
    }
    return ListView.builder(
      itemCount: _results.length,
      itemBuilder: (context, index) {
        final row = _results[index];
        return _SearchResultTile(
          row: row,
          key: ValueKey(row.id),
          onDeleted: () => _search(_buildQuery()),
        );
      },
    );
  }
}

/// Search result tile that loads the full [Item] to display all populated fields.
class _SearchResultTile extends ConsumerStatefulWidget {
  const _SearchResultTile({required this.row, this.onDeleted, super.key});

  final SearchItemRow row;

  /// Called after the detail screen reports that this item was deleted, so
  /// the parent can refresh the result list.
  final VoidCallback? onDeleted;

  @override
  ConsumerState<_SearchResultTile> createState() => _SearchResultTileState();
}

class _SearchResultTileState extends ConsumerState<_SearchResultTile> {
  Item? _item;

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    try {
      final item = await ref.read(itemsApiProvider).getById(widget.row.id);
      if (mounted) {
        setState(() => _item = item);
      }
    } catch (_) {
      // keep showing compact data on error
    }
  }

  @override
  Widget build(BuildContext context) {
    final row = widget.row;
    final item = _item;
    return ListTile(
      title: Text(row.name),
      subtitle: _buildSubtitle(context, row, item),
      trailing: Text(
        '${row.available}/${row.count}',
        style: Theme.of(context).textTheme.bodySmall,
      ),
      isThreeLine: item != null && _hasExtraFields(row, item),
      onTap: () async {
        final deleted = await context.push<Object?>('/items/${row.id}');
        if (deleted == true) widget.onDeleted?.call();
      },
    );
  }

  bool _hasExtraFields(SearchItemRow row, Item item) {
    return item.values.isNotEmpty ||
        item.listValues.isNotEmpty ||
        item.subject != null ||
        item.place != null ||
        item.description != null;
  }

  Widget _buildSubtitle(BuildContext context, SearchItemRow row, Item? item) {
    if (item == null) {
      return Text(row.typeLocale);
    }
    final parts = <String>[row.typeLocale];
    for (final entry in item.listValues.entries) {
      if (entry.value.isNotEmpty) {
        parts.add(entry.value.join(', '));
      }
    }
    for (final entry in item.values.entries) {
      if (entry.value.isNotEmpty) {
        parts.add(entry.value);
      }
    }
    if (item.subject != null) parts.add(item.subject!);
    if (item.place != null) parts.add(item.place!);
    if (item.description != null) parts.add(item.description!);
    return Text(
      parts.join(' · '),
      maxLines: 2,
      overflow: TextOverflow.ellipsis,
    );
  }
}

/// A compact labeled text field for the advanced search panel.
class _AdvancedField extends StatelessWidget {
  const _AdvancedField({
    required this.controller,
    required this.label,
    required this.onChanged,
  });

  final TextEditingController controller;
  final String label;
  final void Function(String) onChanged;

  @override
  Widget build(BuildContext context) {
    return TextField(
      controller: controller,
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
        isDense: true,
      ),
      onChanged: onChanged,
    );
  }
}
