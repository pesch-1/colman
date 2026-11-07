// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// implementation of history page setup
/**
   @file
   @author Christoph Pesch
*/

#include "colman_history_page.hpp"
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_ITEM_WIDGET_HPP
#  include "colman_item_widget.hpp"
#endif
#ifndef COLMAN_PERSON_WIDGET_HPP
#  include "colman_person_widget.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QTABWIDGET
#  include <QTabWidget>
#  define SYSTEM_QTABWIDGET
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_MAP
#  include <map>
#  define SYSTEM_MAP
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QDATETIMEEDIT
#  include <QDateTimeEdit>
#  define SYSTEM_QDATETIMEEDIT
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif


namespace
{
  /// Default: show the last 3 months.
  int constexpr DefaultHistoryMonths = 3;

  void update_history_count(colman::AppData& data);
  void history_load_more(colman::AppData& data);

  /// Updates history table column visibility based on admin status.
  void setup_history_columns(colman::AppData& data, bool is_admin)
  {
    data.mw_.combined_history_table_->setColumnHidden(0, true);
    data.mw_.combined_history_table_->setColumnHidden(6, !is_admin);
    data.mw_.combined_history_table_->setColumnHidden(7, true);
    data.mw_.combined_history_table_->setColumnHidden(8, true);
    data.mw_.combined_history_table_->setColumnHidden(9, true);
    data.mw_.combined_history_table_->setColumnHidden(10, true);
  }


  /// Updates the history count label and load-more button visibility.
  void update_history_count(colman::AppData& data)
  {
    auto const n = data.combined_history_model_.rowCount();
    data.mw_.history_count_label_->setText(
        QApplication::tr("%n action(s) shown", "", n));
    data.mw_.history_load_more_->setVisible(data.history_has_more_);
  }


  /**
     Reloads the combined history query using the current date range
     and populates the table.
     @param data central application state.
  */
  void reset_history_query(colman::AppData& data)
  {
    bool const is_admin = (data.current_data_collector_ == 0);
    bool const show_all = data.mw_.history_show_all_->isChecked();
    data.mw_.history_from_label_->setEnabled(!show_all);
    data.mw_.history_from_date_->setEnabled(!show_all);
    data.mw_.history_to_label_->setEnabled(!show_all);
    data.mw_.history_to_date_->setEnabled(!show_all);
    data.mw_.history_apply_button_->setEnabled(!show_all);
    QDate const from = data.mw_.history_from_date_->date();
    QDate const to = data.mw_.history_to_date_->date();
    data.history_offset_ = 0;
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    auto const action =
        data.mw_.history_action_filter_->currentData().toString();
    auto rows = data.queries_->query_combined_history(
        is_admin, from, to, show_all, lim, 0, action);
    data.history_has_more_ =
        lim > 0 && static_cast<int>(rows.size()) > lim;
    if(data.history_has_more_) {
      rows.pop_back();
    }
    data.combined_history_model_.set_rows(std::move(rows));
    setup_history_columns(data, is_admin);
    update_history_count(data);
  }


  /// Loads the next page of history rows and appends them to the model.
  void history_load_more(colman::AppData& data)
  {
    bool const is_admin = (data.current_data_collector_ == 0);
    bool const show_all = data.mw_.history_show_all_->isChecked();
    QDate const from = data.mw_.history_from_date_->date();
    QDate const to = data.mw_.history_to_date_->date();
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    data.history_offset_ += lim;
    auto const action =
        data.mw_.history_action_filter_->currentData().toString();
    auto rows = data.queries_->query_combined_history(
        is_admin, from, to, show_all, lim, data.history_offset_, action);
    data.history_has_more_ = static_cast<int>(rows.size()) > lim;
    if(data.history_has_more_) {
      rows.pop_back();
    }
    data.combined_history_model_.append_rows(std::move(rows));
    update_history_count(data);
  }


  /**
     Leaves the history page: switches to the central page and clears
     the combined history model.
     @param data central application state.
  */
  void leave_history_page(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.combined_history_model_.set_rows({});
  }


  using ActionKey = std::pair<QString, int>;
  using ActionMap = std::map<ActionKey, colman::HistoryCombinedModel::Row>;

  /**
     Recursively collects \a current and all actions that conflict with its
     deletion into \a result.  Already-visited rows are skipped.
     @param queries SQL back-end.
     @param current action being examined.
     @param result  accumulator for all rows that must be deleted.
  */
  void collect_conflicts(
      colman::IQueries* queries,
      colman::HistoryCombinedModel::Row const& current,
      ActionMap& result)
  {
    ActionKey const key{current.action, current.id};
    if(result.count(key)) {
      return;
    }
    result[key] = current;
    auto const conflicts = queries->find_conflicting_actions(current);
    for(auto const& c : conflicts) {
      collect_conflicts(queries, c, result);
    }
  }


  /**
     Returns the translated display name for an action type string.
     @param action raw action type ("acquisition", "lending", etc.)
  */
  QString action_display_name(QString const& action)
  {
    if(action == "acquisition") {
      return QApplication::tr("acquisition");
    }
    if(action == "liquidation") {
      return QApplication::tr("liquidation");
    }
    if(action == "lending") {
      return QApplication::tr("lending");
    }
    if(action == "return") {
      return QApplication::tr("return");
    }
    if(action == "reservation") {
      return QApplication::tr("reservation");
    }
    return action;
  }


  /**
     Reads the full history row of the table entry at \a idx.
     @param idx index of any cell in the row.
     @param[out] row populated on success.
     @return true if the row could be read.
  */
  bool history_row_at(QModelIndex const& idx,
                      colman::HistoryCombinedModel::Row& row)
  {
    row.id = idx.sibling(idx.row(), 0).data().toInt();
    row.datetime = idx.sibling(idx.row(), 1).data().toDateTime();
    row.action = idx.sibling(idx.row(), 2)
                     .data(Qt::UserRole)
                     .toString();
    row.item_name = idx.sibling(idx.row(), 3).data().toString();
    row.count = idx.sibling(idx.row(), 4).data().toInt();
    row.user_name = idx.sibling(idx.row(), 5).data().toString();
    row.collector = idx.sibling(idx.row(), 6).data().toString();
    row.item_id = idx.sibling(idx.row(), 7).data().toInt();
    row.person_id = idx.sibling(idx.row(), 8).data().toInt();
    row.remarks = idx.sibling(idx.row(), 9).data().toString();
    row.duedate = idx.sibling(idx.row(), 10).data().toDate();
    return row.id > 0 && !row.action.isEmpty();
  }


  /**
     Reads the full history row for the currently selected table entry.
     With several selected rows, the first one is returned.
     @param data central application state.
     @param[out] row populated on success.
     @return true if a row was selected and could be read.
  */
  bool selected_history_row(
      colman::AppData& data,
      colman::HistoryCombinedModel::Row& row)
  {
    auto const& sm = data.mw_.combined_history_table_->selectionModel();
    auto const rows = sm->selectedRows();
    if(rows.isEmpty()) {
      return false;
    }
    return history_row_at(rows.front(), row);
  }


  /**
     Reads the full history rows of all selected table entries.
     @param data central application state.
     @return the selected rows (rows that cannot be read are skipped).
  */
  std::vector<colman::HistoryCombinedModel::Row> selected_history_rows(
      colman::AppData& data)
  {
    std::vector<colman::HistoryCombinedModel::Row> result;
    auto const& sm = data.mw_.combined_history_table_->selectionModel();
    for(auto const& idx : sm->selectedRows()) {
      colman::HistoryCombinedModel::Row row;
      if(history_row_at(idx, row)) {
        result.push_back(row);
      }
    }
    return result;
  }


  /**
     Handles a click on the delete button: confirms with the user, resolves
     conflicts recursively, and deletes all selected actions together with
     the actions depending on them in one transaction.
     @param data central application state.
  */
  void delete_selected_actions(colman::AppData& data)
  {
    auto const selected = selected_history_rows(data);
    if(selected.empty()) {
      return;
    }

    ActionMap to_delete_map;
    for(auto const& row : selected) {
      collect_conflicts(data.queries_.get(), row, to_delete_map);
    }

    std::vector<colman::HistoryCombinedModel::Row> to_delete;
    to_delete.reserve(to_delete_map.size());
    for(auto const& p : to_delete_map) {
      to_delete.push_back(p.second);
    }
    std::sort(to_delete.begin(), to_delete.end(),
              [](colman::HistoryCombinedModel::Row const& a,
                 colman::HistoryCombinedModel::Row const& b) {
                return a.datetime < b.datetime;
              });

    bool confirmed = false;

    if(to_delete.size() == 1) {
      auto const reply = colman::question_yes_no(
          data.mw_.history_page_,
          QApplication::tr("Delete Action"),
          QApplication::tr("Delete this action permanently?"));
      confirmed = (reply == QMessageBox::Yes);
    }
    else {
      int const n_selected = static_cast<int>(selected.size());
      int const n_conflicts =
          static_cast<int>(to_delete.size()) - n_selected;
      QString header_text;
      if(n_selected == 1) {
        header_text = QApplication::tr(
            "Deleting this action requires also deleting "
            "%n dependent action(s):",
            nullptr, n_conflicts);
      }
      else if(n_conflicts == 0) {
        header_text = QApplication::tr(
            "The following %n selected action(s) will be deleted:",
            nullptr, n_selected);
      }
      else {
        header_text =
            QApplication::tr("Deleting the %n selected action(s) requires",
                             nullptr, n_selected) +
            ' ' +
            QApplication::tr("also deleting %n dependent action(s):",
                             nullptr, n_conflicts);
      }
      QDialog dlg(data.mw_.history_page_);
      dlg.setWindowTitle(QApplication::tr("Delete Action"));
      dlg.resize(660, 420);
      QVBoxLayout* layout = new QVBoxLayout(&dlg);

      {
        QHBoxLayout* header = new QHBoxLayout();
        QLabel* icon_label = new QLabel();
        icon_label->setPixmap(
            QPixmap(":/icons/owls/warning.jpeg")
                .scaled(64, 64, Qt::KeepAspectRatio,
                        Qt::SmoothTransformation));
        header->addWidget(icon_label);
        auto* header_label = new QLabel(header_text);
        header_label->setWordWrap(true);
        header->addWidget(header_label, 1);
        layout->addLayout(header);
      }

      QTableWidget* table =
          new QTableWidget(static_cast<int>(to_delete.size()), 4);
      table->setHorizontalHeaderLabels(
          {QApplication::tr("date/time"),
           QApplication::tr("action"),
           QApplication::tr("name"),
           QApplication::tr("user")});
      table->horizontalHeader()
          ->setSectionResizeMode(QHeaderView::ResizeToContents);
      table->setEditTriggers(QAbstractItemView::NoEditTriggers);
      table->setSelectionMode(QAbstractItemView::NoSelection);

      int row_idx = 0;
      for(auto const& row : to_delete) {
        table->setItem(row_idx, 0, new QTableWidgetItem(row.datetime.toLocalTime().toString("yyyy-MM-dd HH:mm")));
        table->setItem(row_idx, 1, new QTableWidgetItem(action_display_name(row.action)));
        table->setItem(row_idx, 2, new QTableWidgetItem(row.item_name));
        table->setItem(row_idx, 3, new QTableWidgetItem(row.user_name));
        ++row_idx;
      }
      layout->addWidget(table);

      layout->addWidget(new QLabel(
          QApplication::tr(
              "Delete all %1 actions permanently?")
              .arg(static_cast<int>(to_delete.size()))));

      QHBoxLayout* btn_layout = new QHBoxLayout();
      QPushButton* ok_btn =
          new QPushButton(QApplication::tr("Delete All"));
      QPushButton* cancel_btn =
          new QPushButton(QApplication::tr("Cancel"));
      btn_layout->addStretch();
      btn_layout->addWidget(cancel_btn);
      btn_layout->addWidget(ok_btn);
      layout->addLayout(btn_layout);

      QObject::connect(ok_btn, &QPushButton::clicked,
                       &dlg, &QDialog::accept);
      QObject::connect(cancel_btn, &QPushButton::clicked,
                       &dlg, &QDialog::reject);

      confirmed = (dlg.exec() == QDialog::Accepted);
    }

    if(!confirmed) {
      return;
    }

    if(!data.queries_->batch_delete_history_actions(to_delete)) {
      QMessageBox::warning(
          data.mw_.history_page_,
          QApplication::tr("Error"),
          QApplication::tr("Failed to delete the selected actions."));
      return;
    }
    refresh_history_page(data);
  }


  /**
     Shows a modal dialog with full details for the selected history row.
     @param data central application state.
  */
  void show_action_details(colman::AppData& data)
  {
    colman::HistoryCombinedModel::Row selected;
    if(!selected_history_row(data, selected)) {
      return;
    }

    QModelIndex const idx =
        data.mw_.combined_history_table_->selectionModel()
            ->selectedRows()
            .front();
    QString const action = idx.sibling(idx.row(), 2).data().toString();

    bool const is_lending_or_return =
        selected.action == "lending" || selected.action == "return" ||
        selected.action == "reservation";

    colman::Item item;
    QString type_name;
    if(selected.item_id > 0) {
      data.queries_->get_item(selected.item_id, item, type_name);
    }

    colman::Person person;
    if(is_lending_or_return && selected.person_id > 0) {
      data.queries_->get_person(selected.person_id, person);
    }

    QDialog dlg(data.mw_.history_page_);
    dlg.setWindowTitle(QApplication::tr("Action Details"));
    dlg.resize(550, 600);
    QVBoxLayout* main_layout = new QVBoxLayout(&dlg);

    QFormLayout* form = new QFormLayout();
    form->addRow(QApplication::tr("date/time:"),
                 new QLabel(selected.datetime.toLocalTime().toString("yyyy-MM-dd HH:mm:ss")));
    form->addRow(QApplication::tr("action:"), new QLabel(action));
    form->addRow(QApplication::tr("number:"),
                 new QLabel(QString().setNum(selected.count)));
    if(!selected.user_name.isEmpty()) {
      form->addRow(QApplication::tr("user:"), new QLabel(selected.user_name));
    }
    if(!selected.collector.isEmpty()) {
      form->addRow(QApplication::tr("data collector:"),
                   new QLabel(selected.collector));
    }
    if(!selected.remarks.isEmpty()) {
      form->addRow(QApplication::tr("remarks:"),
                   new QLabel(selected.remarks));
    }
    main_layout->addLayout(form);

    QTabWidget* tabs = new QTabWidget();
    colman::ItemWidget* item_widget = new colman::ItemWidget();
    item_widget->setup(data);
    item_widget->set_read_only(true);
    item_widget->item_to_fields(item, type_name);
    tabs->addTab(item_widget, QApplication::tr("Item"));

    if(is_lending_or_return && selected.person_id > 0) {
      colman::PersonWidget* person_widget = new colman::PersonWidget();
      person_widget->person_to_fields(person);
      person_widget->set_read_only(true);
      tabs->addTab(person_widget, QApplication::tr("Person"));
    }
    main_layout->addWidget(tabs);

    QPushButton* close_btn =
        new QPushButton(QApplication::tr("Close"));
    QObject::connect(close_btn, &QPushButton::clicked,
                     &dlg, &QDialog::accept);
    main_layout->addWidget(close_btn);

    dlg.exec();
  }


  /**
     Returns a short explanation of why \a conflict prevents applying the
     change from \a changed to \a new_action values.
     @param changed    the original stored row being modified.
     @param new_action the proposed new values (count, datetime).
     @param conflict   the row that makes the change impossible.
  */
  QString reason_for_conflict(
      colman::HistoryCombinedModel::Row const& changed,
      colman::HistoryCombinedModel::Row const& new_action,
      colman::HistoryCombinedModel::Row const& conflict)
  {
    QString const& cf = conflict.action;
    if(cf == "acquisition") {
      return QApplication::tr(
          "Not enough copies acquired for the new count");
    }
    if(cf == "lending") {
      if(changed.action == "return") {
        if(conflict.datetime >= new_action.datetime) {
          return QApplication::tr(
              "The return would precede its parent lending at the new time");
        }
        return QApplication::tr(
            "The return count would exceed the outstanding copies of the lending");
      }
      return QApplication::tr(
          "This later lending requires more copies than would be available");
    }
    if(cf == "liquidation") {
      return QApplication::tr(
          "This later liquidation requires more copies than would be available");
    }
    if(cf == "return") {
      if(conflict.datetime <= new_action.datetime) {
        return QApplication::tr(
            "This return would precede the lending at its new time");
      }
      return QApplication::tr(
          "The total returns of this lending would exceed its new count");
    }
    return {};
  }


  /**
     Shows a read-only conflict table telling the user why the change
     cannot be applied.  \a changed_action is always shown first with a
     coloured background; all conflicting rows follow with a reason column.
     @param parent         parent widget for the dialog.
     @param changed_action the action the user tried to modify.
     @param new_action     the proposed new values (for reason inference).
     @param conflicts      rows that would be violated by the change.
  */
  void show_change_conflict_dialog(
      QWidget* parent,
      colman::HistoryCombinedModel::Row const& changed_action,
      colman::HistoryCombinedModel::Row const& new_action,
      std::vector<colman::HistoryCombinedModel::Row> const& conflicts)
  {
    // Build display list: changed action first, then conflicts (skip duplicate).
    std::vector<colman::HistoryCombinedModel::Row> rows;
    rows.push_back(changed_action);
    for(auto const& c : conflicts) {
      if(c.action != changed_action.action || c.id != changed_action.id) {
        rows.push_back(c);
      }
    }

    QDialog dlg(parent);
    dlg.setWindowTitle(QApplication::tr("Change Action"));
    dlg.resize(820, 380);
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    {
      QHBoxLayout* header = new QHBoxLayout();
      QLabel* icon_label = new QLabel();
      icon_label->setPixmap(
          QPixmap(":/icons/owls/warning.jpeg")
              .scaled(64, 64, Qt::KeepAspectRatio,
                      Qt::SmoothTransformation));
      header->addWidget(icon_label);
      header->addWidget(new QLabel(QApplication::tr(
                            "This change cannot be applied. The highlighted row shows the "
                            "changed action; the other rows are the conflicts:")),
                        1);
      layout->addLayout(header);
    }

    QTableWidget* table =
        new QTableWidget(static_cast<int>(rows.size()), 6);
    table->setHorizontalHeaderLabels(
        {QApplication::tr("date/time"),
         QApplication::tr("action"),
         QApplication::tr("name"),
         QApplication::tr("number"),
         QApplication::tr("user"),
         QApplication::tr("reason")});
    table->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    int row_idx = 0;
    for(auto const& row : rows) {
      bool const is_changed =
          (row.action == changed_action.action && row.id == changed_action.id);
      table->setItem(row_idx, 0, new QTableWidgetItem(row.datetime.toLocalTime().toString("yyyy-MM-dd HH:mm")));
      table->setItem(row_idx, 1, new QTableWidgetItem(action_display_name(row.action)));
      table->setItem(row_idx, 2, new QTableWidgetItem(row.item_name));
      QTableWidgetItem* count_item =
          new QTableWidgetItem(QString::number(row.count));
      count_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      table->setItem(row_idx, 3, count_item);
      table->setItem(row_idx, 4, new QTableWidgetItem(row.user_name));
      QString const reason = is_changed
                                 ? QString()
                                 : reason_for_conflict(changed_action, new_action, row);
      table->setItem(row_idx, 5, new QTableWidgetItem(reason));
      if(is_changed) {
        QColor const highlight(255, 200, 100);
        for(int col = 0; col < 6; ++col) {
          table->item(row_idx, col)->setBackground(highlight);
        }
      }
      ++row_idx;
    }
    layout->addWidget(table);

    QPushButton* back_btn = new QPushButton(QApplication::tr("Back"));
    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->addStretch();
    btn_layout->addWidget(back_btn);
    layout->addLayout(btn_layout);
    QObject::connect(back_btn, &QPushButton::clicked,
                     &dlg, &QDialog::accept);
    dlg.exec();
  }


  /**
     Opens a dialog letting the user edit count, datetime, and (for
     lendings/returns) remarks of the selected history row.  The
     currently logged-in user becomes the new data collector.  Checks
     for conflicts before applying; refreshes the history page on
     success.
     @param data central application state.
  */
  void show_change_dialog(colman::AppData& data)
  {
    colman::HistoryCombinedModel::Row old_row;
    if(!selected_history_row(data, old_row)) {
      return;
    }

    bool const has_remarks =
        (old_row.action == "lending" || old_row.action == "return");
    bool const has_person = (old_row.action == "lending");
    bool const has_duedate =
        (old_row.action == "lending" || old_row.action == "reservation");

    int const dlg_height = has_person ? 420 : (has_remarks ? 320 : 260);
    QDialog dlg(data.mw_.history_page_);
    dlg.setWindowTitle(QApplication::tr("Change Action"));
    dlg.resize(450, dlg_height);
    QVBoxLayout* main_layout = new QVBoxLayout(&dlg);

    QFormLayout* form = new QFormLayout();

    QSpinBox* count_spin = new QSpinBox();
    count_spin->setRange(1, 999999);
    count_spin->setValue(old_row.count);
    form->addRow(QApplication::tr("number:"), count_spin);

    QDateTimeEdit* dt_edit = new QDateTimeEdit(old_row.datetime.toLocalTime());
    dt_edit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    dt_edit->setCalendarPopup(true);
    form->addRow(QApplication::tr("date/time:"), dt_edit);

    QLineEdit* remarks_edit = nullptr;
    if(has_remarks) {
      remarks_edit = new QLineEdit(old_row.remarks);
      form->addRow(QApplication::tr("remarks:"), remarks_edit);
    }

    QComboBox* person_combo = nullptr;
    if(has_person) {
      auto const persons = data.queries_->query_persons(-1, "");
      person_combo = new QComboBox();
      int current_idx = 0;
      for(int i = 0; i < static_cast<int>(persons.size()); ++i) {
        auto const& p = persons[i];
        QString const display =
            p.get_lastname() + ", " + p.get_firstname();
        person_combo->addItem(display, p.get_id());
        if(p.get_id() == old_row.person_id) {
          current_idx = i;
        }
      }
      person_combo->setCurrentIndex(current_idx);
      form->addRow(QApplication::tr("borrower:"), person_combo);
    }

    QCheckBox* duedate_check = nullptr;
    QDateEdit* duedate_edit = nullptr;
    if(has_duedate) {
      duedate_check = new QCheckBox(QApplication::tr("has due date"));
      duedate_check->setChecked(old_row.duedate.isValid());
      duedate_edit = new QDateEdit();
      duedate_edit->setCalendarPopup(true);
      duedate_edit->setDisplayFormat("yyyy-MM-dd");
      duedate_edit->setDate(
          old_row.duedate.isValid() ? old_row.duedate : QDate::currentDate());
      duedate_edit->setEnabled(old_row.duedate.isValid());
      QWidget* duedate_container = new QWidget();
      QHBoxLayout* duedate_layout = new QHBoxLayout(duedate_container);
      duedate_layout->setContentsMargins(0, 0, 0, 0);
      duedate_layout->addWidget(duedate_check);
      duedate_layout->addWidget(duedate_edit);
      duedate_layout->addStretch();
      form->addRow(QApplication::tr("due date:"), duedate_container);
    }
    main_layout->addLayout(form);

    QHBoxLayout* btn_layout = new QHBoxLayout();
    QPushButton* cancel_btn =
        new QPushButton(QApplication::tr("Cancel"));
    QPushButton* apply_btn =
        new QPushButton(QApplication::tr("Apply Change"));
    apply_btn->setEnabled(false);
    btn_layout->addStretch();
    btn_layout->addWidget(cancel_btn);
    btn_layout->addWidget(apply_btn);
    main_layout->addLayout(btn_layout);

    auto const update_apply = [&]() {
      QDate const new_duedate =
          (duedate_check && duedate_check->isChecked())
              ? duedate_edit->date()
              : QDate();
      bool const changed =
          count_spin->value() != old_row.count || dt_edit->dateTime() != old_row.datetime || (remarks_edit && remarks_edit->text() != old_row.remarks) || (person_combo && person_combo->currentData().toInt() != old_row.person_id) || (duedate_check && new_duedate != old_row.duedate);
      apply_btn->setEnabled(changed);
    };

    QObject::connect(count_spin,
                     QOverload<int>::of(&QSpinBox::valueChanged),
                     [&](int) { update_apply(); });
    QObject::connect(dt_edit, &QDateTimeEdit::dateTimeChanged,
                     [&](QDateTime const&) { update_apply(); });
    if(remarks_edit) {
      QObject::connect(remarks_edit, &QLineEdit::textChanged,
                       [&](QString const&) { update_apply(); });
    }
    if(person_combo) {
      QObject::connect(person_combo,
                       QOverload<int>::of(&QComboBox::currentIndexChanged),
                       [&](int) { update_apply(); });
    }
    if(duedate_check) {
      QObject::connect(duedate_check, &QCheckBox::toggled,
                       [&](bool on) {
                         duedate_edit->setEnabled(on);
                         update_apply();
                       });
      QObject::connect(duedate_edit, &QDateEdit::dateChanged,
                       [&](QDate const&) { update_apply(); });
    }
    QObject::connect(cancel_btn, &QPushButton::clicked,
                     &dlg, &QDialog::reject);

    bool applied = false;
    QObject::connect(apply_btn, &QPushButton::clicked,
                     [&]() {
                       colman::HistoryCombinedModel::Row new_row = old_row;
                       new_row.count = count_spin->value();
                       new_row.datetime = dt_edit->dateTime().toUTC();
                       new_row.collector_id = data.current_data_collector_;
                       if(remarks_edit) {
                         new_row.remarks = remarks_edit->text();
                       }
                       if(person_combo) {
                         new_row.person_id = person_combo->currentData().toInt();
                       }
                       if(duedate_check) {
                         new_row.duedate = duedate_check->isChecked()
                                               ? duedate_edit->date()
                                               : QDate();
                       }

                       auto const conflicts =
                           data.queries_->find_change_conflicts(old_row, new_row);
                       if(!conflicts.empty()) {
                         show_change_conflict_dialog(&dlg, old_row, new_row, conflicts);
                         return;
                       }
                       if(!data.queries_->apply_action_change(old_row, new_row)) {
                         QMessageBox::warning(
                             &dlg,
                             QApplication::tr("Error"),
                             QApplication::tr("Failed to apply the change."));
                         return;
                       }
                       applied = true;
                       dlg.accept();
                     });

    dlg.exec();
    if(applied) {
      refresh_history_page(data);
    }
  }


  /**
     Switches to the history page, sets the default date range to the
     last three months, and loads the combined history query.
     @param data central application state.
  */
  void enter_history_page(colman::AppData& data)
  {
    data.mw_.history_from_date_->setDate(QDate::currentDate().addMonths(-DefaultHistoryMonths));
    data.mw_.history_to_date_->setDate(QDate::currentDate());
    data.mw_.history_show_all_->setChecked(false);
    {
      colman::AutoBlockSignals b(data.mw_.history_action_filter_);
      data.mw_.history_action_filter_->setCurrentIndex(0);
    }
    reset_history_query(data);
    data.mw_.history_details_button_->hide();
    data.mw_.history_change_button_->hide();
    data.mw_.history_delete_button_->hide();
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.history_page_);
  }

} // namespace


/**
   Refreshes the history page (after undo or redo).
   @param data central application state.
*/
void colman::refresh_history_page(colman::AppData& data)
{
  reset_history_query(data);
}


/**
   Initializes the history page; to be called once at program start.
   Sets up the proxy model, configures the table view, and connects
   all buttons and selection signals.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_history_page(QApplication& app, AppData& data)
{
  QSortFilterProxyModel* sqlproxy =
      new QSortFilterProxyModel(data.mw_.combined_history_table_);
  sqlproxy->setSourceModel(&data.combined_history_model_);
  data.mw_.combined_history_table_->setModel(sqlproxy);
  data.mw_.combined_history_table_->horizontalHeader()
      ->setSectionResizeMode(QHeaderView::ResizeToContents);

  {
    auto* vbox =
        qobject_cast<QVBoxLayout*>(data.mw_.history_page_->layout());
    if(vbox && vbox->count() > 1) {
      QLayoutItem* heading_item = vbox->takeAt(1);
      QWidget* heading_widget = heading_item->widget();
      delete heading_item;
      QHBoxLayout* title_row = new QHBoxLayout();
      QLabel* owl_label = new QLabel();
      QPixmap const pix(":/icons/owls/list_history.jpeg");
      owl_label->setPixmap(
          pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      title_row->addWidget(owl_label);
      title_row->addWidget(heading_widget);
      title_row->addStretch();
      vbox->insertLayout(1, title_row);
    }
  }

  data.w_.connect(data.history_button_, &QToolButton::clicked,
                  [&data]() {
                    enter_history_page(data);
                  });
  app.connect(data.mw_.back_button_, &QPushButton::clicked,
              [&data]() {
                leave_history_page(data);
              });
  app.connect(data.mw_.history_apply_button_, &QPushButton::clicked,
              [&data]() {
                reset_history_query(data);
              });
  app.connect(data.mw_.history_show_all_, &QCheckBox::toggled,
              [&data](bool) {
                reset_history_query(data);
              });
  app.connect(data.mw_.history_action_filter_,
              &QComboBox::currentIndexChanged,
              [&data](int) {
                reset_history_query(data);
              });
  app.connect(data.mw_.history_details_button_, &QPushButton::clicked,
              [&data]() {
                show_action_details(data);
              });
  app.connect(data.mw_.combined_history_table_, &QTableView::doubleClicked,
              [&data](QModelIndex const&) {
                show_action_details(data);
              });
  app.connect(data.mw_.history_delete_button_, &QPushButton::clicked,
              [&data]() {
                delete_selected_actions(data);
              });
  app.connect(data.mw_.history_change_button_, &QPushButton::clicked,
              [&data]() {
                show_change_dialog(data);
              });
  app.connect(data.mw_.history_load_more_, &QPushButton::clicked,
              [&data]() {
                history_load_more(data);
              });
  app.connect(data.mw_.combined_history_table_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const&, QItemSelection const&) {
                // Count the whole selection, not only the rows that just
                // changed, so that deselecting one of several rows keeps
                // the buttons.
                auto const n = data.mw_.combined_history_table_
                                   ->selectionModel()
                                   ->selectedRows()
                                   .size();
                int const uid = data.current_data_collector_;
                bool const can_change =
                    (uid == 0) || data.queries_->has_permission(uid, "change_actions");
                // Details and change act on a single action only.
                data.mw_.history_details_button_->setVisible(n == 1);
                data.mw_.history_change_button_->setVisible(
                    n == 1 && can_change);
                data.mw_.history_delete_button_->setVisible(
                    n > 0 && can_change);
              });
}

/* EOF */
