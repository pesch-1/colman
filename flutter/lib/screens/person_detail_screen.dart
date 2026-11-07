// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../api/persons_api.dart';
import '../models/person.dart';
import '../permission_policy.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../utils/error_utils.dart';
import '../utils/password_hash.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/class_dropdown.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';

// ─── Permission label helper ───────────────────────────────────────────────

/// Returns the localised human-readable label for a server-side
/// permission column [column] (e.g. `"edit_persons"` → `"Edit persons"`).
///
/// Falls back to [column] itself for unknown permission names so that
/// newly added server-side permissions still appear in the UI.
String _permLabel(String column, AppLocalizations l10n) {
  switch (column) {
    case 'edit_persons':
      return l10n.permEditPersons;
    case 'create_lenders':
      return l10n.permCreateLenders;
    case 'create_data_collectors':
      return l10n.permCreateDataCollectors;
    case 'collect_lendings':
      return l10n.permCollectLendings;
    case 'collect_returns':
      return l10n.permCollectReturns;
    case 'collect_acquisitions':
      return l10n.permCollectAcquisitions;
    case 'collect_liquidations':
      return l10n.permCollectLiquidations;
    case 'view_history':
      return l10n.permViewHistory;
    case 'view_persons':
      return l10n.permViewPersons;
    case 'view_items':
      return l10n.permViewItems;
    case 'edit_preferences':
      return l10n.permEditPreferences;
    case 'print_ids':
      return l10n.permPrintIds;
    case 'print_barcodes':
      return l10n.permPrintBarcodes;
    case 'edit_categories':
      return l10n.permEditCategories;
    case 'view_personal_data':
      return l10n.permViewPersonalData;
    case 'manage_label_layouts':
      return l10n.permManageLabelLayouts;
    case 'calibrate_printers':
      return l10n.permCalibratePrinters;
    case 'edit_id_layout':
      return l10n.permEditIdLayout;
    case 'print_signatures':
      return l10n.permPrintSignatures;
    case 'change_actions':
      return l10n.permChangeActions;
    case 'edit_program_settings':
      return l10n.permEditProgramSettings;
    case 'manage_classes':
      return l10n.permManageClasses;
    case 'reserve':
      return l10n.permReserve;
    case 'edit_reservations':
      return l10n.permEditReservations;
    case 'admin_permissions':
      return l10n.permAdminPermissions;
    default:
      return column;
  }
}

// ─── PersonDetailScreen ────────────────────────────────────────────────────

/// Detail and edit screen for a single person.
///
/// Presents four collapsible sections:
/// - Person form (all contact fields) with save and delete
/// - Login section (username management)
/// - Password section (argon2id password set / remove)
/// - Permissions section (per-permission checkboxes)
///
/// The edit capabilities available to the logged-in user are limited by
/// their own permissions: a non-admin user can only grant permissions
/// that they themselves hold.
class PersonDetailScreen extends ConsumerStatefulWidget {
  const PersonDetailScreen({super.key, required this.personId});

  final int personId;

  @override
  ConsumerState<PersonDetailScreen> createState() =>
      _PersonDetailScreenState();
}

class _PersonDetailScreenState extends ConsumerState<PersonDetailScreen> {
  // ── Load state ─────────────────────────────────────────────────────────────
  bool _loading = true;
  Object? _loadError;

  // ── Person ─────────────────────────────────────────────────────────────────
  Person? _person;
  bool _formBusy = false;

  // Person form controllers
  late final TextEditingController _lastnameCtrl;
  late final TextEditingController _firstnameCtrl;
  late final TextEditingController _titleCtrl;
  String _selectedClass = '';
  List<String> _classes = [];
  late final TextEditingController _barcodeCtrl;
  late final TextEditingController _emailCtrl;
  late final TextEditingController _telephoneCtrl;
  late final TextEditingController _streetCtrl;
  late final TextEditingController _zipCtrl;
  late final TextEditingController _cityCtrl;
  late final TextEditingController _countryCtrl;
  late final TextEditingController _remarksCtrl;
  late final TextEditingController _externalIdCtrl;

  // ── Username ───────────────────────────────────────────────────────────────
  String? _username;
  final _usernameCtrl = TextEditingController();
  bool _usernameBusy = false;

  // ── Password ───────────────────────────────────────────────────────────────
  bool _hasPassword = false;
  bool _passwordBusy = false;

  // ── Permissions ────────────────────────────────────────────────────────────
  /// The permission map as stored on the server (name → current value).
  Map<String, bool> _serverPerms = {};

  /// Local edits not yet saved to the server.
  Map<String, bool> _pendingPerms = {};

  /// Ordered list of permission names from `GET /api/permissions`.
  List<String> _permNames = [];

  bool _permsBusy = false;

  // ── Logged-in user context ─────────────────────────────────────────────────
  late final bool _isAdmin;
  late final Map<String, bool> _myPerms;

  /// Whether the logged-in user may manage this target, given the target's
  /// current permissions (containment invariant).  The server enforces this;
  /// the UI uses it to disable controls that would otherwise return 403.
  bool get _canManageTarget =>
      mayManageTarget(_myPerms, _serverPerms, isAdmin: _isAdmin);

  /// Whether the logged-in user is allowed to grant [column].
  ///
  /// All permissions are shown in the editor; non-grantable ones are greyed
  /// out (disabled) instead of hidden so the user can see the full picture.
  bool _mayGrant(String column) =>
      mayGrantPermission(_myPerms, column, isAdmin: _isAdmin);

  @override
  void initState() {
    super.initState();
    _lastnameCtrl = TextEditingController();
    _firstnameCtrl = TextEditingController();
    _titleCtrl = TextEditingController();
    _barcodeCtrl = TextEditingController();
    _emailCtrl = TextEditingController();
    _telephoneCtrl = TextEditingController();
    _streetCtrl = TextEditingController();
    _zipCtrl = TextEditingController();
    _cityCtrl = TextEditingController();
    _countryCtrl = TextEditingController();
    _remarksCtrl = TextEditingController();
    _externalIdCtrl = TextEditingController();

    final auth = ref.read(authProvider).value;
    _isAdmin = auth is Authenticated && auth.isAdmin;
    _myPerms = auth is Authenticated ? auth.permissions : const {};

    _loadAll();
  }

  @override
  void dispose() {
    _lastnameCtrl.dispose();
    _firstnameCtrl.dispose();
    _titleCtrl.dispose();
    _barcodeCtrl.dispose();
    _emailCtrl.dispose();
    _telephoneCtrl.dispose();
    _streetCtrl.dispose();
    _zipCtrl.dispose();
    _cityCtrl.dispose();
    _countryCtrl.dispose();
    _remarksCtrl.dispose();
    _externalIdCtrl.dispose();
    _usernameCtrl.dispose();
    super.dispose();
  }

  // ── Data loading ───────────────────────────────────────────────────────────

  /// Loads all data for the screen in parallel.
  Future<void> _loadAll() async {
    setState(() {
      _loading = true;
      _loadError = null;
    });
    try {
      final api = ref.read(personsApiProvider);
      final personFuture = api.getById(widget.personId);
      final usernameFuture = api.getUsername(widget.personId);
      final hasPasswordFuture = api.hasPassword(widget.personId);
      final permsFuture = api.getPersonPermissions(widget.personId);
      final classesFuture =
          ref.read(classesApiProvider).loadClassNames();

      final person = await personFuture;
      final username = await usernameFuture;
      final hasPassword = await hasPasswordFuture;
      final perms = await permsFuture;
      final classes = await classesFuture;

      if (!mounted) return;
      _fillControllers(person);
      setState(() {
        _person = person;
        _classes = classes;
        _username = username;
        _usernameCtrl.text = username ?? '';
        _hasPassword = hasPassword;
        _serverPerms = Map.from(perms);
        _pendingPerms = Map.from(perms);
        _permNames = perms.keys.toList();
        _loading = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _loadError = e;
        _loading = false;
      });
    }
  }

  /// Populates form controllers from [person] data.
  void _fillControllers(Person person) {
    _lastnameCtrl.text = person.lastname;
    _firstnameCtrl.text = person.firstname;
    _titleCtrl.text = person.title ?? '';
    _selectedClass = person.schoolClass ?? '';
    _barcodeCtrl.text = person.barcode ?? '';
    _emailCtrl.text = person.emails.join(', ');
    _telephoneCtrl.text = person.telephone ?? '';
    _streetCtrl.text = person.street ?? '';
    _zipCtrl.text = person.zip ?? '';
    _cityCtrl.text = person.city ?? '';
    _countryCtrl.text = person.country ?? '';
    _remarksCtrl.text = person.remarks ?? '';
    _externalIdCtrl.text = person.externalId ?? '';
  }

  // ── Person form actions ────────────────────────────────────────────────────

  /// Collects form controller values into a JSON-compatible map.
  Map<String, dynamic> _collectFormData() {
    return {
      'lastname': _lastnameCtrl.text.trim(),
      'firstname': _firstnameCtrl.text.trim(),
      if (_titleCtrl.text.trim().isNotEmpty) 'title': _titleCtrl.text.trim(),
      if (_selectedClass.isNotEmpty) 'school_class': _selectedClass,
      if (_barcodeCtrl.text.trim().isNotEmpty)
        'barcode': _barcodeCtrl.text.trim(),
      'emails': _emailCtrl.text.trim(),
      if (_telephoneCtrl.text.trim().isNotEmpty)
        'telephone': _telephoneCtrl.text.trim(),
      if (_streetCtrl.text.trim().isNotEmpty)
        'street': _streetCtrl.text.trim(),
      if (_zipCtrl.text.trim().isNotEmpty) 'zip': _zipCtrl.text.trim(),
      if (_cityCtrl.text.trim().isNotEmpty) 'city': _cityCtrl.text.trim(),
      if (_countryCtrl.text.trim().isNotEmpty)
        'country': _countryCtrl.text.trim(),
      if (_remarksCtrl.text.trim().isNotEmpty)
        'remarks': _remarksCtrl.text.trim(),
      if (_externalIdCtrl.text.trim().isNotEmpty)
        'external_id': _externalIdCtrl.text.trim(),
    };
  }

  /// Saves the person form using `PUT /api/persons/{id}`.
  ///
  /// Handles 412 (ETag conflict) with a descriptive message.
  Future<void> _save() async {
    final l10n = AppLocalizations.of(context)!;
    final etag = _person?.etag;
    if (etag == null) return;
    setState(() { _formBusy = true; });
    try {
      final api = ref.read(personsApiProvider);
      final updated =
          await api.update(widget.personId, _collectFormData(), etag);
      if (!mounted) return;
      setState(() {
        _person = updated;
        _formBusy = false;
      });
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(l10n.personSaved)));
    } catch (e) {
      if (!mounted) return;
      setState(() { _formBusy = false; });
      final msg = e.toString().contains('412')
          ? l10n.personConflict
          : l10n.personSaveFailed;
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(msg)));
    }
  }

  /// Asks for confirmation and deletes the person.
  Future<void> _delete() async {
    final l10n = AppLocalizations.of(context)!;
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(l10n.personDelete),
        content: Text(l10n.personDeleteConfirm),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: Text(
                MaterialLocalizations.of(ctx).cancelButtonLabel),
          ),
          TextButton(
            onPressed: () => Navigator.pop(ctx, true),
            child: Text(l10n.personDelete),
          ),
        ],
      ),
    );
    if (confirmed != true || !mounted) return;
    try {
      await ref.read(personsApiProvider).delete(widget.personId);
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personDeleteSuccess)));
      Navigator.of(context).pop();
    } catch (_) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personDeleteFailed)));
    }
  }

  // ── Username actions ───────────────────────────────────────────────────────

  /// Sets the username to the current value of [_usernameCtrl].
  Future<void> _setUsername() async {
    final l10n = AppLocalizations.of(context)!;
    final newName = _usernameCtrl.text.trim();
    if (newName.isEmpty) return;
    setState(() { _usernameBusy = true; });
    try {
      await ref.read(personsApiProvider).setUsername(widget.personId, newName);
      if (!mounted) return;
      setState(() {
        _username = newName;
        _usernameBusy = false;
      });
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personUsernameSaved)));
    } catch (e) {
      if (!mounted) return;
      setState(() { _usernameBusy = false; });
      final isTaken = e.toString().contains('409') ||
          e.toString().toLowerCase().contains('conflict');
      if(isTaken) {
        ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(content: Text(l10n.personUsernameTaken)));
      } else {
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(
          content: Text(friendlyHttpError(e, l10n)),
          action: SnackBarAction(
            label: l10n.errorDetails,
            onPressed: () {
              if(!mounted) return;
              ServerErrorDialog.show(context, e, l10n);
            },
          ),
        ));
      }
    }
  }

  /// Removes the current username.
  Future<void> _removeUsername() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() { _usernameBusy = true; });
    try {
      await ref
          .read(personsApiProvider)
          .removeUsername(widget.personId);
      if (!mounted) return;
      setState(() {
        _username = null;
        _usernameCtrl.clear();
        _usernameBusy = false;
      });
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personUsernameRemoved)));
    } catch (e) {
      if (!mounted) return;
      setState(() { _usernameBusy = false; });
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(friendlyHttpError(e, l10n)),
        action: SnackBarAction(
          label: l10n.errorDetails,
          onPressed: () {
            if (!mounted) return;
            ServerErrorDialog.show(context, e, l10n);
          },
        ),
      ));
    }
  }

  // ── Password actions ───────────────────────────────────────────────────────

  /// Opens the set-password dialog.
  Future<void> _showPasswordDialog() async {
    final l10n = AppLocalizations.of(context)!;
    final saved = await showDialog<bool>(
      context: context,
      builder: (_) => _SetPasswordDialog(
        personId: widget.personId,
        personsApi: ref.read(personsApiProvider),
        l10n: l10n,
      ),
    );
    if (saved == true && mounted) {
      setState(() { _hasPassword = true; });
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personPasswordSaved)));
    }
  }

  /// Removes the stored password hash.
  Future<void> _removePassword() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() { _passwordBusy = true; });
    try {
      await ref
          .read(personsApiProvider)
          .removePassword(widget.personId);
      if (!mounted) return;
      setState(() {
        _hasPassword = false;
        _passwordBusy = false;
      });
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personPasswordRemoved)));
    } catch (e) {
      if (!mounted) return;
      setState(() { _passwordBusy = false; });
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(friendlyHttpError(e, l10n)),
        action: SnackBarAction(
          label: l10n.errorDetails,
          onPressed: () {
            if (!mounted) return;
            ServerErrorDialog.show(context, e, l10n);
          },
        ),
      ));
    }
  }

  // ── Permission actions ─────────────────────────────────────────────────────

  /// Saves all pending permission changes to the server.
  ///
  /// Only permissions that changed relative to [_serverPerms] are sent.
  Future<void> _applyPermissions() async {
    final l10n = AppLocalizations.of(context)!;
    setState(() { _permsBusy = true; });
    try {
      final api = ref.read(personsApiProvider);
      for (final entry in _pendingPerms.entries) {
        if (_serverPerms[entry.key] != entry.value) {
          await api.setPermission(
              widget.personId, entry.key, entry.value);
        }
      }
      if (!mounted) return;
      setState(() {
        _serverPerms = Map.from(_pendingPerms);
        _permsBusy = false;
      });
      ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(l10n.personPermissionsSaved)));
    } catch (e) {
      if (!mounted) return;
      // Reset the pending state so the UI reflects what is actually stored.
      setState(() {
        _pendingPerms = Map.from(_serverPerms);
        _permsBusy = false;
      });
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(friendlyHttpError(e, l10n)),
        action: SnackBarAction(
          label: l10n.errorDetails,
          onPressed: () {
            if (!mounted) return;
            ServerErrorDialog.show(context, e, l10n);
          },
        ),
      ));
    }
  }

  // ── Build ──────────────────────────────────────────────────────────────────

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/edit_persons.jpeg',
          title: _person?.displayName ?? l10n.personDetailFallbackTitle,
        ),
        actions: [
          if (_person != null && _canManageTarget)
            IconButton(
              icon: const Icon(Icons.delete_outline),
              tooltip: l10n.personDelete,
              onPressed: _formBusy ? null : _delete,
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
    if (_loadError != null) {
      return Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            ErrorDisplay(error: _loadError!, l10n: l10n),
            TextButton(
              onPressed: _loadAll,
              child: Text(l10n.retry),
            ),
          ],
        ),
      );
    }
    return SingleChildScrollView(
      padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          _buildPersonFormCard(l10n),
          const SizedBox(height: 16),
          _buildLoginCard(l10n),
          const SizedBox(height: 16),
          _buildPasswordCard(l10n),
          const SizedBox(height: 16),
          _buildPermissionsCard(l10n),
          const SizedBox(height: 32),
        ],
      ),
    );
  }

  // ── Person form card ───────────────────────────────────────────────────────

  Widget _buildPersonFormCard(AppLocalizations l10n) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text(
              l10n.personSectionDetails,
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 16),
            _field(_lastnameCtrl, l10n.personFormLastname,
                required: true, busy: _formBusy),
            _field(_firstnameCtrl, l10n.personFormFirstname,
                required: true, busy: _formBusy),
            _field(_titleCtrl, l10n.personFormTitle, busy: _formBusy),
            ClassDropdown(
              label: l10n.personFormSchoolClass,
              classes: _classes,
              value: _selectedClass,
              enabled: !_formBusy,
              onChanged: (v) => setState(() => _selectedClass = v),
            ),
            _field(_barcodeCtrl, l10n.personFormBarcode,
                busy: _formBusy),
            _field(_emailCtrl, l10n.personFormEmail,
                busy: _formBusy,
                keyboardType: TextInputType.emailAddress),
            _field(_telephoneCtrl, l10n.personFormTelephone,
                busy: _formBusy,
                keyboardType: TextInputType.phone),
            _field(_streetCtrl, l10n.personFormStreet, busy: _formBusy),
            Row(
              children: [
                Expanded(
                    flex: 2,
                    child: _field(_zipCtrl, l10n.personFormZip,
                        busy: _formBusy)),
                const SizedBox(width: 8),
                Expanded(
                    flex: 5,
                    child: _field(_cityCtrl, l10n.personFormCity,
                        busy: _formBusy)),
              ],
            ),
            _field(_countryCtrl, l10n.personFormCountry,
                busy: _formBusy),
            _field(_remarksCtrl, l10n.personFormRemarks,
                busy: _formBusy, maxLines: 3),
            _field(_externalIdCtrl, l10n.personFormExternalId,
                busy: _formBusy),
            const SizedBox(height: 16),
            Align(
              alignment: Alignment.centerRight,
              child: FilledButton(
                onPressed: _formBusy ||
                        !_canManageTarget ||
                        _lastnameCtrl.text.trim().isEmpty ||
                        _firstnameCtrl.text.trim().isEmpty
                    ? null
                    : _save,
                child: _formBusy
                    ? const SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator.adaptive(
                            strokeWidth: 2),
                      )
                    : Text(l10n.personSave),
              ),
            ),
          ],
        ),
      ),
    );
  }

  /// Creates a labelled text field with consistent style.
  Widget _field(
    TextEditingController ctrl,
    String label, {
    bool required = false,
    bool busy = false,
    TextInputType? keyboardType,
    int maxLines = 1,
  }) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 12),
      child: TextField(
        controller: ctrl,
        enabled: !busy,
        keyboardType: keyboardType,
        maxLines: maxLines,
        decoration: InputDecoration(
          labelText: required ? '$label *' : label,
          border: const OutlineInputBorder(),
          isDense: true,
        ),
        onChanged: (_) => setState(() {}),
      ),
    );
  }

  // ── Login / username card ──────────────────────────────────────────────────

  Widget _buildLoginCard(AppLocalizations l10n) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              l10n.personLoginSection,
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: _usernameCtrl,
                    enabled: !_usernameBusy && _canManageTarget,
                    decoration: InputDecoration(
                      labelText: l10n.personUsernameLabel,
                      hintText: l10n.personUsernameNotSet,
                      border: const OutlineInputBorder(),
                      isDense: true,
                    ),
                  ),
                ),
                const SizedBox(width: 8),
                _usernameBusy
                    ? const Padding(
                        padding: EdgeInsets.all(8),
                        child: SizedBox(
                          width: 20,
                          height: 20,
                          child: CircularProgressIndicator.adaptive(
                              strokeWidth: 2),
                        ),
                      )
                    : TextButton(
                        onPressed: _usernameCtrl.text.trim().isEmpty ||
                                !_canManageTarget
                            ? null
                            : _setUsername,
                        child: Text(l10n.personUsernameSetButton),
                      ),
              ],
            ),
            if (_username != null &&
                _username!.isNotEmpty &&
                _canManageTarget) ...[
              const SizedBox(height: 8),
              TextButton.icon(
                onPressed: _usernameBusy ? null : _removeUsername,
                icon: const Icon(Icons.person_remove_outlined, size: 18),
                label: Text(l10n.personUsernameClearButton),
                style: TextButton.styleFrom(
                  foregroundColor:
                      Theme.of(context).colorScheme.error,
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }

  // ── Password card ──────────────────────────────────────────────────────────

  Widget _buildPasswordCard(AppLocalizations l10n) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              l10n.personPasswordSection,
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Icon(
                  _hasPassword ? Icons.lock : Icons.lock_open,
                  color: _hasPassword
                      ? Theme.of(context).colorScheme.primary
                      : Theme.of(context).colorScheme.onSurfaceVariant,
                  size: 20,
                ),
                const SizedBox(width: 8),
                Text(
                  _hasPassword
                      ? l10n.personPasswordIsSet
                      : l10n.personPasswordNotSet,
                  style: Theme.of(context).textTheme.bodyMedium,
                ),
                const Spacer(),
                if (_passwordBusy)
                  const SizedBox(
                    width: 20,
                    height: 20,
                    child: CircularProgressIndicator.adaptive(
                        strokeWidth: 2),
                  )
                else ...[
                  TextButton(
                    onPressed: _canManageTarget ? _showPasswordDialog : null,
                    child: Text(l10n.personPasswordSetButton),
                  ),
                  if (_hasPassword && _canManageTarget) ...[
                    const SizedBox(width: 4),
                    TextButton(
                      onPressed: _removePassword,
                      style: TextButton.styleFrom(
                        foregroundColor:
                            Theme.of(context).colorScheme.error,
                      ),
                      child: Text(l10n.personPasswordRemoveButton),
                    ),
                  ],
                ],
              ],
            ),
          ],
        ),
      ),
    );
  }

  // ── Permissions card ───────────────────────────────────────────────────────

  Widget _buildPermissionsCard(AppLocalizations l10n) {
    if (_permNames.isEmpty) return const SizedBox.shrink();
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text(
              l10n.personPermissionsSection,
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 8),
            ..._permNames.map((perm) {
              // A permission is editable only when the user may grant it and
              // may manage this target; otherwise the row is shown greyed out.
              final canToggle = _mayGrant(perm) && _canManageTarget;
              return CheckboxListTile(
                title: Text(_permLabel(perm, l10n)),
                value: _pendingPerms[perm] ?? false,
                enabled: !_permsBusy && canToggle,
                dense: true,
                controlAffinity: ListTileControlAffinity.leading,
                onChanged: canToggle && !_permsBusy
                    ? (v) => setState(() {
                          _pendingPerms[perm] = v ?? false;
                        })
                    : null,
              );
            }),
            const SizedBox(height: 8),
            Align(
              alignment: Alignment.centerRight,
              child: FilledButton(
                onPressed: _permsBusy || !_canManageTarget
                    ? null
                    : _applyPermissions,
                child: _permsBusy
                    ? const SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator.adaptive(
                            strokeWidth: 2),
                      )
                    : Text(l10n.personPermissionsApply),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

// ─── Set-password dialog ───────────────────────────────────────────────────

/// Dialog for setting or changing the login password of a person.
///
/// Hashes the entered password with Argon2id (matching server parameters)
/// before sending it to `POST /api/persons/{id}/password`.
class _SetPasswordDialog extends StatefulWidget {
  const _SetPasswordDialog({
    required this.personId,
    required this.personsApi,
    required this.l10n,
  });

  final int personId;
  final PersonsApi personsApi;
  final AppLocalizations l10n;

  @override
  State<_SetPasswordDialog> createState() => _SetPasswordDialogState();
}

class _SetPasswordDialogState extends State<_SetPasswordDialog> {
  final _newCtrl = TextEditingController();
  final _confirmCtrl = TextEditingController();
  bool _busy = false;
  @override
  void initState() {
    super.initState();
    _newCtrl.addListener(_onChanged);
    _confirmCtrl.addListener(_onChanged);
  }

  @override
  void dispose() {
    _newCtrl.dispose();
    _confirmCtrl.dispose();
    super.dispose();
  }

  void _onChanged() => setState(() {});

  bool get _valid =>
      _newCtrl.text.isNotEmpty &&
      _newCtrl.text == _confirmCtrl.text;

  /// Hashes the password and posts it to the server.
  Future<void> _save() async {
    final l10n = widget.l10n;
    if (!_valid) return;
    setState(() { _busy = true; });
    try {
      final hash = hashPassword(_newCtrl.text);
      await widget.personsApi.setPassword(widget.personId, hash);
      if (!mounted) return;
      Navigator.pop(context, true);
    } catch (e) {
      if (!mounted) return;
      setState(() { _busy = false; });
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = widget.l10n;
    final mismatch = _newCtrl.text.isNotEmpty &&
        _confirmCtrl.text.isNotEmpty &&
        _newCtrl.text != _confirmCtrl.text;
    return AlertDialog(
      title: Text(l10n.personPasswordSetTitle),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          TextField(
            controller: _newCtrl,
            enabled: !_busy,
            obscureText: true,
            autofocus: true,
            decoration: InputDecoration(
              labelText: l10n.personPasswordNew,
              border: const OutlineInputBorder(),
            ),
            textInputAction: TextInputAction.next,
          ),
          const SizedBox(height: 12),
          TextField(
            controller: _confirmCtrl,
            enabled: !_busy,
            obscureText: true,
            decoration: InputDecoration(
              labelText: l10n.personPasswordConfirm,
              border: const OutlineInputBorder(),
              errorText: mismatch ? l10n.personPasswordMismatch : null,
            ),
            onSubmitted: _valid && !_busy ? (_) => _save() : null,
          ),
        ],
      ),
      actions: [
        TextButton(
          onPressed: _busy ? null : () => Navigator.pop(context),
          child: Text(
              MaterialLocalizations.of(context).cancelButtonLabel),
        ),
        TextButton(
          onPressed: _valid && !_busy ? _save : null,
          child: _busy
              ? const SizedBox(
                  width: 16,
                  height: 16,
                  child:
                      CircularProgressIndicator.adaptive(strokeWidth: 2),
                )
              : Text(l10n.personSave),
        ),
      ],
    );
  }
}
