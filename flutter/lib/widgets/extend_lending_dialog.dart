// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../api/collection_api.dart';
import '../models/due_lending_row.dart';
import '../providers/api_client_provider.dart';
import 'error_display.dart';
import 'server_error_dialog.dart';

/// Lets the user choose open lendings of [personId] and set a new due date.
///
/// Used where only the borrower is known (lending wizard, person step): lists
/// all open lendings of the person, all preselected, plus the new due date,
/// which defaults to today plus the collection's default lending period
/// (today when none is configured).
///
/// Returns `true` when at least one lending was changed.  Callers must only
/// offer this dialog to users holding `collect_lendings`.
Future<bool> showExtendLendingDialog(
  BuildContext context, {
  required int personId,
}) async {
  final result = await showDialog<bool>(
    context: context,
    builder: (_) => _ExtendLendingDialog(personId: personId),
  );
  return result ?? false;
}

/// Sets a new due date for [rows], which the caller has already chosen.
///
/// Used where the lendings are known (selected rows of the open-lendings
/// screen, the lending of the return wizard): shows them as read-only
/// overview and asks only for the new due date.
///
/// Returns `true` when the lendings were changed.  Callers must only offer
/// this dialog to users holding `collect_lendings`.
Future<bool> showExtendSelectedLendingsDialog(
  BuildContext context,
  List<DueLendingRow> rows,
) async {
  if (rows.isEmpty) return false;
  final result = await showDialog<bool>(
    context: context,
    builder: (_) => _ExtendLendingDialog(fixedRows: rows),
  );
  return result ?? false;
}

class _ExtendLendingDialog extends ConsumerStatefulWidget {
  const _ExtendLendingDialog({this.personId, this.fixedRows});

  /// Borrower whose open lendings are offered for selection.
  final int? personId;

  /// Lendings already chosen by the caller; no selection is offered.
  final List<DueLendingRow>? fixedRows;

  bool get _selectable => fixedRows == null;

  @override
  ConsumerState<_ExtendLendingDialog> createState() =>
      _ExtendLendingDialogState();
}

class _ExtendLendingDialogState extends ConsumerState<_ExtendLendingDialog> {
  List<DueLendingRow> _rows = [];
  final Set<int> _selected = {};
  DateTime _duedate = _today();
  bool _loading = true;
  bool _submitting = false;
  Object? _error;

  /// Collection settings used to propose the new due date.
  CollectionInfo? _info;

  /// True once the user picked a date; selection changes then no longer
  /// replace it with a new proposal.
  bool _dateTouched = false;

  static DateTime _today() {
    final now = DateTime.now();
    return DateTime(now.year, now.month, now.day);
  }

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    try {
      final rows =
          widget.fixedRows ??
          await ref.read(lendingsApiProvider).getDue(personId: widget.personId);
      final info = await ref.read(collectionApiProvider).get();
      if (!mounted) return;
      setState(() {
        _rows = rows;
        _selected
          ..clear()
          ..addAll(rows.map((r) => r.lendingId));
        _info = info;
        _duedate = info.proposeExtendedDueDate(rows.map((r) => r.duedate));
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

  Future<void> _pickDate() async {
    final today = _today();
    final picked = await showDatePicker(
      context: context,
      initialDate: _duedate,
      firstDate: today,
      lastDate: DateTime(today.year + 10, today.month, today.day),
    );
    if (picked != null && mounted) {
      setState(() {
        _duedate = picked;
        _dateTouched = true;
      });
    }
  }

  /// Re-proposes the due date for the selected lendings (call inside
  /// setState) unless the user has already picked a date.
  void _updateProposal() {
    final info = _info;
    if (info == null || _dateTouched || _selected.isEmpty) return;
    _duedate = info.proposeExtendedDueDate(
      _rows.where((r) => _selected.contains(r.lendingId)).map((r) => r.duedate),
    );
  }

  Future<void> _extend() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() {
      _submitting = true;
    });
    try {
      await ref.read(lendingsApiProvider).extend(_selected.toList(), _duedate);
      if (!mounted) return;
      Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _submitting = false;
      });
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return AlertDialog(
      title: Text(l10n.extendDueDateTitle),
      content: SizedBox(width: 480, child: _buildContent(l10n)),
      actions: [
        TextButton(
          onPressed: _submitting
              ? null
              : () => Navigator.of(context).pop(false),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        FilledButton(
          onPressed: _loading || _submitting || _selected.isEmpty
              ? null
              : _extend,
          child: _submitting
              ? const SizedBox(
                  width: 18,
                  height: 18,
                  child: CircularProgressIndicator(strokeWidth: 2),
                )
              : Text(l10n.extendDueDateAction),
        ),
      ],
    );
  }

  Widget _buildContent(AppLocalizations l10n) {
    if (_loading) {
      return const SizedBox(
        height: 120,
        child: Center(child: CircularProgressIndicator()),
      );
    }
    if (_error != null) {
      return ErrorDisplay(error: _error!, l10n: l10n);
    }
    if (_rows.isEmpty) {
      return Text(l10n.extendDueDateNoLendings);
    }
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(
          widget._selectable
              ? l10n.extendDueDateSelectHint
              : l10n.extendDueDateFixedHint(_rows.length),
        ),
        const SizedBox(height: 8),
        Flexible(
          child: ListView(
            shrinkWrap: true,
            children: [
              for (final row in _rows)
                if (!widget._selectable)
                  ListTile(
                    dense: true,
                    leading: const Icon(Icons.book_outlined),
                    title: Text(row.itemName),
                    subtitle: Text(
                      '${row.personName}  ·  ${l10n.labelDueDate}: '
                      '${row.duedate.year > 0 ? _fmtDate(row.duedate) : '–'}',
                    ),
                  )
                else
                  CheckboxListTile(
                    dense: true,
                    value: _selected.contains(row.lendingId),
                    onChanged: _submitting
                        ? null
                        : (v) {
                            setState(() {
                              if (v == true) {
                                _selected.add(row.lendingId);
                              } else {
                                _selected.remove(row.lendingId);
                              }
                              _updateProposal();
                            });
                          },
                    title: Text(row.itemName),
                    subtitle: Text(
                      '${l10n.labelDueDate}: '
                      '${row.duedate.year > 0 ? _fmtDate(row.duedate) : '–'}'
                      '  ·  ${l10n.lendingDueOutstanding(row.outstandingCount)}',
                    ),
                  ),
            ],
          ),
        ),
        const SizedBox(height: 12),
        ListTile(
          contentPadding: EdgeInsets.zero,
          leading: const Icon(Icons.event),
          title: Text(l10n.extendDueDateNewDate),
          subtitle: Text(_fmtDate(_duedate)),
          trailing: const Icon(Icons.edit_calendar),
          onTap: _submitting ? null : _pickDate,
        ),
      ],
    );
  }
}

/// Formats [d] as an ISO 8601 date string (YYYY-MM-DD).
String _fmtDate(DateTime d) =>
    '${d.year.toString().padLeft(4, '0')}-'
    '${d.month.toString().padLeft(2, '0')}-'
    '${d.day.toString().padLeft(2, '0')}';
