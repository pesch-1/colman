// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';

/// A labelled dropdown for selecting a school class.
///
/// Renders consistently with the `OutlineInputBorder` text fields used
/// throughout the person form. The first item is always an empty-string
/// sentinel representing "no class". If [value] is non-empty and not
/// present in [classes], it is appended so the widget never crashes.
class ClassDropdown extends StatelessWidget {
  const ClassDropdown({
    super.key,
    required this.label,
    required this.classes,
    required this.value,
    required this.onChanged,
    this.enabled = true,
  });

  final String label;
  final List<String> classes;

  /// Currently selected class name, or empty string for "no class".
  final String value;

  final ValueChanged<String> onChanged;
  final bool enabled;

  @override
  Widget build(BuildContext context) {
    final extraItem =
        value.isNotEmpty && !classes.contains(value) ? value : null;

    final items = <DropdownMenuItem<String>>[
      const DropdownMenuItem(value: '', child: Text('—')),
      for (final cls in classes)
        DropdownMenuItem(value: cls, child: Text(cls)),
      if (extraItem != null)
        DropdownMenuItem(value: extraItem, child: Text(extraItem)),
    ];

    return Padding(
      padding: const EdgeInsets.only(bottom: 12),
      child: InputDecorator(
        decoration: InputDecoration(
          labelText: label,
          border: const OutlineInputBorder(),
          isDense: true,
          contentPadding: const EdgeInsets.symmetric(
            horizontal: 12,
            vertical: 8,
          ),
        ),
        child: DropdownButton<String>(
          value: value,
          items: items,
          isDense: true,
          isExpanded: true,
          underline: const SizedBox.shrink(),
          onChanged: enabled ? (v) => onChanged(v ?? '') : null,
        ),
      ),
    );
  }
}
