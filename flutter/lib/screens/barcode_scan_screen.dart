// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:mobile_scanner/mobile_scanner.dart';

import '../l10n/app_localizations.dart';
import '../widgets/app_bar_title.dart';

/// Full-screen camera view that scans a barcode and returns its raw value.
///
/// Push this screen and await the result:
/// ```dart
/// final code = await Navigator.of(context).push<String>(
///   MaterialPageRoute(builder: (_) => const BarcodeScanScreen()),
/// );
/// ```
/// Returns `null` if the user closes the screen without a successful scan.
class BarcodeScanScreen extends StatefulWidget {
  /// Creates a [BarcodeScanScreen].
  const BarcodeScanScreen({super.key});

  @override
  State<BarcodeScanScreen> createState() => _BarcodeScanScreenState();
}

class _BarcodeScanScreenState extends State<BarcodeScanScreen> {
  final _controller = MobileScannerController(
    formats: [
      BarcodeFormat.ean13,
      BarcodeFormat.ean8,
      BarcodeFormat.upcA,
      BarcodeFormat.upcE,
      BarcodeFormat.qrCode,
      BarcodeFormat.code128,
      BarcodeFormat.code39,
    ],
  );

  bool _detected = false;

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _onDetect(BarcodeCapture capture) {
    if(_detected) return;
    final raw = capture.barcodes.firstOrNull?.rawValue;
    if(raw == null || raw.isEmpty) return;
    _detected = true;
    Navigator.of(context).pop(raw);
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/scan_barcode.jpeg',
          title: l10n.barcodeScanTitle,
        ),
      ),
      body: Stack(
        children: [
          MobileScanner(
            controller: _controller,
            onDetect: _onDetect,
          ),
          _ScanOverlay(),
          Align(
            alignment: Alignment.bottomCenter,
            child: Padding(
              padding: const EdgeInsets.fromLTRB(24, 0, 24, 48),
              child: Text(
                l10n.barcodeScanHint,
                style: const TextStyle(
                  color: Colors.white,
                  fontSize: 16,
                  shadows: [Shadow(blurRadius: 6)],
                ),
                textAlign: TextAlign.center,
              ),
            ),
          ),
        ],
      ),
    );
  }
}

/// Semi-transparent overlay with a centred transparent scan window.
class _ScanOverlay extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
        final w = constraints.maxWidth;
        final h = constraints.maxHeight;
        const windowW = 260.0;
        const windowH = 140.0;
        final left = (w - windowW) / 2;
        final top = (h - windowH) / 2;

        return Stack(
          children: [
            // Dim everything outside the scan window.
            Positioned.fill(
              child: CustomPaint(
                painter: _DimPainter(
                  scanRect: Rect.fromLTWH(left, top, windowW, windowH),
                ),
              ),
            ),
            // Corner markers around the scan window.
            Positioned(
              left: left,
              top: top,
              width: windowW,
              height: windowH,
              child: const _CornerFrame(),
            ),
          ],
        );
      },
    );
  }
}

/// Paints a semi-transparent overlay with a clear cut-out for [scanRect].
class _DimPainter extends CustomPainter {
  const _DimPainter({required this.scanRect});

  final Rect scanRect;

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()..color = Colors.black54;
    final full = Rect.fromLTWH(0, 0, size.width, size.height);
    canvas.drawPath(
      Path.combine(
        PathOperation.difference,
        Path()..addRect(full),
        Path()
          ..addRRect(
            RRect.fromRectAndRadius(scanRect, const Radius.circular(8)),
          ),
      ),
      paint,
    );
  }

  @override
  bool shouldRepaint(_DimPainter old) => old.scanRect != scanRect;
}

/// Four corner brackets drawn around the scan window.
class _CornerFrame extends StatelessWidget {
  const _CornerFrame();

  @override
  Widget build(BuildContext context) {
    const color = Colors.white;
    const len = 20.0;
    const thick = 3.0;
    return Stack(
      children: [
        // Top-left
        Positioned(
          left: 0, top: 0,
          child: _Corner(color: color, length: len, thickness: thick,
              top: true, left: true),
        ),
        // Top-right
        Positioned(
          right: 0, top: 0,
          child: _Corner(color: color, length: len, thickness: thick,
              top: true, left: false),
        ),
        // Bottom-left
        Positioned(
          left: 0, bottom: 0,
          child: _Corner(color: color, length: len, thickness: thick,
              top: false, left: true),
        ),
        // Bottom-right
        Positioned(
          right: 0, bottom: 0,
          child: _Corner(color: color, length: len, thickness: thick,
              top: false, left: false),
        ),
      ],
    );
  }
}

/// A single L-shaped corner bracket.
class _Corner extends StatelessWidget {
  const _Corner({
    required this.color,
    required this.length,
    required this.thickness,
    required this.top,
    required this.left,
  });

  final Color color;
  final double length;
  final double thickness;
  final bool top;
  final bool left;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: length,
      height: length,
      child: CustomPaint(
        painter: _CornerPainter(
          color: color,
          thickness: thickness,
          top: top,
          left: left,
        ),
      ),
    );
  }
}

class _CornerPainter extends CustomPainter {
  const _CornerPainter({
    required this.color,
    required this.thickness,
    required this.top,
    required this.left,
  });

  final Color color;
  final double thickness;
  final bool top;
  final bool left;

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = color
      ..strokeWidth = thickness
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.square;

    final w = size.width;
    final h = size.height;

    // Horizontal arm.
    final hY = top ? 0.0 : h;
    final hX0 = left ? 0.0 : w;
    final hX1 = left ? w : 0.0;
    canvas.drawLine(Offset(hX0, hY), Offset(hX1, hY), paint);

    // Vertical arm.
    final vX = left ? 0.0 : w;
    final vY0 = top ? 0.0 : h;
    final vY1 = top ? h : 0.0;
    canvas.drawLine(Offset(vX, vY0), Offset(vX, vY1), paint);
  }

  @override
  bool shouldRepaint(_CornerPainter old) =>
      old.color != color ||
      old.thickness != thickness ||
      old.top != top ||
      old.left != left;
}
