// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for ISBN validation, conversion, IsbnValidator, and IsbnQueries.
/**
   Covers the pure functions (is_isbn13, is_isbn10, isbn10_to_isbn13,
   isbn13_to_isbn10, fill_isbn), the IsbnValidator class, and the
   IsbnQueries class.  Network access is avoided by calling timeout()
   directly to simulate a completed (timed-out) query.
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#endif
#ifndef COLMAN_EAN_QUERIES_HPP
#  include "colman_ean_queries.hpp"
#endif
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif


namespace colman
{

  class ColmanIsbnTest : public QObject
  {
    Q_OBJECT

  private:
  /// Enables googleapis with a short timeout in test-local QSettings.
    void enable_service()
    {
      QSettings s;
      s.setValue("use_googleapis", true);
      s.setValue("timeout", 100);
      s.sync();
    }

  /// Disables all ISBN services in test-local QSettings.
    void disable_all_services()
    {
      QSettings s;
      s.setValue("use_googleapis", false);
      s.setValue("use_openlibrary", false);
      s.sync();
    }

  private Q_SLOTS:

    void initTestCase()
    {
    // Isolate QSettings from real user config.
      QCoreApplication::setOrganizationName("colman_isbn_test");
      QCoreApplication::setApplicationName("colman_isbn_test");
      QSettings().clear();
      disable_all_services();
    }

    void cleanupTestCase()
    {
      QSettings().clear();
      QSettings().sync();
    }

  // -----------------------------------------------------------------------
  // is_isbn13

    void test_isbn13_valid_978_prefix()
    {
    // 9780596007126 = ISBN-13 for "0596007124" (Programming Python)
      QVERIFY(is_isbn13("9780596007126"));
    }

    void test_isbn13_valid_979_prefix()
    {
    // ISBN-13 with 979 prefix (no ISBN-10 equivalent)
      QVERIFY(is_isbn13("9791032349014"));
    }

    void test_isbn13_wrong_check_digit()
    {
      QVERIFY(!is_isbn13("9780596007125"));
    }

    void test_isbn13_too_short()
    {
      QVERIFY(!is_isbn13("978059600712"));
    }

    void test_isbn13_too_long()
    {
      QVERIFY(!is_isbn13("97805960071261"));
    }

    void test_isbn13_empty()
    {
      QVERIFY(!is_isbn13(""));
    }

    void test_isbn13_contains_non_digit()
    {
      QVERIFY(!is_isbn13("978059600712X"));
    }

  // -----------------------------------------------------------------------
  // is_isbn10

    void test_isbn10_valid_digit_check()
    {
    // 0596007124 (Programming Python)
      QVERIFY(is_isbn10("0596007124"));
    }

    void test_isbn10_valid_x_check()
    {
    // 020161622X (The Pragmatic Programmer)
      QVERIFY(is_isbn10("020161622X"));
    }

    void test_isbn10_wrong_check_digit()
    {
      QVERIFY(!is_isbn10("0596007123"));
    }

    void test_isbn10_too_short()
    {
      QVERIFY(!is_isbn10("059600712"));
    }

    void test_isbn10_too_long()
    {
      QVERIFY(!is_isbn10("05960071241"));
    }

    void test_isbn10_empty()
    {
      QVERIFY(!is_isbn10(""));
    }

    void test_isbn10_contains_non_digit_non_x()
    {
      QVERIFY(!is_isbn10("059600712A"));
    }

    void test_isbn10_x_in_wrong_position()
    {
    // X is only valid as the last character
      QVERIFY(!is_isbn10("059X007124"));
    }

  // -----------------------------------------------------------------------
  // isbn10_to_isbn13

    void test_isbn10_to_isbn13_digit_check()
    {
      QCOMPARE(isbn10_to_isbn13("0596007124"), QString("9780596007126"));
    }

    void test_isbn10_to_isbn13_x_check()
    {
      QCOMPARE(isbn10_to_isbn13("020161622X"), QString("9780201616224"));
    }

    void test_isbn10_to_isbn13_result_is_valid()
    {
      QVERIFY(is_isbn13(isbn10_to_isbn13("0596007124")));
      QVERIFY(is_isbn13(isbn10_to_isbn13("020161622X")));
    }

  // -----------------------------------------------------------------------
  // isbn13_to_isbn10

    void test_isbn13_to_isbn10_digit_check()
    {
      QCOMPARE(isbn13_to_isbn10("9780596007126"), QString("0596007124"));
    }

    void test_isbn13_to_isbn10_x_check()
    {
      QCOMPARE(isbn13_to_isbn10("9780201616224"), QString("020161622X"));
    }

    void test_isbn13_to_isbn10_979_prefix_returns_empty()
    {
    // ISBN-13 with 979 prefix cannot be represented as ISBN-10
      QVERIFY(isbn13_to_isbn10("9791032349014").isEmpty());
    }

    void test_isbn13_to_isbn10_result_is_valid()
    {
      QVERIFY(is_isbn10(isbn13_to_isbn10("9780596007126")));
      QVERIFY(is_isbn10(isbn13_to_isbn10("9780201616224")));
    }

  // -----------------------------------------------------------------------
  // Round-trip isbn10 <-> isbn13

    void test_roundtrip_isbn10_to_isbn13_and_back()
    {
      QString const isbn10("0596007124");
      QCOMPARE(isbn13_to_isbn10(isbn10_to_isbn13(isbn10)), isbn10);
    }

    void test_roundtrip_isbn10_x_to_isbn13_and_back()
    {
      QString const isbn10("020161622X");
      QCOMPARE(isbn13_to_isbn10(isbn10_to_isbn13(isbn10)), isbn10);
    }

    void test_roundtrip_isbn13_to_isbn10_and_back()
    {
      QString const isbn13("9780596007126");
      QCOMPARE(isbn10_to_isbn13(isbn13_to_isbn10(isbn13)), isbn13);
    }

  // -----------------------------------------------------------------------
  // fill_isbn

    void test_fill_isbn_from_isbn13_sets_both_fields()
    {
      Item item;
      fill_isbn("9780596007126", item);
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9780596007126"));
      QCOMPARE(item.get_list_values().at("ISBN10").first(),
               QString("0596007124"));
    }

    void test_fill_isbn_from_isbn10_sets_both_fields()
    {
      Item item;
      fill_isbn("0596007124", item);
      QCOMPARE(item.get_list_values().at("ISBN10").first(),
               QString("0596007124"));
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9780596007126"));
    }

    void test_fill_isbn_from_isbn10_x_sets_both_fields()
    {
      Item item;
      fill_isbn("020161622X", item);
      QCOMPARE(item.get_list_values().at("ISBN10").first(),
               QString("020161622X"));
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9780201616224"));
    }

    void test_fill_isbn_from_979_isbn13_leaves_isbn10_empty()
    {
      Item item;
      fill_isbn("9791032349014", item);
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9791032349014"));
    // 979-prefix has no ISBN-10 equivalent
      QVERIFY(item.get_list_values().at("ISBN10").first().isEmpty());
    }

    void test_fill_isbn_invalid_sets_no_fields()
    {
      Item item;
      fill_isbn("1234567890", item);
      QVERIFY(item.get_list_values().count("ISBN10") == 0);
      QVERIFY(item.get_list_values().count("ISBN13") == 0);
    }

  // -----------------------------------------------------------------------
  // IsbnValidator::fixup

    void test_fixup_removes_hyphens_isbn13()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("978-0-596-00712-6");
      v.fixup(s);
      QCOMPARE(s, QString("9780596007126"));
    }

    void test_fixup_removes_hyphens_isbn10()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("0-596-00712-4");
      v.fixup(s);
      QCOMPARE(s, QString("0596007124"));
    }

    void test_fixup_removes_spaces()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("978 0 596 00712 6");
      v.fixup(s);
      QCOMPARE(s, QString("9780596007126"));
    }

    void test_fixup_keeps_x_check_digit()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("0-201-61622-X");
      v.fixup(s);
      QCOMPARE(s, QString("020161622X"));
    }

    void test_fixup_empty_string()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("");
      v.fixup(s);
      QCOMPARE(s, QString(""));
    }

    void test_fixup_all_non_digits_removed()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("ISBN: ");
      v.fixup(s);
      QCOMPARE(s, QString(""));
    }

  // -----------------------------------------------------------------------
  // IsbnValidator::validate (ISBN-13)

    void test_validate_isbn13_acceptable()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("9780596007126");
      int pos = 13;
      QCOMPARE(v.validate(s, pos), QValidator::Acceptable);
    }

    void test_validate_isbn13_intermediate_too_short()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("978059600712");
      int pos = 12;
      QCOMPARE(v.validate(s, pos), QValidator::Intermediate);
    }

    void test_validate_isbn13_invalid_bad_check_at_end()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("9780596007125");
      int pos = 13;  // cursor past last digit
      QCOMPARE(v.validate(s, pos), QValidator::Invalid);
    }

    void test_validate_isbn13_intermediate_bad_check_cursor_inside()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("9780596007125");
      int pos = 5;  // cursor in the middle — still editing
      QCOMPARE(v.validate(s, pos), QValidator::Intermediate);
    }

    void test_validate_isbn13_intermediate_fourteen_chars()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("97805960071261");
      int pos = 13;  // pos < 14
      QCOMPARE(v.validate(s, pos), QValidator::Intermediate);
    }

    void test_validate_isbn13_invalid_fourteen_chars_cursor_at_end()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("97805960071261");
      int pos = 14;
      QCOMPARE(v.validate(s, pos), QValidator::Invalid);
    }

    void test_validate_isbn13_invalid_non_digit_at_cursor()
    {
      IsbnValidator v(IsbnType::Isbn13);
      QString s("978059600712-");
      int pos = 13;  // last char is '-'
      QCOMPARE(v.validate(s, pos), QValidator::Invalid);
    }

  // -----------------------------------------------------------------------
  // IsbnValidator::validate (ISBN-10)

    void test_validate_isbn10_acceptable()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("0596007124");
      int pos = 10;
      QCOMPARE(v.validate(s, pos), QValidator::Acceptable);
    }

    void test_validate_isbn10_acceptable_x_check()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("020161622X");
      int pos = 10;
      QCOMPARE(v.validate(s, pos), QValidator::Acceptable);
    }

    void test_validate_isbn10_intermediate_too_short()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("059600712");
      int pos = 9;
      QCOMPARE(v.validate(s, pos), QValidator::Intermediate);
    }

    void test_validate_isbn10_invalid_bad_check_at_end()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("0596007123");
      int pos = 10;
      QCOMPARE(v.validate(s, pos), QValidator::Invalid);
    }

    void test_validate_isbn10_intermediate_bad_check_cursor_inside()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("0596007123");
      int pos = 5;
      QCOMPARE(v.validate(s, pos), QValidator::Intermediate);
    }

    void test_validate_isbn10_invalid_non_digit_at_cursor()
    {
      IsbnValidator v(IsbnType::Isbn10);
      QString s("059600712-");
      int pos = 10;
      QCOMPARE(v.validate(s, pos), QValidator::Invalid);
    }

  // -----------------------------------------------------------------------
  // IsbnQueries

    void test_isbn_queries_thumbnail_url_empty_initially()
    {
      IsbnQueries q;
      QVERIFY(q.get_thumbnail_url().isEmpty());
    }

    void test_isbn_queries_query_isbn_false_when_no_services_enabled()
    {
      disable_all_services();
      IsbnQueries q;
      QVERIFY(!q.query_isbn("9780596007126"));
    }

    void test_isbn_queries_query_isbn_true_when_service_enabled()
    {
      enable_service();
      IsbnQueries q;
      q.use_googleapis(true);
      QVERIFY(q.query_isbn("9780596007126"));
      disable_all_services();
    }

    void test_isbn_queries_get_book_data_seeds_isbn13_fields()
    {
      enable_service();
      IsbnQueries q;
      q.use_googleapis(true);
      QVERIFY(q.query_isbn("9780596007126"));
      Item const item = q.get_book_data();
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9780596007126"));
      QCOMPARE(item.get_list_values().at("ISBN10").first(),
               QString("0596007124"));
      disable_all_services();
    }

    void test_isbn_queries_get_book_data_seeds_isbn10_fields()
    {
      enable_service();
      IsbnQueries q;
      q.use_googleapis(true);
      QVERIFY(q.query_isbn("0596007124"));
      Item const item = q.get_book_data();
      QCOMPARE(item.get_list_values().at("ISBN10").first(),
               QString("0596007124"));
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9780596007126"));
      disable_all_services();
    }

    void test_isbn_queries_second_query_replaces_first()
    {
      enable_service();
      IsbnQueries q;
      q.use_googleapis(true);
      q.query_isbn("9780596007126");   // first query (blocking)
      q.query_isbn("9780201616224");   // second query replaces first
      Item const item = q.get_book_data();
      QCOMPARE(item.get_list_values().at("ISBN13").first(),
               QString("9780201616224"));
      QCOMPARE(item.get_list_values().at("ISBN10").first(),
               QString("020161622X"));
      disable_all_services();
    }

    void test_isbn_queries_get_book_data_empty_without_query()
    {
      IsbnQueries q;
      Item const item = q.get_book_data();
      QVERIFY(item.get_list_values().count("ISBN13") == 0);
      QVERIFY(item.get_list_values().count("ISBN10") == 0);
    }

    void test_isbn_queries_thumbnail_url_empty_after_failed_query()
    {
    // With a 100 ms network timeout the real request will fail → no cover URL.
      enable_service();
      IsbnQueries q;
      q.use_googleapis(true);
      q.query_isbn("9780596007126");
      QVERIFY(q.get_thumbnail_url().isEmpty());
      disable_all_services();
    }

  // -----------------------------------------------------------------------
  // IsbnQueries — live OpenLibrary request (requires internet connection)

    void test_isbn_queries_openlibrary_live()
    {
    // Requires a working internet connection; skipped by default so that
    // offline builds and CI remain green.  Set COLMAN_RUN_NETWORK_TESTS=1
    // to enable this test.
      if(qEnvironmentVariableIsEmpty("COLMAN_RUN_NETWORK_TESTS")) {
        QSKIP("Skipped (set COLMAN_RUN_NETWORK_TESTS=1 to enable)");
      }

    // "Programming Python" (3rd ed.) by Mark Lutz — stable, well-known entry.
      QString const isbn13("9780596007126");
      QString const isbn10("0596007124");

      QSettings s;
      s.setValue("use_openlibrary", true);
      s.setValue("use_googleapis", false);
      s.setValue("timeout", 10000);
      s.sync();

      IsbnQueries q;
      q.use_openlibrary(true);
      QVERIFY(q.query_isbn(isbn13));  // blocks until done (10 s timeout)

      Item const item = q.get_book_data();

    // ISBN fields are always filled by fill_isbn() inside get_book_data().
      QCOMPARE(item.get_list_values().at("ISBN13").first(), isbn13);
      QCOMPARE(item.get_list_values().at("ISBN10").first(), isbn10);

    // OpenLibrary should have returned a title.
      QVERIFY2(!item.get_name().isEmpty(),
               "OpenLibrary returned no title for the queried ISBN");

    // Authors and publisher should be present for this well-known book.
      QVERIFY(!item.get_list_values().at("authors").isEmpty());
      QVERIFY(!item.get_values().at("publisher").isEmpty());

    // Cover URL should be available for this edition.
      QVERIFY2(!q.get_thumbnail_url().isEmpty(),
               "OpenLibrary returned no cover URL for the queried ISBN");

      disable_all_services();
    }

  // -----------------------------------------------------------------------
  // dnb_ean_to_item — MARC21 parsing

    void test_dnb_ean_to_item_parses_artist_from_field_700()
    {
      QByteArray const xml(
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
          "<searchRetrieveResponse xmlns=\"http://www.loc.gov/zing/srw/\">"
          "<records><record><recordData>"
          "<record xmlns=\"http://www.loc.gov/MARC21/slim\" type=\"Bibliographic\">"
          "<datafield tag=\"245\" ind1=\"0\" ind2=\"0\">"
          "<subfield code=\"a\">Freigeistin</subfield>"
          "<subfield code=\"c\">Sarah Connor</subfield>"
          "</datafield>"
          "<datafield tag=\"700\" ind1=\"1\" ind2=\" \">"
          "<subfield code=\"8\">1\\p</subfield>"
          "<subfield code=\"0\">(DE-588)135244110</subfield>"
          "<subfield code=\"0\">https://d-nb.info/gnd/135244110</subfield>"
          "<subfield code=\"0\">(DE-101)135244110</subfield>"
          "<subfield code=\"a\">Connor, Sarah</subfield>"
          "<subfield code=\"d\">1980-</subfield>"
          "<subfield code=\"e\">S&#xE4;nger</subfield>"
          "<subfield code=\"4\">sng</subfield>"
          "<subfield code=\"2\">gnd</subfield>"
          "</datafield>"
          "</record></recordData></record></records>"
          "</searchRetrieveResponse>");
      Item const item = dnb_ean_to_item(xml);
      QCOMPARE(item.get_name(), QString("Freigeistin"));
      QVERIFY(!item.get_list_values().at("artist").isEmpty());
      QCOMPARE(item.get_list_values().at("artist").first(), QString("Sarah Connor"));
    }

    void test_is_product_ean_upc_not_isbn()
    {
      QVERIFY(is_product_ean("0602475811497"));
      QVERIFY(is_product_ean("4050538315578"));
      QVERIFY(!is_product_ean("9780596007126")); // ISBN-13
      QVERIFY(!is_product_ean("9791032349014")); // ISBN-13 979
      QVERIFY(!is_product_ean("123"));           // too short
    }

    void test_is_upc_a_valid()
    {
      QVERIFY(is_upc_a("602475811497"));  // prepend 0 → valid EAN-13
      QVERIFY(!is_upc_a("0602475811497")); // 13 digits, not 12
      QVERIFY(!is_upc_a("60247581149"));   // 11 digits
    }
  };

} // namespace colman

QTEST_MAIN(colman::ColmanIsbnTest)
#include "colman_isbn_test.moc"

/* EOF */
