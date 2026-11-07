// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for colman::IQueries / colman::SQLQueries.
/**
   Uses Qt Test framework with in-memory SQLite databases.
   Each test method creates a fresh database and SQLQueries instance via
   setup_db() and tears them down with teardown_db().
   Helper functions take IQueries& so they can be reused for alternative
   implementations.
   @file
   @author Christoph Pesch
 */

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
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
#ifndef SYSTEM_QSTRING
#  include <QString>
#  define SYSTEM_QSTRING
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef COLMAN_TABLE_MODELS_HPP
#  include "colman_table_models.hpp"
#  define COLMAN_TABLE_MODELS_HPP
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif

namespace
{
  // todo!!! move into helpers!
  colman::Type make_magazine_type()
  {
    colman::KeyVector keys;
    keys.push_back({"issn", {colman::DataType::ShortString, {}}});
    return colman::Type("Magazine", keys);
  }

  /// Returns a QDateTime at 2020-01-01 12:00:<sec> for ordered test events.
  QDateTime test_dt(int sec)
  {
    return QDateTime(QDate(2020, 1, 1), QTime(12, 0, sec));
  }

  /// Builds a HistoryCombinedModel::Row for use in delete tests.
  colman::HistoryCombinedModel::Row make_row(int id, QString const& action, int item_id,
                                             int count, QDateTime const& dt, int person_id = 0)
  {
    colman::HistoryCombinedModel::Row r;
    r.id = id;
    r.action = action;
    r.item_id = item_id;
    r.person_id = person_id;
    r.count = count;
    r.datetime = dt;
    return r;
  }

  /// IDs created by setup_scenario().
  struct QueriesScenarioIds
  {
    int type_id = 0;
    int item_id = 0;
    int person_id = 0;
  };

} // anonymous namespace

// ------------------------------------------------------------------
// Helpers taking IQueries& (reusable for alternative implementations)
// ------------------------------------------------------------------

/// Verifies that query_types() returns the expected count.
void check_query_types_count(colman::IQueries& q, int expected_count)
{
  auto const types = q.query_types(-1);
  QCOMPARE(static_cast<int>(types.size()), expected_count);
}

/// Verifies that query_types() filters to a single type by id.
void check_query_types_filter_by_id(colman::IQueries& q, int type_id)
{
  auto const types = q.query_types(type_id);
  QCOMPARE(static_cast<int>(types.size()), 1);
  QCOMPARE(types.front().get_id(), type_id);
}

/// Verifies that query_persons() returns all persons when unfiltered.
void check_query_persons_all(colman::IQueries& q, int expected_count)
{
  auto const persons = q.query_persons(-1, QString());
  QCOMPARE(static_cast<int>(persons.size()), expected_count);
}

/// Verifies that query_persons() filters to a single person by id.
void check_query_persons_by_id(colman::IQueries& q, int person_id)
{
  auto const persons = q.query_persons(person_id, QString());
  QCOMPARE(static_cast<int>(persons.size()), 1);
  QCOMPARE(persons.front().get_id(), person_id);
}

/// Verifies that query_persons() returns a match for a name prefix.
void check_query_persons_by_name(colman::IQueries& q, QString const& prefix,
                                 int expected_count)
{
  auto const persons = q.query_persons(-1, prefix);
  QCOMPARE(static_cast<int>(persons.size()), expected_count);
}

/// Verifies that query_start_page_persons() returns expected count.
void check_start_page_persons_count(colman::IQueries& q, int expected_count)
{
  auto const rows = q.query_start_page_persons(QString());
  QCOMPARE(static_cast<int>(rows.size()), expected_count);
}

/// Verifies that query_items() returns all items when unfiltered.
void check_query_items_all(colman::IQueries& q, int expected_count)
{
  auto const items = q.query_items(-1, QString());
  QCOMPARE(static_cast<int>(items.size()), expected_count);
}

/// Verifies that query_items() filters to a single item by id.
void check_query_items_by_id(colman::IQueries& q, int item_id)
{
  auto const items = q.query_items(item_id, QString());
  QCOMPARE(static_cast<int>(items.size()), 1);
  QCOMPARE(items.front().id, item_id);
}

/// Verifies that query_items() filters by name prefix.
void check_query_items_by_name(colman::IQueries& q, QString const& prefix,
                               int expected_count)
{
  auto const items = q.query_items(-1, prefix);
  QCOMPARE(static_cast<int>(items.size()), expected_count);
}

/// Verifies that query_acquired_items() shows items with count > 0.
void check_query_acquired_items(colman::IQueries& q, int expected_count)
{
  auto const items = q.query_acquired_items(-1, QString());
  QCOMPARE(static_cast<int>(items.size()), expected_count);
}

/// Verifies that query_available_items() shows items with available > 0.
void check_query_available_items(colman::IQueries& q, int expected_count)
{
  auto const items = q.query_available_items(-1, QString());
  QCOMPARE(static_cast<int>(items.size()), expected_count);
}

/// Verifies query_open_lendings() returns open lendings for person/item.
void check_query_open_lendings(colman::IQueries& q, int person_id, int item_id,
                               int expected_count)
{
  auto const rows = q.query_open_lendings(person_id, item_id);
  QCOMPARE(static_cast<int>(rows.size()), expected_count);
}

/// Qt Test class for colman::SQLQueries (implements colman::colman::IQueries).
class ColmanQueriesTest : public QObject
{
  Q_OBJECT

public:
  ColmanQueriesTest() = default;

private:
  std::unique_ptr<colman::IQueries> queries_; ///< SQL query facade (owns its db)
  QString conn_name_;
  int conn_counter_ = 0;

    // ------------------------------------------------------------------
    // DB lifecycle
    // ------------------------------------------------------------------

  void setup_db()
  {
    conn_name_ = QString("colman_qtest_%1").arg(conn_counter_++);
    queries_ = std::make_unique<colman::SQLQueries>(conn_name_);
    QVERIFY(queries_->create_tables());
    auto col = colman::make_test_collection();
    QVERIFY(queries_->create_collection(col));
    QVERIFY(queries_->add_administrator());
  }

  void teardown_db()
  {
    queries_.reset();
  }

    // ------------------------------------------------------------------
    // Scenario helpers
    // ------------------------------------------------------------------

    /// Creates type + item (with acquisition) + person.
  QueriesScenarioIds setup_scenario(int stock = 10)
  {
    QueriesScenarioIds s;
    s.type_id = queries_->add_new_type(colman::make_book_type());
    if(s.type_id < 0) {
      QTest::qFail("add_new_type failed", __FILE__, __LINE__);
      return s;
    }

    colman::Item item;
    item.set_type_id(s.type_id);
    item.set_name("Test Book");
    s.item_id = queries_->add_new_item(item);
    if(s.item_id < 0) {
      QTest::qFail("add_new_item failed", __FILE__, __LINE__);
      return s;
    }

    colman::Acquisition acq;
    acq.set_item_id(s.item_id);
    acq.set_count(static_cast<unsigned int>(stock));
    acq.set_date_time(QDateTime::currentDateTimeUtc());
    if(queries_->add_new_acquisition(acq) < 0) {
      QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
      return s;
    }

    s.person_id = queries_->add_new_person(colman::make_test_person());
    if(s.person_id < 0) {
      QTest::qFail("add_new_person failed", __FILE__, __LINE__);
    }
    return s;
  }

    /// Lends @p count copies of the item in the scenario.
  int lend_items(QueriesScenarioIds const& s, unsigned int count = 3)
  {
    colman::Lending l;
    l.set_item_id(s.item_id);
    l.set_person_id(s.person_id);
    l.set_count(count);
    l.set_date_time(QDateTime::currentDateTimeUtc());
    int id = queries_->add_new_lending(l);
    if(id < 0) {
      QTest::qFail("add_new_lending failed", __FILE__, __LINE__);
    }
    return id;
  }

    /// Returns @p count copies from an existing lending.
  int return_items(int lending_id, unsigned int count = 2)
  {
    colman::Return r;
    r.set_lending_id(lending_id);
    r.set_count(count);
    r.set_date_time(QDateTime::currentDateTimeUtc());
    int id = queries_->add_new_return(r);
    if(id < 0) {
      QTest::qFail("add_new_return failed", __FILE__, __LINE__);
    }
    return id;
  }

    // ------------------------------------------------------------------
    // Helpers for delete action tests (controlled datetimes)
    // ------------------------------------------------------------------

    /// Creates a type, item (no acquisition), and person for delete tests.
  QueriesScenarioIds setup_item_and_person()
  {
    QueriesScenarioIds s;
    s.type_id = queries_->add_new_type(colman::make_book_type());
    if(s.type_id < 0) {
      QTest::qFail("add_new_type failed", __FILE__, __LINE__);
      return s;
    }
    colman::Item item;
    item.set_type_id(s.type_id);
    item.set_name("Test Book");
    s.item_id = queries_->add_new_item(item);
    if(s.item_id < 0) {
      QTest::qFail("add_new_item failed", __FILE__, __LINE__);
      return s;
    }
    s.person_id = queries_->add_new_person(colman::make_test_person());
    if(s.person_id < 0) {
      QTest::qFail("add_new_person failed", __FILE__, __LINE__);
    }
    return s;
  }

  int add_acquisition_at(int item_id, int count, QDateTime const& dt)
  {
    colman::Acquisition a;
    a.set_item_id(item_id);
    a.set_count(static_cast<unsigned int>(count));
    a.set_date_time(dt);
    int id = queries_->add_new_acquisition(a);
    if(id < 0) {
      QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
    }
    return id;
  }

  int add_lending_at(
      int item_id, int person_id, int count, QDateTime const& dt)
  {
    colman::Lending l;
    l.set_item_id(item_id);
    l.set_person_id(person_id);
    l.set_count(static_cast<unsigned int>(count));
    l.set_date_time(dt);
    int id = queries_->add_new_lending(l);
    if(id < 0) {
      QTest::qFail("add_new_lending failed", __FILE__, __LINE__);
    }
    return id;
  }

  int add_return_at(int lending_id, int count, QDateTime const& dt)
  {
    colman::Return r;
    r.set_lending_id(lending_id);
    r.set_count(static_cast<unsigned int>(count));
    r.set_date_time(dt);
    int id = queries_->add_new_return(r);
    if(id < 0) {
      QTest::qFail("add_new_return failed", __FILE__, __LINE__);
    }
    return id;
  }

  int add_liquidation_at(int item_id, int count, QDateTime const& dt)
  {
    colman::Liquidation liq;
    liq.set_item_id(item_id);
    liq.set_count(static_cast<unsigned int>(count));
    liq.set_date_time(dt);
    int id = queries_->add_new_liquidation(liq);
    if(id < 0) {
      QTest::qFail("add_new_liquidation failed", __FILE__, __LINE__);
    }
    return id;
  }

private Q_SLOTS:

    // ==================================================================
    // query_types
    // ==================================================================

  void test_query_types_empty()
  {
    setup_db();
    check_query_types_count(*queries_, 0);
    teardown_db();
  }

  void test_query_types_one()
  {
    setup_db();
    int id = queries_->add_new_type(colman::make_book_type());
    QVERIFY(id >= 0);
    check_query_types_count(*queries_, 1);
    teardown_db();
  }

  void test_query_types_two()
  {
    setup_db();
    QVERIFY(queries_->add_new_type(colman::make_book_type()) >= 0);
    QVERIFY(queries_->add_new_type(make_magazine_type()) >= 0);
    check_query_types_count(*queries_, 2);
    teardown_db();
  }

  void test_query_types_filter_by_id()
  {
    setup_db();
    int id1 = queries_->add_new_type(colman::make_book_type());
    QVERIFY(queries_->add_new_type(make_magazine_type()) >= 0);
    check_query_types_filter_by_id(*queries_, id1);
    teardown_db();
  }

  void test_query_types_unknown_id_returns_empty()
  {
    setup_db();
    QVERIFY(queries_->add_new_type(colman::make_book_type()) >= 0);
    auto const types = queries_->query_types(9999);
    QVERIFY(types.empty());
    teardown_db();
  }

  void test_query_types_names_match()
  {
    setup_db();
    queries_->add_new_type(colman::make_book_type());
    auto const types = queries_->query_types(-1);
    QCOMPARE(static_cast<int>(types.size()), 1);
    QCOMPARE(types.front().get_name(), QString("Book"));
    teardown_db();
  }

    // ==================================================================
    // is_locale_supported
    // ==================================================================

  void test_is_locale_supported_false_on_empty_db()
  {
    setup_db();
    QVERIFY(!queries_->is_locale_supported());
    teardown_db();
  }

    // ==================================================================
    // query_persons
    // ==================================================================

  void test_query_persons_empty()
  {
    setup_db();
    check_query_persons_all(*queries_, 0);
    teardown_db();
  }

  void test_query_persons_one()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person()) >= 0);
    check_query_persons_all(*queries_, 1);
    teardown_db();
  }

  void test_query_persons_by_id()
  {
    setup_db();
    int id = queries_->add_new_person(colman::make_test_person());
    QVERIFY(queries_->add_new_person(colman::make_test_person("Smith", "Anna")) >= 0);
    check_query_persons_by_id(*queries_, id);
    teardown_db();
  }

  void test_query_persons_by_name_prefix()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person("Murphy", "Max")) >= 0);
    QVERIFY(queries_->add_new_person(colman::make_test_person("Murray", "Anna")) >= 0);
    QVERIFY(queries_->add_new_person(colman::make_test_person("Smith", "Karl")) >= 0);
    check_query_persons_by_name(*queries_, "Mu", 2);
    teardown_db();
  }

  void test_query_persons_by_name_no_match()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person()) >= 0);
    check_query_persons_by_name(*queries_, "XYZ", 0);
    teardown_db();
  }

  void test_query_persons_sorted_by_lastname()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person("Wilson", "Anna")) >= 0);
    QVERIFY(queries_->add_new_person(colman::make_test_person("Abel", "Bob")) >= 0);
    auto const persons = queries_->query_persons(-1, QString());
    QCOMPARE(static_cast<int>(persons.size()), 2);
    QCOMPARE(persons.front().get_lastname(), QString("Abel"));
    teardown_db();
  }

    // ==================================================================
    // get_recently_created_person_ids
    // ==================================================================

  void test_get_recently_created_person_ids_empty()
  {
    setup_db();
    auto const ids = queries_->get_recently_created_person_ids(10);
    QVERIFY(ids.empty());
    teardown_db();
  }

  void test_get_recently_created_person_ids_order_and_limit()
  {
    setup_db();
    // Created in this order; ids are assigned from a monotonic counter.
    int const id1 =
        queries_->add_new_person(colman::make_test_person("Abel", "Anna"));
    int const id2 =
        queries_->add_new_person(colman::make_test_person("Brown", "Bob"));
    int const id3 =
        queries_->add_new_person(colman::make_test_person("Clark", "Cleo"));
    QVERIFY(id1 >= 0 && id2 >= 0 && id3 >= 0);

    // Newest created first (highest id first).
    auto const all = queries_->get_recently_created_person_ids(10);
    QCOMPARE(static_cast<int>(all.size()), 3);
    QCOMPARE(all[0], id3);
    QCOMPARE(all[1], id2);
    QCOMPARE(all[2], id1);

    // Limited to the two most recent.
    auto const last_two = queries_->get_recently_created_person_ids(2);
    QCOMPARE(static_cast<int>(last_two.size()), 2);
    QCOMPARE(last_two[0], id3);
    QCOMPARE(last_two[1], id2);

    // A count of zero yields nothing.
    QVERIFY(queries_->get_recently_created_person_ids(0).empty());
    teardown_db();
  }

    // ==================================================================
    // query_start_page_persons
    // ==================================================================

  void test_query_start_page_persons_empty()
  {
    setup_db();
    check_start_page_persons_count(*queries_, 0);
    teardown_db();
  }

  void test_query_start_page_persons_one()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person()) >= 0);
    check_start_page_persons_count(*queries_, 1);
    teardown_db();
  }

  void test_query_start_page_persons_no_password()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person()) >= 0);
    auto const rows = queries_->query_start_page_persons(QString());
    QCOMPARE(static_cast<int>(rows.size()), 1);
    QVERIFY(!rows.front().has_password);
    teardown_db();
  }

  void test_query_start_page_persons_search_filter()
  {
    setup_db();
    QVERIFY(queries_->add_new_person(colman::make_test_person("Murphy", "Max")) >= 0);
    QVERIFY(queries_->add_new_person(colman::make_test_person("Smith", "Karl")) >= 0);
    auto const rows = queries_->query_start_page_persons("Murp");
    QCOMPARE(static_cast<int>(rows.size()), 1);
    QCOMPARE(rows.front().lastname, QString("Murphy"));
    teardown_db();
  }

  void test_query_start_page_persons_fields()
  {
    setup_db();
    auto p = colman::make_test_person("Huber", "Eva");
    p.set_title("Prof.");
    QVERIFY(queries_->add_new_person(p) >= 0);
    auto const rows = queries_->query_start_page_persons(QString());
    QCOMPARE(static_cast<int>(rows.size()), 1);
    QCOMPARE(rows.front().lastname, QString("Huber"));
    QCOMPARE(rows.front().firstname, QString("Eva"));
    QCOMPARE(rows.front().title, QString("Prof."));
    teardown_db();
  }

    // ==================================================================
    // query_items
    // ==================================================================

  void test_query_items_empty()
  {
    setup_db();
    queries_->add_new_type(colman::make_book_type());
    check_query_items_all(*queries_, 0);
    teardown_db();
  }

  void test_query_items_after_add()
  {
    setup_db();
    auto const s = setup_scenario();
    check_query_items_all(*queries_, 1);
    QVERIFY(s.item_id >= 0);
    teardown_db();
  }

  void test_query_items_by_id()
  {
    setup_db();
    auto const s = setup_scenario();
    check_query_items_by_id(*queries_, s.item_id);
    teardown_db();
  }

  void test_query_items_by_name_prefix()
  {
    setup_db();
    auto const s = setup_scenario();
    QVERIFY(s.item_id >= 0);
    check_query_items_by_name(*queries_, "Test", 1);
    check_query_items_by_name(*queries_, "XYZ", 0);
    teardown_db();
  }

  void test_query_items_fields()
  {
    setup_db();
    auto const s = setup_scenario(5);
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(static_cast<int>(items.size()), 1);
    QCOMPARE(items.front().name, QString("Test Book"));
    QCOMPARE(items.front().count, 5);
    QCOMPARE(items.front().available, 5);
    QCOMPARE(items.front().type_name, QString("Book"));
    teardown_db();
  }

  /// Editing a school item's grade level must persist across a reload.
  void test_change_item_academic_year_roundtrip()
  {
    setup_db();
    int const type_id = queries_->add_new_type(colman::make_book_type());
    colman::Item item;
    item.set_type_id(type_id);
    item.set_name("School Book");
    item.set_is_school_item(true);
    item.set_academic_year(colman::IntPairVec{{5, 7}});
    int const id = queries_->add_new_item(item);
    QVERIFY(id >= 0);

    // Reload, change the grade level, save, reload again.
    colman::Item loaded;
    QString tn;
    QVERIFY(queries_->get_item(id, loaded, tn));
    QCOMPARE(colman::academic_years_to_string(loaded.get_academic_year()),
             QString("5-7"));

    loaded.set_academic_year(colman::IntPairVec{{8, 9}});
    QVERIFY(queries_->change_item(id, loaded));

    colman::Item reloaded;
    QVERIFY(queries_->get_item(id, reloaded, tn));
    QCOMPARE(colman::academic_years_to_string(reloaded.get_academic_year()),
             QString("8-9"));
    teardown_db();
  }

  void test_query_items_two_items()
  {
    setup_db();
    int type_id = queries_->add_new_type(colman::make_book_type());
    colman::Item item1;
    item1.set_type_id(type_id);
    item1.set_name("Aachen");
    colman::Item item2;
    item2.set_type_id(type_id);
    item2.set_name("Zwickau");
    QVERIFY(queries_->add_new_item(item1) >= 0);
    QVERIFY(queries_->add_new_item(item2) >= 0);
    check_query_items_all(*queries_, 2);
    teardown_db();
  }

    // ==================================================================
    // query_items_for_search — grade-level (academic year) filter
    // ==================================================================

  void test_query_items_for_search_grade_level()
  {
    setup_db();
    int const type_id = queries_->add_new_type(colman::make_book_type());

    auto add_school_item = [&](char const* name, char const* years) {
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name(QString::fromUtf8(name));
      if(years != nullptr) {
        item.set_is_school_item(true);
        item.set_academic_year(colman::string_to_academic_years(years));
      }
      QVERIFY(queries_->add_new_item(item) >= 0);
    };

    add_school_item("Grades 5 to 7", "5-7");
    add_school_item("Grade 10", "10");
    add_school_item("Generic", nullptr); // no grade level

    auto search = [&](int gmin, int gmax) {
      return queries_->query_items_for_search(
          "", "", "", "", "", -1, {}, gmin, gmax, 0, 0);
    };

    // No filter: all three items (including the grade-less "Generic").
    QCOMPARE(static_cast<int>(search(0, 0).size()), 3);
    // Once a span is active, the grade-less "Generic" is excluded.
    // Span 5-6 overlaps only "5-7".
    QCOMPARE(static_cast<int>(search(5, 6).size()), 1);
    // Span 10-13 overlaps only "Grade 10".
    QCOMPARE(static_cast<int>(search(10, 13).size()), 1);
    // Span 8-9 overlaps neither graded item.
    QCOMPARE(static_cast<int>(search(8, 9).size()), 0);
    // Lower bound only (>= 10): "Grade 10".
    QCOMPARE(static_cast<int>(search(10, 0).size()), 1);
    // Upper bound only (<= 7): "5-7".
    QCOMPARE(static_cast<int>(search(0, 7).size()), 1);

    teardown_db();
  }

  void test_academic_years_valid()
  {
    // Empty (no grade level) and well-ordered ranges are valid.
    QVERIFY(colman::academic_years_valid({}));
    QVERIFY(colman::academic_years_valid({{5, 7}}));
    QVERIFY(colman::academic_years_valid({{8, 8}}));
    QVERIFY(colman::academic_years_valid({{1, 4}, {6, 9}}));
    // A reversed range (first > second) is invalid, even among valid ones.
    QVERIFY(!colman::academic_years_valid({{9, 5}}));
    QVERIFY(!colman::academic_years_valid({{1, 4}, {9, 5}}));
  }

    // ==================================================================
    // get_items_ordered_by_name
    // ==================================================================

  void test_get_items_ordered_by_name_empty()
  {
    setup_db();
    auto const items = queries_->get_items_ordered_by_name();
    QVERIFY(items.empty());
    teardown_db();
  }

  void test_get_items_ordered_by_name_order()
  {
    setup_db();
    int type_id = queries_->add_new_type(colman::make_book_type());
    colman::Item item1;
    item1.set_type_id(type_id);
    item1.set_name("Zebra");
    colman::Item item2;
    item2.set_type_id(type_id);
    item2.set_name("Alpha");
    colman::Item item3;
    item3.set_type_id(type_id);
    item3.set_name("Beta");
    int id1 = queries_->add_new_item(item1);
    int id2 = queries_->add_new_item(item2);
    int id3 = queries_->add_new_item(item3);
    auto const items = queries_->get_items_ordered_by_name();
    QCOMPARE(static_cast<int>(items.size()), 3);
    QCOMPARE(items[0].first, id2);
    QCOMPARE(items[0].second, QString("Alpha"));
    QCOMPARE(items[1].first, id3);
    QCOMPARE(items[1].second, QString("Beta"));
    QCOMPARE(items[2].first, id1);
    QCOMPARE(items[2].second, QString("Zebra"));
    teardown_db();
  }

    // ==================================================================
    // get_recently_acquired_item_ids
    // ==================================================================

  void test_get_recently_acquired_item_ids_empty()
  {
    setup_db();
    auto const ids = queries_->get_recently_acquired_item_ids(10);
    QVERIFY(ids.empty());
    teardown_db();
  }

  void test_get_recently_acquired_item_ids_order_and_limit()
  {
    setup_db();
    int const type_id = queries_->add_new_type(colman::make_book_type());

    // Three items, each with one acquisition at increasing times.
    QDateTime const base =
        QDateTime(QDate(2026, 1, 1), QTime(8, 0));
    int ids[3] = {-1, -1, -1};
    for(int i = 0; i < 3; ++i) {
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name(QString("Item %1").arg(i));
      ids[i] = queries_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(ids[i]);
      acq.set_count(1);
      acq.set_date_time(base.addDays(i));
      QVERIFY(queries_->add_new_acquisition(acq) >= 0);
    }

    // Newest acquisition first.
    auto const all = queries_->get_recently_acquired_item_ids(10);
    QCOMPARE(static_cast<int>(all.size()), 3);
    QCOMPARE(all[0], ids[2]);
    QCOMPARE(all[1], ids[1]);
    QCOMPARE(all[2], ids[0]);

    // Limited to the two most recent.
    auto const last_two = queries_->get_recently_acquired_item_ids(2);
    QCOMPARE(static_cast<int>(last_two.size()), 2);
    QCOMPARE(last_two[0], ids[2]);
    QCOMPARE(last_two[1], ids[1]);

    // A count of zero yields nothing.
    QVERIFY(queries_->get_recently_acquired_item_ids(0).empty());
    teardown_db();
  }

    // ==================================================================
    // get_items_acquired_between
    // ==================================================================

  void test_get_items_acquired_between_empty()
  {
    setup_db();
    auto const ids = queries_->get_items_acquired_between(
        QDateTime(QDate(2026, 1, 1), QTime(0, 0)),
        QDateTime(QDate(2026, 12, 31), QTime(23, 59, 59)));
    QVERIFY(ids.empty());
    teardown_db();
  }

  void test_get_items_acquired_between_range_and_bounds()
  {
    setup_db();
    int const type_id = queries_->add_new_type(colman::make_book_type());

    // Three items, each acquired on a distinct day.
    QDate const days[3] = {QDate(2026, 1, 10), QDate(2026, 1, 20),
                           QDate(2026, 2, 5)};
    int ids[3] = {-1, -1, -1};
    for(int i = 0; i < 3; ++i) {
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name(QString("Item %1").arg(i));
      ids[i] = queries_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(ids[i]);
      acq.set_count(1);
      acq.set_date_time(QDateTime(days[i], QTime(12, 0)));
      QVERIFY(queries_->add_new_acquisition(acq) >= 0);
    }

    // Window covering only the first two items, ordered by id.
    auto const mid = queries_->get_items_acquired_between(
        QDateTime(QDate(2026, 1, 1), QTime(0, 0)),
        QDateTime(QDate(2026, 1, 31), QTime(23, 59, 59)));
    QCOMPARE(static_cast<int>(mid.size()), 2);
    QCOMPARE(mid[0], ids[0]);
    QCOMPARE(mid[1], ids[1]);

    // Bounds are inclusive: a window pinned to the acquisition day matches.
    auto const exact = queries_->get_items_acquired_between(
        QDateTime(days[2], QTime(0, 0)),
        QDateTime(days[2], QTime(23, 59, 59)));
    QCOMPARE(static_cast<int>(exact.size()), 1);
    QCOMPARE(exact[0], ids[2]);

    // An empty window yields nothing.
    QVERIFY(queries_
                ->get_items_acquired_between(
                    QDateTime(QDate(2025, 1, 1), QTime(0, 0)),
                    QDateTime(QDate(2025, 12, 31), QTime(23, 59, 59)))
                .empty());
    teardown_db();
  }

    // ==================================================================
    // get_items_with_type_and_thumbnails
    // ==================================================================

  void test_get_items_with_type_and_thumbnails_empty()
  {
    setup_db();
    auto const items = queries_->get_items_with_type_and_thumbnails();
    QVERIFY(items.empty());
    teardown_db();
  }

  void test_get_items_with_type_and_thumbnails_content()
  {
    setup_db();
    auto const s = setup_scenario(5);
    auto const items = queries_->get_items_with_type_and_thumbnails();
    QCOMPARE(static_cast<int>(items.size()), 1);
    QCOMPARE(items.front().item_id_, s.item_id);
    QCOMPARE(items.front().item_name_, QString("Test Book"));
    QCOMPARE(items.front().type_name_, QString("Book"));
    QCOMPARE(items.front().thumb_id_, -1);
    teardown_db();
  }

    // ==================================================================
    // get_user_permissions
    // ==================================================================

  void test_get_user_permissions_admin()
  {
    setup_db();
    auto const perms = queries_->get_user_permissions(0);
    QVERIFY(!perms.empty());
    for(bool p : perms) {
      QVERIFY(p);
    }
    teardown_db();
  }

  void test_get_user_permissions_regular_user()
  {
    setup_db();
    int id = queries_->add_new_person(colman::make_test_person());
    QVERIFY(id > 0);
    auto const perms = queries_->get_user_permissions(id);
    QVERIFY(!perms.empty());
    int true_count = 0;
    for(bool p : perms) {
      if(p) {
        ++true_count;
      }
    }
    QCOMPARE(true_count, 2); // view_personal_data + reserve
    teardown_db();
  }

    // ==================================================================
    // query_acquired_items
    // ==================================================================

  void test_query_acquired_items_none_before_acquisition()
  {
    setup_db();
    int type_id = queries_->add_new_type(colman::make_book_type());
    colman::Item item;
    item.set_type_id(type_id);
    item.set_name("Test Book");
    QVERIFY(queries_->add_new_item(item) >= 0);
      // No acquisition yet — count stays 0.
    check_query_acquired_items(*queries_, 0);
    teardown_db();
  }

  void test_query_acquired_items_after_acquisition()
  {
    setup_db();
    setup_scenario(3);
    check_query_acquired_items(*queries_, 1);
    teardown_db();
  }

  void test_query_acquired_items_count_field()
  {
    setup_db();
    auto const s = setup_scenario(7);
    auto const items = queries_->query_acquired_items(s.item_id, QString());
    QCOMPARE(static_cast<int>(items.size()), 1);
    QCOMPARE(items.front().count, 7);
    teardown_db();
  }

  /**
     A legacy (format 1) colman item barcode with 13 digits can carry a
     valid EAN-13 check digit by chance (e.g. 5486111410276).  The search
     must still resolve it as a colman barcode and not as an ISBN-13.
  */
  void test_query_items_barcode_looking_like_isbn13()
  {
    setup_db();
    auto col = queries_->read_collection();
    col.set_barcode_prefix("54861");
    QVERIFY(queries_->change_collection(col));
    auto const s = setup_scenario(2);
      // Find a 4-digit item id whose barcode passes the ISBN-13 check.
    auto item_id = 1000;
    while(!colman::is_isbn13(
        colman::legacy_item_barcode("54861", s.type_id, item_id))) {
      ++item_id;
    }
    QSqlQuery q(QSqlDatabase::database(conn_name_));
    QVERIFY(q.exec("UPDATE items SET id = " + QString::number(item_id) +
                   " WHERE id = " + QString::number(s.item_id)));
    auto const barcode =
        colman::legacy_item_barcode("54861", s.type_id, item_id);
    auto const acquired = queries_->query_acquired_items(-1, barcode);
    QCOMPARE(static_cast<int>(acquired.size()), 1);
    QCOMPARE(acquired.front().id, item_id);
    auto const available = queries_->query_available_items(-1, barcode);
    QCOMPARE(static_cast<int>(available.size()), 1);
    auto const all = queries_->query_items(-1, barcode);
    QCOMPARE(static_cast<int>(all.size()), 1);
    auto const with_type =
        queries_->query_items_with_type(s.type_id, barcode);
    QCOMPARE(static_cast<int>(with_type.size()), 1);
      // The current barcode of the same item is found as well.
    auto const current = queries_->query_acquired_items(
        -1, colman::compute_item_barcode(col, s.type_id, item_id));
    QCOMPARE(static_cast<int>(current.size()), 1);
    QCOMPARE(current.front().id, item_id);
    teardown_db();
  }

    // ==================================================================
    // query_available_items
    // ==================================================================

  void test_query_available_items_after_acquisition()
  {
    setup_db();
    setup_scenario(4);
    check_query_available_items(*queries_, 1);
    teardown_db();
  }

  void test_query_available_items_all_lent_out()
  {
    setup_db();
    auto const s = setup_scenario(3);
    lend_items(s, 3);
    check_query_available_items(*queries_, 0);
    teardown_db();
  }

  void test_query_available_items_partially_lent()
  {
    setup_db();
    auto const s = setup_scenario(5);
    lend_items(s, 3);
    auto const items = queries_->query_available_items(-1, QString());
    QCOMPARE(static_cast<int>(items.size()), 1);
    QCOMPARE(items.front().available, 2);
    teardown_db();
  }

    // ==================================================================
    // query_items_with_type
    // ==================================================================

  void test_query_items_with_type_matches()
  {
    setup_db();
    auto const s = setup_scenario();
    auto const items = queries_->query_items_with_type(s.type_id, QString());
    QCOMPARE(static_cast<int>(items.size()), 1);
    QCOMPARE(items.front().id, s.item_id);
    teardown_db();
  }

  void test_query_items_with_type_wrong_type()
  {
    setup_db();
    auto const s = setup_scenario();
    int mag_type = queries_->add_new_type(make_magazine_type());
    QVERIFY(mag_type >= 0);
    auto const items = queries_->query_items_with_type(mag_type, QString());
    QVERIFY(items.empty());
    QVERIFY(s.item_id >= 0);
    teardown_db();
  }

  void test_query_items_with_type_name_filter()
  {
    setup_db();
    auto const s = setup_scenario();
    auto const items =
        queries_->query_items_with_type(s.type_id, "Test");
    QCOMPARE(static_cast<int>(items.size()), 1);
    auto const no_match =
        queries_->query_items_with_type(s.type_id, "XYZ");
    QVERIFY(no_match.empty());
    teardown_db();
  }

    // ==================================================================
    // query_open_lendings
    // ==================================================================

  void test_query_open_lendings_empty_before_lending()
  {
    setup_db();
    auto const s = setup_scenario();
    check_query_open_lendings(*queries_, s.person_id, s.item_id, 0);
    teardown_db();
  }

  void test_query_open_lendings_one()
  {
    setup_db();
    auto const s = setup_scenario(10);
    lend_items(s, 3);
    check_query_open_lendings(*queries_, s.person_id, s.item_id, 1);
    teardown_db();
  }

  void test_query_open_lendings_closed_after_full_return()
  {
    setup_db();
    auto const s = setup_scenario(10);
    int lid = lend_items(s, 3);
    return_items(lid, 3u);
    check_query_open_lendings(*queries_, s.person_id, s.item_id, 0);
    teardown_db();
  }

  void test_query_open_lendings_still_open_after_partial_return()
  {
    setup_db();
    auto const s = setup_scenario(10);
    int lid = lend_items(s, 3);
    return_items(lid);
    check_query_open_lendings(*queries_, s.person_id, s.item_id, 1);
    teardown_db();
  }

    // ==================================================================
    // query_lent_items
    // ==================================================================

  void test_query_lent_items_empty_before_lend()
  {
    setup_db();
    auto const s = setup_scenario();
    auto const items = queries_->query_lent_items(
        s.person_id, -1, QString(), QString());
    QVERIFY(items.empty());
    teardown_db();
  }

  void test_query_lent_items_present_after_lend()
  {
    setup_db();
    auto const s = setup_scenario(10);
    lend_items(s, 3);
    auto const items = queries_->query_lent_items(
        s.person_id, -1, QString(), QString());
    QCOMPARE(static_cast<int>(items.size()), 1);
    QCOMPARE(items.front().id, s.item_id);
    teardown_db();
  }

  void test_query_lent_items_gone_after_full_return()
  {
    setup_db();
    auto const s = setup_scenario(10);
    int lid = lend_items(s, 3);
    return_items(lid, 3u);
    auto const items = queries_->query_lent_items(
        s.person_id, -1, QString(), QString());
    QVERIFY(items.empty());
    teardown_db();
  }

    // ==================================================================
    // query_person_field_completions
    // ==================================================================

  void test_query_person_field_completions_empty_db()
  {
    setup_db();
    auto const vals = queries_->query_person_field_completions("city");
      // Empty db — no persons yet.
    QVERIFY(vals.isEmpty());
    teardown_db();
  }

  void test_query_person_field_completions_returns_distinct()
  {
    setup_db();
    auto p1 = colman::make_test_person("Doe", "John");
    p1.set_city("Berlin");
    auto p2 = colman::make_test_person("Smith", "Anna");
    p2.set_city("Berlin");
    auto p3 = colman::make_test_person("Baker", "Karl");
    p3.set_city("Munich");
    QVERIFY(queries_->add_new_person(p1) >= 0);
    QVERIFY(queries_->add_new_person(p2) >= 0);
    QVERIFY(queries_->add_new_person(p3) >= 0);
    auto const vals = queries_->query_person_field_completions("city");
    QCOMPARE(vals.size(), 2);
    QVERIFY(vals.contains("Berlin"));
    QVERIFY(vals.contains("Munich"));
    teardown_db();
  }

    // ==================================================================
    // find_conflicting_actions — acquisition
    // ==================================================================

  void test_find_conflicts_acq_no_later_actions()
  {
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    auto const row =
        make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

  void test_find_conflicts_acq_lending_surplus()
  {
      // Deleting first acquisition leaves 3 copies — enough for 2 lent.
    setup_db();
    auto const s = setup_item_and_person();
    int acq1 = add_acquisition_at(s.item_id, 5, test_dt(10));
    add_acquisition_at(s.item_id, 3, test_dt(20));
    add_lending_at(s.item_id, s.person_id, 2, test_dt(30));
    auto const row =
        make_row(acq1, "acquisition", s.item_id, 5, test_dt(10));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

  void test_find_conflicts_acq_lending_needed()
  {
      // Only acquisition covers the lending — deleting it is a conflict.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 3, test_dt(10));
    add_lending_at(s.item_id, s.person_id, 3, test_dt(20));
    auto const row =
        make_row(acq_id, "acquisition", s.item_id, 3, test_dt(10));
    auto const conflicts = queries_->find_conflicting_actions(row);
    QCOMPARE(static_cast<int>(conflicts.size()), 1);
    QCOMPARE(conflicts.front().action, QString("lending"));
    teardown_db();
  }

  void test_find_conflicts_acq_liquidation_needed()
  {
      // Only acquisition covers the liquidation — deleting it is a conflict.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 3, test_dt(10));
    add_liquidation_at(s.item_id, 3, test_dt(20));
    auto const row =
        make_row(acq_id, "acquisition", s.item_id, 3, test_dt(10));
    auto const conflicts = queries_->find_conflicting_actions(row);
    QCOMPARE(static_cast<int>(conflicts.size()), 1);
    QCOMPARE(conflicts.front().action, QString("liquidation"));
    teardown_db();
  }

  void test_find_conflicts_acq_liquidation_surplus()
  {
      // Deleting first acquisition leaves 3 copies — enough for liquidation 2.
    setup_db();
    auto const s = setup_item_and_person();
    int acq1 = add_acquisition_at(s.item_id, 5, test_dt(10));
    add_acquisition_at(s.item_id, 3, test_dt(20));
    add_liquidation_at(s.item_id, 2, test_dt(30));
    auto const row =
        make_row(acq1, "acquisition", s.item_id, 5, test_dt(10));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

    // ==================================================================
    // find_conflicting_actions — lending
    // ==================================================================

  void test_find_conflicts_lending_no_returns()
  {
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    auto const row =
        make_row(lend_id, "lending", s.item_id, 3, test_dt(10));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

  void test_find_conflicts_lending_own_return()
  {
      // Lending's own return is a conflict.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    add_return_at(lend_id, 2, test_dt(20));
    auto const row =
        make_row(lend_id, "lending", s.item_id, 3, test_dt(10));
    auto const conflicts = queries_->find_conflicting_actions(row);
    QCOMPARE(static_cast<int>(conflicts.size()), 1);
    QCOMPARE(conflicts.front().action, QString("return"));
    teardown_db();
  }

  void test_find_conflicts_lending_other_return_not_conflict()
  {
      // Return for a different lending is not a conflict for lending1.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend1 = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int lend2 = add_lending_at(s.item_id, s.person_id, 4, test_dt(20));
    add_return_at(lend2, 2, test_dt(30));
    auto const row =
        make_row(lend1, "lending", s.item_id, 3, test_dt(10));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

    // ==================================================================
    // find_conflicting_actions — return
    // ==================================================================

  void test_find_conflicts_return_no_later_lendings()
  {
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 5, test_dt(10));
    int ret_id = add_return_at(lend_id, 3, test_dt(20));
    auto const row =
        make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

  void test_find_conflicts_return_lending_needed()
  {
      // Without the return the second lending cannot be satisfied.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 3, test_dt(5));
    int lend1 = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend1, 3, test_dt(20));
    add_lending_at(s.item_id, s.person_id, 3, test_dt(30));
    auto const row =
        make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    auto const conflicts = queries_->find_conflicting_actions(row);
    QCOMPARE(static_cast<int>(conflicts.size()), 1);
    QCOMPARE(conflicts.front().action, QString("lending"));
    teardown_db();
  }

  void test_find_conflicts_return_lending_not_needed()
  {
      // Surplus stock means second lending is fine without the return.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 6, test_dt(5));
    int lend1 = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend1, 3, test_dt(20));
    add_lending_at(s.item_id, s.person_id, 3, test_dt(30));
    auto const row =
        make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

  void test_find_conflicts_return_liquidation_needed()
  {
      // Without the return there is no stock left to liquidate.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 3, test_dt(5));
    int lend1 = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend1, 3, test_dt(20));
    add_liquidation_at(s.item_id, 3, test_dt(30));
    auto const row =
        make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    auto const conflicts = queries_->find_conflicting_actions(row);
    QCOMPARE(static_cast<int>(conflicts.size()), 1);
    QCOMPARE(conflicts.front().action, QString("liquidation"));
    teardown_db();
  }

  void test_find_conflicts_return_liquidation_not_needed()
  {
      // Surplus stock means liquidation is fine without the return.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 6, test_dt(5));
    int lend1 = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend1, 3, test_dt(20));
    add_liquidation_at(s.item_id, 3, test_dt(30));
    auto const row =
        make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

    // ==================================================================
    // find_conflicting_actions — liquidation (always empty)
    // ==================================================================

  void test_find_conflicts_liquidation_always_empty()
  {
      // Deleting a liquidation only increases available stock — no conflicts.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int liq_id = add_liquidation_at(s.item_id, 3, test_dt(10));
    auto const row =
        make_row(liq_id, "liquidation", s.item_id, 3, test_dt(10));
    QVERIFY(queries_->find_conflicting_actions(row).empty());
    teardown_db();
  }

    // ==================================================================
    // batch_delete_history_actions
    // ==================================================================

  void test_batch_delete_empty()
  {
    setup_db();
    setup_item_and_person();
    QVERIFY(queries_->batch_delete_history_actions({}));
    teardown_db();
  }

  void test_batch_delete_acquisition()
  {
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    QVERIFY(queries_->query_items(s.item_id, QString()).empty());
    teardown_db();
  }

  void test_batch_delete_lending()
  {
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(lend_id, "lending", s.item_id, 3, test_dt(10))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 10);
    teardown_db();
  }

  void test_batch_delete_return()
  {
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend_id, 2, test_dt(20));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(ret_id, "return", s.item_id, 2, test_dt(20))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 7);
    teardown_db();
  }

  void test_batch_delete_liquidation()
  {
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int liq_id = add_liquidation_at(s.item_id, 2, test_dt(10));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(liq_id, "liquidation", s.item_id, 2, test_dt(10))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 5);
    QCOMPARE(items.front().available, 5);
    teardown_db();
  }

  void test_batch_delete_lending_cascades_return()
  {
      // Passing only the lending — batch_delete cascades its orphaned returns.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 5, test_dt(10));
    add_return_at(lend_id, 3, test_dt(20));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(lend_id, "lending", s.item_id, 5, test_dt(10))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 10);
    teardown_db();
  }

  void test_batch_delete_multiple_actions()
  {
      // Delete return and its parent lending together.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 5, test_dt(10));
    int ret_id = add_return_at(lend_id, 3, test_dt(20));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(ret_id, "return", s.item_id, 3, test_dt(20)),
        make_row(lend_id, "lending", s.item_id, 5, test_dt(10))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 10);
    teardown_db();
  }

  void test_batch_delete_preserves_other_item()
  {
      // Deleting item1's acquisition must not affect item2.
    setup_db();
    auto const s = setup_item_and_person();
    int acq1 = add_acquisition_at(s.item_id, 5, test_dt(10));

    int type2 = queries_->add_new_type(make_magazine_type());
    colman::Item item2;
    item2.set_type_id(type2);
    item2.set_name("Magazine");
    int item2_id = queries_->add_new_item(item2);
    add_acquisition_at(item2_id, 3, test_dt(15));

    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(acq1, "acquisition", s.item_id, 5, test_dt(10))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));

    QVERIFY(queries_->query_items(s.item_id, QString()).empty());
    auto const items2 = queries_->query_items(item2_id, QString());
    QCOMPARE(items2.front().count, 3);
    teardown_db();
  }

  void test_batch_delete_full_chain()
  {
      // Delete acquisition + both lendings + return in one batch.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend1 = add_lending_at(s.item_id, s.person_id, 5, test_dt(10));
    int ret1 = add_return_at(lend1, 5, test_dt(20));
    int lend2 = add_lending_at(s.item_id, s.person_id, 5, test_dt(30));
    std::vector<colman::HistoryCombinedModel::Row> to_del{
        make_row(acq_id, "acquisition", s.item_id, 5, test_dt(5)),
        make_row(lend1, "lending", s.item_id, 5, test_dt(10)),
        make_row(ret1, "return", s.item_id, 5, test_dt(20)),
        make_row(lend2, "lending", s.item_id, 5, test_dt(30))};
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    QVERIFY(queries_->query_items(s.item_id, QString()).empty());
    teardown_db();
  }

  void test_batch_delete_group_lending()
  {
      // Deleting all lendings of a group lending restores the stock.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int const p2 = queries_->add_new_person(colman::make_test_person("B", "b"));
    int const p3 = queries_->add_new_person(colman::make_test_person("C", "c"));
    colman::Lending tmpl;
    tmpl.set_item_id(s.item_id);
    tmpl.set_date_time(test_dt(10));
    auto const res =
        queries_->add_group_lending(tmpl, {s.person_id, p2, p3});
    QCOMPARE(res.status_,
             colman::IQueries::GroupLendingResult::Status::Ok);
    std::vector<colman::HistoryCombinedModel::Row> to_del;
    for(int const id : res.lending_ids_) {
      to_del.push_back(make_row(id, "lending", s.item_id, 1, test_dt(10)));
    }
    QVERIFY(queries_->batch_delete_history_actions(to_del));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 10);
    QVERIFY(queries_->open_lendings_for_item(s.item_id).empty());
    teardown_db();
  }

    // ==================================================================
    // find_change_conflicts
    // ==================================================================

  void test_change_no_conflict_acq_count_increase()
  {
      // Increasing an acquisition never creates conflicts.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    add_lending_at(s.item_id, s.person_id, 5, test_dt(20));
    auto old_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    auto new_row = make_row(acq_id, "acquisition", s.item_id, 10, test_dt(10));
    QVERIFY(queries_->find_change_conflicts(old_row, new_row).empty());
    teardown_db();
  }

  void test_change_acq_moved_later_lending_conflict()
  {
      // Moving an acquisition past the lending removes the stock it needed.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    add_lending_at(s.item_id, s.person_id, 3, test_dt(20));
    auto old_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    auto new_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(25));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QCOMPARE(static_cast<int>(c.size()), 1);
    QCOMPARE(c.front().action, QString("lending"));
    teardown_db();
  }

  void test_change_acq_count_reduced_lending_conflict()
  {
      // Reducing an acquisition below what the lending needs.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    add_lending_at(s.item_id, s.person_id, 5, test_dt(20));
    auto old_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    auto new_row = make_row(acq_id, "acquisition", s.item_id, 3, test_dt(10));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QCOMPARE(static_cast<int>(c.size()), 1);
    QCOMPARE(c.front().action, QString("lending"));
    teardown_db();
  }

  void test_change_lending_self_check_insufficient_stock()
  {
      // Moving a lending to a time before the acquisition exists — the
      // acquisition is reported as the root cause.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(20));
    auto old_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(20));
    auto new_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(5));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QCOMPARE(static_cast<int>(c.size()), 1);
    QCOMPARE(c.front().action, QString("acquisition"));
    QCOMPARE(c.front().id, acq_id);
    teardown_db();
  }

  void test_change_lending_no_conflict_sufficient_stock()
  {
      // Moving a lending to a time with ample stock — no conflict.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(10));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(20));
    auto old_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(20));
    auto new_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(15));
    QVERIFY(queries_->find_change_conflicts(old_row, new_row).empty());
    teardown_db();
  }

  void test_change_lending_return_ordering_conflict()
  {
      // Moving the lending past its return violates ordering.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    add_return_at(lend_id, 3, test_dt(20));
    auto old_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(10));
    auto new_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(25));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QVERIFY(!c.empty());
    bool found_return = false;
    for(auto const& r : c) {
      if(r.action == "return") {
        found_return = true;
      }
    }
    QVERIFY(found_return);
    teardown_db();
  }

  void test_change_lending_count_excess_returns()
  {
      // Reducing a lending count below total returns → all returns conflict.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 5, test_dt(10));
    add_return_at(lend_id, 3, test_dt(20));
    add_return_at(lend_id, 2, test_dt(25));
    auto old_row = make_row(lend_id, "lending", s.item_id, 5, test_dt(10));
    auto new_row = make_row(lend_id, "lending", s.item_id, 4, test_dt(10));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    int ret_count = 0;
    for(auto const& r : c) {
      if(r.action == "return") {
        ++ret_count;
      }
    }
    QCOMPARE(ret_count, 2);
    teardown_db();
  }

  void test_change_return_self_check_insufficient_outstanding()
  {
      // Increasing a return count beyond what was lent — the parent lending
      // is reported as the root cause.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend_id, 2, test_dt(20));
      // Ask to return 4 — but only 3 were lent, so 4 exceeds outstanding.
    auto old_row = make_row(ret_id, "return", s.item_id, 2, test_dt(20));
    auto new_row = make_row(ret_id, "return", s.item_id, 4, test_dt(20));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QCOMPARE(static_cast<int>(c.size()), 1);
    QCOMPARE(c.front().action, QString("lending"));
    QCOMPARE(c.front().id, lend_id);
    teardown_db();
  }

  void test_change_return_parent_ordering_conflict()
  {
      // Moving a return before its parent lending.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(20));
    int ret_id = add_return_at(lend_id, 3, test_dt(30));
    auto old_row = make_row(ret_id, "return", s.item_id, 3, test_dt(30));
    auto new_row = make_row(ret_id, "return", s.item_id, 3, test_dt(15));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QVERIFY(!c.empty());
    bool found_lending = false;
    for(auto const& r : c) {
      if(r.action == "lending" && r.id == lend_id) {
        found_lending = true;
      }
    }
    QVERIFY(found_lending);
    teardown_db();
  }

  void test_change_liquidation_self_check_insufficient_stock()
  {
      // Moving a liquidation to a time before the acquisition — the
      // acquisition is reported as the root cause.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 3, test_dt(10));
    int liq_id = add_liquidation_at(s.item_id, 3, test_dt(20));
    auto old_row = make_row(liq_id, "liquidation", s.item_id, 3, test_dt(20));
    auto new_row = make_row(liq_id, "liquidation", s.item_id, 3, test_dt(5));
    auto const c = queries_->find_change_conflicts(old_row, new_row);
    QCOMPARE(static_cast<int>(c.size()), 1);
    QCOMPARE(c.front().action, QString("acquisition"));
    QCOMPARE(c.front().id, acq_id);
    teardown_db();
  }

  void test_change_liquidation_no_conflict()
  {
      // Moving a liquidation to a time with ample stock — no conflict.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int liq_id = add_liquidation_at(s.item_id, 3, test_dt(20));
    auto old_row = make_row(liq_id, "liquidation", s.item_id, 3, test_dt(20));
    auto new_row = make_row(liq_id, "liquidation", s.item_id, 3, test_dt(15));
    QVERIFY(queries_->find_change_conflicts(old_row, new_row).empty());
    teardown_db();
  }

  void test_change_lending_remarks_only_no_conflict()
  {
      // Changing only remarks (same count, same datetime) is always safe.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    auto old_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(10));
    auto new_row = old_row;
    new_row.remarks = "updated remark";
    QVERIFY(queries_->find_change_conflicts(old_row, new_row).empty());
    teardown_db();
  }

  void test_change_return_no_conflict_count_decrease()
  {
      // Decreasing a return count is always safe.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 5, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend_id, 3, test_dt(20));
    add_lending_at(s.item_id, s.person_id, 3, test_dt(30));
    auto old_row = make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    auto new_row = make_row(ret_id, "return", s.item_id, 1, test_dt(20));
      // The second lending at t=30 needs 3 copies. Without old return (3) but
      // with new return (1): avail at t=30 = 5 - 3 + 1 = 3 >= 3 → no conflict.
    QVERIFY(queries_->find_change_conflicts(old_row, new_row).empty());
    teardown_db();
  }

    // ==================================================================
    // apply_action_change
    // ==================================================================

  void test_apply_change_acquisition_count()
  {
      // Changing acquisition count recomputes item counts correctly.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    auto old_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    auto new_row = make_row(acq_id, "acquisition", s.item_id, 3, test_dt(10));
    QVERIFY(queries_->apply_action_change(old_row, new_row));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 3);
    QCOMPARE(items.front().available, 3);
    teardown_db();
  }

  void test_apply_change_acquisition_datetime()
  {
      // Changing acquisition datetime is reflected in the stored record.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    auto old_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    auto new_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(15));
    QVERIFY(queries_->apply_action_change(old_row, new_row));
    colman::Acquisition acq;
    QVERIFY(queries_->get_acquisition(acq_id, acq));
    QCOMPARE(acq.get_date_time(), test_dt(15));
    teardown_db();
  }

  void test_apply_change_liquidation_count()
  {
      // Changing liquidation count recomputes item counts correctly.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int liq_id = add_liquidation_at(s.item_id, 3, test_dt(10));
    auto old_row = make_row(liq_id, "liquidation", s.item_id, 3, test_dt(10));
    auto new_row = make_row(liq_id, "liquidation", s.item_id, 1, test_dt(10));
    QVERIFY(queries_->apply_action_change(old_row, new_row));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 9);
    QCOMPARE(items.front().available, 9);
    teardown_db();
  }

  void test_apply_change_lending_count_and_remarks()
  {
      // Changing lending count updates items.available; remarks are stored.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    auto old_row = make_row(lend_id, "lending", s.item_id, 3, test_dt(10));
    old_row.person_id = s.person_id;
    auto new_row = old_row;
    new_row.count = 5;
    new_row.remarks = "changed remark";
    QVERIFY(queries_->apply_action_change(old_row, new_row));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 5);
    colman::Lending l;
    QVERIFY(queries_->get_lending(lend_id, l));
    QCOMPARE(l.get_remarks(), QString("changed remark"));
    teardown_db();
  }

  void test_apply_change_return_count()
  {
      // Changing return count updates items.available.
    setup_db();
    auto const s = setup_item_and_person();
    add_acquisition_at(s.item_id, 10, test_dt(5));
    int lend_id = add_lending_at(s.item_id, s.person_id, 3, test_dt(10));
    int ret_id = add_return_at(lend_id, 2, test_dt(20));
    auto old_row = make_row(ret_id, "return", s.item_id, 2, test_dt(20));
    auto new_row = make_row(ret_id, "return", s.item_id, 3, test_dt(20));
    QVERIFY(queries_->apply_action_change(old_row, new_row));
    auto const items = queries_->query_items(s.item_id, QString());
    QCOMPARE(items.front().count, 10);
    QCOMPARE(items.front().available, 10);
    teardown_db();
  }

  void test_apply_change_updates_collector_id()
  {
      // collector_id written to the acquisitions table.
    setup_db();
    auto const s = setup_item_and_person();
    int acq_id = add_acquisition_at(s.item_id, 5, test_dt(10));
    auto old_row = make_row(acq_id, "acquisition", s.item_id, 5, test_dt(10));
    auto new_row = old_row;
    new_row.collector_id = s.person_id;
    QVERIFY(queries_->apply_action_change(old_row, new_row));
    colman::Acquisition acq;
    QVERIFY(queries_->get_acquisition(acq_id, acq));
    QCOMPARE(acq.get_data_collector_id(), s.person_id);
    teardown_db();
  }
};


QTEST_MAIN(ColmanQueriesTest)
#include "colman_queries_test.moc"

/* EOF */
