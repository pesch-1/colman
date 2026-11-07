// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:io' show Platform;

import 'package:flutter/services.dart';

/// A USB device reported by the Android platform side.
class UsbDeviceInfo {
  const UsbDeviceInfo({
    required this.name,
    required this.vendorId,
    required this.productId,
    required this.product,
    required this.manufacturer,
    required this.isPrinter,
    required this.hasPermission,
  });

  /// Stable device identifier used to address the device in later calls.
  final String name;
  final int vendorId;
  final int productId;

  /// Human-readable product / manufacturer strings (may be empty).
  final String product;
  final String manufacturer;

  /// Whether the device exposes a USB printer-class (7) interface.
  final bool isPrinter;

  /// Whether the app already holds permission to access the device.
  final bool hasPermission;

  /// Best-effort display label for the device list.
  String get label {
    final parts = [manufacturer, product].where((s) => s.isNotEmpty).toList();
    if (parts.isNotEmpty) return parts.join(' ');
    return 'USB ${vendorId.toRadixString(16)}:${productId.toRadixString(16)}';
  }

  factory UsbDeviceInfo.fromMap(Map<dynamic, dynamic> m) {
    return UsbDeviceInfo(
      name: m['name'] as String? ?? '',
      vendorId: (m['vendorId'] as num?)?.toInt() ?? 0,
      productId: (m['productId'] as num?)?.toInt() ?? 0,
      product: m['product'] as String? ?? '',
      manufacturer: m['manufacturer'] as String? ?? '',
      isPrinter: m['isPrinter'] as bool? ?? false,
      hasPermission: m['hasPermission'] as bool? ?? false,
    );
  }
}

/// Raised when a USB print operation cannot be completed.
class UsbPrinterException implements Exception {
  const UsbPrinterException(this.message);
  final String message;
  @override
  String toString() => message;
}

/// Thin Dart wrapper around the Android `UsbManager` platform channel that
/// sends raw bytes (a TSPL stream) to a cable-connected thermal label printer.
///
/// USB-host printing is Android-only; on every other platform the methods
/// report that the feature is unavailable. The native side lives in
/// `android/.../UsbPrinterChannel.kt`.
class UsbLabelPrinter {
  static const _channel =
      MethodChannel('de.colman.colman_client/usb_printer');

  /// USB vendor/product id of the Vevor Y486 (and identical OEM units).
  static const int vevorVendorId = 0x5958;
  static const int vevorProductId = 0x0486;

  /// Whether direct USB printing is supported on this platform.
  bool get isSupported => Platform.isAndroid;

  /// Lists the currently attached USB devices.
  Future<List<UsbDeviceInfo>> listDevices() async {
    if (!isSupported) return const [];
    final result = await _channel.invokeMethod<List<dynamic>>('listDevices');
    return (result ?? const [])
        .map((e) => UsbDeviceInfo.fromMap(e as Map<dynamic, dynamic>))
        .toList();
  }

  /// Returns the most likely label printer: the Vevor Y486 by id, otherwise the
  /// first device exposing a printer-class interface, otherwise null.
  Future<UsbDeviceInfo?> findPrinter() async {
    final devices = await listDevices();
    for (final d in devices) {
      if (d.vendorId == vevorVendorId && d.productId == vevorProductId) {
        return d;
      }
    }
    for (final d in devices) {
      if (d.isPrinter) return d;
    }
    return null;
  }

  /// Requests the user's permission to access [deviceName]; returns whether it
  /// was granted.
  Future<bool> requestPermission(String deviceName) async {
    if (!isSupported) return false;
    final granted = await _channel.invokeMethod<bool>(
        'requestPermission', {'name': deviceName});
    return granted ?? false;
  }

  /// Sends [bytes] to [deviceName] over its bulk OUT endpoint, requesting
  /// permission first if necessary. Throws [UsbPrinterException] on failure.
  Future<void> printBytes(String deviceName, Uint8List bytes) async {
    if (!isSupported) {
      throw const UsbPrinterException('USB printing is only available on Android.');
    }
    try {
      await _channel.invokeMethod<void>('printBytes', {
        'name': deviceName,
        'bytes': bytes,
      });
    } on PlatformException catch (e) {
      throw UsbPrinterException(e.message ?? 'USB print failed.');
    }
  }
}
