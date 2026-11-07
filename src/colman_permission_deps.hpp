// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Permission dependency declarations.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERMISSION_DEPS_HPP
#  define COLMAN_PERMISSION_DEPS_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QSET
#    include <QSet>
#    define SYSTEM_QSET
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_UTILITY
#    include <utility>
#    define SYSTEM_UTILITY
#  endif

namespace colman
{

  class IQueries;

  /// Returns the list of permission dependencies.
  /**
   * Each pair {A, B} means: if permission A is granted, B must also be
   * granted.  The list is the single source of truth for all dependency
   * enforcement — UI cascades and database migration both read it here.
   *
   * To add a new dependency, append a pair to the vector returned by
   * the implementation in colman_permission_deps.cpp.  Transitivity is
   * handled automatically by apply_permission_dependencies().
   */
  std::vector<std::pair<QString, QString>> get_permission_dependencies();

  /// Applies permission dependencies to a flat permission vector.
  /**
   * Iterates the dependency list repeatedly until the vector is stable.
   * Only adds missing prerequisites; never removes existing permissions.
   * Handles transitive dependencies (A→B→C) automatically.
   *
   * @param perms   permission flags, in the order of \a columns
   * @param columns permission column names, in the same order as \a perms
   */
  void apply_permission_dependencies(std::vector<bool>& perms,
                                     std::vector<QString> const& columns);

  /// Columns that must also be set to \a value when \a column is set so.
  /**
   * Computes the transitive closure of the dependency graph for a single
   * checkbox/flag change, so the invariant from get_permission_dependencies()
   * holds no matter where a permission is changed (UI cascade, REST, SQL):
   *  - when \a value is true: \a column plus every permission it (transitively)
   *    requires;
   *  - when \a value is false: \a column plus every permission that
   *    (transitively) requires it.
   * The returned set always contains \a column itself.
   * @param column the permission column being changed
   * @param value  the new value for \a column
   * @return all columns that must take the value \a value
   */
  QSet<QString> permission_cascade(QString const& column, bool value);

  // ---------------------------------------------------------------------
  // Permission profiles for delegated person creation
  // ---------------------------------------------------------------------

  /// Permission columns a "borrower" (lender) account may hold.
  /**
   * A borrower may view items and reserve items for themselves, which
   * includes cancelling their own reservations.  Editing other users'
   * reservations (edit_reservations) is deliberately excluded.
   */
  std::vector<QString> borrower_profile();

  /// Permission columns a "data collector" account may hold.
  /**
   * A data collector may record acquisitions and liquidations and view the
   * item list needed to do so.
   */
  std::vector<QString> data_collector_profile();

  /// Permission columns the delegation right \a creator_column may grant.
  /**
   * Returns the borrower profile for "create_lenders", the data-collector
   * profile for "create_data_collectors", and an empty vector for any other
   * column (including "edit_persons", which grants everything and is handled
   * by the caller).  view_personal_data is a baseline permission held by
   * every account and is therefore not listed here.
   */
  std::vector<QString> grantable_profile(QString const& creator_column);

  // ---------------------------------------------------------------------
  // Delegated person-management authorization
  // ---------------------------------------------------------------------

  /// Returns true if \a caller_id may create or edit persons at all.
  /**
   * True when the caller holds edit_persons, create_lenders, or
   * create_data_collectors.
   * @param queries   the query facade used to read the caller's permissions
   * @param caller_id the acting person
   */
  bool may_manage_persons(IQueries& queries, int caller_id);

  /// Returns true if \a caller_id may grant permission \a column to someone.
  /**
   * edit_persons holders may grant any permission.  Delegated creators may
   * grant only permissions within their profile, plus the view_personal_data
   * baseline.
   * @param queries   the query facade used to read the caller's permissions
   * @param caller_id the acting person
   * @param column    the permission column to be granted
   */
  bool may_grant_permission(IQueries& queries, int caller_id,
                            QString const& column);

  /// Returns true if \a caller_id may create or edit the person \a target_id.
  /**
   * edit_persons holders may manage anyone.  Delegated creators may manage
   * only targets whose current permission set lies entirely within the union
   * of the profiles the caller is allowed to grant (the containment
   * invariant), preventing them from editing staff or administrator accounts.
   * @param queries   the query facade used to read both persons' permissions
   * @param caller_id the acting person
   * @param target_id the person to be created or edited
   */
  bool may_manage_target(IQueries& queries, int caller_id, int target_id);

} // namespace colman

#endif /* #ifndef COLMAN_PERMISSION_DEPS_HPP */

/* EOF */
