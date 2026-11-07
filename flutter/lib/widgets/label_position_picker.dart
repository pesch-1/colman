// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';

/// A grid of the label positions on one sheet (columns × rows). Tapping a cell
/// toggles whether it will be printed on, letting the user skip already-used
/// positions on a partly used sheet. Mirrors the desktop `LabelSelectionWidget`.
///
/// Controlled widget: [selected] holds the cell indices (row-major, 0-based)
/// that will receive a label; [onToggle] is called with a cell index on tap.
class LabelPositionPicker extends StatelessWidget {
  const LabelPositionPicker({
    super.key,
    required this.columns,
    required this.rows,
    required this.selected,
    required this.onToggle,
    this.aspectRatio = 2.0,
  });

  final int columns;
  final int rows;
  final Set<int> selected;
  final void Function(int cell) onToggle;

  /// Width/height ratio of a single label, for a faithful preview.
  final double aspectRatio;

  @override
  Widget build(BuildContext context) {
    final cols = columns < 1 ? 1 : columns;
    final count = cols * (rows < 1 ? 1 : rows);
    final scheme = Theme.of(context).colorScheme;
    return GridView.builder(
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
        crossAxisCount: cols,
        childAspectRatio: aspectRatio.clamp(0.3, 6.0),
        mainAxisSpacing: 4,
        crossAxisSpacing: 4,
      ),
      itemCount: count,
      itemBuilder: (ctx, i) {
        final on = selected.contains(i);
        return InkWell(
          onTap: () => onToggle(i),
          child: Container(
            decoration: BoxDecoration(
              borderRadius: BorderRadius.circular(3),
              border: Border.all(
                color: on ? scheme.primary : scheme.outlineVariant,
                width: on ? 2 : 1,
              ),
              color: on ? scheme.primary.withValues(alpha: 0.15) : null,
            ),
            child: Center(
              child: Text(
                '${i + 1}',
                style: TextStyle(
                  fontSize: 11,
                  color: on ? scheme.primary : scheme.onSurfaceVariant,
                ),
              ),
            ),
          ),
        );
      },
    );
  }
}
