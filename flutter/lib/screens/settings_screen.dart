// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_svg/flutter_svg.dart';

import '../l10n/app_localizations.dart';
import '../models/internet_services_settings.dart';
import '../providers/internet_services_provider.dart';
import '../providers/poll_interval_provider.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';

class SettingsScreen extends ConsumerStatefulWidget {
  const SettingsScreen({super.key});

  @override
  ConsumerState<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends ConsumerState<SettingsScreen> {
  InternetServicesSettings? _draft;
  bool _initialized = false;
  bool _saving = false;

  final _googleapisKeyCtrl = TextEditingController();
  final _bggTokenCtrl = TextEditingController();
  final _opengtindbUseridCtrl = TextEditingController();
  final _timeoutCtrl = TextEditingController();
  final _pollIntervalCtrl = TextEditingController();

  @override
  void dispose() {
    _googleapisKeyCtrl.dispose();
    _bggTokenCtrl.dispose();
    _opengtindbUseridCtrl.dispose();
    _timeoutCtrl.dispose();
    _pollIntervalCtrl.dispose();
    super.dispose();
  }

  void _initFromSettings(InternetServicesSettings s) {
    _draft = s;
    _googleapisKeyCtrl.text = s.googleapisKey;
    _bggTokenCtrl.text = s.bggToken;
    _opengtindbUseridCtrl.text = s.opengtindbUserid;
    _timeoutCtrl.text = (s.timeoutMs ~/ 1000).toString();
    _pollIntervalCtrl.text =
        (ref.read(pollIntervalProvider).value ??
                PollIntervalNotifier.defaultSeconds)
            .toString();
    _initialized = true;
  }

  void _revert() {
    final async = ref.read(internetServicesProvider);
    if (async.value != null) {
      setState(() => _initFromSettings(async.value!));
    }
  }

  Future<void> _save() async {
    if (_draft == null) return;
    final timeoutSec = int.tryParse(_timeoutCtrl.text.trim());
    final updated = _draft!.copyWith(
      googleapisKey: _googleapisKeyCtrl.text.trim(),
      bggToken: _bggTokenCtrl.text.trim(),
      opengtindbUserid: _opengtindbUseridCtrl.text.trim(),
      timeoutMs: timeoutSec != null && timeoutSec > 0
          ? timeoutSec * 1000
          : _draft!.timeoutMs,
    );
    final pollSec = int.tryParse(_pollIntervalCtrl.text.trim());
    setState(() => _saving = true);
    try {
      await ref.read(internetServicesProvider.notifier).save(updated);
      if (pollSec != null) {
        await ref.read(pollIntervalProvider.notifier).set(pollSec);
      }
      if (mounted) {
        final l10n = AppLocalizations.of(context)!;
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(SnackBar(content: Text(l10n.settingsSaved)));
      }
    } finally {
      if (mounted) setState(() => _saving = false);
    }
  }

  /// Opens a dialog to add or edit an SRU server.
  ///
  /// Returns the updated [SruServer] or null if cancelled.
  Future<SruServer?> _showSruDialog({SruServer? existing}) async {
    final l10n = AppLocalizations.of(context)!;
    final nameCtrl = TextEditingController(text: existing?.name ?? '');
    final endpointCtrl = TextEditingController(text: existing?.endpoint ?? '');
    final isbnCtrl = TextEditingController(text: existing?.isbnField ?? '');
    final eanCtrl = TextEditingController(text: existing?.eanField ?? '');
    final titleCtrl = TextEditingController(text: existing?.titleField ?? '');
    final schemaCtrl = TextEditingController(
      text: existing?.recordSchema ?? 'marcxml',
    );
    final enabled = existing?.enabled ?? false;

    SruServer? result;
    await showDialog<void>(
      context: context,
      builder: (ctx) => StatefulBuilder(
        builder: (ctx, setStateDialog) => AlertDialog(
          title: Text(
            existing != null
                ? l10n.settingsSruEditServer
                : l10n.settingsSruAddServer,
          ),
          content: SingleChildScrollView(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                TextField(
                  controller: nameCtrl,
                  decoration: InputDecoration(labelText: l10n.settingsSruName),
                ),
                const SizedBox(height: 8),
                TextField(
                  controller: endpointCtrl,
                  decoration: InputDecoration(
                    labelText: l10n.settingsSruEndpoint,
                  ),
                  keyboardType: TextInputType.url,
                ),
                const SizedBox(height: 8),
                TextField(
                  controller: isbnCtrl,
                  decoration: InputDecoration(
                    labelText: l10n.settingsSruIsbnField,
                  ),
                ),
                const SizedBox(height: 8),
                TextField(
                  controller: eanCtrl,
                  decoration: InputDecoration(
                    labelText: l10n.settingsSruEanField,
                  ),
                ),
                const SizedBox(height: 8),
                TextField(
                  controller: titleCtrl,
                  decoration: InputDecoration(
                    labelText: l10n.settingsSruTitleField,
                  ),
                ),
                const SizedBox(height: 8),
                TextField(
                  controller: schemaCtrl,
                  decoration: InputDecoration(
                    labelText: l10n.settingsSruSchema,
                  ),
                ),
              ],
            ),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(ctx).pop(),
              child: Text(MaterialLocalizations.of(ctx).cancelButtonLabel),
            ),
            TextButton(
              onPressed: () {
                result = SruServer(
                  name: nameCtrl.text.trim(),
                  endpoint: endpointCtrl.text.trim(),
                  isbnField: isbnCtrl.text.trim(),
                  eanField: eanCtrl.text.trim(),
                  titleField: titleCtrl.text.trim(),
                  recordSchema: schemaCtrl.text.trim(),
                  enabled: enabled,
                );
                Navigator.of(ctx).pop();
              },
              child: Text(MaterialLocalizations.of(ctx).okButtonLabel),
            ),
          ],
        ),
      ),
    );

    nameCtrl.dispose();
    endpointCtrl.dispose();
    isbnCtrl.dispose();
    eanCtrl.dispose();
    titleCtrl.dispose();
    schemaCtrl.dispose();
    return result;
  }

  Widget _sectionHeader(String title) => Padding(
    padding: const EdgeInsets.fromLTRB(16, 20, 16, 4),
    child: Text(
      title,
      style: Theme.of(context).textTheme.titleSmall?.copyWith(
        color: Theme.of(context).colorScheme.primary,
        fontWeight: FontWeight.bold,
      ),
    ),
  );

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final async = ref.watch(internetServicesProvider);

    return async.when(
      loading: () => Scaffold(
        appBar: AppBar(
          title: AppBarTitle(
            icon: 'assets/icons/preferences.jpeg',
            title: l10n.settingsTitle,
          ),
        ),
        body: const Center(child: CircularProgressIndicator()),
      ),
      error: (e, _) => Scaffold(
        appBar: AppBar(
          title: AppBarTitle(
            icon: 'assets/icons/preferences.jpeg',
            title: l10n.settingsTitle,
          ),
        ),
        body: Center(
          child: ErrorDisplay(error: e, l10n: l10n),
        ),
      ),
      data: (settings) {
        if (!_initialized) {
          WidgetsBinding.instance.addPostFrameCallback(
            (_) => setState(() => _initFromSettings(settings)),
          );
          return Scaffold(
            appBar: AppBar(
              title: AppBarTitle(
                icon: 'assets/icons/preferences.jpeg',
                title: l10n.settingsTitle,
              ),
            ),
            body: const Center(child: CircularProgressIndicator()),
          );
        }

        final draft = _draft!;

        return Scaffold(
          appBar: AppBar(
            title: AppBarTitle(
              icon: 'assets/icons/preferences.jpeg',
              title: l10n.settingsTitle,
            ),
          ),
          body: Column(
            children: [
              Expanded(
                child: ListView(
                  children: [
                    // ── Internet services ──────────────────────────────
                    _sectionHeader(l10n.settingsInternetServicesSection),

                    // Open Library
                    SwitchListTile(
                      title: Text(l10n.settingsOpenLibrary),
                      value: draft.useOpenlibrary,
                      onChanged: (v) => setState(
                        () => _draft = draft.copyWith(useOpenlibrary: v),
                      ),
                    ),

                    // Google Books
                    SwitchListTile(
                      title: Text(l10n.settingsGoogleBooks),
                      value: draft.useGoogleapis,
                      onChanged: (v) => setState(
                        () => _draft = draft.copyWith(useGoogleapis: v),
                      ),
                    ),
                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
                      child: TextField(
                        controller: _googleapisKeyCtrl,
                        decoration: InputDecoration(
                          labelText: l10n.settingsGoogleApisKey,
                          isDense: true,
                        ),
                      ),
                    ),

                    // MusicBrainz
                    SwitchListTile(
                      title: Text(l10n.settingsMusicBrainz),
                      value: draft.useMusicbrainz,
                      onChanged: (v) => setState(
                        () => _draft = draft.copyWith(useMusicbrainz: v),
                      ),
                    ),

                    // BoardGameGeek
                    SwitchListTile(
                      title: Text(l10n.settingsBgg),
                      value: draft.useBgg,
                      onChanged: (v) =>
                          setState(() => _draft = draft.copyWith(useBgg: v)),
                    ),
                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
                      child: TextField(
                        controller: _bggTokenCtrl,
                        decoration: InputDecoration(
                          labelText: l10n.settingsBggToken,
                          isDense: true,
                        ),
                      ),
                    ),

                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 4, 16, 8),
                      child: SvgPicture.asset(
                        'assets/thirdparty/powered-by-bgg-rgb.svg',
                        height: 38,
                        alignment: Alignment.centerLeft,
                      ),
                    ),

                    // OpenGTINdb
                    SwitchListTile(
                      title: Text(l10n.settingsOpenGtindb),
                      value: draft.useOpengtindb,
                      onChanged: (v) => setState(
                        () => _draft = draft.copyWith(useOpengtindb: v),
                      ),
                    ),
                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
                      child: TextField(
                        controller: _opengtindbUseridCtrl,
                        decoration: InputDecoration(
                          labelText: l10n.settingsOpenGtindbUserId,
                          isDense: true,
                        ),
                      ),
                    ),

                    // Download cover images
                    SwitchListTile(
                      title: Text(l10n.settingsDownloadThumbnails),
                      value: draft.downloadThumbnails,
                      onChanged: (v) => setState(
                        () => _draft = draft.copyWith(downloadThumbnails: v),
                      ),
                    ),

                    // ── SRU servers ────────────────────────────────────
                    _sectionHeader(l10n.settingsSruServersSection),

                    ...List.generate(draft.sruServers.length, (i) {
                      final srv = draft.sruServers[i];
                      return ExpansionTile(
                        leading: Checkbox(
                          value: srv.enabled,
                          onChanged: (v) {
                            final updated = List<SruServer>.of(
                              draft.sruServers,
                            );
                            updated[i] = srv.copyWith(enabled: v ?? false);
                            setState(
                              () =>
                                  _draft = draft.copyWith(sruServers: updated),
                            );
                          },
                        ),
                        title: Text(srv.name),
                        subtitle: Text(
                          srv.endpoint,
                          maxLines: 1,
                          overflow: TextOverflow.ellipsis,
                          style: Theme.of(context).textTheme.bodySmall,
                        ),
                        trailing: Row(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            IconButton(
                              icon: const Icon(Icons.edit, size: 20),
                              tooltip: l10n.settingsSruEditServer,
                              onPressed: () async {
                                final edited = await _showSruDialog(
                                  existing: srv,
                                );
                                if (edited != null) {
                                  final updated = List<SruServer>.of(
                                    draft.sruServers,
                                  );
                                  updated[i] = edited.copyWith(
                                    enabled: srv.enabled,
                                  );
                                  setState(
                                    () => _draft = draft.copyWith(
                                      sruServers: updated,
                                    ),
                                  );
                                }
                              },
                            ),
                            IconButton(
                              icon: const Icon(Icons.delete, size: 20),
                              tooltip: l10n.settingsSruDeleteServer,
                              onPressed: () {
                                final updated = List<SruServer>.of(
                                  draft.sruServers,
                                )..removeAt(i);
                                setState(
                                  () => _draft = draft.copyWith(
                                    sruServers: updated,
                                  ),
                                );
                              },
                            ),
                          ],
                        ),
                        children: [
                          Padding(
                            padding: const EdgeInsets.fromLTRB(72, 0, 16, 12),
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                _detailRow(
                                  l10n.settingsSruIsbnField,
                                  srv.isbnField,
                                ),
                                _detailRow(
                                  l10n.settingsSruEanField,
                                  srv.eanField,
                                ),
                                _detailRow(
                                  l10n.settingsSruTitleField,
                                  srv.titleField,
                                ),
                                _detailRow(
                                  l10n.settingsSruSchema,
                                  srv.recordSchema,
                                ),
                              ],
                            ),
                          ),
                        ],
                      );
                    }),

                    Padding(
                      padding: const EdgeInsets.symmetric(horizontal: 16),
                      child: OutlinedButton.icon(
                        icon: const Icon(Icons.add),
                        label: Text(l10n.settingsSruAddServer),
                        onPressed: () async {
                          final added = await _showSruDialog();
                          if (added != null) {
                            setState(
                              () => _draft = draft.copyWith(
                                sruServers: [...draft.sruServers, added],
                              ),
                            );
                          }
                        },
                      ),
                    ),

                    // ── Network ────────────────────────────────────────
                    _sectionHeader(l10n.settingsNetworkSection),

                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 4, 16, 8),
                      child: TextField(
                        controller: _timeoutCtrl,
                        decoration: InputDecoration(
                          labelText: l10n.settingsTimeoutSeconds,
                          isDense: true,
                        ),
                        keyboardType: TextInputType.number,
                        inputFormatters: [
                          FilteringTextInputFormatter.digitsOnly,
                        ],
                      ),
                    ),

                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 4, 16, 16),
                      child: TextField(
                        controller: _pollIntervalCtrl,
                        decoration: InputDecoration(
                          labelText: l10n.settingsPollIntervalSeconds,
                          helperText: l10n.settingsPollIntervalHint,
                          helperMaxLines: 2,
                          isDense: true,
                        ),
                        keyboardType: TextInputType.number,
                        inputFormatters: [
                          FilteringTextInputFormatter.digitsOnly,
                        ],
                      ),
                    ),

                    const SizedBox(height: 16),
                  ],
                ),
              ),

              // ── Bottom action bar ──────────────────────────────────────
              const Divider(height: 1),
              Padding(
                padding: const EdgeInsets.symmetric(
                  horizontal: 16,
                  vertical: 8,
                ),
                child: Row(
                  children: [
                    OutlinedButton(
                      onPressed: _saving ? null : _revert,
                      child: Text(l10n.settingsRevert),
                    ),
                    const Spacer(),
                    FilledButton(
                      onPressed: _saving ? null : _save,
                      child: _saving
                          ? const SizedBox(
                              width: 20,
                              height: 20,
                              child: CircularProgressIndicator(
                                strokeWidth: 2,
                                color: Colors.white,
                              ),
                            )
                          : Text(l10n.settingsSave),
                    ),
                  ],
                ),
              ),
            ],
          ),
        );
      },
    );
  }

  Widget _detailRow(String label, String value) => Padding(
    padding: const EdgeInsets.only(bottom: 2),
    child: Row(
      children: [
        Text(
          '$label: ',
          style: Theme.of(
            context,
          ).textTheme.bodySmall?.copyWith(fontWeight: FontWeight.bold),
        ),
        Flexible(
          child: Text(
            value.isEmpty ? '—' : value,
            style: Theme.of(context).textTheme.bodySmall,
          ),
        ),
      ],
    ),
  );
}
