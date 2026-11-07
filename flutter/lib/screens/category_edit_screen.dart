// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../models/item_type.dart';
import '../providers/api_client_provider.dart';
import '../utils/column_name.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';

/// Create or edit an item type ("category"): display name, search category
/// (media kind) and the list of custom fields (name, data type, repeatable).
///
/// Min/max constraints, MARC21 tags and nested tables of existing fields are
/// not editable here but are preserved across save (see [TypeKey.info]).
class CategoryEditScreen extends ConsumerStatefulWidget {
  const CategoryEditScreen({super.key, this.typeId});

  /// Database id of the type to edit, or null to create a new one.
  final int? typeId;

  @override
  ConsumerState<CategoryEditScreen> createState() =>
      _CategoryEditScreenState();
}

class _CategoryEditScreenState extends ConsumerState<CategoryEditScreen> {
  final TextEditingController _nameCtrl = TextEditingController();
  MediaKind _mediaKind = MediaKind.unknown;
  List<TypeKey> _fields = [];
  String? _etag;

  bool _loading = false;
  bool _busy = false;
  Object? _error;

  bool get _isNew => widget.typeId == null;

  @override
  void initState() {
    super.initState();
    if (!_isNew) {
      _load();
    }
  }

  @override
  void dispose() {
    _nameCtrl.dispose();
    super.dispose();
  }

  Future<void> _load() async {
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final type = await ref.read(typesApiProvider).getById(widget.typeId!);
      if (!mounted) return;
      setState(() {
        _nameCtrl.text = type.name;
        _mediaKind = type.mediaKind;
        _fields = List<TypeKey>.from(type.keys);
        _etag = type.etag;
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

  Future<void> _editField(AppLocalizations l10n, {int? index}) async {
    final existing = index != null ? _fields[index] : null;
    final result = await showDialog<TypeKey>(
      context: context,
      builder: (_) => _FieldDialog(l10n: l10n, field: existing),
    );
    if (result == null || !mounted) return;
    setState(() {
      if (index != null) {
        _fields[index] = result;
      } else {
        _fields.add(result);
      }
    });
  }

  Future<void> _save(AppLocalizations l10n) async {
    final name = _nameCtrl.text.trim();
    if (name.isEmpty) return;
    setState(() {
      _busy = true;
      _error = null;
    });
    final type = ItemType(
      id: widget.typeId ?? 0,
      name: name,
      keys: _fields,
      mediaKind: _mediaKind,
      etag: _etag,
    );
    try {
      final api = ref.read(typesApiProvider);
      if (_isNew) {
        final tablename = '_${stringToColumnName(name)}';
        final newId = await api.create(type.toCreateJson(tablename));
        if (newId <= 0) {
          if (!mounted) return;
          setState(() => _busy = false);
          await showDialog<void>(
            context: context,
            builder: (ctx) => AlertDialog(
              title: Text(l10n.categoriesAddTitle),
              content: Text(l10n.categoriesSaveFailed),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(ctx),
                  child:
                      Text(MaterialLocalizations.of(ctx).okButtonLabel),
                ),
              ],
            ),
          );
          return;
        }
      } else {
        await api.update(widget.typeId!, type.toUpdateJson(), _etag ?? '');
      }
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(l10n.categoriesSaveSuccess)),
      );
      Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      setState(() => _busy = false);
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final canSave = !_busy && _nameCtrl.text.trim().isNotEmpty;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/edit_types.jpeg',
          title: _isNew ? l10n.categoriesAddTitle : l10n.categoriesEditTitle,
        ),
        actions: [
          IconButton(
            icon: _busy
                ? const SizedBox(
                    width: 20,
                    height: 20,
                    child: CircularProgressIndicator(strokeWidth: 2),
                  )
                : const Icon(Icons.save),
            tooltip: l10n.settingsSave,
            onPressed: canSave ? () => _save(l10n) : null,
          ),
        ],
      ),
      body: _buildBody(l10n),
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if (_loading) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null && _fields.isEmpty && _nameCtrl.text.isEmpty) {
      return Center(child: ErrorDisplay(error: _error!, l10n: l10n));
    }
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        TextField(
          controller: _nameCtrl,
          enabled: !_busy,
          textCapitalization: TextCapitalization.sentences,
          decoration: InputDecoration(
            labelText: l10n.categoriesNameLabel,
            border: const OutlineInputBorder(),
          ),
          onChanged: (_) => setState(() {}),
        ),
        const SizedBox(height: 16),
        DropdownButtonFormField<MediaKind>(
          initialValue: _mediaKind,
          decoration: InputDecoration(
            labelText: l10n.categoriesSearchCategory,
            border: const OutlineInputBorder(),
          ),
          items: MediaKind.values
              .map((m) => DropdownMenuItem(
                    value: m,
                    child: Text(mediaKindLabel(m, l10n)),
                  ))
              .toList(),
          onChanged: _busy
              ? null
              : (m) => setState(() => _mediaKind = m ?? MediaKind.unknown),
        ),
        const SizedBox(height: 24),
        Row(
          children: [
            Text(
              l10n.categoriesFields,
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const Spacer(),
            TextButton.icon(
              onPressed: _busy ? null : () => _editField(l10n),
              icon: const Icon(Icons.add),
              label: Text(l10n.categoriesAddField),
            ),
          ],
        ),
        if (_fields.isEmpty)
          Padding(
            padding: const EdgeInsets.symmetric(vertical: 16),
            child: Text(
              l10n.categoriesNoFields,
              style: Theme.of(context).textTheme.bodySmall,
            ),
          )
        else
          ReorderableListView(
            shrinkWrap: true,
            physics: const NeverScrollableScrollPhysics(),
            buildDefaultDragHandles: false,
            onReorderItem: (oldIndex, newIndex) {
              setState(() {
                final item = _fields.removeAt(oldIndex);
                _fields.insert(newIndex, item);
              });
            },
            children: [
              for (var i = 0; i < _fields.length; i++)
                _fieldTile(l10n, i, _fields[i]),
            ],
          ),
      ],
    );
  }

  Widget _fieldTile(AppLocalizations l10n, int index, TypeKey field) {
    final subtitle = field.isList
        ? '${dataTypeLabel(field.dataType, l10n)} · '
            '${l10n.categoriesFieldRepeatable}'
        : dataTypeLabel(field.dataType, l10n);
    return ListTile(
      key: ValueKey('field_${field.name}_$index'),
      title: Text(field.displayName),
      subtitle: Text(subtitle),
      onTap: _busy ? null : () => _editField(l10n, index: index),
      trailing: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          IconButton(
            icon: const Icon(Icons.delete_outline),
            tooltip: l10n.categoriesFieldDelete,
            onPressed: _busy
                ? null
                : () => setState(() => _fields.removeAt(index)),
          ),
          ReorderableDragStartListener(
            index: index,
            child: const Icon(Icons.drag_handle),
          ),
        ],
      ),
    );
  }
}

// ─── Field dialog ──────────────────────────────────────────────────────────

class _FieldDialog extends StatefulWidget {
  const _FieldDialog({required this.l10n, this.field});

  final AppLocalizations l10n;

  /// Existing field to edit, or null when adding a new one.
  final TypeKey? field;

  @override
  State<_FieldDialog> createState() => _FieldDialogState();
}

class _FieldDialogState extends State<_FieldDialog> {
  late final TextEditingController _nameCtrl;
  late TypeKeyDataType _dataType;
  late bool _repeatable;

  bool get _isNew => widget.field == null;

  @override
  void initState() {
    super.initState();
    _nameCtrl = TextEditingController(text: widget.field?.displayName ?? '');
    _nameCtrl.addListener(() => setState(() {}));
    _dataType = widget.field?.dataType ?? TypeKeyDataType.shortString;
    _repeatable = widget.field?.isList ?? false;
  }

  @override
  void dispose() {
    _nameCtrl.dispose();
    super.dispose();
  }

  bool get _valid => _nameCtrl.text.trim().isNotEmpty;

  void _submit() {
    final name = _nameCtrl.text.trim();
    final TypeKey result;
    if (_isNew) {
      result = TypeKey(
        name: stringToColumnName(name),
        displayName: name,
        dataType: _dataType,
        isList: _repeatable,
      );
    } else {
      // Keep the immutable internal column name; only the label may change.
      result = widget.field!.copyWith(
        displayName: name,
        dataType: _dataType,
        isList: _repeatable,
      );
    }
    Navigator.pop(context, result);
  }

  @override
  Widget build(BuildContext context) {
    final l10n = widget.l10n;
    return AlertDialog(
      title: Text(l10n.categoriesEditFieldTitle),
      content: SingleChildScrollView(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            TextField(
              controller: _nameCtrl,
              autofocus: true,
              decoration: InputDecoration(
                labelText: l10n.categoriesFieldName,
                border: const OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 16),
            DropdownButtonFormField<TypeKeyDataType>(
              initialValue: _dataType,
              decoration: InputDecoration(
                labelText: l10n.categoriesFieldType,
                border: const OutlineInputBorder(),
              ),
              items: TypeKeyDataType.values
                  .map((t) => DropdownMenuItem(
                        value: t,
                        child: Text(dataTypeLabel(t, l10n)),
                      ))
                  .toList(),
              onChanged: (t) => setState(
                  () => _dataType = t ?? TypeKeyDataType.shortString),
            ),
            const SizedBox(height: 8),
            SwitchListTile(
              contentPadding: EdgeInsets.zero,
              title: Text(l10n.categoriesFieldRepeatable),
              value: _repeatable,
              onChanged: (v) => setState(() => _repeatable = v),
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
          onPressed: _valid ? _submit : null,
          child: Text(MaterialLocalizations.of(context).okButtonLabel),
        ),
      ],
    );
  }
}

// ─── Label helpers ─────────────────────────────────────────────────────────

String mediaKindLabel(MediaKind kind, AppLocalizations l10n) {
  switch (kind) {
    case MediaKind.unknown:
      return l10n.mediaKindUnknown;
    case MediaKind.book:
      return l10n.mediaKindBook;
    case MediaKind.cd:
      return l10n.mediaKindCd;
    case MediaKind.dvd:
      return l10n.mediaKindDvd;
    case MediaKind.bluRay:
      return l10n.mediaKindBluRay;
    case MediaKind.boardGame:
      return l10n.mediaKindBoardGame;
  }
}

String dataTypeLabel(TypeKeyDataType type, AppLocalizations l10n) {
  switch (type) {
    case TypeKeyDataType.string:
      return l10n.dataTypeString;
    case TypeKeyDataType.shortString:
      return l10n.dataTypeShortString;
    case TypeKeyDataType.date:
      return l10n.dataTypeDate;
    case TypeKeyDataType.time:
      return l10n.dataTypeTime;
    case TypeKeyDataType.integer:
      return l10n.dataTypeInteger;
    case TypeKeyDataType.float:
      return l10n.dataTypeFloat;
    case TypeKeyDataType.duration:
      return l10n.dataTypeDuration;
    case TypeKeyDataType.table:
      return l10n.dataTypeTable;
    case TypeKeyDataType.integerRanges:
      return l10n.dataTypeIntegerRanges;
  }
}
