// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Print-position correction for a normal (sheet/system) printer.
///
/// Mirrors the desktop client's `PrinterCalibration`: a per-axis translation
/// (in millimetres) and scale that compensate for a printer's print-position
/// error. A default-constructed value (zero offset, unit scale) is the identity
/// transform and leaves the output unchanged. Only the sheet/PDF print paths
/// use it; thermal printing keeps its own `xOffsetMm` and is not calibrated.
class PrinterCalibration {
  const PrinterCalibration({
    this.offsetXMm = 0.0,
    this.offsetYMm = 0.0,
    this.scaleX = 1.0,
    this.scaleY = 1.0,
  });

  /// Horizontal translation correction in millimetres.
  final double offsetXMm;

  /// Vertical translation correction in millimetres.
  final double offsetYMm;

  /// Horizontal scale correction (1.0 = no change).
  final double scaleX;

  /// Vertical scale correction (1.0 = no change).
  final double scaleY;

  // Nominal positions of the three reference crosses on the calibration sheet
  // (millimetres from the paper corner), shared with the test-sheet builder.
  static const double refXMm = 50.0;
  static const double refYMm = 50.0;
  static const double spanXMm = 100.0;
  static const double spanYMm = 100.0;

  /// Whether this is the identity transform (no correction).
  bool get isIdentity =>
      offsetXMm == 0.0 && offsetYMm == 0.0 && scaleX == 1.0 && scaleY == 1.0;

  /// Computes the correction from the measured cross positions on a printed
  /// calibration sheet (in millimetres from the paper corner).
  ///
  /// Ports the desktop algorithm (see `colman_printer_calibration_widget.cpp`):
  /// the printer error transform `printed = scale_p * nominal + trans_p` is
  /// derived from the measured A/B/C crosses, then inverted into the correction
  /// applied before printing. A degenerate measurement (zero span) falls back
  /// to the identity on that axis.
  factory PrinterCalibration.fromMeasurements({
    required double aXMm,
    required double aYMm,
    required double bXMm,
    required double cYMm,
  }) {
    // Correction for one axis: invert the printer error transform
    // `printed = scaleP * nominal + transP`. Returns (offset, scale).
    (double, double) axis(double measuredA, double measuredFar,
        double nominalRef, double nominalSpan) {
      final scaleP = (measuredFar - measuredA) / nominalSpan;
      if (scaleP == 0.0) return (0.0, 1.0); // degenerate: leave axis unchanged
      final transP = measuredA - scaleP * nominalRef;
      final scale = 1.0 / scaleP;
      return (-transP * scale, scale);
    }

    final (offsetX, scaleX) = axis(aXMm, bXMm, refXMm, spanXMm);
    final (offsetY, scaleY) = axis(aYMm, cYMm, refYMm, spanYMm);
    return PrinterCalibration(
      offsetXMm: offsetX,
      offsetYMm: offsetY,
      scaleX: scaleX,
      scaleY: scaleY,
    );
  }

  PrinterCalibration copyWith({
    double? offsetXMm,
    double? offsetYMm,
    double? scaleX,
    double? scaleY,
  }) {
    return PrinterCalibration(
      offsetXMm: offsetXMm ?? this.offsetXMm,
      offsetYMm: offsetYMm ?? this.offsetYMm,
      scaleX: scaleX ?? this.scaleX,
      scaleY: scaleY ?? this.scaleY,
    );
  }

  factory PrinterCalibration.fromJson(Map<String, dynamic> json) {
    return PrinterCalibration(
      offsetXMm: (json['offset_x_mm'] as num?)?.toDouble() ?? 0.0,
      offsetYMm: (json['offset_y_mm'] as num?)?.toDouble() ?? 0.0,
      scaleX: (json['scale_x'] as num?)?.toDouble() ?? 1.0,
      scaleY: (json['scale_y'] as num?)?.toDouble() ?? 1.0,
    );
  }

  Map<String, dynamic> toJson() => {
        'offset_x_mm': offsetXMm,
        'offset_y_mm': offsetYMm,
        'scale_x': scaleX,
        'scale_y': scaleY,
      };
}
