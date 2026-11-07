// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// the liquidation wizard
/**
   @file
   @author Christoph Pesch
*/

#include "colman_liquidation_page.hpp"
#ifndef COLMAN_CAMERA_SCAN_DIALOG_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_SORT_MODEL_HPP
#  include "colman_sort_model.hpp"
#endif
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
#endif
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif
#ifndef SYSTEM_QCAMERADEVICE
#  include <QCameraDevice>
#  define SYSTEM_QCAMERADEVICE
#endif
#ifndef SYSTEM_QMEDIADEVICES
#  include <QMediaDevices>
#  define SYSTEM_QMEDIADEVICES
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif

namespace
{
  void goto_item_page(colman::AppData& data);
  void goto_count_page(colman::AppData& data);
  void goto_done_page(colman::AppData& data);
  void update_liquidation_item_count(colman::AppData& data);
  void liquidation_load_more(colman::AppData& data);


  /// Updates column visibility for the liquidation item table.
  void setup_liquidation_item_columns(colman::AppData& data)
  {
    auto* view = data.mw_.liquidation_item_items_view_;
    view->setColumnHidden(0, true);
    if(data.queries_->is_locale_supported()) {
      view->setColumnHidden(4, false);
      view->setColumnHidden(5, true);
    }
    else {
      view->setColumnHidden(4, true);
      view->setColumnHidden(5, false);
    }
    view->setColumnHidden(6, true);
  }


  /// Updates the liquidation item count label and load-more visibility.
  void update_liquidation_item_count(colman::AppData& data)
  {
    auto const n = data.liquidation_item_model_.rowCount();
    data.mw_.liquidation_item_item_count_->setText(
        QApplication::tr("%n item(s) shown", "", n));
    data.mw_.liquidation_item_load_more_->setVisible(
        data.liquidation_item_has_more_);
  }


  /**
   * Reloads the item table on the item page using the current search field text as a
   * filter and refreshes the column headers. Only items that still have copies
   * available in stock are shown (uses the available-items query).
   * @param data Central application state.
   */
  void refresh_item_query(colman::AppData& data)
  {
    QString const& search = data.mw_.liquidation_item_search_->text();
    data.liquidation_item_offset_ = 0;
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    auto rows =
        data.queries_->query_available_items(-1, search, lim, 0);
    data.liquidation_item_has_more_ =
        lim > 0 && static_cast<int>(rows.size()) > lim;
    if(data.liquidation_item_has_more_) {
      rows.pop_back();
    }
    data.liquidation_item_model_.set_rows(std::move(rows));
    setup_liquidation_item_columns(data);
    update_liquidation_item_count(data);
  }


  /// Loads the next page of liquidation items and appends them to the model.
  void liquidation_load_more(colman::AppData& data)
  {
    QString const& search = data.mw_.liquidation_item_search_->text();
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    data.liquidation_item_offset_ += lim;
    auto rows = data.queries_->query_available_items(
        -1, search, lim, data.liquidation_item_offset_);
    data.liquidation_item_has_more_ =
        static_cast<int>(rows.size()) > lim;
    if(data.liquidation_item_has_more_) {
      rows.pop_back();
    }
    data.liquidation_item_model_.append_rows(std::move(rows));
    update_liquidation_item_count(data);
  }


  /**
   * Selects the item identified by @p id and populates the item detail widget
   * on the item page with its data. Also updates the available-copy count label and
   * the maximum value of the count spin box on the count page. If the search field
   * contains a valid item barcode, automatically advances to the count page. Shows a
   * warning dialog and clears the search field when the item is not found.
   * @param data Central application state.
   * @param id   Database ID of the item to select.
   */
  void select_item(colman::AppData& data, int id)
  {
    colman::Item item;
    QString type_name;
    if(!data.queries_->get_item(id, item, type_name)) {
      QMessageBox::warning(data.mw_.liquidation_item_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("unknown item"));
      data.mw_.liquidation_item_search_->setText("");
      return;
    }
    unsigned int const available = data.queries_->available(id);
    data.mw_.liquidation_count_available_->setText(QString().setNum(available));
    data.mw_.liquidation_count_count_->setMaximum(available);
    data.mw_.liquidation_item_item_->item_to_fields(item, type_name);
    colman::select_and_scroll_to(data.mw_.liquidation_item_items_view_, 0, id);
    data.mw_.liquidation_item_next_->setEnabled(true);
    if(data.queries_->is_item_barcode(
           data.mw_.liquidation_item_search_->text()))
    {
      goto_count_page(data);
    }
  }


  /**
   * Clears the item detail widget and deselects any row in the item table.
   * Disables the Next button so the user cannot proceed without selecting an
   * item.
   * @param data Central application state.
   */
  void deselect_item(colman::AppData& data)
  {
    data.mw_.liquidation_item_item_->clear_fields();
    data.mw_.liquidation_item_items_view_->clearSelection();
    data.mw_.liquidation_item_next_->setEnabled(false);
  }


  /**
   * Refreshes the item table using the current search field text and, if
   * exactly one row matches, selects that row automatically.
   * @param data Central application state.
   */
  void search_barcode(colman::AppData& data)
  {
    refresh_item_query(data);
    int const rows = data.liquidation_item_model_.rowCount();
    if(1 == rows) {
      data.mw_.liquidation_item_items_view_->selectRow(0);
    }
  }


  /**
   * Displays the item page of the liquidation wizard. Refreshes
   * the item table, moves keyboard focus to the search field, and disables the
   * Next button until an item is selected.
   * @param data Central application state.
   */
  void goto_item_page(colman::AppData& data)
  {
    refresh_item_query(data);
    data.mw_.liquidation_item_search_->setFocus();
    data.mw_.liquidation_item_next_->setEnabled(false);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.liquidation_item_);
  }

  /**
   * Displays the count page of the liquidation wizard.
   * Reads the item name from the item-page item widget and shows it as a
   * confirmation label on the count page.
   * @param data Central application state.
   */
  void goto_count_page(colman::AppData& data)
  {
    colman::Item item;
    data.mw_.liquidation_item_item_->fields_to_item(item);
    data.mw_.liquidation_count_item_name_->setText(item.get_name());

    data.mw_.pages_stack_->setCurrentWidget(data.mw_.liquidation_count_);
  }


  /// Inserts a new liquidation record and returns its database id.
  /**
     writes a backup if necessary.
     @param data Application state containing the database and undo stack.
     @param liquidation Data of the liquidation to insert.
     @return Database id of the newly inserted liquidation, or -1 on failure.
   */
  int add_new_liquidation(colman::AppData& data,
                          colman::Liquidation const& liquidation)
  {
    colman::Liquidation l = liquidation;
    l.set_data_collector_id(data.current_data_collector_);
    return data.queries_->add_new_liquidation(l);
  }


  /**
   * Commits the liquidation to the database and advances to the done page.
   * Reads the selected item ID, the chosen date, and the copy count
   * from the count-page widgets. If the count is greater than zero a backup is
   * written and a new Liquidation record is inserted; a warning dialog is shown
   * and the wizard stays on the count page if the insertion fails. When count is zero
   * no database record is created and the wizard still advances to the done page.
   * @param data Central application state.
   */
  void goto_done_page(colman::AppData& data)
  {
    int item_id = colman::selected_id(data.mw_.liquidation_item_items_view_);
    auto const datetime = QDateTime::currentDateTimeUtc();
    int const count = data.mw_.liquidation_count_count_->value();
    if(count > 0) {
      colman::Liquidation liquidation;
      liquidation.set_item_id(item_id);
      liquidation.set_count(count);
      liquidation.set_date_time(datetime);
      int const aid = ::add_new_liquidation(data, liquidation);
      if(aid < 0) {
        QMessageBox::warning(data.mw_.liquidation_count_,
                             QApplication::tr("Collection Manager"),
                             QApplication::tr("Could not add liquidation."));
        return;
      }
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.liquidation_done_);
  }


  /**
   * Exits the liquidation wizard and returns to the central start page.
   * Clears the item query mode.
   * @param data Central application state.
   */
  void leave_liquidation_pages(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.liquidation_item_model_.set_rows({});
  }

} // namespace


/**
 * One-time initialisation of the liquidation wizard pages. Sets up the
 * sort proxy model and column resize behaviour for the item table view,
 * configures the item widget as read-only, installs a ClearSelectionFilter
 * event filter on the table viewport, and connects all wizard navigation
 * buttons (Next, Previous, Cancel, Finish, Yes/No on the completion page),
 * the barcode search field, and the main toolbar Liquidation button to their
 * respective handler lambdas.
 * @param app  The Qt application object used for signal/slot connections.
 * @param data Central application state.
 */
void colman::setup_liquidation_pages(QApplication& app, colman::AppData& data)
{
  QSortFilterProxyModel*
      sqlproxy = new QSortFilterProxyModel(data.mw_.liquidation_item_items_view_);
  sqlproxy->setSourceModel(&data.liquidation_item_model_);
  sqlproxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  data.mw_.liquidation_item_items_view_->setModel(sqlproxy);
  // Scroll smoothly (per pixel) so a wide name column can be
  // panned into view instead of jumping column by column.
  data.mw_.liquidation_item_items_view_->setHorizontalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.liquidation_item_items_view_->setVerticalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.liquidation_item_items_view_->horizontalHeader()
      ->setSectionResizeMode(QHeaderView::ResizeToContents);
  data.mw_.liquidation_item_items_view_->viewport()->installEventFilter(new ClearSelectionFilter(data.mw_.liquidation_item_items_view_));
  data.mw_.liquidation_item_items_view_->clearSelection();
  data.mw_.liquidation_item_item_->setup(data);
  data.mw_.liquidation_item_item_->set_read_only(true);
  data.mw_.liquidation_count_datetime_->hide();
  data.mw_.label_23->hide();

  app.connect(data.mw_.liquidation_item_next_, &QPushButton::clicked,
              [&data]() {
                goto_count_page(data);
              });
  app.connect(data.mw_.liquidation_item_load_more_, &QPushButton::clicked,
              [&data]() {
                liquidation_load_more(data);
              });
  app.connect(data.mw_.liquidation_item_back_, &QPushButton::clicked,
              [&data]() {
                leave_liquidation_pages(data);
              });
  app.connect(data.mw_.liquidation_count_finish_, &QPushButton::clicked,
              [&data]() {
                goto_done_page(data);
              });
  app.connect(data.mw_.liquidation_count_previous_, &QPushButton::clicked,
              [&data]() {
                data.mw_.liquidation_item_search_->clear();
                goto_item_page(data);
              });
  app.connect(data.mw_.liquidation_count_back_, &QPushButton::clicked,
              [&data]() {
                data.mw_.liquidation_item_search_->clear();
                goto_item_page(data);
              });
  app.connect(data.mw_.liquidation_done_yes_, &QPushButton::clicked,
              [&data]() {
                data.mw_.liquidation_item_search_->clear();
                goto_item_page(data);
              });
  app.connect(data.mw_.liquidation_done_no_, &QPushButton::clicked,
              [&data]() {
                leave_liquidation_pages(data);
              });
  app.connect(data.mw_.liquidation_item_search_,
              &QLineEdit::editingFinished,
              [&data]() {
                if(data.mw_.liquidation_item_search_->isModified()) {
                  data.mw_.liquidation_item_search_->setModified(false);
                  search_barcode(data);
                }
              });
  data.mw_.liquidation_item_camera_->setIcon(
      QIcon(":/icons/owls/scan_barcode.jpeg"));
  app.connect(data.mw_.liquidation_item_camera_, &QToolButton::clicked,
              [&data]() {
                colman::scan_barcode_into(
                    data, data.mw_.liquidation_item_search_);
              });
  if(QMediaDevices::videoInputs().isEmpty()) {
    data.mw_.liquidation_item_camera_->setEnabled(false);
  }
  app.connect(data.mw_.liquidation_item_items_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const& selected,
                      QItemSelection const&) {
                auto const index_list = selected.indexes();
                if(!index_list.isEmpty()) {
                  QModelIndex const& model_index = index_list.front();
                  QAbstractItemModel const* model = model_index.model();
                  if((model)) {
                    int const row = model_index.row();
                    QVariant const
                        id = model->data(model->index(row, 0), Qt::DisplayRole);
                    select_item(data, id.toInt());
                  }
                }
                else {
                  deselect_item(data);
                }
              });
  app.connect(data.liquidation_button_, &QToolButton::clicked,
              [&data]() {
                enter_liquidation_page(data);
              });
}


/**
 * Prepares the liquidation wizard for a fresh transaction by clearing the
 * search field and navigating to the item page. 
 * @param data Central application state.
 */
void colman::enter_liquidation_page(AppData& data)
{
  data.mw_.liquidation_item_search_->clear();
  data.mw_.liquidation_item_item_->reset_item_type();
  goto_item_page(data);
}


/**
 * Refreshes the currently visible liquidation wizard page.
 * Determines which page is active and re-runs the
 * appropriate query or navigation function so that the displayed data reflects
 * the current database state.
 * @param data Central application state.
 */
void colman::refresh_liquidation_pages(colman::AppData& data)
{
  QWidget const* const current = data.mw_.pages_stack_->currentWidget();
  if(current == data.mw_.liquidation_item_) {
    goto_item_page(data);
    search_barcode(data);
  }
  else if(current == data.mw_.liquidation_count_) {
    goto_count_page(data);
  }
}

void colman::enter_liquidation_page_for_action(
    colman::AppData& data,
    colman::ActionInfo const& ai)
{
  enter_liquidation_page(data);
  if(ai.item_id_ >= 0) {
    colman::select_and_scroll_to(
        data.mw_.liquidation_item_items_view_, 0, ai.item_id_);
    goto_count_page(data);
  }
}

/* EOF */
