// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';
import 'dart:math' as math;
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:barcode/barcode.dart' as bc;
import 'package:flutter/material.dart';

import '../models/id_card_layout.dart';

/// A 1-bit-per-pixel monochrome bitmap, rows packed MSB-first with bit value
/// 0 = black (printed) — the polarity the TSPL `BITMAP` command expects.
class MonoBitmap {
  MonoBitmap(this.widthDots, this.heightDots, this.data,
      {double? widthMm, double? heightMm})
      : widthBytes = (widthDots + 7) ~/ 8,
        widthMm = widthMm ?? 0,
        heightMm = heightMm ?? 0;

  final int widthDots;
  final int heightDots;
  final int widthBytes;
  final Uint8List data;

  /// Physical label size in millimetres (the source layout size), used for the
  /// TSPL `SIZE` command so it is not subject to dot-rounding.
  final double widthMm;
  final double heightMm;
}

/// One text-zone line plus its fitting policy. [maxLines] is how many display
/// lines it may wrap onto (1 for most fields, 2 for the title). [truncate] is
/// whether it may be shortened with an ellipsis when too wide — only the title
/// and author truncate; other fields are scaled down to fit instead.
class LabelTextLine {
  const LabelTextLine(this.text,
      {this.maxLines = 1, this.truncate = false, this.center = false});
  final String text;
  final int maxLines;
  final bool truncate;

  /// Whether the line is centred horizontally within the zone (signatures)
  /// rather than left-aligned (book-label fields).
  final bool center;
}

/// Placement and capability profile of one TSPL printer model. TSPL origin
/// handling differs between firmwares: the Vevor Y486 needs the print origin
/// shifted via `REFERENCE` and the bitmap indented to land on the label,
/// while the Munbyn RW403B prints edge-exact from the standard origin (both
/// verified on hardware; matches the desktop's `TsplDeviceProfile`).
class TsplProfile {
  const TsplProfile(
      {this.referenceXDots = 0, this.baseXMm = 0.0, this.epl2Density = false});

  /// TSPL `REFERENCE` x (dots) aligning the print head origin with the label.
  final int referenceXDots;

  /// Calibrated horizontal `BITMAP` base position (mm) at the neutral
  /// x-offset setting.
  final double baseXMm;

  /// Whether the job must set the darkness via the EPL2 density register
  /// (`\nN\nD<n>\n` prefix) instead of relying on TSPL `DENSITY`. The
  /// bilingual Y486 firmware accepts `DENSITY` but it has virtually no
  /// effect on the print; only the EPL2 register — the path its working
  /// CUPS driver uses — changes the darkness (verified on hardware).
  /// Never send the prefix to other models: the Munbyn RW403B discards
  /// jobs preceded by foreign commands.
  final bool epl2Density;

  /// Calibrated profile of the Vevor Y486 (USB id 5958:0486).
  static const vevorY486 =
      TsplProfile(referenceXDots: -64, baseXMm: 11.0, epl2Density: true);

  /// Returns the profile for the USB device [vendorId]:[productId]. Unknown
  /// models get the default profile (standard TSPL origin) — correct for
  /// printers such as the Munbyn RW403B (0d28:ccdd).
  static TsplProfile forDevice(int vendorId, int productId) {
    if (vendorId == 0x5958 && productId == 0x0486) return vevorY486;
    return const TsplProfile();
  }
}

/// Neutral x-offset setting in millimetres: the value at which the print sits
/// at the profile's calibrated base position.
const double _neutralXOffsetMm = 11.0;

/// Default Code128 X-dimension (module width) in millimetres, matching the
/// desktop default (`DefaultBarcodeUnitMm` in colman_code128_item.hpp). The
/// barcode is drawn at this module width and centred in its zone instead of
/// being stretched to the zone width: stretched modules leave the range
/// scanners expect, and centring keeps the spare zone width as quiet zone.
const double defaultBarcodeUnitMm = 0.33;

/// Total number of modules (narrowest-bar widths) in the Code128 symbol for
/// [data]: in a probe render the narrowest bar is exactly one module. Returns
/// 0 when no symbol can be derived.
int code128Modules(String data) {
  if (data.isEmpty) return 0;
  const probeW = 1000.0;
  var minBar = double.infinity;
  for (final e in bc.Barcode.code128().make(data, width: probeW, height: 10)) {
    if (e is bc.BarcodeBar && e.width < minBar) {
      minBar = e.width;
    }
  }
  if (!minBar.isFinite || minBar <= 0) return 0;
  return (probeW / minBar).round();
}

/// Renders a book-label [layout] (barcode + text + optional logo zones, each
/// positioned, sized and rotated like an ID card) to a monochrome bitmap.
///
/// The whole label is drawn with `dart:ui` and thresholded to 1-bit, mirroring
/// the desktop's scene→image→printer pipeline so logos and arbitrary zone
/// rotations are handled uniformly. When [rotate180] is set the content is
/// pre-rotated so it reads upright once printed in TSPL `DIRECTION 0` (whose
/// origin is the first-ejected edge).
Future<MonoBitmap> renderBookLabel({
  required IdCardLayout layout,
  required String barcode,
  required List<LabelTextLine> textLines,
  ui.Image? logo,
  required int dpi,
  bool rotate180 = true,
  String? fontFamily,
  double barWidthReductionMm = 0.0,
  double barcodeUnitMm = defaultBarcodeUnitMm,
}) async {
  final dotsPerMm = dpi / 25.4;
  final w = math.max(1, (layout.cardWidthMm * dotsPerMm).round());
  final h = math.max(1, (layout.cardHeightMm * dotsPerMm).round());
  final image = await _renderLayoutImage(
    layout: layout,
    barcode: barcode,
    textLines: textLines,
    logo: logo,
    dpi: dpi,
    rotate180: rotate180,
    fontFamily: fontFamily,
    barWidthReductionMm: barWidthReductionMm,
    barcodeUnitMm: barcodeUnitMm,
  );
  return _toMono(image, w, h,
      widthMm: layout.cardWidthMm, heightMm: layout.cardHeightMm);
}

/// Renders the [layout] to a PNG image (upright, no 180° pre-rotation) for the
/// PDF / system-printer fallback path.
Future<Uint8List> renderBookLabelPng({
  required IdCardLayout layout,
  required String barcode,
  required List<LabelTextLine> textLines,
  ui.Image? logo,
  required int dpi,
  String? fontFamily,
  double barWidthReductionMm = 0.0,
  double barcodeUnitMm = defaultBarcodeUnitMm,
}) async {
  final image = await _renderLayoutImage(
    layout: layout,
    barcode: barcode,
    textLines: textLines,
    logo: logo,
    dpi: dpi,
    rotate180: false,
    fontFamily: fontFamily,
    barWidthReductionMm: barWidthReductionMm,
    barcodeUnitMm: barcodeUnitMm,
  );
  final bd = await image.toByteData(format: ui.ImageByteFormat.png);
  return bd!.buffer.asUint8List();
}

/// Draws the [layout] (barcode, text, optional logo zones) onto an offscreen
/// image at [dpi]. When [rotate180] the content is pre-rotated for TSPL
/// `DIRECTION 0`.
Future<ui.Image> _renderLayoutImage({
  required IdCardLayout layout,
  required String barcode,
  required List<LabelTextLine> textLines,
  ui.Image? logo,
  required int dpi,
  required bool rotate180,
  String? fontFamily,
  double barWidthReductionMm = 0.0,
  double barcodeUnitMm = defaultBarcodeUnitMm,
}) async {
  final dotsPerMm = dpi / 25.4;
  final reductionDots = barWidthReductionMm * dotsPerMm;
  final w = math.max(1, (layout.cardWidthMm * dotsPerMm).round());
  final h = math.max(1, (layout.cardHeightMm * dotsPerMm).round());

  final recorder = ui.PictureRecorder();
  final canvas = Canvas(recorder);
  canvas.drawRect(Rect.fromLTWH(0, 0, w.toDouble(), h.toDouble()),
      Paint()..color = const Color(0xFFFFFFFF));

  canvas.save();
  if (rotate180) {
    canvas.translate(w.toDouble(), h.toDouble());
    canvas.rotate(math.pi);
  }

  void inZone(IdCardElement e, void Function(double zw, double zh) draw) {
    if (e.widthMm <= 0 || e.heightMm <= 0) return;
    final zw = e.widthMm * dotsPerMm;
    final zh = e.heightMm * dotsPerMm;
    canvas.save();
    canvas.translate(e.xMm * dotsPerMm, e.yMm * dotsPerMm);
    // Rotate the zone about its centre, keeping its on-label footprint.
    final odd = (e.orientation ~/ 90) % 2 != 0;
    final natW = odd ? zh : zw;
    final natH = odd ? zw : zh;
    canvas.translate(zw / 2, zh / 2);
    canvas.rotate(e.orientation * math.pi / 180.0);
    canvas.translate(-natW / 2, -natH / 2);
    draw(natW, natH);
    canvas.restore();
  }

  inZone(layout.barcode, (zw, zh) {
    _drawBarcode(canvas, barcode, zw, zh, fontFamily, reductionDots, dotsPerMm,
        barcodeUnitMm);
  });
  inZone(layout.text, (zw, zh) {
    _drawTextBlock(canvas, textLines, zw, zh, fontFamily);
  });
  if (layout.hasLogo && logo != null) {
    inZone(layout.logo, (zw, zh) {
      _drawImageContain(canvas, logo, zw, zh);
    });
  }

  canvas.restore();
  return recorder.endRecording().toImage(w, h);
}

/// Renders a signature label (label size + a single positioned text zone, no
/// barcode or logo) to a monochrome bitmap.
Future<MonoBitmap> renderSignatureLabel({
  required double widthMm,
  required double heightMm,
  required IdCardElement textZone,
  required List<LabelTextLine> textLines,
  required int dpi,
  bool rotate180 = true,
  String? fontFamily,
}) {
  final layout = IdCardLayout(
    cardWidthMm: widthMm,
    cardHeightMm: heightMm,
    hasLogo: false,
    barcode: const IdCardElement(),
    text: textZone,
    logo: const IdCardElement(),
  );
  return renderBookLabel(
    layout: layout,
    barcode: '',
    textLines: textLines,
    dpi: dpi,
    rotate180: rotate180,
    fontFamily: fontFamily,
  );
}

void _drawBarcode(Canvas canvas, String data, double w, double h,
    String? fontFamily, double reductionDots, double dotsPerMm,
    double unitMm) {
  if (data.isEmpty || w <= 0 || h <= 0) return;
  final paint = Paint()..color = const Color(0xFF000000);
  final fontHeight = math.min(h * 0.2, 14.0 * (h / 50));
  final code = bc.Barcode.code128();
  final modules = code128Modules(data);
  var drawW = w;
  if (modules > 0) {
    // Snap the module to whole printer dots so 1-bit thresholding keeps all
    // bars even; shrink it only when the zone cannot hold the fixed
    // X-dimension. If even one dot per module does not fit, keep the
    // stretched zone width.
    var module = math.max(1.0, (unitMm * dotsPerMm).roundToDouble());
    if (modules * module > w) {
      module = (w / modules).floorToDouble();
    }
    if (module >= 1.0) {
      drawW = modules * module;
    }
  }
  final xOff = (w - drawW) / 2;
  // Shrink each black bar symmetrically to compensate for thermal ink spread.
  final shrink = reductionDots.clamp(0.0, 4.0);
  for (final e in code.make(data,
      width: drawW, height: h, drawText: true, fontHeight: fontHeight)) {
    if (e is bc.BarcodeBar && e.black) {
      final bw = math.max(0.2, e.width - shrink);
      canvas.drawRect(
          Rect.fromLTWH(
              xOff + e.left + (e.width - bw) / 2, e.top, bw, e.height),
          paint);
    } else if (e is bc.BarcodeText) {
      final tp = TextPainter(
        text: TextSpan(
          text: e.text,
          style: TextStyle(
              color: const Color(0xFF000000),
              fontSize: e.height,
              fontFamily: fontFamily),
        ),
        textAlign: TextAlign.center,
        textDirection: TextDirection.ltr,
      )..layout(minWidth: e.width, maxWidth: e.width);
      tp.paint(canvas, Offset(xOff + e.left, e.top));
    }
  }
}

void _drawImageContain(Canvas canvas, ui.Image img, double w, double h) {
  final src =
      Rect.fromLTWH(0, 0, img.width.toDouble(), img.height.toDouble());
  final scale = math.min(w / img.width, h / img.height);
  final dw = img.width * scale;
  final dh = img.height * scale;
  final dst = Rect.fromLTWH((w - dw) / 2, (h - dh) / 2, dw, dh);
  canvas.drawImageRect(img, src, dst, Paint());
}

/// Draws each line in its own equal-height band, left-aligned. Lines are sized
/// independently (a long author no longer shrinks the title): each fills its
/// band height, and width overflow is handled per line by word-boundary
/// truncation with an ellipsis. A line allowed two lines (e.g. the title) is
/// word-wrapped onto two lines before being ellipsised.
void _drawTextBlock(Canvas canvas, List<LabelTextLine> lines, double w,
    double h, String? fontFamily) {
  final visible = lines.where((l) => l.text.trim().isNotEmpty).toList();
  if (visible.isEmpty || w <= 0 || h <= 0) return;
  final bandH = h / visible.length;
  // Cap the font so short lines do not become oversized in a tall band. Single
  // lines (e.g. signatures) may therefore fill up to this fraction of the zone.
  final maxFont = h * 0.6;
  var y = 0.0;
  for (final line in visible) {
    _drawFittedLine(canvas, line, w, bandH, fontFamily, y, maxFont);
    y += bandH;
  }
}

/// Draws [line] centred vertically in a [w]×[bandH] band. The font fills the
/// band height (capped at [maxFont]). A line that does not fit the width is
/// scaled down to fit — unless it is allowed to truncate (title/author), in
/// which case it is word-truncated with an ellipsis (the title across two
/// lines).
void _drawFittedLine(Canvas canvas, LabelTextLine line, double w, double bandH,
    String? fontFamily, double yTop, double maxFont) {
  final text = line.text;
  final baseSize = math.min(bandH * 0.82, maxFont);
  final natural = _layout(text, double.infinity, baseSize, 1, fontFamily);
  final TextPainter tp;
  if (natural.width <= w) {
    tp = _layout(text, w, baseSize, 1, fontFamily);
  } else if (!line.truncate) {
    // Scale down to fit the width without cutting anything off.
    final size = baseSize * (w / natural.width) * 0.98;
    tp = _layout(text, w, size, 1, fontFamily);
  } else if (line.maxLines >= 2) {
    tp = _fitLine(text, w, math.min(bandH * 0.42, maxFont), 2, fontFamily);
  } else {
    tp = _fitLine(text, w, baseSize, 1, fontFamily);
  }
  final x = line.center ? (w - tp.width) / 2 : 0.0;
  tp.paint(canvas, Offset(x, yTop + (bandH - tp.height) / 2));
}

TextPainter _layout(
    String text, double maxWidth, double fontSize, int maxLines, String? ff) {
  return TextPainter(
    text: TextSpan(
      text: text,
      style: TextStyle(
          color: const Color(0xFF000000), fontSize: fontSize, fontFamily: ff),
    ),
    maxLines: maxLines,
    textDirection: TextDirection.ltr,
  )..layout(maxWidth: maxWidth);
}

/// Lays out [text] within [maxLines], truncating at word boundaries with an
/// ellipsis when it does not fit.
TextPainter _fitLine(
    String text, double maxWidth, double fontSize, int maxLines, String? ff) {
  final full = _layout(text, maxWidth, fontSize, maxLines, ff);
  if (!full.didExceedMaxLines) return full;
  final words = text.trim().split(RegExp(r'\s+'));
  for (var k = words.length - 1; k >= 1; k--) {
    final candidate = _layout(
        '${words.take(k).join(' ')}…', maxWidth, fontSize, maxLines, ff);
    if (!candidate.didExceedMaxLines) return candidate;
  }
  // Even one word overflows: fall back to a hard ellipsis.
  return TextPainter(
    text: TextSpan(
      text: text,
      style: TextStyle(
          color: const Color(0xFF000000), fontSize: fontSize, fontFamily: ff),
    ),
    maxLines: maxLines,
    ellipsis: '…',
    textDirection: TextDirection.ltr,
  )..layout(maxWidth: maxWidth);
}

/// Thresholds an RGBA [image] to 1-bit (0 bit = black), packed MSB-first.
Future<MonoBitmap> _toMono(ui.Image image, int w, int h,
    {double? widthMm, double? heightMm}) async {
  final bd = await image.toByteData(format: ui.ImageByteFormat.rawRgba);
  final rgba = bd!.buffer.asUint8List();
  final widthBytes = (w + 7) ~/ 8;
  final out = Uint8List(widthBytes * h)..fillRange(0, widthBytes * h, 0xFF);
  for (var y = 0; y < h; y++) {
    for (var x = 0; x < w; x++) {
      final i = (y * w + x) * 4;
      final r = rgba[i], g = rgba[i + 1], b = rgba[i + 2], a = rgba[i + 3];
      // Composite onto white, then threshold luminance.
      final lum = a == 0
          ? 255
          : (0.299 * r + 0.587 * g + 0.114 * b) * a ~/ 255 + (255 - a);
      if (lum < 128) {
        out[y * widthBytes + (x >> 3)] &= ~(0x80 >> (x & 7));
      }
    }
  }
  return MonoBitmap(w, h, out, widthMm: widthMm, heightMm: heightMm);
}

/// Wraps already-rendered [bitmaps] into a single TSPL job: the printer is set
/// up once (size/gap/density/direction/reference), then each label is a
/// `CLS … BITMAP … PRINT 1` block.
///
/// The bitmap is drawn in mode 1 (OR): onto the CLS-cleared buffer this
/// produces exactly the overwrite result, but unlike mode 0 it works on every
/// tested firmware — the Munbyn RW403B silently discards whole jobs whose
/// `BITMAP` uses mode 0 (verified on hardware).
Uint8List labelsToTsplBitmap(
  List<MonoBitmap> bitmaps, {
  required int dpi,
  required double gapMm,
  required int darkness,
  required double xOffsetMm,
  TsplProfile profile = const TsplProfile(),
}) {
  final dotsPerMm = dpi / 25.4;
  // The x-offset reads intuitively (a larger value moves content further right
  // in reading direction). 11 mm is the neutral position; the bitmap x
  // decreases as the offset grows, which — with the 180° content rotation —
  // shifts the print to the right.
  final xDots = math.max(0,
      ((_neutralXOffsetMm + profile.baseXMm - xOffsetMm) * dotsPerMm).round());
  final density = darkness.clamp(0, 15);
  // The physical label size follows the rendered layout so SIZE always matches
  // the printed image (using the exact mm, free of dot-rounding).
  final widthMm = bitmaps.isEmpty ? 0.0 : bitmaps.first.widthMm;
  final heightMm = bitmaps.isEmpty ? 0.0 : bitmaps.first.heightMm;

  final header = StringBuffer()
    ..write('SIZE ${_mm(widthMm)} mm,${_mm(heightMm)} mm\r\n')
    ..write('GAP ${_mm(gapMm)} mm,0 mm\r\n')
    ..write('DIRECTION 0\r\n')
    ..write('REFERENCE ${profile.referenceXDots},0\r\n')
    ..write('DENSITY $density\r\n');

  final bytes = <int>[
    if (profile.epl2Density) ...latin1.encode('\nN\nD$density\n'),
    ...latin1.encode(header.toString()),
  ];
  for (final bmp in bitmaps) {
    bytes.addAll(latin1.encode('CLS\r\n'));
    bytes.addAll(latin1.encode(
        'BITMAP $xDots,0,${bmp.widthBytes},${bmp.heightDots},1,'));
    bytes.addAll(bmp.data);
    bytes.addAll(latin1.encode('\r\nPRINT 1\r\n'));
  }
  return Uint8List.fromList(bytes);
}

String _mm(double v) =>
    v == v.roundToDouble() ? v.toStringAsFixed(0) : v.toStringAsFixed(1);
