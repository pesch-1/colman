// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../api/classes_api.dart';
import '../models/affected_person.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';

/// Two-step wizard for executing a school-year class transition.
///
/// Step 1: edit the old→new name mapping for each class (with suggested
/// names computed by [_computeNewName]).
/// Step 2: review affected persons and, optionally, mark them for deletion.
class NewSchoolYearScreen extends StatefulWidget {
  const NewSchoolYearScreen({
    super.key,
    required this.classesApi,
    required this.classes,
    required this.gradeMin,
    required this.gradeMax,
  });

  final ClassesApi classesApi;
  final List<String> classes;
  final int gradeMin;
  final int gradeMax;

  @override
  State<NewSchoolYearScreen> createState() => _NewSchoolYearScreenState();
}

class _NewSchoolYearScreenState extends State<NewSchoolYearScreen> {
  // Step index: 0 = mapping, 1 = persons.
  int _step = 0;

  // Step 1 state: list of (oldClass, newNameController) pairs.
  // Entries with empty oldClass are "extra" new classes added by the user.
  late List<_ClassRow> _rows;

  // Step 2 state.
  List<AffectedPerson> _affected = [];
  Set<int> _toDelete = {};
  bool _busy = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    _rows = widget.classes.map((cls) {
      final suggested = _computeNewName(
        cls,
        widget.gradeMin,
        widget.gradeMax,
      );
      return _ClassRow(
        oldClass: cls,
        ctrl: TextEditingController(text: suggested),
      );
    }).toList();
  }

  @override
  void dispose() {
    for (final row in _rows) {
      row.ctrl.dispose();
    }
    super.dispose();
  }

  // ─── Step 1 helpers ──────────────────────────────────────────────────────

  void _addExtraClass() {
    setState(() {
      _rows.add(_ClassRow(oldClass: '', ctrl: TextEditingController()));
    });
  }

  void _removeRow(int index) {
    final row = _rows[index];
    setState(() {
      _rows.removeAt(index);
    });
    row.ctrl.dispose();
  }

  /// Builds the old→new map for the API call.
  ///
  /// Only includes rows with a non-empty [oldClass] (extra new-class rows
  /// are not included because the server applies the new_class_list directly).
  Map<String, String> _buildOldToNew() {
    return {
      for (final row in _rows)
        if (row.oldClass.isNotEmpty) row.oldClass: row.ctrl.text.trim(),
    };
  }

  /// All new class names (non-empty) from step 1 — becomes the new class list.
  List<String> _buildNewClassList() {
    final names = <String>{};
    for (final row in _rows) {
      final n = row.ctrl.text.trim();
      if (n.isNotEmpty) {
        names.add(n);
      }
    }
    return names.toList()..sort();
  }

  Future<void> _goToStep2() async {
    setState(() {
      _busy = true;
      _error = null;
    });
    try {
      final persons =
          await widget.classesApi.getAffectedPersons(_buildOldToNew());
      if (!mounted) return;
      setState(() {
        _affected = persons;
        _toDelete = {};
        _step = 1;
        _busy = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
        _error = e;
      });
    }
  }

  // ─── Step 2 helpers ──────────────────────────────────────────────────────

  void _updatePersonClass(int personId, String newClass) {
    setState(() {
      _affected = [
        for (final p in _affected)
          p.personId == personId ? p.copyWith(newClass: newClass) : p,
      ];
      if (newClass.isNotEmpty) {
        _toDelete.remove(personId);
      }
    });
  }

  void _toggleDelete(int personId, bool delete) {
    setState(() {
      if (delete) {
        _toDelete.add(personId);
      } else {
        _toDelete.remove(personId);
      }
    });
  }

  Future<void> _apply() async {
    setState(() {
      _busy = true;
      _error = null;
    });
    try {
      final personToClass = <int, String>{
        for (final p in _affected)
          if (!_toDelete.contains(p.personId)) p.personId: p.newClass,
      };
      await widget.classesApi.applyYearChanges(
        personToClass: personToClass,
        toDelete: _toDelete.toList(),
        newClassList: _buildNewClassList(),
      );
      if (!mounted) return;
      Navigator.of(context).pop();
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _busy = false;
        _error = e;
      });
    }
  }

  // ─── Build ────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/pupils.jpeg',
          title: l10n.classesNewSchoolYear,
        ),
      ),
      body: IndexedStack(
        index: _step,
        children: [
          _buildStep1(l10n),
          _buildStep2(l10n),
        ],
      ),
    );
  }

  // ─── Step 1: class name mapping ───────────────────────────────────────────

  Widget _buildStep1(AppLocalizations l10n) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Padding(
          padding: const EdgeInsets.fromLTRB(16, 16, 16, 4),
          child: Text(
            l10n.classesNewSchoolYearStep1,
            style: Theme.of(context).textTheme.titleMedium,
          ),
        ),
        Expanded(
          child: ListView.builder(
            padding: const EdgeInsets.symmetric(horizontal: 16),
            itemCount: _rows.length,
            itemBuilder: (_, i) => _buildMappingRow(l10n, i),
          ),
        ),
        if (_error != null)
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
            child: ErrorDisplay(error: _error!, l10n: l10n),
          ),
        Padding(
          padding: const EdgeInsets.all(16),
          child: Row(
            children: [
              OutlinedButton.icon(
                onPressed: _busy ? null : _addExtraClass,
                icon: const Icon(Icons.add),
                label: Text(l10n.classesNewSchoolYearAddClass),
              ),
              const Spacer(),
              FilledButton(
                onPressed: _busy ? null : _goToStep2,
                child: _busy
                    ? const SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator.adaptive(
                            strokeWidth: 2),
                      )
                    : Text(l10n.classesNext),
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildMappingRow(AppLocalizations l10n, int index) {
    final row = _rows[index];
    final newName = row.ctrl.text.trim();
    final dissolved = row.oldClass.isNotEmpty && newName.isEmpty;
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        children: [
          if (row.oldClass.isNotEmpty) ...[
            SizedBox(
              width: 64,
              child: Text(
                row.oldClass,
                style: Theme.of(context).textTheme.bodyMedium,
              ),
            ),
            const Icon(Icons.arrow_forward, size: 16),
            const SizedBox(width: 8),
          ] else ...[
            const SizedBox(width: 88),
          ],
          Expanded(
            child: TextField(
              controller: row.ctrl,
              decoration: InputDecoration(
                isDense: true,
                contentPadding:
                    const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
                border: const OutlineInputBorder(),
                hintText: dissolved ? l10n.classesNewSchoolYearDissolved : null,
                hintStyle: dissolved
                    ? TextStyle(
                        color: Theme.of(context).colorScheme.outline,
                        fontStyle: FontStyle.italic,
                      )
                    : null,
              ),
              onChanged: (_) => setState(() {}),
            ),
          ),
          IconButton(
            icon: const Icon(Icons.close, size: 18),
            onPressed: () => _removeRow(index),
          ),
        ],
      ),
    );
  }

  // ─── Step 2: affected persons ─────────────────────────────────────────────

  Widget _buildStep2(AppLocalizations l10n) {
    final newClasses = _buildNewClassList();
    final noClassOption = l10n.classesNewSchoolYearNoClass;

    if (_step == 0) {
      // Not yet visible; return empty to avoid computing newClasses early.
      return const SizedBox.shrink();
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Padding(
          padding: const EdgeInsets.fromLTRB(16, 16, 16, 4),
          child: Text(
            l10n.classesNewSchoolYearStep2,
            style: Theme.of(context).textTheme.titleMedium,
          ),
        ),
        Expanded(
          child: _affected.isEmpty
              ? Center(child: Text(l10n.classesNewSchoolYearNoPersons))
              : _buildPersonList(l10n, newClasses, noClassOption),
        ),
        if (_error != null)
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
            child: ErrorDisplay(error: _error!, l10n: l10n),
          ),
        Padding(
          padding: const EdgeInsets.all(16),
          child: Row(
            children: [
              OutlinedButton(
                onPressed: _busy
                    ? null
                    : () => setState(() {
                          _step = 0;
                          _error = null;
                        }),
                child: Text(
                    MaterialLocalizations.of(context).backButtonTooltip),
              ),
              const Spacer(),
              FilledButton(
                onPressed: _busy ? null : _apply,
                child: _busy
                    ? const SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator.adaptive(
                            strokeWidth: 2),
                      )
                    : Text(l10n.classesApply),
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildPersonList(
    AppLocalizations l10n,
    List<String> newClasses,
    String noClassOption,
  ) {
    // Group persons by oldClass.
    final groups = <String, List<AffectedPerson>>{};
    for (final p in _affected) {
      groups.putIfAbsent(p.oldClass, () => []).add(p);
    }
    final oldClasses = groups.keys.toList()..sort();

    final items = <Widget>[];
    for (final oldClass in oldClasses) {
      items.add(
        Padding(
          padding: const EdgeInsets.fromLTRB(16, 12, 16, 4),
          child: Text(
            oldClass,
            style: Theme.of(context).textTheme.labelLarge?.copyWith(
                  color: Theme.of(context).colorScheme.primary,
                ),
          ),
        ),
      );
      for (final p in groups[oldClass]!) {
        final deleted = _toDelete.contains(p.personId);
        items.add(
          _PersonRow(
            person: p,
            newClasses: newClasses,
            noClassOption: noClassOption,
            deleteLabel: l10n.classesNewSchoolYearDeletePerson,
            deleted: deleted,
            onClassChanged: (cls) =>
                _updatePersonClass(p.personId, cls == noClassOption ? '' : cls),
            onDeleteToggled: (d) => _toggleDelete(p.personId, d),
          ),
        );
      }
    }

    return ListView(
      children: items,
    );
  }
}

// ─── Step 1 row data ──────────────────────────────────────────────────────────

class _ClassRow {
  _ClassRow({required this.oldClass, required this.ctrl});

  final String oldClass;
  final TextEditingController ctrl;
}

// ─── Step 2 person row ────────────────────────────────────────────────────────

class _PersonRow extends StatelessWidget {
  const _PersonRow({
    required this.person,
    required this.newClasses,
    required this.noClassOption,
    required this.deleteLabel,
    required this.deleted,
    required this.onClassChanged,
    required this.onDeleteToggled,
  });

  final AffectedPerson person;
  final List<String> newClasses;
  final String noClassOption;
  final String deleteLabel;
  final bool deleted;
  final ValueChanged<String> onClassChanged;
  final ValueChanged<bool> onDeleteToggled;

  @override
  Widget build(BuildContext context) {
    final dropdownValue = person.newClass.isEmpty
        ? noClassOption
        : (newClasses.contains(person.newClass)
            ? person.newClass
            : noClassOption);
    final dropdownItems = [
      ...newClasses.map(
        (cls) => DropdownMenuItem(value: cls, child: Text(cls)),
      ),
      DropdownMenuItem(
        value: noClassOption,
        child: Text(
          noClassOption,
          style: const TextStyle(fontStyle: FontStyle.italic),
        ),
      ),
    ];

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 2),
      child: Row(
        children: [
          Expanded(
            child: Text(
              person.displayName,
              style: deleted
                  ? TextStyle(
                      decoration: TextDecoration.lineThrough,
                      color: Theme.of(context).colorScheme.outline,
                    )
                  : null,
            ),
          ),
          DropdownButton<String>(
            value: dropdownValue,
            items: dropdownItems,
            onChanged: deleted ? null : (v) => onClassChanged(v!),
          ),
          Checkbox(
            value: deleted,
            onChanged: person.newClass.isEmpty
                ? (v) => onDeleteToggled(v ?? false)
                : (v) => onDeleteToggled(v ?? false),
          ),
          Text(
            deleteLabel,
            style: Theme.of(context).textTheme.bodySmall,
          ),
        ],
      ),
    );
  }
}

// ─── Grade transition algorithm ───────────────────────────────────────────────

/// Computes the suggested class name for the next school year.
///
/// Port of `colman::compute_new_name` from `colman_class_names.hpp`.
///
/// - One digit sequence: increment by 1; return '' if result exceeds [max].
/// - Two digit sequences: increment both; if either exceeds [max], reset it to
///   [min] and swap the two numbers in the string.
/// - No digit sequences: return [name] unchanged.
String _computeNewName(String name, int min, int max) {
  final rx = RegExp(r'(\d+)');
  final matches = rx.allMatches(name).toList();

  if (matches.length == 1) {
    final n = int.parse(matches[0].group(0)!) + 1;
    if (n > max) return '';
    return name.substring(0, matches[0].start) +
        n.toString() +
        name.substring(matches[0].end);
  }

  if (matches.length == 2) {
    int a = int.parse(matches[0].group(0)!) + 1;
    int b = int.parse(matches[1].group(0)!) + 1;
    final swapNeeded = a > max || b > max;
    if (a > max) a = min;
    if (b > max) b = min;
    if (swapNeeded) {
      final t = a;
      a = b;
      b = t;
    }
    // Replace right-to-left to preserve string indices.
    var result = name;
    result = result.substring(0, matches[1].start) +
        b.toString() +
        result.substring(matches[1].end);
    result = result.substring(0, matches[0].start) +
        a.toString() +
        result.substring(matches[0].end);
    return result;
  }

  return name;
}
