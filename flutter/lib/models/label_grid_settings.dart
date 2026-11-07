// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';

import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../printing/label_bitmap.dart' show defaultBarcodeUnitMm;
import 'printer_calibration.dart';

/// Supported print paper sizes. The screen maps these to `PdfPageFormat`.
enum PaperSize { a3, a4, a5, letter, legal }

/// How ID cards are tiled onto a printed page.
///
/// Mirrors the geometry fields of the Qt client's label-layout preset
/// (`columns`, `rows`, left/top margin and the gaps between labels) plus the
/// chosen paper size and orientation. The card size itself comes from the
/// collection's `id_card_layout`.
class LabelGridSettings {
  const LabelGridSettings({
    required this.columns,
    required this.rows,
    required this.leftMarginMm,
    required this.topMarginMm,
    required this.hGapMm,
    required this.vGapMm,
    required this.paperSize,
    required this.landscape,
    required this.drawBorder,
    this.barcodeUnitMm = defaultBarcodeUnitMm,
    this.calibration = const PrinterCalibration(),
  });

  /// Number of label columns per sheet.
  final int columns;

  /// Number of label rows per sheet.
  final int rows;

  /// Page left edge → left edge of the first label, in millimetres.
  final double leftMarginMm;

  /// Page top edge → top edge of the first label, in millimetres.
  final double topMarginMm;

  /// Horizontal gap between adjacent labels, in millimetres.
  final double hGapMm;

  /// Vertical gap between adjacent labels, in millimetres.
  final double vGapMm;

  /// Paper size of the printed sheet.
  final PaperSize paperSize;

  /// Whether the sheet is printed in landscape (vs. portrait) orientation.
  final bool landscape;

  /// Whether a thin border rectangle is drawn around each card.
  final bool drawBorder;

  /// Code128 X-dimension (narrowest bar width) in millimetres.
  final double barcodeUnitMm;

  /// Print-position correction for the sheet/system printer.
  final PrinterCalibration calibration;

  /// Sensible defaults for an ID-card sheet on A4 portrait.
  static const LabelGridSettings defaults = LabelGridSettings(
    columns: 2,
    rows: 5,
    leftMarginMm: 10.0,
    topMarginMm: 10.0,
    hGapMm: 5.0,
    vGapMm: 5.0,
    paperSize: PaperSize.a4,
    landscape: false,
    drawBorder: true,
  );

  LabelGridSettings copyWith({
    int? columns,
    int? rows,
    double? leftMarginMm,
    double? topMarginMm,
    double? hGapMm,
    double? vGapMm,
    PaperSize? paperSize,
    bool? landscape,
    bool? drawBorder,
    double? barcodeUnitMm,
    PrinterCalibration? calibration,
  }) {
    return LabelGridSettings(
      columns: columns ?? this.columns,
      rows: rows ?? this.rows,
      leftMarginMm: leftMarginMm ?? this.leftMarginMm,
      topMarginMm: topMarginMm ?? this.topMarginMm,
      hGapMm: hGapMm ?? this.hGapMm,
      vGapMm: vGapMm ?? this.vGapMm,
      paperSize: paperSize ?? this.paperSize,
      landscape: landscape ?? this.landscape,
      drawBorder: drawBorder ?? this.drawBorder,
      barcodeUnitMm: barcodeUnitMm ?? this.barcodeUnitMm,
      calibration: calibration ?? this.calibration,
    );
  }

  factory LabelGridSettings.fromJson(Map<String, dynamic> json) {
    return LabelGridSettings(
      columns: (json['columns'] as num?)?.toInt() ?? defaults.columns,
      rows: (json['rows'] as num?)?.toInt() ?? defaults.rows,
      leftMarginMm:
          (json['left_margin_mm'] as num?)?.toDouble() ?? defaults.leftMarginMm,
      topMarginMm:
          (json['top_margin_mm'] as num?)?.toDouble() ?? defaults.topMarginMm,
      hGapMm: (json['h_gap_mm'] as num?)?.toDouble() ?? defaults.hGapMm,
      vGapMm: (json['v_gap_mm'] as num?)?.toDouble() ?? defaults.vGapMm,
      paperSize: PaperSize.values.firstWhere(
        (p) => p.name == json['paper_size'],
        orElse: () => defaults.paperSize,
      ),
      landscape: json['landscape'] as bool? ?? defaults.landscape,
      drawBorder: json['draw_border'] as bool? ?? defaults.drawBorder,
      barcodeUnitMm: ((json['barcode_unit_mm'] as num?)?.toDouble() ??
              defaultBarcodeUnitMm)
          .clamp(0.15, 1.0),
      calibration: json['calibration'] is Map<String, dynamic>
          ? PrinterCalibration.fromJson(
              json['calibration'] as Map<String, dynamic>)
          : const PrinterCalibration(),
    );
  }

  Map<String, dynamic> toJson() => {
        'columns': columns,
        'rows': rows,
        'left_margin_mm': leftMarginMm,
        'top_margin_mm': topMarginMm,
        'h_gap_mm': hGapMm,
        'v_gap_mm': vGapMm,
        'paper_size': paperSize.name,
        'landscape': landscape,
        'draw_border': drawBorder,
        'barcode_unit_mm': barcodeUnitMm,
        'calibration': calibration.toJson(),
      };
}

/// Persists [LabelGridSettings] on the device via secure storage.
///
/// The settings are a per-device print preference, so they are stored locally
/// rather than on the server.
class LabelGridStore {
  static const _storage = FlutterSecureStorage();
  static const _key = 'id_card_grid_settings';

  /// Loads the stored settings, falling back to [LabelGridSettings.defaults].
  Future<LabelGridSettings> load() async {
    try {
      final raw = await _storage.read(key: _key);
      if (raw == null || raw.isEmpty) return LabelGridSettings.defaults;
      return LabelGridSettings.fromJson(
          jsonDecode(raw) as Map<String, dynamic>);
    } catch (_) {
      return LabelGridSettings.defaults;
    }
  }

  /// Saves [settings] to secure storage.
  Future<void> save(LabelGridSettings settings) async {
    await _storage.write(key: _key, value: jsonEncode(settings.toJson()));
  }
}
