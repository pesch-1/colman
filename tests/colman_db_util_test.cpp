// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for colman_db_util.hpp functions.
/**
   Uses Qt Test framework with in-memory SQLite databases.
   Each test method creates a fresh database via setup_db() and
   tears it down with teardown_db() to guarantee isolation.
   @file
   @author Christoph Pesch
 */

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#endif
#ifndef COLMAN_PERSON_HPP
#  include "colman_person.hpp"
#endif
#ifndef COLMAN_LENDING_HPP
#  include "colman_lending.hpp"
#endif
#ifndef COLMAN_RETURN_HPP
#  include "colman_return.hpp"
#endif
#ifndef COLMAN_ACQUISITION_HPP
#  include "colman_acquisition.hpp"
#endif
#ifndef COLMAN_LIQUIDATION_HPP
#  include "colman_liquidation.hpp"
#endif
#ifndef COLMAN_TYPE_HPP
#  include "colman_type.hpp"
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_OPTIONAL
#  include <optional>
#  define SYSTEM_OPTIONAL
#endif
#ifndef SYSTEM_QSTRING
#  include <QString>
#  define SYSTEM_QSTRING
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSQLDATABASE
#  include <QSqlDatabase>
#  define SYSTEM_QSQLDATABASE
#endif
#ifndef SYSTEM_QSCOPEGUARD
#  include <QScopeGuard>
#  define SYSTEM_QSCOPEGUARD
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif

namespace
{
  /// Holds the ids that scenario helpers return.
  struct ScenarioIds
  {
    int type_id = 0;   ///< type id of the created item type
    int item_id = 0;   ///< item id of the created item
    int person_id = 0; ///< person id of the created person
  };

  /// Qt Test class exercising all public functions in colman_db_util.hpp.
  class ColmanSqlTest : public QObject
  {
    Q_OBJECT

  public:
    ColmanSqlTest() = default;

  private:
    std::optional<colman::SQLQueries> q_; ///< query facade (owns its in-memory database)
    QString conn_name_;    ///< unique connection name for the current test
    int conn_counter_ = 0; ///< monotone counter to generate unique names

    // ------------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------------

    /// Opens a fresh in-memory database and initialises all tables.
    void setup_db()
    {
      conn_name_ = QString("colman_test_%1").arg(conn_counter_++);
      q_.emplace(conn_name_);
      QVERIFY(q_->create_tables());
      auto col = colman::make_test_collection();
      QVERIFY(q_->create_collection(col));
      QVERIFY(q_->add_administrator());
    }

    /// Releases the current test database (SQLQueries destructor handles cleanup).
    void teardown_db()
    {
      q_.reset();
    }

    /// Creates a book type + item + acquisition + person in one step.
    /**
       @param stock  number of copies to acquire (default 10)
       @return ScenarioIds with the created ids
     */
    ScenarioIds setup_scenario(int stock = 10)
    {
      ScenarioIds s;
      s.type_id = q_->add_new_type(colman::make_book_type());
      if(s.type_id < 0) {
        QTest::qFail("add_new_type failed", __FILE__, __LINE__);
        return s;
      }

      colman::Item item;
      item.set_type_id(s.type_id);
      item.set_name("Test Book");
      s.item_id = q_->add_new_item(item);
      if(s.item_id < 0) {
        QTest::qFail("add_new_item failed", __FILE__, __LINE__);
        return s;
      }

      if(stock > 0) {
        colman::Acquisition acq;
        acq.set_item_id(s.item_id);
        acq.set_count(static_cast<unsigned int>(stock));
        acq.set_date_time(QDateTime::currentDateTimeUtc());
        if(q_->add_new_acquisition(acq) < 0) {
          QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
          return s;
        }
      }

      s.person_id = q_->add_new_person(colman::make_test_person());
      if(s.person_id < 0) {
        QTest::qFail("add_new_person failed", __FILE__, __LINE__);
      }
      return s;
    }

    /// Adds a Lending of \a count copies and returns the lending id.
    int lend_items(ScenarioIds const& s, unsigned int count = 3)
    {
      colman::Lending l;
      l.set_item_id(s.item_id);
      l.set_person_id(s.person_id);
      l.set_count(count);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      int id = q_->add_new_lending(l);
      if(id < 0) {
        QTest::qFail("add_new_lending failed", __FILE__, __LINE__);
      }
      return id;
    }

    /// Returns a group lending template for \a item_id.
    colman::Lending group_template(int item_id)
    {
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      l.set_remarks("class set");
      return l;
    }

  private Q_SLOTS:

    // ==================================================================
    // Collection
    // ==================================================================

    void test_create_and_read_collection()
    {
      setup_db();
      colman::Collection c = q_->read_collection();
      QCOMPARE(c.get_name(), QString("Test Library"));
      QCOMPARE(c.get_organization(), QString("Test School"));
      QCOMPARE(c.get_barcode_prefix(), QString("TBIB0"));
      QCOMPARE(c.get_data_version(), QString("1.0"));
      QCOMPARE(c.get_max_person_id(), 0);
      QVERIFY(c.get_use_lending_period());
      teardown_db();
    }

    void test_change_collection_name()
    {
      setup_db();
      colman::Collection c = q_->read_collection();
      c.set_name("New Library");
      QVERIFY(q_->change_collection(c));
      colman::Collection c2 = q_->read_collection();
      QCOMPARE(c2.get_name(), QString("New Library"));
      teardown_db();
    }

    void test_change_collection_responsible()
    {
      // max_person_id is managed by add_new_person, not change_collection.
      // Test a field that change_collection does update.
      setup_db();
      colman::Collection c = q_->read_collection();
      c.set_responsible("New Responsible");
      c.set_organization("New School");
      QVERIFY(q_->change_collection(c));
      colman::Collection c2 = q_->read_collection();
      QCOMPARE(c2.get_responsible(), QString("New Responsible"));
      QCOMPARE(c2.get_organization(), QString("New School"));
      teardown_db();
    }

    void test_collection_lending_period()
    {
      setup_db();
      colman::Collection c = q_->read_collection();
      c.set_lending_period_years(1);
      c.set_lending_period_months(2);
      c.set_lending_period_days(3);
      QVERIFY(q_->change_collection(c));
      colman::Collection c2 = q_->read_collection();
      QCOMPARE(c2.get_lending_period_years(), 1);
      QCOMPARE(c2.get_lending_period_months(), 2);
      QCOMPARE(c2.get_lending_period_days(), 3);
      teardown_db();
    }

    void test_collection_cache_sees_external_change()
    {
      // Simulates another process (e.g. the desktop app while the REST
      // server is running) changing the collection settings directly in
      // the database, bypassing this instance's cache invalidation.
      setup_db();
      QVERIFY(q_->read_collection().get_use_lending_period());
      QSqlQuery raw(QSqlDatabase::database(conn_name_));
      QVERIFY(raw.exec("UPDATE collection SET value = 0"
                       " WHERE key = 'use_lending_period'"));
      QVERIFY(!q_->read_collection().get_use_lending_period());
      teardown_db();
    }

    // ==================================================================
    // Types
    // ==================================================================

    void test_add_and_get_type()
    {
      setup_db();
      colman::Type book = colman::make_book_type();
      int id = q_->add_new_type(book);
      QVERIFY(id >= 0);
      colman::Type got = q_->get_type(id);
      QCOMPARE(got.get_id(), id);
      QCOMPARE(got.get_name(), QString("Book"));
      QCOMPARE(q_->get_type_tablename(id), QString("_Book"));
      QCOMPARE(got.get_keys().size(), size_t(2));
      teardown_db();
    }

    void test_get_type_id_by_name()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QCOMPARE(q_->get_type_id("Book"), id);
      teardown_db();
    }

    void test_get_type_id_unknown_returns_minus_one()
    {
      setup_db();
      QCOMPARE(q_->get_type_id("Unknown"), -1);
      teardown_db();
    }

    void test_get_type_name()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QCOMPARE(q_->get_type_name(id), QString("Book"));
      teardown_db();
    }

    void test_get_type_tablename()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QCOMPARE(q_->get_type_tablename(id), QString("_Book"));
      teardown_db();
    }

    void test_get_type_names_multiple()
    {
      setup_db();
      q_->add_new_type(colman::make_book_type());
      colman::Type mag("Magazine", {});
      q_->add_new_type(mag);
      auto names = q_->get_type_names();
      QCOMPARE(static_cast<int>(names.size()), 2);
      teardown_db();
    }

    void test_get_tablename()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QCOMPARE(q_->get_tablename(id), QString("_Book"));
      teardown_db();
    }

    void test_change_type_adds_attribute()
    {
      // change_type restructures the per-type table schema but does not
      // update the display name in the types table directly.
      // Test that after change_type the new attribute list is reflected.
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      colman::Type updated = colman::make_book_type();
      colman::KeyVector new_keys;
      new_keys.push_back({"isbn", {colman::DataType::ShortString, {}}});
      new_keys.push_back({"author", {colman::DataType::ShortString, {}}});
      new_keys.push_back({"year", {colman::DataType::Integer, {}}});
      updated.set_keys(new_keys);
      QVERIFY(q_->change_type(id, updated, {}));
      auto const keys = q_->get_type(id).get_keys();
      QCOMPARE(static_cast<int>(keys.size()), 3);
      auto has_key = [&](QString const& name) {
        for(auto const& k : keys) {
          if(k.first == name) {
            return true;
          }
        }
        return false;
      };
      QVERIFY(has_key(QStringLiteral("year")));
      teardown_db();
    }

    void test_remove_type()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QVERIFY(q_->remove_type(id));
      QCOMPARE(q_->get_type_id("Book"), -1);
      teardown_db();
    }

    void test_get_type_locale_vector()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      // add localised name
      QString tablename = q_->get_type_tablename(id);
      q_->add_translation(tablename, "isbn", QLocale::German, "ISBN-Nummer");
      auto locales = q_->get_type_locale();
      QVERIFY(!locales.empty());
      teardown_db();
    }

    void test_get_table_columns()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QString tablename = q_->get_type_tablename(id);
      // start_index 1 skips item_id
      auto cols = q_->get_table_columns(tablename, 1);
      QVERIFY(!cols.empty());
      teardown_db();
    }

    // ==================================================================
    // Items
    // ==================================================================

    void test_add_and_get_item()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("The Hitchhiker's Guide to the Galaxy");
      item.set_place("Shelf A1");
      item.set_description("Classic science-fiction novel");
      item.set_value("isbn", "978-3-453-14697-2");
      item.set_value("author", "Douglas Adams");
      int item_id = q_->add_new_item(item);
      QVERIFY(item_id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(item_id, got, type_name));
      QCOMPARE(got.get_name(), QString("The Hitchhiker's Guide to the Galaxy"));
      QCOMPARE(got.get_place(), QString("Shelf A1"));
      QCOMPARE(type_name, QString("Book"));
      teardown_db();
    }

    void test_get_item_unknown_id_returns_false()
    {
      setup_db();
      colman::Item got;
      QString type_name;
      QVERIFY(!q_->get_item(9999, got, type_name));
      teardown_db();
    }

    void test_change_item()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Old Title");
      int id = q_->add_new_item(item);
      item.set_name("New Title");
      QVERIFY(q_->change_item(id, item));
      colman::Item got;
      QString type_name;
      q_->get_item(id, got, type_name);
      QCOMPARE(got.get_name(), QString("New Title"));
      teardown_db();
    }

    void test_remove_item_without_lending()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Disposable Item");
      int id = q_->add_new_item(item);
      QVERIFY(q_->remove_item(id));
      colman::Item got;
      QString type_name;
      QVERIFY(!q_->get_item(id, got, type_name));
      teardown_db();
    }

    void test_remove_item_with_open_lending_fails()
    {
      setup_db();
      auto s = setup_scenario(5);
      int lending_id = lend_items(s, 1);
      Q_UNUSED(lending_id)
      QVERIFY(!q_->remove_item(s.item_id));
      teardown_db();
    }

    void test_force_remove_item()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Force Delete Test");
      int id = q_->add_new_item(item);
      QVERIFY(q_->force_remove_item(id));
      colman::Item got;
      QString type_name;
      QVERIFY(!q_->get_item(id, got, type_name));
      teardown_db();
    }

    void test_add_item_with_list_values()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_repeatable_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Tagged Book");
      item.set_list_value("tags", {"science", "fiction", "classic"});
      int id = q_->add_new_item(item);
      QVERIFY(id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_list_values().at("tags"),
               QStringList({"science", "fiction", "classic"}));
      teardown_db();
    }

    void test_change_item_list_values()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_repeatable_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Mutable Item");
      item.set_list_value("tags", {"alpha"});
      int id = q_->add_new_item(item);
      item.set_list_value("tags", {"beta", "gamma"});
      QVERIFY(q_->change_item(id, item));
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_list_values().at("tags"),
               QStringList({"beta", "gamma"}));
      teardown_db();
    }

    void test_remove_item_clears_list_values()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_repeatable_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Temp Item");
      item.set_list_value("tags", {"remove_me"});
      int id = q_->add_new_item(item);
      QVERIFY(q_->force_remove_item(id));
      QSqlQuery check(QSqlDatabase::database(conn_name_));
      check.prepare(
          "SELECT COUNT(*) FROM item_list_values WHERE item_id = :id");
      check.bindValue(":id", id);
      QVERIFY(check.exec());
      QVERIFY(check.next());
      QCOMPARE(check.value(0).toInt(), 0);
      teardown_db();
    }

    void test_remove_type_clears_list_values()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_repeatable_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Type-Owned Item");
      item.set_list_value("tags", {"gone"});
      int id = q_->add_new_item(item);
      QVERIFY(q_->remove_type(type_id));
      QSqlQuery check(QSqlDatabase::database(conn_name_));
      check.prepare(
          "SELECT COUNT(*) FROM item_list_values WHERE item_id = :id");
      check.bindValue(":id", id);
      QVERIFY(check.exec());
      QVERIFY(check.next());
      QCOMPARE(check.value(0).toInt(), 0);
      teardown_db();
    }

    void test_remove_type_clears_common_field_positions()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      QVERIFY(q_->save_field_order(type_id, {"__name__", "__barcode__"}));
      QVERIFY(q_->remove_type(type_id));
      QSqlQuery check(QSqlDatabase::database(conn_name_));
      check.prepare(
          "SELECT COUNT(*) FROM common_field_positions WHERE type_id = :id");
      check.bindValue(":id", type_id);
      QVERIFY(check.exec());
      QVERIFY(check.next());
      QCOMPARE(check.value(0).toInt(), 0);
      teardown_db();
    }

    void test_have_lendings_initially_false()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Never Lent");
      int id = q_->add_new_item(item);
      QVERIFY(!q_->have_lendings(id));
      teardown_db();
    }

    void test_have_lendings_true_after_lending()
    {
      setup_db();
      auto s = setup_scenario();
      QVERIFY(!q_->have_lendings(s.item_id));
      lend_items(s, 1);
      QVERIFY(q_->have_lendings(s.item_id));
      teardown_db();
    }

    void test_restrict_item_to_type_removes_unknown_keys()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Restriction Test");
      item.set_value("isbn", "123");
      item.set_value("unknown_field", "to be deleted");
      q_->restrict_item_to_type(item);
      auto const& vals = item.get_values();
      QVERIFY(vals.find("isbn") != vals.end());
      QVERIFY(vals.find("unknown_field") == vals.end());
      teardown_db();
    }

    void test_restrict_item_to_type_keeps_known_keys()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_value("isbn", "978-3");
      item.set_value("author", "Adams");
      q_->restrict_item_to_type(item);
      auto const& vals = item.get_values();
      QCOMPARE(vals.at("isbn"), QString("978-3"));
      QCOMPARE(vals.at("author"), QString("Adams"));
      teardown_db();
    }

    void test_item_signature_top_and_bottom_stored()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Signature Test");
      item.set_signature_top("BIO");
      item.set_signature_bottom("ADA");
      int id = q_->add_new_item(item);
      QVERIFY(id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_signature_top(), QString("BIO"));
      QCOMPARE(got.get_signature_bottom(), QString("ADA"));
      teardown_db();
    }

    void test_item_signature_top_and_bottom_after_change()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Signature Change");
      int id = q_->add_new_item(item);
      item.set_signature_top("RO");
      item.set_signature_bottom("GOM");
      QVERIFY(q_->change_item(id, item));
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_signature_top(), QString("RO"));
      QCOMPARE(got.get_signature_bottom(), QString("GOM"));
      teardown_db();
    }

    void test_item_signature_empty_by_default()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("No-Signature Item");
      int id = q_->add_new_item(item);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QVERIFY(got.get_signature_top().isEmpty());
      QVERIFY(got.get_signature_bottom().isEmpty());
      teardown_db();
    }

    void test_item_ddc_and_genre_stored()
    {
      setup_db();
      colman::KeyVector keys;
      keys.push_back({"isbn", {colman::DataType::ShortString, {}}});
      keys.push_back({"author", {colman::DataType::ShortString, {}}});
      keys.push_back({"ddc", {colman::DataType::ShortString, {}}});
      keys.push_back({"genre", {colman::DataType::ShortString, {}}});
      int type_id = q_->add_new_type(colman::Type("Book", keys), "books");
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("DDC Genre Test");
      item.set_value("ddc", "823.914");
      item.set_value("genre", "Science Fiction");
      int id = q_->add_new_item(item);
      QVERIFY(id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_values().at("ddc"), QString("823.914"));
      QCOMPARE(got.get_values().at("genre"), QString("Science Fiction"));
      teardown_db();
    }

    void test_item_ddc_and_genre_after_change()
    {
      setup_db();
      colman::KeyVector keys;
      keys.push_back({"isbn", {colman::DataType::ShortString, {}}});
      keys.push_back({"author", {colman::DataType::ShortString, {}}});
      keys.push_back({"ddc", {colman::DataType::ShortString, {}}});
      keys.push_back({"genre", {colman::DataType::ShortString, {}}});
      int type_id = q_->add_new_type(colman::Type("Book", keys), "books");
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("DDC Genre Change");
      item.set_value("ddc", "000");
      item.set_value("genre", "Non-Fiction");
      int id = q_->add_new_item(item);
      item.set_value("ddc", "530");
      item.set_value("genre", "Physics");
      QVERIFY(q_->change_item(id, item));
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_values().at("ddc"), QString("530"));
      QCOMPARE(got.get_values().at("genre"), QString("Physics"));
      teardown_db();
    }

    void test_item_ddc_genre_and_signature_combined()
    {
      setup_db();
      colman::KeyVector keys;
      keys.push_back({"isbn", {colman::DataType::ShortString, {}}});
      keys.push_back({"author", {colman::DataType::ShortString, {}}});
      keys.push_back({"ddc", {colman::DataType::ShortString, {}}});
      keys.push_back({"genre", {colman::DataType::ShortString, {}}});
      int type_id = q_->add_new_type(colman::Type("Book", keys), "books");
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Combined Test");
      item.set_place("Shelf B3");
      item.set_signature_top("530");
      item.set_signature_bottom("FEY");
      item.set_value("ddc", "530");
      item.set_value("genre", "Non-Fiction");
      item.set_value("author", "Feynman");
      int id = q_->add_new_item(item);
      QVERIFY(id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QCOMPARE(got.get_place(), QString("Shelf B3"));
      QCOMPARE(got.get_signature_top(), QString("530"));
      QCOMPARE(got.get_signature_bottom(), QString("FEY"));
      QCOMPARE(got.get_values().at("ddc"), QString("530"));
      QCOMPARE(got.get_values().at("genre"), QString("Non-Fiction"));
      QCOMPARE(got.get_values().at("author"), QString("Feynman"));
      teardown_db();
    }

    void test_item_keywords_stored()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Keywords Test");
      item.set_keywords({"Science Fiction", "Space", "Adams"});
      int id = q_->add_new_item(item);
      QVERIFY(id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      auto const& kw = got.get_keywords();
      QCOMPARE(static_cast<int>(kw.size()), 3);
      QCOMPARE(kw[0], QString("Science Fiction"));
      QCOMPARE(kw[1], QString("Space"));
      QCOMPARE(kw[2], QString("Adams"));
      teardown_db();
    }

    void test_item_keywords_empty_by_default()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("No Keywords");
      int id = q_->add_new_item(item);
      QVERIFY(id >= 0);
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QVERIFY(got.get_keywords().empty());
      teardown_db();
    }

    void test_item_keywords_after_change()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Keywords Change");
      item.set_keywords({"old"});
      int id = q_->add_new_item(item);
      item.set_keywords({"new1", "new2"});
      QVERIFY(q_->change_item(id, item));
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      auto const& kw = got.get_keywords();
      QCOMPARE(static_cast<int>(kw.size()), 2);
      QCOMPARE(kw[0], QString("new1"));
      QCOMPARE(kw[1], QString("new2"));
      teardown_db();
    }

    void test_item_keywords_cleared()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Keywords Clear");
      item.set_keywords({"first", "second"});
      int id = q_->add_new_item(item);
      item.set_keywords({});
      QVERIFY(q_->change_item(id, item));
      colman::Item got;
      QString type_name;
      QVERIFY(q_->get_item(id, got, type_name));
      QVERIFY(got.get_keywords().empty());
      teardown_db();
    }

    void test_keywords_to_string_and_back()
    {
      QStringList const kw = {"Physics", "Thermodynamics", "Feynman"};
      QString const s = colman::keywords_to_string(kw);
      QCOMPARE(s, QString("Physics;Thermodynamics;Feynman"));
      QStringList const back = colman::string_to_keywords(s);
      QCOMPARE(back, kw);
    }

    void test_keywords_to_string_empty()
    {
      QVERIFY(colman::keywords_to_string({}).isEmpty());
    }

    void test_string_to_keywords_empty_string()
    {
      QVERIFY(colman::string_to_keywords("").empty());
    }

    void test_string_to_keywords_single()
    {
      auto const kw = colman::string_to_keywords("Mathematics");
      QCOMPARE(static_cast<int>(kw.size()), 1);
      QCOMPARE(kw[0], QString("Mathematics"));
    }

    // ==================================================================
    // Acquisitions
    // ==================================================================

    void test_add_and_get_acquisition()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Acquisition Item");
      int item_id = q_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(10);
      acq.set_date_time(QDateTime({2024, 1, 15}, {12, 00}));
      int acq_id = q_->add_new_acquisition(acq);
      QVERIFY(acq_id >= 0);
      colman::Acquisition got;
      QVERIFY(q_->get_acquisition(acq_id, got));
      QCOMPARE(got.get_item_id(), item_id);
      QCOMPARE(got.get_count(), 10u);
      QCOMPARE(got.get_date_time(), QDateTime({2024, 1, 15}, {12, 00}));
      teardown_db();
    }

    void test_get_acquisition_unknown_id_returns_false()
    {
      setup_db();
      colman::Acquisition got;
      QVERIFY(!q_->get_acquisition(9999, got));
      teardown_db();
    }

    void test_change_acquisition()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Book");
      int item_id = q_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(5);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      int acq_id = q_->add_new_acquisition(acq);
      acq.set_count(15);
      QVERIFY(q_->change_acquisition(acq_id, acq));
      colman::Acquisition got;
      q_->get_acquisition(acq_id, got);
      QCOMPARE(got.get_count(), 15u);
      teardown_db();
    }

    void test_remove_acquisition()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Book");
      int item_id = q_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(3);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      int acq_id = q_->add_new_acquisition(acq);
      QVERIFY(q_->remove_acquisition(acq_id));
      colman::Acquisition got;
      QVERIFY(!q_->get_acquisition(acq_id, got));
      teardown_db();
    }

    void test_stock_size_and_available_after_acquisition()
    {
      setup_db();
      auto s = setup_scenario(0);
      // before any acquisition
      QCOMPARE(q_->stock_size(s.item_id), 0u);
      QCOMPARE(q_->available(s.item_id), 0u);
      colman::Acquisition acq;
      acq.set_item_id(s.item_id);
      acq.set_count(10);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_acquisition(acq);
      QCOMPARE(q_->stock_size(s.item_id), 10u);
      QCOMPARE(q_->available(s.item_id), 10u);
      teardown_db();
    }

    void test_two_acquisitions_accumulate()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Book");
      int item_id = q_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(5);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_acquisition(acq);
      acq.set_count(7);
      q_->add_new_acquisition(acq);
      QCOMPARE(q_->stock_size(item_id), 12u);
      QCOMPARE(q_->available(item_id), 12u);
      teardown_db();
    }

    // ==================================================================
    // Liquidations
    // ==================================================================

    void test_add_and_get_liquidation()
    {
      setup_db();
      auto s = setup_scenario(10);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(3);
      liq.set_date_time(QDateTime({2024, 6, 1}, {12, 0}));
      int liq_id = q_->add_new_liquidation(liq);
      QVERIFY(liq_id >= 0);
      colman::Liquidation got;
      QVERIFY(q_->get_liquidation(liq_id, got));
      QCOMPARE(got.get_count(), 3u);
      QCOMPARE(got.get_date_time(), QDateTime({2024, 6, 1}, {12, 0}));
      teardown_db();
    }

    void test_get_liquidation_unknown_id_returns_false()
    {
      setup_db();
      colman::Liquidation got;
      QVERIFY(!q_->get_liquidation(9999, got));
      teardown_db();
    }

    void test_stock_and_available_after_liquidation()
    {
      setup_db();
      auto s = setup_scenario(10);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(4);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_liquidation(liq);
      QCOMPARE(q_->stock_size(s.item_id), 6u);
      QCOMPARE(q_->available(s.item_id), 6u);
      teardown_db();
    }

    void test_change_liquidation()
    {
      setup_db();
      auto s = setup_scenario(10);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(2);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      int liq_id = q_->add_new_liquidation(liq);
      liq.set_count(3);
      QVERIFY(q_->change_liquidation(liq_id, liq));
      colman::Liquidation got;
      q_->get_liquidation(liq_id, got);
      QCOMPARE(got.get_count(), 3u);
      teardown_db();
    }

    void test_remove_liquidation()
    {
      setup_db();
      auto s = setup_scenario(10);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(2);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      int liq_id = q_->add_new_liquidation(liq);
      QVERIFY(q_->remove_liquidation(liq_id));
      colman::Liquidation got;
      QVERIFY(!q_->get_liquidation(liq_id, got));
      teardown_db();
    }

    // ==================================================================
    // Persons
    // ==================================================================

    void test_add_and_get_person()
    {
      setup_db();
      auto p = colman::make_test_person("Smith", "Anna");
      int id = q_->add_new_person(p);
      QVERIFY(id >= 0);
      colman::Person got;
      QVERIFY(q_->get_person(id, got));
      QCOMPARE(got.get_lastname(), QString("Smith"));
      QCOMPARE(got.get_firstname(), QString("Anna"));
      QCOMPARE(got.get_title(), QString("Dr."));
      QCOMPARE(got.get_emails(), QString("test@example.com"));
      QCOMPARE(got.get_city(), QString("Sample City"));
      teardown_db();
    }

    void test_get_person_unknown_id_returns_false()
    {
      setup_db();
      colman::Person got;
      QVERIFY(!q_->get_person(9999, got));
      teardown_db();
    }

    void test_change_person()
    {
      setup_db();
      colman::Person p = colman::make_test_person("Jones", "Alice");
      int id = q_->add_new_person(p);
      p.set_lastname("Brown");
      p.set_street("Baker Street 99");
      QVERIFY(q_->change_person(id, p));
      colman::Person got;
      q_->get_person(id, got);
      QCOMPARE(got.get_lastname(), QString("Brown"));
      QCOMPARE(got.get_street(), QString("Baker Street 99"));
      teardown_db();
    }

    void test_remove_person()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(q_->remove_person(id));
      colman::Person got;
      QVERIFY(!q_->get_person(id, got));
      teardown_db();
    }

    void test_person_known_true()
    {
      setup_db();
      q_->add_new_person(colman::make_test_person("Known", "Person"));
      QVERIFY(q_->person_known("Known", "Person"));
      teardown_db();
    }

    void test_person_known_false_for_absent_person()
    {
      setup_db();
      QVERIFY(!q_->person_known("Unknown", "Person"));
      teardown_db();
    }

    void test_external_id_initially_empty()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QCOMPARE(q_->get_person_by_external_id("any-sub"), -1);
      teardown_db();
    }

    void test_set_and_get_person_by_external_id()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "auth-sub-123"));
      QCOMPARE(q_->get_person_by_external_id("auth-sub-123"), id);
      teardown_db();
    }

    void test_get_person_by_external_id_unknown_sub_returns_minus_one()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "auth-sub-123"));
      QCOMPARE(q_->get_person_by_external_id("wrong-sub"), -1);
      teardown_db();
    }

    void test_set_person_external_id_unknown_person_returns_false()
    {
      setup_db();
      QVERIFY(!q_->set_person_external_id(9999, "any-sub"));
      teardown_db();
    }

    void test_external_id_can_be_updated()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "sub-v1"));
      QVERIFY(q_->set_person_external_id(id, "sub-v2"));
      QCOMPARE(q_->get_person_by_external_id("sub-v1"), -1);
      QCOMPARE(q_->get_person_by_external_id("sub-v2"), id);
      teardown_db();
    }

    void test_external_ids_are_independent_per_person()
    {
      setup_db();
      int id1 = q_->add_new_person(colman::make_test_person("Alpha", "A"));
      int id2 = q_->add_new_person(colman::make_test_person("Beta", "B"));
      QVERIFY(id1 >= 0);
      QVERIFY(id2 >= 0);
      QVERIFY(q_->set_person_external_id(id1, "sub-alpha"));
      QVERIFY(q_->set_person_external_id(id2, "sub-beta"));
      QCOMPARE(q_->get_person_by_external_id("sub-alpha"), id1);
      QCOMPARE(q_->get_person_by_external_id("sub-beta"), id2);
      teardown_db();
    }

    void test_get_person_by_barcode()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QString bc = q_->compute_person_barcode(id);
      QVERIFY(!bc.isEmpty());
      QCOMPARE(q_->get_person(bc), id);
      teardown_db();
    }

    // ==================================================================
    // username (get_username / set_username / remove_username)
    // ==================================================================

    void test_username_initially_empty()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->get_username(id).isEmpty());
      teardown_db();
    }

    void test_set_and_get_username()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_username(id, QStringLiteral("alice")));
      QCOMPARE(q_->get_username(id), QStringLiteral("alice"));
      teardown_db();
    }

    void test_username_can_be_updated()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_username(id, QStringLiteral("alice")));
      QVERIFY(q_->set_username(id, QStringLiteral("alice2")));
      QCOMPARE(q_->get_username(id), QStringLiteral("alice2"));
      teardown_db();
    }

    void test_set_username_duplicate_returns_false()
    {
      setup_db();
      int id1 = q_->add_new_person(colman::make_test_person("Alpha", "A"));
      int id2 = q_->add_new_person(colman::make_test_person("Beta", "B"));
      QVERIFY(id1 >= 0);
      QVERIFY(id2 >= 0);
      QVERIFY(q_->set_username(id1, QStringLiteral("shared")));
      QVERIFY(!q_->set_username(id2, QStringLiteral("shared")));
      QVERIFY(q_->get_username(id2).isEmpty());
      teardown_db();
    }

    void test_remove_username()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_username(id, QStringLiteral("alice")));
      QVERIFY(q_->remove_username(id));
      QVERIFY(q_->get_username(id).isEmpty());
      teardown_db();
    }

    void test_username_on_admin_person()
    {
      setup_db();
      QVERIFY(q_->set_username(0, QStringLiteral("admin")));
      QCOMPARE(q_->get_username(0), QStringLiteral("admin"));
      QVERIFY(q_->remove_username(0));
      QVERIFY(q_->get_username(0).isEmpty());
      teardown_db();
    }

    void test_usernames_are_independent_per_person()
    {
      setup_db();
      int id1 = q_->add_new_person(colman::make_test_person("Alpha", "A"));
      int id2 = q_->add_new_person(colman::make_test_person("Beta", "B"));
      QVERIFY(id1 >= 0);
      QVERIFY(id2 >= 0);
      QVERIFY(q_->set_username(id1, QStringLiteral("alice")));
      QVERIFY(q_->set_username(id2, QStringLiteral("bob")));
      QCOMPARE(q_->get_username(id1), QStringLiteral("alice"));
      QCOMPARE(q_->get_username(id2), QStringLiteral("bob"));
      teardown_db();
    }

    void test_query_change_passwords_persons_includes_username()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person("Doe", "John"));
      QVERIFY(id >= 0);
      QVERIFY(q_->set_username(id, QStringLiteral("jdoe")));
      auto rows = q_->query_change_passwords_persons();
      bool found = false;
      for(auto const& r : rows) {
        if(r.id == id) {
          QCOMPARE(r.username, QStringLiteral("jdoe"));
          found = true;
        }
      }
      QVERIFY(found);
      teardown_db();
    }

    void test_query_change_passwords_persons_empty_username_when_not_set()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      auto rows = q_->query_change_passwords_persons();
      bool found = false;
      for(auto const& r : rows) {
        if(r.id == id) {
          QVERIFY(r.username.isEmpty());
          found = true;
        }
      }
      QVERIFY(found);
      teardown_db();
    }

    // ==================================================================
    // has_non_admin_persons
    // ==================================================================

    void test_has_non_admin_persons_false_when_table_empty()
    {
      // Fresh database: persons table is empty, so no non-admin persons exist.
      setup_db();
      QVERIFY(!q_->has_non_admin_persons());
      teardown_db();
    }

    void test_has_non_admin_persons_true_after_adding_one_person()
    {
      // A regular person (id > 0, assigned by add_new_person) is detected.
      setup_db();
      QVERIFY(q_->add_new_person(colman::make_test_person()) >= 0);
      QVERIFY(q_->has_non_admin_persons());
      teardown_db();
    }

    void test_has_non_admin_persons_true_with_multiple_persons()
    {
      // Holds even when several regular persons are present.
      setup_db();
      QVERIFY(q_->add_new_person(colman::make_test_person("Alpha", "Anna")) >= 0);
      QVERIFY(q_->add_new_person(colman::make_test_person("Beta", "Ben")) >= 0);
      QVERIFY(q_->has_non_admin_persons());
      teardown_db();
    }

    void test_has_non_admin_persons_false_after_removing_all_persons()
    {
      // After the only regular person is removed the result is false again.
      setup_db();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->has_non_admin_persons());
      QVERIFY(q_->remove_person(id));
      QVERIFY(!q_->has_non_admin_persons());
      teardown_db();
    }

    // ==================================================================
    // Barcodes
    // ==================================================================

    void test_compute_and_parse_item_barcode()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Barcode Book");
      int item_id = q_->add_new_item(item);
      QString bc = q_->compute_item_barcode(type_id, item_id);
      QVERIFY(!bc.isEmpty());
      QVERIFY(q_->is_item_barcode(bc));
      int parsed_type = -1, parsed_item = -1;
      QVERIFY(q_->parse_item_barcode(bc, parsed_type, parsed_item));
      QCOMPARE(parsed_type, type_id);
      QCOMPARE(parsed_item, item_id);
      teardown_db();
    }

    void test_is_item_barcode_rejects_invalid()
    {
      setup_db();
      q_->add_new_type(colman::make_book_type());
      QVERIFY(!q_->is_item_barcode("INVALID_BARCODE"));
      QVERIFY(!q_->is_item_barcode(""));
      teardown_db();
    }

    void test_compute_and_parse_person_barcode()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QString bc = q_->compute_person_barcode(id);
      QVERIFY(!bc.isEmpty());
      QVERIFY(q_->is_person_barcode(bc));
      int parsed_id = -1;
      QVERIFY(q_->parse_person_barcode(bc, parsed_id));
      QCOMPARE(parsed_id, id);
      teardown_db();
    }

    void test_is_person_barcode_rejects_invalid()
    {
      setup_db();
      QVERIFY(!q_->is_person_barcode("INVALID_BARCODE"));
      QVERIFY(!q_->is_person_barcode(""));
      teardown_db();
    }

    void test_item_barcode_differs_from_person_barcode()
    {
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Book");
      int item_id = q_->add_new_item(item);
      int person_id = q_->add_new_person(colman::make_test_person());
      QString item_bc = q_->compute_item_barcode(type_id, item_id);
      QString person_bc = q_->compute_person_barcode(person_id);
      QVERIFY(item_bc != person_bc);
      QVERIFY(q_->is_item_barcode(item_bc));
      QVERIFY(!q_->is_person_barcode(item_bc));
      QVERIFY(q_->is_person_barcode(person_bc));
      QVERIFY(!q_->is_item_barcode(person_bc));
      teardown_db();
    }

    /// Returns a collection with the numeric barcode prefix "54861".
    static colman::Collection numeric_prefix_collection()
    {
      colman::Collection c = colman::make_test_collection();
      c.set_barcode_prefix("54861");
      return c;
    }

    /// Returns true if @p s is a digit string with a valid GTIN check digit.
    static bool is_valid_gtin(QString const& s)
    {
      auto sum = 0;
      for(auto i = 0; i < s.size(); ++i) {
        auto const d = s[s.size() - 1 - i].digitValue();
        if(d < 0) {
          return false;
        }
        sum += ((i % 2 == 1) ? 3 : 1) * d;
      }
      return sum % 10 == 0;
    }

    void test_barcode_format2_examples()
    {
      auto const c = numeric_prefix_collection();
      QCOMPARE(colman::compute_item_barcode(c, 1, 1027),
               QString("548610011410271"));
      QCOMPARE(colman::compute_person_barcode(c, 227),
               QString("5486100032279"));
    }

    void test_barcode_format2_never_isbn_or_ean()
    {
      auto const c = numeric_prefix_collection();
      for(auto id = 1; id < 100000; ++id) {
        auto const person_bc = colman::compute_person_barcode(c, id);
        auto person_id = 0;
        QVERIFY(colman::parse_person_barcode(c, person_bc, person_id));
        QCOMPARE(person_id, id);
        QVERIFY(!colman::is_item_barcode(c, person_bc));
        QVERIFY(person_bc.size() >= 11);
        QVERIFY(!is_valid_gtin(person_bc));
        QVERIFY(!colman::is_isbn10(person_bc));
        QVERIFY(!colman::is_isbn13(person_bc));
        for(auto type_id : {1, 9, 12}) {
          auto const item_bc = colman::compute_item_barcode(c, type_id, id);
          auto parsed_type = 0;
          auto parsed_item = 0;
          QVERIFY(colman::parse_item_barcode(c, item_bc, parsed_type,
                                             parsed_item));
          QCOMPARE(parsed_type, type_id);
          QCOMPARE(parsed_item, id);
          QVERIFY(!colman::is_person_barcode(c, item_bc));
          QVERIFY(item_bc.size() >= 12);
          QVERIFY(!is_valid_gtin(item_bc));
          QVERIFY(!colman::is_isbn13(item_bc));
        }
      }
    }

    void test_barcode_format1_still_parsed()
    {
      auto const c = numeric_prefix_collection();
      auto type_id = 0;
      auto item_id = 0;
      QVERIFY(colman::parse_item_barcode(c, "5486111410276", type_id,
                                         item_id));
      QCOMPARE(type_id, 1);
      QCOMPARE(item_id, 1027);
      for(auto id = 1; id < 100000; id += 13) {
        auto const item_bc = colman::legacy_item_barcode("54861", 3, id);
        QVERIFY(colman::parse_item_barcode(c, item_bc, type_id, item_id));
        QCOMPARE(type_id, 3);
        QCOMPARE(item_id, id);
        QVERIFY(!colman::is_person_barcode(c, item_bc));
        auto const person_bc = colman::legacy_person_barcode("54861", id);
        auto person_id = 0;
        QVERIFY(colman::parse_person_barcode(c, person_bc, person_id));
        QCOMPARE(person_id, id);
        QVERIFY(!colman::is_item_barcode(c, person_bc));
      }
    }

    void test_barcode_wrong_check_digit_rejected()
    {
      auto const c = numeric_prefix_collection();
      auto const item_bc = colman::compute_item_barcode(c, 2, 4711);
      auto const person_bc = colman::compute_person_barcode(c, 4711);
      for(auto d = '0'; d <= '9'; ++d) {
        auto bad_item = item_bc;
        bad_item[bad_item.size() - 1] = QChar(d);
        QCOMPARE(colman::is_item_barcode(c, bad_item), bad_item == item_bc);
        auto bad_person = person_bc;
        bad_person[bad_person.size() - 1] = QChar(d);
        QCOMPARE(colman::is_person_barcode(c, bad_person),
                 bad_person == person_bc);
      }
        // Format 2 body with a format 1 check digit is not accepted.
      auto const body = item_bc.left(item_bc.size() - 1);
      auto const legacy_check = colman::with_legacy_check_digit(body);
      if(legacy_check != item_bc) {
        QVERIFY(!colman::is_item_barcode(c, legacy_check));
      }
      QVERIFY(!colman::is_item_barcode(c, "54861"));
      QVERIFY(!colman::is_person_barcode(c, "5486100"));
      QVERIFY(!colman::is_item_barcode(c, "6486100114102712"));
    }

    // ==================================================================
    // Lendings
    // ==================================================================

    void test_add_and_get_lending()
    {
      setup_db();
      auto s = setup_scenario(10);
      colman::Lending l;
      l.set_item_id(s.item_id);
      l.set_person_id(s.person_id);
      l.set_count(3);
      l.set_date_time(QDateTime({2024, 3, 1}, {12, 0}));
      l.set_duedate(QDate(2024, 4, 1));
      l.set_remarks("Test Lending");
      int id = q_->add_new_lending(l);
      QVERIFY(id >= 0);
      colman::Lending got;
      QVERIFY(q_->get_lending(id, got));
      QCOMPARE(got.get_item_id(), s.item_id);
      QCOMPARE(got.get_person_id(), s.person_id);
      QCOMPARE(got.get_count(), 3u);
      QCOMPARE(got.get_date_time(), QDateTime({2024, 3, 1}, {12, 0}));
      QCOMPARE(got.get_duedate(), QDate(2024, 4, 1));
      QCOMPARE(got.get_remarks(), QString("Test Lending"));
      teardown_db();
    }

    void test_get_lending_unknown_id_returns_false()
    {
      setup_db();
      colman::Lending got;
      QVERIFY(!q_->get_lending(9999, got));
      teardown_db();
    }

    void test_lending_reduces_available()
    {
      setup_db();
      auto s = setup_scenario(10);
      lend_items(s, 4);
      QCOMPARE(q_->available(s.item_id), 6u);
      teardown_db();
    }

    void test_lending_exceeding_available_fails()
    {
      setup_db();
      auto s = setup_scenario(5);
      colman::Lending l;
      l.set_item_id(s.item_id);
      l.set_person_id(s.person_id);
      l.set_count(10); // more than available
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QCOMPARE(q_->add_new_lending(l), -1);
      QCOMPARE(q_->available(s.item_id), 5u); // unchanged
      teardown_db();
    }

    void test_group_lending_lends_one_copy_each()
    {
      setup_db();
      auto s = setup_scenario(5);
      int const p2 = q_->add_new_person(colman::make_test_person("B", "b"));
      int const p3 = q_->add_new_person(colman::make_test_person("C", "c"));
      auto const res = q_->add_group_lending(group_template(s.item_id),
                                             {s.person_id, p2, p3});
      QCOMPARE(res.status_,
               colman::IQueries::GroupLendingResult::Status::Ok);
      QCOMPARE(static_cast<int>(res.lending_ids_.size()), 3);
      QCOMPARE(q_->available(s.item_id), 2u);
      colman::Lending got;
      QVERIFY(q_->get_lending(res.lending_ids_[1], got));
      QCOMPARE(got.get_person_id(), p2);
      QCOMPARE(got.get_count(), 1u);
      QCOMPARE(got.get_remarks(), QString("class set"));
      teardown_db();
    }

    void test_group_lending_over_capacity_changes_nothing()
    {
      setup_db();
      auto s = setup_scenario(2);
      int const p2 = q_->add_new_person(colman::make_test_person("B", "b"));
      int const p3 = q_->add_new_person(colman::make_test_person("C", "c"));
      auto const res = q_->add_group_lending(group_template(s.item_id),
                                             {s.person_id, p2, p3});
      QCOMPARE(res.status_, colman::IQueries::GroupLendingResult::Status::
                                NotEnoughAvailable);
      QCOMPARE(res.lendable_, 2);
      QVERIFY(res.lending_ids_.empty());
      QCOMPARE(q_->available(s.item_id), 2u);
      QVERIFY(q_->open_lendings_for_item(s.item_id).empty());
      teardown_db();
    }

    void test_group_lending_respects_reservations()
    {
      setup_db();
      auto s = setup_scenario(4);
      int const p2 = q_->add_new_person(colman::make_test_person("B", "b"));
      int const p3 = q_->add_new_person(colman::make_test_person("C", "c"));
      int const outsider =
          q_->add_new_person(colman::make_test_person("D", "d"));
      auto const expires = QDate::currentDate().addDays(7);
      int res_id = 0;
      // An outsider blocks one copy, a group member reserved two.
      QCOMPARE(q_->add_reservation(s.item_id, outsider, 1, expires, 0,
                                   res_id),
               colman::IQueries::ReservationResult::Ok);
      QCOMPARE(q_->add_reservation(s.item_id, p2, 2, expires, 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      // 4 copies - 1 (outsider) - 1 (p2 keeps one reserved) = 2 lendable.
      auto res = q_->add_group_lending(group_template(s.item_id),
                                       {s.person_id, p2, p3});
      QCOMPARE(res.status_, colman::IQueries::GroupLendingResult::Status::
                                NotEnoughAvailable);
      QCOMPARE(res.lendable_, 2);
      res = q_->add_group_lending(group_template(s.item_id),
                                  {s.person_id, p2});
      QCOMPARE(res.status_,
               colman::IQueries::GroupLendingResult::Status::Ok);
      QCOMPARE(q_->available(s.item_id), 2u);
      // p2's reservation was reduced by the copy they received.
      auto const ress = q_->get_reservations_for_item(s.item_id);
      int p2_reserved = 0;
      for(auto const& r : ress) {
        if(r.person_id == p2) {
          p2_reserved = r.count;
        }
      }
      QCOMPARE(p2_reserved, 1);
      teardown_db();
    }

    void test_group_lending_rejects_current_holder()
    {
      setup_db();
      auto s = setup_scenario(5);
      lend_items(s, 1);
      int const p2 = q_->add_new_person(colman::make_test_person("B", "b"));
      auto const res = q_->add_group_lending(group_template(s.item_id),
                                             {p2, s.person_id});
      QCOMPARE(res.status_,
               colman::IQueries::GroupLendingResult::Status::AlreadyLent);
      QCOMPARE(res.person_id_, s.person_id);
      QCOMPARE(q_->available(s.item_id), 4u);
      teardown_db();
    }

    void test_group_return_closes_all_open_lendings()
    {
      using Status = colman::IQueries::GroupReturnResult::Status;
      setup_db();
      auto s = setup_scenario(10);
      int const p2 = q_->add_new_person(colman::make_test_person("B", "b"));
      int const p3 = q_->add_new_person(colman::make_test_person("C", "c"));
      // s.person_id: two lendings (1 + 2 copies, one copy already back)
      int const first = lend_items(s, 1);
      int const second = lend_items(s, 2);
      colman::Return partial;
      partial.set_lending_id(second);
      partial.set_count(1);
      partial.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_return(partial) > 0);
      QCOMPARE(q_->add_group_lending(group_template(s.item_id), {p2, p3})
                   .status_,
               colman::IQueries::GroupLendingResult::Status::Ok);
      QCOMPARE(q_->available(s.item_id), 6u);
      colman::Return tmpl;
      tmpl.set_date_time(QDateTime::currentDateTimeUtc());
      tmpl.set_remarks("collected");
      auto const res =
          q_->add_group_return(s.item_id, {s.person_id, p2}, tmpl);
      QCOMPARE(res.status_, Status::Ok);
      QCOMPARE(static_cast<int>(res.return_ids_.size()), 3);
      QCOMPARE(res.returned_count_, 3);
      QCOMPARE(q_->available(s.item_id), 9u);
      std::size_t lent = 0;
      std::size_t returned = 0;
      QVERIFY(q_->lending_balance(first, lent, returned));
      QCOMPARE(returned, lent);
      QVERIFY(q_->lending_balance(second, lent, returned));
      QCOMPARE(returned, lent);
      // only p3 still holds the item
      auto const open = q_->open_lendings_for_item(s.item_id);
      QCOMPARE(static_cast<int>(open.size()), 1);
      QCOMPARE(open.front().second, p3);
      teardown_db();
    }

    void test_group_return_rejects_person_without_lending()
    {
      using Status = colman::IQueries::GroupReturnResult::Status;
      setup_db();
      auto s = setup_scenario(5);
      lend_items(s, 1);
      int const p2 = q_->add_new_person(colman::make_test_person("B", "b"));
      colman::Return tmpl;
      tmpl.set_date_time(QDateTime::currentDateTimeUtc());
      auto const res = q_->add_group_return(s.item_id, {s.person_id, p2},
                                            tmpl);
      QCOMPARE(res.status_, Status::NotLent);
      QCOMPARE(res.person_id_, p2);
      QCOMPARE(q_->available(s.item_id), 4u);
      QCOMPARE(static_cast<int>(q_->open_lendings_for_item(s.item_id).size()),
               1);
      teardown_db();
    }

    void test_lending_persons_have_school_class()
    {
      setup_db();
      auto s = setup_scenario(5);
      colman::Person p;
      QVERIFY(q_->get_person(s.person_id, p));
      p.set_school_class("5a");
      QVERIFY(q_->change_person(s.person_id, p));
      lend_items(s, 1);
      auto const persons =
          q_->query_lending_persons(s.item_id, -1, QString(), QString());
      QCOMPARE(static_cast<int>(persons.size()), 1);
      QCOMPARE(persons.front().get_school_class(), QString("5a"));
      teardown_db();
    }

    void test_change_lending()
    {
      setup_db();
      auto s = setup_scenario(10);
      int id = lend_items(s, 2);
      colman::Lending l;
      QVERIFY(q_->get_lending(id, l));
      l.set_remarks("changed");
      QVERIFY(q_->change_lending(id, l));
      colman::Lending got;
      q_->get_lending(id, got);
      QCOMPARE(got.get_remarks(), QString("changed"));
      teardown_db();
    }

    void test_remove_lending()
    {
      setup_db();
      auto s = setup_scenario(10);
      int id = lend_items(s, 1);
      QVERIFY(q_->remove_lending(id));
      colman::Lending got;
      QVERIFY(!q_->get_lending(id, got));
      teardown_db();
    }

    void test_lending_balance_no_returns()
    {
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      size_t lent = 0, returned = 0;
      QVERIFY(q_->lending_balance(lending_id, lent, returned));
      QCOMPARE(lent, size_t(5));
      QCOMPARE(returned, size_t(0));
      teardown_db();
    }

    // ==================================================================
    // Returns
    // ==================================================================

    void test_add_return_partial()
    {
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(2);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      int ret_id = q_->add_new_return(ret);
      QVERIFY(ret_id >= 0);
      QCOMPARE(q_->available(s.item_id), 7u);
      teardown_db();
    }

    void test_return_exceeding_lent_fails()
    {
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 3);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(5); // more than lent
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      QCOMPARE(q_->add_new_return(ret), -1);
      teardown_db();
    }

    void test_full_return_restores_availability()
    {
      // add_new_return does not auto-archive a lending; it only records
      // the return and updates availability. A lending stays in the active
      // table until explicitly archived at a higher level.
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 3);
      QCOMPARE(q_->available(s.item_id), 7u);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(3);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_return(ret) >= 0);
      // Availability restored after full return
      QCOMPARE(q_->available(s.item_id), 10u);
      // colman::Lending record still exists (not auto-archived at SQL level)
      colman::Lending still_there;
      QVERIFY(q_->get_lending(lending_id, still_there));
      teardown_db();
    }

    void test_get_and_change_return()
    {
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(2);
      ret.set_date_time(QDateTime({2024, 5, 10}, {12, 0}));
      ret.set_remarks("first return");
      int ret_id = q_->add_new_return(ret);
      colman::Return got;
      QVERIFY(q_->get_return(ret_id, got));
      QCOMPARE(got.get_count(), 2u);
      QCOMPARE(got.get_date_time(), QDateTime({2024, 5, 10}, {12, 0}));
      ret.set_remarks("changed");
      QVERIFY(q_->change_return(ret_id, ret));
      colman::Return got2;
      q_->get_return(ret_id, got2);
      QCOMPARE(got2.get_remarks(), QString("changed"));
      teardown_db();
    }

    void test_remove_return()
    {
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(2);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      int ret_id = q_->add_new_return(ret);
      QVERIFY(q_->remove_return(ret_id));
      colman::Return got;
      QVERIFY(!q_->get_return(ret_id, got));
      teardown_db();
    }

    void test_lending_balance_after_partial_returns()
    {
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 6);
      colman::Return r1;
      r1.set_lending_id(lending_id);
      r1.set_count(2);
      r1.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_return(r1);
      colman::Return r2;
      r2.set_lending_id(lending_id);
      r2.set_count(3);
      r2.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_return(r2);
      size_t lent = 0, returned = 0;
      QVERIFY(q_->lending_balance(lending_id, lent, returned));
      QCOMPARE(lent, size_t(6));
      QCOMPARE(returned, size_t(5));
      teardown_db();
    }

    // ==================================================================
    // Integrated workflow: multiple lendings, partial and full returns
    // ==================================================================

    void test_multiple_lendings_and_returns()
    {
      // add_new_return does not auto-archive: lendings stay active.
      // Only availability is updated.
      setup_db();
      auto s = setup_scenario(20);
      // first lending: 6 copies
      int l1 = lend_items(s, 6);
      QCOMPARE(q_->available(s.item_id), 14u);
      // second person also borrows 4 copies
      auto p2 = colman::make_test_person("Second", "Person");
      int p2_id = q_->add_new_person(p2);
      colman::Lending l2_data;
      l2_data.set_item_id(s.item_id);
      l2_data.set_person_id(p2_id);
      l2_data.set_count(4);
      l2_data.set_date_time(QDateTime::currentDateTimeUtc());
      int l2 = q_->add_new_lending(l2_data);
      QCOMPARE(q_->available(s.item_id), 10u);
      // partial return on first lending
      colman::Return r1;
      r1.set_lending_id(l1);
      r1.set_count(3);
      r1.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_return(r1);
      QCOMPARE(q_->available(s.item_id), 13u);
      // full return on second lending
      colman::Return r2;
      r2.set_lending_id(l2);
      r2.set_count(4);
      r2.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_return(r2);
      QCOMPARE(q_->available(s.item_id), 17u);
      // return the remaining 3 of l1
      colman::Return r3;
      r3.set_lending_id(l1);
      r3.set_count(3);
      r3.set_date_time(QDateTime::currentDateTimeUtc());
      q_->add_new_return(r3);
      QCOMPARE(q_->available(s.item_id), 20u);
      teardown_db();
    }

    void test_item_removal_blocked_while_lent()
    {
      setup_db();
      auto s = setup_scenario(5);
      lend_items(s, 2);
      // can't remove item while copies are out
      QVERIFY(!q_->remove_item(s.item_id));
      // but force_remove succeeds
      QVERIFY(q_->force_remove_item(s.item_id));
      colman::Item gone;
      QString tn;
      QVERIFY(!q_->get_item(s.item_id, gone, tn));
      teardown_db();
    }

    // ==================================================================
    // Translations
    // ==================================================================

    void test_add_and_get_translation()
    {
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QString tablename = q_->get_type_tablename(id);
      QVERIFY(q_->add_translation(tablename, "isbn", QLocale::German,
                                  "ISBN-Nummer"));
      QString translated =
          q_->get_translation(tablename, "isbn", QLocale::German);
      QCOMPARE(translated, QString("ISBN-Nummer"));
      teardown_db();
    }

    void test_get_translation_missing_returns_empty()
    {
      // add_new_type() creates AnyLanguage and system_language() entries.
      // A language that is neither of those must still return empty.
      setup_db();
      int id = q_->add_new_type(colman::make_book_type());
      QString tablename = q_->get_type_tablename(id);
      QString result = q_->get_translation(tablename, "isbn", QLocale::French);
      QVERIFY(result.isEmpty());
      teardown_db();
    }

    // ==================================================================
    // string_to_column_name
    // ==================================================================

    void test_type_book_translated_to_buch()
    {
      setup_db();
      QVERIFY(q_->add_translation("types", "book", QLocale::German, "Buch"));
      colman::KeyVector keys;
      keys.push_back({"title", {colman::DataType::ShortString, {}}});
      int id = q_->add_new_type(colman::Type("book", keys), "books");
      QVERIFY(id > 0);

      // vector version
      auto locales = q_->get_type_locale();
      QVERIFY(!locales.empty());
      bool found_buch = false;
      for(auto const& p : locales) {
        if(p.second == "Buch") {
          found_buch = true;
        }
      }
      QVERIFY2(found_buch, "Expected type 'book' to be translated to 'Buch'");

      // by-id version
      QCOMPARE(q_->get_type_locale(id), QString("Buch"));

      // by-name version
      QCOMPARE(q_->get_type_locale(QString("book")), QString("Buch"));

      teardown_db();
    }

    /// Regression: get_type_locale(int) previously returned the first type's
    /// name for every id because the WHERE condition was in the LEFT JOIN ON
    /// clause instead of a WHERE clause.
    void test_get_type_locale_by_id_returns_correct_type_when_multiple_exist()
    {
      setup_db();
      colman::KeyVector kv;
      kv.push_back({"title", {colman::DataType::ShortString, {}}});
      int const id1 = q_->add_new_type(colman::Type("book", kv), "books");
      int const id2 = q_->add_new_type(colman::Type("cd", kv), "cds");
      QVERIFY(id1 > 0);
      QVERIFY(id2 > 0);
      QVERIFY(id1 != id2);
      q_->add_translation("types", "book", QLocale::German, "Buch");
      q_->add_translation("types", "cd", QLocale::German, "CD");
      QCOMPARE(q_->get_type_locale(id1), QString("Buch"));
      QCOMPARE(q_->get_type_locale(id2), QString("CD"));
      teardown_db();
    }

    // ==================================================================
    // Media kind (search category)
    // ==================================================================

    /// add_new_type stores the media kind; get_type reads it back; change_type
    /// updates it.
    void test_media_kind_round_trip()
    {
      setup_db();
      colman::KeyVector kv;
      kv.push_back({"title", {colman::DataType::ShortString, {}}});
      colman::Type dvd("dvd", kv);
      dvd.set_media_kind(colman::MediaKind::Dvd);
      int const id = q_->add_new_type(dvd, "dvds");
      QVERIFY(id > 0);
      QCOMPARE(q_->get_type(id).get_media_kind(), colman::MediaKind::Dvd);
      QCOMPARE(q_->media_kind_for_type_id(id), colman::MediaKind::Dvd);

      colman::Type changed = q_->get_type(id);
      changed.set_media_kind(colman::MediaKind::BluRay);
      QVERIFY(q_->change_type(id, changed, {}));
      QCOMPARE(q_->media_kind_for_type_id(id), colman::MediaKind::BluRay);
      teardown_db();
    }

    /// type_id_for_media_kind returns the lowest id when several types share a
    /// kind, and -1 when no type carries the kind.
    void test_type_id_for_media_kind_lowest_id_wins()
    {
      setup_db();
      colman::KeyVector kv;
      kv.push_back({"title", {colman::DataType::ShortString, {}}});
      colman::Type dvd1("dvd_main", kv);
      dvd1.set_media_kind(colman::MediaKind::Dvd);
      colman::Type dvd2("dvd_alt", kv);
      dvd2.set_media_kind(colman::MediaKind::Dvd);
      int const id1 = q_->add_new_type(dvd1, "dvd_main");
      int const id2 = q_->add_new_type(dvd2, "dvd_alt");
      QVERIFY(id1 > 0 && id2 > 0 && id1 < id2);
      QCOMPARE(q_->type_id_for_media_kind(colman::MediaKind::Dvd), id1);
      // No type carries the BoardGame kind in this database.
      QCOMPARE(q_->type_id_for_media_kind(colman::MediaKind::BoardGame), -1);
      // Unknown never resolves to a type.
      QCOMPARE(q_->type_id_for_media_kind(colman::MediaKind::Unknown), -1);
      teardown_db();
    }

    /// Databases created before the media_kind column existed are migrated on
    /// create_tables(): the column is added and built-in types are back-filled
    /// by their canonical name; the migration is idempotent.
    void test_media_kind_migration_backfills_builtins()
    {
      conn_name_ = QString("colman_test_%1").arg(conn_counter_++);
      q_.emplace(conn_name_);
      // Simulate a pre-media_kind schema: a types table without the column,
      // populated with built-in and user-defined types.
      {
        QSqlQuery setup(QSqlDatabase::database(conn_name_));
        QVERIFY(setup.exec("CREATE TABLE types (id INTEGER PRIMARY KEY, "
                           "name VARCHAR, tablename VARCHAR)"));
        QVERIFY(setup.exec("INSERT INTO types (name, tablename) VALUES "
                           "('book','books'),('bluray','blurays'),"
                           "('mything','mythings')"));
      }
      // Migration runs inside create_tables().
      QVERIFY(q_->create_tables());
      QCOMPARE(q_->media_kind_for_type_id(q_->get_type_id("book")),
               colman::MediaKind::Book);
      QCOMPARE(q_->media_kind_for_type_id(q_->get_type_id("bluray")),
               colman::MediaKind::BluRay);
      // User-defined types default to Unknown until assigned a kind.
      QCOMPARE(q_->media_kind_for_type_id(q_->get_type_id("mything")),
               colman::MediaKind::Unknown);
      // Regression: after migration, listing the types (left column of the
      // category page) and inserting a new type must work — both query the
      // media_kind column.
      QVERIFY(!q_->query_types(-1).empty());
      colman::KeyVector kv;
      kv.push_back({"title", {colman::DataType::ShortString, {}}});
      QVERIFY(q_->add_new_type(colman::Type("newtype", kv), "newtypes") > 0);
      // A second create_tables() must not crash or change the back-fill.
      QVERIFY(q_->create_tables());
      QCOMPARE(q_->media_kind_for_type_id(q_->get_type_id("book")),
               colman::MediaKind::Book);
      teardown_db();
    }

    // ==================================================================
    // Permissions
    // ==================================================================

    void test_create_default_permissions_admin()
    {
      setup_db();
      QVERIFY(q_->has_permission(0, "edit_persons"));
      QVERIFY(q_->has_permission(0, "create_lenders"));
      QVERIFY(q_->has_permission(0, "create_data_collectors"));
      QVERIFY(q_->has_permission(0, "collect_lendings"));
      QVERIFY(q_->has_permission(0, "collect_returns"));
      QVERIFY(q_->has_permission(0, "collect_acquisitions"));
      QVERIFY(q_->has_permission(0, "collect_liquidations"));
      QVERIFY(q_->has_permission(0, "view_history"));
      QVERIFY(q_->has_permission(0, "view_persons"));
      QVERIFY(q_->has_permission(0, "view_items"));
      QVERIFY(q_->has_permission(0, "edit_preferences"));
      QVERIFY(q_->has_permission(0, "print_ids"));
      QVERIFY(q_->has_permission(0, "print_barcodes"));
      QVERIFY(q_->has_permission(0, "edit_categories"));
      QVERIFY(q_->has_permission(0, "view_personal_data"));
      teardown_db();
    }

    void test_create_default_permissions_regular_user()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id > 0);
      // add_new_person already calls create_default_permissions
      QVERIFY(!q_->has_permission(id, "edit_persons"));
      QVERIFY(!q_->has_permission(id, "create_lenders"));
      QVERIFY(!q_->has_permission(id, "create_data_collectors"));
      QVERIFY(!q_->has_permission(id, "collect_lendings"));
      QVERIFY(!q_->has_permission(id, "collect_returns"));
      QVERIFY(!q_->has_permission(id, "collect_acquisitions"));
      QVERIFY(!q_->has_permission(id, "collect_liquidations"));
      QVERIFY(!q_->has_permission(id, "view_history"));
      QVERIFY(!q_->has_permission(id, "view_persons"));
      QVERIFY(!q_->has_permission(id, "view_items"));
      QVERIFY(!q_->has_permission(id, "edit_preferences"));
      QVERIFY(!q_->has_permission(id, "print_ids"));
      QVERIFY(!q_->has_permission(id, "print_barcodes"));
      QVERIFY(!q_->has_permission(id, "edit_categories"));
      QVERIFY(q_->has_permission(id, "view_personal_data"));
      teardown_db();
    }

    void test_set_permission()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id > 0);
      QVERIFY(!q_->has_permission(id, "collect_lendings"));
      QVERIFY(q_->set_permission(id, "collect_lendings", true));
      QVERIFY(q_->has_permission(id, "collect_lendings"));
      QVERIFY(q_->set_permission(id, "collect_lendings", false));
      QVERIFY(!q_->has_permission(id, "collect_lendings"));
      teardown_db();
    }

    void test_remove_permissions()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id > 0);
      QVERIFY(q_->has_permission(id, "view_personal_data"));
      QVERIFY(q_->remove_permissions(id));
      QVERIFY(!q_->has_permission(id, "view_personal_data"));
      teardown_db();
    }

    void test_remove_person_cascades_permissions()
    {
      setup_db();
      int id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id > 0);
      QVERIFY(q_->has_permission(id, "view_personal_data"));
      QVERIFY(q_->remove_person(id));
      QVERIFY(!q_->has_permission(id, "view_personal_data"));
      teardown_db();
    }

    // ==================================================================
    // string_to_column_name
    // ==================================================================

    void test_string_to_column_name_spaces_to_underscores()
    {
      QString result = colman::string_to_column_name("Hello World");
      QVERIFY(!result.contains(' '));
    }

    void test_string_to_column_name_ascii_letters_preserved()
    {
      QString result = colman::string_to_column_name("abc");
      QCOMPARE(result, QString("abc"));
    }

    void test_string_to_column_name_empty_input()
    {
      // just verify it does not crash
      QString result = colman::string_to_column_name("");
      Q_UNUSED(result)
    }

    void test_predefined_books_type_german_translations()
    {
      // Replicate what add_predefined_books_type does:
      // add German translations first, then add the type.
      setup_db();
      colman::KeyVector const kv{
          {"authors", {colman::DataType::ShortString, {}}},
          {"edition", {colman::DataType::ShortString, {}}},
          {"publication date", {colman::DataType::Date, {}}},
          {"publisher", {colman::DataType::ShortString, {}}},
          {"pages", {colman::DataType::Integer, {}}},
          {"ddc", {colman::DataType::ShortString, {}}}};
      QVERIFY(q_->add_translation("books", "authors",
                                  QLocale::German, "Autoren"));
      QVERIFY(q_->add_translation("books", "edition",
                                  QLocale::German, "Ausgabe"));
      QVERIFY(q_->add_translation("books", "publication date",
                                  QLocale::German, "Erscheinungsdatum"));
      QVERIFY(q_->add_translation("books", "publisher",
                                  QLocale::German, "Verlag"));
      QVERIFY(q_->add_translation("books", "pages",
                                  QLocale::German, "Seiten"));
      QVERIFY(q_->add_translation("books", "ddc",
                                  QLocale::German, "DDC"));
      int const type_id = q_->add_new_type(colman::Type("book", kv), "books");
      QVERIFY(type_id > 0);
      QCOMPARE(q_->get_translation("books", "authors",
                                   QLocale::German),
               QString("Autoren"));
      QCOMPARE(q_->get_translation("books", "edition",
                                   QLocale::German),
               QString("Ausgabe"));
      QCOMPARE(q_->get_translation("books", "publication_date",
                                   QLocale::German),
               QString("Erscheinungsdatum"));
      QCOMPARE(q_->get_translation("books", "publisher",
                                   QLocale::German),
               QString("Verlag"));
      QCOMPARE(q_->get_translation("books", "pages",
                                   QLocale::German),
               QString("Seiten"));
      QCOMPARE(q_->get_translation("books", "ddc",
                                   QLocale::German),
               QString("DDC"));
      teardown_db();
    }

    // ==================================================================
    // Additional invariant tests (V2)
    // ==================================================================

    void test_change_type_removes_attribute()
    {
      // change_type with a shorter key list drops the removed attribute.
      setup_db();
      int id = q_->add_new_type(colman::make_book_type()); // isbn + author
      colman::Type updated = colman::make_book_type();
      colman::KeyVector single_key;
      single_key.push_back({"isbn", {colman::DataType::ShortString, {}}});
      updated.set_keys(single_key);
      QVERIFY(q_->change_type(id, updated, {}));
      auto const keys = q_->get_type(id).get_keys();
      QCOMPARE(static_cast<int>(keys.size()), 1);
      auto has_key = [&](QString const& name) {
        for(auto const& k : keys) {
          if(k.first == name) {
            return true;
          }
        }
        return false;
      };
      QVERIFY(has_key(QStringLiteral("isbn")));
      QVERIFY(!has_key(QStringLiteral("author")));
      teardown_db();
    }

    void test_two_item_barcodes_are_distinct()
    {
      // Different items must produce different barcodes (uniqueness invariant).
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item1;
      item1.set_type_id(type_id);
      item1.set_name("Book 1");
      colman::Item item2;
      item2.set_type_id(type_id);
      item2.set_name("Book 2");
      int id1 = q_->add_new_item(item1);
      int id2 = q_->add_new_item(item2);
      QVERIFY(id1 >= 0);
      QVERIFY(id2 >= 0);
      QString bc1 = q_->compute_item_barcode(type_id, id1);
      QString bc2 = q_->compute_item_barcode(type_id, id2);
      QVERIFY(!bc1.isEmpty());
      QVERIFY(!bc2.isEmpty());
      QVERIFY(bc1 != bc2);
      teardown_db();
    }

    void test_same_item_barcode_is_stable()
    {
      // The same (type_id, item_id) pair always yields the same barcode.
      setup_db();
      int type_id = q_->add_new_type(colman::make_book_type());
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Stability Test");
      int item_id = q_->add_new_item(item);
      QVERIFY(item_id >= 0);
      QString bc1 = q_->compute_item_barcode(type_id, item_id);
      QString bc2 = q_->compute_item_barcode(type_id, item_id);
      QCOMPARE(bc1, bc2);
      teardown_db();
    }
    // ==================================================================
    // Concurrency: version tracking and versioned updates
    // ==================================================================

    void test_initial_version_is_one_for_all_entities()
    {
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(10);
      QCOMPARE(q_->get_version(VE::Item, s.item_id), 1);
      QCOMPARE(q_->get_version(VE::Person, s.person_id), 1);
      colman::Acquisition acq;
      acq.set_item_id(s.item_id);
      acq.set_count(5);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      int acq_id = q_->add_new_acquisition(acq);
      // setup_scenario already adds one acquisition, so add a second one for
      // an explicit acquisition id we can check
      QCOMPARE(q_->get_version(VE::Acquisition, acq_id), 1);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(1);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      int liq_id = q_->add_new_liquidation(liq);
      QCOMPARE(q_->get_version(VE::Liquidation, liq_id), 1);
      int lending_id = lend_items(s, 3);
      QCOMPARE(q_->get_version(VE::Lending, lending_id), 1);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(1);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      int ret_id = q_->add_new_return(ret);
      QCOMPARE(q_->get_version(VE::Return, ret_id), 1);
      teardown_db();
    }

    void test_get_version_unknown_id_returns_minus_one()
    {
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      QCOMPARE(q_->get_version(VE::Item, 9999), -1);
      QCOMPARE(q_->get_version(VE::Person, 9999), -1);
      QCOMPARE(q_->get_version(VE::Lending, 9999), -1);
      QCOMPARE(q_->get_version(VE::Return, 9999), -1);
      QCOMPARE(q_->get_version(VE::Acquisition, 9999), -1);
      QCOMPARE(q_->get_version(VE::Liquidation, 9999), -1);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // change_item_versioned
    // ------------------------------------------------------------------

    void test_change_item_versioned_ok()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(0);
      colman::Item item;
      item.set_type_id(s.type_id);
      item.set_name("Updated Title");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 1), R::Ok);
      colman::Item got;
      QString tn;
      q_->get_item(s.item_id, got, tn);
      QCOMPARE(got.get_name(), QString("Updated Title"));
      QCOMPARE(q_->get_version(VE::Item, s.item_id), 2);
      teardown_db();
    }

    void test_change_item_versioned_wrong_version_returns_mismatch()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(0);
      colman::Item item;
      item.set_type_id(s.type_id);
      item.set_name("Should Not Stick");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 99), R::VersionMismatch);
      colman::Item got;
      QString tn;
      q_->get_item(s.item_id, got, tn);
      QCOMPARE(got.get_name(), QString("Test Book")); // unchanged
      teardown_db();
    }

    void test_change_item_versioned_unknown_id_returns_not_found()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      colman::Item item;
      QCOMPARE(q_->change_item_versioned(9999, item, 1), R::NotFound);
      teardown_db();
    }

    void test_change_item_versioned_sequential_updates()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(0);
      colman::Item item;
      item.set_type_id(s.type_id);
      item.set_name("First");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 1), R::Ok);
      // version is now 2; using stale version 1 must fail
      item.set_name("Stale");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 1), R::VersionMismatch);
      // using current version 2 succeeds
      item.set_name("Second");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 2), R::Ok);
      QCOMPARE(q_->get_version(VE::Item, s.item_id), 3);
      colman::Item got;
      QString tn;
      q_->get_item(s.item_id, got, tn);
      QCOMPARE(got.get_name(), QString("Second"));
      teardown_db();
    }

    // ------------------------------------------------------------------
    // change_person_versioned
    // ------------------------------------------------------------------

    void test_change_person_versioned_ok()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(0);
      colman::Person p = colman::make_test_person("Updated", "Name");
      QCOMPARE(q_->change_person_versioned(s.person_id, p, 1), R::Ok);
      colman::Person got;
      q_->get_person(s.person_id, got);
      QCOMPARE(got.get_lastname(), QString("Updated"));
      QCOMPARE(q_->get_version(VE::Person, s.person_id), 2);
      teardown_db();
    }

    void test_change_person_versioned_wrong_version_returns_mismatch()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(0);
      colman::Person p = colman::make_test_person("Should Not Stick", "Name");
      QCOMPARE(q_->change_person_versioned(s.person_id, p, 42), R::VersionMismatch);
      colman::Person got;
      q_->get_person(s.person_id, got);
      QCOMPARE(got.get_lastname(), QString("Doe")); // original from make_test_person
      teardown_db();
    }

    void test_change_person_versioned_unknown_id_returns_not_found()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      colman::Person p;
      QCOMPARE(q_->change_person_versioned(9999, p, 1), R::NotFound);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // change_acquisition_versioned
    // ------------------------------------------------------------------

    void test_change_acquisition_versioned_ok()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(5);
      // setup_scenario created one acquisition; find its id via stock_size
      // Instead, create a second controlled acquisition:
      colman::Acquisition acq;
      acq.set_item_id(s.item_id);
      acq.set_count(3);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      int acq_id = q_->add_new_acquisition(acq);
      unsigned int const before = q_->stock_size(s.item_id);
      acq.set_count(6);
      QCOMPARE(q_->change_acquisition_versioned(acq_id, acq, 1), R::Ok);
      QCOMPARE(q_->stock_size(s.item_id), before + 3); // 3 extra copies added
      QCOMPARE(q_->get_version(VE::Acquisition, acq_id), 2);
      teardown_db();
    }

    void test_change_acquisition_versioned_wrong_version_returns_mismatch()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(5);
      colman::Acquisition acq;
      acq.set_item_id(s.item_id);
      acq.set_count(2);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      int acq_id = q_->add_new_acquisition(acq);
      unsigned int const stock_before = q_->stock_size(s.item_id);
      acq.set_count(99);
      QCOMPARE(q_->change_acquisition_versioned(acq_id, acq, 99), R::VersionMismatch);
      QCOMPARE(q_->stock_size(s.item_id), stock_before); // stock unchanged
      teardown_db();
    }

    void test_change_acquisition_versioned_unknown_id_returns_not_found()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      colman::Acquisition acq;
      QCOMPARE(q_->change_acquisition_versioned(9999, acq, 1), R::NotFound);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // change_liquidation_versioned
    // ------------------------------------------------------------------

    void test_change_liquidation_versioned_ok()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(10);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(2);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      int liq_id = q_->add_new_liquidation(liq);
      unsigned int const stock_before = q_->stock_size(s.item_id);
      liq.set_count(3); // liquidate 1 extra copy
      QCOMPARE(q_->change_liquidation_versioned(liq_id, liq, 1), R::Ok);
      QCOMPARE(q_->stock_size(s.item_id), stock_before - 1);
      QCOMPARE(q_->get_version(VE::Liquidation, liq_id), 2);
      teardown_db();
    }

    void test_change_liquidation_versioned_wrong_version_returns_mismatch()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(10);
      colman::Liquidation liq;
      liq.set_item_id(s.item_id);
      liq.set_count(2);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      int liq_id = q_->add_new_liquidation(liq);
      unsigned int const stock_before = q_->stock_size(s.item_id);
      liq.set_count(99);
      QCOMPARE(q_->change_liquidation_versioned(liq_id, liq, 99), R::VersionMismatch);
      QCOMPARE(q_->stock_size(s.item_id), stock_before); // unchanged
      teardown_db();
    }

    void test_change_liquidation_versioned_unknown_id_returns_not_found()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      colman::Liquidation liq;
      QCOMPARE(q_->change_liquidation_versioned(9999, liq, 1), R::NotFound);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // change_lending_versioned
    // ------------------------------------------------------------------

    void test_change_lending_versioned_ok()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 3);
      colman::Lending l;
      QVERIFY(q_->get_lending(lending_id, l));
      l.set_remarks("versioned change");
      QCOMPARE(q_->change_lending_versioned(lending_id, l, 1), R::Ok);
      colman::Lending got;
      q_->get_lending(lending_id, got);
      QCOMPARE(got.get_remarks(), QString("versioned change"));
      QCOMPARE(q_->get_version(VE::Lending, lending_id), 2);
      teardown_db();
    }

    void test_change_lending_versioned_wrong_version_returns_mismatch()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 3);
      colman::Lending l;
      QVERIFY(q_->get_lending(lending_id, l));
      l.set_remarks("should not stick");
      QCOMPARE(q_->change_lending_versioned(lending_id, l, 42), R::VersionMismatch);
      colman::Lending got;
      q_->get_lending(lending_id, got);
      QVERIFY(got.get_remarks() != QString("should not stick"));
      teardown_db();
    }

    void test_change_lending_versioned_unknown_id_returns_not_found()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      colman::Lending l;
      QCOMPARE(q_->change_lending_versioned(9999, l, 1), R::NotFound);
      teardown_db();
    }

    void test_change_lending_versioned_count_adjusts_available()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 3);
      QCOMPARE(q_->available(s.item_id), 7u);
      colman::Lending l;
      QVERIFY(q_->get_lending(lending_id, l));
      l.set_count(5); // borrow 2 more
      QCOMPARE(q_->change_lending_versioned(lending_id, l, 1), R::Ok);
      QCOMPARE(q_->available(s.item_id), 5u);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // change_return_versioned
    // ------------------------------------------------------------------

    void test_change_return_versioned_ok()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(2);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      ret.set_remarks("original");
      int ret_id = q_->add_new_return(ret);
      ret.set_remarks("updated");
      QCOMPARE(q_->change_return_versioned(ret_id, ret, 1), R::Ok);
      colman::Return got;
      q_->get_return(ret_id, got);
      QCOMPARE(got.get_remarks(), QString("updated"));
      QCOMPARE(q_->get_version(VE::Return, ret_id), 2);
      teardown_db();
    }

    void test_change_return_versioned_wrong_version_returns_mismatch()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(2);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      ret.set_remarks("original");
      int ret_id = q_->add_new_return(ret);
      ret.set_remarks("should not stick");
      QCOMPARE(q_->change_return_versioned(ret_id, ret, 42), R::VersionMismatch);
      colman::Return got;
      q_->get_return(ret_id, got);
      QCOMPARE(got.get_remarks(), QString("original"));
      teardown_db();
    }

    void test_change_return_versioned_unknown_id_returns_not_found()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      colman::Return ret;
      QCOMPARE(q_->change_return_versioned(9999, ret, 1), R::NotFound);
      teardown_db();
    }

    void test_change_return_versioned_count_adjusts_available()
    {
      using R = colman::IQueries::UpdateResult;
      setup_db();
      auto s = setup_scenario(10);
      int lending_id = lend_items(s, 5);
      colman::Return ret;
      ret.set_lending_id(lending_id);
      ret.set_count(2);
      ret.set_date_time(QDateTime::currentDateTimeUtc());
      int ret_id = q_->add_new_return(ret);
      QCOMPARE(q_->available(s.item_id), 7u); // 10 - 5 + 2
      ret.set_count(4); // return 2 more
      QCOMPARE(q_->change_return_versioned(ret_id, ret, 1), R::Ok);
      QCOMPARE(q_->available(s.item_id), 9u); // 10 - 5 + 4
      teardown_db();
    }

    // ------------------------------------------------------------------
    // Cross-entity invariants
    // ------------------------------------------------------------------

    void test_versions_are_independent_per_entity()
    {
      // Updating one entity must not affect the version of another.
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(0);
      int v_person_before = q_->get_version(VE::Person, s.person_id);
      colman::Item item;
      item.set_type_id(s.type_id);
      item.set_name("Changed");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 1), R::Ok);
      QCOMPARE(q_->get_version(VE::Person, s.person_id), v_person_before);
      teardown_db();
    }

    void test_mismatch_does_not_increment_version()
    {
      using R = colman::IQueries::UpdateResult;
      using VE = colman::IQueries::VersionedEntity;
      setup_db();
      auto s = setup_scenario(0);
      colman::Item item;
      item.set_type_id(s.type_id);
      item.set_name("Rejected");
      QCOMPARE(q_->change_item_versioned(s.item_id, item, 99), R::VersionMismatch);
      QCOMPARE(q_->get_version(VE::Item, s.item_id), 1); // still 1
      teardown_db();
    }

  }; // class ColmanSqlTest

} // anonymous namespace

QTEST_GUILESS_MAIN(ColmanSqlTest)
#include "colman_db_util_test.moc"

/* EOF */
