// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:pdf/widgets.dart' as pw;
import 'package:printing/printing.dart';

import '../models/due_lending_row.dart';
import '../models/person.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/extend_lending_dialog.dart';
import '../widgets/person_search_field.dart';
import '../widgets/server_error_dialog.dart';
import 'return_screen.dart' show ReturnInitialData;

/// Screen listing all currently open (unreturned) lendings.
///
/// Mirrors the Qt REST client's "Open Lendings" view: filterable by person
/// and by school class, with a print button that generates a PDF table.
///
/// When [lockedPersonId] is provided the screen operates in personal mode:
/// it shows only that person's open lendings and hides the filter controls.
class OpenLendingsScreen extends ConsumerStatefulWidget {
  const OpenLendingsScreen({super.key, this.lockedPersonId});

  /// When set, restricts the view to this person and hides filter controls.
  final int? lockedPersonId;

  @override
  ConsumerState<OpenLendingsScreen> createState() => _OpenLendingsScreenState();
}

class _OpenLendingsScreenState extends ConsumerState<OpenLendingsScreen> {
  List<DueLendingRow> _rows = [];
  bool _loading = false;
  Object? _error;

  List<String> _classes = [];
  bool _classesLoading = false;

  Person? _selectedPerson;
  String? _selectedClass;

  // Incremented to force PersonSearchField to reset its internal controller.
  int _personSearchKey = 0;

  /// True while rows are being selected to extend their due dates.
  bool _selecting = false;

  /// Lending ids selected in [_selecting] mode.
  final Set<int> _selected = {};

  bool get _personalMode => widget.lockedPersonId != null;

  @override
  void initState() {
    super.initState();
    if (!_personalMode) _loadClasses();
    _load();
  }

  Future<void> _loadClasses() async {
    if (!mounted) return;
    setState(() {
      _classesLoading = true;
    });
    try {
      final names = await ref.read(classesApiProvider).loadClassNames();
      if (!mounted) return;
      setState(() {
        _classes = names;
        _classesLoading = false;
      });
    } catch (_) {
      if (!mounted) return;
      setState(() {
        _classesLoading = false;
      });
    }
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
          .getDue(
            personId: widget.lockedPersonId ?? _selectedPerson?.id,
            schoolClass: _personalMode ? null : _selectedClass,
          );
      if (!mounted) return;
      setState(() {
        _rows = rows;
        _loading = false;
        // drop selections of lendings that are no longer open or shown
        final ids = rows.map((r) => r.lendingId).toSet();
        _selected.removeWhere((id) => !ids.contains(id));
      });
    } catch (e) {
      if (!mounted) return;
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
      setState(() {
        _rows = [];
        _loading = false;
        _error = e;
      });
    }
  }

  void _clearPerson() {
    setState(() {
      _selectedPerson = null;
      _personSearchKey++;
    });
    _load();
  }

  // ── Extend due dates ──────────────────────────────────────────────────────

  /// Enters selection mode, optionally with [lendingId] already selected.
  void _startSelecting([int? lendingId]) {
    setState(() {
      _selecting = true;
      _selected.clear();
      if (lendingId != null) _selected.add(lendingId);
    });
  }

  /// Leaves selection mode and clears the selection.
  void _stopSelecting() {
    setState(() {
      _selecting = false;
      _selected.clear();
    });
  }

  /// Toggles whether [lendingId] is selected.
  void _toggle(int lendingId) {
    setState(() {
      if (!_selected.remove(lendingId)) _selected.add(lendingId);
    });
  }

  /// Selects all shown rows, or none when all are already selected.
  void _toggleAll() {
    setState(() {
      if (_selected.length == _rows.length) {
        _selected.clear();
      } else {
        _selected.addAll(_rows.map((r) => r.lendingId));
      }
    });
  }

  /// Asks for the new due date of the selected lendings and applies it.
  ///
  /// The selected rows are already the user's choice, so the dialog only
  /// asks for the date.  On success selection mode ends and the list is
  /// reloaded to show the new due dates.
  Future<void> _extendSelected() async {
    final rows = _rows.where((r) => _selected.contains(r.lendingId)).toList();
    final extended = await showExtendSelectedLendingsDialog(context, rows);
    if (!extended || !mounted) return;
    _stopSelecting();
    await _load();
  }

  Future<void> _print(AppLocalizations l10n) async {
    final rows = _rows;
    final font = await PdfGoogleFonts.notoSansRegular();
    final fontBold = await PdfGoogleFonts.notoSansBold();
    final doc = pw.Document();
    doc.addPage(
      pw.MultiPage(
        theme: pw.ThemeData.withFont(base: font, bold: fontBold),
        build: (ctx) {
          final filterParts = <String>[
            if (_selectedPerson != null)
              '${l10n.openLendingsPersonLabel}: '
                  '${_selectedPerson!.firstname} ${_selectedPerson!.lastname}',
            if (_selectedClass != null)
              '${l10n.openLendingsLabelClass}: $_selectedClass',
          ];
          return [
            pw.Header(level: 0, child: pw.Text(l10n.navOpenLendings)),
            if (filterParts.isNotEmpty)
              pw.Padding(
                padding: const pw.EdgeInsets.only(bottom: 8),
                child: pw.Text(filterParts.join('   ')),
              ),
            pw.TableHelper.fromTextArray(
              headers: [
                '#',
                l10n.openLendingsColDueDate,
                l10n.openLendingsColBorrower,
                l10n.openLendingsColCount,
                l10n.openLendingsColItem,
              ],
              data: List.generate(rows.length, (i) {
                final r = rows[i];
                final due = r.duedate.year > 0 ? _fmtDate(r.duedate) : '—';
                return [
                  '${i + 1}',
                  due,
                  r.personName,
                  '${r.outstandingCount}',
                  r.itemName,
                ];
              }),
            ),
          ];
        },
      ),
    );
    await Printing.layoutPdf(onLayout: (_) async => doc.save());
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['lendings', 'items', 'persons'], _load);
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canReturn =
        auth is Authenticated && auth.hasPermission('collect_returns');
    final canExtend =
        auth is Authenticated && auth.hasPermission('collect_lendings');
    return PopScope(
      // Back first leaves selection mode instead of the screen.
      canPop: !_selecting,
      onPopInvokedWithResult: (didPop, _) {
        if (!didPop && _selecting) _stopSelecting();
      },
      child: Scaffold(
        appBar: _selecting
            ? _buildSelectionAppBar(l10n)
            : AppBar(
                title: AppBarTitle(
                  icon: 'assets/icons/waiting.jpeg',
                  title: _personalMode
                      ? l10n.navMyLendings
                      : l10n.navOpenLendings,
                ),
                actions: [
                  IconButton(
                    icon: const Icon(Icons.print),
                    tooltip: l10n.openLendingsPrint,
                    onPressed: (_loading || _rows.isEmpty)
                        ? null
                        : () => _print(l10n),
                  ),
                ],
              ),
        body: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            if (!_personalMode) _buildFilterBar(l10n),
            if (_loading) const LinearProgressIndicator(),
            if (_selecting)
              Padding(
                padding: const EdgeInsets.fromLTRB(16, 8, 16, 0),
                child: Text(
                  l10n.openLendingsSelectHint,
                  style: Theme.of(context).textTheme.bodySmall,
                ),
              ),
            const Divider(height: 1),
            Expanded(child: _buildBody(l10n, canReturn, canExtend)),
          ],
        ),
        bottomNavigationBar: canExtend && (_selecting || _rows.isNotEmpty)
            ? _buildExtendBar(l10n)
            : null,
      ),
    );
  }

  /// Bottom bar with the labelled "extend due date" buttons.
  ///
  /// Outside selection mode a single button starts selecting lendings; in
  /// selection mode "cancel" and "extend (n)" are offered, so the next step
  /// is always visible, also on narrow phone screens.
  Widget _buildExtendBar(AppLocalizations l10n) {
    return SafeArea(
      child: Padding(
        padding: const EdgeInsets.fromLTRB(12, 8, 12, 8),
        child: _selecting
            ? Row(
                children: [
                  OutlinedButton(
                    onPressed: _stopSelecting,
                    child: Text(
                      MaterialLocalizations.of(context).cancelButtonLabel,
                    ),
                  ),
                  const SizedBox(width: 12),
                  Expanded(
                    child: FilledButton.icon(
                      onPressed: _selected.isEmpty ? null : _extendSelected,
                      icon: const Icon(Icons.update),
                      label: Text(
                        l10n.openLendingsExtendSelected(_selected.length),
                      ),
                    ),
                  ),
                ],
              )
            : FilledButton.tonalIcon(
                onPressed: _loading ? null : _startSelecting,
                icon: const Icon(Icons.update),
                label: Text(l10n.extendDueDateButton),
              ),
      ),
    );
  }

  /// App bar shown while selecting lendings to extend.
  PreferredSizeWidget _buildSelectionAppBar(AppLocalizations l10n) {
    final allSelected = _rows.isNotEmpty && _selected.length == _rows.length;
    return AppBar(
      leading: IconButton(
        icon: const Icon(Icons.close),
        tooltip: MaterialLocalizations.of(context).cancelButtonLabel,
        onPressed: _stopSelecting,
      ),
      title: Text(l10n.openLendingsSelectedCount(_selected.length)),
      actions: [
        IconButton(
          icon: Icon(allSelected ? Icons.deselect : Icons.select_all),
          tooltip: allSelected
              ? l10n.openLendingsDeselectAll
              : l10n.openLendingsSelectAll,
          onPressed: _rows.isEmpty ? null : _toggleAll,
        ),
      ],
    );
  }

  Widget _buildFilterBar(AppLocalizations l10n) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(12, 8, 12, 8),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Expanded(
                child: PersonSearchField(
                  key: ValueKey(_personSearchKey),
                  personsApi: ref.read(personsApiProvider),
                  labelText: l10n.labelPersonSearch,
                  onSelected: (p) {
                    setState(() {
                      _selectedPerson = p;
                    });
                    _load();
                  },
                ),
              ),
              if (_selectedPerson != null)
                Padding(
                  padding: const EdgeInsets.only(top: 4),
                  child: IconButton(
                    icon: const Icon(Icons.clear),
                    tooltip: l10n.openLendingsClearFilter,
                    onPressed: _clearPerson,
                  ),
                ),
            ],
          ),
          const SizedBox(height: 8),
          _buildClassDropdown(l10n),
        ],
      ),
    );
  }

  Widget _buildClassDropdown(AppLocalizations l10n) {
    if (_classesLoading) {
      return const LinearProgressIndicator();
    }
    return DropdownButtonFormField<String?>(
      initialValue: _selectedClass,
      decoration: InputDecoration(
        labelText: l10n.openLendingsLabelClass,
        border: const OutlineInputBorder(),
        isDense: true,
      ),
      items: [
        DropdownMenuItem<String?>(
          value: null,
          child: Text(l10n.openLendingsAllClasses),
        ),
        ..._classes.map(
          (cls) => DropdownMenuItem<String?>(value: cls, child: Text(cls)),
        ),
      ],
      onChanged: (value) {
        setState(() {
          _selectedClass = value;
        });
        _load();
      },
    );
  }

  Widget _buildBody(AppLocalizations l10n, bool canReturn, bool canExtend) {
    if (_loading && _rows.isEmpty) {
      return const SizedBox.shrink();
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
          final dueStr = row.duedate.year > 0
              ? '  ·  ${l10n.labelDueDate}: ${_fmtDate(row.duedate)}'
              : '';
          if (_selecting) {
            return CheckboxListTile(
              value: _selected.contains(row.lendingId),
              onChanged: (_) => _toggle(row.lendingId),
              controlAffinity: ListTileControlAffinity.leading,
              title: Text(row.itemName),
              subtitle: Text('${row.personName}$dueStr'),
              secondary: Text(
                l10n.lendingDueOutstanding(row.outstandingCount),
                style: const TextStyle(fontSize: 12),
              ),
            );
          }
          return ListTile(
            leading: const Icon(Icons.book_outlined, color: Colors.blue),
            title: Text(row.itemName),
            subtitle: Text('${row.personName}$dueStr'),
            trailing: Text(
              l10n.lendingDueOutstanding(row.outstandingCount),
              style: const TextStyle(fontSize: 12),
            ),
            // long press starts selecting lendings to extend
            onLongPress: canExtend
                ? () => _startSelecting(row.lendingId)
                : null,
            onTap: canReturn
                ? () async {
                    await ctx.push(
                      '/return',
                      extra: ReturnInitialData(
                        itemId: row.itemId,
                        personId: row.personId,
                      ),
                    );
                    if (mounted) _load();
                  }
                : null,
          );
        },
      ),
    );
  }
}

String _fmtDate(DateTime d) => d.toIso8601String().substring(0, 10);
