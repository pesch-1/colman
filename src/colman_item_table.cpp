// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// functions for item table support
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item_table.hpp"
#ifndef COLMAN_LIST_VALUE_WIDGET_HPP
#  include "colman_list_value_widget.hpp"
#  define COLMAN_LIST_VALUE_WIDGET_HPP
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QABSTRACTITEMVIEW
#  include <QAbstractItemView>
#  define SYSTEM_QABSTRACTITEMVIEW
#endif
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif
#ifndef SYSTEM_QCOMPLETER
#  include <QCompleter>
#  define SYSTEM_QCOMPLETER
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef COLMAN_PARTIAL_DATE_EDIT_HPP
#  include "colman_partial_date_edit.hpp"
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif

namespace
{
  /// Provides tr() in the former "table" context of colman_table_dialog.ui.
  /**
     The nested-table edit dialog used to be generated from
     colman_table_dialog.ui (UI class "table").  Hand-writing the dialog keeps
     the existing German translations valid by reusing the same tr() context.
  */
  struct TableDialogTr
  {
    Q_DECLARE_TR_FUNCTIONS(table)
  };

  /**
     Attaches an auto-complete model to @p edit based on distinct values
     of @p key in the type-specific table for @p type_id.
     @param data    central application state (provides db connection).
     @param type_id database id of the item type.
     @param key     column name used to populate the completion list.
     @param edit    line edit widget to attach the completer to.
  */
  void set_completer(colman::AppData& data, int type_id,
                     QString const& key, QLineEdit& edit)
  {
    delete edit.completer();
    QCompleter* completer = new QCompleter(
        data.queries_->query_type_field_completions(type_id, key), &edit);
    edit.setCompleter(completer);
  }

  /**
     If @p isbn10 is a valid ISBN-10, converts it to ISBN-13 and writes the
     result into the ISBN13 field found in the grid.
     @param grid  grid layout containing the ISBN13 label/edit pair.
     @param isbn10 ISBN-10 string entered by the user.
  */
  void fill_isbn13_from_isbn10(QGridLayout* grid, QString const& isbn10)
  {
    if(colman::is_isbn10(isbn10)) {
      int const rows = grid->rowCount();
      for(int r = 0; r < rows; ++r) {
        auto* item = grid->itemAtPosition(r, 0);
        QWidget* widget = (item ? item->widget() : nullptr);
        QLabel* label = qobject_cast<QLabel*>(widget);
        if((label) && label->property("db_column") == "ISBN13") {
          QString const isbn13 = colman::isbn10_to_isbn13(isbn10);
          item = grid->itemAtPosition(r, 1);
          widget = (item ? item->widget() : nullptr);
          QLineEdit* line = qobject_cast<QLineEdit*>(widget);
          if((line)) {
            line->blockSignals(true);
            line->setText(isbn13);
            line->setProperty("last_text", isbn13);
            line->blockSignals(false);
          }
          break;
        }
      }
    }
  }

  /**
     If @p isbn13 is a valid ISBN-13, converts it to ISBN-10 and writes the
     result into the ISBN10 field found in the grid.
     @param grid  grid layout containing the ISBN10 label/edit pair.
     @param isbn13 ISBN-13 string entered by the user.
  */
  void fill_isbn10_from_isbn13(QGridLayout* grid, QString const& isbn13)
  {
    if(colman::is_isbn13(isbn13)) {
      int const rows = grid->rowCount();
      for(int r = 0; r < rows; ++r) {
        auto* item = grid->itemAtPosition(r, 0);
        QWidget* widget = (item ? item->widget() : nullptr);
        QLabel* label = qobject_cast<QLabel*>(widget);
        if((label) && label->property("db_column") == "ISBN10") {
          QString const isbn10 = colman::isbn13_to_isbn10(isbn13);
          item = grid->itemAtPosition(r, 1);
          widget = (item ? item->widget() : nullptr);
          QLineEdit* line = qobject_cast<QLineEdit*>(widget);
          if((line)) {
            line->blockSignals(true);
            line->setText(isbn10);
            line->setProperty("last_text", isbn10);
            line->blockSignals(false);
          }
          break;
        }
      }
    }
  }

  /**
     Attaches an appropriate input validator to @p line based on the data
     type and column key.  Handles Integer, Date, Time, Float, Duration,
     IntegerRanges, and the special keys ISBN10/ISBN13.
     @param line line edit to validate.
     @param type data type information for this field.
     @param key  column name (used to detect ISBN fields).
  */
  void set_validator(QLineEdit& line, colman::DataTypeInfo type,
                     QString const& key)
  {
    // todo: minimum, maximum for all types!!!???
    switch(type.type_) {
    case colman::DataType::Integer: {
      QIntValidator* validator = new QIntValidator(&line);
      if(type.info_["use_minimum"].toBool()) {
        validator->setBottom(type.info_["minimum"].toInt());
      }
      if(type.info_["use_maximum"].toBool()) {
        validator->setTop(type.info_["maximum"].toInt());
        ;
      }
      line.setValidator(validator);
    } break;
    case colman::DataType::Time: {
      QRegularExpression rx(QApplication::tr(
          "((1[012]|[1-9]):[0-5][0-9](:[0-5][0-9])?(\\s)?(?i)(am|pm))?"));
      // rx("(([ 0-1][0-9]|2[0-3])(:[0-5][0-9])?(:[0-5][0-9])?)?");
      QRegularExpressionValidator* validator =
          new QRegularExpressionValidator(rx, &line);
      line.setValidator(validator);
    } break;
    case colman::DataType::Float: {
      QDoubleValidator* validator = new QDoubleValidator(&line);
      if(type.info_["use_minimum"].toBool()) {
        validator->setBottom(type.info_["minimum"].toDouble());
      }
      if(type.info_["use_maximum"].toBool()) {
        validator->setTop(type.info_["maximum"].toDouble());
      }
      line.setValidator(validator);
    } break;
    case colman::DataType::Duration: {
      // hours, optional :minutes, optional :seconds
      QRegularExpression rx("(\\d+(:[0-5][0-9])?(:[0-5][0-9])?)?");
      QRegularExpressionValidator* validator =
          new QRegularExpressionValidator(rx, &line);
      line.setValidator(validator);
    } break;
    case colman::DataType::IntegerRanges: {
      QRegularExpression rx("((\\d+|\\d+-\\d+)(\\,\\s*(\\d+|\\d+-\\d+))*)?");
      QRegularExpressionValidator* validator =
          new QRegularExpressionValidator(rx, &line);
      line.setValidator(validator);
    } break;
    case colman::DataType::ShortString:
    default:
      break;
    }
    if(key == "ISBN10") {
      line.setValidator(
          new colman::IsbnValidator(colman::IsbnType::Isbn10, &line));
    }
    else if(key == "ISBN13") {
      line.setValidator(
          new colman::IsbnValidator(colman::IsbnType::Isbn13, &line));
    }
  }

  /**
     Looks up the localized display name for @p key for @p type_id and sets
     it as the text of the label widget in column 0 of @p row.  Creates the
     label if it does not yet exist.
     @param data      central application state (provides db connection).
     @param grid      grid layout containing the row.
     @param row       row index.
     @param key       database column name to look up.
     @param type_id   item-type database id.
  */
  void fill_row_label(colman::AppData& data, QGridLayout* grid, int row,
                      QString const& key, int type_id)
  {
    auto* item = grid->itemAtPosition(row, 0);
    QWidget* widget = (item ? item->widget() : nullptr);
    QLabel* label = qobject_cast<QLabel*>(widget);
    QString trans = data.queries_->get_type_field_label(
        type_id, key, colman::system_language());
    if(trans.isEmpty()) {
      trans = data.queries_->get_type_field_label(type_id, key, QLocale::AnyLanguage);
    }
    if(!label) {
      label = new QLabel;
      grid->addWidget(label, row, 0);
    }
    label->setText(trans + ":");
    label->setProperty("db_column", key);
  }

  /**
     Fills column 1 of @p row with a QPlainTextEdit containing @p value.
     Creates the widget if it does not exist yet.
     @param grid   grid layout.
     @param row    row index.
     @param value  text to display.
     @param widget existing widget at (row, 1), or null.
     @param ro     if true, makes the widget read-only.
  */
  void fill_string_row(QGridLayout* grid, int row, QString const& value,
                       QWidget* widget, bool ro)
  {
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(widget);
    if(!edit) {
      edit = new QPlainTextEdit;
      grid->addWidget(edit, row, 1);
    }
    edit->setPlainText(value);
    edit->setReadOnly(ro);
  }

  /**
     Sets column headers of @p table from the "table_info" key in @p type.
     @param type  data type info containing the table column definitions.
     @param table table widget whose headers are to be configured.
  */
  void setup_table_header(colman::DataTypeInfo const& type, QTableWidget& table)
  {
    auto const it = type.info_.find("table_info");
    if(it != type.info_.end()) {
      QVariant const& variant = it.value();
      colman::KeyVector const keys = colman::variant_to_key_vector(variant);
      int col = 0;
      table.setColumnCount((int)keys.size());
      for(auto const& key : keys) {
        table.setHorizontalHeaderItem(col++, new QTableWidgetItem(key.first));
      }
      table.horizontalHeader()->setSectionResizeMode(
          QHeaderView::ResizeToContents);
    }
  }

  /**
     Serializes the contents of @p table into a nested QVariantList.
     Each row becomes a QVariantList of cell values; texts are read from
     QLineEdit, QPlainTextEdit, or QPushButton (via "table_data" property).
     @param table source table widget.
     @return QVariant containing a QVariantList of QVariantList rows.
  */
  QVariant serialize_table(QTableWidget& table)
  {
    QVariantList data;
    int const rows = table.rowCount();
    int const cols = table.columnCount();
    for(int r = 0; r < rows; ++r) {
      QVariantList row;
      for(int c = 0; c < cols; ++c) {
        QWidget* widget = table.cellWidget(r, c);
        QLineEdit* edit = qobject_cast<QLineEdit*>(widget);
        if((edit)) {
          row.push_back(QVariant(edit->text()));
        }
        else {
          QPlainTextEdit* pedit = qobject_cast<QPlainTextEdit*>(widget);
          if((pedit)) {
            row.push_back(QVariant(pedit->toPlainText()));
          }
          else {
            QPushButton* button = qobject_cast<QPushButton*>(widget);
            if((button)) {
              row.push_back(button->property("table_data"));
            }
            else {
              row.push_back(QVariant());
            }
          }
        }
      }
      data.push_back(QVariant(row));
    }
    return QVariant(data);
  }

  void fill_table_cell(QString const& key, QVariant const& value,
                       colman::DataTypeInfo const& type, QTableWidget& table,
                       int row, int col, bool ro = false);

  /**
     Populates all rows and cells of @p table from the serialized @p value.
     @param type  data type info providing the column definitions.
     @param value serialized table data (nested QVariantList).
     @param table table widget to fill.
     @param ro    if true, all cells are set to read-only.
  */
  void fill_table(colman::DataTypeInfo const& type, QVariant const& value,
                  QTableWidget& table, bool ro = false)
  {
    QVariantList const collist = value.toList();
    table.setRowCount(collist.size());
    colman::KeyVector keys;
    auto const it = type.info_.find("table_info");
    if(it != type.info_.end()) {
      QVariant const& variant = it.value();
      keys = colman::variant_to_key_vector(variant);
    }
    int r = 0;
    for(QVariant const& row : collist) {
      size_t c = 0;
      QVariantList const rowlist = row.toList();
      for(QVariant const& col : rowlist) {
        if(c >= keys.size()) {
          break;
        }
        fill_table_cell(keys[c].first, col, keys[c].second, table, r, c, ro);
        ++c;
      }
      ++r;
    }
  }

  /**
     Opens a modal dialog for editing the nested table stored in @p button's
     "table_data" property.  On acceptance, writes the updated data back to
     the property.
     @param key    column display name used as dialog title.
     @param type   data type info providing column definitions and row limits.
     @param button button widget holding the serialized table data.
  */
  void edit_table_in_dialog(QString const& key,
                            colman::DataTypeInfo const& type,
                            QPushButton& button)
  {
    QVariant const value = button.property("table_data");
    QDialog diag;
    diag.resize(400, 300);
    auto* layout = new QVBoxLayout(&diag);
    auto* table = new QTableWidget(&diag);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    // The data cells host input widgets that swallow clicks, and a table
    // without columns has no selectable cells at all.  Track the row picked
    // via the vertical header so "remove row" works in both cases.
    QApplication::connect(
        table->verticalHeader(), &QHeaderView::sectionClicked,
        [table](int row) { table->setProperty("selected_row", row); });
    layout->addWidget(table);
    auto* button_row = new QHBoxLayout;
    auto* add_row = new QPushButton(TableDialogTr::tr("add row"), &diag);
    auto* remove_row = new QPushButton(TableDialogTr::tr("remove row"), &diag);
    auto* remove_all = new QPushButton(TableDialogTr::tr("remove all"), &diag);
    button_row->addWidget(add_row);
    button_row->addWidget(remove_row);
    button_row->addWidget(remove_all);
    button_row->addStretch();
    layout->addLayout(button_row);
    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &diag);
    layout->addWidget(button_box);
    QApplication::connect(button_box, &QDialogButtonBox::accepted, &diag,
                          &QDialog::accept);
    QApplication::connect(button_box, &QDialogButtonBox::rejected, &diag,
                          &QDialog::reject);
    setup_table_header(type, *table);
    fill_table(type, value, *table);
    QApplication::connect(
        add_row, &QPushButton::clicked, [table, type]() {
          int const rows = table->rowCount();
          auto it = type.info_.find("use_maximum");
          if(it != type.info_.end() && it.value().toBool()) {
            it = type.info_.find("maximum");
            if(it != type.info_.end()) {
              int const max_rows = it.value().toInt();
              if(rows >= max_rows) {
                // too many rows to insert another one
                return;
              }
            }
          }
          int const cols = table->columnCount();
          table->setRowCount(rows + 1);
          colman::KeyVector keys;
          it = type.info_.find("table_info");
          if(it != type.info_.end()) {
            QVariant const& variant = it.value();
            keys = colman::variant_to_key_vector(variant);
          }
          for(int c = 0; c < cols; ++c) {
            fill_table_cell(keys[c].first, QVariant(), keys[c].second,
                            *table, rows, c);
          }
        });
    QApplication::connect(remove_row, &QPushButton::clicked,
                          [table, type]() {
                            int const rows = table->rowCount();
                            auto it = type.info_.find("use_minimum");
                            if(it != type.info_.end() && it.value().toBool()) {
                              it = type.info_.find("minimum");
                              if(it != type.info_.end()) {
                                int const min_rows = it.value().toInt();
                                if(rows <= min_rows) {
                                  // not enough rows to delete one
                                  return;
                                }
                              }
                            }
                            int row_to_remove = -1;
                            auto* model = table->selectionModel();
                            if((model) && model->hasSelection()) {
                              auto const sel = model->selectedRows();
                              if(!sel.empty()) {
                                row_to_remove = sel.front().row();
                              }
                            }
                            if(row_to_remove < 0) {
                              QVariant const sr =
                                  table->property("selected_row");
                              if(sr.isValid()) {
                                row_to_remove = sr.toInt();
                              }
                            }
                            if(row_to_remove >= 0 && row_to_remove < rows) {
                              table->removeRow(row_to_remove);
                              table->setProperty("selected_row", -1);
                            }
                          });
    QApplication::connect(
        remove_all, &QPushButton::clicked, [table, type]() {
          int const rows = table->rowCount();
          int min_rows = 0;
          auto it = type.info_.find("use_minimum");
          if(it != type.info_.end() && it.value().toBool()) {
            it = type.info_.find("minimum");
            if(it != type.info_.end()) {
              min_rows = it.value().toInt();
            }
          }
          table->setRowCount(std::min(rows, min_rows));
        });
    diag.setWindowTitle(QApplication::tr("Edit") + " " + key);
    if(diag.exec() == QDialog::Accepted) {
      QVariant const var = serialize_table(*table);
      button.setProperty("table_data", var);
    }
  }

  /**
     Opens a read-only dialog showing the nested table stored in @p button's
     "table_data" property.
     @param key    column display name used as dialog title.
     @param type   data type info providing column definitions.
     @param button button widget holding the serialized table data.
  */
  void display_table_in_dialog(QString const& key,
                               colman::DataTypeInfo const& type,
                               QPushButton& button)
  {
    QVariant const value = button.property("table_data");
    QDialog diag;
    auto* layout = new QVBoxLayout(&diag);
    auto* table = new QTableWidget(&diag);
    table->setAlternatingRowColors(true);
    layout->addWidget(table);
    auto* button_box =
        new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &diag);
    button_box->setCenterButtons(true);
    layout->addWidget(button_box);
    QApplication::connect(button_box, &QDialogButtonBox::accepted, &diag,
                          &QDialog::accept);
    QApplication::connect(button_box, &QDialogButtonBox::rejected, &diag,
                          &QDialog::reject);
    setup_table_header(type, *table);
    fill_table(type, value, *table, true);
    diag.setWindowTitle(QApplication::tr("Display") + " " + key);
    diag.exec();
  }

  /**
     Fills a single cell of @p table at (@p row, @p col) with an appropriate
     widget for @p type.  Uses QPlainTextEdit for String, QPushButton for Table,
     and QLineEdit for all other types.
     @param key   column name (for validator and nested-table dialog title).
     @param value cell value.
     @param type  data type info for this column.
     @param table table widget.
     @param row   row index.
     @param col   column index.
     @param ro    if true, makes the widget read-only.
  */
  void fill_table_cell(QString const& key, QVariant const& value,
                       colman::DataTypeInfo const& type, QTableWidget& table,
                       int row, int col, bool ro)
  {
    QWidget* widget = table.cellWidget(row, col);
    switch(type.type_) {
    case colman::DataType::String: {
      QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(widget);
      if(!edit) {
        edit = new QPlainTextEdit(&table);
        table.setCellWidget(row, col, edit);
      }
      edit->setPlainText(value.toString());
      edit->setReadOnly(ro);
    } break;
    case colman::DataType::Table: {
      QPushButton* edit = qobject_cast<QPushButton*>(widget);
      if(!edit) {
        edit = new QPushButton(QApplication::tr("view"));
        table.setCellWidget(row, col, edit);
      }
      else {
        edit->disconnect();
      }
      edit->setProperty("table_data", value);
      if(ro) {
        QApplication::connect(edit, &QPushButton::clicked, [key, type, edit]() {
          display_table_in_dialog(key, type, *edit);
        });
      }
      else {
        QApplication::connect(edit, &QPushButton::clicked, [key, type, edit]() {
          edit_table_in_dialog(key, type, *edit);
        });
      }
    } break;
    default: {
      QLineEdit* line = qobject_cast<QLineEdit*>(widget);
      if(!line) {
        line = new QLineEdit(&table);
        table.setCellWidget(row, col, line);
      }
      line->setText(value.toString());
      if(ro) {
        line->setReadOnly(true);
      }
      else {
        set_validator(*line, type, key);
        line->setReadOnly(false);
      }
    } break;
    }
  }

  /**
     Fills column 1 of @p row in @p grid with a QPushButton that opens an
     edit or view dialog for the Table-typed field.
     @param grid   grid layout.
     @param row    row index.
     @param key    column name used as dialog title.
     @param value  base64-encoded, QDataStream-serialized QVariant table data.
     @param type   data type info providing column definitions.
     @param widget existing widget at (row, 1), or null.
     @param ro     if true, the button opens a read-only dialog.
  */
  void fill_table_row(QGridLayout* grid, int row, QString const& key,
                      QString const& value, colman::DataTypeInfo const& type,
                      QWidget* widget, bool ro = false)
  {
    QPushButton* edit = qobject_cast<QPushButton*>(widget);
    if(!edit) {
      edit = new QPushButton(QApplication::tr("view"));
      grid->addWidget(edit, row, 1);
    }
    else {
      edit->disconnect();
    }
    QVariant variant(value);
    QByteArray arr = variant.toByteArray();
    arr = QByteArray::fromBase64(arr);
    QDataStream ds(&arr, QIODevice::ReadOnly);
    QVariant v;
    ds >> v;
    edit->setProperty("table_data", v);
    if(ro) {
      QApplication::connect(edit, &QPushButton::clicked, [key, type, edit]() {
        display_table_in_dialog(key, type, *edit);
      });
    }
    else {
      QApplication::connect(edit, &QPushButton::clicked, [key, type, edit]() {
        edit_table_in_dialog(key, type, *edit);
      });
    }
  }

  /**
     Fills column 1 of @p row in @p grid with a QLineEdit for a standard
     (non-String, non-Table) field.  In edit mode, attaches a validator,
     completer, and connects editingFinished to @p check.
     For ISBN10/ISBN13 keys, also cross-fills the counterpart field.
     @param data      central application state.
     @param grid      grid layout.
     @param row       row index.
     @param key       column name.
     @param value     current field value.
     @param type_id   item-type database id (for completer).
     @param type      data type info (for validator).
     @param widget    existing widget at (row, 1), or null.
     @param check     callback invoked after the value changes.
     @param ro        if true, makes the line edit read-only.
  */
  void fill_default_row(colman::AppData& data, QGridLayout* grid, int row,
                        QString const& key, QString const& value,
                        int type_id,
                        colman::DataTypeInfo const& type, QWidget* widget,
                        std::function<void(colman::AppData&)> check,
                        bool ro = false)
  {
    QLineEdit* line = qobject_cast<QLineEdit*>(widget);
    if(!line) {
      line = new QLineEdit;
      grid->addWidget(line, row, 1);
    }
    QObject::disconnect(line, &QLineEdit::editingFinished, nullptr, nullptr);
    QObject::disconnect(line, &QLineEdit::textChanged, nullptr, nullptr);
    line->setText(value);
    if(ro) {
      line->setReadOnly(true);
    }
    else {
      line->setProperty("last_text", value);
      if(key == "ISBN10") {
        QApplication::connect(line, &QLineEdit::editingFinished,
                              [&data, grid, line, check]() {
                                fill_isbn13_from_isbn10(grid, line->text());
                                check(data);
                              });
      }
      else if(key == "ISBN13") {
        QApplication::connect(line, &QLineEdit::editingFinished,
                              [&data, grid, line, check]() {
                                fill_isbn10_from_isbn13(grid, line->text());
                                check(data);
                              });
      }
      QApplication::connect(line, &QLineEdit::editingFinished,
                            [&data, line, check]() {
                              if(line->property("last_text") != line->text()) {
                                line->setProperty("last_text", line->text());
                                check(data);
                              }
                            });
      QApplication::connect(
          line, &QLineEdit::textChanged, [line](QString const&) {
            if(line->hasAcceptableInput()) {
              line->setStyleSheet("QLineEdit { background-color: white }");
            }
            else {
              line->setStyleSheet("QLineEdit { background-color: yellow }");
            }
          });
      set_validator(*line, type, key);
      set_completer(data, type_id, key, *line);
    }
  }

  /// Fills a date row with three sub-fields: Tag, Monat, Jahr.
  /**
     Creates or reuses a container widget at grid (row, 1) holding three
     digit-only QLineEdits for day, month, and year.  Focus advances
     automatically from day to month (after 2 digits) and from month to
     year (after 2 digits).  A calendar picker button is placed at column 2
     in read-write mode.
     @param data  application state.
     @param grid  grid layout containing the row.
     @param row   row index.
     @param value current date string ("d.M.yyyy" / "M.yyyy" / "yyyy").
     @param check callback invoked after any sub-field is edited.
     @param ro    if true the sub-fields are shown read-only.
  */
  void fill_date_row(colman::AppData& data, QGridLayout* grid, int row,
                     QString const& value,
                     std::function<void(colman::AppData&)> check,
                     bool ro = false)
  {
    // Get or create PartialDateEdit.
    auto* layout_item = grid->itemAtPosition(row, 1);
    colman::PartialDateEdit* de =
        layout_item
            ? qobject_cast<colman::PartialDateEdit*>(layout_item->widget())
            : nullptr;
    if(!de) {
      de = new colman::PartialDateEdit;
      grid->addWidget(de, row, 1);
    }
    QObject::disconnect(de, &colman::PartialDateEdit::dateChanged, nullptr,
                        nullptr);
    de->setDateString(value);
    de->setReadOnly(ro);
    if(ro) {
      return;
    }
    QApplication::connect(de, &colman::PartialDateEdit::dateChanged,
                          [&data, check]() { check(data); });
  }

} // namespace

/**
   Fills a row in a dynamic item attribute grid in read-write mode.
   Dispatches to fill_string_row, fill_table_row, or fill_default_row
   depending on @p type.
   @param data      central application state.
   @param grid      grid layout containing the row.
   @param row       row index.
   @param key       database column name.
   @param value     current value to display.
   @param type_id   item-type database id (for completer and labels).
   @param type      data type info for this attribute.
   @param check     callback invoked whenever the value is changed by the user.
*/
/// Returns true if @p s is a valid date in German d.M.yyyy format.
/**
  Accepts empty strings, year-only (yyyy), month+year (M.yyyy),
  and full dates (d.M.yyyy) with optional leading zeros on day and month.
  @param s string to validate.
*/
bool colman::is_valid_date_input(QString const& s)
{
  if(s.isEmpty()) {
    return true;
  }
  QRegularExpression const rx(
      "^(((0?[1-9]|[1-2][0-9]|3[01])\\.)?(0?[1-9]|1[0-2])\\.)?(\\d{4})$");
  return rx.match(s).hasMatch();
}

void colman::fill_row_rw(AppData& data, QGridLayout* grid, int row,
                         QString const& key, QString const& value,
                         int type_id,
                         colman::DataTypeInfo const& type,
                         std::function<void(AppData&)> check)
{
  if((grid)) {
    fill_row_label(data, grid, row, key, type_id);
    auto* item = grid->itemAtPosition(row, 1);
    QWidget* widget = (item ? item->widget() : nullptr);
    switch(type.type_) {
    case colman::DataType::String:
      fill_string_row(grid, row, value, widget, false);
      break;
    case colman::DataType::Table:
      fill_table_row(grid, row, key, value, type, widget, false);
      break;
    case colman::DataType::Date:
      fill_date_row(data, grid, row, value, check, false);
      break;
    default:
      fill_default_row(data, grid, row, key, value, type_id, type, widget,
                       check, false);
      break;
    }
  }
}

/**
   Fills a row in a dynamic item attribute grid in read-only mode.
   Dispatches to fill_string_row, fill_table_row, or fill_default_row
   depending on @p type; all widgets are set to read-only.
   @param data      central application state.
   @param grid      grid layout containing the row.
   @param row       row index.
   @param key       database column name.
   @param value     value to display.
   @param type_id   item-type database id (for labels).
   @param type      data type info for this attribute.
*/
void colman::fill_row_ro(AppData& data, QGridLayout* grid, int row,
                         QString const& key, QString const& value,
                         int type_id,
                         colman::DataTypeInfo const& type)
{
  if((grid)) {
    fill_row_label(data, grid, row, key, type_id);
    auto* item = grid->itemAtPosition(row, 1);
    QWidget* widget = (item ? item->widget() : nullptr);
    switch(type.type_) {
    case colman::DataType::String:
      fill_string_row(grid, row, value, widget, true);
      break;
    case colman::DataType::Table:
      fill_table_row(grid, row, key, value, type, widget, true);
      break;
    default:
      fill_default_row(
          data, grid, row, key, value, type_id, type, widget, [](AppData&) {},
          true);
      break;
    }
  }
}

/// Adds a read-write repeatable-field row to \a grid at \a row.
/**
   @param data    application state
   @param grid    target layout
   @param row     grid row index to fill
   @param key     attribute name (displayed as label)
   @param values  current list of values
   @param type_id  item-type database id
   @param type    data type info (must satisfy is_repeatable())
   @param check   callback invoked after each user edit
*/
void colman::fill_row_rw(colman::AppData& data,
                         QGridLayout* grid,
                         int row,
                         QString const& key,
                         QStringList const& values,
                         int type_id,
                         DataTypeInfo const& type,
                         std::function<void(AppData&)> check)
{
  if(!grid) {
    return;
  }
  fill_row_label(data, grid, row, key, type_id);
  auto* existing = grid->itemAtPosition(row, 1);
  auto* lv = existing
                 ? qobject_cast<colman::ListValueWidget*>(existing->widget())
                 : nullptr;
  if(!lv) {
    lv = new colman::ListValueWidget;
    grid->addWidget(lv, row, 1);
  }
  lv->set_values(values);
  lv->set_read_only(false);
  QObject::connect(lv, &colman::ListValueWidget::value_changed,
                   [&data, check]() { check(data); });
  Q_UNUSED(type);
}


/// Adds a read-only repeatable-field row to \a grid at \a row.
/**
   @param data    application state
   @param grid    target layout
   @param row     grid row index to fill
   @param key     attribute name (displayed as label)
   @param values  current list of values
   @param type_id  item-type database id
   @param type    data type info (must satisfy is_repeatable())
*/
void colman::fill_row_ro(AppData& data,
                         QGridLayout* grid,
                         int row,
                         QString const& key,
                         QStringList const& values,
                         int type_id,
                         colman::DataTypeInfo const& type)
{
  if(!grid) {
    return;
  }
  fill_row_label(data, grid, row, key, type_id);
  auto* existing = grid->itemAtPosition(row, 1);
  auto* lv = existing
                 ? qobject_cast<colman::ListValueWidget*>(existing->widget())
                 : nullptr;
  if(!lv) {
    lv = new colman::ListValueWidget;
    grid->addWidget(lv, row, 1);
  }
  lv->set_values(values);
  lv->set_read_only(true);
  Q_UNUSED(type);
}

/* EOF */
