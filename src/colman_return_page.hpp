// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// page for returns
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_RETURN_PAGE_HPP
#  define COLMAN_RETURN_PAGE_HPP

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
     Sets up the return pages; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_return_pages(QApplication& app, AppData& data);
  /**
     Switches the view to the return page.
     @param data central application state.
  */
  void enter_return_page(AppData& data);
  /**
     Refreshes the return pages after an undo or redo operation.
     @param data central application state.
  */
  void refresh_return_pages(AppData& data);
  /**
     Switches to the return wizard and pre-selects from \a ai.
     Pre-fills the barcode field and selects the item on page 1, selects
     the person on page 2, then advances directly to page 3 (lending
     selection).  If \a ai.lending_id_ >= 0 the lending row is selected
     and the wizard advances to page 4 (return details).
     @param data  central application state.
     @param ai    Return action produced by possible_actions().
  */
  void enter_return_page_for_action(AppData& data,
                                    ActionInfo const& ai);

} // namespace colman

#endif /* #ifndef COLMAN_RETURN_PAGE_HPP */

/* EOF */
