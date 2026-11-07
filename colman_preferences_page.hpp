// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// setup of preferences page
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PREFERENCES_PAGE_HPP
#  define COLMAN_PREFERENCES_PAGE_HPP

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
     Initializes the preferences page; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_preferences_page(QApplication& app, colman::AppData& data);
  /// Populates all program-settings widgets from QSettings.
  void enter_preferences_tab(colman::AppData& data);
  /// Returns true if any preference widget differs from the stored value.
  bool preferences_tab_changed(colman::AppData& data);
  /// Writes the current preference-widget values to QSettings.
  void apply_preferences_tab(colman::AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_PREFERENCES_PAGE_HPP */

/* EOF */
