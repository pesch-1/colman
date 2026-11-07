// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// setup of settings page
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SETTINGS_PAGE_HPP
#  define COLMAN_SETTINGS_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif

namespace colman
{
  /// Initializes the settings page.  To be called at program start.
  void setup_settings_page(QApplication& app, colman::AppData& data);
  /// Populates all collection-settings widgets from the database.
  void enter_settings_tab(colman::AppData& data);
  /// Returns true if any settings widget differs from the stored value.
  bool settings_tab_changed(colman::AppData& data);
  /// Writes the current settings-widget values to the database.
  void apply_settings_tab(colman::AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_SETTINGS_PAGE_HPP */

/* EOF */
