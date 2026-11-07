// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class ClearSelectionFilter.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_clear_selection_filter.hpp"

/**
   Constructs a ClearSelectionFilter that monitors the given table view.
   @param view the table view whose selection is managed by this filter.
*/
colman::ClearSelectionFilter::ClearSelectionFilter(QTableView* view)
    : QObject(nullptr),
      view_(view)
{
}


/**
   Intercepts mouse press events and clears the table selection when the
   user clicks on an empty area (no valid index under the cursor).
   @param obj   object that generated the event.
   @param event the event to filter.
   @return true if the event was consumed; false to pass it on.
*/
bool colman::ClearSelectionFilter::eventFilter(QObject* obj, QEvent* event)
{
  if((obj) && (event) && event->type() == QEvent::MouseButtonPress) {
    QMouseEvent* me = static_cast<QMouseEvent*>(event);
    if(!view_->indexAt(me->pos()).isValid()) {
      if(auto* sm = view_->selectionModel()) {
        sm->clearSelection();
      }
      return true;
    }
  }
  return QObject::eventFilter(obj, event);
}

/* EOF */
