// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// the liquidation wizard
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_LIQUIDATION_PAGE_HPP
#  define COLMAN_LIQUIDATION_PAGE_HPP

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
     Sets up the liquidation pages; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_liquidation_pages(QApplication& app, AppData& data);
  /**
     Switches the view to the liquidation page.
     @param data central application state.
  */
  void enter_liquidation_page(AppData& data);
  /**
     Refreshes the liquidation pages after an undo or redo operation.
     @param data central application state.
  */
  void refresh_liquidation_pages(AppData& data);
  /**
     Switches to the liquidation wizard and pre-selects from \a ai.
     Selects the item on page 1 and advances directly to page 2.
     @param data  central application state.
     @param ai    Liquidation action produced by possible_actions().
  */
  void enter_liquidation_page_for_action(AppData& data,
                                         ActionInfo const& ai);

} // namespace colman

#endif /* #ifndef COLMAN_LIQUIDATION_PAGE_HPP */

/* EOF */
