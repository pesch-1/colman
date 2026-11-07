// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import '../utils/error_utils.dart';

/// Displays a user-friendly error message with an expandable technical
/// details section.
///
/// For full-page errors, wrap in [Center]. For inline errors inside dialogs
/// or form sections, use directly.
class ErrorDisplay extends StatelessWidget {
  const ErrorDisplay({
    super.key,
    required this.error,
    required this.l10n,
  });

  final Object error;
  final AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    final friendly = friendlyHttpError(error, l10n);
    final detail = extractHttpDetail(error);
    final theme = Theme.of(context);
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        SelectableText(
          friendly,
          style: TextStyle(color: theme.colorScheme.error),
        ),
        Theme(
          data: theme.copyWith(dividerColor: Colors.transparent),
          child: ExpansionTile(
            tilePadding: EdgeInsets.zero,
            title: Text(
              l10n.errorDetails,
              style: theme.textTheme.bodySmall,
            ),
            children: [
              Align(
                alignment: Alignment.centerLeft,
                child: Padding(
                  padding: const EdgeInsets.only(bottom: 8),
                  child: SelectableText(detail, style: theme.textTheme.bodySmall),
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }
}
