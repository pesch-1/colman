// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:printing/printing.dart';

import '../models/label_preset.dart';
import '../printing/label_sheet_pdf.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/label_position_picker.dart';

/// Second step of sheet printing: after items are chosen, the user picks which
/// positions on the sheet to use (at most as many as remaining items), prints
/// one sheet, and repeats for any remaining items.
class SheetPositionsScreen extends StatefulWidget {
  const SheetPositionsScreen({
    super.key,
    required this.preset,
    required this.itemIds,
    required this.renderPng,
  });

  final LabelPreset preset;
  final List<int> itemIds;

  /// Renders the label PNG for an item id.
  final Future<Uint8List> Function(int id) renderPng;

  @override
  State<SheetPositionsScreen> createState() => _SheetPositionsScreenState();
}

class _SheetPositionsScreenState extends State<SheetPositionsScreen> {
  late final List<int> _remaining = List.of(widget.itemIds);
  final Set<int> _cells = {};
  bool _printing = false;

  Future<void> _printSheet(AppLocalizations l10n) async {
    final cells = _cells.toList()..sort();
    final count = cells.length < _remaining.length ? cells.length : _remaining.length;
    if(count == 0) return;
    final ids = _remaining.take(count).toList();
    setState(() { _printing = true; });
    try {
      final pngs = <Uint8List>[];
      for(final id in ids) {
        pngs.add(await widget.renderPng(id));
      }
      final pdf = await buildLabelSheetPdf(
        labelPngs: pngs,
        preset: widget.preset,
        cellOrder: cells.take(count).toList(),
      );
      final printed = await Printing.layoutPdf(
        format: sheetPageFormat(widget.preset),
        onLayout: (_) async => pdf,
      );
      if(!mounted) return;
      setState(() {
        _printing = false;
        if(printed) {
          _remaining.removeRange(0, count);
          _cells.clear();
        }
      });
      if(printed && _remaining.isEmpty && mounted) {
        Navigator.of(context).pop();
      }
    } catch(e) {
      if(!mounted) return;
      setState(() { _printing = false; });
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('$e')));
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final p = widget.preset;
    final limit = _remaining.length;
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/print_label.jpeg',
          title: l10n.labelPrintPositions,
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.print_outlined),
            tooltip: l10n.labelPrintButton,
            onPressed:
                (_printing || _cells.isEmpty) ? null : () => _printSheet(l10n),
          ),
        ],
      ),
      body: ListView(
        padding: const EdgeInsets.all(12),
        children: [
          Text(l10n.labelPrintRemaining(_remaining.length)),
          const SizedBox(height: 4),
          Text(l10n.labelPrintPositionsHint),
          const SizedBox(height: 8),
          Row(
            children: [
              Text(l10n.labelPrintCellsSelected(_cells.length, limit)),
              const Spacer(),
              TextButton(
                onPressed: () => setState(() {
                  _cells.clear();
                  final total = p.columns * p.rows;
                  for(var i = 0; i < total && _cells.length < limit; i++) {
                    _cells.add(i);
                  }
                }),
                child: Text(l10n.labelPrintSelectAll),
              ),
              TextButton(
                onPressed: () => setState(_cells.clear),
                child: Text(l10n.labelPrintDeselectAll),
              ),
            ],
          ),
          const SizedBox(height: 8),
          if(_printing) const LinearProgressIndicator(),
          LabelPositionPicker(
            columns: p.columns,
            rows: p.rows,
            selected: _cells,
            aspectRatio:
                p.labelHeightMm <= 0 ? 2.0 : p.labelWidthMm / p.labelHeightMm,
            onToggle: (cell) => setState(() {
              if(_cells.contains(cell)) {
                _cells.remove(cell);
              } else if(_cells.length < limit) {
                _cells.add(cell);
              } else {
                ScaffoldMessenger.of(context).showSnackBar(
                  SnackBar(content: Text(l10n.labelPrintTooManyCells)),
                );
              }
            }),
          ),
        ],
      ),
    );
  }
}
