// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/id_card_layout.dart';
import '../providers/api_client_provider.dart';
import 'layout_editor_screen.dart';

/// Edits the person ID-card layout, kept locally on the device.
///
/// A thin wrapper around the reusable [LayoutEditorScreen]. The layout is stored
/// per client in [IdCardLayoutStore]; on first use it is seeded once from the
/// server's current layout. The collection is still fetched to preview the logo.
/// Editing requires the `edit_id_layout` permission, which the router enforces.
class IdCardLayoutScreen extends ConsumerWidget {
  const IdCardLayoutScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    return LayoutEditorScreen(
      title: l10n.idCardLayoutTitle,
      iconAsset: 'assets/icons/edit_id.jpeg',
      savedMessage: l10n.idCardLayoutSaved,
      sampleTextLines: [l10n.idCardSampleName],
      onLoad: (ref) async {
        final info = await ref.read(collectionApiProvider).get();
        final layout = await const IdCardLayoutStore()
            .loadOrSeed(() async => info.idCardLayout);
        return LayoutEditorData(layout: layout, logoBytes: info.logoBytes);
      },
      onSave: (ref, layout) => const IdCardLayoutStore().save(layout),
    );
  }
}
