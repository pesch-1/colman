// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../api/persons_api.dart';
import '../models/person.dart';
import '../permission_policy.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/class_dropdown.dart';
import '../widgets/error_display.dart';

/// Searchable list of persons with create, edit, and delete functionality.
///
/// Only accessible to users holding the `edit_persons` permission; the
/// router redirects unauthorized users to the home screen before this
/// widget is even mounted.
class PersonsScreen extends ConsumerStatefulWidget {
  const PersonsScreen({super.key});

  @override
  ConsumerState<PersonsScreen> createState() => _PersonsScreenState();
}

class _PersonsScreenState extends ConsumerState<PersonsScreen> {
  List<Person> _persons = [];
  List<String> _classes = [];
  bool _loading = false;
  Object? _error;
  final _searchCtrl = TextEditingController();
  Timer? _debounce;

  @override
  void initState() {
    super.initState();
    _load('');
    _loadClasses();
  }

  Future<void> _loadClasses() async {
    try {
      final classes = await ref.read(classesApiProvider).loadClassNames();
      if (mounted) {
        setState(() => _classes = classes);
      }
    } catch (_) {
      // Class list is optional; fall back to empty (free-text only).
    }
  }

  @override
  void dispose() {
    _debounce?.cancel();
    _searchCtrl.dispose();
    super.dispose();
  }

  /// Fetches persons matching [query] and updates the list.
  Future<void> _load(String query) async {
    if (!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final persons = await ref.read(personsApiProvider).search(query);
      if (!mounted) return;
      setState(() {
        _persons = persons;
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

  void _onSearchChanged(String query) {
    _debounce?.cancel();
    _debounce = Timer(const Duration(milliseconds: 300), () => _load(query));
  }

  /// Opens the create-person dialog and navigates to the detail screen
  /// on success so the user can set credentials and permissions immediately.
  Future<void> _showCreateDialog() async {
    final l10n = AppLocalizations.of(context)!;
    final api = ref.read(personsApiProvider);
    final created = await showDialog<Person>(
      context: context,
      builder: (_) =>
          _CreatePersonDialog(personsApi: api, l10n: l10n, classes: _classes),
    );
    if (created != null && mounted) {
      await context.push('/persons/${created.id}');
      if (mounted) {
        _load(_searchCtrl.text);
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const [
      'persons',
      'classes',
    ], () => _load(_searchCtrl.text));
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canEdit =
        auth is Authenticated &&
        mayManagePersons(auth.permissions, isAdmin: auth.isAdmin);
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/edit_persons.jpeg',
          title: l10n.personsTitle,
        ),
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 12, 12, 8),
            child: TextField(
              controller: _searchCtrl,
              decoration: InputDecoration(
                hintText: l10n.personsSearchHint,
                prefixIcon: const Icon(Icons.search),
                border: const OutlineInputBorder(),
                isDense: true,
                contentPadding: const EdgeInsets.symmetric(vertical: 10),
              ),
              onChanged: _onSearchChanged,
            ),
          ),
          Expanded(child: _buildBody(l10n)),
        ],
      ),
      floatingActionButton: canEdit
          ? FloatingActionButton(
              onPressed: _showCreateDialog,
              tooltip: l10n.personsAddTooltip,
              child: const Icon(Icons.person_add),
            )
          : null,
    );
  }

  Widget _buildBody(AppLocalizations l10n) {
    if (_loading && _persons.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (_persons.isEmpty) {
      return Center(child: Text(l10n.personsNoResults));
    }
    return RefreshIndicator(
      onRefresh: () => _load(_searchCtrl.text),
      child: ListView.separated(
        itemCount: _persons.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (context, i) {
          final p = _persons[i];
          return _PersonTile(
            person: p,
            onTap: () async {
              await context.push('/persons/${p.id}');
              if (mounted) {
                _load(_searchCtrl.text);
              }
            },
          );
        },
      ),
    );
  }
}

// ─── Person list tile ──────────────────────────────────────────────────────

class _PersonTile extends StatelessWidget {
  const _PersonTile({required this.person, required this.onTap});

  final Person person;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final parts = <String>[
      if (person.username != null && person.username!.isNotEmpty)
        '@${person.username}',
      if (person.schoolClass != null && person.schoolClass!.isNotEmpty)
        person.schoolClass!,
    ];
    return ListTile(
      leading: const CircleAvatar(child: Icon(Icons.person)),
      title: Text(person.displayName),
      subtitle: parts.isNotEmpty
          ? Text(
              parts.join(' · '),
              style: Theme.of(context).textTheme.bodySmall,
            )
          : null,
      trailing: const Icon(Icons.chevron_right),
      onTap: onTap,
    );
  }
}

// ─── Create dialog ─────────────────────────────────────────────────────────

/// Modal dialog for creating a new person with the minimum required fields.
///
/// Only last name and first name are collected here; all other attributes
/// (contact data, username, password, permissions) are edited in
/// [PersonDetailScreen] immediately after creation.
class _CreatePersonDialog extends StatefulWidget {
  const _CreatePersonDialog({
    required this.personsApi,
    required this.l10n,
    required this.classes,
  });

  final PersonsApi personsApi;
  final AppLocalizations l10n;
  final List<String> classes;

  @override
  State<_CreatePersonDialog> createState() => _CreatePersonDialogState();
}

class _CreatePersonDialogState extends State<_CreatePersonDialog> {
  final _lastnameCtrl = TextEditingController();
  final _firstnameCtrl = TextEditingController();
  String _selectedClass = '';
  bool _busy = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    _lastnameCtrl.addListener(_onChanged);
    _firstnameCtrl.addListener(_onChanged);
  }

  @override
  void dispose() {
    _lastnameCtrl.dispose();
    _firstnameCtrl.dispose();
    super.dispose();
  }

  void _onChanged() => setState(() {});

  bool get _valid =>
      _lastnameCtrl.text.trim().isNotEmpty &&
      _firstnameCtrl.text.trim().isNotEmpty;

  Future<void> _create() async {
    setState(() {
      _busy = true;
      _error = null;
    });
    try {
      final person = await widget.personsApi.create({
        'lastname': _lastnameCtrl.text.trim(),
        'firstname': _firstnameCtrl.text.trim(),
        'emails': '',
        if (_selectedClass.isNotEmpty) 'school_class': _selectedClass,
      });
      if (!mounted) return;
      Navigator.pop(context, person);
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
      title: Text(l10n.personCreateTitle),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          TextField(
            controller: _lastnameCtrl,
            enabled: !_busy,
            autofocus: true,
            decoration: InputDecoration(
              labelText: l10n.personFormLastname,
              border: const OutlineInputBorder(),
            ),
            textInputAction: TextInputAction.next,
          ),
          const SizedBox(height: 12),
          TextField(
            controller: _firstnameCtrl,
            enabled: !_busy,
            decoration: InputDecoration(
              labelText: l10n.personFormFirstname,
              border: const OutlineInputBorder(),
            ),
            onSubmitted: _valid && !_busy ? (_) => _create() : null,
          ),
          if (widget.classes.isNotEmpty) ...[
            const SizedBox(height: 12),
            ClassDropdown(
              label: l10n.personFormSchoolClass,
              classes: widget.classes,
              value: _selectedClass,
              enabled: !_busy,
              onChanged: (v) => setState(() => _selectedClass = v),
            ),
          ],
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
          onPressed: _valid && !_busy ? _create : null,
          child: _busy
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child: CircularProgressIndicator.adaptive(strokeWidth: 2),
                )
              : Text(l10n.personCreateTitle),
        ),
      ],
    );
  }
}
