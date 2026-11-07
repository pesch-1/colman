// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../api/api_client.dart';
import '../providers/auth_provider.dart';
import '../services/certificate_trust_store.dart';
import '../services/mdns_browser.dart';
import '../utils/error_utils.dart';
import '../widgets/server_error_dialog.dart';

enum _DiscoveryStatus { searching, noServers, unavailable }

class LoginScreen extends ConsumerStatefulWidget {
  const LoginScreen({super.key});

  @override
  ConsumerState<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends ConsumerState<LoginScreen> {
  final _formKey = GlobalKey<FormState>();
  final _serverController = TextEditingController();
  final _usernameController = TextEditingController();
  final _passwordController = TextEditingController();
  bool _obscurePassword = true;

  final List<MdnsServiceInfo> _discoveredServers = [];
  bool _isDiscovering = false;
  _DiscoveryStatus _discoveryStatus = _DiscoveryStatus.searching;
  StreamSubscription<MdnsServiceInfo>? _subscription;

  int _page = 0;
  MdnsServiceInfo? _selectedServer;

  @override
  void initState() {
    super.initState();
    _isDiscovering = true;
    _subscription = _makeSubscription();
  }

  @override
  void dispose() {
    _subscription?.cancel();
    _serverController.dispose();
    _usernameController.dispose();
    _passwordController.dispose();
    super.dispose();
  }

  StreamSubscription<MdnsServiceInfo> _makeSubscription() {
    return browseMdnsServices().listen(
      (info) {
        if (!mounted) return;
        setState(() {
          final idx = _discoveredServers.indexWhere((s) => s.url == info.url);
          if (idx >= 0) {
            _discoveredServers[idx] = info;
          } else {
            _discoveredServers.add(info);
          }
        });
      },
      onDone: () {
        if (!mounted) return;
        setState(() {
          _isDiscovering = false;
          if (_discoveredServers.isEmpty) {
            _discoveryStatus = _DiscoveryStatus.noServers;
          }
        });
      },
      onError: (Object e) {
        if (!mounted) return;
        setState(() {
          _isDiscovering = false;
          _discoveryStatus = _DiscoveryStatus.unavailable;
        });
      },
      cancelOnError: true,
    );
  }

  void _startDiscovery() {
    _subscription?.cancel();
    setState(() {
      _discoveredServers.clear();
      _selectedServer = null;
      _isDiscovering = true;
      _discoveryStatus = _DiscoveryStatus.searching;
    });
    _subscription = _makeSubscription();
  }

  Future<void> _login() async {
    if (!_formKey.currentState!.validate()) return;
    await ref.read(authProvider.notifier).loginWithPassword(
          _serverController.text.trim(),
          _usernameController.text.trim(),
          _passwordController.text,
        );
  }

  Future<void> _showCertDialog(CertificateException ex) async {
    if (!mounted) return;
    final l10n = AppLocalizations.of(context)!;
    String fmt(DateTime dt) => dt.toLocal().toIso8601String().substring(0, 10);
    final trusted = await showDialog<bool>(
      context: context,
      barrierDismissible: false,
      builder: (ctx) => AlertDialog(
        title: Text(l10n.certDialogTitle),
        content: SingleChildScrollView(
          child: Text(l10n.certDialogBody(
            ex.host,
            ex.subject,
            fmt(ex.validFrom),
            fmt(ex.validUntil),
            ex.fingerprint,
          )),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx, false),
            child: Text(l10n.certDialogCancel),
          ),
          FilledButton(
            onPressed: () => Navigator.pop(ctx, true),
            child: Text(l10n.certDialogTrust),
          ),
        ],
      ),
    );
    if (trusted == true && mounted) {
      await certificateTrustStore.trust(ex.host, ex.fingerprint);
      await _login();
    }
  }

  @override
  Widget build(BuildContext context) {
    final authState = ref.watch(authProvider);
    final l10n = AppLocalizations.of(context)!;

    ref.listen<AsyncValue<AuthState>>(authProvider, (prev, next) {
      if (next.hasError && next.error is CertificateException) {
        _showCertDialog(next.error! as CertificateException);
      }
    });
    final theme = Theme.of(context);
    final showCredentials = _page == 1 || _selectedServer != null;

    return Scaffold(
      body: SingleChildScrollView(
        child: Center(
          child: ConstrainedBox(
            constraints: const BoxConstraints(maxWidth: 480),
            child: Padding(
              padding: const EdgeInsets.all(32),
              child: Form(
                key: _formKey,
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    Image.asset(
                      'assets/icons/connection2.jpeg',
                      height: 96,
                    ),
                    const SizedBox(height: 8),
                    Text(
                      l10n.appTitle,
                      style: theme.textTheme.headlineLarge,
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 4),
                    Text(
                      l10n.loginSubtitle,
                      style: theme.textTheme.bodyMedium,
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 24),
                    if (_page == 0)
                      _buildDiscoveryPage(theme, l10n)
                    else
                      _buildManualPage(l10n),
                    if (showCredentials) _buildCredentials(l10n),
                    if (authState.hasError &&
                        authState.error is! CertificateException) ...[
                      const SizedBox(height: 12),
                      Text(
                        friendlyHttpError(authState.error!, l10n),
                        style: TextStyle(color: theme.colorScheme.error),
                      ),
                      TextButton(
                        onPressed: () => ServerErrorDialog.show(
                          context,
                          authState.error!,
                          l10n,
                        ),
                        child: Text(l10n.errorDetails),
                      ),
                    ],
                    if (showCredentials) ...[
                      const SizedBox(height: 24),
                      FilledButton(
                        onPressed: authState.isLoading ? null : _login,
                        child: authState.isLoading
                            ? const SizedBox(
                                height: 20,
                                width: 20,
                                child: CircularProgressIndicator(
                                    strokeWidth: 2),
                              )
                            : Text(l10n.loginSignIn),
                      ),
                    ],
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildDiscoveryPage(ThemeData theme, AppLocalizations l10n) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildDiscoverySection(theme, l10n),
        const SizedBox(height: 8),
        Align(
          alignment: Alignment.centerLeft,
          child: TextButton.icon(
            icon: const Icon(Icons.arrow_forward, size: 16),
            label: Text(l10n.loginEnterUrlManually),
            onPressed: () => setState(() => _page = 1),
          ),
        ),
      ],
    );
  }

  Widget _buildManualPage(AppLocalizations l10n) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Align(
          alignment: Alignment.centerLeft,
          child: TextButton.icon(
            icon: const Icon(Icons.arrow_back, size: 16),
            label: Text(l10n.loginBackToDiscovery),
            onPressed: () => setState(() => _page = 0),
          ),
        ),
        const SizedBox(height: 8),
        TextFormField(
          controller: _serverController,
          decoration: InputDecoration(
            labelText: l10n.loginServerUrl,
            hintText: l10n.loginServerUrlHint,
            prefixIcon: const Icon(Icons.dns),
            border: const OutlineInputBorder(),
          ),
          keyboardType: TextInputType.url,
          autocorrect: false,
          validator: (v) =>
              (v == null || v.trim().isEmpty) ? l10n.loginRequired : null,
        ),
      ],
    );
  }

  Widget _buildCredentials(AppLocalizations l10n) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        const SizedBox(height: 16),
        TextFormField(
          controller: _usernameController,
          decoration: InputDecoration(
            labelText: l10n.loginUsername,
            prefixIcon: const Icon(Icons.person),
            border: const OutlineInputBorder(),
          ),
          autocorrect: false,
          validator: (v) =>
              (v == null || v.trim().isEmpty) ? l10n.loginRequired : null,
        ),
        const SizedBox(height: 16),
        TextFormField(
          controller: _passwordController,
          decoration: InputDecoration(
            labelText: l10n.loginPassword,
            prefixIcon: const Icon(Icons.lock),
            border: const OutlineInputBorder(),
            suffixIcon: IconButton(
              icon: Icon(_obscurePassword
                  ? Icons.visibility
                  : Icons.visibility_off),
              onPressed: () =>
                  setState(() => _obscurePassword = !_obscurePassword),
            ),
          ),
          obscureText: _obscurePassword,
          validator: (v) =>
              (v == null || v.isEmpty) ? l10n.loginRequired : null,
          onFieldSubmitted: (_) => _login(),
        ),
      ],
    );
  }

  Widget _buildDiscoverySection(ThemeData theme, AppLocalizations l10n) {
    final statusText = switch (_discoveryStatus) {
      _DiscoveryStatus.searching => l10n.loginSearching,
      _DiscoveryStatus.noServers => l10n.loginNoServers,
      _DiscoveryStatus.unavailable => l10n.loginDiscoveryUnavailable,
    };
    return Card(
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(8),
        side: BorderSide(color: theme.dividerColor),
      ),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(12, 8, 4, 4),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Row(
              children: [
                Text(l10n.loginDiscoverOnLan,
                    style: theme.textTheme.labelLarge),
                const Spacer(),
                if (_isDiscovering)
                  const Padding(
                    padding: EdgeInsets.symmetric(horizontal: 12),
                    child: SizedBox(
                      width: 16,
                      height: 16,
                      child: CircularProgressIndicator(strokeWidth: 2),
                    ),
                  ),
                TextButton.icon(
                  icon: const Icon(Icons.refresh, size: 16),
                  label: Text(l10n.loginRefresh),
                  onPressed: _isDiscovering ? null : _startDiscovery,
                ),
              ],
            ),
            if (_discoveredServers.isEmpty)
              Padding(
                padding: const EdgeInsets.fromLTRB(4, 0, 4, 8),
                child: Text(
                  statusText,
                  style: theme.textTheme.bodySmall?.copyWith(
                    color: theme.colorScheme.onSurfaceVariant,
                  ),
                ),
              )
            else
              ..._discoveredServers.map(
                (s) => ListTile(
                  title: Text(s.name),
                  subtitle: Text('${s.host}:${s.port}'),
                  trailing: _selectedServer?.url == s.url
                      ? const Icon(Icons.check_circle, color: Colors.green)
                      : const Icon(Icons.chevron_right),
                  dense: true,
                  contentPadding:
                      const EdgeInsets.symmetric(horizontal: 4),
                  onTap: () => setState(() {
                    _selectedServer = s;
                    _serverController.text = s.url;
                  }),
                ),
              ),
          ],
        ),
      ),
    );
  }
}
