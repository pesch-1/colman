// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// pages for group lendings
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_GROUP_LENDING_PAGE_HPP
#  define COLMAN_GROUP_LENDING_PAGE_HPP

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
     Sets up the group lending pages; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_group_lending_pages(QApplication& app, AppData& data);
  /**
     Resets the group lending wizard and shows its item page.
     @param data central application state.
  */
  void enter_group_lending_page(AppData& data);
  /**
     Refreshes the group lending pages after an undo or redo operation.
     @param data central application state.
  */
  void refresh_group_lending_pages(AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_GROUP_LENDING_PAGE_HPP */

/* EOF */
