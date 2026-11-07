// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for colman_permission_deps.hpp: profiles and delegated authorization.
/**
   Covers the permission dependency graph, the borrower / data-collector
   profiles, and the delegated person-management authorization helpers
   (may_manage_persons / may_grant_permission / may_manage_target).
   DB-backed tests use an in-memory SQLite database via SQLQueries.
   @file
   @author Christoph Pesch
 */

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_PERMISSION_DEPS_HPP
#  include "colman_permission_deps.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif
#ifndef SYSTEM_OPTIONAL
#  include <optional>
#  define SYSTEM_OPTIONAL
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{

  /// Returns true if \a haystack contains \a needle.
  bool contains(std::vector<QString> const& haystack, QString const& needle)
  {
    return std::find(haystack.begin(), haystack.end(), needle) !=
           haystack.end();
  }

  /// Returns true if dependency list contains the pair {a, b}.
  bool has_dependency(QString const& a, QString const& b)
  {
    auto const deps = colman::get_permission_dependencies();
    for(auto const& d : deps) {
      if(d.first == a && d.second == b) {
        return true;
      }
    }
    return false;
  }

  class PermissionDepsTest : public QObject
  {
    Q_OBJECT

  public:
    PermissionDepsTest() = default;

  private:
    std::optional<colman::SQLQueries> q_;
    int conn_counter_ = 0;

    void setup_db()
    {
      auto const conn_name =
          QString("perm_deps_test_%1").arg(conn_counter_++);
      q_.emplace(conn_name);
      QVERIFY(q_->create_tables());
      QVERIFY(q_->create_collection(colman::make_test_collection()));
      QVERIFY(q_->add_administrator());
    }

    void teardown_db() { q_.reset(); }

    /// Creates a regular person and grants the given delegation right.
    int make_person_with(QString const& column)
    {
      int const id = q_->add_new_person(colman::make_test_person());
      Q_ASSERT(id > 0);
      if(!column.isEmpty()) {
        q_->set_permission(id, column, true);
      }
      return id;
    }

  private Q_SLOTS:

  // ------------------------------------------------------------------
  // Pure profile functions (no DB)
  // ------------------------------------------------------------------

    void test_borrower_profile_contents()
    {
      auto const p = colman::borrower_profile();
      QVERIFY(contains(p, "view_items"));
      QVERIFY(contains(p, "reserve"));
      // Editing other users' reservations is not part of the profile.
      QVERIFY(!contains(p, "edit_reservations"));
      QVERIFY(!contains(p, "edit_persons"));
    }

    void test_data_collector_profile_contents()
    {
      auto const p = colman::data_collector_profile();
      QVERIFY(contains(p, "collect_acquisitions"));
      QVERIFY(contains(p, "collect_liquidations"));
      QVERIFY(contains(p, "view_items"));
      QVERIFY(!contains(p, "edit_persons"));
      QVERIFY(!contains(p, "collect_lendings"));
    }

    void test_grantable_profile_maps_creator_rights()
    {
      QCOMPARE(colman::grantable_profile("create_lenders"),
               colman::borrower_profile());
      QCOMPARE(colman::grantable_profile("create_data_collectors"),
               colman::data_collector_profile());
    }

    void test_grantable_profile_empty_for_others()
    {
      // edit_persons grants everything and is handled by callers, not here.
      QVERIFY(colman::grantable_profile("edit_persons").empty());
      QVERIFY(colman::grantable_profile("view_items").empty());
      QVERIFY(colman::grantable_profile("").empty());
    }

  // ------------------------------------------------------------------
  // Dependency graph regression
  // ------------------------------------------------------------------

    void test_create_rights_do_not_imply_edit_persons()
    {
      // The old privilege-escalation dependencies must be gone.
      QVERIFY(!has_dependency("create_lenders", "edit_persons"));
      QVERIFY(!has_dependency("create_data_collectors", "edit_persons"));
    }

    void test_apply_deps_does_not_add_edit_persons()
    {
      std::vector<QString> const columns{
          "create_lenders", "edit_persons", "view_items", "view_persons"};
      std::vector<bool> perms{true, false, false, false};
      colman::apply_permission_dependencies(perms, columns);
      // create_lenders must NOT force edit_persons on (no escalation) ...
      QVERIFY(!perms[1]);
      // ... but it DOES force the view rights it needs to do its job.
      QVERIFY(perms[2]); // view_items
      QVERIFY(perms[3]); // view_persons
    }

    void test_management_rights_force_view_permissions()
    {
      setup_db();
      for(auto const& right : {QStringLiteral("edit_persons"),
                               QStringLiteral("create_lenders"),
                               QStringLiteral("create_data_collectors")}) {
        int const id = make_person_with(right);
        // Granting a management right cascades the required view rights.
        QVERIFY2(q_->has_permission(id, "view_persons"),
                 qPrintable(right));
        QVERIFY2(q_->has_permission(id, "view_items"),
                 qPrintable(right));
        // ... but never edit_persons (unless that was the right itself).
        if(right != QStringLiteral("edit_persons")) {
          QVERIFY2(!q_->has_permission(id, "edit_persons"),
                   qPrintable(right));
        }
      }
      teardown_db();
    }

  // ------------------------------------------------------------------
  // may_manage_persons
  // ------------------------------------------------------------------

    void test_admin_may_manage_persons()
    {
      setup_db();
      QVERIFY(colman::may_manage_persons(*q_, 0));
      teardown_db();
    }

    void test_regular_user_may_not_manage_persons()
    {
      setup_db();
      int const id = make_person_with("");
      QVERIFY(!colman::may_manage_persons(*q_, id));
      teardown_db();
    }

    void test_each_delegation_right_enables_management()
    {
      setup_db();
      QVERIFY(colman::may_manage_persons(*q_, make_person_with("edit_persons")));
      QVERIFY(
          colman::may_manage_persons(*q_, make_person_with("create_lenders")));
      QVERIFY(colman::may_manage_persons(
          *q_, make_person_with("create_data_collectors")));
      teardown_db();
    }

  // ------------------------------------------------------------------
  // may_grant_permission
  // ------------------------------------------------------------------

    void test_admin_may_grant_anything()
    {
      setup_db();
      // The administrator (person 0) holds every permission and may grant
      // any of them.
      QVERIFY(colman::may_grant_permission(*q_, 0, "edit_persons"));
      QVERIFY(colman::may_grant_permission(*q_, 0, "collect_acquisitions"));
      QVERIFY(colman::may_grant_permission(*q_, 0, "view_items"));
      teardown_db();
    }

    void test_edit_persons_holder_grants_only_held_permissions()
    {
      setup_db();
      // A delegated edit_persons holder may grant what they hold (edit_persons
      // itself and the cascaded view rights) but NOT permissions they lack —
      // otherwise they could escalate via a second account.
      int const id = make_person_with("edit_persons");
      QVERIFY(colman::may_grant_permission(*q_, id, "edit_persons"));
      QVERIFY(colman::may_grant_permission(*q_, id, "view_items"));
      QVERIFY(colman::may_grant_permission(*q_, id, "view_persons"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "collect_acquisitions"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "edit_program_settings"));
      // Once they actually hold a right, they may grant it.
      QVERIFY(q_->set_permission(id, "collect_acquisitions", true));
      QVERIFY(colman::may_grant_permission(*q_, id, "collect_acquisitions"));
      teardown_db();
    }

    void test_create_lenders_grants_only_borrower_profile()
    {
      setup_db();
      int const id = make_person_with("create_lenders");
      // Holds the cascaded view_items and the reserve baseline, both in the
      // borrower profile.
      QVERIFY(colman::may_grant_permission(*q_, id, "view_items"));
      QVERIFY(colman::may_grant_permission(*q_, id, "reserve"));
      // Baseline is always grantable by a manager.
      QVERIFY(colman::may_grant_permission(*q_, id, "view_personal_data"));
      // Not holding the right (anti-escalation):
      QVERIFY(!colman::may_grant_permission(*q_, id, "edit_persons"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "edit_reservations"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "collect_acquisitions"));
      // Held (cascaded) but outside the borrower profile → still not grantable.
      QVERIFY(q_->has_permission(id, "view_persons"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "view_persons"));
      teardown_db();
    }

    void test_create_data_collectors_grants_only_collector_profile()
    {
      setup_db();
      int const id = make_person_with("create_data_collectors");
      // The creator must also hold the collect rights to delegate them
      // (anti-escalation); a senior collector who can also create collectors.
      QVERIFY(q_->set_permission(id, "collect_acquisitions", true));
      QVERIFY(q_->set_permission(id, "collect_liquidations", true));
      QVERIFY(colman::may_grant_permission(*q_, id, "collect_acquisitions"));
      QVERIFY(colman::may_grant_permission(*q_, id, "collect_liquidations"));
      QVERIFY(colman::may_grant_permission(*q_, id, "view_items"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "edit_persons"));
      // Held (cascaded) but outside the collector profile.
      QVERIFY(!colman::may_grant_permission(*q_, id, "view_persons"));
      teardown_db();
    }

    void test_create_data_collectors_cannot_grant_unheld_collect_right()
    {
      setup_db();
      // A create_data_collectors holder who does NOT themselves do
      // acquisitions cannot delegate that right.
      int const id = make_person_with("create_data_collectors");
      QVERIFY(!q_->has_permission(id, "collect_acquisitions"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "collect_acquisitions"));
      teardown_db();
    }

    void test_non_manager_may_not_grant_baseline()
    {
      setup_db();
      int const id = make_person_with("");
      QVERIFY(!colman::may_grant_permission(*q_, id, "view_personal_data"));
      QVERIFY(!colman::may_grant_permission(*q_, id, "view_items"));
      teardown_db();
    }

  // ------------------------------------------------------------------
  // may_manage_target (containment invariant)
  // ------------------------------------------------------------------

    void test_admin_may_manage_anyone()
    {
      setup_db();
      int const staff = make_person_with("edit_persons");
      QVERIFY(colman::may_manage_target(*q_, 0, staff));
      teardown_db();
    }

    void test_create_lenders_may_manage_fresh_and_borrower_targets()
    {
      setup_db();
      int const creator = make_person_with("create_lenders");
      // A freshly created person has only baseline rights.
      int const fresh = make_person_with("");
      QVERIFY(colman::may_manage_target(*q_, creator, fresh));
      // Granting a borrower-profile right keeps it manageable.
      q_->set_permission(fresh, "view_items", true);
      QVERIFY(colman::may_manage_target(*q_, creator, fresh));
      teardown_db();
    }

    void test_create_lenders_may_not_manage_out_of_profile_target()
    {
      setup_db();
      int const creator = make_person_with("create_lenders");
      int const target = make_person_with("");
      // edit_reservations is outside the borrower profile.
      q_->set_permission(target, "edit_reservations", true);
      QVERIFY(!colman::may_manage_target(*q_, creator, target));
      teardown_db();
    }

    void test_create_lenders_may_not_manage_staff_or_admin()
    {
      setup_db();
      int const creator = make_person_with("create_lenders");
      int const staff = make_person_with("edit_persons");
      QVERIFY(!colman::may_manage_target(*q_, creator, staff));
      // The administrator (id 0) is never manageable by a delegate.
      QVERIFY(!colman::may_manage_target(*q_, creator, 0));
      teardown_db();
    }

    void test_create_data_collectors_containment()
    {
      setup_db();
      int const creator = make_person_with("create_data_collectors");
      // The creator must also do acquisitions to manage a collector who does
      // (anti-escalation: otherwise they could reset that collector's password
      // and log in to gain the right).
      q_->set_permission(creator, "collect_acquisitions", true);
      int const collector = make_person_with("");
      q_->set_permission(collector, "collect_acquisitions", true);
      q_->set_permission(collector, "view_items", true);
      QVERIFY(colman::may_manage_target(*q_, creator, collector));
      // A borrower with edit_reservations is outside the collector profile.
      int const borrower = make_person_with("");
      q_->set_permission(borrower, "edit_reservations", true);
      QVERIFY(!colman::may_manage_target(*q_, creator, borrower));
      // A collector with a collect right the creator lacks is unmanageable.
      int const senior = make_person_with("");
      q_->set_permission(senior, "collect_liquidations", true);
      QVERIFY(!colman::may_manage_target(*q_, creator, senior));
      teardown_db();
    }

    void test_creator_with_both_rights_covers_union()
    {
      setup_db();
      int const creator = make_person_with("create_lenders");
      q_->set_permission(creator, "create_data_collectors", true);
      // Holds the rights it intends to delegate.
      q_->set_permission(creator, "collect_acquisitions", true);
      int const target = make_person_with("");
      q_->set_permission(target, "view_items", true);
      q_->set_permission(target, "collect_acquisitions", true);
      // Spans both profiles; manageable because the creator holds both the
      // rights and the matching delegation profiles.
      QVERIFY(colman::may_manage_target(*q_, creator, target));
      teardown_db();
    }

    void test_regular_user_may_not_manage_target()
    {
      setup_db();
      int const id = make_person_with("");
      int const other = make_person_with("");
      QVERIFY(!colman::may_manage_target(*q_, id, other));
      teardown_db();
    }

  }; // class PermissionDepsTest

} // anonymous namespace

QTEST_GUILESS_MAIN(PermissionDepsTest)
#include "colman_permission_deps_test.moc"

/* EOF */
