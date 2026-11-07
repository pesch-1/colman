// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// page for lendings
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_LENDING_PAGE_HPP
#  define COLMAN_LENDING_PAGE_HPP

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
     Sets up the lending pages; to be called once at program start.
     @param app Qt application object.
     @param data central application state.
  */
  void setup_lending_pages(QApplication& app, AppData& data);
  /**
     Switches the view to the lend page.
     @param data central application state.
  */
  void enter_lend_page(AppData& data);
  /**
     Refreshes the lending pages after an undo or redo operation.
     @param data central application state.
  */
  void refresh_lending_pages(AppData& data);
  /**
     Switches to the lending wizard and pre-selects from \a ai:
     - item_id >= 0: the barcode field on page 2 is pre-filled and the
       item row is pre-selected; the wizard starts at page 1 so the user
       can still choose a person before advancing.
     - person_id >= 0: the person is pre-selected on page 1.
     @param data  central application state.
     @param ai    Lending action produced by possible_actions().
  */
  void enter_lend_page_for_action(AppData& data,
                                  ActionInfo const& ai);
  /**
     Switches to the lending wizard for a reservation, pre-selecting
     \a item_id.  With the "edit_reservations" permission the wizard starts
     at page 1 (person selection); page 2 is skipped because the item is
     already selected and Next jumps directly to page 3.  Without that
     permission the wizard opens directly on page 3 with the current user
     and the item pre-selected.  Page 3 opens with the reserve radio
     pre-selected in both cases.
     @param data    central application state.
     @param item_id the item to reserve.
  */
  void enter_reserve_page_for_item(AppData& data, int item_id);

} // namespace colman

#endif /* #ifndef COLMAN_LENDING_PAGE_HPP */

/* EOF */
