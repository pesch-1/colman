// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../models/item.dart';
import '../models/item_type.dart';
import '../providers/api_client_provider.dart';
import '../providers/table_watch.dart';
import 'thumbnail_widget.dart';

/// Renders the detail fields of a library item.
///
/// Shows thumbnail, type, name, barcode, type-specific custom fields, school
/// fields, location fields, description, keywords, and availability counts.
/// Does not include action buttons — those belong to the embedding screen.
class ItemDetailBody extends StatelessWidget {
  const ItemDetailBody({super.key, required this.item, this.itemType});

  final Item item;

  /// When non-null, type-specific custom fields are rendered with their
  /// display names; when null the custom-field section is omitted.
  final ItemType? itemType;

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        ThumbnailWidget(thumbnailId: item.thumbnailId),
        _DetailRow(label: l10n.labelType, value: item.typeName ?? '—'),
        _DetailRow(label: l10n.labelName, value: item.name),
        if ((item.barcode ?? '').isNotEmpty)
          _DetailRow(label: l10n.labelBarcode, value: item.barcode!),
        if (itemType != null)
          for (final key in itemType!.keys) ...[
            if (key.isList && (item.listValues[key.name] ?? []).isNotEmpty)
              _DetailRow(
                label: key.displayName,
                value: item.listValues[key.name]!.join(', '),
              )
            else if (!key.isList && (item.values[key.name] ?? '').isNotEmpty)
              _DetailRow(label: key.displayName, value: item.values[key.name]!),
          ],
        if (item.isSchoolItem && item.subject != null)
          _DetailRow(label: l10n.labelSubject, value: item.subject!),
        if (item.isSchoolItem && item.academicYear.isNotEmpty)
          _DetailRow(
            label: l10n.labelAcademicYear,
            value: _formatAcademicYear(item.academicYear),
          ),
        if (item.place != null)
          _DetailRow(label: l10n.labelPlace, value: item.place!),
        if (item.signatureTop != null)
          _DetailRow(label: l10n.labelSignatureTop, value: item.signatureTop!),
        if (item.signatureBottom != null)
          _DetailRow(
            label: l10n.labelSignatureBottom,
            value: item.signatureBottom!,
          ),
        if (item.description != null)
          _DetailRow(label: l10n.labelDescription, value: item.description!),
        if (item.keywords.isNotEmpty)
          _DetailRow(
            label: l10n.labelKeywords,
            value: item.keywords.join(', '),
          ),
        _AvailabilityRow(itemId: item.id, l10n: l10n),
      ],
    );
  }
}

String _formatAcademicYear(List<List<int>> years) {
  return years
      .map((pair) {
        if (pair.length >= 2 && pair[0] != pair[1]) {
          return '${pair[0]}/${pair[1]}';
        }
        return pair.isNotEmpty ? '${pair[0]}' : '';
      })
      .where((s) => s.isNotEmpty)
      .join(', ');
}

class _DetailRow extends StatelessWidget {
  const _DetailRow({required this.label, required this.value});

  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 6),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            width: 100,
            child: Text(
              label,
              style: Theme.of(context).textTheme.labelMedium?.copyWith(
                color: Theme.of(context).colorScheme.onSurfaceVariant,
              ),
            ),
          ),
          Expanded(child: Text(value)),
        ],
      ),
    );
  }
}

/// Loads and displays the availability counts for a single item.
///
/// The counts (stock and available) are not carried by the [Item] model, so
/// they are fetched separately. The row re-fetches automatically whenever a
/// relevant table changes on the server (via [watchTables]), so the displayed
/// availability stays in sync with lends, returns, acquisitions and
/// liquidations. Errors are suppressed — availability is supplementary
/// information and the detail view must remain functional if the count
/// endpoints fail.
class _AvailabilityRow extends ConsumerStatefulWidget {
  const _AvailabilityRow({required this.itemId, required this.l10n});

  final int itemId;
  final AppLocalizations l10n;

  @override
  ConsumerState<_AvailabilityRow> createState() => _AvailabilityRowState();
}

class _AvailabilityRowState extends ConsumerState<_AvailabilityRow> {
  int? _count;
  int? _available;

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    try {
      final api = ref.read(itemsApiProvider);
      final results = await Future.wait([
        api.getAcquired(widget.itemId),
        api.getAvailable(widget.itemId),
      ]);
      if (!mounted) return;
      setState(() {
        _count = results[0];
        _available = results[1];
      });
    } catch (_) {
      // availability is supplementary — silently ignore errors
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const [
      'items',
      'lendings',
      'returns',
      'acquisitions',
      'liquidations',
    ], _load);
    if (_count == null) return const SizedBox.shrink();
    return _DetailRow(
      label: widget.l10n.labelAvailability,
      value: widget.l10n.availabilityOf(_available!, _count!),
    );
  }
}
