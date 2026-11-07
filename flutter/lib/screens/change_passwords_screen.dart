// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../api/persons_api.dart';
import '../models/change_password_person.dart';
import '../providers/api_client_provider.dart';
import '../utils/error_utils.dart';
import '../utils/password_hash.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';

/// Administrator screen for managing every user's login credentials.
///
/// Mirrors the desktop `ChangePasswordsPage`: a list of all persons whose
/// password (and username) can be set or removed. Reachable only by the
/// administrator (guarded in `router.dart`). The server still enforces the
/// per-target anti-escalation rules.
class ChangePasswordsScreen extends ConsumerStatefulWidget {
  const ChangePasswordsScreen({super.key});

  @override
  ConsumerState<ChangePasswordsScreen> createState() =>
      _ChangePasswordsScreenState();
}

class _ChangePasswordsScreenState
    extends ConsumerState<ChangePasswordsScreen> {
  late Future<List<ChangePasswordPerson>> _future;
  final _searchCtrl = TextEditingController();
  String _filter = '';

  @override
  void initState() {
    super.initState();
    _future = ref.read(personsApiProvider).queryChangePasswords();
    _searchCtrl.addListener(
        () => setState(() => _filter = _searchCtrl.text.toLowerCase()));
  }

  @override
  void dispose() {
    _searchCtrl.dispose();
    super.dispose();
  }

  void _reload() {
    setState(() {
      _future = ref.read(personsApiProvider).queryChangePasswords();
    });
  }

  /// Opens the credential panel for [person] and reloads on changes.
  Future<void> _openPanel(ChangePasswordPerson person) async {
    await showModalBottomSheet<void>(
      context: context,
      isScrollControlled: true,
      showDragHandle: true,
      builder: (_) => Padding(
        padding: EdgeInsets.only(
          bottom: MediaQuery.of(context).viewInsets.bottom,
        ),
        child: _CredentialPanel(person: person, onChanged: _reload),
      ),
    );
  }

  List<ChangePasswordPerson> _applyFilter(
      List<ChangePasswordPerson> all) {
    if (_filter.isEmpty) return all;
    return all
        .where((p) =>
            p.displayName.toLowerCase().contains(_filter) ||
            p.username.toLowerCase().contains(_filter))
        .toList();
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/edit_passwords.jpeg',
          title: l10n.changePasswordsTitle,
        ),
      ),
      body: FutureBuilder<List<ChangePasswordPerson>>(
        future: _future,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator.adaptive());
          }
          if (snapshot.hasError) {
            return Center(
              child: Padding(
                padding: const EdgeInsets.all(24),
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    ErrorDisplay(error: snapshot.error!, l10n: l10n),
                    const SizedBox(height: 12),
                    FilledButton(
                      onPressed: _reload,
                      child: Text(l10n.retry),
                    ),
                  ],
                ),
              ),
            );
          }
          final persons = _applyFilter(snapshot.data ?? []);
          return Column(
            children: [
              Padding(
                padding: const EdgeInsets.all(12),
                child: TextField(
                  controller: _searchCtrl,
                  decoration: InputDecoration(
                    prefixIcon: const Icon(Icons.search),
                    hintText: l10n.navPersons,
                    border: const OutlineInputBorder(),
                    isDense: true,
                  ),
                ),
              ),
              Expanded(
                child: ListView.separated(
                  itemCount: persons.length,
                  separatorBuilder: (_, __) =>
                      const Divider(height: 1),
                  itemBuilder: (context, i) {
                    final p = persons[i];
                    return ListTile(
                      leading: Icon(
                        p.username.isEmpty
                            ? Icons.person_outline
                            : Icons.account_circle,
                      ),
                      title: Text(p.displayName),
                      subtitle: Text(p.username.isEmpty
                          ? l10n.changePasswordsNoUsername
                          : p.username),
                      trailing: const Icon(Icons.chevron_right),
                      onTap: () => _openPanel(p),
                    );
                  },
                ),
              ),
            ],
          );
        },
      ),
    );
  }
}

/// Bottom-sheet panel to set/remove the password and username of one person.
class _CredentialPanel extends ConsumerStatefulWidget {
  const _CredentialPanel({required this.person, required this.onChanged});

  final ChangePasswordPerson person;

  /// Called after a successful credential change so the list can refresh.
  final VoidCallback onChanged;

  @override
  ConsumerState<_CredentialPanel> createState() =>
      _CredentialPanelState();
}

class _CredentialPanelState extends ConsumerState<_CredentialPanel> {
  final _newCtrl = TextEditingController();
  final _confirmCtrl = TextEditingController();
  late final TextEditingController _usernameCtrl;

  bool? _hasPassword;
  bool _busy = false;

  @override
  void initState() {
    super.initState();
    _usernameCtrl = TextEditingController(text: widget.person.username);
    _newCtrl.addListener(() => setState(() {}));
    _confirmCtrl.addListener(() => setState(() {}));
    _loadHasPassword();
  }

  @override
  void dispose() {
    _newCtrl.dispose();
    _confirmCtrl.dispose();
    _usernameCtrl.dispose();
    super.dispose();
  }

  PersonsApi get _api => ref.read(personsApiProvider);

  Future<void> _loadHasPassword() async {
    try {
      final has = await _api.hasPassword(widget.person.id);
      if (!mounted) return;
      setState(() => _hasPassword = has);
    } catch (_) {
      if (!mounted) return;
      setState(() => _hasPassword = false);
    }
  }

  bool get _passwordValid =>
      _newCtrl.text.isNotEmpty && _newCtrl.text == _confirmCtrl.text;

  /// Runs [action], showing a snackbar with [success] or a friendly error.
  Future<void> _run(Future<void> Function() action, String success) async {
    final l10n = AppLocalizations.of(context)!;
    setState(() => _busy = true);
    try {
      await action();
      if (!mounted) return;
      widget.onChanged();
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(success)));
      setState(() => _busy = false);
    } catch (e) {
      if (!mounted) return;
      setState(() => _busy = false);
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(friendlyHttpError(e, l10n)),
        action: SnackBarAction(
          label: l10n.errorDetails,
          onPressed: () => ServerErrorDialog.show(context, e, l10n),
        ),
      ));
    }
  }

  Future<void> _changePassword(AppLocalizations l10n) async {
    if (!_passwordValid) return;
    final hash = hashPassword(_newCtrl.text);
    await _run(
      () => _api.setPassword(widget.person.id, hash),
      l10n.personPasswordSaved,
    );
    if (mounted) {
      _newCtrl.clear();
      _confirmCtrl.clear();
      setState(() => _hasPassword = true);
    }
  }

  Future<void> _deletePassword(AppLocalizations l10n) async {
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        content: Text(
            l10n.changePasswordsDeleteConfirm(widget.person.displayName)),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx, false),
            child: Text(l10n.cancel),
          ),
          TextButton(
            onPressed: () => Navigator.pop(ctx, true),
            child: Text(l10n.personPasswordRemoveButton),
          ),
        ],
      ),
    );
    if (confirmed != true) return;
    await _run(
      () => _api.removePassword(widget.person.id),
      l10n.personPasswordRemoved,
    );
    if (mounted) setState(() => _hasPassword = false);
  }

  Future<void> _saveUsername(AppLocalizations l10n) async {
    final name = _usernameCtrl.text.trim();
    if (name.isEmpty) return;
    await _run(
      () => _api.setUsername(widget.person.id, name),
      l10n.personUsernameSaved,
    );
  }

  Future<void> _removeUsername(AppLocalizations l10n) async {
    await _run(
      () => _api.removeUsername(widget.person.id),
      l10n.personUsernameRemoved,
    );
    if (mounted) _usernameCtrl.clear();
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final theme = Theme.of(context);
    final mismatch = _newCtrl.text.isNotEmpty &&
        _confirmCtrl.text.isNotEmpty &&
        _newCtrl.text != _confirmCtrl.text;
    final enabled = !_busy;
    return SingleChildScrollView(
      padding: const EdgeInsets.fromLTRB(20, 0, 20, 24),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Text(widget.person.displayName,
              style: theme.textTheme.titleLarge),
          const SizedBox(height: 20),

          // ── Username ──────────────────────────────────────────────
          Text(l10n.personUsernameLabel, style: theme.textTheme.titleMedium),
          const SizedBox(height: 8),
          TextField(
            controller: _usernameCtrl,
            enabled: enabled,
            decoration: InputDecoration(
              labelText: l10n.personUsernameLabel,
              border: const OutlineInputBorder(),
              isDense: true,
            ),
          ),
          const SizedBox(height: 8),
          Row(
            mainAxisAlignment: MainAxisAlignment.end,
            children: [
              TextButton(
                onPressed: enabled ? () => _removeUsername(l10n) : null,
                child: Text(l10n.personUsernameClearButton),
              ),
              const SizedBox(width: 8),
              FilledButton(
                onPressed: enabled && _usernameCtrl.text.trim().isNotEmpty
                    ? () => _saveUsername(l10n)
                    : null,
                child: Text(l10n.personUsernameSetButton),
              ),
            ],
          ),

          const Divider(height: 32),

          // ── Password ──────────────────────────────────────────────
          Row(
            children: [
              Text(l10n.personPasswordSection,
                  style: theme.textTheme.titleMedium),
              const SizedBox(width: 8),
              if (_hasPassword != null)
                Icon(
                  _hasPassword! ? Icons.lock : Icons.lock_open,
                  size: 18,
                  color: _hasPassword!
                      ? theme.colorScheme.primary
                      : theme.colorScheme.onSurfaceVariant,
                ),
            ],
          ),
          const SizedBox(height: 8),
          TextField(
            controller: _newCtrl,
            enabled: enabled,
            obscureText: true,
            decoration: InputDecoration(
              labelText: l10n.personPasswordNew,
              border: const OutlineInputBorder(),
              isDense: true,
            ),
          ),
          const SizedBox(height: 12),
          TextField(
            controller: _confirmCtrl,
            enabled: enabled,
            obscureText: true,
            decoration: InputDecoration(
              labelText: l10n.personPasswordConfirm,
              border: const OutlineInputBorder(),
              isDense: true,
              errorText: mismatch ? l10n.personPasswordMismatch : null,
            ),
          ),
          const SizedBox(height: 8),
          Row(
            mainAxisAlignment: MainAxisAlignment.end,
            children: [
              if (_hasPassword == true)
                TextButton(
                  onPressed:
                      enabled ? () => _deletePassword(l10n) : null,
                  style: TextButton.styleFrom(
                      foregroundColor: theme.colorScheme.error),
                  child: Text(l10n.personPasswordRemoveButton),
                ),
              const SizedBox(width: 8),
              FilledButton(
                onPressed: enabled && _passwordValid
                    ? () => _changePassword(l10n)
                    : null,
                child: _busy
                    ? const SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator.adaptive(
                            strokeWidth: 2),
                      )
                    : Text(l10n.personPasswordSetButton),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
