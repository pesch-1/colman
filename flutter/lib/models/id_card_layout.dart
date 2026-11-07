// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

import 'package:flutter_secure_storage/flutter_secure_storage.dart';

/// Persists the ID-card layout editor's free margin on the device.
///
/// Mirrors the Qt client, where the margin is a per-device placement
/// preference (stored in QSettings) rather than part of the shared,
/// server-stored layout. It bounds where zones may be placed and is shown as a
/// guide in the editor.
class IdCardMarginStore {
  /// Creates a margin store under [_key], letting different layout editors
  /// (ID card, book label, …) keep separate margins on the device.
  const IdCardMarginStore([this._key = 'id_card_editor_margin_mm']);

  static const _storage = FlutterSecureStorage();
  final String _key;

  /// Largest accepted margin, matching the Qt editor's 0–10 mm slider.
  static const double maxMm = 10.0;

  /// Loads the stored margin in millimetres, clamped to `0..maxMm`; defaults
  /// to 0 when nothing is stored or the value cannot be parsed.
  Future<double> load() async {
    try {
      final raw = await _storage.read(key: _key);
      final v = double.tryParse(raw ?? '') ?? 0.0;
      return v.clamp(0.0, maxMm);
    } catch (_) {
      return 0.0;
    }
  }

  /// Saves [marginMm] (clamped to `0..maxMm`) to secure storage.
  Future<void> save(double marginMm) async {
    final v = marginMm.clamp(0.0, maxMm);
    await _storage.write(key: _key, value: v.toString());
  }
}

/// One placeable zone (barcode, text or logo) of an ID-card layout.
///
/// Mirrors the server-side `IdCardElement` JSON object: position and size in
/// millimetres relative to the card's top-left corner, plus an orientation in
/// whole 90-degree steps (0, 90, 180 or 270).
class IdCardElement {
  const IdCardElement({
    this.orientation = 0,
    this.xMm = 0.0,
    this.yMm = 0.0,
    this.widthMm = 0.0,
    this.heightMm = 0.0,
  });

  /// Rotation of the zone in degrees; always a multiple of 90.
  final int orientation;

  /// Left edge relative to the card's left edge, in millimetres.
  final double xMm;

  /// Top edge relative to the card's top edge, in millimetres.
  final double yMm;

  /// Zone width in millimetres.
  final double widthMm;

  /// Zone height in millimetres.
  final double heightMm;

  IdCardElement copyWith({
    int? orientation,
    double? xMm,
    double? yMm,
    double? widthMm,
    double? heightMm,
  }) {
    return IdCardElement(
      orientation: orientation ?? this.orientation,
      xMm: xMm ?? this.xMm,
      yMm: yMm ?? this.yMm,
      widthMm: widthMm ?? this.widthMm,
      heightMm: heightMm ?? this.heightMm,
    );
  }

  factory IdCardElement.fromJson(Map<String, dynamic> json) {
    return IdCardElement(
      orientation: (json['orientation'] as num?)?.toInt() ?? 0,
      xMm: (json['x_mm'] as num?)?.toDouble() ?? 0.0,
      yMm: (json['y_mm'] as num?)?.toDouble() ?? 0.0,
      widthMm: (json['width_mm'] as num?)?.toDouble() ?? 0.0,
      heightMm: (json['height_mm'] as num?)?.toDouble() ?? 0.0,
    );
  }

  Map<String, dynamic> toJson() => {
        'orientation': orientation,
        'x_mm': xMm,
        'y_mm': yMm,
        'width_mm': widthMm,
        'height_mm': heightMm,
      };
}

/// Complete ID-card layout: card dimensions plus the three zones.
///
/// Mirrors the `id_card_layout` JSON object served read-only by
/// `GET /api/collection` (used once to seed a client's local copy) and stored
/// per device by [IdCardLayoutStore].
class IdCardLayout {
  const IdCardLayout({
    this.cardWidthMm = 0.0,
    this.cardHeightMm = 0.0,
    this.hasLogo = false,
    this.barcode = const IdCardElement(),
    this.text = const IdCardElement(),
    this.logo = const IdCardElement(),
  });

  /// Card width (longer side) in millimetres.
  final double cardWidthMm;

  /// Card height (shorter side) in millimetres.
  final double cardHeightMm;

  /// Whether the logo zone is part of the layout.
  final bool hasLogo;

  final IdCardElement barcode;
  final IdCardElement text;
  final IdCardElement logo;

  IdCardLayout copyWith({
    double? cardWidthMm,
    double? cardHeightMm,
    bool? hasLogo,
    IdCardElement? barcode,
    IdCardElement? text,
    IdCardElement? logo,
  }) {
    return IdCardLayout(
      cardWidthMm: cardWidthMm ?? this.cardWidthMm,
      cardHeightMm: cardHeightMm ?? this.cardHeightMm,
      hasLogo: hasLogo ?? this.hasLogo,
      barcode: barcode ?? this.barcode,
      text: text ?? this.text,
      logo: logo ?? this.logo,
    );
  }

  factory IdCardLayout.fromJson(Map<String, dynamic> json) {
    return IdCardLayout(
      cardWidthMm: (json['card_width_mm'] as num?)?.toDouble() ?? 0.0,
      cardHeightMm: (json['card_height_mm'] as num?)?.toDouble() ?? 0.0,
      hasLogo: json['has_logo'] as bool? ?? false,
      barcode: IdCardElement.fromJson(
          (json['barcode'] as Map<String, dynamic>?) ?? const {}),
      text: IdCardElement.fromJson(
          (json['text'] as Map<String, dynamic>?) ?? const {}),
      logo: IdCardElement.fromJson(
          (json['logo'] as Map<String, dynamic>?) ?? const {}),
    );
  }

  Map<String, dynamic> toJson() => {
        'card_width_mm': cardWidthMm,
        'card_height_mm': cardHeightMm,
        'has_logo': hasLogo,
        'barcode': barcode.toJson(),
        'text': text.toJson(),
        'logo': logo.toJson(),
      };
}

/// Persists the person ID-card layout on the device.
///
/// Card and label layouts are kept per client (each client may drive different
/// printers), so the layout lives in secure storage rather than on the server.
/// [loadOrSeed] performs a one-time seed from the server-supplied layout so an
/// existing collection's design is preserved on first use; from then on the
/// stored copy is authoritative and survives restarts and app updates.
class IdCardLayoutStore {
  const IdCardLayoutStore();

  static const _storage = FlutterSecureStorage();
  static const _key = 'id_card_layout';

  /// Returns the stored layout, or `null` when nothing has been stored yet.
  Future<IdCardLayout?> load() async {
    try {
      final raw = await _storage.read(key: _key);
      if (raw == null || raw.isEmpty) return null;
      return IdCardLayout.fromJson(jsonDecode(raw) as Map<String, dynamic>);
    } catch (_) {
      return null;
    }
  }

  /// Replaces the stored layout.
  Future<void> save(IdCardLayout layout) async {
    await _storage.write(key: _key, value: jsonEncode(layout.toJson()));
  }

  /// Returns the stored layout; if none exists yet, obtains one via [seed]
  /// (typically the server's current layout), stores it and returns it.
  Future<IdCardLayout> loadOrSeed(
      Future<IdCardLayout> Function() seed) async {
    final local = await load();
    if (local != null) return local;
    final seeded = await seed();
    await save(seeded);
    return seeded;
  }
}

/// Computes a person's Colman barcode from the collection [prefix] and the
/// person's database [id], client-side.
///
/// This mirrors the server's `compute_person_barcode` (barcode format 2):
/// `prefix + "000" + <digit-count of id> + <id> + <check digit>`, where the
/// check digit is the GTIN (EAN-13) check digit of the preceding string
/// shifted by 5.  A colman barcode is thus never a valid EAN/UPC/ISBN.
/// Computing it locally lets the print page work with only the `print_ids`
/// permission, without needing the `view_persons`-gated barcode endpoint.
String computePersonBarcode(String prefix, int id) {
  final num = id.toString();
  var body = '$prefix' '000' '${num.length}' '$num';
  return '$body${_barcodeCheckDigit(body)}';
}

/// Check-digit helper for [computePersonBarcode]; matches the C++ algorithm
/// (`barcode_check_digit_v2`), treating any non-digit character as a digit
/// value of -1.
int _barcodeCheckDigit(String s) {
  var sum = 0;
  for (var i = 0; i < s.length; ++i) {
    final c = s.codeUnitAt(s.length - 1 - i);
    final digit = (c >= 0x30 && c <= 0x39) ? c - 0x30 : -1;
    sum += (i.isEven ? 3 : 1) * digit;
  }
  final gtin = (10 - sum % 10) % 10;
  return (gtin + 5) % 10;
}
