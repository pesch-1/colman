// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Unit tests for the reservation feature.
/**
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_OPTIONAL
#  include <optional>
#  define SYSTEM_OPTIONAL
#endif
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef SYSTEM_QOBJECT
#  include <QObject>
#  define SYSTEM_QOBJECT
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_ACQUISITION_HPP
#  include "colman_acquisition.hpp"
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#endif
#ifndef COLMAN_LENDING_HPP
#  include "colman_lending.hpp"
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif

namespace
{

  /// Returns tomorrow's date.
  QDate tomorrow()
  {
    return QDate::currentDate().addDays(1);
  }

  /// Returns yesterday's date (already expired).
  QDate yesterday()
  {
    return QDate::currentDate().addDays(-1);
  }

  class ColmanReservationsTest : public QObject
  {
    Q_OBJECT

  public:
    ColmanReservationsTest() = default;

  private:
    std::optional<colman::SQLQueries> q_;
    QString conn_name_;
    int conn_counter_ = 0;

    void setup_db()
    {
      conn_name_ = QString("colman_res_test_%1").arg(conn_counter_++);
      q_.emplace(conn_name_);
      QVERIFY(q_->create_tables());
      auto col = colman::make_test_collection();
      QVERIFY(q_->create_collection(col));
      QVERIFY(q_->add_administrator());
    }

    void teardown_db()
    {
      q_.reset();
    }

    /// Creates a book type + item with \a stock copies; returns item_id.
    int setup_item(int stock = 3)
    {
      int type_id = q_->add_new_type(colman::make_book_type());
      if(type_id < 0) {
        type_id = q_->get_type_id("Book");
      }
      if(type_id < 0) {
        QTest::qFail("add_new_type failed", __FILE__, __LINE__);
        return -1;
      }
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Test Book");
      int const item_id = q_->add_new_item(item);
      if(item_id < 0) {
        QTest::qFail("add_new_item failed", __FILE__, __LINE__);
        return -1;
      }
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(static_cast<unsigned int>(stock));
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      if(q_->add_new_acquisition(acq) < 0) {
        QTest::qFail("add_new_acquisition failed", __FILE__, __LINE__);
        return -1;
      }
      return item_id;
    }

    /// Creates a person and returns the person_id.
    int setup_person(QString const& name = "User")
    {
      int const pid = q_->add_new_person(colman::make_test_person(name));
      if(pid < 0) {
        QTest::qFail("add_new_person failed", __FILE__, __LINE__);
      }
      return pid;
    }

  private Q_SLOTS:

    // ------------------------------------------------------------------
    // Collection settings
    // ------------------------------------------------------------------

    void test_allow_reservations_default_is_true()
    {
      setup_db();
      auto const col = q_->read_collection();
      QVERIFY(col.get_allow_reservations());
      teardown_db();
    }

    void test_reservation_period_days_default()
    {
      setup_db();
      auto const col = q_->read_collection();
      QCOMPARE(col.get_reservation_period_days(), 14);
      teardown_db();
    }

    void test_change_reservation_settings()
    {
      setup_db();
      auto col = q_->read_collection();
      col.set_allow_reservations(false);
      col.set_reservation_period_days(7);
      QVERIFY(q_->change_collection(col));
      auto const col2 = q_->read_collection();
      QVERIFY(!col2.get_allow_reservations());
      QCOMPARE(col2.get_reservation_period_days(), 7);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // Permissions
    // ------------------------------------------------------------------

    void test_new_user_has_reserve_permission()
    {
      setup_db();
      int const pid = setup_person();
      QVERIFY(q_->has_permission(pid, "reserve"));
      teardown_db();
    }

    void test_new_user_has_no_edit_reservations()
    {
      setup_db();
      int const pid = setup_person();
      QVERIFY(!q_->has_permission(pid, "edit_reservations"));
      teardown_db();
    }

    void test_admin_has_edit_reservations()
    {
      setup_db();
      QVERIFY(q_->has_permission(0, "edit_reservations"));
      teardown_db();
    }

    // ------------------------------------------------------------------
    // Core reservation logic
    // ------------------------------------------------------------------

    void test_add_reservation_succeeds()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const person_id = setup_person();
      int new_id = 0;
      auto const result =
          q_->add_reservation(item_id, person_id, 1, tomorrow(), 0, new_id);
      QCOMPARE(result, colman::IQueries::ReservationResult::Ok);
      QVERIFY(new_id > 0);
      teardown_db();
    }

    void test_add_second_reservation_same_person_tops_up()
    {
      setup_db();
      int const item_id = setup_item(2);
      int const person_id = setup_person();
      int new_id = 0;
      QDate const first_expiry = tomorrow();
      QCOMPARE(q_->add_reservation(item_id, person_id, 1, first_expiry, 0,
                                   new_id),
               colman::IQueries::ReservationResult::Ok);
      // Reserving again increases the count of the existing reservation
      // and keeps its expiry date.
      int new_id2 = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 1,
                                   tomorrow().addDays(30), 0, new_id2),
               colman::IQueries::ReservationResult::Ok);
      QCOMPARE(new_id2, new_id);
      colman::IQueries::Reservation r;
      QVERIFY(q_->get_reservation(item_id, person_id, r));
      QCOMPARE(r.count, 2);
      QCOMPARE(r.expires_at, first_expiry);
      // All copies are reserved now: a third reservation must fail.
      int new_id3 = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 1, tomorrow(), 0,
                                   new_id3),
               colman::IQueries::ReservationResult::NotEnoughAvailable);
      teardown_db();
    }

    void test_two_users_can_reserve_same_item()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const p1 = setup_person("Alice");
      int const p2 = setup_person("Bob");
      int id1 = 0, id2 = 0;
      QCOMPARE(q_->add_reservation(item_id, p1, 1, tomorrow(), 0, id1),
               colman::IQueries::ReservationResult::Ok);
      QCOMPARE(q_->add_reservation(item_id, p2, 1, tomorrow(), 0, id2),
               colman::IQueries::ReservationResult::Ok);
      QVERIFY(id1 != id2);
      teardown_db();
    }

    void test_total_reservation_count_cannot_exceed_available()
    {
      setup_db();
      int const item_id = setup_item(2);
      int const p1 = setup_person("Alice");
      int const p2 = setup_person("Bob");
      int const p3 = setup_person("Carol");
      int id1 = 0;
      QCOMPARE(q_->add_reservation(item_id, p1, 1, tomorrow(), 0, id1),
               colman::IQueries::ReservationResult::Ok);
      int id2 = 0;
      QCOMPARE(q_->add_reservation(item_id, p2, 1, tomorrow(), 0, id2),
               colman::IQueries::ReservationResult::Ok);
      // available=2, both copies now reserved: p3 cannot reserve
      int id3 = 0;
      QCOMPARE(q_->add_reservation(item_id, p3, 1, tomorrow(), 0, id3),
               colman::IQueries::ReservationResult::NotEnoughAvailable);
      teardown_db();
    }

    void test_remove_reservation()
    {
      setup_db();
      int const item_id = setup_item();
      int const person_id = setup_person();
      int new_id = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 1, tomorrow(), 0, new_id),
               colman::IQueries::ReservationResult::Ok);
      QVERIFY(q_->remove_reservation(new_id));
      colman::IQueries::Reservation r;
      QVERIFY(!q_->get_reservation(item_id, person_id, r));
      teardown_db();
    }

    void test_get_reservation_missing_returns_false()
    {
      setup_db();
      int const item_id = setup_item();
      int const person_id = setup_person();
      colman::IQueries::Reservation r;
      QVERIFY(!q_->get_reservation(item_id, person_id, r));
      teardown_db();
    }

    void test_get_reservations_for_item_returns_all()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const p1 = setup_person("Alice");
      int const p2 = setup_person("Bob");
      int id1 = 0, id2 = 0;
      q_->add_reservation(item_id, p1, 1, tomorrow(), 0, id1);
      q_->add_reservation(item_id, p2, 1, tomorrow(), 0, id2);
      auto const ress = q_->get_reservations_for_item(item_id);
      QCOMPARE(static_cast<int>(ress.size()), 2);
      teardown_db();
    }

    void test_get_reservations_for_person()
    {
      setup_db();
      int const item1 = setup_item(3);
      int const item2 = setup_item(3);
      int const pid = setup_person();
      int id1 = 0, id2 = 0;
      q_->add_reservation(item1, pid, 1, tomorrow(), 0, id1);
      q_->add_reservation(item2, pid, 1, tomorrow(), 0, id2);
      auto const ress = q_->get_reservations(pid);
      QCOMPARE(static_cast<int>(ress.size()), 2);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // Expiry / lazy cleanup
    // ------------------------------------------------------------------

    void test_expired_reservation_purged_on_get()
    {
      setup_db();
      int const item_id = setup_item();
      int const person_id = setup_person();
      int new_id = 0;
      // Insert reservation that has already expired.
      QCOMPARE(q_->add_reservation(item_id, person_id, 1, yesterday(), 0, new_id),
               colman::IQueries::ReservationResult::Ok);
      colman::IQueries::Reservation r;
      QVERIFY(!q_->get_reservation(item_id, person_id, r));
      teardown_db();
    }

    void test_expired_reservation_purged_on_lending()
    {
      setup_db();
      int const item_id = setup_item(2);
      int const p_reserver = setup_person("Reserver");
      int const p_borrower = setup_person("Borrower");
      // Insert expired reservation for p_reserver.
      int res_id = 0;
      QCOMPARE(q_->add_reservation(item_id, p_reserver, 2, yesterday(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      // p_borrower should now be able to lend (expired reservation cleaned up).
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(p_borrower);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // Lending blocked / freed by reservations
    // ------------------------------------------------------------------

    void test_lending_blocked_when_all_copies_reserved_by_others()
    {
      setup_db();
      int const item_id = setup_item(2);
      int const p_reserver = setup_person("Reserver");
      int const p_borrower = setup_person("Borrower");
      int res_id = 0;
      // p_reserver reserves all 2 copies.
      QCOMPARE(q_->add_reservation(item_id, p_reserver, 2, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(p_borrower);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QCOMPARE(q_->add_new_lending(l), -2);
      // available must be unchanged
      QCOMPARE(static_cast<int>(q_->available(item_id)), 2);
      teardown_db();
    }

    void test_lending_possible_when_some_copies_not_reserved()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const p_reserver = setup_person("Reserver");
      int const p_borrower = setup_person("Borrower");
      int res_id = 0;
      // p_reserver reserves 2 of 3 copies.
      QCOMPARE(q_->add_reservation(item_id, p_reserver, 2, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      // p_borrower can lend the remaining 1 copy.
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(p_borrower);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      QCOMPARE(static_cast<int>(q_->available(item_id)), 2);
      teardown_db();
    }

    void test_reservation_holder_can_lend_own_reserved_copies()
    {
      setup_db();
      int const item_id = setup_item(2);
      int const person_id = setup_person();
      int res_id = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 2, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(person_id);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      teardown_db();
    }

    void test_reservation_partially_consumed_on_lending()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const person_id = setup_person();
      int res_id = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 2, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      // Lend 1 copy — reservation should shrink to 1.
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(person_id);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      colman::IQueries::Reservation r;
      QVERIFY(q_->get_reservation(item_id, person_id, r));
      QCOMPARE(r.count, 1);
      teardown_db();
    }

    void test_reservation_fully_consumed_on_lending()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const person_id = setup_person();
      int res_id = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 2, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      // Lend 2 copies — reservation should be deleted.
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(person_id);
      l.set_count(2);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      colman::IQueries::Reservation r;
      QVERIFY(!q_->get_reservation(item_id, person_id, r));
      teardown_db();
    }

    void test_lending_after_reservation_deleted_succeeds()
    {
      setup_db();
      int const item_id = setup_item(2);
      int const p_holder = setup_person("Holder");
      int const p_other = setup_person("Other");
      int res_id = 0;
      QCOMPARE(q_->add_reservation(item_id, p_holder, 2, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      QVERIFY(q_->remove_reservation(res_id));
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(p_other);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      teardown_db();
    }

    void test_available_count_correct_after_reservation_and_lending()
    {
      setup_db();
      int const item_id = setup_item(3);
      int const person_id = setup_person();
      // Reservations must not change the available count.
      QCOMPARE(static_cast<int>(q_->available(item_id)), 3);
      int res_id = 0;
      QCOMPARE(q_->add_reservation(item_id, person_id, 1, tomorrow(), 0, res_id),
               colman::IQueries::ReservationResult::Ok);
      QCOMPARE(static_cast<int>(q_->available(item_id)), 3);
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(person_id);
      l.set_count(1);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      QVERIFY(q_->add_new_lending(l) > 0);
      QCOMPARE(static_cast<int>(q_->available(item_id)), 2);
      teardown_db();
    }

    // ------------------------------------------------------------------
    // Migration idempotency
    // ------------------------------------------------------------------

    void test_migrate_reservations_idempotent()
    {
      setup_db();
      // A second create_tables() call must not crash or fail (IF NOT EXISTS).
      QVERIFY(q_->create_tables());
      teardown_db();
    }
  };

} // namespace

QTEST_MAIN(ColmanReservationsTest)
#include "colman_reservations_test.moc"
