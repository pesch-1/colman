// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
///
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ACTIONS_HPP
#  define COLMAN_ACTIONS_HPP

#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QSQLDATABASE
#    include <QSqlDatabase>
#    define SYSTEM_QSQLDATABASE
#  endif
namespace colman
{
  enum struct Action {
    None,
    Acquisition,
    Liquidation,
    Lending,
    Return
  };


  struct ActionInfo
  {
    Action action_ = Action::None;
    int item_id_ = -1;
    int acquisition_id_ = -1;
    int lending_id_ = -1;
    int person_id_ = -1;
    QString barcode_; ///< scanned barcode; set for Acquisition actions
  };


  /// Prints a human-readable representation of \a actions to stdout.
  void print_actions(std::vector<ActionInfo> const& actions);

  /// Returns the list of possible actions for \a barcode against \a db.
  /**
   * Identifies the barcode type (item, person, or ISBN) and queries the
   * database to determine which actions (Acquisition, Lending, Return) are
   * applicable.
   * @param queries database query object
   * @param barcode scanned barcode string
   * @return        list of possible actions; empty only on database error
   */
  std::vector<ActionInfo> possible_actions(IQueries& queries,
                                           QString const& barcode);

} // namespace colman

#endif /* #ifndef COLMAN_ACTIONS_HPP */

/* EOF */
