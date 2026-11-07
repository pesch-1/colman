// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import '../utils/error_utils.dart';

/// Modal error dialog for REST action failures.
///
/// Shows a user-friendly message and an expandable section with technical
/// details (HTTP status, server message body).
class ServerErrorDialog extends StatelessWidget {
  const ServerErrorDialog({
    super.key,
    required this.error,
    required this.l10n,
    this.message,
  });

  final Object error;
  final AppLocalizations l10n;

  /// Optional override for the user-facing message.
  ///
  /// If omitted, [friendlyHttpError] is used.
  final String? message;

  /// Shows the dialog and waits for the user to dismiss it.
  static Future<void> show(
    BuildContext context,
    Object error,
    AppLocalizations l10n, {
    String? message,
  }) {
    return showDialog<void>(
      context: context,
      builder: (ctx) => ServerErrorDialog(
        error: error,
        l10n: l10n,
        message: message,
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final msg = message ?? friendlyHttpError(error, l10n);
    final detail = extractHttpDetail(error);
    return AlertDialog(
      icon: Icon(
        Icons.error_outline,
        color: theme.colorScheme.error,
        size: 40,
      ),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SelectableText(msg),
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
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text(MaterialLocalizations.of(context).okButtonLabel),
        ),
      ],
    );
  }
}
