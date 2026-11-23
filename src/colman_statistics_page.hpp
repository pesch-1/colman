// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Setup of the statistics page.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_STATISTICS_PAGE_HPP
#  define COLMAN_STATISTICS_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif

namespace colman
{
  /// Sets up the statistics page.  To be called once at program start.
  void setup_statistics_page(QApplication& app, colman::AppData& data);
} // namespace colman

#endif /* #ifndef COLMAN_STATISTICS_PAGE_HPP */

/* EOF */
