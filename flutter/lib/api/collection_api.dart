// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';
import 'dart:typed_data';

import '../models/id_card_layout.dart';
import 'api_client.dart';

/// Collection-level metadata needed by the client.
///
/// Populated from `GET /api/collection`. Only the fields relevant to the
/// ID-card layout, printing pages and the default lending period are kept
/// here.
class CollectionInfo {
  const CollectionInfo({
    required this.name,
    required this.organization,
    required this.barcodePrefix,
    required this.idCardLayout,
    this.logoBytes,
    this.useLendingPeriod = false,
    this.lendingPeriodYears = 0,
    this.lendingPeriodMonths = 0,
    this.lendingPeriodDays = 0,
  });

  /// Library / collection name (printed as the first card line).
  final String name;

  /// Operating organization (printed as the second card line).
  final String organization;

  /// Barcode prefix used to compute person barcodes client-side.
  final String barcodePrefix;

  /// The current ID-card layout.
  final IdCardLayout idCardLayout;

  /// Raw logo image bytes, or `null` when no logo is stored.
  final Uint8List? logoBytes;

  /// True when the collection defines a default lending period.
  final bool useLendingPeriod;

  /// Years of the default lending period.
  final int lendingPeriodYears;

  /// Months of the default lending period.
  final int lendingPeriodMonths;

  /// Days of the default lending period.
  final int lendingPeriodDays;

  /// Returns [from] plus the default lending period (date only).
  ///
  /// Like the desktop client (`QDate::addMonths`), a day that does not exist
  /// in the target month is clamped to the month's last day.
  DateTime addLendingPeriod(DateTime from) {
    final totalMonths =
        from.month - 1 + lendingPeriodMonths + 12 * lendingPeriodYears;
    final year = from.year + totalMonths ~/ 12;
    final month = totalMonths % 12 + 1;
    final lastDay = DateTime(year, month + 1, 0).day;
    final day = from.day > lastDay ? lastDay : from.day;
    return DateTime(year, month, day + lendingPeriodDays);
  }

  /// Extension used when the collection has no default lending period.
  static const fallbackExtensionDays = 14;

  /// Proposes a new due date that never shortens any current due date.
  ///
  /// Starts from the later of [today] (default: now) and the latest of
  /// [currentDueDates] and adds the default lending period, or
  /// [fallbackExtensionDays] when none is configured.  Due dates with
  /// `year == 0` (no due date, see `DueLendingRow`) count as today.
  DateTime proposeExtendedDueDate(
    Iterable<DateTime> currentDueDates, {
    DateTime? today,
  }) {
    final now = today ?? DateTime.now();
    var base = DateTime(now.year, now.month, now.day);
    for (final d in currentDueDates) {
      if (d.year == 0) continue;
      final day = DateTime(d.year, d.month, d.day);
      if (day.isAfter(base)) base = day;
    }
    if (useLendingPeriod) {
      final proposal = addLendingPeriod(base);
      if (proposal.isAfter(base)) return proposal;
    }
    return DateTime(base.year, base.month, base.day + fallbackExtensionDays);
  }

  /// Default due date for a new lending, or `null` when the collection
  /// has no default lending period.
  DateTime? defaultDueDate() {
    if (!useLendingPeriod) return null;
    final now = DateTime.now();
    return addLendingPeriod(DateTime(now.year, now.month, now.day));
  }

  factory CollectionInfo.fromJson(Map<String, dynamic> json) {
    final logoBase64 = json['logo'] as String?;
    Uint8List? logo;
    if (logoBase64 != null && logoBase64.isNotEmpty) {
      logo = base64Decode(logoBase64);
    }
    return CollectionInfo(
      name: json['name'] as String? ?? '',
      organization: json['organization'] as String? ?? '',
      barcodePrefix: json['barcode_prefix'] as String? ?? '',
      idCardLayout: IdCardLayout.fromJson(
        (json['id_card_layout'] as Map<String, dynamic>?) ?? const {},
      ),
      logoBytes: logo,
      useLendingPeriod: json['use_lending_period'] as bool? ?? false,
      lendingPeriodYears: (json['lending_period_years'] as num?)?.toInt() ?? 0,
      lendingPeriodMonths:
          (json['lending_period_months'] as num?)?.toInt() ?? 0,
      lendingPeriodDays: (json['lending_period_days'] as num?)?.toInt() ?? 0,
    );
  }
}

/// REST client for the collection's ID-card layout (`/api/collection`).
class CollectionApi {
  const CollectionApi(this._client);

  final ApiClient _client;

  /// Fetches the collection metadata.
  ///
  /// Uses `GET /api/collection` (needs only authentication). The returned
  /// [CollectionInfo.idCardLayout] is the server's layout, used solely to seed
  /// a fresh client's local copy once; clients keep their layout locally
  /// thereafter (see [IdCardLayoutStore]).
  Future<CollectionInfo> get() async {
    final response = await _client.dio.get('/api/collection');
    return CollectionInfo.fromJson(response.data as Map<String, dynamic>);
  }
}
