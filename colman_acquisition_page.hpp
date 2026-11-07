// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// the acquisition wizard
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ACQUISITION_PAGE_HPP
#  define COLMAN_ACQUISITION_PAGE_HPP

#  ifndef COLMAN_ACTIONS_HPP
#    include "colman_actions.hpp"
#  endif
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
     Sets up the acquisition pages; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_acquisition_pages(QApplication& app, AppData& data);
  /**
     Switches the view to the acquisition page.
     @param data central application state.
  */
  void enter_acquisition_page(AppData& data);
  /**
     Refreshes the acquisition pages after an undo or redo operation.
     @param data central application state.
  */
  void refresh_acquisition_pages(AppData& data);
  /**
     Switches to the acquisition wizard pre-populated from \a ai.
     Skips page 1 (type selection) by advancing directly to page 2, using the
     item's type when item_id >= 0, or the collection default otherwise.
     Places \a ai.barcode_ in the page-2 search field and triggers the search,
     so that an internet look-up fires automatically for unknown ISBNs.
     @param data  central application state.
     @param ai    Acquisition action produced by possible_actions().
  */
  void enter_acquisition_page_for_action(AppData& data,
                                         ActionInfo const& ai);

} // namespace colman

#endif /* #ifndef COLMAN_ACQUISITION_PAGE_HPP */

/* EOF */
