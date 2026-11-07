// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for colman_json.hpp (to_json / from_json for all entities).
/**
   No database is required — tests operate on plain entity objects.
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_JSON_HPP
#  include "colman_json.hpp"
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QJSONOBJECT
#  include <QJsonObject>
#  define SYSTEM_QJSONOBJECT
#endif

/// Qt Test class exercising to_json() / from_json() for all entities.
class ColmanJsonTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:

  // ------------------------------------------------------------------
  // Item
  // ------------------------------------------------------------------

  void test_item_round_trip();
  void test_item_to_json_defaults();
  void test_item_from_json_missing_type_id();
  void test_item_from_json_type_id_wrong_type();
  void test_item_from_json_type_id_negative();
  void test_item_from_json_optional_fields_absent();
  void test_item_values_round_trip();
  void test_item_list_values_round_trip();
  void test_item_academic_year_round_trip();
  void test_item_keywords_round_trip();

  // ------------------------------------------------------------------
  // Person
  // ------------------------------------------------------------------

  void test_person_round_trip();
  void test_person_from_json_always_succeeds();
  void test_person_id_defaults_to_zero();

  // ------------------------------------------------------------------
  // Lending
  // ------------------------------------------------------------------

  void test_lending_round_trip();
  void test_lending_null_datetime();
  void test_lending_null_duedate();
  void test_lending_from_json_missing_item_id();
  void test_lending_from_json_missing_person_id();
  void test_lending_from_json_missing_count();
  void test_lending_from_json_optional_fields_absent();

  // ------------------------------------------------------------------
  // Return
  // ------------------------------------------------------------------

  void test_return_round_trip();
  void test_return_from_json_missing_lending_id();
  void test_return_from_json_missing_count();

  // ------------------------------------------------------------------
  // Acquisition
  // ------------------------------------------------------------------

  void test_acquisition_round_trip();
  void test_acquisition_from_json_missing_item_id();
  void test_acquisition_from_json_missing_count();

  // ------------------------------------------------------------------
  // Liquidation
  // ------------------------------------------------------------------

  void test_liquidation_round_trip();
  void test_liquidation_from_json_missing_item_id();
  void test_liquidation_from_json_missing_count();
};

// ====================================================================
// Item
// ====================================================================

void ColmanJsonTest::test_item_round_trip()
{
  colman::Item src;
  src.set_type_id(3);
  src.set_name("History of Rome");
  src.set_barcode("LIB001");
  src.set_place("A2-3");
  src.set_description("A comprehensive history.");
  src.set_thumbnail_id(42);
  src.set_is_school_item(true);
  src.set_subject("History");
  src.set_type("Reference");
  src.set_data_collector_id(7);
  src.set_signature_top("SIG-TOP");
  src.set_signature_bottom("SIG-BOT");

  auto const obj = colman::to_json(src);

  colman::Item dst;
  QVERIFY(colman::from_json(obj, dst));

  QCOMPARE(dst.get_type_id(), 3);
  QCOMPARE(dst.get_name(), QString("History of Rome"));
  QCOMPARE(dst.get_barcode(), QString("LIB001"));
  QCOMPARE(dst.get_place(), QString("A2-3"));
  QCOMPARE(dst.get_description(), QString("A comprehensive history."));
  QCOMPARE(dst.get_thumbnail_id(), 42);
  QCOMPARE(dst.get_is_school_item(), true);
  QCOMPARE(dst.get_subject(), QString("History"));
  QCOMPARE(dst.get_type(), QString("Reference"));
  QCOMPARE(dst.get_data_collector_id(), 7);
  QCOMPARE(dst.get_signature_top(), QString("SIG-TOP"));
  QCOMPARE(dst.get_signature_bottom(), QString("SIG-BOT"));
}

void ColmanJsonTest::test_item_to_json_defaults()
{
  colman::Item src;
  src.set_type_id(0);
  auto const obj = colman::to_json(src);

  QCOMPARE(obj.value("type_id").toInt(), 0);
  QCOMPARE(obj.value("name").toString(), QString{});
  QCOMPARE(obj.value("thumbnail_id").toInt(), -1);
  QCOMPARE(obj.value("is_school_item").toBool(), false);
  QCOMPARE(obj.value("data_collector_id").toInt(), -1);
}

void ColmanJsonTest::test_item_from_json_missing_type_id()
{
  QJsonObject obj;
  obj.insert("name", "Test");
  colman::Item item;
  QVERIFY(!colman::from_json(obj, item));
}

void ColmanJsonTest::test_item_from_json_type_id_wrong_type()
{
  QJsonObject obj;
  obj.insert("type_id", "not-a-number");
  colman::Item item;
  QVERIFY(!colman::from_json(obj, item));
}

void ColmanJsonTest::test_item_from_json_type_id_negative()
{
  QJsonObject obj;
  obj.insert("type_id", -1);
  colman::Item item;
  QVERIFY(!colman::from_json(obj, item));
}

void ColmanJsonTest::test_item_from_json_optional_fields_absent()
{
  QJsonObject obj;
  obj.insert("type_id", 1);
  colman::Item item;
  QVERIFY(colman::from_json(obj, item));

  QCOMPARE(item.get_type_id(), 1);
  QCOMPARE(item.get_name(), QString{});
  QCOMPARE(item.get_barcode(), QString{});
  QCOMPARE(item.get_thumbnail_id(), -1);
  QCOMPARE(item.get_is_school_item(), false);
  QCOMPARE(item.get_data_collector_id(), -1);
  QVERIFY(item.get_values().empty());
  QVERIFY(item.get_list_values().empty());
  QVERIFY(item.get_academic_year().empty());
  QVERIFY(item.get_keywords().isEmpty());
}

void ColmanJsonTest::test_item_values_round_trip()
{
  colman::StringMap values;
  values["isbn"] = "978-3-16-148410-0";
  values["author"] = "Gibbon";

  colman::Item src;
  src.set_type_id(1);
  src.set_values(values);

  colman::Item dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));
  QVERIFY(dst.get_values() == values);
}

void ColmanJsonTest::test_item_list_values_round_trip()
{
  colman::StringListMap lv;
  lv["authors"] = QStringList{"Alice", "Bob"};
  lv["tags"] = QStringList{"fiction", "history"};

  colman::Item src;
  src.set_type_id(1);
  src.set_list_values(lv);

  colman::Item dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));
  QVERIFY(dst.get_list_values() == lv);
}

void ColmanJsonTest::test_item_academic_year_round_trip()
{
  colman::IntPairVec ay{{5, 6}, {7, 8}, {9, 10}};

  colman::Item src;
  src.set_type_id(1);
  src.set_academic_year(ay);

  colman::Item dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));
  QVERIFY(dst.get_academic_year() == ay);
}

void ColmanJsonTest::test_item_keywords_round_trip()
{
  QStringList kw{"rome", "history", "ancient"};

  colman::Item src;
  src.set_type_id(1);
  src.set_keywords(kw);

  colman::Item dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));
  QCOMPARE(dst.get_keywords(), kw);
}

// ====================================================================
// Person
// ====================================================================

void ColmanJsonTest::test_person_round_trip()
{
  colman::Person src;
  src.set_id(5);
  src.set_lastname("Doe");
  src.set_firstname("Jane");
  src.set_title("Dr.");
  src.set_school_class("10a");
  src.set_barcode("PER005");
  src.set_emails("jane@example.com");
  src.set_telephone("+49 123 456");
  src.set_street("Main St 1");
  src.set_zip("12345");
  src.set_city("Berlin");
  src.set_country("Germany");
  src.set_remarks("VIP borrower");
  src.set_thumbnail_id(11);
  src.set_data_collector_id(2);

  colman::Person dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));

  QCOMPARE(dst.get_id(), 5);
  QCOMPARE(dst.get_lastname(), QString("Doe"));
  QCOMPARE(dst.get_firstname(), QString("Jane"));
  QCOMPARE(dst.get_title(), QString("Dr."));
  QCOMPARE(dst.get_school_class(), QString("10a"));
  QCOMPARE(dst.get_barcode(), QString("PER005"));
  QCOMPARE(dst.get_emails(), QString("jane@example.com"));
  QCOMPARE(dst.get_telephone(), QString("+49 123 456"));
  QCOMPARE(dst.get_street(), QString("Main St 1"));
  QCOMPARE(dst.get_zip(), QString("12345"));
  QCOMPARE(dst.get_city(), QString("Berlin"));
  QCOMPARE(dst.get_country(), QString("Germany"));
  QCOMPARE(dst.get_remarks(), QString("VIP borrower"));
  QCOMPARE(dst.get_thumbnail_id(), 11);
  QCOMPARE(dst.get_data_collector_id(), 2);
}

void ColmanJsonTest::test_person_from_json_always_succeeds()
{
  colman::Person person;
  QVERIFY(colman::from_json(QJsonObject{}, person));
}

void ColmanJsonTest::test_person_id_defaults_to_zero()
{
  colman::Person person;
  QVERIFY(colman::from_json(QJsonObject{}, person));
  QCOMPARE(person.get_id(), 0);
}

// ====================================================================
// Lending
// ====================================================================

void ColmanJsonTest::test_lending_round_trip()
{
  colman::Lending src;
  src.set_item_id(10);
  src.set_person_id(20);
  src.set_count(2);
  src.set_date_time(
      QDateTime::fromString("2024-09-01T08:00:00", Qt::ISODate));
  src.set_duedate(QDate(2024, 9, 30));
  src.set_remarks("handle with care");
  src.set_data_collector_id(3);

  colman::Lending dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));

  QCOMPARE(dst.get_item_id(), 10);
  QCOMPARE(dst.get_person_id(), 20);
  QCOMPARE(dst.get_count(), 2u);
  QCOMPARE(dst.get_date_time(),
           QDateTime::fromString("2024-09-01T08:00:00", Qt::ISODate));
  QCOMPARE(dst.get_duedate(), QDate(2024, 9, 30));
  QCOMPARE(dst.get_remarks(), QString("handle with care"));
  QCOMPARE(dst.get_data_collector_id(), 3);
}

void ColmanJsonTest::test_lending_null_datetime()
{
  colman::Lending src;
  src.set_item_id(1);
  src.set_person_id(1);
  src.set_count(1);
  // datetime left as default (invalid)

  auto const obj = colman::to_json(src);
  QVERIFY(obj.value("date_time").isNull());

  colman::Lending dst;
  QVERIFY(colman::from_json(obj, dst));
  QVERIFY(!dst.get_date_time().isValid());
}

void ColmanJsonTest::test_lending_null_duedate()
{
  colman::Lending src;
  src.set_item_id(1);
  src.set_person_id(1);
  src.set_count(1);
  // duedate left as default (invalid)

  auto const obj = colman::to_json(src);
  QVERIFY(obj.value("duedate").isNull());

  colman::Lending dst;
  QVERIFY(colman::from_json(obj, dst));
  QVERIFY(!dst.get_duedate().isValid());
}

void ColmanJsonTest::test_lending_from_json_missing_item_id()
{
  QJsonObject obj;
  obj.insert("person_id", 1);
  obj.insert("count", 1);
  colman::Lending lending;
  QVERIFY(!colman::from_json(obj, lending));
}

void ColmanJsonTest::test_lending_from_json_missing_person_id()
{
  QJsonObject obj;
  obj.insert("item_id", 1);
  obj.insert("count", 1);
  colman::Lending lending;
  QVERIFY(!colman::from_json(obj, lending));
}

void ColmanJsonTest::test_lending_from_json_missing_count()
{
  QJsonObject obj;
  obj.insert("item_id", 1);
  obj.insert("person_id", 1);
  colman::Lending lending;
  QVERIFY(!colman::from_json(obj, lending));
}

void ColmanJsonTest::test_lending_from_json_optional_fields_absent()
{
  QJsonObject obj;
  obj.insert("item_id", 5);
  obj.insert("person_id", 9);
  obj.insert("count", 3);

  colman::Lending lending;
  QVERIFY(colman::from_json(obj, lending));

  QCOMPARE(lending.get_item_id(), 5);
  QCOMPARE(lending.get_person_id(), 9);
  QCOMPARE(lending.get_count(), 3u);
  QVERIFY(!lending.get_date_time().isValid());
  QVERIFY(!lending.get_duedate().isValid());
  QCOMPARE(lending.get_remarks(), QString{});
  QCOMPARE(lending.get_data_collector_id(), -1);
}

// ====================================================================
// Return
// ====================================================================

void ColmanJsonTest::test_return_round_trip()
{
  colman::Return src;
  src.set_lending_id(55);
  src.set_count(1);
  src.set_date_time(
      QDateTime::fromString("2024-10-15T12:00:00", Qt::ISODate));
  src.set_remarks("good condition");
  src.set_data_collector_id(4);

  colman::Return dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));

  QCOMPARE(dst.get_lending_id(), 55);
  QCOMPARE(dst.get_count(), 1u);
  QCOMPARE(dst.get_date_time(),
           QDateTime::fromString("2024-10-15T12:00:00", Qt::ISODate));
  QCOMPARE(dst.get_remarks(), QString("good condition"));
  QCOMPARE(dst.get_data_collector_id(), 4);
}

void ColmanJsonTest::test_return_from_json_missing_lending_id()
{
  QJsonObject obj;
  obj.insert("count", 1);
  colman::Return ret;
  QVERIFY(!colman::from_json(obj, ret));
}

void ColmanJsonTest::test_return_from_json_missing_count()
{
  QJsonObject obj;
  obj.insert("lending_id", 1);
  colman::Return ret;
  QVERIFY(!colman::from_json(obj, ret));
}

// ====================================================================
// Acquisition
// ====================================================================

void ColmanJsonTest::test_acquisition_round_trip()
{
  colman::Acquisition src;
  src.set_item_id(7);
  src.set_count(5);
  src.set_date_time(
      QDateTime::fromString("2024-08-01T09:00:00", Qt::ISODate));
  src.set_data_collector_id(2);

  colman::Acquisition dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));

  QCOMPARE(dst.get_item_id(), 7);
  QCOMPARE(dst.get_count(), 5u);
  QCOMPARE(dst.get_date_time(),
           QDateTime::fromString("2024-08-01T09:00:00", Qt::ISODate));
  QCOMPARE(dst.get_data_collector_id(), 2);
}

void ColmanJsonTest::test_acquisition_from_json_missing_item_id()
{
  QJsonObject obj;
  obj.insert("count", 1);
  colman::Acquisition acq;
  QVERIFY(!colman::from_json(obj, acq));
}

void ColmanJsonTest::test_acquisition_from_json_missing_count()
{
  QJsonObject obj;
  obj.insert("item_id", 1);
  colman::Acquisition acq;
  QVERIFY(!colman::from_json(obj, acq));
}

// ====================================================================
// Liquidation
// ====================================================================

void ColmanJsonTest::test_liquidation_round_trip()
{
  colman::Liquidation src;
  src.set_item_id(12);
  src.set_count(3);
  src.set_date_time(
      QDateTime::fromString("2024-07-04T16:30:00", Qt::ISODate));
  src.set_data_collector_id(1);

  colman::Liquidation dst;
  QVERIFY(colman::from_json(colman::to_json(src), dst));

  QCOMPARE(dst.get_item_id(), 12);
  QCOMPARE(dst.get_count(), 3u);
  QCOMPARE(dst.get_date_time(),
           QDateTime::fromString("2024-07-04T16:30:00", Qt::ISODate));
  QCOMPARE(dst.get_data_collector_id(), 1);
}

void ColmanJsonTest::test_liquidation_from_json_missing_item_id()
{
  QJsonObject obj;
  obj.insert("count", 1);
  colman::Liquidation liq;
  QVERIFY(!colman::from_json(obj, liq));
}

void ColmanJsonTest::test_liquidation_from_json_missing_count()
{
  QJsonObject obj;
  obj.insert("item_id", 1);
  colman::Liquidation liq;
  QVERIFY(!colman::from_json(obj, liq));
}

QTEST_GUILESS_MAIN(ColmanJsonTest)
#include "colman_json_test.moc"

/* EOF */
