// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// the lending wizard
/**
   @file
   @author Christoph Pesch
*/

#include "colman_lending_page.hpp"
#ifndef COLMAN_CAMERA_SCAN_DIALOG_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_PERSON_WIDGET_HPP
#  include "colman_person_widget.hpp"
#endif
#ifndef COLMAN_PERSON_LIST_WIDGET_HPP
#  include "colman_person_list_widget.hpp"
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#endif
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
#endif
#ifndef COLMAN_SORT_MODEL_HPP
#  include "colman_sort_model.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_EXTEND_LENDING_DIALOG_HPP
#  include "colman_extend_lending_dialog.hpp"
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QITEMSELECTION
#  include <QItemSelection>
#  define SYSTEM_QITEMSELECTION
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QEVENT
#  include <QEvent>
#  define SYSTEM_QEVENT
#endif
#ifndef SYSTEM_QRADIOBUTTON
#  include <QRadioButton>
#  define SYSTEM_QRADIOBUTTON
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
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
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif


namespace
{
  void goto_person_page(colman::AppData& data);
  void goto_item_page(colman::AppData& data);
  void goto_details_page(colman::AppData& data, bool force_reserve = false);
  void goto_confirm_page(colman::AppData& data);
  void goto_done_page(colman::AppData& data);
  bool is_reserve_only(colman::AppData const& data);
  void update_lending_item_count(colman::AppData& data);
  void lending_load_more(colman::AppData& data);


  /// Returns true when the logged-in user may only reserve, not lend.
  bool is_reserve_only(colman::AppData const& data)
  {
    int const uid = data.current_data_collector_;
    if(uid == 0) {
      return false;
    }
    return !data.queries_->has_permission(uid, "collect_lendings") &&
           data.queries_->has_permission(uid, "reserve");
  }


  /// we assume that person is already selected, we now adjust everything
  void show_person(colman::AppData& data, int person_id)
  {
    colman::Person person;
    if(person_id >= 0) {
      data.queries_->get_person(person_id, person);
    }
    data.mw_.lending_person_person_->person_to_fields(person);
  }


  /// resets person query to original state (shows all persons)
  void reset_person_query(colman::AppData& data)
  {
    data.mw_.lending_person_person_list_->reset_person_query();
  }


  /// Updates view column visibility for the lending item table.
  void setup_lending_item_columns(colman::AppData& data)
  {
    auto* view = data.mw_.lending_item_item_view_;
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


  /// Updates the item count label and load-more button visibility.
  void update_lending_item_count(colman::AppData& data)
  {
    auto const n = data.lending_item_model_.rowCount();
    data.mw_.lending_item_item_count_->setText(
        QApplication::tr("%n item(s) shown", "", n));
    data.mw_.lending_item_load_more_->setVisible(
        data.lending_item_has_more_);
  }


  /**
     Resets the item query to show all acquired items (no filter).
     @param data central application state.
  */
  void reset_item_query(colman::AppData& data)
  {
    data.lending_item_offset_ = 0;
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    auto rows =
        data.queries_->query_acquired_items(-1, "", lim, 0);
    data.lending_item_has_more_ =
        lim > 0 && static_cast<int>(rows.size()) > lim;
    if(data.lending_item_has_more_) {
      rows.pop_back();
    }
    data.lending_item_model_.set_rows(std::move(rows));
    setup_lending_item_columns(data);
    update_lending_item_count(data);
  }


  /// Loads the next page of lending items and appends them to the model.
  void lending_load_more(colman::AppData& data)
  {
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    data.lending_item_offset_ += lim;
    auto rows = data.queries_->query_acquired_items(
        -1, "", lim, data.lending_item_offset_);
    data.lending_item_has_more_ =
        static_cast<int>(rows.size()) > lim;
    if(data.lending_item_has_more_) {
      rows.pop_back();
    }
    data.lending_item_model_.append_rows(std::move(rows));
    update_lending_item_count(data);
  }

} // namespace


/**
   Resets all fields on the lend pages and switches to the person page.
   Blocks signals during reset to avoid spurious callbacks.
   @param data central application state.
*/
void colman::enter_lend_page(colman::AppData& data)
{
  colman::AutoBlockSignals b2(data.mw_.lending_item_barcode_);
  colman::AutoBlockSignals b3(data.mw_.lending_person_person_list_);
  colman::AutoBlockSignals b4(data.mw_.lending_item_item_view_);
  colman::AutoBlockSignals b7(data.mw_.lending_details_due_date_);
  colman::AutoBlockSignals b8(data.mw_.lending_details_due_date_check_);
  colman::AutoBlockSignals b9(data.mw_.lending_details_remarks_);
  colman::AutoBlockSignals b10(data.mw_.lending_details_count_);
  colman::AutoBlockSignals
      b12(data.mw_.lending_item_item_view_->selectionModel());
  data.mw_.lending_person_person_list_->select_person(-1);
  data.mw_.lending_person_person_list_->clear_search_field();
  data.mw_.lending_item_item_view_->clearSelection();
  data.mw_.lending_item_barcode_->setText("");
  data.mw_.lending_details_due_date_->setDate(QDate());
  data.mw_.lending_details_due_date_->setEnabled(false);
  data.mw_.lending_details_due_date_check_->setChecked(false);
  data.mw_.lending_details_remarks_->setPlainText("");
  data.mw_.lending_details_count_->setValue(0);
  data.lending_reserve_intent_ = false;
  data.lending_action_item_id_ = -1;
  data.mw_.item_barcode_item_widget_->reset_item_type();
  data.mw_.lending_item_item_widget_->reset_item_type();
  reset_person_query(data);
  reset_item_query(data);
  if(is_reserve_only(data)) {
    data.mw_.lending_person_person_list_->select_person(
        data.current_data_collector_);
    goto_item_page(data);
  }
  else {
    goto_person_page(data);
  }
}


namespace
{
  /// we assume that item is already selected, we now adjust everything
  void show_item(colman::AppData& data, int item_id)
  {
    colman::Item item;
    QString type_str;
    if(item_id >= 0) {
      data.queries_->get_item(item_id, item, type_str);
    }
    data.mw_.lending_item_item_widget_->item_to_fields(item, type_str);
  }


  /**
     Filters the item list by the current barcode/search string.
     Automatically selects and advances to the details page when a barcode matches
     exactly one item.
     @param data central application state.
  */
  void search_item(colman::AppData& data)
  {
    colman::AutoBlockSignals b1(data.mw_.lending_item_item_view_);
    colman::AutoBlockSignals
        b2(data.mw_.lending_item_item_view_->selectionModel());
    QString const item_search = data.mw_.lending_item_barcode_->text();
    data.lending_item_offset_ = 0;
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    auto rows = data.queries_->query_acquired_items(
        -1, item_search, lim, 0);
    data.lending_item_has_more_ =
        lim > 0 && static_cast<int>(rows.size()) > lim;
    if(data.lending_item_has_more_) {
      rows.pop_back();
    }
    data.lending_item_model_.set_rows(std::move(rows));
    setup_lending_item_columns(data);
    update_lending_item_count(data);
    int const item_id =
        colman::select_if_single_row(data.mw_.lending_item_item_view_);
    if(item_id > 0) {
      data.mw_.lending_item_next_->setEnabled(true);
      colman::Collection const collection = data.queries_->read_collection();
      if(colman::is_item_barcode(collection, item_search)) {
        goto_details_page(data, data.lending_reserve_intent_);
        return;
      }
    }
    show_item(data, item_id);
  }


  /// Shows the "extend due date" button only to users allowed to use it.
  /**
     @param data      central application state.
     @param person_id currently selected person, or -1 for none.
  */
  void update_extend_button(colman::AppData& data, int person_id)
  {
    data.mw_.lending_person_extend_->setVisible(
        colman::may_extend_lendings(data));
    data.mw_.lending_person_extend_->setEnabled(person_id > 0);
  }


  /// Displays the person selection page of the lending wizard.
  void goto_person_page(colman::AppData& data)
  {
    int const person_id =
        data.mw_.lending_person_person_list_->get_selected_person_id();
    show_person(data, person_id);
    data.mw_.lending_person_next_->setEnabled(person_id > 0);
    update_extend_button(data, person_id);
    data.mw_.lending_person_person_list_->clear_search_field();
    data.mw_.lending_person_person_list_->focus_search_field();
    // Clear item selection so that "Next" always visits the item page for item choice.
    {
      colman::AutoBlockSignals bs(
          data.mw_.lending_item_item_view_->selectionModel());
      data.mw_.lending_item_item_view_->clearSelection();
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.lending_person_);
  }


  /// Displays the item selection page of the lending wizard.
  void goto_item_page(colman::AppData& data)
  {
    int const item_id =
        colman::selected_id(data.mw_.lending_item_item_view_);
    show_item(data, item_id);
    if(item_id > 0) {
      unsigned int const available = data.queries_->available(item_id);
      data.mw_.lending_item_next_->setEnabled(available > 0);
    }
    else {
      data.mw_.lending_item_next_->setEnabled(false);
    }
    if(is_reserve_only(data)) {
      data.mw_.label_472->setText(
          QApplication::tr("Step 2: Please select item to reserve!"));
    }
    else {
      data.mw_.label_472->setText(
          QApplication::tr("Step 2: Please select item to lend!"));
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.lending_item_);
  }


  /// Updates the count spinbox suffix for singular/plural and lend/reserve.
  void update_count_label(colman::AppData& data)
  {
    bool const is_reserve =
        data.mw_.lending_details_reserve_radio_->isChecked();
    int const n = data.mw_.lending_details_count_->value();
    QString suffix;
    if(is_reserve) {
      suffix = (n == 1)
                   ? QApplication::tr(" item to reserve")
                   : QApplication::tr(" items to reserve");
    }
    else {
      suffix = (n == 1)
                   ? QApplication::tr(" item to lend")
                   : QApplication::tr(" items to lend");
    }
    data.mw_.lending_details_count_->setSuffix(suffix);
  }


  /// Updates the details-page count range and Next button for the
  /// selected action.
  /**
     Lending may use all copies not reserved by other persons (the
     borrower's own reservation does not block their lending), while
     reserving may only add copies that nobody has reserved yet.
  */
  void update_details_limits(colman::AppData& data)
  {
    int const item_id =
        colman::selected_id(data.mw_.lending_item_item_view_);
    if(item_id < 0) {
      return;
    }
    int const borrower_id =
        data.mw_.lending_person_person_list_->get_selected_person_id();
    auto const reservations =
        data.queries_->get_reservations_for_item(item_id);
    int reserved_for_others = 0;
    int reserved_total = 0;
    for(auto const& r : reservations) {
      reserved_total += r.count;
      if(r.person_id != borrower_id) {
        reserved_for_others += r.count;
      }
    }
    int const available =
        static_cast<int>(data.queries_->available(item_id));
    bool const is_reserve =
        data.mw_.lending_details_reserve_radio_->isChecked();
    int const effective = std::max(
        0,
        available - (is_reserve ? reserved_total : reserved_for_others));
    data.mw_.lending_details_count_->setRange(
        (effective > 0 ? 1 : 0), effective);
    int const uid = data.current_data_collector_;
    bool const can_act =
        is_reserve
            ? (uid >= 0 && data.queries_->has_permission(uid, "reserve"))
            : ((uid == 0) ||
               data.queries_->has_permission(uid, "collect_lendings"));
    data.mw_.lending_details_next_->setEnabled(
        can_act && effective > 0);
  }


  /// Displays the lending details page of the lending wizard.
  void goto_details_page(colman::AppData& data, bool force_reserve)
  {
    int const person_id =
        data.mw_.lending_person_person_list_->get_selected_person_id();
    int const item_id =
        colman::selected_id(data.mw_.lending_item_item_view_);
    colman::Person person;
    if(data.queries_->get_person(person_id, person)) {
      QString name(person.get_title());
      if(name.size() > 0) {
        name += ' ';
      }
      name += person.get_lastname();
      if(name.size() > 0 && person.get_firstname().size() > 0) {
        name += ", ";
      }
      name += person.get_firstname();
      data.mw_.lending_details_person_name_->setText(name);
    }
    colman::Item item;
    QString tmp;
    if(data.queries_->get_item(item_id, item, tmp)) {
      data.mw_.lending_details_item_name_->setText(item.get_name());
    }
    unsigned int const available = data.queries_->available(item_id);
    int const uid = data.current_data_collector_;
    // Use the borrower's id (not the staff member's) to identify own reservations.
    int const borrower_id =
        data.mw_.lending_person_person_list_->get_selected_person_id();
    auto const reservations =
        data.queries_->get_reservations_for_item(item_id);
    int reserved_for_others = 0;
    for(auto const& r : reservations) {
      if(r.person_id != borrower_id) {
        reserved_for_others += r.count;
      }
    }
    int const effective =
        std::max(0, static_cast<int>(available) - reserved_for_others);
    QString avail_text =
        QApplication::tr("%n item(s) available", "", effective);
    if(reserved_for_others > 0) {
      avail_text +=
          " (" +
          QApplication::tr("%n item(s) reserved by others",
                           "", reserved_for_others) +
          ")";
    }
    data.mw_.lending_details_available_->setText(avail_text);
    colman::Collection const col = data.queries_->read_collection();
    // Preset the due date from the collection's default lending period.
    bool const use_period = col.get_use_lending_period();
    auto due_date = QDate::currentDate();
    if(use_period) {
      due_date = col.add_lending_period(due_date);
    }
    data.mw_.lending_details_due_date_->setDate(due_date);
    data.mw_.lending_details_due_date_check_->setChecked(use_period);
    data.mw_.lending_details_due_date_->setEnabled(use_period);

    bool item_allows = col.get_allow_reservations();
    if(item.get_allow_reservations() != -1) {
      item_allows = (item.get_allow_reservations() != 0);
    }
    bool const can_reserve =
        uid >= 0 &&
        item_allows &&
        data.queries_->has_permission(uid, "reserve");
    bool const can_lend =
        (uid == 0) ||
        data.queries_->has_permission(uid, "collect_lendings");
    // Show both radios only when the user has both permissions as a choice.
    data.mw_.lending_details_reserve_radio_->setVisible(
        can_reserve && can_lend);
    data.mw_.lending_details_lend_radio_->setVisible(can_reserve && can_lend);
    int const effective_period =
        (item.get_reservation_period_days() >= 0)
            ? item.get_reservation_period_days()
            : col.get_reservation_period_days();
    data.mw_.lending_details_expires_date_->setDate(
        QDate::currentDate().addDays(effective_period));
    if(can_reserve && !can_lend) {
      data.mw_.lending_details_reserve_radio_->setChecked(true);
      data.mw_.lending_details_duedate_row_->setVisible(false);
      data.mw_.lending_details_expires_row_->setVisible(
          data.queries_->has_permission(uid, "edit_reservations"));
    }
    else if(force_reserve && can_reserve) {
      data.mw_.lending_details_reserve_radio_->setChecked(true);
      // The reserve_radio toggled signal handles duedate/expires row visibility.
    }
    else {
      data.mw_.lending_details_lend_radio_->setChecked(true);
      data.mw_.lending_details_duedate_row_->setVisible(true);
      data.mw_.lending_details_expires_row_->setVisible(false);
    }
    update_count_label(data);
    update_details_limits(data);
    bool const show_reserve_labels =
        (can_reserve && !can_lend) || (force_reserve && can_reserve);
    if(show_reserve_labels) {
      data.mw_.label_494->setText(
          QApplication::tr("Step 3: Please select number of items to "
                           "reserve and optionally add comments!"));
      data.mw_.label_151->setText(
          QApplication::tr(
              "Do you have some remarks for this reservation?"));
    }
    else {
      data.mw_.label_494->setText(
          QApplication::tr("Step 3: Please select number of items to "
                           "lend and optionally add comments!"));
      data.mw_.label_151->setText(
          QApplication::tr("Do you have some remarks for this lending?"));
    }

    data.mw_.pages_stack_->setCurrentWidget(data.mw_.lending_details_);
  }


  /// Performs a reservation and advances to the success page.
  void do_reserve(colman::AppData& data)
  {
    int const item_id =
        colman::selected_id(data.mw_.lending_item_item_view_);
    int const person_id =
        data.mw_.lending_person_person_list_->get_selected_person_id();
    int const count = data.mw_.lending_details_count_->value();
    colman::Collection const col = data.queries_->read_collection();
    QDate expires_at;
    if(data.mw_.lending_details_expires_row_->isVisible()) {
      expires_at = data.mw_.lending_details_expires_date_->date();
    }
    else {
      colman::Item item;
      QString tmp;
      data.queries_->get_item(item_id, item, tmp);
      int const period =
          (item.get_reservation_period_days() >= 0)
              ? item.get_reservation_period_days()
              : col.get_reservation_period_days();
      expires_at = QDate::currentDate().addDays(period);
    }
    int new_id = 0;
    auto const result =
        data.queries_->add_reservation(item_id, person_id, count,
                                       expires_at,
                                       data.current_data_collector_,
                                       new_id);
    if(result == colman::IQueries::ReservationResult::Ok) {
      search_item(data);
      data.mw_.lending_done_question_->setText(
          QApplication::tr("Further reservations for person?"));
      data.mw_.pages_stack_->setCurrentWidget(data.mw_.lending_done_);
    }
    else if(result ==
            colman::IQueries::ReservationResult::AlreadyExists) {
      QMessageBox::warning(
          data.mw_.lending_details_,
          QApplication::tr("Collection Manager"),
          QApplication::tr(
              "You already have a reservation for this item."));
    }
    else if(result ==
            colman::IQueries::ReservationResult::NotEnoughAvailable) {
      unsigned int const avail = data.queries_->available(item_id);
      auto const ress = data.queries_->get_reservations_for_item(item_id);
      int not_reserved = static_cast<int>(avail);
      for(auto const& r : ress) {
        if(r.person_id != person_id) {
          not_reserved -= r.count;
        }
      }
      not_reserved = std::max(0, not_reserved);
      QMessageBox::warning(
          data.mw_.lending_details_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("Could not reserve item. "
                           "%n copy(ies) can be reserved.",
                           "", not_reserved));
    }
    else {
      QMessageBox::warning(
          data.mw_.lending_details_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("Could not reserve item."));
    }
  }


  /// Displays the confirmation page of the lending wizard.
  void goto_confirm_page(colman::AppData& data)
  {
    data.mw_.lending_confirm_person_->setText(data.mw_.lending_details_person_name_->text());
    data.mw_.lending_confirm_item_->setText(data.mw_.lending_details_item_name_->text());
    int const count = data.mw_.lending_details_count_->value();
    data.mw_.lending_confirm_number_->setText(QString().setNum(count));
    data.mw_.lending_confirm_due_date_->setText(data.mw_.lending_details_due_date_check_->isChecked()
                                                    ? data.mw_.lending_details_due_date_->date().toString(QApplication::tr("yyyy-MM-dd"))
                                                    : QString());
    data.mw_.lending_confirm_remarks_->setPlainText(data.mw_.lending_details_remarks_->toPlainText());
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.lending_confirm_);
  }


  /**
     Inserts a new lending and returns its database id.

     writes backup if needed.
     @param data Application state containing the database and undo stack.
     @param lending Data of the lending to insert.
     @return Database id of the newly inserted lending, or -1 on failure.
   */
  int add_new_lending(colman::AppData& data, colman::Lending const& lending)
  {
    colman::Lending le = lending;
    le.set_data_collector_id(data.current_data_collector_);
    return data.queries_->add_new_lending(le);
  }


  /// Commits the lending and displays the done page of the lending wizard.
  void goto_done_page(colman::AppData& data)
  {
    colman::Lending lending;
    lending.set_person_id(data.mw_.lending_person_person_list_->get_selected_person_id());
    lending.set_item_id(colman::selected_id(data.mw_.lending_item_item_view_));
    bool const with_duedate = data.mw_.lending_details_due_date_check_->isChecked();
    if(with_duedate) {
      lending.set_duedate(data.mw_.lending_details_due_date_->date());
    }
    lending.set_date_time(QDateTime::currentDateTimeUtc());
    lending.set_count(data.mw_.lending_details_count_->value());
    lending.set_remarks(data.mw_.lending_details_remarks_->toPlainText());
    int const lend_id = ::add_new_lending(data, lending);
    if(lend_id > 0) {
      search_item(data);
      data.mw_.lending_done_question_->setText(
          QApplication::tr("Further lendings for person?"));
      data.mw_.pages_stack_->setCurrentWidget(data.mw_.lending_done_);
    }
    else if(lend_id == -2) {
      int const item_id = lending.get_item_id();
      int const borrower_id = lending.get_person_id();
      auto const ress = data.queries_->get_reservations_for_item(item_id);
      int not_reserved = static_cast<int>(data.queries_->available(item_id));
      for(auto const& r : ress) {
        if(r.person_id != borrower_id) {
          not_reserved -= r.count;
        }
      }
      not_reserved = std::max(0, not_reserved);
      QMessageBox::warning(
          data.mw_.lending_confirm_,
          QApplication::tr("Collection Manager"),
          QApplication::tr(
              "Not enough unreserved copies available. "
              "%n copy(ies) can be lent.",
              "", not_reserved));
    }
    else {
      QMessageBox::warning(data.mw_.lending_confirm_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("Could not lend item."));
    }
  }


  /**
     Leaves the lending pages: resets mode functions, switches to the
     central page, and clears lending item and person models.
     @param data central application state.
  */
  void leave_lending_pages(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.lending_item_model_.set_rows({});
    data.lending_person_model_.set_persons({});
  }

} // namespace


/**
   Initializes the lending pages; to be called once at program start.
   Sets up person list, item view (with sort proxy and clear-selection filter),
   and connects all page navigation, selection, and barcode-search signals.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_lending_pages(QApplication& app, colman::AppData& data)
{
  data.mw_.lending_person_person_list_->setup(data.lending_person_model_);
  data.mw_.lending_person_person_list_->hide_column(
      colman::PersonListModel::TitleColumn);
  data.mw_.lending_person_person_list_->set_query_function([&data](QString const& s) {
    data.lending_person_model_.set_persons(data.queries_->query_persons(-1, s));
  });
  data.mw_.lending_person_person_->set_read_only(true);
  data.lending_item_model_.set_grayout_unavailable(true);
  SortModel<2, 3>*
      sqlproxy2 = new SortModel<2, 3>(data.mw_.lending_item_item_view_);
  sqlproxy2->setSourceModel(&data.lending_item_model_);
  data.mw_.lending_item_item_widget_->setup(data);
  data.mw_.lending_item_item_widget_->set_read_only(true);
  data.mw_.lending_item_item_view_->setModel(sqlproxy2);
  // Scroll smoothly (per pixel) so a wide name column can be
  // panned into view instead of jumping column by column.
  data.mw_.lending_item_item_view_->setHorizontalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.lending_item_item_view_->setVerticalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.lending_item_item_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  data.mw_.lending_item_item_view_->viewport()->installEventFilter(new ClearSelectionFilter(data.mw_.lending_item_item_view_));
  data.mw_.lending_details_due_date_->setDate(QDate::currentDate());
  data.mw_.lending_details_datetime_->hide();
  data.mw_.label_150->hide();

  app.connect(data.mw_.lending_person_person_list_,
              &PersonListWidget::selection_changed,
              [&data](int person_id) {
                show_person(data, person_id);
                data.mw_.lending_person_next_->setEnabled(person_id > 0);
                update_extend_button(data, person_id);
                if(person_id > 0 && data.queries_->is_person_barcode(
                                        data.mw_.lending_person_person_list_->get_search_string()))
                {
                  int item_id =
                      colman::selected_id(data.mw_.lending_item_item_view_);
                  if(item_id <= 0 &&
                     data.lending_action_item_id_ > 0)
                  {
                    colman::select_and_scroll_to(
                        data.mw_.lending_item_item_view_, 0,
                        data.lending_action_item_id_);
                    item_id =
                        colman::selected_id(data.mw_.lending_item_item_view_);
                  }
                  if(item_id > 0) {
                    goto_details_page(data, data.lending_reserve_intent_);
                  }
                  else {
                    goto_item_page(data);
                  }
                }
              });
  app.connect(data.mw_.lending_item_item_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const&,
                      QItemSelection const&) {
                int const item_id =
                    colman::selected_id(data.mw_.lending_item_item_view_);
                show_item(data, item_id);
                if(item_id > 0) {
                  unsigned int const available = data.queries_->available(
                      item_id);
                  data.mw_.lending_item_next_->setEnabled(available > 0);
                }
                else {
                  data.mw_.lending_item_next_->setEnabled(false);
                }
              });
  app.connect(data.mw_.lending_item_barcode_, &QLineEdit::editingFinished,
              [&data]() {
                if(data.mw_.lending_item_barcode_->isModified()) {
                  data.mw_.lending_item_barcode_->setModified(false);
                  search_item(data);
                }
              });
  data.mw_.lending_item_camera_->setIcon(
      QIcon(":/icons/owls/scan_barcode.jpeg"));
  app.connect(data.mw_.lending_item_camera_, &QToolButton::clicked,
              [&data]() {
                colman::scan_barcode_into(
                    data, data.mw_.lending_item_barcode_);
              });
  if(QMediaDevices::videoInputs().isEmpty()) {
    data.mw_.lending_item_camera_->setEnabled(false);
  }
  app.connect(data.mw_.lending_person_extend_, &QPushButton::clicked,
              [&data]() {
                colman::extend_lendings_of_person(
                    data.mw_.lending_person_, data,
                    data.mw_.lending_person_person_list_
                        ->get_selected_person_id());
              });
  app.connect(data.mw_.lending_person_next_, &QPushButton::clicked,
              [&data]() {
                int item_id =
                    colman::selected_id(data.mw_.lending_item_item_view_);
                if(item_id <= 0 &&
                   data.lending_action_item_id_ > 0)
                {
                  colman::select_and_scroll_to(
                      data.mw_.lending_item_item_view_, 0,
                      data.lending_action_item_id_);
                  item_id =
                      colman::selected_id(data.mw_.lending_item_item_view_);
                }
                if(item_id > 0) {
                  goto_details_page(data, data.lending_reserve_intent_);
                }
                else {
                  goto_item_page(data);
                }
              });
  app.connect(data.mw_.lending_item_next_, &QPushButton::clicked,
              [&data]() {
                goto_details_page(data, data.lending_reserve_intent_);
              });
  app.connect(data.mw_.lending_item_load_more_, &QPushButton::clicked,
              [&data]() {
                lending_load_more(data);
              });
  app.connect(data.mw_.lending_details_reserve_radio_, &QRadioButton::toggled,
              [&data](bool checked) {
                data.mw_.lending_details_duedate_row_->setVisible(!checked);
                // Show custom-expiry row only for reserve mode + edit_reservations.
                bool const can_edit_res =
                    checked &&
                    data.queries_->has_permission(
                        data.current_data_collector_, "edit_reservations");
                data.mw_.lending_details_expires_row_->setVisible(can_edit_res);
                update_details_limits(data);
                update_count_label(data);
              });
  app.connect(data.mw_.lending_details_count_,
              QOverload<int>::of(&QSpinBox::valueChanged),
              [&data](int) {
                update_count_label(data);
              });
  app.connect(data.mw_.lending_details_next_, &QPushButton::clicked,
              [&data]() {
                if(data.mw_.lending_details_reserve_radio_->isChecked()) {
                  do_reserve(data);
                }
                else {
                  goto_confirm_page(data);
                }
              });
  app.connect(data.mw_.lending_confirm_finish_, &QPushButton::clicked,
              [&data]() {
                goto_done_page(data);
              });
  app.connect(data.mw_.lending_person_back_, &QPushButton::clicked,
              [&data]() { leave_lending_pages(data); });
  app.connect(data.mw_.lending_item_back_, &QPushButton::clicked,
              [&data]() {
                if(is_reserve_only(data)) {
                  leave_lending_pages(data);
                }
                else {
                  goto_person_page(data);
                }
              });
  app.connect(data.mw_.lending_details_back_, &QPushButton::clicked,
              [&data]() { goto_item_page(data); });
  app.connect(data.mw_.lending_confirm_back_, &QPushButton::clicked,
              [&data]() { goto_details_page(data, data.lending_reserve_intent_); });
  app.connect(data.mw_.lending_done_back_, &QPushButton::clicked,
              [&data]() { leave_lending_pages(data); });
  app.connect(data.mw_.lending_done_no_, &QPushButton::clicked,
              [&data]() { leave_lending_pages(data); });
  app.connect(data.mw_.lending_done_yes_, &QPushButton::clicked,
              [&data]() {
                goto_item_page(data);
              });
  app.connect(data.lend_button_, &QToolButton::clicked,
              [&data]() {
                enter_lend_page(data);
              });
}


/**
   Refreshes the currently visible lending page.
   Re-runs the person or item query depending on which sub-page is active.
   @param data central application state.
*/
void colman::refresh_lending_pages(colman::AppData& data)
{
  QWidget const* const current = data.mw_.pages_stack_->currentWidget();
  if(current == data.mw_.lending_person_) {
    goto_person_page(data);
    data.mw_.lending_person_person_list_->refresh_person_query();
  }
  else if(current == data.mw_.lending_item_) {
    goto_item_page(data);
    search_item(data);
  }
  else if(current == data.mw_.lending_details_) {
    goto_details_page(data, data.lending_reserve_intent_);
  }
  else if(current == data.mw_.lending_confirm_) {
    goto_confirm_page(data);
  }
  //else if(current == data.mw_.lending_done_){
  //}
}

/**
   Enters the lending wizard pre-populated from \a ai.

   For item_id >= 0: starts at the person page (person selection) so the user can
   still choose a person; pre-fills the barcode field on the item page and
   pre-selects the item row so that both are ready when the user advances.

   For person_id >= 0: starts at the person page and pre-selects the person so
   the user only needs to press "Next".
   @param data  central application state.
   @param ai    Lending action produced by possible_actions().
*/
void colman::enter_lend_page_for_action(colman::AppData& data,
                                        colman::ActionInfo const& ai)
{
  enter_lend_page(data);
  if(ai.item_id_ >= 0) {
    data.mw_.lending_item_barcode_->setText(ai.barcode_);
    {
      // Filter item model to just this item by ID so select_and_scroll_to
      // always finds it regardless of collection size or barcode type.
      colman::AutoBlockSignals b1(data.mw_.lending_item_item_view_);
      colman::AutoBlockSignals
          b2(data.mw_.lending_item_item_view_->selectionModel());
      data.lending_item_offset_ = 0;
      auto const lim = QSettings().value("search_threshold", 1000).toInt();
      auto rows =
          data.queries_->query_acquired_items(ai.item_id_, "", lim, 0);
      data.lending_item_has_more_ =
          lim > 0 && static_cast<int>(rows.size()) > lim;
      if(data.lending_item_has_more_) {
        rows.pop_back();
      }
      data.lending_item_model_.set_rows(std::move(rows));
      setup_lending_item_columns(data);
      update_lending_item_count(data);
    }
    colman::select_and_scroll_to(
        data.mw_.lending_item_item_view_, 0, ai.item_id_);
    data.lending_action_item_id_ = ai.item_id_;
  }
  if(ai.person_id_ >= 0) {
    data.mw_.lending_person_person_list_->select_person(ai.person_id_);
    data.mw_.lending_person_next_->setEnabled(true);
  }
}


/**
   Switches to the lending wizard for a reservation pre-selecting \a item_id.
   With the "edit_reservations" permission the wizard starts at the person page so the
   operator can choose a person; the item page is skipped because the item is already
   selected and the Next button on the person page jumps straight to the details page.
   Without that permission the wizard skips both pages and opens directly on
   the details page with the current user and the item pre-selected.
   In both cases the details page opens with the reserve radio pre-selected.
   @param data    central application state.
   @param item_id the item to reserve.
*/
void colman::enter_reserve_page_for_item(colman::AppData& data,
                                         int item_id)
{
  enter_lend_page(data);
  data.lending_reserve_intent_ = true;
  colman::select_and_scroll_to(data.mw_.lending_item_item_view_, 0, item_id);
  int const uid = data.current_data_collector_;
  bool const can_edit_res =
      uid >= 0 && data.queries_->has_permission(uid, "edit_reservations");
  if(!can_edit_res) {
    if(uid >= 0) {
      colman::AutoBlockSignals bs(data.mw_.lending_person_person_list_);
      data.mw_.lending_person_person_list_->select_person(uid);
      data.mw_.lending_person_next_->setEnabled(true);
    }
    goto_details_page(data, true);
  }
}


/* EOF */
