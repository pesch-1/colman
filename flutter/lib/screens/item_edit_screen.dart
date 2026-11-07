// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../models/item.dart';
import '../models/item_type.dart';
import '../providers/api_client_provider.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';
import '../widgets/thumbnail_widget.dart';

/// Full-page form for editing an existing library item.
///
/// Loads [Item] and [ItemType] on mount, pre-fills all editable fields,
/// and submits the changes via `PUT /api/items/:id`.
class ItemEditScreen extends ConsumerStatefulWidget {
  /// Creates an [ItemEditScreen] for item [itemId].
  const ItemEditScreen({super.key, required this.itemId});

  /// Primary key of the item to edit.
  final int itemId;

  @override
  ConsumerState<ItemEditScreen> createState() => _ItemEditScreenState();
}

class _ItemEditScreenState extends ConsumerState<ItemEditScreen> {
  Item? _item;
  ItemType? _selectedType;
  List<({int id, String name})> _allTypes = [];
  bool _loading = true;
  Object? _loadError;
  bool _submitting = false;
  String? _submitError;

  // ── Thumbnail ─────────────────────────────────────────────────────────────
  int? _thumbnailId;

  // ── Fixed-field controllers ───────────────────────────────────────────────
  final _nameCtrl = TextEditingController();
  final _barcodeCtrl = TextEditingController();
  final _placeCtrl = TextEditingController();
  final _descCtrl = TextEditingController();
  final _sigTopCtrl = TextEditingController();
  final _sigBottomCtrl = TextEditingController();
  final _keywordsCtrl = TextEditingController();

  // ── Type-specific field state ─────────────────────────────────────────────
  Map<String, TextEditingController> _valueCtrl = {};
  Map<String, DateTime?> _dateValues = {};
  Map<String, List<TextEditingController>> _listCtrl = {};

  @override
  void initState() {
    super.initState();
    _load();
  }

  @override
  void dispose() {
    _nameCtrl.dispose();
    _barcodeCtrl.dispose();
    _placeCtrl.dispose();
    _descCtrl.dispose();
    _sigTopCtrl.dispose();
    _sigBottomCtrl.dispose();
    _keywordsCtrl.dispose();
    _disposeTypeControllers();
    super.dispose();
  }

  void _disposeTypeControllers() {
    for(final c in _valueCtrl.values) {
      c.dispose();
    }
    for(final cs in _listCtrl.values) {
      for(final c in cs) {
        c.dispose();
      }
    }
  }

  Future<void> _load() async {
    try {
      final item = await ref.read(itemsApiProvider).getById(widget.itemId);
      final type = await ref.read(typesApiProvider).getById(item.typeId);
      final allTypes = await ref.read(typesApiProvider).getLocale();
      if(!mounted) return;
      _nameCtrl.text = item.name;
      _barcodeCtrl.text = item.barcode ?? '';
      _placeCtrl.text = item.place ?? '';
      _descCtrl.text = item.description ?? '';
      _sigTopCtrl.text = item.signatureTop ?? '';
      _sigBottomCtrl.text = item.signatureBottom ?? '';
      _keywordsCtrl.text = item.keywords.join(', ');
      _disposeTypeControllers();
      _valueCtrl = {};
      _dateValues = {};
      _listCtrl = {};
      for(final key in type.keys) {
        if(key.isList) {
          final vals = item.listValues[key.name] ?? [];
          _listCtrl[key.name] = vals.isEmpty
              ? [TextEditingController()]
              : vals.map((v) => TextEditingController(text: v)).toList();
        } else if(key.dataType == TypeKeyDataType.date) {
          final s = item.values[key.name];
          _dateValues[key.name] = s != null ? DateTime.tryParse(s) : null;
        } else {
          _valueCtrl[key.name] =
              TextEditingController(text: item.values[key.name] ?? '');
        }
      }
      setState(() {
        _item = item;
        _selectedType = type;
        _allTypes = allTypes;
        _thumbnailId = item.thumbnailId;
        _loading = false;
      });
    } catch(e) {
      if(!mounted) return;
      setState(() {
        _loadError = e;
        _loading = false;
      });
    }
  }

  Future<void> _onTypeChanged(int newTypeId) async {
    final savedScalar = <String, String>{};
    final savedDates = <String, DateTime?>{};
    final savedLists = <String, List<String>>{};
    final current = _selectedType;
    if(current != null) {
      for(final key in current.keys) {
        if(key.isList) {
          savedLists[key.name] = (_listCtrl[key.name] ?? [])
              .map((c) => c.text.trim())
              .where((s) => s.isNotEmpty)
              .toList();
        } else if(key.dataType == TypeKeyDataType.date) {
          savedDates[key.name] = _dateValues[key.name];
        } else {
          savedScalar[key.name] = _valueCtrl[key.name]?.text.trim() ?? '';
        }
      }
    }
    _disposeTypeControllers();
    try {
      final newType = await ref.read(typesApiProvider).getById(newTypeId);
      if(!mounted) return;
      _valueCtrl = {};
      _dateValues = {};
      _listCtrl = {};
      for(final key in newType.keys) {
        if(key.isList) {
          final vals = savedLists[key.name] ?? [];
          _listCtrl[key.name] = vals.isEmpty
              ? [TextEditingController()]
              : vals.map((v) => TextEditingController(text: v)).toList();
        } else if(key.dataType == TypeKeyDataType.date) {
          _dateValues[key.name] = savedDates[key.name];
        } else {
          _valueCtrl[key.name] = TextEditingController(
            text: savedScalar[key.name] ?? '',
          );
        }
      }
      setState(() { _selectedType = newType; });
    } catch(e) {
      if(!mounted) return;
      await ServerErrorDialog.show(context, e, AppLocalizations.of(context)!);
    }
  }

  Future<void> _save() async {
    final item = _item!;
    final type = _selectedType!;
    final name = _nameCtrl.text.trim();
    if(name.isEmpty) return;
    setState(() {
      _submitting = true;
      _submitError = null;
    });
    try {
      final values = <String, String>{};
      final listValues = <String, List<String>>{};
      for(final key in type.keys) {
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
      final keywords = _keywordsCtrl.text
          .split(',')
          .map((s) => s.trim())
          .where((s) => s.isNotEmpty)
          .toList();
      final barcode = _barcodeCtrl.text.trim();
      final place = _placeCtrl.text.trim();
      final desc = _descCtrl.text.trim();
      final sigTop = _sigTopCtrl.text.trim();
      final sigBottom = _sigBottomCtrl.text.trim();
      final data = <String, dynamic>{
        'type_id': type.id,
        'name': name,
        if(barcode.isNotEmpty) 'barcode': barcode,
        if(place.isNotEmpty) 'place': place,
        if(desc.isNotEmpty) 'description': desc,
        if(sigTop.isNotEmpty) 'signature_top': sigTop,
        if(sigBottom.isNotEmpty) 'signature_bottom': sigBottom,
        'keywords': keywords,
        'values': values,
        'list_values': listValues,
        'is_school_item': item.isSchoolItem,
        if(item.subject != null) 'subject': item.subject,
        'academic_year': item.academicYear,
        'allow_reservations': item.allowReservations ? 1 : 0,
        if(item.reservationPeriodDays != null)
          'reservation_period_days': item.reservationPeriodDays,
        if(_thumbnailId != null) 'thumbnail_id': _thumbnailId,
      };
      final etag = item.etag;
      if(etag == null) {
        setState(() {
          _submitting = false;
          _submitError = 'Version information missing — reload and try again.';
        });
        return;
      }
      await ref.read(itemsApiProvider).update(item.id, data, etag);
      if(!mounted) return;
      context.pop(true);
    } catch(e) {
      if(!mounted) return;
      setState(() { _submitting = false; });
      await ServerErrorDialog.show(context, e, AppLocalizations.of(context)!);
    }
  }

  Future<void> _onFilePicked(Uint8List pngBytes) async {
    final key = 'file_${DateTime.now().millisecondsSinceEpoch}';
    final id = await ref
        .read(itemsApiProvider)
        .storeThumbnail(pngBytes, key, '');
    if(!mounted || id == null) return;
    setState(() { _thumbnailId = id; });
  }

  /// Amazon search term: ISBN/EAN when the item has one, otherwise the item
  /// name as currently typed.
  String _amazonSearchTerm() {
    final item = _item;
    if(item != null) {
      final isbn = item.listValues['ISBN13']?.firstOrNull;
      if(isbn != null && isbn.isNotEmpty) return isbn;
      final ean = item.values['ean'];
      if(ean != null && ean.isNotEmpty) return ean;
    }
    return _nameCtrl.text.trim();
  }

  // ── Build ─────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/search.jpeg',
          title: l10n.actionEditItem,
        ),
        actions: [
          if(!_loading && _loadError == null)
            IconButton(
              icon: _submitting
                  ? const SizedBox(
                      width: 20,
                      height: 20,
                      child: CircularProgressIndicator(strokeWidth: 2),
                    )
                  : const Icon(Icons.save),
              tooltip: l10n.personSave,
              onPressed: _submitting ? null : _save,
            ),
        ],
      ),
      body: _buildBody(l10n),
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if(_loading) {
      return const Center(child: CircularProgressIndicator());
    }
    if(_loadError != null) {
      return Center(child: ErrorDisplay(error: _loadError!, l10n: l10n));
    }
    final type = _selectedType!;
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        ThumbnailWidget(
          thumbnailId: _thumbnailId,
          amazonSearchTerm: _amazonSearchTerm,
          onPickFile: _submitting ? null : _onFilePicked,
          onRemove: _submitting
              ? null
              : () => setState(() { _thumbnailId = null; }),
        ),
        TextField(
          controller: _nameCtrl,
          enabled: !_submitting,
          autofocus: true,
          decoration: InputDecoration(
            labelText: l10n.labelName,
            border: const OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 12),
        DropdownButtonFormField<int>(
          initialValue: type.id,
          decoration: InputDecoration(
            labelText: l10n.labelType,
            border: const OutlineInputBorder(),
          ),
          items: _allTypes
              .map((t) => DropdownMenuItem(value: t.id, child: Text(t.name)))
              .toList(),
          onChanged: _submitting
              ? null
              : (id) {
                  if(id != null && id != _selectedType?.id) {
                    _onTypeChanged(id);
                  }
                },
        ),
        for(final key in type.keys) ...[
          const SizedBox(height: 12),
          _buildKeyField(key),
        ],
        const SizedBox(height: 12),
        TextField(
          controller: _barcodeCtrl,
          enabled: !_submitting,
          decoration: InputDecoration(
            labelText: l10n.labelBarcode,
            border: const OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 12),
        TextField(
          controller: _placeCtrl,
          enabled: !_submitting,
          decoration: InputDecoration(
            labelText: l10n.labelPlace,
            border: const OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 12),
        TextField(
          controller: _sigTopCtrl,
          enabled: !_submitting,
          decoration: InputDecoration(
            labelText: l10n.labelSignatureTop,
            border: const OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 12),
        TextField(
          controller: _sigBottomCtrl,
          enabled: !_submitting,
          decoration: InputDecoration(
            labelText: l10n.labelSignatureBottom,
            border: const OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 12),
        TextField(
          controller: _keywordsCtrl,
          enabled: !_submitting,
          decoration: InputDecoration(
            labelText: l10n.labelKeywords,
            hintText: l10n.itemEditKeywordsHint,
            border: const OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 12),
        TextField(
          controller: _descCtrl,
          enabled: !_submitting,
          maxLines: 4,
          decoration: InputDecoration(
            labelText: l10n.labelDescription,
            border: const OutlineInputBorder(),
          ),
        ),
        if(_submitError != null) ...[
          const SizedBox(height: 12),
          Text(
            l10n.errorPrefix(_submitError!),
            style: TextStyle(color: Theme.of(context).colorScheme.error),
          ),
        ],
        const SizedBox(height: 80),
      ],
    );
  }

  // ── Key field builders ────────────────────────────────────────────────────

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
        for(int i = 0; i < ctrls.length; i++) ...[
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

  String _fmtDate(DateTime dt) =>
      '${dt.year.toString().padLeft(4, '0')}-'
      '${dt.month.toString().padLeft(2, '0')}-'
      '${dt.day.toString().padLeft(2, '0')}';
}
