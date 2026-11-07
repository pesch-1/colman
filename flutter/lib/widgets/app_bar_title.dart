// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';

/// AppBar title that shows a small icon before the title text.
///
/// Mirrors the home screen navigation card icon so users always know
/// which section they are in.
class AppBarTitle extends StatelessWidget {
  const AppBarTitle({super.key, required this.icon, required this.title});

  final String icon;
  final String title;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Image.asset(icon, height: 24),
        const SizedBox(width: 8),
        Flexible(child: Text(title)),
      ],
    );
  }
}
