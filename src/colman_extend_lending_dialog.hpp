// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// dialog to extend the lending period of open lendings
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_EXTEND_LENDING_DIALOG_HPP
#  define COLMAN_EXTEND_LENDING_DIALOG_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef COLMAN_TABLE_MODELS_HPP
#    include "colman_table_models.hpp"
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif

namespace colman
{
  /// Returns true if the current user may extend lending periods.
  bool may_extend_lendings(AppData& data);

  /// Lets the user choose open lendings of a person and set a new due date.
  /**
     Used where only the borrower is known (lending wizard, step 1): lists
     all open lendings of @p person_id, all preselected; the new due date
     defaults to today plus the collection's default lending period.
     @param parent    parent widget of the dialog.
     @param data      central application state.
     @param person_id borrower whose open lendings are listed.
     @return true if at least one lending was changed.
  */
  bool extend_lendings_of_person(QWidget* parent, AppData& data,
                                 int person_id);

  /// Sets a new due date for lendings the caller has already chosen.
  /**
     Used where the lendings are known (open-lendings page, return wizard):
     shows them as read-only overview and asks only for the new due date.
     @param parent parent widget of the dialog.
     @param data   central application state.
     @param rows   lendings to change.
     @return true if the lendings were changed.
  */
  bool extend_selected_lendings(
      QWidget* parent, AppData& data,
      std::vector<DueLendingListModel::Row> const& rows);
} // namespace colman

#endif /* #ifndef COLMAN_EXTEND_LENDING_DIALOG_HPP */

/* EOF */
