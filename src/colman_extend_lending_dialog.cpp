// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// dialog to extend the lending period of open lendings
/**
   @file
   @author Christoph Pesch
*/

#include "colman_extend_lending_dialog.hpp"
#ifndef COLMAN_TABLE_MODELS_HPP
#  include "colman_table_models.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QDATEEDIT
#  include <QDateEdit>
#  define SYSTEM_QDATEEDIT
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QITEMSELECTIONMODEL
#  include <QItemSelectionModel>
#  define SYSTEM_QITEMSELECTIONMODEL
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QSIGNALBLOCKER
#  include <QSignalBlocker>
#  define SYSTEM_QSIGNALBLOCKER
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif

namespace
{
  /// Extension used when the collection has no default lending period.
  int constexpr FallbackExtensionDays = 14;

  /**
     Proposes a new due date for @p rows that never shortens any of them:
     the later of today and the latest current due date, plus the default
     lending period (FallbackExtensionDays when none is configured).
     Lendings without due date count as due today.
     @param col  collection settings (default lending period).
     @param rows lendings to be extended.
     @return the proposed new due date.
  */
  QDate propose_due_date(
      colman::Collection const& col,
      std::vector<colman::DueLendingListModel::Row> const& rows)
  {
    auto base = QDate::currentDate();
    for(auto const& r : rows) {
      if(r.duedate.isValid() && r.duedate > base) {
        base = r.duedate;
      }
    }
    if(col.get_use_lending_period()) {
      auto const proposal = col.add_lending_period(base);
      if(proposal > base) {
        return proposal;
      }
    }
    return base.addDays(FallbackExtensionDays);
  }


  /**
     Shows the modal "extend due date" dialog for @p rows.

     With @p selectable the table allows multiple selection (all rows
     preselected) and the "extend" button is enabled only while at least
     one lending is selected; this is used when only the borrower is known.
     Without it the table is a read-only overview of lendings the caller
     has already chosen, so only the new due date is asked for.  The new
     due date is proposed by propose_due_date() for the (selected) rows and
     follows selection changes until the user edits it; it can be set
     earlier by hand, but not into the past.
     @param parent     parent widget of the dialog.
     @param data       central application state.
     @param rows       lendings shown in the dialog (must not be empty).
     @param selectable true to let the user choose among @p rows.
     @return true if at least one lending was changed.
  */
  bool run_dialog(QWidget* parent, colman::AppData& data,
                  std::vector<colman::DueLendingListModel::Row> const& rows,
                  bool selectable)
  {
    QDialog dlg(parent);
    dlg.setWindowTitle(QApplication::tr("Extend due date"));
    dlg.resize(600, 400);
    auto* layout = new QVBoxLayout(&dlg);
    if(selectable) {
      layout->addWidget(new QLabel(
          QApplication::tr("Select the lendings whose due date shall be "
                           "changed:")));
    }
    else {
      layout->addWidget(new QLabel(QApplication::tr(
          "%n lending(s) will be extended:", "",
          static_cast<int>(rows.size()))));
    }

    auto* model = new colman::DueLendingListModel(&dlg);
    model->set_rows(rows);
    auto* view = new QTableView();
    view->setModel(model);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(selectable ? QAbstractItemView::ExtendedSelection
                                      : QAbstractItemView::NoSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAlternatingRowColors(true);
    view->verticalHeader()->setVisible(false);
    for(int const col : {0, 6, 7}) {
      view->setColumnHidden(col, true);
    }
    // all lendings of one person: the borrower column is redundant
    view->setColumnHidden(2, selectable);
    view->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    view->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(view);

    auto* form = new QFormLayout();
    auto* date_edit = new QDateEdit();
    date_edit->setCalendarPopup(true);
    date_edit->setDisplayFormat(QApplication::tr("yyyy-MM-dd"));
    auto const today = QDate::currentDate();
    auto const col = data.queries_->read_collection();
    date_edit->setMinimumDate(today);
    date_edit->setDate(propose_due_date(col, rows));
    // once the user picks a date, selection changes no longer override it
    bool date_touched = false;
    QObject::connect(date_edit, &QDateEdit::dateChanged,
                     [&date_touched]() { date_touched = true; });
    form->addRow(QApplication::tr("new due date:"), date_edit);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox();
    auto* extend_btn = buttons->addButton(QApplication::tr("Extend"),
                                          QDialogButtonBox::AcceptRole);
    buttons->addButton(QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    auto* sel = view->selectionModel();
    if(selectable) {
      QObject::connect(
          sel, &QItemSelectionModel::selectionChanged, [&]() {
            extend_btn->setEnabled(sel->hasSelection());
            if(date_touched || !sel->hasSelection()) {
              return;
            }
            std::vector<colman::DueLendingListModel::Row> chosen;
            for(auto const& idx : sel->selectedRows()) {
              chosen.push_back(model->row_at(idx.row()));
            }
            QSignalBlocker block(date_edit);
            date_edit->setDate(propose_due_date(col, chosen));
          });
      view->selectAll();
    }

    QObject::connect(buttons, &QDialogButtonBox::rejected,
                     &dlg, &QDialog::reject);
    bool extended = false;
    QObject::connect(
        buttons, &QDialogButtonBox::accepted, [&]() {
          std::vector<int> ids;
          if(selectable) {
            for(auto const& idx : sel->selectedRows()) {
              ids.push_back(model->row_at(idx.row()).lending_id);
            }
          }
          else {
            for(auto const& r : rows) {
              ids.push_back(r.lending_id);
            }
          }
          if(!data.queries_->extend_lendings(ids, date_edit->date())) {
            QMessageBox::warning(
                &dlg, QApplication::tr("Error"),
                QApplication::tr("Failed to change the due date."));
            return;
          }
          extended = true;
          dlg.accept();
        });
    dlg.exec();
    return extended;
  }

} // namespace


/**
   Extending a lending period changes the due date of an existing lending,
   so it requires the same permission as lending items.
   @param data central application state.
   @return true for the administrator and users with collect_lendings.
*/
bool colman::may_extend_lendings(AppData& data)
{
  int const uid = data.current_data_collector_;
  return uid == 0 ||
         (uid > 0 && data.queries_->has_permission(uid, "collect_lendings"));
}


/**
   Loads the open lendings of @p person_id and lets the user choose which of
   them get a new due date; all are preselected.  When the person has no
   open lendings an information box is shown instead.
   @param parent    parent widget of the dialog.
   @param data      central application state.
   @param person_id borrower whose open lendings are listed.
   @return true if at least one lending was changed.
*/
bool colman::extend_lendings_of_person(QWidget* parent, AppData& data,
                                       int person_id)
{
  auto const rows =
      data.queries_->query_due_lendings(QDate(), QDate(), person_id);
  if(rows.empty()) {
    QMessageBox::information(
        parent, QApplication::tr("Extend due date"),
        QApplication::tr("This person has no open lendings."));
    return false;
  }
  return run_dialog(parent, data, rows, true);
}


/**
   Asks only for the new due date of @p rows, which the caller has already
   chosen (e.g. selected rows of the open-lendings page or the lending of
   the return wizard).  The rows are shown as read-only overview.
   @param parent parent widget of the dialog.
   @param data   central application state.
   @param rows   lendings to change; nothing happens when empty.
   @return true if the lendings were changed.
*/
bool colman::extend_selected_lendings(
    QWidget* parent, AppData& data,
    std::vector<DueLendingListModel::Row> const& rows)
{
  if(rows.empty()) {
    return false;
  }
  return run_dialog(parent, data, rows, false);
}

/* EOF */
