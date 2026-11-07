// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// the group lending wizard
/**
   @file
   @author Christoph Pesch
*/

#include "colman_group_lending_page.hpp"
#ifndef COLMAN_CAMERA_SCAN_DIALOG_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_CLASS_NAMES_HPP
#  include "colman_class_names.hpp"
#endif
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
#endif
#ifndef COLMAN_PERSON_CHECK_LIST_WIDGET_HPP
#  include "colman_person_check_list_widget.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_SORT_MODEL_HPP
#  include "colman_sort_model.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QITEMSELECTION
#  include <QItemSelection>
#  define SYSTEM_QITEMSELECTION
#endif
#ifndef SYSTEM_QMEDIADEVICES
#  include <QMediaDevices>
#  define SYSTEM_QMEDIADEVICES
#endif
#ifndef SYSTEM_QCAMERADEVICE
#  include <QCameraDevice>
#  define SYSTEM_QCAMERADEVICE
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  include <QPageLayout>
#  include <QMarginsF>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QPRINTPREVIEWDIALOG
#  include <QPrintPreviewDialog>
#  define SYSTEM_QPRINTPREVIEWDIALOG
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QTEXTDOCUMENT
#  include <QTextDocument>
#  define SYSTEM_QTEXTDOCUMENT
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif


namespace
{
  /// Returns true when the logged-in user may record lendings.
  bool may_lend(colman::AppData& data)
  {
    int const uid = data.current_data_collector_;
    return uid == 0 ||
           (uid > 0 && data.queries_->has_permission(uid, "collect_lendings"));
  }


  /// Returns the id of the selected item, or -1.
  int selected_item_id(colman::AppData& data)
  {
    return colman::selected_id(data.mw_.group_lending_item_view_);
  }


  /// Returns the display name of \a person: "Lastname, Firstname".
  QString display_name(colman::Person const& person)
  {
    QString name = person.get_lastname();
    if(!name.isEmpty() && !person.get_firstname().isEmpty()) {
      name += ", ";
    }
    name += person.get_firstname();
    return name;
  }


  /// Updates view column visibility for the item table.
  void setup_item_columns(colman::AppData& data)
  {
    auto* view = data.mw_.group_lending_item_view_;
    view->setColumnHidden(0, true);
    bool const locale = data.queries_->is_locale_supported();
    view->setColumnHidden(4, !locale);
    view->setColumnHidden(5, locale);
    view->setColumnHidden(6, true);
  }


  /// Updates the item count label and load-more button visibility.
  void update_item_count(colman::AppData& data)
  {
    auto const n = data.group_lending_item_model_.rowCount();
    data.mw_.group_lending_item_count_->setText(
        QApplication::tr("%n item(s) shown", "", n));
    data.mw_.group_lending_item_load_more_->setVisible(
        data.group_lending_item_has_more_);
  }


  /**
     Loads one page of acquired items matching the search field.
     @param data   central application state.
     @param append true to append the next page, false to start anew.
  */
  void load_items(colman::AppData& data, bool append)
  {
    colman::AutoBlockSignals b(
        data.mw_.group_lending_item_view_->selectionModel());
    auto const lim = QSettings().value("search_threshold", 1000).toInt();
    if(append) {
      data.group_lending_item_offset_ += lim;
    }
    else {
      data.group_lending_item_offset_ = 0;
    }
    auto rows = data.queries_->query_acquired_items(
        -1, data.mw_.group_lending_item_barcode_->text(), lim,
        data.group_lending_item_offset_);
    data.group_lending_item_has_more_ =
        lim > 0 && static_cast<int>(rows.size()) > lim;
    if(data.group_lending_item_has_more_) {
      rows.pop_back();
    }
    if(append) {
      data.group_lending_item_model_.append_rows(std::move(rows));
    }
    else {
      data.group_lending_item_model_.set_rows(std::move(rows));
    }
    setup_item_columns(data);
    update_item_count(data);
  }


  /// Shows the selected item and enables "next" when it can be lent.
  void show_selected_item(colman::AppData& data)
  {
    int const item_id = selected_item_id(data);
    colman::Item item;
    QString type_str;
    if(item_id >= 0) {
      data.queries_->get_item(item_id, item, type_str);
    }
    data.mw_.group_lending_item_widget_->item_to_fields(item, type_str);
    data.mw_.group_lending_item_next_->setEnabled(
        item_id > 0 && may_lend(data) && data.queries_->available(item_id) > 0);
  }


  /// Displays the item selection page.
  void goto_item_page(colman::AppData& data)
  {
    show_selected_item(data);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.group_lending_item_);
    data.mw_.group_lending_item_barcode_->setFocus();
  }


  /// Returns the name of the selected item.
  QString selected_item_name(colman::AppData& data)
  {
    colman::Item item;
    QString type_str;
    data.queries_->get_item(selected_item_id(data), item, type_str);
    return item.get_name();
  }


  /**
     Updates the borrower count label and the "next" button.

     Borrowers holding an own reservation of the item use a reserved copy;
     every other borrower needs one of the free copies.
     @param data central application state.
  */
  void update_person_status(colman::AppData& data)
  {
    auto* list = data.mw_.group_lending_persons_list_;
    auto const ids = list->checked_ids();
    auto without_reservation = 0;
    for(int const id : ids) {
      if(!data.group_lending_reserving_ids_.contains(id)) {
        ++without_reservation;
      }
    }
    int const remaining =
        std::max(0, data.group_lending_free_copies_ - without_reservation);
    data.mw_.group_lending_persons_status_->setText(
        QApplication::tr("%n borrower(s) selected", "", ids.size()) + ", " +
        QApplication::tr("%n more copy(ies) available", "", remaining));
    data.mw_.group_lending_persons_next_->setEnabled(!ids.isEmpty());
  }


  /**
     Reads availability, reservations and current holders of the item and
     applies them to the borrower list.  Keeps the check states, but
     unchecks persons that meanwhile hold the item.
     @param data central application state.
  */
  void refresh_person_constraints(colman::AppData& data)
  {
    int const item_id = data.group_lending_persons_item_id_;
    auto* list = data.mw_.group_lending_persons_list_;
    QHash<int, QString> notes;
    QSet<int> holders;
    for(auto const& lending : data.queries_->open_lendings_for_item(item_id)) {
      holders.insert(lending.second);
      notes.insert(lending.second,
                   QApplication::tr("already has this item"));
    }
    int free = static_cast<int>(data.queries_->available(item_id));
    data.group_lending_reserving_ids_.clear();
    for(auto const& r : data.queries_->get_reservations_for_item(item_id)) {
      free -= r.count;
      data.group_lending_reserving_ids_.insert(r.person_id);
      if(!holders.contains(r.person_id)) {
        notes.insert(r.person_id,
                     QApplication::tr("has reserved %n copy(ies)", "",
                                      r.count));
      }
    }
    data.group_lending_free_copies_ = std::max(0, free);
    list->set_disabled_ids(holders);
    list->set_notes(notes);
    update_person_status(data);
  }


  /**
     Displays the borrower selection page.  The list is rebuilt when the
     item changed since the last visit; otherwise the selection is kept.
     @param data central application state.
  */
  void goto_persons_page(colman::AppData& data)
  {
    int const item_id = selected_item_id(data);
    if(item_id <= 0) {
      return;
    }
    auto* list = data.mw_.group_lending_persons_list_;
    if(item_id != data.group_lending_persons_item_id_) {
      data.group_lending_persons_item_id_ = item_id;
      auto const persons = data.queries_->query_persons(-1, "");
      list->set_persons(
          persons,
          colman::merge_class_names(data.queries_->load_class_names(),
                                    persons));
      list->clear_search_field();
    }
    data.mw_.group_lending_persons_item_name_->setText(
        selected_item_name(data));
    data.mw_.group_lending_persons_message_->clear();
    refresh_person_constraints(data);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.group_lending_persons_);
    list->focus_search_field();
  }


  /// Returns the vetoing guard for checking another borrower.
  /**
     A borrower with an own reservation is always allowed; any other
     borrower only while free copies are left.
     @param data central application state.
     @param person_id the borrower to check
     @return true if the borrower may be checked
  */
  bool may_check_person(colman::AppData& data, int person_id)
  {
    if(data.group_lending_reserving_ids_.contains(person_id)) {
      return true;
    }
    auto without_reservation = 0;
    for(int const id : data.mw_.group_lending_persons_list_->checked_ids()) {
      if(!data.group_lending_reserving_ids_.contains(id)) {
        ++without_reservation;
      }
    }
    return without_reservation < data.group_lending_free_copies_;
  }


  /**
     Checks the borrower whose library card was scanned into the search
     field and reports the outcome below the list.
     @param data central application state.
     @param text the search field text
  */
  void check_scanned_person(colman::AppData& data, QString const& text)
  {
    using CheckResult = colman::PersonCheckListWidget::CheckResult;
    auto const code = text.trimmed();
    if(code.isEmpty() || !data.queries_->is_person_barcode(code)) {
      return;
    }
    auto* list = data.mw_.group_lending_persons_list_;
    auto* message = data.mw_.group_lending_persons_message_;
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
    case CheckResult::Disabled:
      message->setText(QApplication::tr("%1 already has this item.").arg(name));
      QApplication::beep();
      break;
    case CheckResult::Refused:
      message->setText(
          QApplication::tr("No copy left for %1.").arg(name));
      QApplication::beep();
      break;
    case CheckResult::Unknown:
      message->setText(QApplication::tr("Unknown library card."));
      QApplication::beep();
      break;
    }
  }


  /// Displays the details page with the default due date preset.
  void goto_details_page(colman::AppData& data, bool reset_due_date)
  {
    data.mw_.group_lending_details_item_name_->setText(
        selected_item_name(data));
    data.mw_.group_lending_details_persons_->setText(QString::number(
        data.mw_.group_lending_persons_list_->checked_count()));
    if(reset_due_date) {
      auto const col = data.queries_->read_collection();
      bool const use_period = col.get_use_lending_period();
      auto due_date = QDate::currentDate();
      if(use_period) {
        due_date = col.add_lending_period(due_date);
      }
      data.mw_.group_lending_details_due_date_->setDate(due_date);
      data.mw_.group_lending_details_due_date_check_->setChecked(use_period);
      data.mw_.group_lending_details_due_date_->setEnabled(use_period);
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.group_lending_details_);
  }


  /// Returns the due date text, empty when lending without due date.
  QString due_date_text(colman::AppData& data)
  {
    if(!data.mw_.group_lending_details_due_date_check_->isChecked()) {
      return QString();
    }
    return data.mw_.group_lending_details_due_date_->date().toString(
        QApplication::tr("yyyy-MM-dd"));
  }


  /// Displays the confirmation page.
  void goto_confirm_page(colman::AppData& data)
  {
    auto const persons =
        data.mw_.group_lending_persons_list_->checked_persons();
    data.mw_.group_lending_confirm_item_->setText(selected_item_name(data));
    data.mw_.group_lending_confirm_number_->setText(
        QString::number(persons.size()));
    data.mw_.group_lending_confirm_due_date_->setText(due_date_text(data));
    data.mw_.group_lending_confirm_remarks_->setPlainText(
        data.mw_.group_lending_details_remarks_->toPlainText());
    auto* list = data.mw_.group_lending_confirm_persons_;
    list->clear();
    for(auto const& person : persons) {
      auto text = display_name(person);
      if(!person.get_school_class().isEmpty()) {
        text += " (" + person.get_school_class() + ")";
      }
      list->addItem(text);
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.group_lending_confirm_);
  }


  /// Returns the display name of the checked person \a person_id.
  QString checked_person_name(colman::AppData& data, int person_id)
  {
    for(auto const& person :
        data.mw_.group_lending_persons_list_->checked_persons())
    {
      if(person.get_id() == person_id) {
        return display_name(person);
      }
    }
    return QString::number(person_id);
  }


  /**
     Records the lendings and shows the done page.  On a conflict (e.g.
     copies lent meanwhile at another workstation) the user is told why
     and returned to the borrower page.
     @param data central application state.
  */
  void finish(colman::AppData& data)
  {
    using Status = colman::IQueries::GroupLendingResult::Status;
    auto* list = data.mw_.group_lending_persons_list_;
    std::vector<int> person_ids;
    for(int const id : list->checked_ids()) {
      person_ids.push_back(id);
    }
    colman::Lending lending;
    lending.set_item_id(selected_item_id(data));
    lending.set_date_time(QDateTime::currentDateTimeUtc());
    if(data.mw_.group_lending_details_due_date_check_->isChecked()) {
      lending.set_duedate(data.mw_.group_lending_details_due_date_->date());
    }
    lending.set_remarks(data.mw_.group_lending_details_remarks_->toPlainText());
    lending.set_data_collector_id(data.current_data_collector_);
    auto const result = data.queries_->add_group_lending(lending, person_ids);
    auto* parent = data.mw_.group_lending_confirm_;
    auto const title = QApplication::tr("Collection Manager");
    switch(result.status_) {
    case Status::Ok:
      data.mw_.group_lending_done_message_->setText(
          QApplication::tr("%n lending(s) of \"%1\" recorded.", "",
                           static_cast<int>(result.lending_ids_.size()))
              .arg(selected_item_name(data)));
      load_items(data, false);
      data.mw_.pages_stack_->setCurrentWidget(data.mw_.group_lending_done_);
      return;
    case Status::NotEnoughAvailable:
      QMessageBox::warning(
          parent, title,
          QApplication::tr("Not enough copies available. "
                           "%n copy(ies) can be lent to this group.",
                           "", result.lendable_));
      goto_persons_page(data);
      return;
    case Status::AlreadyLent:
      QMessageBox::warning(
          parent, title,
          QApplication::tr("%1 already has this item.")
              .arg(checked_person_name(data, result.person_id_)));
      goto_persons_page(data);
      return;
    case Status::Unsupported:
      QMessageBox::warning(
          parent, title,
          QApplication::tr("The server does not support group lendings. "
                           "Please update the server."));
      return;
    case Status::Error:
      QMessageBox::warning(parent, title,
                           QApplication::tr("Could not lend item."));
      return;
    }
  }


  /// Prints a hand-out list with a signature column for each borrower.
  void print_handout_list(colman::AppData& data)
  {
    auto const persons =
        data.mw_.group_lending_persons_list_->checked_persons();
    auto const col = data.queries_->read_collection();
    QString html;
    html += "<h2>" + QApplication::tr("Hand-out list").toHtmlEscaped() +
            "</h2>";
    html += "<p>" + col.get_name().toHtmlEscaped() + "<br>";
    html += QApplication::tr("Item: %1")
                .arg(selected_item_name(data))
                .toHtmlEscaped() +
            "<br>";
    html += QApplication::tr("Date: %1")
                .arg(QDate::currentDate().toString(
                    QApplication::tr("yyyy-MM-dd")))
                .toHtmlEscaped();
    auto const due = due_date_text(data);
    if(!due.isEmpty()) {
      html += "<br>" +
              QApplication::tr("Due date: %1").arg(due).toHtmlEscaped();
    }
    html += "</p>";
    html += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"6\" "
            "width=\"100%\"><tr>";
    for(auto const& header :
        {QApplication::tr("No."), QApplication::tr("Last name"),
         QApplication::tr("First name"), QApplication::tr("Class"),
         QApplication::tr("Signature")})
    {
      html += "<th align=\"left\">" + header.toHtmlEscaped() + "</th>";
    }
    html += "</tr>";
    auto number = 0;
    for(auto const& person : persons) {
      ++number;
      html += "<tr><td>" + QString::number(number) + "</td><td>" +
              person.get_lastname().toHtmlEscaped() + "</td><td>" +
              person.get_firstname().toHtmlEscaped() + "</td><td>" +
              person.get_school_class().toHtmlEscaped() +
              "</td><td width=\"35%\">&nbsp;</td></tr>";
    }
    html += "</table>";
    QTextDocument doc;
    doc.setHtml(html);
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageMargins(QMarginsF(20, 20, 20, 20),
                           QPageLayout::Millimeter);
    QPrintPreviewDialog dialog(&printer, data.mw_.group_lending_done_);
    dialog.connect(&dialog, &QPrintPreviewDialog::paintRequested,
                   [&doc](QPrinter* p) { doc.print(p); });
    dialog.exec();
  }


  /// Leaves the wizard and returns to the central page.
  void leave_group_lending_pages(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.group_lending_item_model_.set_rows({});
    data.group_lending_persons_item_id_ = -1;
    data.mw_.group_lending_persons_list_->set_persons({}, {});
  }

} // namespace


/**
   Resets all fields of the group lending wizard and shows the item page.
   @param data central application state.
*/
void colman::enter_group_lending_page(colman::AppData& data)
{
  {
    colman::AutoBlockSignals b1(data.mw_.group_lending_item_barcode_);
    colman::AutoBlockSignals b2(data.mw_.group_lending_details_remarks_);
    data.mw_.group_lending_item_barcode_->clear();
    data.mw_.group_lending_details_remarks_->clear();
  }
  data.group_lending_persons_item_id_ = -1;
  data.mw_.group_lending_persons_list_->set_persons({}, {});
  data.mw_.group_lending_item_widget_->reset_item_type();
  data.mw_.group_lending_item_view_->clearSelection();
  load_items(data, false);
  goto_item_page(data);
}


/**
   Initializes the group lending pages; to be called once at program start.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_group_lending_pages(QApplication& app,
                                       colman::AppData& data)
{
  auto& mw = data.mw_;
  data.group_lending_item_model_.set_grayout_unavailable(true);
  auto* proxy = new SortModel<2, 3>(mw.group_lending_item_view_);
  proxy->setSourceModel(&data.group_lending_item_model_);
  mw.group_lending_item_widget_->setup(data);
  mw.group_lending_item_widget_->set_read_only(true);
  mw.group_lending_item_view_->setModel(proxy);
  mw.group_lending_item_view_->setHorizontalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  mw.group_lending_item_view_->setVerticalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  mw.group_lending_item_view_->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  mw.group_lending_item_view_->viewport()->installEventFilter(
      new ClearSelectionFilter(mw.group_lending_item_view_));
  mw.group_lending_item_camera_->setIcon(
      QIcon(":/icons/owls/scan_barcode.jpeg"));
  if(QMediaDevices::videoInputs().isEmpty()) {
    mw.group_lending_item_camera_->setEnabled(false);
  }
  mw.group_lending_persons_list_->set_check_guard(
      [&data](int person_id) { return may_check_person(data, person_id); });

  // Item page
  app.connect(mw.group_lending_item_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const&, QItemSelection const&) {
                show_selected_item(data);
              });
  app.connect(mw.group_lending_item_barcode_, &QLineEdit::editingFinished,
              [&data]() {
                auto* field = data.mw_.group_lending_item_barcode_;
                if(!field->isModified()) {
                  return;
                }
                field->setModified(false);
                load_items(data, false);
                int const item_id = colman::select_if_single_row(
                    data.mw_.group_lending_item_view_);
                show_selected_item(data);
                if(item_id > 0 &&
                   data.mw_.group_lending_item_next_->isEnabled() &&
                   colman::is_item_barcode(data.queries_->read_collection(),
                                           field->text()))
                {
                  goto_persons_page(data);
                }
              });
  app.connect(mw.group_lending_item_camera_, &QToolButton::clicked,
              [&data]() {
                colman::scan_barcode_into(
                    data, data.mw_.group_lending_item_barcode_);
              });
  app.connect(mw.group_lending_item_load_more_, &QPushButton::clicked,
              [&data]() { load_items(data, true); });
  app.connect(mw.group_lending_item_back_, &QPushButton::clicked,
              [&data]() { leave_group_lending_pages(data); });
  app.connect(mw.group_lending_item_next_, &QPushButton::clicked,
              [&data]() { goto_persons_page(data); });

  // Borrower page
  app.connect(mw.group_lending_persons_list_,
              &PersonCheckListWidget::checked_changed,
              [&data](int) {
                data.mw_.group_lending_persons_message_->clear();
                update_person_status(data);
              });
  app.connect(mw.group_lending_persons_list_,
              &PersonCheckListWidget::check_refused,
              [&data](int person_id) {
                auto* message = data.mw_.group_lending_persons_message_;
                if(person_id < 0) {
                  message->setText(QApplication::tr(
                      "Not all persons could be selected: "
                      "no more copies available."));
                }
                else {
                  message->setText(
                      QApplication::tr("No more copies available."));
                }
                QApplication::beep();
              });
  app.connect(mw.group_lending_persons_list_,
              &PersonCheckListWidget::search_return_pressed,
              [&data](QString const& text) {
                check_scanned_person(data, text);
              });
  app.connect(mw.group_lending_persons_back_, &QPushButton::clicked,
              [&data]() { goto_item_page(data); });
  app.connect(mw.group_lending_persons_next_, &QPushButton::clicked,
              [&data]() { goto_details_page(data, true); });

  // Details, confirmation and done pages
  app.connect(mw.group_lending_details_back_, &QPushButton::clicked,
              [&data]() { goto_persons_page(data); });
  app.connect(mw.group_lending_details_next_, &QPushButton::clicked,
              [&data]() { goto_confirm_page(data); });
  app.connect(mw.group_lending_confirm_back_, &QPushButton::clicked,
              [&data]() { goto_details_page(data, false); });
  app.connect(mw.group_lending_confirm_finish_, &QPushButton::clicked,
              [&data]() { finish(data); });
  app.connect(mw.group_lending_done_print_, &QPushButton::clicked,
              [&data]() { print_handout_list(data); });
  app.connect(mw.group_lending_done_again_, &QPushButton::clicked,
              [&data]() { enter_group_lending_page(data); });
  app.connect(mw.group_lending_done_back_, &QPushButton::clicked,
              [&data]() { leave_group_lending_pages(data); });
  app.connect(mw.group_lending_done_close_, &QPushButton::clicked,
              [&data]() { leave_group_lending_pages(data); });
  app.connect(data.group_lend_button_, &QToolButton::clicked,
              [&data]() { enter_group_lending_page(data); });
}


/**
   Refreshes the currently visible group lending page.
   @param data central application state.
*/
void colman::refresh_group_lending_pages(colman::AppData& data)
{
  QWidget const* const current = data.mw_.pages_stack_->currentWidget();
  if(current == data.mw_.group_lending_item_) {
    int const item_id = selected_item_id(data);
    load_items(data, false);
    if(item_id > 0) {
      colman::select_and_scroll_to(data.mw_.group_lending_item_view_, 0,
                                   item_id);
    }
    show_selected_item(data);
  }
  else if(current == data.mw_.group_lending_persons_) {
    refresh_person_constraints(data);
  }
}


/* EOF */
