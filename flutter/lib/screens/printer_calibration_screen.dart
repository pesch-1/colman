// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:pdf/pdf.dart';
import 'package:printing/printing.dart';

import '../models/printer_calibration.dart';
import '../printing/calibration_sheet_pdf.dart';
import '../widgets/app_bar_title.dart';

/// Guided print-position calibration for a normal (sheet/system) printer.
///
/// Mirrors the desktop `PrinterCalibrationWidget`: print a reference sheet with
/// three crosses, measure their actual positions with a ruler, and compute the
/// translation/scale correction that compensates the printer's error. The
/// result is handed back through [onSave]; the caller persists it in the
/// relevant device-local preset.
class PrinterCalibrationScreen extends StatefulWidget {
  const PrinterCalibrationScreen({
    super.key,
    required this.initial,
    required this.onSave,
  });

  /// The calibration to pre-fill (identity when none was stored yet).
  final PrinterCalibration initial;

  /// Persists the saved calibration; called before the screen pops.
  final Future<void> Function(PrinterCalibration) onSave;

  @override
  State<PrinterCalibrationScreen> createState() =>
      _PrinterCalibrationScreenState();
}

class _PrinterCalibrationScreenState extends State<PrinterCalibrationScreen> {
  final _aX = TextEditingController();
  final _aY = TextEditingController();
  final _bX = TextEditingController();
  final _cY = TextEditingController();

  late PrinterCalibration _result = widget.initial;
  bool _printing = false;

  @override
  void initState() {
    super.initState();
    // Pre-fill the measurement fields with the values that reproduce the stored
    // calibration, so re-opening the screen shows a consistent starting point.
    _aX.text = _mm(PrinterCalibration.refXMm);
    _aY.text = _mm(PrinterCalibration.refYMm);
    _bX.text = _mm(PrinterCalibration.refXMm + PrinterCalibration.spanXMm);
    _cY.text = _mm(PrinterCalibration.refYMm + PrinterCalibration.spanYMm);
  }

  @override
  void dispose() {
    for (final c in [_aX, _aY, _bX, _cY]) {
      c.dispose();
    }
    super.dispose();
  }

  String _mm(double v) =>
      v == v.roundToDouble() ? v.toStringAsFixed(0) : v.toStringAsFixed(1);

  double _read(TextEditingController c, double fallback) =>
      double.tryParse(c.text.trim().replaceAll(',', '.')) ?? fallback;

  Future<void> _printSheet() async {
    setState(() => _printing = true);
    try {
      final pdf = await buildCalibrationSheetPdf();
      await Printing.layoutPdf(
        format: PdfPageFormat.a4,
        onLayout: (_) async => pdf,
      );
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context)
            .showSnackBar(SnackBar(content: Text('$e')));
      }
    } finally {
      if (mounted) setState(() => _printing = false);
    }
  }

  void _compute() {
    setState(() {
      _result = PrinterCalibration.fromMeasurements(
        aXMm: _read(_aX, PrinterCalibration.refXMm),
        aYMm: _read(_aY, PrinterCalibration.refYMm),
        bXMm: _read(_bX, PrinterCalibration.refXMm + PrinterCalibration.spanXMm),
        cYMm: _read(_cY, PrinterCalibration.refYMm + PrinterCalibration.spanYMm),
      );
    });
  }

  Future<void> _save(AppLocalizations l10n) async {
    await widget.onSave(_result);
    if (!mounted) return;
    ScaffoldMessenger.of(context)
        .showSnackBar(SnackBar(content: Text(l10n.calibrationSaved)));
    Navigator.of(context).pop();
  }

  void _reset() {
    setState(() {
      _result = const PrinterCalibration();
      _aX.text = _mm(PrinterCalibration.refXMm);
      _aY.text = _mm(PrinterCalibration.refYMm);
      _bX.text = _mm(PrinterCalibration.refXMm + PrinterCalibration.spanXMm);
      _cY.text = _mm(PrinterCalibration.refYMm + PrinterCalibration.spanYMm);
    });
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/printer_calibration.jpeg',
          title: l10n.calibrationTitle,
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.save),
            tooltip: l10n.calibrationSave,
            onPressed: () => _save(l10n),
          ),
        ],
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          Text(l10n.calibrationIntro),
          const SizedBox(height: 16),
          // Step 1: print the reference sheet.
          FilledButton.tonalIcon(
            onPressed: _printing ? null : _printSheet,
            icon: const Icon(Icons.print_outlined),
            label: Text(l10n.calibrationPrintSheet),
          ),
          if (_printing) ...[
            const SizedBox(height: 8),
            const LinearProgressIndicator(),
          ],
          const SizedBox(height: 24),
          // Step 2: enter the measured positions.
          Text(l10n.calibrationMeasureHeading,
              style: Theme.of(context).textTheme.titleMedium),
          const SizedBox(height: 4),
          Text(l10n.calibrationMeasureHint),
          const SizedBox(height: 12),
          Row(children: [
            Expanded(child: _numField(_aX, l10n.calibrationMeasureAX)),
            const SizedBox(width: 12),
            Expanded(child: _numField(_aY, l10n.calibrationMeasureAY)),
          ]),
          const SizedBox(height: 12),
          Row(children: [
            Expanded(child: _numField(_bX, l10n.calibrationMeasureBX)),
            const SizedBox(width: 12),
            Expanded(child: _numField(_cY, l10n.calibrationMeasureCY)),
          ]),
          const SizedBox(height: 16),
          FilledButton.icon(
            onPressed: _compute,
            icon: const Icon(Icons.calculate_outlined),
            label: Text(l10n.calibrationCompute),
          ),
          const SizedBox(height: 24),
          // Step 3: review and save the computed correction.
          Text(l10n.calibrationResultHeading,
              style: Theme.of(context).textTheme.titleMedium),
          const SizedBox(height: 8),
          _resultRow(l10n.calibrationOffsetX, '${_mm(_result.offsetXMm)} mm'),
          _resultRow(l10n.calibrationOffsetY, '${_mm(_result.offsetYMm)} mm'),
          _resultRow(l10n.calibrationScaleX, _result.scaleX.toStringAsFixed(4)),
          _resultRow(l10n.calibrationScaleY, _result.scaleY.toStringAsFixed(4)),
          const SizedBox(height: 24),
          Row(children: [
            Expanded(
              child: OutlinedButton.icon(
                onPressed: _reset,
                icon: const Icon(Icons.restart_alt),
                label: Text(l10n.calibrationReset),
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: FilledButton.icon(
                onPressed: () => _save(l10n),
                icon: const Icon(Icons.save),
                label: Text(l10n.calibrationSave),
              ),
            ),
          ]),
        ],
      ),
    );
  }

  Widget _numField(TextEditingController controller, String label) {
    return TextField(
      controller: controller,
      keyboardType: const TextInputType.numberWithOptions(decimal: true),
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
        isDense: true,
        suffixText: 'mm',
      ),
    );
  }

  Widget _resultRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label),
          Text(value, style: const TextStyle(fontWeight: FontWeight.bold)),
        ],
      ),
    );
  }
}
