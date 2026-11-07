// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of setup_central_page().
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CENTRAL_PAGE_HPP
#  define COLMAN_CENTRAL_PAGE_HPP

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
     Replaces the UI-generated central_page_ with a C++-created widget
     and sets up all central-page signal/slot connections.  To be called
     once at application startup, before any collection is opened.
     @param app  Qt application object.
     @param data central application state.
  */
  void setup_central_page(QApplication& app, colman::AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_CENTRAL_PAGE_HPP */

/* EOF */
