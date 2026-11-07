// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// item barcode page
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ITEM_BARCODE_PAGE_HPP
#  define COLMAN_ITEM_BARCODE_PAGE_HPP

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
     Sets up the item barcode page; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_item_barcode_page(QApplication& app, colman::AppData& data);
  /**
     Refreshes the item barcode page after an undo or redo operation.
     @param data central application state.
  */
  void refresh_item_barcode_page(colman::AppData& data);

  /// Navigates to the item barcode page with @p item_id pre-checked.
  /**
     Convenience entry point for post-acquisition / post-action flows that
     want the user to land directly on the item barcode page with a specific
     item already checked, ready to print.
     @param data    central application state.
     @param item_id ID of the item to pre-check; ignored when < 1.
     @param back    invoked when the user leaves the page via its back
                    button; navigates back to the calling page.
  */
  void enter_item_barcode_page_for_item(colman::AppData& data, int item_id,
                                        colman::AppData::AppFunction back);

} // namespace colman

#endif /* #ifndef COLMAN_ITEM_BARCODE_PAGE_HPP */

/* EOF */
