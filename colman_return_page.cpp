// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// the return wizard
/**
   @file
   @author Christoph Pesch
*/

#include "colman_return_page.hpp"
#ifndef COLMAN_CAMERA_SCAN_DIALOG_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_PERSON_WIDGET_HPP
#  include "colman_person_widget.hpp"
#endif
#ifndef COLMAN_PERSON_CHECK_LIST_WIDGET_HPP
#  include "colman_person_check_list_widget.hpp"
#endif
#ifndef COLMAN_CLASS_NAMES_HPP
#  include "colman_class_names.hpp"
#endif
#ifndef COLMAN_ITEM_TABLE_HPP
#  include "colman_item_table.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
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
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QEVENT
#  include <QEvent>
#  define SYSTEM_QEVENT
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{
  void goto_item_page(colman::AppData& data);
  void goto_borrower_page(colman::AppData& data);
  void goto_lending_page(colman::AppData& data);
  void goto_borrower_page_or_further(colman::AppData& data);
  void goto_lending_page_or_further(colman::AppData& data);
  void goto_details_page(colman::AppData& data);
  void goto_confirm_page(colman::AppData& data);
  void goto_done_page(colman::AppData& data);

  /// we assume that person is already selected, we now adjust everything
  /**
   * Reads the person record identified by @p person_id from the database and
   * populates the person detail widget on the return page 2. When
   * @p person_id is negative a default-constructed (empty) Person is shown.
   * @param data    Central application state.
   * @param person_id Database ID of the person to display, or -1 for none.
   */
  void show_person(colman::AppData& data, int person_id)
  {
    colman::Person person;
    if(person_id >= 0) {
      data.queries_->get_person(person_id, person);
    }
    data.mw_.return_borrower_person_->person_to_fields(person);
  }

  /// Returns the ids of the ticked returning persons.
  QList<int> checked_person_ids(colman::AppData& data)
  {
    return data.mw_.return_borrower_person_list_->checked_ids();
  }


  /// Returns the id of the only ticked person, or -1 (none or several).
  int single_person_id(colman::AppData& data)
  {
    auto const ids = checked_person_ids(data);
    return ids.size() == 1 ? ids.front() : -1;
  }


  /// Returns true when several persons are ticked (group return).
  bool is_multi(colman::AppData& data)
  {
    return data.mw_.return_borrower_person_list_->checked_count() > 1;
  }


  /// Returns "Lastname, Firstname" of \a person.
  QString display_name(colman::Person const& person)
  {
    QString name = person.get_lastname();
    if(!name.isEmpty() && !person.get_firstname().isEmpty()) {
      name += ", ";
    }
    name += person.get_firstname();
    return name;
  }


  /**
     Returns the number of copies of \a item_id still on loan, per person.
     @param data    Central application state.
     @param item_id the item
     @return person id → open copies
  */
  QHash<int, int> open_copies_by_person(colman::AppData& data, int item_id)
  {
    QHash<int, int> person_of_lending;
    for(auto const& lending : data.queries_->open_lendings_for_item(item_id)) {
      person_of_lending.insert(lending.first, lending.second);
    }
    QHash<int, int> result;
    for(auto const& row : data.queries_->query_open_lendings(-1, item_id)) {
      int const person_id = person_of_lending.value(row.id, -1);
      if(person_id >= 0) {
        result[person_id] += row.lent_count - row.returned_count;
      }
    }
    return result;
  }


  /// Updates the "persons selected" label and the next button of step 2.
  void update_borrower_status(colman::AppData& data)
  {
    int const n = data.mw_.return_borrower_person_list_->checked_count();
    data.mw_.return_borrower_status_->setText(
        QApplication::tr("%n person(s) selected", "", n));
    data.mw_.return_borrower_next_->setEnabled(n > 0);
  }

  /**
   * Reloads the item table on the return item page with all currently lent
   * items (no person or item filter applied) and refreshes the column headers.
   * @param data Central application state.
   */
  void reset_item_query(colman::AppData& data)
  {
    data.return_item_model_.set_rows(data.queries_->query_lent_items(-1, -1, "", ""));
    {
      auto* view = data.mw_.return_item_item_view_;
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
  }

} // namespace


/// resets all field on the return pages before showing the page
/**
 * Prepares the return wizard for a fresh transaction: clears all
 * selections and input fields, resets the person and item queries to show
 * all records, sets the return date to today, and navigates to the item page.
 * @param data Central application state.
 */
void colman::enter_return_page(colman::AppData& data)
{
  colman::AutoBlockSignals b2(data.mw_.return_item_barcode_);
  colman::AutoBlockSignals b3(data.mw_.return_borrower_person_list_);
  colman::AutoBlockSignals b4(data.mw_.return_item_item_view_);
  colman::AutoBlockSignals b9(data.mw_.return_details_remarks_);
  colman::AutoBlockSignals b10(data.mw_.return_details_count_);
  colman::AutoBlockSignals
      b12(data.mw_.return_item_item_view_->selectionModel());
  data.mw_.return_borrower_person_list_->set_persons({}, {});
  data.mw_.return_borrower_person_list_->clear_search_field();
  data.return_borrower_item_id_ = -1;
  data.mw_.return_item_item_view_->clearSelection();
  data.mw_.return_item_barcode_->setText("");
  data.mw_.return_details_remarks_->setPlainText("");
  data.mw_.return_details_count_->setValue(0);
  reset_item_query(data);
  data.mw_.return_item_item_widget_->reset_item_type();
  goto_item_page(data);
}


namespace
{
  /// we assume that item is already selected, we now adjust everything
  /**
   * Reads the item record identified by @p item_id from the database and
   * populates the item detail widget on the return item page. When @p item_id
   * is negative a default-constructed (empty) Item is shown.
   * @param data    Central application state.
   * @param item_id Database ID of the item to display, or -1 for none.
   */
  void show_item(colman::AppData& data, int item_id)
  {
    colman::Item item;
    QString type_str;
    if(item_id >= 0) {
      data.queries_->get_item(item_id, item, type_str);
    }
    data.mw_.return_item_item_widget_->item_to_fields(item, type_str);
  }


  /**
   * Filters the item table by the text currently in the barcode/search field
   * and, if exactly one matching lent item is found, selects it automatically.
   * If the search string is a recognised item barcode and a unique item was
   * found, the wizard advances to the borrower page (or further) immediately.
   * @param data Central application state.
   */
  void search_item(colman::AppData& data)
  {
    colman::AutoBlockSignals b1(data.mw_.return_item_item_view_);
    colman::AutoBlockSignals
        b2(data.mw_.return_item_item_view_->selectionModel());
    QString const item_search = data.mw_.return_item_barcode_->text();
    data.return_item_model_.set_rows(data.queries_->query_lent_items(-1, -1, "", item_search));
    {
      auto* view = data.mw_.return_item_item_view_;
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
    int const item_id =
        colman::select_if_single_row(data.mw_.return_item_item_view_);
    show_item(data, item_id);
    if(item_id > 0) {
      colman::Collection const collection = data.queries_->read_collection();
      if(colman::is_item_barcode(collection, item_search)) {
        goto_borrower_page_or_further(data);
        return;
      }
    }
  }


  /**
   * Displays the item page of the return wizard. Refreshes the
   * item detail widget for the currently selected item and enables the Next
   * button only when an item with outstanding lendings is selected.
   * @param data Central application state.
   */
  void goto_item_page(colman::AppData& data)
  {
    int const item_id =
        colman::selected_id(data.mw_.return_item_item_view_);
    show_item(data, item_id);
    data.mw_.return_item_next_->setEnabled(item_id > 0 && data.queries_->have_lendings(item_id));
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_item_);
  }


  /**
   * Loads the persons holding an open lending of the selected item into
   * the check list of step 2.  Ticks survive a reload for the same item
   * (undo/redo, back navigation); a single borrower is ticked right away.
   * The note column shows the open copies when a person holds several.
   * @param data Central application state.
   */
  void load_borrowers(colman::AppData& data)
  {
    int const item_id =
        colman::selected_id(data.mw_.return_item_item_view_);
    QString const item_search = data.mw_.return_item_barcode_->text();
    auto* list = data.mw_.return_borrower_person_list_;
    QSet<int> checked;
    if(item_id == data.return_borrower_item_id_) {
      for(int const id : list->checked_ids()) {
        checked.insert(id);
      }
    }
    else {
      list->clear_search_field();
    }
    data.return_borrower_item_id_ = item_id;
    auto const persons =
        data.queries_->query_lending_persons(item_id, -1, "", item_search);
    list->set_persons(
        persons,
        colman::merge_class_names(data.queries_->load_class_names(),
                                  persons));
    QHash<int, QString> notes;
    auto const copies = open_copies_by_person(data, item_id);
    for(auto it = copies.cbegin(); it != copies.cend(); ++it) {
      if(it.value() > 1) {
        notes.insert(it.key(), QApplication::tr("%n copies on loan", "",
                                                it.value()));
      }
    }
    list->set_notes(notes);
    if(persons.size() == 1) {
      checked = {persons.front().get_id()};
    }
    list->set_checked_ids(checked);
    update_borrower_status(data);
  }


  /**
   * Displays the borrower page of the return wizard with all persons
   * holding an open lending of the selected item.  The Next button is
   * enabled when at least one person is ticked.
   * @param data Central application state.
   */
  void goto_borrower_page(colman::AppData& data)
  {
    load_borrowers(data);
    data.mw_.return_borrower_message_->clear();
    show_person(data, single_person_id(data));
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_borrower_);
    data.mw_.return_borrower_person_list_->focus_search_field();
  }


  /**
     Ticks the borrower whose library card was scanned into the search
     field of step 2 and reports the outcome below the list.
     @param data Central application state.
     @param text the search field text
  */
  void check_scanned_person(colman::AppData& data, QString const& text)
  {
    using CheckResult = colman::PersonCheckListWidget::CheckResult;
    auto const code = text.trimmed();
    if(code.isEmpty() || !data.queries_->is_person_barcode(code)) {
      return;
    }
    auto* list = data.mw_.return_borrower_person_list_;
    auto* message = data.mw_.return_borrower_message_;
    list->clear_search_field();
    auto const persons = data.queries_->query_persons(-1, code);
    if(persons.size() != 1) {
      message->setText(QApplication::tr("Unknown library card."));
      QApplication::beep();
      return;
    }
    auto const name = display_name(persons.front());
    switch(list->check_person(persons.front().get_id())) {
    case CheckResult::Checked:
      message->setText(QApplication::tr("%1 added.").arg(name));
      break;
    case CheckResult::AlreadyChecked:
      message->setText(QApplication::tr("%1 is already selected.").arg(name));
      break;
    case CheckResult::Unknown:
    case CheckResult::Disabled:
    case CheckResult::Refused:
      message->setText(
          QApplication::tr("%1 has no open lending of this item.").arg(name));
      QApplication::beep();
      break;
    }
  }


  /**
   * Returns true when the person list on the borrower page contains exactly one entry,
   * meaning the borrower is unambiguous and can be selected automatically.
   * @param data Central application state.
   * @return true if there is exactly one person in the list; false otherwise.
   */
  bool is_person_known(colman::AppData& data)
  {
    return data.mw_.return_borrower_person_list_ &&
           data.mw_.return_borrower_person_list_->person_count() == 1;
  }

  /**
   * Navigates to the borrower page and, if the person list
   * contains exactly one entry, automatically selects that person and
   * advances to the lending page (or further).
   * @param data Central application state.
   */
  void goto_borrower_page_or_further(colman::AppData& data)
  {
    goto_borrower_page(data);
    if(is_person_known(data)) {
      goto_lending_page_or_further(data);
    }
  }

  /**
   * Displays the lending page of the return wizard. Queries
   * all open lendings for the selected person and item, populates the lending
   * table, and shows the person and item names as confirmation labels. The
   * previously selected lending row (if any) is restored.
   * @param data Central application state.
   */
  void goto_lending_page(colman::AppData& data)
  {
    int const
        lending_id = colman::selected_id(data.mw_.return_lending_lendings_view_);
    int const item_id =
        colman::selected_id(data.mw_.return_item_item_view_);
    int const person_id = single_person_id(data);
    data.return_lending_model_.set_rows(
        data.queries_->query_open_lendings(person_id, item_id));
    data.mw_.return_lending_lendings_view_->setColumnHidden(0, true);
    colman::select_and_scroll_to(data.mw_.return_lending_lendings_view_,
                                 0, lending_id);
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
      data.mw_.return_lending_person_name_->setText(name);
    }
    colman::Item item;
    QString type_name;
    if(data.queries_->get_item(item_id, item, type_name)) {
      data.mw_.return_lending_item_name_->setText(item.get_name());
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_lending_);
  }

  /**
   * Checks whether the lending table on the lending page contains exactly one row and,
   * if so, selects it automatically.
   * @param data Central application state.
   * @return true if a single lending was found and selected; false otherwise.
   */
  bool is_lending_known(colman::AppData& data)
  {
    return colman::select_if_single_row(data.mw_.return_lending_lendings_view_) > 0;
  }

  /**
   * Navigates to the lending page and, if exactly one open
   * lending exists for the selected person/item combination, automatically
   * selects it and advances to the details page.
   * @param data Central application state.
   */
  void goto_lending_page_or_further(colman::AppData& data)
  {
    goto_lending_page(data);
    if(is_lending_known(data)) {
      goto_details_page(data);
    }
  }

  /**
     Switches steps 4 and 5 between single return (count, lent/returned
     counts, extend button) and group return (summary, name list).
     @param data   Central application state.
     @param single true for the single return layout
  */
  void set_single_return_layout(colman::AppData& data, bool single)
  {
    auto* counts = data.mw_.return_details_counts_form_;
    for(int row = 2; row < counts->rowCount(); ++row) {
      counts->setRowVisible(row, single);
    }
    data.mw_.return_details_count_form_->setRowVisible(
        data.mw_.return_details_count_, single);
    data.mw_.return_details_summary_->setVisible(!single);
    data.mw_.return_confirm_form_->setRowVisible(
        data.mw_.return_confirm_persons_, !single);
  }


  /// Returns the name of the item selected in step 1.
  QString selected_item_name(colman::AppData& data)
  {
    colman::Item item;
    QString type_name;
    data.queries_->get_item(
        colman::selected_id(data.mw_.return_item_item_view_), item,
        type_name);
    return item.get_name();
  }


  /**
     Displays step 4 for a group return: all open copies of the ticked
     persons are returned, only remarks can be entered.
     @param data Central application state.
  */
  void goto_group_details_page(colman::AppData& data)
  {
    set_single_return_layout(data, false);
    int const n = data.mw_.return_borrower_person_list_->checked_count();
    data.mw_.return_details_person_name_->setText(
        QApplication::tr("%n person(s)", "", n));
    data.mw_.return_details_item_name_->setText(selected_item_name(data));
    data.mw_.return_details_summary_->setText(
        QApplication::tr("All copies still on loan to these %n person(s) "
                         "are returned.", "", n));
    data.mw_.label_164->setText(
        QApplication::tr("Step 4: Here you can add remarks"));
    data.mw_.return_details_datetime_->hide();
    data.mw_.label_169->hide();
    data.mw_.return_details_extend_->setVisible(false);
    data.mw_.return_details_next_->setEnabled(n > 1);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_details_);
  }


  /**
   * Displays the details page of the return wizard. Shows the
   * person and item names as confirmation labels, computes the lent/returned/
   * remaining copy counts from the database, pre-fills the return date with
   * today's date, and enables the Next button only when there are still copies
   * to be returned. If any required selection is missing the Next button is
   * disabled and the function returns early.
   * @param data Central application state.
   */
  void goto_details_page(colman::AppData& data)
  {
    if(is_multi(data)) {
      goto_group_details_page(data);
      return;
    }
    set_single_return_layout(data, true);
    int const person_id = single_person_id(data);
    int const item_id =
        colman::selected_id(data.mw_.return_item_item_view_);
    int const
        lending_id = colman::selected_id(data.mw_.return_lending_lendings_view_);
    if(person_id < 0 || item_id < 0 || lending_id < 0) {
      data.mw_.return_lending_next_->setEnabled(false);
      return;
    }
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
      data.mw_.return_details_person_name_->setText(name);
    }
    colman::Item item;
    QString tmp;
    if(data.queries_->get_item(item_id, item, tmp)) {
      data.mw_.return_details_item_name_->setText(item.get_name());
    }
    // Read the lent/returned counts straight from the selected lending row,
    // which is already loaded in the list.  This avoids an extra
    // lending_balance() round-trip that, in the REST client, would otherwise
    // require its own permission and made the count show up as 0.
    int lent = 0;
    int returned = 0;
    {
      auto* sel = data.mw_.return_lending_lendings_view_->selectionModel();
      if(sel && sel->hasSelection() && !sel->selectedRows().isEmpty()) {
        QModelIndex const row = sel->selectedRows().front();
        lent = row.sibling(row.row(), 1).data().toInt();
        returned = row.sibling(row.row(), 2).data().toInt();
      }
    }
    int const remaining = lent - returned;
    data.mw_.return_details_lent_count_->setText(QString().setNum(lent));
    data.mw_.return_details_returned_count_->setText(QString().setNum(returned));
    data.mw_.return_details_remaining_count_->setText(QString().setNum(remaining));
    data.mw_.return_details_next_->setEnabled(lent > returned);
    data.mw_.return_details_count_->setRange((remaining > 0 ? 1 : 0), remaining);
    if(remaining == 1) {
      data.mw_.label_164->setText(
          QApplication::tr("Step 4: Here you can add remarks"));
    }
    else {
      data.mw_.label_164->setText(
          QApplication::tr(
              "Step 4: Enter count of returned items and remarks"));
    }
    data.mw_.return_details_datetime_->hide();
    data.mw_.label_169->hide();
    data.mw_.return_details_extend_->setVisible(
        colman::may_extend_lendings(data));
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_details_);
  }


  /**
   * Displays the confirm page of the return wizard. Copies
   * the person name, item name, return count, date, and remarks from the
   * details-page input widgets into the read-only summary labels on the confirm page so the
   * user can verify the data before committing.
   * @param data Central application state.
   */
  void goto_confirm_page(colman::AppData& data)
  {
    data.mw_.return_confirm_person_->setText(data.mw_.return_details_person_name_->text());
    data.mw_.return_confirm_item_->setText(data.mw_.return_details_item_name_->text());
    auto count = data.mw_.return_details_count_->value();
    auto* names = data.mw_.return_confirm_persons_;
    names->clear();
    if(is_multi(data)) {
      int const item_id =
          colman::selected_id(data.mw_.return_item_item_view_);
      auto const copies = open_copies_by_person(data, item_id);
      count = 0;
      for(auto const& person :
          data.mw_.return_borrower_person_list_->checked_persons())
      {
        int const n = copies.value(person.get_id());
        count += n;
        auto text = display_name(person);
        if(!person.get_school_class().isEmpty()) {
          text += " (" + person.get_school_class() + ")";
        }
        if(n > 1) {
          text += ": " + QApplication::tr("%n copies", "", n);
        }
        names->addItem(text);
      }
    }
    data.mw_.return_confirm_number_->setText(QString().setNum(count));
    data.mw_.return_confirm_datetime_->setText(QLocale().toString(QDateTime::currentDateTime(),
                                                                  QLocale::ShortFormat));
    data.mw_.return_confirm_remarks_->setPlainText(data.mw_.return_details_remarks_->toPlainText());
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_confirm_);
  }


  /**
     Inserts a new return record and returns its database id.

     writes a backup if needed.
     @param data Application state containing the database and undo stack.
     @param returning Data of the return to insert.
     @return Database id of the newly inserted return, or -1 on failure.
   */
  int add_new_return(colman::AppData& data, colman::Return const& returning)
  {
    colman::Return r = returning;
    r.set_data_collector_id(data.current_data_collector_);
    return data.queries_->add_new_return(r);
  }


  /**
   * Commits the return to the database and, on success, displays the done page
   * (completion page) which asks the user whether to record another return.
   * Also refreshes the item query so that the availability counts reflect the
   * newly recorded return. On failure a warning dialog is shown and the wizard
   * stays on the current page.
   * @param data Central application state.
   */
  void goto_done_page(colman::AppData& data)
  {
    if(is_multi(data)) {
      using Status = colman::IQueries::GroupReturnResult::Status;
      std::vector<int> person_ids;
      for(int const id : checked_person_ids(data)) {
        person_ids.push_back(id);
      }
      colman::Return tmpl;
      tmpl.set_date_time(QDateTime::currentDateTimeUtc());
      tmpl.set_remarks(data.mw_.return_details_remarks_->toPlainText());
      tmpl.set_data_collector_id(data.current_data_collector_);
      auto const result = data.queries_->add_group_return(
          colman::selected_id(data.mw_.return_item_item_view_), person_ids,
          tmpl);
      auto* parent = data.mw_.return_confirm_;
      auto const title = QApplication::tr("Collection Manager");
      switch(result.status_) {
      case Status::Ok:
        search_item(data);
        data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_done_);
        return;
      case Status::NotLent:
      {
        QString name = QString::number(result.person_id_);
        for(auto const& person :
            data.mw_.return_borrower_person_list_->checked_persons())
        {
          if(person.get_id() == result.person_id_) {
            name = display_name(person);
          }
        }
        QMessageBox::warning(
            parent, title,
            QApplication::tr("%1 has no open lending of this item.")
                .arg(name));
        goto_borrower_page(data);
        return;
      }
      case Status::Unsupported:
        QMessageBox::warning(
            parent, title,
            QApplication::tr("The server does not support group returns. "
                             "Please update the server."));
        return;
      case Status::Error:
        QMessageBox::warning(parent, title,
                             QApplication::tr("Could not return item."));
        return;
      }
      return;
    }
    colman::Return returning;
    int const
        lending_id = colman::selected_id(data.mw_.return_lending_lendings_view_);
    returning.set_lending_id(lending_id);
    returning.set_date_time(QDateTime::currentDateTimeUtc());
    returning.set_count(data.mw_.return_details_count_->value());
    returning.set_remarks(data.mw_.return_details_remarks_->toPlainText());
    int const return_id = ::add_new_return(data, returning);
    if(return_id > 0) {
      search_item(data);
      data.mw_.pages_stack_->setCurrentWidget(data.mw_.return_done_);
    }
    else {
      QMessageBox::warning(data.mw_.return_confirm_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("Could not return item."));
    }
  }


  /**
   * Exits the return wizard and returns to the central start page.
   * Clears the item and person query models.
   * @param data Central application state.
   */
  void leave_return_pages(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.return_item_model_.set_rows({});
    data.mw_.return_borrower_person_list_->set_persons({}, {});
    data.return_borrower_item_id_ = -1;
  }

} // namespace


/**
 * One-time initialisation of the return wizard pages. Sets up the sort
 * proxy models and column resize behaviour for the item and lending table
 * views, configures the item and person widgets as read-only, installs
 * ClearSelectionFilter event filters on both table viewports, and connects all
 * wizard navigation buttons (Next, Previous, Cancel, Finish, Yes/No on the
 * completion page) as well as the barcode search field and the main toolbar
 * Return button to their respective handler lambdas.
 * @param app  The Qt application object used for signal/slot connections.
 * @param data Central application state.
 */
void colman::setup_return_pages(QApplication& app, colman::AppData& data)
{
  data.mw_.return_borrower_person_->set_read_only(true);
  SortModel<2, 3>*
      sqlproxy2 = new SortModel<2, 3>(data.mw_.return_item_item_view_);
  sqlproxy2->setSourceModel(&data.return_item_model_);
  data.mw_.return_item_item_widget_->setup(data);
  data.mw_.return_item_item_widget_->set_read_only(true);
  data.mw_.return_item_item_view_->setModel(sqlproxy2);
  // Scroll smoothly (per pixel) so a wide name column can be
  // panned into view instead of jumping column by column.
  data.mw_.return_item_item_view_->setHorizontalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.return_item_item_view_->setVerticalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.return_item_item_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  data.mw_.return_item_item_view_->viewport()->installEventFilter(new ClearSelectionFilter(data.mw_.return_item_item_view_));
  data.mw_.return_details_datetime_->hide();
  data.mw_.label_169->hide();

  SortModel<2, 3>*
      sqlproxy3 = new SortModel<2, 3>(data.mw_.return_lending_lendings_view_);
  sqlproxy3->setSourceModel(&data.return_lending_model_);
  data.mw_.return_lending_lendings_view_->setModel(sqlproxy3);
  data.mw_.return_lending_lendings_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  data.mw_.return_lending_lendings_view_->viewport()->installEventFilter(new ClearSelectionFilter(data.mw_.return_lending_lendings_view_));

  app.connect(data.mw_.return_item_item_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const&,
                      QItemSelection const&) {
                int const item_id =
                    colman::selected_id(data.mw_.return_item_item_view_);
                show_item(data, item_id);
                data.mw_.return_item_next_->setEnabled(data.queries_->have_lendings(item_id));
              });
  app.connect(data.mw_.return_borrower_person_list_,
              &PersonCheckListWidget::current_changed,
              [&data](int person_id) { show_person(data, person_id); });
  app.connect(data.mw_.return_borrower_person_list_,
              &PersonCheckListWidget::checked_changed,
              [&data](int) {
                data.mw_.return_borrower_message_->clear();
                update_borrower_status(data);
              });
  app.connect(data.mw_.return_borrower_person_list_,
              &PersonCheckListWidget::search_return_pressed,
              [&data](QString const& text) {
                check_scanned_person(data, text);
              });
  app.connect(data.mw_.return_item_barcode_, &QLineEdit::editingFinished,
              [&data]() {
                if(data.mw_.return_item_barcode_->isModified()) {
                  data.mw_.return_item_barcode_->setModified(false);
                  search_item(data);
                }
              });
  data.mw_.return_item_camera_->setIcon(
      QIcon(":/icons/owls/scan_barcode.jpeg"));
  app.connect(data.mw_.return_item_camera_, &QToolButton::clicked,
              [&data]() {
                colman::scan_barcode_into(
                    data, data.mw_.return_item_barcode_);
              });
  if(QMediaDevices::videoInputs().isEmpty()) {
    data.mw_.return_item_camera_->setEnabled(false);
  }
  app.connect(data.mw_.return_lending_lendings_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const&,
                      QItemSelection const&) {
                int const lending_id =
                    colman::selected_id(data.mw_.return_lending_lendings_view_);
                data.mw_.return_lending_next_->setEnabled(lending_id > 0);
              });
  app.connect(data.mw_.return_item_next_, &QPushButton::clicked,
              [&data]() {
                goto_borrower_page_or_further(data);
              });
  app.connect(data.mw_.return_borrower_next_, &QPushButton::clicked,
              [&data]() {
                if(is_multi(data)) {
                  goto_details_page(data);
                }
                else {
                  goto_lending_page_or_further(data);
                }
              });
  app.connect(data.mw_.return_lending_next_, &QPushButton::clicked,
              [&data]() {
                goto_details_page(data);
              });
  app.connect(data.mw_.return_details_extend_, &QPushButton::clicked,
              [&data]() {
                int const person_id = single_person_id(data);
                int const item_id =
                    colman::selected_id(data.mw_.return_item_item_view_);
                auto* sel =
                    data.mw_.return_lending_lendings_view_->selectionModel();
                if(!sel || sel->selectedRows().isEmpty()) {
                  return;
                }
                // Only the lending chosen in step 3 is extended.
                QModelIndex const row = sel->selectedRows().front();
                colman::DueLendingListModel::Row lending;
                lending.lending_id = row.sibling(row.row(), 0).data().toInt();
                lending.item_id = item_id;
                lending.person_id = person_id;
                lending.duedate = row.sibling(row.row(), 4).data().toDate();
                lending.person_name =
                    data.mw_.return_details_person_name_->text();
                lending.outstanding_count =
                    row.sibling(row.row(), 1).data().toInt() -
                    row.sibling(row.row(), 2).data().toInt();
                lending.item_name = data.mw_.return_details_item_name_->text();
                int const lending_id = lending.lending_id;
                if(colman::extend_selected_lendings(
                       data.mw_.return_details_, data, {lending}))
                {
                  // show the new due date in the lending list (step 3)
                  data.return_lending_model_.set_rows(
                      data.queries_->query_open_lendings(person_id,
                                                         item_id));
                  colman::select_and_scroll_to(
                      data.mw_.return_lending_lendings_view_, 0,
                      lending_id);
                }
              });
  app.connect(data.mw_.return_details_next_, &QPushButton::clicked,
              [&data]() {
                goto_confirm_page(data);
              });
  app.connect(data.mw_.return_confirm_finish_, &QPushButton::clicked,
              [&data]() {
                goto_done_page(data);
              });
  app.connect(data.mw_.return_item_back_, &QPushButton::clicked,
              [&data]() { leave_return_pages(data); });
  app.connect(data.mw_.return_borrower_back_, &QPushButton::clicked,
              [&data]() { goto_item_page(data); });
  app.connect(data.mw_.return_lending_back_, &QPushButton::clicked,
              [&data]() {
                goto_borrower_page(data);
                if(is_person_known(data)) {
                  goto_item_page(data);
                }
              });
  app.connect(data.mw_.return_details_back_, &QPushButton::clicked,
              [&data]() {
                if(is_multi(data)) {
                  goto_borrower_page(data);
                  return;
                }
                goto_lending_page(data);
                if(is_lending_known(data)) {
                  goto_borrower_page(data);
                  if(is_person_known(data)) {
                    goto_item_page(data);
                  }
                }
              });
  app.connect(data.mw_.return_confirm_back_, &QPushButton::clicked,
              [&data]() { goto_details_page(data); });
  app.connect(data.mw_.return_done_back_, &QPushButton::clicked,
              [&data]() { leave_return_pages(data); });
  app.connect(data.mw_.return_done_no_, &QPushButton::clicked,
              [&data]() { leave_return_pages(data); });
  app.connect(data.mw_.return_done_yes_, &QPushButton::clicked,
              [&data]() {
                enter_return_page(data);
              });
  app.connect(data.return_button_, &QToolButton::clicked,
              [&data]() {
                enter_return_page(data);
              });
}


/**
 * Refreshes the currently visible return wizard page.
 * Determines which page is active and re-runs the appropriate
 * query or navigation function so that the displayed data reflects the current
 * database state.
 * @param data Central application state.
 */
void colman::refresh_return_pages(colman::AppData& data)
{
  QWidget const* const current = data.mw_.pages_stack_->currentWidget();
  if(current == data.mw_.return_item_) {
    goto_item_page(data);
    search_item(data);
  }
  else if(current == data.mw_.return_borrower_) {
    goto_borrower_page(data);
  }
  else if(current == data.mw_.return_lending_) {
    goto_lending_page(data);
  }
  else if(current == data.mw_.return_details_) {
    goto_details_page(data);
  }
  else if(current == data.mw_.return_confirm_) {
    goto_confirm_page(data);
  }
}

void colman::enter_return_page_for_action(colman::AppData& data,
                                          colman::ActionInfo const& ai)
{
  enter_return_page(data);
  if(!ai.barcode_.isEmpty()) {
    data.mw_.return_item_barcode_->setText(ai.barcode_);
  }
  if(ai.item_id_ >= 0) {
    colman::select_and_scroll_to(
        data.mw_.return_item_item_view_, 0, ai.item_id_);
  }
  if(ai.person_id_ >= 0) {
    load_borrowers(data);
    data.mw_.return_borrower_person_list_->check_person(ai.person_id_);
  }
  goto_lending_page(data);
  if(ai.lending_id_ >= 0) {
    colman::select_and_scroll_to(
        data.mw_.return_lending_lendings_view_, 0, ai.lending_id_);
    goto_details_page(data);
  }
}

/* EOF */
