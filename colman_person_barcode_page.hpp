// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// setup of person barcode page
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERSON_BARCODE_PAGE_HPP
#  define COLMAN_PERSON_BARCODE_PAGE_HPP

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
     Sets up the person barcode page; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_person_barcode_page(QApplication& app, colman::AppData& data);
  /**
     Refreshes the person barcode page after an undo or redo operation.
     @param data central application state.
  */
  void refresh_person_barcode_page(colman::AppData& data);

} // namespace colman

#endif /* #ifndef COLMAN_PERSON_BARCODE_PAGE_HPP */

/* EOF */
