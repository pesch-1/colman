// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for possible_actions() in colman_actions.hpp.
/**
   Uses Qt Test with in-memory SQLite databases.
   Each test method creates a fresh database via setup_db() and tears it
   down with teardown_db() to guarantee isolation.
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_ACTIONS_HPP
#  include "colman_actions.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
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
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif
#ifndef COLMAN_EAN_QUERIES_HPP
#  include "colman_ean_queries.hpp"
#  define COLMAN_EAN_QUERIES_HPP
#endif

namespace colman
{

  namespace
  {

    /// Builds a book Type with ISBN10, ISBN13 (repeatable), and author.
    /**
     * ISBN10 and ISBN13 are marked repeatable so they are stored in
     * item_list_values, matching the lookup performed by items_by_isbn().
     */
    Type make_isbn_book_type()
    {
      KeyVector keys;
      QVariantMap rep;
      rep["is_repeatable"] = true;
      keys.push_back({"ISBN10", {DataType::ShortString, rep}});
      keys.push_back({"ISBN13", {DataType::ShortString, rep}});
      keys.push_back({"author", {DataType::ShortString, {}}});
      return Type("Book", keys);
    }

    /// Builds a CD Type with a non-repeatable "ean" ShortString field.
    Type make_cd_type()
    {
      KeyVector keys;
      keys.push_back({"ean", {DataType::ShortString, {}}});
      return Type("CD", keys);
    }

    /// Returns true if \a actions contains exactly one action of type \a a.
    bool has_action(std::vector<ActionInfo> const& actions, Action a)
    {
      return std::any_of(actions.begin(), actions.end(),
                         [a](ActionInfo const& ai) { return ai.action_ == a; });
    }

    /// Returns the number of actions of type \a a in \a actions.
    int count_actions(std::vector<ActionInfo> const& actions, Action a)
    {
      return static_cast<int>(std::count_if(actions.begin(), actions.end(),
                                            [a](ActionInfo const& ai) { return ai.action_ == a; }));
    }

    /// Returns the first action of type \a a, or a default-constructed one.
    ActionInfo find_action(std::vector<ActionInfo> const& actions, Action a)
    {
      auto it = std::find_if(actions.begin(), actions.end(),
                             [a](ActionInfo const& ai) { return ai.action_ == a; });
      return it != actions.end() ? *it : ActionInfo{};
    }

  } // anonymous namespace

  /// Qt Test class exercising possible_actions() for all barcode cases.
  class ColmanActionsTest : public QObject
  {
    Q_OBJECT

  public:
    ColmanActionsTest() = default;

  private:
    std::unique_ptr<SQLQueries> queries_; ///< query facade (owns its db)
    QString conn_name_;    ///< unique connection name for the current test
    int conn_counter_ = 0;   ///< monotone counter to generate unique names

    // ------------------------------------------------------------------
    // Infrastructure
    // ------------------------------------------------------------------

    /// Opens a fresh in-memory database and initialises all tables.
    void setup_db()
    {
      conn_name_ = QString("actions_test_%1").arg(conn_counter_++);
      queries_ = std::make_unique<SQLQueries>(conn_name_);
      QVERIFY(queries_->create_tables());
      Collection col = make_test_collection();
      QVERIFY(queries_->create_collection(col));
      QVERIFY(queries_->add_administrator());
    }

    /// Releases the current test database (SQLQueries destructor handles cleanup).
    void teardown_db()
    {
      queries_.reset();
    }

    /// Creates a book type + item + acquisition + person.
    /**
     * @param stock    number of copies to acquire
     * @param isbn10   ISBN-10 to store on the item (empty = omit)
     * @param isbn13   ISBN-13 to store on the item (empty = omit)
     * @param item_id  output: created item id
     * @param person_id output: created person id
     * @param type_id  output: created type id
     */
    void setup_book(int stock, QString const& isbn10, QString const& isbn13,
                    int& item_id, int& person_id, int& type_id)
    {
      type_id = queries_->add_new_type(make_isbn_book_type());
      if(type_id < 0) {
        QTest::qFail("add_new_type failed", __FILE__, __LINE__);
        return;
      }

      Item item;
      item.set_type_id(type_id);
      item.set_name("Test Book");
      if(!isbn10.isEmpty()) {
        item.set_list_value("ISBN10", QStringList{isbn10});
      }
      if(!isbn13.isEmpty()) {
        item.set_list_value("ISBN13", QStringList{isbn13});
      }
      item_id = queries_->add_new_item(item);
      if(item_id < 0) {
        QTest::qFail("add_new_item failed", __FILE__, __LINE__);
        return;
      }

      if(stock > 0) {
        Acquisition acq;
        acq.set_item_id(item_id);
        acq.set_count(static_cast<unsigned int>(stock));
        acq.set_date_time(QDateTime::currentDateTimeUtc());
        if(queries_->add_new_acquisition(acq) < 0) {
          QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
          return;
        }
      }

      person_id = queries_->add_new_person(make_test_person());
      if(person_id < 0) {
        QTest::qFail("add_new_person failed", __FILE__, __LINE__);
      }
    }

    /// Creates a lending of \a count copies and returns the lending id.
    int lend(int item_id, int person_id, unsigned int count = 1)
    {
      Lending l;
      l.set_item_id(item_id);
      l.set_person_id(person_id);
      l.set_count(count);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      int id = queries_->add_new_lending(l);
      if(id < 0) {
        QTest::qFail("add_new_lending failed", __FILE__, __LINE__);
      }
      return id;
    }

    /// Returns \a count copies of lending \a lending_id.
    void ret(int lending_id, unsigned int count = 1)
    {
      Return r;
      r.set_lending_id(lending_id);
      r.set_count(count);
      r.set_date_time(QDateTime::currentDateTimeUtc());
      if(queries_->add_new_return(r) < 0) {
        QTest::qFail("add_new_return failed", __FILE__, __LINE__);
      }
    }

  private Q_SLOTS:

    // ==================================================================
    // 1. Item barcode
    // ==================================================================

    void test_item_barcode_available_no_lendings()
    {
      // 1.2: available copies → Lending; no open lendings → no Return.
      // No Acquisition action for item barcodes.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Return), 0);

      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);

      teardown_db();
    }

    void test_item_barcode_all_lent_out()
    {
      // 1.1: all copies borrowed → Return; no copies available → no Lending.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(3, "", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 3);
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Return), 1);
      QCOMPARE(count_actions(actions, Action::Lending), 0);

      ActionInfo const ret = find_action(actions, Action::Return);
      QCOMPARE(ret.item_id_, item_id);
      QCOMPARE(ret.lending_id_, lending_id);
      QCOMPARE(ret.person_id_, person_id);

      teardown_db();
    }

    void test_item_barcode_partially_lent()
    {
      // 1.1 + 1.2: some copies borrowed and some available.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 2);
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Return), 1);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      ActionInfo const ret = find_action(actions, Action::Return);
      QCOMPARE(ret.item_id_, item_id);
      QCOMPARE(ret.lending_id_, lending_id);
      QCOMPARE(ret.person_id_, person_id);

      teardown_db();
    }

    void test_item_barcode_multiple_open_lendings()
    {
      // 1.1: two persons have open lendings → two Return actions.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(10, "", "", item_id, person_id, type_id);
      int const p2_id = queries_->add_new_person(make_test_person("Second", "Person"));
      int const l1 = lend(item_id, person_id, 3);
      int const l2 = lend(item_id, p2_id, 2);
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Return), 2);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      // Both lending ids must appear.
      bool found_l1 = false, found_l2 = false;
      for(ActionInfo const& ai : actions) {
        if(ai.action_ != Action::Return) {
          continue;
        }
        if(ai.lending_id_ == l1) {
          found_l1 = true;
        }
        if(ai.lending_id_ == l2) {
          found_l2 = true;
        }
      }
      QVERIFY(found_l1);
      QVERIFY(found_l2);

      teardown_db();
    }

    void test_item_barcode_lending_fully_returned()
    {
      // A closed lending (all copies returned) must not produce a Return.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 3);
      ret(lending_id, 3);
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Return), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Acquisition), 0);

      teardown_db();
    }

    void test_item_barcode_lending_partially_returned_still_open()
    {
      // A lending where only some copies were returned is still open.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 3);
      ret(lending_id, 1); // 2 copies still out
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Return), 1);
      ActionInfo const r = find_action(actions, Action::Return);
      QCOMPARE(r.lending_id_, lending_id);

      teardown_db();
    }

    void test_item_barcode_lending_carries_item_id()
    {
      // The Lending action for an item barcode must carry item_id.
      // No Acquisition action is produced for item barcodes.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      QString const bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);
      QCOMPARE(lend.person_id_, -1);

      teardown_db();
    }

    // ==================================================================
    // 2. Person barcode
    // ==================================================================

    void test_person_barcode_no_open_lendings()
    {
      // 2.2: person has nothing borrowed → Lending only.
      setup_db();
      int const person_id = queries_->add_new_person(make_test_person());
      QString const bc = queries_->compute_person_barcode(person_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Return), 0);

      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.person_id_, person_id);
      QCOMPARE(lend.item_id_, -1);

      teardown_db();
    }

    void test_person_barcode_one_open_lending()
    {
      // 2.1 + 2.2: one open lending → Return + Lending.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 2);
      QString const bc = queries_->compute_person_barcode(person_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Return), 1);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      ActionInfo const ret = find_action(actions, Action::Return);
      QCOMPARE(ret.item_id_, item_id);
      QCOMPARE(ret.lending_id_, lending_id);
      QCOMPARE(ret.person_id_, person_id);

      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.person_id_, person_id);

      teardown_db();
    }

    void test_person_barcode_multiple_items_borrowed()
    {
      // 2.1 + 2.2: person borrowed two different items → two Return + Lending.
      setup_db();
      int item1 = -1, item2 = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item1, person_id, type_id);
      // second item shares same type; add manually
      Item item;
      item.set_type_id(type_id);
      item.set_name("Second Book");
      item2 = queries_->add_new_item(item);
      Acquisition acq;
      acq.set_item_id(item2);
      acq.set_count(5);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      if(queries_->add_new_acquisition(acq) < 0) {
        QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
      }

      int const l1 = lend(item1, person_id, 1);
      int const l2 = lend(item2, person_id, 1);
      QString const bc = queries_->compute_person_barcode(person_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Return), 2);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      bool found_l1 = false, found_l2 = false;
      for(ActionInfo const& ai : actions) {
        if(ai.action_ != Action::Return) {
          continue;
        }
        if(ai.lending_id_ == l1) {
          found_l1 = true;
        }
        if(ai.lending_id_ == l2) {
          found_l2 = true;
        }
      }
      QVERIFY(found_l1);
      QVERIFY(found_l2);

      teardown_db();
    }

    void test_person_barcode_fully_returned_lending_no_return_action()
    {
      // A fully returned lending must not generate a Return action.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 2);
      ret(lending_id, 2);
      QString const bc = queries_->compute_person_barcode(person_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QCOMPARE(count_actions(actions, Action::Return), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      teardown_db();
    }

    void test_person_barcode_lending_always_present_even_with_open_returns()
    {
      // 2.2: Lending is added unconditionally, even when open lendings exist.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      lend(item_id, person_id, 3);
      QString const bc = queries_->compute_person_barcode(person_id);

      auto const actions = colman::possible_actions(*queries_, bc);

      QVERIFY(has_action(actions, Action::Lending));
      QVERIFY(has_action(actions, Action::Return));

      teardown_db();
    }

    // ==================================================================
    // 3. ISBN barcode
    // ==================================================================

    void test_isbn_barcode_book_not_in_collection()
    {
      // 3.1: unknown ISBN → single Acquisition without any ids.
      setup_db();
      queries_->add_new_type(make_isbn_book_type()); // type exists but no item with this isbn

      auto const actions = colman::possible_actions(*queries_, "9999999999");

      QCOMPARE(static_cast<int>(actions.size()), 1);
      QCOMPARE(actions[0].action_, Action::Acquisition);
      QCOMPARE(actions[0].item_id_, -1);
      QCOMPARE(actions[0].lending_id_, -1);
      QCOMPARE(actions[0].person_id_, -1);

      teardown_db();
    }

    void test_isbn10_barcode_book_available()
    {
      // 3.2 + 1.2: ISBN-10 matches, copies available → Lending only.
      // No Acquisition when the book is already in the collection.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "0306406152", "", item_id, person_id, type_id);

      auto const actions = colman::possible_actions(*queries_, "0306406152");

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Return), 0);

      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);

      teardown_db();
    }

    void test_isbn13_barcode_book_available()
    {
      // 3.2 + 1.2: ISBN-13 matches, copies available → Lending only.
      // No Acquisition when the book is already in the collection.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "9780306406157", item_id, person_id, type_id);

      auto const actions = colman::possible_actions(*queries_, "9780306406157");

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Return), 0);

      teardown_db();
    }

    void test_isbn_barcode_book_partially_lent()
    {
      // 3.2 + 1.1 + 1.2: some copies borrowed → Return + Lending (no Acquisition).
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "0306406152", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 2);

      auto const actions = colman::possible_actions(*queries_, "0306406152");

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Return), 1);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      ActionInfo const ret = find_action(actions, Action::Return);
      QCOMPARE(ret.item_id_, item_id);
      QCOMPARE(ret.lending_id_, lending_id);
      QCOMPARE(ret.person_id_, person_id);

      teardown_db();
    }

    void test_isbn_barcode_all_copies_lent()
    {
      // 3.2 + 1.1: all copies lent → Return only (no Acquisition, no Lending).
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(3, "0306406152", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 3);

      auto const actions = colman::possible_actions(*queries_, "0306406152");

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Return), 1);
      QCOMPARE(count_actions(actions, Action::Lending), 0);

      ActionInfo const ret = find_action(actions, Action::Return);
      QCOMPARE(ret.lending_id_, lending_id);

      teardown_db();
    }

    void test_isbn_barcode_lending_carries_item_id()
    {
      // For a known ISBN the Lending action must carry item_id.
      // No Acquisition is produced when the book is already in the collection.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "0306406152", "", item_id, person_id, type_id);

      auto const actions = colman::possible_actions(*queries_, "0306406152");

      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);
      QCOMPARE(lend.person_id_, -1);
      QCOMPARE(lend.lending_id_, -1);

      teardown_db();
    }

    void test_isbn_barcode_unknown_barcode_treated_as_unknown_isbn()
    {
      // A barcode that is neither item nor person barcode and matches no ISBN
      // is treated as an unknown book: single Acquisition with no ids.
      setup_db();

      auto const actions = colman::possible_actions(*queries_, "UNKNOWN9999");

      QCOMPARE(static_cast<int>(actions.size()), 1);
      QCOMPARE(actions[0].action_, Action::Acquisition);
      QCOMPARE(actions[0].item_id_, -1);

      teardown_db();
    }

    void test_isbn_barcode_closed_lending_not_returned()
    {
      // A fully-returned lending must not generate a Return even via ISBN.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "0306406152", "", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 3);
      ret(lending_id, 3);

      auto const actions = colman::possible_actions(*queries_, "0306406152");

      QCOMPARE(count_actions(actions, Action::Return), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);

      teardown_db();
    }

    // ==================================================================
    // 3. Product EAN / UPC-A barcode
    // ==================================================================

    /// Creates a CD type + item with \a ean set + acquisition + person.
    void setup_cd(int stock, QString const& ean,
                  int& item_id, int& person_id, int& type_id)
    {
      type_id = queries_->add_new_type(make_cd_type());
      if(type_id < 0) {
        QTest::qFail("add_new_type failed", __FILE__, __LINE__);
        return;
      }
      Item item;
      item.set_type_id(type_id);
      item.set_name("Test CD");
      if(!ean.isEmpty()) {
        item.set_value("ean", ean);
      }
      item_id = queries_->add_new_item(item);
      if(item_id < 0) {
        QTest::qFail("add_new_item failed", __FILE__, __LINE__);
        return;
      }
      if(stock > 0) {
        Acquisition acq;
        acq.set_item_id(item_id);
        acq.set_count(static_cast<unsigned int>(stock));
        acq.set_date_time(QDateTime::currentDateTimeUtc());
        if(queries_->add_new_acquisition(acq) < 0) {
          QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
          return;
        }
      }
      person_id = queries_->add_new_person(make_test_person());
      if(person_id < 0) {
        QTest::qFail("add_new_person failed", __FILE__, __LINE__);
      }
    }

    void test_ean_barcode_not_in_collection_yields_acquisition()
    {
      // 3.1: EAN not in DB → single Acquisition with the barcode set.
      setup_db();
      queries_->add_new_type(make_cd_type());
      // A syntactically valid product EAN-13 with correct checksum.
      auto const actions =
          colman::possible_actions(*queries_, "4050538315578");
      QCOMPARE(static_cast<int>(actions.size()), 1);
      QCOMPARE(actions[0].action_, Action::Acquisition);
      QCOMPARE(actions[0].item_id_, -1);
      QCOMPARE(actions[0].barcode_, QString("4050538315578"));
      teardown_db();
    }

    void test_ean_barcode_in_collection_available_yields_lending()
    {
      // 3.2 + 1.2: EAN matches item with available copies → Lending, no Acquisition.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_cd(3, "4050538315578", item_id, person_id, type_id);
      auto const actions =
          colman::possible_actions(*queries_, "4050538315578");
      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Return), 0);
      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);
      teardown_db();
    }

    void test_ean_barcode_in_collection_all_lent_yields_return()
    {
      // 3.2 + 1.1: EAN matches item, all copies lent → Return only.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_cd(2, "4050538315578", item_id, person_id, type_id);
      int const lending_id = lend(item_id, person_id, 2);
      auto const actions =
          colman::possible_actions(*queries_, "4050538315578");
      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Return), 1);
      QCOMPARE(count_actions(actions, Action::Lending), 0);
      ActionInfo const ret = find_action(actions, Action::Return);
      QCOMPARE(ret.item_id_, item_id);
      QCOMPARE(ret.lending_id_, lending_id);
      teardown_db();
    }

    void test_ean_13digit_finds_12digit_in_db()
    {
      // When the DB stores the EAN without a leading zero (12-digit form),
      // scanning the full 13-digit EAN (with leading zero) must still find it.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      // Store 12-digit form (e.g. as imported from DNB field 024).
      setup_cd(2, "602475811497", item_id, person_id, type_id);
      // Scan the 13-digit form.
      auto const actions =
          colman::possible_actions(*queries_, "0602475811497");
      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      teardown_db();
    }

    void test_upc_a_barcode_in_collection_yields_lending()
    {
      // A 12-digit UPC-A barcode is converted to EAN-13 by prepending "0".
      // The item stored with EAN "0602475811497" must be found.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_cd(2, "0602475811497", item_id, person_id, type_id);
      // Scan the 12-digit UPC-A form (is_upc_a returns true for this).
      auto const actions =
          colman::possible_actions(*queries_, "602475811497");
      QCOMPARE(count_actions(actions, Action::Acquisition), 0);
      QCOMPARE(count_actions(actions, Action::Lending), 1);
      QCOMPARE(count_actions(actions, Action::Return), 0);
      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);
      teardown_db();
    }

    // ==================================================================
    // Mixed / edge cases
    // ==================================================================

    void test_item_barcode_not_recognised_as_person_barcode()
    {
      // An item barcode must not match as a person barcode.
      setup_db();
      int item_id = -1, person_id = -1, type_id = -1;
      setup_book(5, "", "", item_id, person_id, type_id);
      QString const item_bc = queries_->compute_item_barcode(type_id, item_id);

      auto const actions = colman::possible_actions(*queries_, item_bc);

      // Must produce item-case actions, not person-case.
      // Lending carries item_id (not person_id) for item barcodes.
      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.item_id_, item_id);
      QCOMPARE(lend.person_id_, -1);

      teardown_db();
    }

    void test_person_barcode_not_recognised_as_item_barcode()
    {
      // A person barcode must not match as an item barcode.
      setup_db();
      int const person_id = queries_->add_new_person(make_test_person());
      QString const person_bc = queries_->compute_person_barcode(person_id);

      auto const actions = colman::possible_actions(*queries_, person_bc);

      // Must produce person-case actions.
      ActionInfo const lend = find_action(actions, Action::Lending);
      QCOMPARE(lend.person_id_, person_id);
      QCOMPARE(lend.item_id_, -1);

      teardown_db();
    }

    void test_empty_result_never_returned()
    {
      // possible_actions must always return at least one action.
      setup_db();
      queries_->add_new_type(make_isbn_book_type());

      auto const actions = colman::possible_actions(*queries_, "XXXXX9999");

      QVERIFY(!actions.empty());

      teardown_db();
    }

  }; // class ColmanActionsTest

} // namespace colman

QTEST_GUILESS_MAIN(colman::ColmanActionsTest)
#include "colman_actions_test.moc"

/* EOF */
