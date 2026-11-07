// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class management page setup functions.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CLASS_MANAGEMENT_PAGE_HPP
#  define COLMAN_CLASS_MANAGEMENT_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif

namespace colman
{
  /// Sets up the three class management pages (main, school-year planning,
  /// review/execute) and wires the class management button.
  /** Must be called once at startup inside the not_first_start block,
      after the central page button has been created.
      @param app  Qt application object
      @param data central application state */
  void setup_class_management_pages(QApplication& app, AppData& data);

  /// Switches to the class management main page.
  /** Reloads the class list and grade range from the database.
      @param data central application state */
  void enter_class_management_page(AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_CLASS_MANAGEMENT_PAGE_HPP */

/* EOF */
