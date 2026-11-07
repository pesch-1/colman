// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../api/history_api.dart';
import '../api/persons_api.dart';
import '../models/history_row.dart';
import '../models/person.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/person_search_field.dart';
import '../widgets/server_error_dialog.dart';

const int _kPageSize = 100;

/// History screen showing all library actions with date-range filtering,
/// pagination, detail view, and admin-only delete / edit with conflict
/// resolution.
class HistoryScreen extends ConsumerStatefulWidget {
  const HistoryScreen({super.key});

  @override
  ConsumerState<HistoryScreen> createState() => _HistoryScreenState();
}

class _HistoryScreenState extends ConsumerState<HistoryScreen> {
  late DateTime _fromDate;
  late DateTime _toDate;
  bool _showAll = false;

  /// Action type to restrict the history to; empty = all types.
  String _actionFilter = '';
  List<HistoryRow> _rows = [];
  bool _hasMore = false;
  int _offset = 0;
  bool _loading = false;
  Object? _error;
  late final bool _isAdmin;

  @override
  void initState() {
    super.initState();
    final auth = ref.read(authProvider).value;
    _isAdmin = auth is Authenticated && auth.currentUserId == 0;
    final now = DateTime.now();
    _toDate = DateTime(now.year, now.month, now.day);
    _fromDate = DateTime(now.year, now.month - 3, now.day);
    _load();
  }

  Future<void> _load() async {
    if (!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
      _offset = 0;
    });
    try {
      final rows = await ref
          .read(historyApiProvider)
          .getHistory(
            isAdmin: _isAdmin,
            from: _fromDate,
            to: _toDate,
            showAll: _showAll,
            action: _actionFilter,
            limit: _kPageSize + 1,
            offset: 0,
          );
      if (!mounted) return;
      final hasMore = rows.length > _kPageSize;
      setState(() {
        _rows = hasMore ? rows.sublist(0, _kPageSize) : rows;
        _hasMore = hasMore;
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

  Future<void> _loadMore() async {
    if (_loading) return;
    setState(() {
      _loading = true;
    });
    try {
      _offset += _kPageSize;
      final rows = await ref
          .read(historyApiProvider)
          .getHistory(
            isAdmin: _isAdmin,
            from: _fromDate,
            to: _toDate,
            showAll: _showAll,
            action: _actionFilter,
            limit: _kPageSize + 1,
            offset: _offset,
          );
      if (!mounted) return;
      final hasMore = rows.length > _kPageSize;
      setState(() {
        _rows = [..._rows, ...(hasMore ? rows.sublist(0, _kPageSize) : rows)];
        _hasMore = hasMore;
        _loading = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _loading = false;
      });
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
    }
  }

  Future<void> _pickFromDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _fromDate,
      firstDate: DateTime(2000),
      lastDate: _toDate,
    );
    if (picked != null) {
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
      lastDate: DateTime.now().add(const Duration(days: 1)),
    );
    if (picked != null) {
      setState(() {
        _toDate = picked;
      });
    }
  }

  Future<void> _showDetail(HistoryRow row) async {
    final historyApi = ref.read(historyApiProvider);
    final personsApi = ref.read(personsApiProvider);
    final l10n = AppLocalizations.of(context)!;
    final changed = await showDialog<bool>(
      context: context,
      builder: (_) => _HistoryDetailDialog(
        row: row,
        isAdmin: _isAdmin,
        historyApi: historyApi,
        personsApi: personsApi,
        l10n: l10n,
      ),
    );
    if (!mounted) return;
    if (changed == true) {
      _load();
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['items', 'persons', 'lendings', 'returns'], _load);
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/list_history.jpeg',
          title: l10n.historyTitle,
        ),
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 12, 12, 0),
            child: Row(
              children: [
                Expanded(
                  child: _DateField(
                    label: l10n.historyFromDate,
                    date: _fromDate,
                    enabled: !_showAll,
                    onTap: _showAll ? null : _pickFromDate,
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: _DateField(
                    label: l10n.historyToDate,
                    date: _toDate,
                    enabled: !_showAll,
                    onTap: _showAll ? null : _pickToDate,
                  ),
                ),
                const SizedBox(width: 8),
                TextButton(
                  onPressed: _showAll ? null : _load,
                  child: Text(l10n.historyApply),
                ),
              ],
            ),
          ),
          Row(
            children: [
              Expanded(
                child: CheckboxListTile(
                  title: Text(l10n.historyShowAll),
                  value: _showAll,
                  controlAffinity: ListTileControlAffinity.leading,
                  dense: true,
                  onChanged: (v) {
                    setState(() {
                      _showAll = v ?? false;
                    });
                    _load();
                  },
                ),
              ),
              Padding(
                padding: const EdgeInsets.only(right: 12),
                child: DropdownButton<String>(
                  value: _actionFilter,
                  onChanged: (v) {
                    setState(() {
                      _actionFilter = v ?? '';
                    });
                    _load();
                  },
                  items: [
                    DropdownMenuItem(
                      value: '',
                      child: Text(l10n.historyFilterAllActions),
                    ),
                    for (final action in _kFilterActions)
                      DropdownMenuItem(
                        value: action,
                        child: Text(_actionLabel(action, l10n)),
                      ),
                  ],
                ),
              ),
            ],
          ),
          if (_rows.isNotEmpty)
            Padding(
              padding: const EdgeInsets.fromLTRB(16, 0, 16, 4),
              child: Align(
                alignment: Alignment.centerLeft,
                child: Text(
                  l10n.historyActionsShown(_rows.length),
                  style: Theme.of(context).textTheme.bodySmall,
                ),
              ),
            ),
          Expanded(child: _buildBody(l10n)),
        ],
      ),
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
    if (!_loading && _rows.isEmpty) {
      return Center(child: Text(l10n.historyNoResults));
    }
    final itemCount = _rows.length + (_hasMore || _loading ? 1 : 0);
    return ListView.separated(
      itemCount: itemCount,
      separatorBuilder: (_, __) => const Divider(height: 1),
      itemBuilder: (context, index) {
        if (index == _rows.length) {
          if (_loading) {
            return const Padding(
              padding: EdgeInsets.all(16),
              child: Center(child: CircularProgressIndicator()),
            );
          }
          return Padding(
            padding: const EdgeInsets.all(12),
            child: Center(
              child: TextButton(
                onPressed: _loadMore,
                child: Text(l10n.historyLoadMore),
              ),
            ),
          );
        }
        final row = _rows[index];
        return _HistoryTile(
          row: row,
          isAdmin: _isAdmin,
          onTap: () => _showDetail(row),
        );
      },
    );
  }
}

// ─── Tile ──────────────────────────────────────────────────────────────────

class _HistoryTile extends StatelessWidget {
  const _HistoryTile({
    required this.row,
    required this.isAdmin,
    required this.onTap,
  });

  final HistoryRow row;
  final bool isAdmin;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final (color, icon) = _actionVisual(row.action);
    final subtitle = isAdmin && row.collector.isNotEmpty
        ? '${row.userName} · ${_fmtDateTime(row.datetime)} · ${row.collector}'
        : '${row.userName} · ${_fmtDateTime(row.datetime)}';
    return ListTile(
      leading: CircleAvatar(
        backgroundColor: color.withAlpha(40),
        child: Icon(icon, color: color, size: 20),
      ),
      title: Text(row.itemName),
      subtitle: Text(subtitle, style: Theme.of(context).textTheme.bodySmall),
      trailing: Text(
        '${row.count}',
        style: Theme.of(context).textTheme.bodyMedium,
      ),
      onTap: onTap,
    );
  }
}

// ─── Detail dialog ─────────────────────────────────────────────────────────

class _HistoryDetailDialog extends StatefulWidget {
  const _HistoryDetailDialog({
    required this.row,
    required this.isAdmin,
    required this.historyApi,
    required this.personsApi,
    required this.l10n,
  });

  final HistoryRow row;
  final bool isAdmin;
  final HistoryApi historyApi;
  final PersonsApi personsApi;
  final AppLocalizations l10n;

  @override
  State<_HistoryDetailDialog> createState() => _HistoryDetailDialogState();
}

class _HistoryDetailDialogState extends State<_HistoryDetailDialog> {
  bool _busy = false;

  Future<void> _confirmDelete() async {
    setState(() {
      _busy = true;
    });
    final List<HistoryRow> conflicts;
    try {
      conflicts = await widget.historyApi.getConflicts(widget.row);
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
      });
      await ServerErrorDialog.show(context, e, widget.l10n);
      return;
    }
    if (!mounted) return;
    setState(() {
      _busy = false;
    });

    final toDelete = [widget.row, ...conflicts];

    final bool? confirmed;
    if (conflicts.isEmpty) {
      confirmed = await showDialog<bool>(
        context: context,
        builder: (ctx) => AlertDialog(
          title: Text(widget.l10n.historyDeleteTitle),
          content: Text(widget.l10n.historyDeleteConfirm),
          actions: [
            TextButton(
              onPressed: () => Navigator.pop(ctx),
              child: Text(MaterialLocalizations.of(ctx).cancelButtonLabel),
            ),
            TextButton(
              onPressed: () => Navigator.pop(ctx, true),
              child: Text(widget.l10n.historyDeleteAll),
            ),
          ],
        ),
      );
    } else {
      confirmed = await showDialog<bool>(
        context: context,
        builder: (ctx) =>
            _ConflictDeleteDialog(toDelete: toDelete, l10n: widget.l10n),
      );
    }

    if (!mounted || confirmed != true) return;

    try {
      await widget.historyApi.deleteActions(toDelete);
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(widget.l10n.historyDeleteSuccess)));
      Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      await ServerErrorDialog.show(context, e, widget.l10n);
    }
  }

  Future<void> _showEdit() async {
    final changed = await showDialog<bool>(
      context: context,
      builder: (_) => _EditDialog(
        row: widget.row,
        historyApi: widget.historyApi,
        personsApi: widget.personsApi,
        l10n: widget.l10n,
      ),
    );
    if (!mounted) return;
    if (changed == true) {
      Navigator.of(context).pop(true);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = widget.l10n;
    final row = widget.row;
    return AlertDialog(
      title: Text(l10n.historyDetailTitle),
      content: SizedBox(
        width: double.maxFinite,
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              _DetailRow(
                label: l10n.historyLabelDateTime,
                value: _fmtDateTime(row.datetime),
              ),
              _DetailRow(
                label: l10n.historyLabelAction,
                value: _actionLabel(row.action, l10n),
              ),
              _DetailRow(label: l10n.historyLabelItem, value: row.itemName),
              _DetailRow(label: l10n.historyLabelUser, value: row.userName),
              _DetailRow(label: l10n.labelCount, value: row.count.toString()),
              if (row.remarks.isNotEmpty)
                _DetailRow(label: l10n.historyLabelRemarks, value: row.remarks),
              if (row.action == 'lending' && row.duedate != null)
                _DetailRow(
                  label: l10n.labelDueDate,
                  value: _fmtDate(row.duedate!),
                ),
              if (widget.isAdmin && row.collector.isNotEmpty)
                _DetailRow(
                  label: l10n.historyLabelCollector,
                  value: row.collector,
                ),
            ],
          ),
        ),
      ),
      actions: [
        if (_busy)
          const Padding(
            padding: EdgeInsets.only(right: 8),
            child: SizedBox(
              width: 20,
              height: 20,
              child: CircularProgressIndicator.adaptive(strokeWidth: 2),
            ),
          ),
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).closeButtonLabel),
        ),
        if (widget.isAdmin) ...[
          TextButton(
            onPressed: _busy ? null : _showEdit,
            child: Text(l10n.historyEditTitle),
          ),
          TextButton(
            onPressed: _busy ? null : _confirmDelete,
            child: Text(l10n.historyDeleteTitle),
          ),
        ],
      ],
    );
  }
}

// ─── Edit dialog ───────────────────────────────────────────────────────────

class _EditDialog extends StatefulWidget {
  const _EditDialog({
    required this.row,
    required this.historyApi,
    required this.personsApi,
    required this.l10n,
  });

  final HistoryRow row;
  final HistoryApi historyApi;
  final PersonsApi personsApi;
  final AppLocalizations l10n;

  @override
  State<_EditDialog> createState() => _EditDialogState();
}

class _EditDialogState extends State<_EditDialog> {
  late DateTime _datetime;
  late int _count;
  late final TextEditingController _remarksCtrl;
  late int _personId;
  late String _personName;
  DateTime? _duedate;
  bool _busy = false;

  bool get _isLending => widget.row.action == 'lending';
  bool get _hasRemarks =>
      widget.row.action == 'lending' || widget.row.action == 'return';

  @override
  void initState() {
    super.initState();
    _datetime = widget.row.datetime;
    _count = widget.row.count;
    _remarksCtrl = TextEditingController(text: widget.row.remarks);
    _personId = widget.row.personId;
    _personName = widget.row.userName;
    _duedate = widget.row.duedate;
  }

  @override
  void dispose() {
    _remarksCtrl.dispose();
    super.dispose();
  }

  Future<void> _pickDate() async {
    final local = _datetime.toLocal();
    final picked = await showDatePicker(
      context: context,
      initialDate: local,
      firstDate: DateTime(2000),
      lastDate: DateTime.now().add(const Duration(days: 1)),
    );
    if (!mounted || picked == null) return;
    final time = TimeOfDay.fromDateTime(local);
    final picked2 = await showTimePicker(context: context, initialTime: time);
    if (!mounted) return;
    final t = picked2 ?? time;
    setState(() {
      _datetime = DateTime(
        picked.year,
        picked.month,
        picked.day,
        t.hour,
        t.minute,
      );
    });
  }

  Future<void> _pickDueDate() async {
    final picked = await showDatePicker(
      context: context,
      initialDate: _duedate ?? DateTime.now().add(const Duration(days: 14)),
      firstDate: DateTime(2000),
      lastDate: DateTime.now().add(const Duration(days: 730)),
    );
    if (picked != null) {
      setState(() {
        _duedate = picked;
      });
    }
  }

  Future<void> _save() async {
    setState(() {
      _busy = true;
    });

    final newRow = widget.row.copyWith(
      datetime: _datetime,
      count: _count,
      remarks: _hasRemarks ? _remarksCtrl.text : null,
      personId: _isLending ? _personId : null,
      userName: _isLending ? _personName : null,
      duedate: _isLending ? _duedate : null,
      clearDuedate: _isLending && _duedate == null,
    );

    final List<HistoryRow> conflicts;
    try {
      conflicts = await widget.historyApi.getChangeConflicts(
        widget.row,
        newRow,
      );
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
      });
      await ServerErrorDialog.show(context, e, widget.l10n);
      return;
    }

    if (!mounted) return;

    if (conflicts.isNotEmpty) {
      final proceed =
          await showDialog<bool>(
            context: context,
            builder: (ctx) =>
                _ChangeConflictDialog(conflicts: conflicts, l10n: widget.l10n),
          ) ==
          true;
      if (!mounted || !proceed) {
        setState(() {
          _busy = false;
        });
        return;
      }
    }

    try {
      await widget.historyApi.changeAction(widget.row, newRow);
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(widget.l10n.historyEditSuccess)));
      Navigator.of(context).pop(true);
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
    return AlertDialog(
      title: Text(l10n.historyEditTitle),
      content: SizedBox(
        width: double.maxFinite,
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              ListTile(
                contentPadding: EdgeInsets.zero,
                title: Text(l10n.historyLabelDateTime),
                subtitle: Text(_fmtDateTime(_datetime)),
                trailing: const Icon(Icons.edit_calendar),
                onTap: _busy ? null : _pickDate,
              ),
              Row(
                children: [
                  Text(l10n.labelCount),
                  const Spacer(),
                  IconButton(
                    onPressed: _count > 1 && !_busy
                        ? () => setState(() {
                            _count--;
                          })
                        : null,
                    icon: const Icon(Icons.remove),
                  ),
                  Text('$_count'),
                  IconButton(
                    onPressed: _busy
                        ? null
                        : () => setState(() {
                            _count++;
                          }),
                    icon: const Icon(Icons.add),
                  ),
                ],
              ),
              if (_isLending) ...[
                const SizedBox(height: 8),
                PersonSearchField(
                  personsApi: widget.personsApi,
                  labelText: l10n.labelPersonSearch,
                  initialText: _personName,
                  onSelected: (Person p) {
                    setState(() {
                      _personId = p.id;
                      _personName = '${p.firstname} ${p.lastname}';
                    });
                  },
                ),
                ListTile(
                  contentPadding: EdgeInsets.zero,
                  title: Text(l10n.labelDueDate),
                  subtitle: Text(_duedate != null ? _fmtDate(_duedate!) : '—'),
                  trailing: const Icon(Icons.calendar_today),
                  onTap: _busy ? null : _pickDueDate,
                ),
              ],
              if (_hasRemarks) ...[
                const SizedBox(height: 8),
                TextField(
                  controller: _remarksCtrl,
                  enabled: !_busy,
                  decoration: InputDecoration(
                    labelText: l10n.historyLabelRemarks,
                    border: const OutlineInputBorder(),
                  ),
                  maxLines: 2,
                ),
              ],
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: _busy ? null : _save,
          child: _busy
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child: CircularProgressIndicator.adaptive(strokeWidth: 2),
                )
              : Text(l10n.historyEditSave),
        ),
      ],
    );
  }
}

// ─── Conflict dialogs ──────────────────────────────────────────────────────

class _ConflictDeleteDialog extends StatelessWidget {
  const _ConflictDeleteDialog({required this.toDelete, required this.l10n});

  final List<HistoryRow> toDelete;
  final AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    final nConflicts = toDelete.length - 1;
    return AlertDialog(
      title: Text(l10n.historyDeleteTitle),
      content: SizedBox(
        width: double.maxFinite,
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(l10n.historyDeleteWithConflicts(nConflicts)),
              const SizedBox(height: 8),
              ...toDelete.map(
                (r) => ListTile(
                  dense: true,
                  title: Text(r.itemName),
                  subtitle: Text(
                    '${_actionLabel(r.action, l10n)} · '
                    '${r.userName} · '
                    '${_fmtDateTime(r.datetime)}',
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: () => Navigator.pop(context, true),
          child: Text(l10n.historyDeleteAll),
        ),
      ],
    );
  }
}

class _ChangeConflictDialog extends StatelessWidget {
  const _ChangeConflictDialog({required this.conflicts, required this.l10n});

  final List<HistoryRow> conflicts;
  final AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(l10n.historyChangeConflictsTitle),
      content: SizedBox(
        width: double.maxFinite,
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(l10n.historyChangeConflicts(conflicts.length)),
              const SizedBox(height: 8),
              ...conflicts.map(
                (r) => ListTile(
                  dense: true,
                  title: Text(r.itemName),
                  subtitle: Text(
                    '${_actionLabel(r.action, l10n)} · ${r.userName}',
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: () => Navigator.pop(context, true),
          child: Text(l10n.actionConfirm),
        ),
      ],
    );
  }
}

// ─── Shared helpers ────────────────────────────────────────────────────────

class _DetailRow extends StatelessWidget {
  const _DetailRow({required this.label, required this.value});

  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            width: 96,
            child: Text(
              label,
              style: Theme.of(context).textTheme.labelMedium?.copyWith(
                color: Theme.of(context).colorScheme.onSurfaceVariant,
              ),
            ),
          ),
          Expanded(child: Text(value)),
        ],
      ),
    );
  }
}

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

// ─── Pure helper functions ─────────────────────────────────────────────────

String _fmtDate(DateTime d) => d.toIso8601String().substring(0, 10);

String _fmtDateTime(DateTime dt) =>
    dt.toLocal().toIso8601String().substring(0, 16).replaceAll('T', ' ');

(Color, IconData) _actionVisual(String action) {
  switch (action) {
    case 'acquisition':
      return (Colors.green, Icons.add_circle_outline);
    case 'liquidation':
      return (Colors.grey, Icons.remove_circle_outline);
    case 'lending':
      return (Colors.blue, Icons.arrow_forward);
    case 'return':
      return (Colors.teal, Icons.arrow_back);
    case 'reservation':
      return (Colors.orange, Icons.bookmark_outline);
    default:
      return (Colors.grey, Icons.help_outline);
  }
}

/// Action types offered by the history filter (REST values).
const _kFilterActions = [
  'acquisition',
  'liquidation',
  'lending',
  'return',
  'reservation',
];

String _actionLabel(String action, AppLocalizations l10n) {
  switch (action) {
    case 'acquisition':
      return l10n.historyActionAcquisition;
    case 'liquidation':
      return l10n.historyActionLiquidation;
    case 'lending':
      return l10n.historyActionLending;
    case 'return':
      return l10n.historyActionReturn;
    case 'reservation':
      return l10n.historyActionReservation;
    default:
      return action;
  }
}
