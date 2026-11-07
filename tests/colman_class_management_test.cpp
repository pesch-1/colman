// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for compute_new_name() and merge_class_names().
/**
   Covers single-number increment, two-number swap, overflow clamping,
   the mutual-inverse property (swap case), names without numbers, and
   merging the class list with the classes used by persons.
   @file
   @author Christoph Pesch
*/

#include "colman_class_names.hpp"
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif

namespace colman
{

  class ColmanClassManagementTest : public QObject
  {
    Q_OBJECT

  private Q_SLOTS:

  // -----------------------------------------------------------------------
  // Single-number class names

    void test_single_number_incremented()
    {
      QCOMPARE(compute_new_name("2a", 1, 13), QString("3a"));
    }

    void test_single_number_at_max_returns_empty()
    {
      QCOMPARE(compute_new_name("13a", 1, 13), QString());
    }

    void test_single_number_one_below_max_reaches_max()
    {
      QCOMPARE(compute_new_name("12a", 1, 13), QString("13a"));
    }

    void test_single_number_zero_incremented_to_one()
    {
      QCOMPARE(compute_new_name("0a", 1, 13), QString("1a"));
    }

    void test_single_number_preserves_surrounding_text()
    {
      QCOMPARE(compute_new_name("Klasse3b", 1, 13), QString("Klasse4b"));
    }

    void test_single_number_prefix_and_suffix_preserved()
    {
    // Number embedded between non-numeric characters.
      QCOMPARE(compute_new_name("Jg.5.Kl", 1, 13), QString("Jg.6.Kl"));
    }

  // -----------------------------------------------------------------------
  // Two-number class names (split classes, e.g. "M1/3")

    void test_two_numbers_no_overflow()
    {
    // M1/3 with max=4: a=2, b=4 — no overflow, no swap.
      QCOMPARE(compute_new_name("M1/3", 1, 4), QString("M2/4"));
    }

    void test_two_numbers_b_overflows_triggers_swap()
    {
    // M2/4 with max=4: b=5>4 → b=min=1, swap → a=1, b=3 → M1/3.
      QCOMPARE(compute_new_name("M2/4", 1, 4), QString("M1/3"));
    }

    void test_two_numbers_swap_is_mutual_inverse()
    {
    // The two classes must map to each other — applying compute_new_name
    // to either result must yield the other name again (with min=1, max=4).
      QString const r13 = compute_new_name("M1/3", 1, 4); // → "M2/4"
      QString const r24 = compute_new_name("M2/4", 1, 4); // → "M1/3"
      QCOMPARE(r13, QString("M2/4"));
      QCOMPARE(r24, QString("M1/3"));
      QCOMPARE(r13, compute_new_name(r24, 1, 4));
      QCOMPARE(r24, compute_new_name(r13, 1, 4));
    }

    void test_two_numbers_only_b_at_max()
    {
    // M3/4 with max=4: a=4 (ok), b=5>4 → b=1, swap → a=1, b=4 → M1/4.
      QCOMPARE(compute_new_name("M3/4", 1, 4), QString("M1/4"));
    }

    void test_two_numbers_both_overflow()
    {
    // M4/4 with max=4: a=5>4→1, b=5>4→1, swap → M1/1.
      QCOMPARE(compute_new_name("M4/4", 1, 4), QString("M1/1"));
    }

    void test_two_numbers_no_overflow_small_range()
    {
    // M1/2 with max=4: a=2, b=3, no swap → M2/3.
      QCOMPARE(compute_new_name("M1/2", 1, 4), QString("M2/3"));
    }

    void test_two_numbers_right_to_left_replacement()
    {
    // Verify that replacing the second number first preserves the first
    // number's position (both numbers have the same digit count here).
      QCOMPARE(compute_new_name("X10/20", 1, 99), QString("X11/21"));
    }

  // -----------------------------------------------------------------------
  // Names without numbers

    void test_no_numbers_unchanged()
    {
      QCOMPARE(compute_new_name("SL", 1, 13), QString("SL"));
    }

    void test_empty_string_unchanged()
    {
      QCOMPARE(compute_new_name("", 1, 13), QString(""));
    }

    void test_letters_only_unchanged()
    {
      QCOMPARE(compute_new_name("ABC", 1, 13), QString("ABC"));
    }

  // -----------------------------------------------------------------------
  // merge_class_names()

    void test_merge_adds_classes_used_by_persons()
    {
      QCOMPARE(merge_class_names({"5a", "6b"}, {"5a", "7x", "6b"}),
               QStringList({"5a", "6b", "7x"}));
    }

    void test_merge_drops_empty_and_duplicates()
    {
      QCOMPARE(merge_class_names({"5a", ""}, {"", "5a", "5a"}),
               QStringList({"5a"}));
    }

    void test_merge_sorts_naturally()
    {
      QCOMPARE(merge_class_names({"10a", "9b"}, {"5c"}),
               QStringList({"5c", "9b", "10a"}));
    }
  };

} // namespace colman

QTEST_MAIN(colman::ColmanClassManagementTest)
#include "colman_class_management_test.moc"

/* EOF */
