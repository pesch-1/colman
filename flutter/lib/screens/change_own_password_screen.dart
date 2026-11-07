// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../providers/api_client_provider.dart';
import '../utils/password_hash.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/server_error_dialog.dart';

/// Screen letting any signed-in user change their own login password.
///
/// Mirrors the desktop `ChangePasswordPage`: the user must enter their current
/// password (verified server-side via `POST /api/me/password`) and a new
/// password twice. A wrong current password locks the fields for three
/// seconds, matching the desktop behaviour.
class ChangeOwnPasswordScreen extends ConsumerStatefulWidget {
  const ChangeOwnPasswordScreen({super.key});

  @override
  ConsumerState<ChangeOwnPasswordScreen> createState() =>
      _ChangeOwnPasswordScreenState();
}

class _ChangeOwnPasswordScreenState
    extends ConsumerState<ChangeOwnPasswordScreen> {
  final _currentCtrl = TextEditingController();
  final _newCtrl = TextEditingController();
  final _confirmCtrl = TextEditingController();
  bool _busy = false;
  bool _locked = false;
  String? _error;
  Timer? _lockTimer;

  @override
  void initState() {
    super.initState();
    _currentCtrl.addListener(_onChanged);
    _newCtrl.addListener(_onChanged);
    _confirmCtrl.addListener(_onChanged);
  }

  @override
  void dispose() {
    _lockTimer?.cancel();
    _currentCtrl.dispose();
    _newCtrl.dispose();
    _confirmCtrl.dispose();
    super.dispose();
  }

  void _onChanged() => setState(() {});

  bool get _valid =>
      _currentCtrl.text.isNotEmpty &&
      _newCtrl.text.isNotEmpty &&
      _newCtrl.text == _confirmCtrl.text;

  /// Verifies the current password server-side and stores the new one.
  Future<void> _submit() async {
    final l10n = AppLocalizations.of(context)!;
    if (!_valid || _busy || _locked) return;
    setState(() {
      _busy = true;
      _error = null;
    });
    try {
      final hash = hashPassword(_newCtrl.text);
      await ref
          .read(personsApiProvider)
          .changeOwnPassword(_currentCtrl.text, hash);
      if (!mounted) return;
      ScaffoldMessenger.of(context)
          .showSnackBar(SnackBar(content: Text(l10n.personPasswordSaved)));
      if (Navigator.of(context).canPop()) {
        Navigator.of(context).pop();
      }
    } on DioException catch (e) {
      if (!mounted) return;
      final status = e.response?.statusCode;
      if (status == 401 || status == 403) {
        _onWrongPassword(l10n);
      } else {
        setState(() => _busy = false);
        await ServerErrorDialog.show(context, e, l10n);
      }
    } catch (e) {
      if (!mounted) return;
      setState(() => _busy = false);
      await ServerErrorDialog.show(context, e, l10n);
    }
  }

  /// Shows the wrong-password error and locks the fields for three seconds.
  void _onWrongPassword(AppLocalizations l10n) {
    setState(() {
      _busy = false;
      _locked = true;
      _error = l10n.changePasswordWrong;
      _currentCtrl.clear();
    });
    _lockTimer?.cancel();
    _lockTimer = Timer(const Duration(seconds: 3), () {
      if (!mounted) return;
      setState(() {
        _locked = false;
        _error = null;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final mismatch = _newCtrl.text.isNotEmpty &&
        _confirmCtrl.text.isNotEmpty &&
        _newCtrl.text != _confirmCtrl.text;
    final enabled = !_busy && !_locked;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/change_password.jpeg',
          title: l10n.changePasswordTitle,
        ),
      ),
      body: Center(
        child: ConstrainedBox(
          constraints: const BoxConstraints(maxWidth: 420),
          child: SingleChildScrollView(
            padding: const EdgeInsets.all(24),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                TextField(
                  controller: _currentCtrl,
                  enabled: enabled,
                  obscureText: true,
                  autofocus: true,
                  decoration: InputDecoration(
                    labelText: l10n.changePasswordCurrent,
                    border: const OutlineInputBorder(),
                  ),
                  textInputAction: TextInputAction.next,
                ),
                const SizedBox(height: 16),
                TextField(
                  controller: _newCtrl,
                  enabled: enabled,
                  obscureText: true,
                  decoration: InputDecoration(
                    labelText: l10n.personPasswordNew,
                    border: const OutlineInputBorder(),
                  ),
                  textInputAction: TextInputAction.next,
                ),
                const SizedBox(height: 16),
                TextField(
                  controller: _confirmCtrl,
                  enabled: enabled,
                  obscureText: true,
                  decoration: InputDecoration(
                    labelText: l10n.personPasswordConfirm,
                    border: const OutlineInputBorder(),
                    errorText:
                        mismatch ? l10n.personPasswordMismatch : null,
                  ),
                  onSubmitted:
                      _valid && enabled ? (_) => _submit() : null,
                ),
                if (_error != null) ...[
                  const SizedBox(height: 12),
                  Text(
                    _error!,
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      color: Theme.of(context).colorScheme.error,
                    ),
                  ),
                ],
                const SizedBox(height: 24),
                FilledButton(
                  onPressed: _valid && enabled ? _submit : null,
                  child: _busy
                      ? const SizedBox(
                          width: 16,
                          height: 16,
                          child: CircularProgressIndicator.adaptive(
                              strokeWidth: 2),
                        )
                      : Text(l10n.changePasswordSubmit),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
