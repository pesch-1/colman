// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Helper functions for QTableView selection and scrolling.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  define COLMAN_TABLEVIEW_HELPERS_HPP

#  ifndef SYSTEM_QTABLEVIEW
#    include <QTableView>
#    define SYSTEM_QTABLEVIEW
#  endif
#  ifndef SYSTEM_SET
#    include <set>
#    define SYSTEM_SET
#  endif

namespace colman
{
  /// select row which contains certain id in given column
  bool select_and_scroll_to(QTableView* table, int column, int id);
  /// returns selected id (in hidden column 0) or -1 if no row is selected
  int selected_id(QTableView* table);
  /// returns selected ids (in hidden column 0)
  std::set<int> selected_ids(QTableView* view);
  /// select row, iff only one row is present, returns associated id
  int select_if_single_row(QTableView* table);
  /// returns true, iff only one row is present.
  bool single_row(QTableView* table);
} // namespace colman

#endif /* #ifndef COLMAN_TABLEVIEW_HELPERS_HPP */

/* EOF */
