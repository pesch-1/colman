// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:io';

import 'package:flutter/services.dart';

/// Controls the Android Wi-Fi multicast lock.
///
/// Android Wi-Fi drivers filter out received multicast packets by default
/// to save power, which silently drops mDNS responses. Acquiring a
/// [WifiManager.MulticastLock] on the platform side disables that filter.
/// On all other platforms the methods are no-ops.
///
/// Hold the lock only while mDNS discovery is running — it costs battery.
class MulticastLock {
  static const _channel =
      MethodChannel('de.colman.colman_client/multicast_lock');

  /// Acquires the multicast lock; safe to call when already held.
  static Future<void> acquire() => _invoke('acquire');

  /// Releases the multicast lock; safe to call when not held.
  static Future<void> release() => _invoke('release');

  static Future<void> _invoke(String method) async {
    if (!Platform.isAndroid) return;
    try {
      await _channel.invokeMethod<void>(method);
    } on PlatformException {
      // Discovery may still work without the lock; never fail the caller.
    } on MissingPluginException {
      // Platform side not available (e.g. in tests).
    }
  }
}
