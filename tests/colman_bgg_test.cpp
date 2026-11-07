// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for BGG and opengtindb parsing helpers.
/**
   Covers opengtindb_to_item() with UTF-8 and ISO-8859-1 encoded responses,
   error codes, and missing fields.  No network access.
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_OPENGTINDB_HPP
#  include "colman_opengtindb.hpp"
#  define COLMAN_OPENGTINDB_HPP
#endif
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif

namespace colman
{

  class ColmanBggTest : public QObject
  {
    Q_OBJECT

  private Q_SLOTS:

    // -------------------------------------------------------------------
    // opengtindb_to_item — name extraction

    void test_empty_input()
    {
      QCOMPARE(opengtindb_to_item(QByteArray()).get_name(), QString());
    }

    void test_name_only()
    {
      QByteArray const data =
          "error=0\n"
          "ean=4002051683320\n"
          "name=Spielname\n"
          "vendor=Musterverlag\n";
      QCOMPARE(opengtindb_to_item(data).get_name(), QString("Spielname"));
    }

    void test_detailname_preferred_over_name()
    {
      QByteArray const data =
          "error=0\n"
          "name=Kurzname\n"
          "detailname=Detaillierter Spielname\n";
      QCOMPARE(opengtindb_to_item(data).get_name(),
               QString("Detaillierter Spielname"));
    }

    void test_no_detailname_falls_back_to_name()
    {
      QByteArray const data =
          "error=0\n"
          "name=Nur Name\n";
      QCOMPARE(opengtindb_to_item(data).get_name(), QString("Nur Name"));
    }

    /// ISO-8859-1 encoded response with an umlaut (ä = 0xE4).
    void test_iso8859_umlaut()
    {
      QByteArray data =
          "error=0\n"
          "name=Mensch \xe4rgere dich nicht\n";
      QCOMPARE(opengtindb_to_item(data).get_name(),
               QString::fromLatin1("Mensch \xe4rgere dich nicht"));
    }

    void test_error_code_nonzero_returns_empty()
    {
      QCOMPARE(opengtindb_to_item(QByteArray("error=1\n")).get_name(),
               QString());
    }

    void test_error_13_returns_empty()
    {
      QCOMPARE(opengtindb_to_item(QByteArray("error=13\n")).get_name(),
               QString());
    }

    void test_no_name_field_returns_empty()
    {
      QByteArray const data = "error=0\nvendor=Musterverlag\n";
      QCOMPARE(opengtindb_to_item(data).get_name(), QString());
    }

    void test_empty_name_returns_empty()
    {
      QByteArray const data = "error=0\nname=\ndetailname=\n";
      QCOMPARE(opengtindb_to_item(data).get_name(), QString());
    }

    void test_no_error_field_with_name()
    {
      QCOMPARE(opengtindb_to_item(QByteArray("name=Spielname\n")).get_name(),
               QString("Spielname"));
    }

    /// Trailing manufacturer code in parentheses must be stripped.
    void test_detailname_trailing_parens_stripped()
    {
      QByteArray const data =
          "error=0\n"
          "name=Gesellschaftsspiel\n"
          "detailname=Die Siedler von Catan (KOSMOS 6930150)\n";
      QCOMPARE(opengtindb_to_item(data).get_name(),
               QString("Die Siedler von Catan"));
    }

    // -------------------------------------------------------------------
    // opengtindb_to_item — description and publisher

    void test_description_from_descr()
    {
      // ö = 0xF6 in ISO-8859-1
      QByteArray const data =
          "error=0\n"
          "name=Spielname\n"
          "descr=Preisgekr\xf6ntes Spiel\n";
      QCOMPARE(opengtindb_to_item(data).get_description(),
               QString::fromLatin1("Preisgekr\xf6ntes Spiel"));
    }

    void test_description_falls_back_to_descr_en()
    {
      QByteArray const data =
          "error=0\n"
          "name=Spielname\n"
          "descr=\n"
          "descr_en=Award-winning game\n";
      QCOMPARE(opengtindb_to_item(data).get_description(),
               QString("Award-winning game"));
    }

    void test_publisher_from_vendor()
    {
      QByteArray const data =
          "error=0\n"
          "name=Spielname\n"
          "vendor=KOSMOS\n";
      QCOMPARE(opengtindb_to_item(data).get_values().at("publisher"),
               QString("KOSMOS"));
    }

    void test_full_catan_response()
    {
      QByteArray const data =
          "error=0\n"
          "name=Gesellschaftsspiel\n"
          "detailname=Die Siedler von Catan (KOSMOS 6930150)\n"
          "vendor=KOSMOS\n"
          "descr=Auszeichnungen: Spiel des Jahres\n"
          "descr_en=\n";
      auto const item = opengtindb_to_item(data);
      QCOMPARE(item.get_name(), QString("Die Siedler von Catan"));
      QCOMPARE(item.get_description(), QString("Auszeichnungen: Spiel des Jahres"));
      QCOMPARE(item.get_values().at("publisher"), QString("KOSMOS"));
    }
  };

} // namespace colman

QTEST_MAIN(colman::ColmanBggTest)
#include "colman_bgg_test.moc"

/* EOF */
