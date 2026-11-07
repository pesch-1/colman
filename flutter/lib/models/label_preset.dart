// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../printing/label_bitmap.dart' show defaultBarcodeUnitMm;
import 'id_card_layout.dart';
import 'printer_calibration.dart';
import 'signature_layout.dart';

/// What a label preset is used for. Determines the arrangement (zones) and the
/// print flow it appears in.
enum LabelPurpose { idCard, bookLabel, signature }

/// The kind of printer a preset targets. On mobile this is a type rather than a
/// concrete printer: [thermalUsb] uses the cable-connected thermal printer,
/// [systemSheet] tiles labels on a sheet and prints via the OS print dialog
/// (any network/system printer).
enum PrinterType { thermalUsb, systemSheet }

/// Supported sheet paper sizes for [PrinterType.systemSheet] presets.
enum PaperSize { a3, a4, a5, letter, legal }

/// A named, device-local label layout preset — the Flutter equivalent of the
/// desktop `LabelLayoutPreset`. Bundles the printer type, the printer-specific
/// parameters (thermal vs. sheet) and the on-label arrangement under one name,
/// so the user picks items first and then a preset (which carries printer,
/// label size and labels-per-sheet) when printing.
class LabelPreset {
  const LabelPreset({
    required this.name,
    required this.purpose,
    required this.printerType,
    this.barWidthReductionMm = 0.0,
    this.barcodeUnitMm = defaultBarcodeUnitMm,
    // Thermal parameters.
    this.dpi = 203,
    this.darkness = 8,
    this.xOffsetMm = 11.0,
    this.gapMm = 4.0,
    // Sheet parameters.
    this.columns = 4,
    this.rows = 10,
    this.leftMarginMm = 5.0,
    this.topMarginMm = 10.0,
    this.hGapMm = 2.5,
    this.vGapMm = 0.0,
    this.paperSize = PaperSize.a4,
    this.landscape = false,
    this.calibration = const PrinterCalibration(),
    // Arrangements (the active one depends on [purpose]).
    this.cardLayout = _defaultCardLayout,
    this.signatureLayout = const SignatureLayout(),
  });

  /// Unique display name (the storage key).
  final String name;
  final LabelPurpose purpose;
  final PrinterType printerType;

  /// Barcode bar-width reduction in millimetres (thermal ink-spread comp.).
  final double barWidthReductionMm;

  /// Code128 X-dimension (narrowest bar width) in millimetres.
  final double barcodeUnitMm;

  // Thermal printer parameters.
  final int dpi;
  final int darkness;
  final double xOffsetMm;
  final double gapMm;

  // Sheet printer parameters.
  final int columns;
  final int rows;
  final double leftMarginMm;
  final double topMarginMm;
  final double hGapMm;
  final double vGapMm;
  final PaperSize paperSize;
  final bool landscape;

  /// Print-position correction for the sheet/system printer (sheet presets
  /// only; thermal presets ignore it and rely on [xOffsetMm]).
  final PrinterCalibration calibration;

  /// Zone arrangement for [LabelPurpose.idCard] / [LabelPurpose.bookLabel].
  final IdCardLayout cardLayout;

  /// Zone arrangement for [LabelPurpose.signature].
  final SignatureLayout signatureLayout;

  /// Highest darkness the print head honours (TSPL `DENSITY` is a 4-bit value).
  static const int maxDarkness = 15;

  static const IdCardLayout _defaultCardLayout = IdCardLayout(
    cardWidthMm: 75.0,
    cardHeightMm: 50.0,
    hasLogo: false,
    barcode: IdCardElement(xMm: 3.0, yMm: 24.0, widthMm: 50.0, heightMm: 22.0),
    text: IdCardElement(xMm: 3.0, yMm: 3.0, widthMm: 69.0, heightMm: 18.0),
    logo: IdCardElement(xMm: 55.0, yMm: 24.0, widthMm: 17.0, heightMm: 22.0),
  );

  /// Label width in millimetres (from the active arrangement).
  double get labelWidthMm => purpose == LabelPurpose.signature
      ? signatureLayout.cardWidthMm
      : cardLayout.cardWidthMm;

  /// Label height in millimetres (from the active arrangement).
  double get labelHeightMm => purpose == LabelPurpose.signature
      ? signatureLayout.cardHeightMm
      : cardLayout.cardHeightMm;

  LabelPreset copyWith({
    String? name,
    LabelPurpose? purpose,
    PrinterType? printerType,
    double? barWidthReductionMm,
    double? barcodeUnitMm,
    int? dpi,
    int? darkness,
    double? xOffsetMm,
    double? gapMm,
    int? columns,
    int? rows,
    double? leftMarginMm,
    double? topMarginMm,
    double? hGapMm,
    double? vGapMm,
    PaperSize? paperSize,
    bool? landscape,
    PrinterCalibration? calibration,
    IdCardLayout? cardLayout,
    SignatureLayout? signatureLayout,
  }) {
    return LabelPreset(
      name: name ?? this.name,
      purpose: purpose ?? this.purpose,
      printerType: printerType ?? this.printerType,
      barWidthReductionMm: barWidthReductionMm ?? this.barWidthReductionMm,
      barcodeUnitMm: barcodeUnitMm ?? this.barcodeUnitMm,
      dpi: dpi ?? this.dpi,
      darkness: darkness ?? this.darkness,
      xOffsetMm: xOffsetMm ?? this.xOffsetMm,
      gapMm: gapMm ?? this.gapMm,
      columns: columns ?? this.columns,
      rows: rows ?? this.rows,
      leftMarginMm: leftMarginMm ?? this.leftMarginMm,
      topMarginMm: topMarginMm ?? this.topMarginMm,
      hGapMm: hGapMm ?? this.hGapMm,
      vGapMm: vGapMm ?? this.vGapMm,
      paperSize: paperSize ?? this.paperSize,
      landscape: landscape ?? this.landscape,
      calibration: calibration ?? this.calibration,
      cardLayout: cardLayout ?? this.cardLayout,
      signatureLayout: signatureLayout ?? this.signatureLayout,
    );
  }

  factory LabelPreset.fromJson(Map<String, dynamic> json) {
    T enumByName<T>(List<T> values, Object? name, T fallback) {
      for (final v in values) {
        if ((v as Enum).name == name) return v;
      }
      return fallback;
    }

    const d = LabelPreset(
        name: '',
        purpose: LabelPurpose.bookLabel,
        printerType: PrinterType.thermalUsb);
    return LabelPreset(
      name: json['name'] as String? ?? '',
      purpose:
          enumByName(LabelPurpose.values, json['purpose'], LabelPurpose.bookLabel),
      printerType: enumByName(
          PrinterType.values, json['printer_type'], PrinterType.thermalUsb),
      barWidthReductionMm:
          (json['bar_width_reduction_mm'] as num?)?.toDouble() ?? 0.0,
      barcodeUnitMm: ((json['barcode_unit_mm'] as num?)?.toDouble() ??
              defaultBarcodeUnitMm)
          .clamp(0.15, 1.0),
      dpi: (json['dpi'] as num?)?.toInt() ?? d.dpi,
      darkness:
          ((json['darkness'] as num?)?.toInt() ?? d.darkness).clamp(0, maxDarkness),
      xOffsetMm: (json['x_offset_mm'] as num?)?.toDouble() ?? d.xOffsetMm,
      gapMm: (json['gap_mm'] as num?)?.toDouble() ?? d.gapMm,
      columns: (json['columns'] as num?)?.toInt() ?? d.columns,
      rows: (json['rows'] as num?)?.toInt() ?? d.rows,
      leftMarginMm: (json['left_margin_mm'] as num?)?.toDouble() ?? d.leftMarginMm,
      topMarginMm: (json['top_margin_mm'] as num?)?.toDouble() ?? d.topMarginMm,
      hGapMm: (json['h_gap_mm'] as num?)?.toDouble() ?? d.hGapMm,
      vGapMm: (json['v_gap_mm'] as num?)?.toDouble() ?? d.vGapMm,
      paperSize: enumByName(PaperSize.values, json['paper_size'], PaperSize.a4),
      landscape: json['landscape'] as bool? ?? false,
      calibration: json['calibration'] is Map<String, dynamic>
          ? PrinterCalibration.fromJson(
              json['calibration'] as Map<String, dynamic>)
          : const PrinterCalibration(),
      cardLayout: json['card_layout'] is Map<String, dynamic>
          ? IdCardLayout.fromJson(json['card_layout'] as Map<String, dynamic>)
          : _defaultCardLayout,
      signatureLayout: json['signature_layout'] is Map<String, dynamic>
          ? SignatureLayout.fromJson(
              json['signature_layout'] as Map<String, dynamic>)
          : const SignatureLayout(),
    );
  }

  Map<String, dynamic> toJson() => {
        'name': name,
        'purpose': purpose.name,
        'printer_type': printerType.name,
        'bar_width_reduction_mm': barWidthReductionMm,
        'barcode_unit_mm': barcodeUnitMm,
        'dpi': dpi,
        'darkness': darkness,
        'x_offset_mm': xOffsetMm,
        'gap_mm': gapMm,
        'columns': columns,
        'rows': rows,
        'left_margin_mm': leftMarginMm,
        'top_margin_mm': topMarginMm,
        'h_gap_mm': hGapMm,
        'v_gap_mm': vGapMm,
        'paper_size': paperSize.name,
        'landscape': landscape,
        'calibration': calibration.toJson(),
        'card_layout': cardLayout.toJson(),
        'signature_layout': signatureLayout.toJson(),
      };
}

/// Persists the named [LabelPreset]s on the device.
class LabelPresetStore {
  static const _storage = FlutterSecureStorage();
  static const _key = 'label_presets';

  /// The presets seeded when nothing is stored yet: a thermal and a sheet book
  /// label, plus a thermal signature.
  static List<LabelPreset> get defaults => const [
        LabelPreset(
          name: 'Buchetikett (Thermo)',
          purpose: LabelPurpose.bookLabel,
          printerType: PrinterType.thermalUsb,
        ),
        LabelPreset(
          name: 'Buchetikett (Bogen)',
          purpose: LabelPurpose.bookLabel,
          printerType: PrinterType.systemSheet,
        ),
        LabelPreset(
          name: 'Signatur (Thermo)',
          purpose: LabelPurpose.signature,
          printerType: PrinterType.thermalUsb,
        ),
      ];

  /// Loads all presets, seeding [defaults] when none are stored.
  Future<List<LabelPreset>> loadAll() async {
    try {
      final raw = await _storage.read(key: _key);
      if (raw == null || raw.isEmpty) return defaults;
      final list = jsonDecode(raw) as List<dynamic>;
      final presets = list
          .map((e) => LabelPreset.fromJson(e as Map<String, dynamic>))
          .toList();
      return presets.isEmpty ? defaults : presets;
    } catch (_) {
      return defaults;
    }
  }

  /// Replaces the stored preset list.
  Future<void> saveAll(List<LabelPreset> presets) async {
    await _storage.write(
        key: _key,
        value: jsonEncode(presets.map((p) => p.toJson()).toList()));
  }

  /// Reads the preset name last selected in the print dialog [dialogKey].
  Future<String?> lastPresetName(String dialogKey) async {
    try {
      return await _storage.read(key: 'label_last_preset_$dialogKey');
    } catch (_) {
      return null;
    }
  }

  /// Remembers [name] as the last preset selected in print dialog [dialogKey].
  Future<void> setLastPresetName(String dialogKey, String name) async {
    await _storage.write(key: 'label_last_preset_$dialogKey', value: name);
  }
}
