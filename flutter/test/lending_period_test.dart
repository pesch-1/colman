// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:colman_client/api/collection_api.dart';
import 'package:flutter_test/flutter_test.dart';

CollectionInfo _info({int years = 0, int months = 0, int days = 0}) =>
    CollectionInfo.fromJson({
      'use_lending_period': true,
      'lending_period_years': years,
      'lending_period_months': months,
      'lending_period_days': days,
    });

void main() {
  test('reads the lending period from GET /api/collection', () {
    final info = _info(years: 1, months: 2, days: 3);
    expect(info.useLendingPeriod, isTrue);
    expect(info.lendingPeriodYears, 1);
    expect(info.lendingPeriodMonths, 2);
    expect(info.lendingPeriodDays, 3);
  });

  test('adds years, months and days', () {
    final info = _info(years: 1, months: 2, days: 3);
    expect(info.addLendingPeriod(DateTime(2026, 11, 6)), DateTime(2028, 1, 9));
  });

  test('clamps the day to the end of the month like QDate::addMonths', () {
    final info = _info(months: 1);
    expect(info.addLendingPeriod(DateTime(2026, 1, 31)), DateTime(2026, 2, 28));
    expect(info.addLendingPeriod(DateTime(2028, 1, 31)), DateTime(2028, 2, 29));
  });

  test('no default due date without a lending period', () {
    final info = CollectionInfo.fromJson(const {});
    expect(info.defaultDueDate(), isNull);
  });

  group('proposeExtendedDueDate', () {
    final today = DateTime(2026, 11, 6);

    test('extends from a future due date, never shortens it', () {
      final info = CollectionInfo.fromJson(const {});
      final due = DateTime(2026, 11, 20);
      expect(
        info.proposeExtendedDueDate([due], today: today),
        DateTime(2026, 12, 4),
      );
    });

    test('uses the latest due date of several lendings', () {
      final info = _info(days: 21);
      expect(
        info.proposeExtendedDueDate([
          DateTime(2026, 11, 10),
          DateTime(2026, 11, 30),
        ], today: today),
        DateTime(2026, 12, 21),
      );
    });

    test('starts from today for overdue or missing due dates', () {
      final info = _info(days: 21);
      expect(
        info.proposeExtendedDueDate([
          DateTime(2026, 10, 1),
          DateTime.utc(0),
        ], today: today),
        DateTime(2026, 11, 27),
      );
    });

    test('falls back to 14 days for a zero lending period', () {
      final info = _info();
      expect(
        info.proposeExtendedDueDate(const [], today: today),
        DateTime(2026, 11, 20),
      );
    });
  });
}
