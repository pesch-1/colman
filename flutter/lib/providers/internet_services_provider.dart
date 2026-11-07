// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../models/internet_services_settings.dart';

/// Manages persistence of [InternetServicesSettings] in [FlutterSecureStorage].
class InternetServicesNotifier
    extends AsyncNotifier<InternetServicesSettings> {
  static const _storage = FlutterSecureStorage();

  static const _keyUseOpenlibrary = 'inet_use_openlibrary';
  static const _keyUseGoogleapis = 'inet_use_googleapis';
  static const _keyGoogleapisKey = 'inet_googleapis_key';
  static const _keyUseMusicbrainz = 'inet_use_musicbrainz';
  static const _keyUseBgg = 'inet_use_bgg';
  static const _keyBggToken = 'inet_bgg_token';
  static const _keyUseOpengtindb = 'inet_use_opengtindb';
  static const _keyOpengtindbUserid = 'inet_opengtindb_userid';
  static const _keyTimeoutMs = 'inet_timeout_ms';
  static const _keySruServers = 'inet_sru_servers';
  static const _keyDownloadThumbnails = 'inet_download_thumbnails';

  @override
  Future<InternetServicesSettings> build() async {
    return _load();
  }

  Future<InternetServicesSettings> _load() async {
    final defaults = InternetServicesSettings.defaults();

    bool readBool(String? raw, bool fallback) =>
        raw == null ? fallback : raw == 'true';

    final useOpenlibrary = readBool(
      await _storage.read(key: _keyUseOpenlibrary),
      defaults.useOpenlibrary,
    );
    final useGoogleapis = readBool(
      await _storage.read(key: _keyUseGoogleapis),
      defaults.useGoogleapis,
    );
    final googleapisKey =
        await _storage.read(key: _keyGoogleapisKey) ?? defaults.googleapisKey;
    final useMusicbrainz = readBool(
      await _storage.read(key: _keyUseMusicbrainz),
      defaults.useMusicbrainz,
    );
    final useBgg = readBool(
      await _storage.read(key: _keyUseBgg),
      defaults.useBgg,
    );
    final bggToken =
        await _storage.read(key: _keyBggToken) ?? defaults.bggToken;
    final useOpengtindb = readBool(
      await _storage.read(key: _keyUseOpengtindb),
      defaults.useOpengtindb,
    );
    final opengtindbUserid =
        await _storage.read(key: _keyOpengtindbUserid) ??
        defaults.opengtindbUserid;
    final timeoutRaw = await _storage.read(key: _keyTimeoutMs);
    final timeoutMs =
        timeoutRaw != null
            ? (int.tryParse(timeoutRaw) ?? defaults.timeoutMs)
            : defaults.timeoutMs;
    final sruRaw = await _storage.read(key: _keySruServers);
    final sruServers =
        sruRaw != null ? sruServersFromJson(sruRaw) : defaults.sruServers;
    final downloadThumbnails = readBool(
      await _storage.read(key: _keyDownloadThumbnails),
      defaults.downloadThumbnails,
    );

    return InternetServicesSettings(
      useOpenlibrary: useOpenlibrary,
      useGoogleapis: useGoogleapis,
      googleapisKey: googleapisKey,
      useMusicbrainz: useMusicbrainz,
      useBgg: useBgg,
      bggToken: bggToken,
      useOpengtindb: useOpengtindb,
      opengtindbUserid: opengtindbUserid,
      timeoutMs: timeoutMs,
      sruServers: sruServers,
      downloadThumbnails: downloadThumbnails,
    );
  }

  /// Persists [settings] to secure storage and updates the provider state.
  Future<void> save(InternetServicesSettings settings) async {
    await _storage.write(
      key: _keyUseOpenlibrary,
      value: settings.useOpenlibrary.toString(),
    );
    await _storage.write(
      key: _keyUseGoogleapis,
      value: settings.useGoogleapis.toString(),
    );
    await _storage.write(
      key: _keyGoogleapisKey,
      value: settings.googleapisKey,
    );
    await _storage.write(
      key: _keyUseMusicbrainz,
      value: settings.useMusicbrainz.toString(),
    );
    await _storage.write(
      key: _keyUseBgg,
      value: settings.useBgg.toString(),
    );
    await _storage.write(key: _keyBggToken, value: settings.bggToken);
    await _storage.write(
      key: _keyUseOpengtindb,
      value: settings.useOpengtindb.toString(),
    );
    await _storage.write(
      key: _keyOpengtindbUserid,
      value: settings.opengtindbUserid,
    );
    await _storage.write(
      key: _keyTimeoutMs,
      value: settings.timeoutMs.toString(),
    );
    await _storage.write(
      key: _keySruServers,
      value: sruServersToJson(settings.sruServers),
    );
    await _storage.write(
      key: _keyDownloadThumbnails,
      value: settings.downloadThumbnails.toString(),
    );
    state = AsyncValue.data(settings);
  }
}

final internetServicesProvider =
    AsyncNotifierProvider<InternetServicesNotifier, InternetServicesSettings>(
      InternetServicesNotifier.new,
    );
