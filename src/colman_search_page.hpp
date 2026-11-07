// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of setup_search_page() and enter_search_page().
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SEARCH_PAGE_HPP
#  define COLMAN_SEARCH_PAGE_HPP

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
     Initializes the search page; to be called once at program start.
     @param app  Qt application object.
     @param data central application state.
  */
  void setup_search_page(QApplication& app, colman::AppData& data);
} // namespace colman

#endif /* #ifndef COLMAN_SEARCH_PAGE_HPP */

/* EOF */
