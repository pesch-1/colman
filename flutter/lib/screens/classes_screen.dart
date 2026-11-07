// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../api/classes_api.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';
import 'new_school_year_screen.dart';

/// Displays and manages school classes.
///
/// Read-only for regular users; users with `manage_classes` can add, rename,
/// and delete classes, edit the grade range, and launch the New School Year
/// wizard.
class ClassesScreen extends ConsumerStatefulWidget {
  const ClassesScreen({super.key});

  @override
  ConsumerState<ClassesScreen> createState() => _ClassesScreenState();
}

class _ClassesScreenState extends ConsumerState<ClassesScreen> {
  List<String> _classes = [];
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
      final classes = await ref.read(classesApiProvider).loadClassNames();
      if (!mounted) return;
      setState(() {
        _classes = classes;
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

  Future<void> _showAddDialog(AppLocalizations l10n, ClassesApi api) async {
    final added = await showDialog<String>(
      context: context,
      builder: (_) => _NameDialog(
        title: l10n.classesAddTitle,
        label: l10n.classesAddNameLabel,
        confirmLabel: l10n.classesAddTitle,
      ),
    );
    if (added != null && mounted) {
      try {
        await api.addClass(added);
        if (mounted) {
          _load();
        }
      } catch (e) {
        if (mounted) {
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
    }
  }

  Future<void> _showRenameDialog(
    AppLocalizations l10n,
    ClassesApi api,
    String oldName,
  ) async {
    final newName = await showDialog<String>(
      context: context,
      builder: (_) => _NameDialog(
        title: l10n.classesRenameTitle,
        label: l10n.classesRenameNewNameLabel,
        initialValue: oldName,
        confirmLabel: l10n.settingsSave,
      ),
    );
    if (newName != null && newName != oldName && mounted) {
      try {
        await api.renameClass(oldName, newName);
        if (mounted) {
          _load();
        }
      } catch (e) {
        if (mounted) {
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
    }
  }

  Future<void> _showDeleteConfirm(
    AppLocalizations l10n,
    ClassesApi api,
    String name,
  ) async {
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        content: Text(l10n.classesDeleteConfirm(name)),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: Text(MaterialLocalizations.of(ctx).cancelButtonLabel),
          ),
          TextButton(
            onPressed: () => Navigator.pop(ctx, true),
            child: Text(l10n.personDelete),
          ),
        ],
      ),
    );
    if (confirmed == true && mounted) {
      try {
        await api.removeClass(name);
        if (mounted) {
          _load();
        }
      } catch (e) {
        if (mounted) {
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
    }
  }

  Future<void> _showGradeRangeDialog(
    AppLocalizations l10n,
    ClassesApi api,
  ) async {
    late final ({int min, int max}) range;
    try {
      range = await api.fetchGradeRange();
    } catch (e) {
      if (mounted) {
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
      return;
    }
    if (!mounted) return;
    await showDialog<void>(
      context: context,
      builder: (_) => _GradeRangeDialog(
        api: api,
        initialMin: range.min,
        initialMax: range.max,
        l10n: l10n,
      ),
    );
  }

  Future<void> _openNewSchoolYearWizard(
    AppLocalizations l10n,
    ClassesApi api,
  ) async {
    late final ({int min, int max}) range;
    try {
      range = await api.fetchGradeRange();
    } catch (e) {
      if (mounted) {
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
      return;
    }
    if (!mounted) return;
    await Navigator.of(context).push(
      MaterialPageRoute<void>(
        builder: (_) => NewSchoolYearScreen(
          classesApi: api,
          classes: List.unmodifiable(_classes),
          gradeMin: range.min,
          gradeMax: range.max,
        ),
      ),
    );
    if (mounted) {
      _load();
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['classes'], _load);
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canManage =
        auth is Authenticated && auth.hasPermission('manage_classes');
    final api = ref.read(classesApiProvider);
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/pupils.jpeg',
          title: l10n.classesTitle,
        ),
        actions: canManage
            ? [
                IconButton(
                  icon: const Icon(Icons.tune),
                  tooltip: l10n.classesGradeRange,
                  onPressed: () => _showGradeRangeDialog(l10n, api),
                ),
                IconButton(
                  icon: const Icon(Icons.school),
                  tooltip: l10n.classesNewSchoolYear,
                  onPressed: () => _openNewSchoolYearWizard(l10n, api),
                ),
              ]
            : null,
      ),
      body: _buildBody(l10n, canManage, api),
      floatingActionButton: canManage
          ? FloatingActionButton(
              onPressed: () => _showAddDialog(l10n, api),
              tooltip: l10n.classesAddTooltip,
              child: const Icon(Icons.add),
            )
          : null,
    );
  }

  Widget _buildBody(AppLocalizations l10n, bool canManage, ClassesApi api) {
    if (_loading && _classes.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (_classes.isEmpty) {
      return Center(child: Text(l10n.classesNoResults));
    }
    return RefreshIndicator(
      onRefresh: _load,
      child: ListView.separated(
        itemCount: _classes.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (context, i) {
          final name = _classes[i];
          return ListTile(
            title: Text(name),
            trailing: canManage
                ? PopupMenuButton<_ClassAction>(
                    onSelected: (action) {
                      if (action == _ClassAction.rename) {
                        _showRenameDialog(l10n, api, name);
                      } else {
                        _showDeleteConfirm(l10n, api, name);
                      }
                    },
                    itemBuilder: (_) => [
                      PopupMenuItem(
                        value: _ClassAction.rename,
                        child: Text(l10n.classesRenameTitle),
                      ),
                      PopupMenuItem(
                        value: _ClassAction.delete,
                        child: Text(l10n.personDelete),
                      ),
                    ],
                  )
                : null,
          );
        },
      ),
    );
  }
}

enum _ClassAction { rename, delete }

// ─── Name input dialog ────────────────────────────────────────────────────────

class _NameDialog extends StatefulWidget {
  const _NameDialog({
    required this.title,
    required this.label,
    required this.confirmLabel,
    this.initialValue = '',
  });

  final String title;
  final String label;
  final String confirmLabel;
  final String initialValue;

  @override
  State<_NameDialog> createState() => _NameDialogState();
}

class _NameDialogState extends State<_NameDialog> {
  late final TextEditingController _ctrl;

  @override
  void initState() {
    super.initState();
    _ctrl = TextEditingController(text: widget.initialValue);
    _ctrl.addListener(() => setState(() {}));
  }

  @override
  void dispose() {
    _ctrl.dispose();
    super.dispose();
  }

  bool get _valid => _ctrl.text.trim().isNotEmpty;

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text(widget.title),
      content: TextField(
        controller: _ctrl,
        autofocus: true,
        decoration: InputDecoration(
          labelText: widget.label,
          border: const OutlineInputBorder(),
        ),
        onSubmitted: _valid
            ? (_) => Navigator.pop(context, _ctrl.text.trim())
            : null,
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: _valid
              ? () => Navigator.pop(context, _ctrl.text.trim())
              : null,
          child: Text(widget.confirmLabel),
        ),
      ],
    );
  }
}

// ─── Grade range dialog ───────────────────────────────────────────────────────

class _GradeRangeDialog extends StatefulWidget {
  const _GradeRangeDialog({
    required this.api,
    required this.initialMin,
    required this.initialMax,
    required this.l10n,
  });

  final ClassesApi api;
  final int initialMin;
  final int initialMax;
  final AppLocalizations l10n;

  @override
  State<_GradeRangeDialog> createState() => _GradeRangeDialogState();
}

class _GradeRangeDialogState extends State<_GradeRangeDialog> {
  late int _min;
  late int _max;
  bool _busy = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    _min = widget.initialMin;
    _max = widget.initialMax;
  }

  Future<void> _save() async {
    setState(() {
      _busy = true;
      _error = null;
    });
    try {
      await widget.api.saveGradeRange(_min, _max);
      if (!mounted) return;
      Navigator.pop(context);
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
        _error = e;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = widget.l10n;
    return AlertDialog(
      title: Text(l10n.classesGradeRange),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          _SpinRow(
            label: l10n.classesGradeRangeMin,
            value: _min,
            onDecrement: _busy || _min <= 1
                ? null
                : () => setState(() => _min--),
            onIncrement: _busy || _min >= _max
                ? null
                : () => setState(() => _min++),
          ),
          _SpinRow(
            label: l10n.classesGradeRangeMax,
            value: _max,
            onDecrement: _busy || _max <= _min
                ? null
                : () => setState(() => _max--),
            onIncrement: _busy || _max >= 99
                ? null
                : () => setState(() => _max++),
          ),
          if (_error != null) ...[
            const SizedBox(height: 8),
            ErrorDisplay(error: _error!, l10n: l10n),
          ],
        ],
      ),
      actions: [
        TextButton(
          onPressed: _busy ? null : () => Navigator.pop(context),
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
              : Text(l10n.settingsSave),
        ),
      ],
    );
  }
}

class _SpinRow extends StatelessWidget {
  const _SpinRow({
    required this.label,
    required this.value,
    required this.onDecrement,
    required this.onIncrement,
  });

  final String label;
  final int value;
  final VoidCallback? onDecrement;
  final VoidCallback? onIncrement;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Expanded(child: Text(label)),
        IconButton(icon: const Icon(Icons.remove), onPressed: onDecrement),
        SizedBox(width: 36, child: Text('$value', textAlign: TextAlign.center)),
        IconButton(icon: const Icon(Icons.add), onPressed: onIncrement),
      ],
    );
  }
}
