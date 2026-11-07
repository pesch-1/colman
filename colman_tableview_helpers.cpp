// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of tableview helper functions.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_tableview_helpers.hpp"
#ifndef SYSTEM_QITEMSELECTIONMODEL
#  include <QItemSelectionModel>
#  define SYSTEM_QITEMSELECTIONMODEL
#endif
#ifndef SYSTEM_QABSTRACTITEMMODEL
#  include <QAbstractItemModel>
#  define SYSTEM_QABSTRACTITEMMODEL
#endif
#ifndef SYSTEM_QDEBUG
#  include <QDebug>
#  define SYSTEM_QDEBUG
#endif

/**
   Selects the row containing \a id, if found.
   Deselects everything, if \a id was not found or is < 0.
   @return true, iff \a id was found
*/
bool colman::select_and_scroll_to(QTableView* table, int column, int id)
{
  if((table)) {
    if(id >= 0) {
      // first we try the already selected item:
      QItemSelectionModel const* smodel = table->selectionModel();
      if((smodel) && smodel->hasSelection()) {
        auto const rows = smodel->selectedRows(column);
        if(!rows.empty()) {
          auto const index = rows.front();
          if(index.data(Qt::DisplayRole).toInt() == id) {
            table->selectRow(index.row());
            table->showRow(index.row());
            table->scrollTo(index, QAbstractItemView::EnsureVisible);
            table->update();
            return true;
          }
        }
      }
      QAbstractItemModel const* model = table->model();
      if((model)) {
        int const rows = model->rowCount();
        // now we search the complete table:
        for(int r = 0; r < rows; ++r) {
          QModelIndex index = model->index(r, column);
          if(model->data(index).toInt() == id) {
            table->selectRow(index.row());
            table->showRow(index.row());
            table->scrollTo(index, QAbstractItemView::EnsureVisible);
            table->update();
            return true;
          }
        }
      }
    }
    table->clearSelection();
  }
  return false;
}


/**
   Returns all selected ids from the hidden column 0.
*/
std::set<int> colman::selected_ids(QTableView* view)
{
  std::set<int> pids;
  if((view)) {
    auto* model = view->selectionModel();
    if((model) && model->hasSelection()) {
      auto const rows = model->selectedRows();
      for(auto const& row : rows) {
        pids.insert(row.data(Qt::DisplayRole).toInt());
      }
    }
  }
  return pids;
}


/**
   Returns the selected id from the hidden column 0, or -1 if nothing is selected.
*/
int colman::selected_id(QTableView* view)
{
  if((view)) {
    auto* model = view->selectionModel();
    if((model) && model->hasSelection()) {
      auto const rows = model->selectedRows();
      if(!rows.empty()) {
        auto const index = rows.front();
        return index.data(Qt::DisplayRole).toInt();
      }
    }
  }
  return -1;
}


/**
   Returns true if the table contains exactly one row.
*/
bool colman::single_row(QTableView* table)
{
  if((table)) {
    auto* model = table->model();
    if((model) && model->rowCount() == 1) {
      return true;
    }
  }
  return false;
}


/**
   Selects the single row if exactly one exists, returning its id; returns -1 otherwise.
*/
int colman::select_if_single_row(QTableView* table)
{
  if((table)) {
    auto* model = table->model();
    if((model)) {
      if(model->rowCount() == 1) {
        QModelIndex left_index = model->index(0, 0);
        QModelIndex right_index = model->index(0, model->columnCount() - 1);
        QItemSelection selection(left_index, right_index);
        auto* smodel = table->selectionModel();
        smodel->select(selection, QItemSelectionModel::Select);
        return left_index.data(Qt::DisplayRole).toInt();
      }
    }
  }
  return -1;
}

/* EOF */
