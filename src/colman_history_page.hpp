// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// setup of history page
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_HISTORY_PAGE_HPP
#  define COLMAN_HISTORY_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif

namespace colman
{
  /// Sets up the history page.  To be called at program start.
  void setup_history_page(QApplication& app, colman::AppData& data);
  /// refreshes history page (after undo or redo)
  void refresh_history_page(colman::AppData& data);
} // namespace colman

#endif /* #ifndef COLMAN_HISTORY_PAGE_HPP */

/* EOF */
