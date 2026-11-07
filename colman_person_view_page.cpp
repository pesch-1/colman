// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// person view page setup and interaction logic
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person_view_page.hpp"
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
#endif
#ifndef SYSTEM_QTEXTDOCUMENT
#  include <QTextDocument>
#  define SYSTEM_QTEXTDOCUMENT
#endif
#ifndef SYSTEM_QTEXTCURSOR
#  include <QTextCursor>
#  define SYSTEM_QTEXTCURSOR
#endif
#ifndef SYSTEM_QTEXTTABLE
#  include <QTextTable>
#  define SYSTEM_QTEXTTABLE
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
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
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif

namespace
{
  /**
     Opens a print preview dialog and prints @p doc to the selected printer.
     @param doc document to print.
  */
  void print_document(QTextDocument& doc)
  {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
    QPrintPreviewDialog dialog(&printer);
    dialog.connect(&dialog, &QPrintPreviewDialog::paintRequested,
                   [&doc](QPrinter* printer) {
                     doc.print(printer);
                   });
    dialog.exec();
  }


  /**
     Applies a non-breakable block format to @p cursor's current block.
     Optionally makes the text bold.
     @param cursor cursor pointing to the block to format.
     @param bold   if true, applies bold character formatting.
  */
  void set_block_format(QTextCursor& cursor, bool bold = false)
  {
    QTextBlockFormat format;
    format.setNonBreakableLines(true);
    cursor.setBlockFormat(format);
    if(bold) {
      QTextCharFormat tformat;
      tformat.setFontWeight(QFont::Bold);
      cursor.mergeCharFormat(tformat);
    }
  }


  /**
     Queries all currently active lendings for person @p pid and appends them
     as a nested table row inside @p table.  Each outstanding lending is shown
     with item name, subject, academic year, book type, and outstanding count.
     @param data central application state.
     @param table text table to append item lending rows into.
     @param pid   database id of the person whose lendings to display.
  */
  void insert_item_text(colman::AppData& data, QTextTable& table, int pid)
  {
    auto const& lendings = data.queries_->query_lendings(pid);
    if(!lendings.empty()) {
      int const j = table.rows();
      table.insertRows(j, 1);
      table.mergeCells(j, 1, 1, 5);
      QTextCursor cell_cursor = table.cellAt(j, 1).firstCursorPosition();
      QTextTableFormat format;
      format.setCellSpacing(4);
      format.setBorder(0);
      QTextTable* ctable = cell_cursor.insertTable(1, 5, format);
      int k = 0;
      for(auto const& lending : lendings) {
        int const lid = lending.lending_id_;
        size_t lent = 0, returned = 0;
        // TODO: N+1 query — lending_balance called per lending row;
        //       fix by adding a combined query to IQueries.
        (void)data.queries_->lending_balance(lid, lent, returned);
        if(lent != returned) {
          if(k > 0) {
            ctable->insertRows(k, 1);
          }
          auto const& name = lending.item_name_;
          //auto const count=lending.lending_count_;
          cell_cursor = ctable->cellAt(k, 0).firstCursorPosition();
          set_block_format(cell_cursor);
          cell_cursor.insertText(name);
          cell_cursor.insertText(",");
          cell_cursor = ctable->cellAt(k, 4).firstCursorPosition();
          set_block_format(cell_cursor);
          cell_cursor.insertText(QApplication::tr("lent %n item(s)", "",
                                                  lent - returned));
          ++k;
        }
      }
    }
  }


  /**
     Fills @p pids with the set of selected person IDs if the
     "only selected" checkbox is checked; otherwise clears @p pids.
     @param data central application state.
     @param pids set to populate with the selected person IDs.
  */
  void selected_persons(colman::AppData& data, std::set<int>& pids)
  {
    pids.clear();
    if(data.mw_.person_view_only_selected_->isChecked()) {
      pids = data.mw_.person_view_person_list_->get_selected_person_ids();
    }
  }


  /**
     Generates an HTML/text report of all persons into @p doc.
     If "only selected" is checked only those persons are included.
     If "with items" is checked each person's active lendings are listed.
     @param data central application state.
     @param doc  document to write the report into (cleared first).
  */
  void create_report(colman::AppData& data, QTextDocument& doc)
  {
    doc.clear();
    QTextCursor cursor(&doc);
    cursor.beginEditBlock();
    auto const& persons = data.queries_->query_persons(0, "");
    std::set<int> pids;
    selected_persons(data, pids);
    int i = 1;
    QTextTableFormat format;
    format.setCellSpacing(4);
    format.setBorder(0);
    format.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    QVector<QTextLength> widths(4);
    widths[0] = QTextLength(QTextLength::FixedLength, 1);
    widths[1] = QTextLength(QTextLength::FixedLength, 1);
    widths[2] = QTextLength(QTextLength::FixedLength, 0);
    widths[3] = QTextLength(QTextLength::PercentageLength, 100);
    format.setColumnWidthConstraints(widths);
    // 0   1     2             3
    // #.) name, dummy intern, dummy extern
    QTextTable* table = cursor.insertTable(1, 4);
    table->setFormat(format);
    for(auto const& person : persons) {
      int const pid = person.get_id();
      if(data.mw_.person_view_only_selected_->isChecked() && !pids.count(pid)) {
        continue;
      }
      int j = table->rows() - 1;
      if(i > 1) {
        ++j;
        table->insertRows(j, 1);
      }
      QTextCursor cell_cursor = table->cellAt(j, 0).firstCursorPosition();
      set_block_format(cell_cursor, true);
      cell_cursor.insertText(QString().setNum(i) + ".)");
      QString const last_name = person.get_lastname();
      QString const first_name = person.get_firstname();
      QString title = person.get_title();
      if(!title.isEmpty()) {
        title += " ";
      }
      cell_cursor = table->cellAt(j, 1).firstCursorPosition();
      set_block_format(cell_cursor, true);
      cell_cursor.insertText(title + last_name + ", " + first_name);
      if(data.mw_.person_view_with_items_->isChecked()) {
        insert_item_text(data, *table, pid);
      }
      ++i;
    }
    cursor.endEditBlock();
  }


  /**
     Re-executes the person query to reflect the current database state
     while keeping any active search filter in place.
     @param data central application state.
  */
  void refresh_person_query(colman::AppData& data)
  {
    data.mw_.person_view_person_list_->refresh_person_query();
  }


  /**
     Leaves the person view page: clears mode functions, navigates back to the
     central page, and releases the person query model.
     @param data central application state.
  */
  void leave_person_view_page(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.pbc_person_model_.set_persons({});
  }


  /**
     Switches to the person view page: clears mode functions, clears the
     preview document, resets the person list, and shows the page.
     @param data central application state.
  */
  void enter_person_view_page(colman::AppData& data)
  {
    QTextDocument* doc = data.mw_.person_view_preview_->document();
    if((doc)) {
      doc->clear();
    }
    data.mw_.person_view_person_list_->clear_search_field();
    refresh_person_query(data);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.person_view_page_);
  }

} // namespace


/**
   Refreshes the person view page.
   @param data central application state.
*/
void colman::refresh_person_view_page(colman::AppData& data)
{
  enter_person_view_page(data);
}


/**
   Initializes the person view page; to be called once at program start.
   Connects the back, enter, generate, and print buttons to their handlers.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_person_view_page(QApplication& app, colman::AppData& data)
{
  data.mw_.person_view_person_list_->setup(data.pbc_person_model_);
  data.mw_.person_view_person_list_->set_query_function([&data](QString const& s) {
    data.pbc_person_model_.set_persons(data.queries_->query_persons(-1, s));
  });
  data.mw_.person_view_person_list_->allow_multiple_selection(true);
  app.connect(data.mw_.person_view_back_, &QPushButton::clicked,
              [&data]() {
                leave_person_view_page(data);
              });
  if(data.person_view_button_) {
    app.connect(data.person_view_button_, &QToolButton::clicked,
                [&data]() {
                  enter_person_view_page(data);
                });
  }
  app.connect(data.mw_.person_view_generate_, &QToolButton::clicked,
              [&data]() {
                QTextDocument* doc = data.mw_.person_view_preview_->document();
                if((doc)) {
                  create_report(data, *doc);
                }
              });
  app.connect(data.mw_.person_view_print_, &QToolButton::clicked,
              [&data]() {
                QTextDocument* doc = data.mw_.person_view_preview_->document();
                if((doc)) {
                  print_document(*doc);
                }
              });
}

/* EOF */
