// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// start page (open, open recent or create new)
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_START_PAGE_HPP
#  define COLMAN_START_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif


namespace colman
{
  /**
     Initializes the start page; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
     @param collection_file collection file to open (may be empty).
  */
  void setup_start_page(QApplication& app, colman::AppData& data,
                        QString const& collection_file);

  /**
     Updates the status bar to show the current login name.
     @param data central application state.
  */
  void update_login_status(colman::AppData& data);

  /**
     Logs out the current user: resets current_data_collector_ to -1,
     clears the status bar, and switches to the login selection page
     (or the REST re-login page if present).
     @param data central application state.
  */
  void logout(colman::AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_START_PAGE_HPP */

/* EOF */
