// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Permission dependency list and enforcement logic.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_permission_deps.hpp"

#include "colman_iqueries.hpp"

#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

/// Returns the list of permission dependencies.
/**
 * Each entry {A, B} means "if A is granted, B must also be granted".
 * Add new entries here to extend the dependency graph; the rest of the
 * infrastructure (UI cascades, database migration) picks them up
 * automatically.
 */
std::vector<std::pair<QString, QString>>
colman::get_permission_dependencies()
{
  return {
      // Reserving items requires seeing the item list.
      {"reserve", "view_items"},
      // Editing reservations requires the basic reserve permission.
      {"edit_reservations", "reserve"},
      // Person-management rights require seeing the person and item lists.
      // This is both a functional prerequisite (the management UI needs
      // those views) and what keeps the "you may only grant permissions you
      // hold yourself" principle workable: a manager always holds view_items
      // / view_persons and can therefore grant them.  These rights must NOT
      // imply edit_persons — that would be a privilege escalation; the
      // restricted scope is enforced via grantable_profile() /
      // may_manage_target().
      {"edit_persons", "view_persons"},
      {"edit_persons", "view_items"},
      {"create_lenders", "view_persons"},
      {"create_lenders", "view_items"},
      {"create_data_collectors", "view_persons"},
      {"create_data_collectors", "view_items"},
      // Recording a lending requires seeing both items and persons.
      {"collect_lendings", "view_items"},
      {"collect_lendings", "view_persons"},
      // Recording a return requires seeing items and persons.
      {"collect_returns", "view_items"},
      {"collect_returns", "view_persons"},
      // Printing ID cards requires seeing the person list.
      {"print_ids", "view_persons"},
      // Printing barcodes requires seeing the item list.
      {"print_barcodes", "view_items"},
  };
}

/// Applies all permission dependencies to \a perms until stable.
/**
 * Iterates get_permission_dependencies() in a fixed-point loop.
 * On each pass, any dependency {A, B} where A is true but B is false
 * causes B to be set to true.  Repeats until no more changes occur,
 * which correctly handles transitive chains (A→B→C).
 *
 * @param perms   permission flags in the order of \a columns
 * @param columns permission column names in the same order as \a perms
 */
void colman::apply_permission_dependencies(
    std::vector<bool>& perms, std::vector<QString> const& columns)
{
  auto const deps = get_permission_dependencies();
  bool changed = true;
  while(changed) {
    changed = false;
    for(auto const& dep : deps) {
      // Find indices of A and B.
      int idx_a = -1;
      int idx_b = -1;
      auto const n = static_cast<int>(columns.size());
      for(int i = 0; i < n; ++i) {
        if(columns[i] == dep.first) {
          idx_a = i;
        }
        if(columns[i] == dep.second) {
          idx_b = i;
        }
      }
      if(idx_a < 0 || idx_b < 0) {
        continue;
      }
      if(perms[idx_a] && !perms[idx_b]) {
        perms[idx_b] = true;
        changed = true;
      }
    }
  }
}

QSet<QString> colman::permission_cascade(QString const& column, bool value)
{
  auto const deps = get_permission_dependencies();
  QSet<QString> affected{column};
  bool found_more = true;
  while(found_more) {
    found_more = false;
    for(auto const& dep : deps) {
      if(value) {
        // affected requires dep.second → dep.second must also be set true.
        if(affected.contains(dep.first) && !affected.contains(dep.second)) {
          affected.insert(dep.second);
          found_more = true;
        }
      }
      else {
        // dep.first requires affected → dep.first must also be set false.
        if(affected.contains(dep.second) && !affected.contains(dep.first)) {
          affected.insert(dep.first);
          found_more = true;
        }
      }
    }
  }
  return affected;
}

// -------------------------------------------------------------------------
// Permission profiles for delegated person creation
// -------------------------------------------------------------------------

std::vector<QString> colman::borrower_profile()
{
  // reserve covers creating and cancelling one's own reservations.
  return {QStringLiteral("view_items"), QStringLiteral("reserve")};
}

std::vector<QString> colman::data_collector_profile()
{
  return {QStringLiteral("collect_acquisitions"),
          QStringLiteral("collect_liquidations"),
          QStringLiteral("view_items")};
}

std::vector<QString> colman::grantable_profile(QString const& creator_column)
{
  if(creator_column == QStringLiteral("create_lenders")) {
    return borrower_profile();
  }
  if(creator_column == QStringLiteral("create_data_collectors")) {
    return data_collector_profile();
  }
  return {};
}

namespace
{

  /// Returns true for permissions every account holds by default.
  /**
   * create_default_permissions() grants view_personal_data and reserve to
   * every account (admin and non-admin alike).  These baseline rights are
   * therefore always grantable by any person manager and are ignored by the
   * containment check, mirroring is_reserve_only().
   */
  bool is_baseline_permission(QString const& column)
  {
    return column == QStringLiteral("view_personal_data") ||
           column == QStringLiteral("reserve");
  }

  /// Returns the union of all profiles \a caller_id is allowed to grant.
  /**
   * Empty when the caller holds no delegation right.  edit_persons is handled
   * separately by the callers and is not reflected here.
   */
  std::vector<QString> caller_grantable_columns(colman::IQueries& queries,
                                                int caller_id)
  {
    std::vector<QString> columns;
    auto const append_profile = [&columns](std::vector<QString> const& p) {
      for(auto const& c : p) {
        if(std::find(columns.begin(), columns.end(), c) == columns.end()) {
          columns.push_back(c);
        }
      }
    };
    if(queries.has_permission(caller_id,
                              QStringLiteral("create_lenders"))) {
      append_profile(colman::borrower_profile());
    }
    if(queries.has_permission(caller_id,
                              QStringLiteral("create_data_collectors"))) {
      append_profile(colman::data_collector_profile());
    }
    return columns;
  }

} // namespace

// -------------------------------------------------------------------------
// Delegated person-management authorization
// -------------------------------------------------------------------------

bool colman::may_manage_persons(IQueries& queries, int caller_id)
{
  return queries.has_permission(caller_id,
                                QStringLiteral("edit_persons")) ||
         queries.has_permission(caller_id,
                                QStringLiteral("create_lenders")) ||
         queries.has_permission(
             caller_id, QStringLiteral("create_data_collectors"));
}

bool colman::may_grant_permission(IQueries& queries, int caller_id,
                                  QString const& column)
{
  // Anti-escalation: never grant a permission the caller does not hold.
  // Otherwise a manager could escalate by creating a second account with
  // rights they lack and then logging in as that account.  This applies to
  // edit_persons holders too; only the administrator holds every permission.
  if(!queries.has_permission(caller_id, column)) {
    return false;
  }
  // edit_persons holders may grant any right they hold to any target.
  if(queries.has_permission(caller_id, QStringLiteral("edit_persons"))) {
    return true;
  }
  // Baseline rights may be granted by any person manager.
  if(is_baseline_permission(column)) {
    return may_manage_persons(queries, caller_id);
  }
  // Delegated creators may grant only rights within their target profile.
  auto const grantable = caller_grantable_columns(queries, caller_id);
  return std::find(grantable.begin(), grantable.end(), column) !=
         grantable.end();
}

bool colman::may_manage_target(IQueries& queries, int caller_id,
                               int target_id)
{
  if(!may_manage_persons(queries, caller_id)) {
    return false;
  }
  auto const columns = queries.get_permissions();
  auto const caller_perms = queries.get_user_permissions(caller_id);
  auto const target_perms = queries.get_user_permissions(target_id);

  // Collect the columns the caller holds, and whether they are a full
  // (edit_persons) manager.
  QSet<QString> caller_held;
  bool caller_full = false;
  {
    auto const n = std::min(columns.size(), caller_perms.size());
    for(std::size_t i = 0; i < n; ++i) {
      if(caller_perms[i]) {
        caller_held.insert(columns[i]);
        if(columns[i] == QStringLiteral("edit_persons")) {
          caller_full = true;
        }
      }
    }
  }
  auto const grantable = caller_grantable_columns(queries, caller_id);

  // Containment + anti-escalation: the caller must be able to grant every
  // (non-baseline) permission the target currently holds — i.e. hold it
  // themselves AND be allowed to delegate it (edit_persons, or within their
  // delegated profile).  Besides preventing a downgrade of higher-privileged
  // accounts, this closes the credential-reset escalation: a manager can only
  // reset passwords of accounts whose rights are a subset of their own.
  auto const n = std::min(columns.size(), target_perms.size());
  for(std::size_t i = 0; i < n; ++i) {
    if(!target_perms[i] || is_baseline_permission(columns[i])) {
      continue;
    }
    if(!caller_held.contains(columns[i])) {
      return false;
    }
    if(!caller_full &&
       std::find(grantable.begin(), grantable.end(), columns[i]) ==
           grantable.end()) {
      return false;
    }
  }
  return true;
}

/* EOF */
