// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PersonViewPage.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERSON_VIEW_PAGE_HPP
#  define COLMAN_PERSON_VIEW_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif

namespace colman
{
  /**
     Initializes the person view page (read-only detail view); to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_person_view_page(QApplication& app, colman::AppData& data);
  /**
     Refreshes the person view page after an undo or redo operation.
     @param data central application state.
  */
  void refresh_person_view_page(colman::AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_PERSON_VIEW_PAGE_HPP */

/* EOF */
