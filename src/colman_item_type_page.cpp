// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// setup of item type page
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item_type_page.hpp"
#ifndef COLMAN_TYPE_HPP
#  include "colman_type.hpp"
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
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
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
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QPLAINTEXTEDIT
#  include <QPlainTextEdit>
#  define SYSTEM_QPLAINTEXTEDIT
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QDATEEDIT
#  include <QDateEdit>
#  define SYSTEM_QDATEEDIT
#endif
#ifndef SYSTEM_QTIMEEDIT
#  include <QTimeEdit>
#  define SYSTEM_QTIMEEDIT
#endif
#ifndef SYSTEM_QDOUBLESPINBOX
#  include <QDoubleSpinBox>
#  define SYSTEM_QDOUBLESPINBOX
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_LIMITS
#  include <limits>
#  define SYSTEM_LIMITS
#endif
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  include <QRegularExpressionValidator>
#  define SYSTEM_QREGULAREXPRESSION
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef SYSTEM_QMAP
#  include <QMap>
#  define SYSTEM_QMAP
#endif
#ifndef SYSTEM_QSTANDARDITEMMODEL
#  include <QStandardItemModel>
#  define SYSTEM_QSTANDARDITEMMODEL
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QMENU
#  include <QMenu>
#  define SYSTEM_QMENU
#endif
#ifndef COLMAN_MARC21_HPP
#  include "colman_marc21.hpp"
#endif


namespace
{
  void type_table_dialog(QPushButton& button);
  void update_min_max(QTableWidget& table);

  /// Hand-written replacement for the former colman_type_table.ui dialog.
  /**
     Builds the nested-table definition dialog in C++.  The member names mirror
     the widgets of the former generated Ui_TableDialog; setup_ui() constructs
     them and wires the Ok/Cancel buttons.  Q_DECLARE_TR_FUNCTIONS(TableDialog)
     is declared first so lupdate records the "TableDialog" context for the
     tr() calls in the inline setup_ui() below; this preserves the existing
     German translations.  The macro ends with a private: specifier, so an
     explicit public: restores access to the data members.
  */
  struct TypeTableDialogUi
  {
    Q_DECLARE_TR_FUNCTIONS(TableDialog)

  public:
    QLabel* table_name_ = nullptr;           ///< nested-table name caption
    QTableWidget* table_ = nullptr;          ///< field-definition grid
    QPushButton* remove_all_ = nullptr;      ///< remove all fields
    QPushButton* remove_row_ = nullptr;      ///< remove selected field
    QPushButton* add_row_ = nullptr;         ///< append a field
    QPushButton* move_up_ = nullptr;         ///< move field up
    QPushButton* move_down_ = nullptr;       ///< move field down
    QDialogButtonBox* button_box_ = nullptr; ///< Ok/Cancel box

    /// Builds the dialog widgets into @p dialog.
    void setup_ui(QDialog& dialog)
    {
      dialog.setWindowTitle(tr("Table"));
      dialog.resize(640, 400);
      auto* layout = new QVBoxLayout(&dialog);
      table_name_ = new QLabel(&dialog);
      layout->addWidget(table_name_);
      table_ = new QTableWidget(&dialog);
      table_->setAlternatingRowColors(true);
      table_->horizontalHeader()->setDefaultSectionSize(90);
      QStringList const headers{tr("name"), tr("type"),
                                tr("use minimum"), tr("minimum"),
                                tr("use maximum"), tr("maximum"),
                                tr("edit")};
      table_->setColumnCount((int)headers.size());
      table_->setHorizontalHeaderLabels(headers);
      layout->addWidget(table_);
      auto* row = new QHBoxLayout;
      remove_all_ = new QPushButton(tr("remove all"), &dialog);
      remove_row_ = new QPushButton(tr("remove field"), &dialog);
      add_row_ = new QPushButton(tr("add field"), &dialog);
      move_up_ = new QPushButton(tr("Move up"), &dialog);
      move_down_ = new QPushButton(tr("Move down"), &dialog);
      row->addWidget(remove_all_);
      row->addWidget(remove_row_);
      row->addWidget(add_row_);
      row->addWidget(move_up_);
      row->addWidget(move_down_);
      row->addStretch();
      layout->addLayout(row);
      button_box_ = new QDialogButtonBox(
          QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal,
          &dialog);
      layout->addWidget(button_box_);
      QObject::connect(button_box_, &QDialogButtonBox::accepted, &dialog,
                       &QDialog::accept);
      QObject::connect(button_box_, &QDialogButtonBox::rejected, &dialog,
                       &QDialog::reject);
    }
  };


  /// sets type of minimum or maximum widget
  /**
   * Creates or returns the appropriate editor widget for a min/max constraint
   * cell, depending on the given data type.
   * @param table  table widget containing the cell.
   * @param row    row index of the cell.
   * @param col    column index of the cell.
   * @param type   data type determining the widget kind
   *               (QDateEdit, QTimeEdit, QDoubleSpinBox, QLineEdit, or
   *               QSpinBox).
   * @param check  checkbox that enables/disables the widget.
   * @param content initial value to set when the widget is first created.
   * @return pointer to the cell widget.
   */
  QWidget* set_widget_type(QTableWidget& table,
                           int row, int col,
                           colman::DataType type,
                           QCheckBox* check,
                           QVariant const& content)
  {
    QWidget* widget = table.cellWidget(row, col);
    switch(type) {
    case colman::DataType::Date: {
      QDateEdit* edit = qobject_cast<QDateEdit*>(widget);
      if(!edit) {
        edit = new QDateEdit;
        table.setCellWidget(row, col, edit);
        edit->setDate(content.toDate());
        QApplication::connect(check, &QCheckBox::toggled,
                              edit, &QDateEdit::setEnabled);
      }
      edit->setEnabled(check->isChecked());
      return edit;
    }
    case colman::DataType::Time: {
      QTimeEdit* edit = qobject_cast<QTimeEdit*>(widget);
      if(!edit) {
        edit = new QTimeEdit;
        table.setCellWidget(row, col, edit);
        edit->setTime(content.toTime());
        QApplication::connect(check, &QCheckBox::toggled,
                              edit, &QTimeEdit::setEnabled);
      }
      edit->setEnabled(check->isChecked());
      return edit;
    }
    case colman::DataType::Float: {
      QDoubleSpinBox* edit = qobject_cast<QDoubleSpinBox*>(widget);
      if(!edit) {
        edit = new QDoubleSpinBox;
        table.setCellWidget(row, col, edit);
        edit->setValue(content.toDouble());
        edit->setMaximum(std::numeric_limits<float>::max());
        edit->setMinimum(-std::numeric_limits<float>::max());
        QApplication::connect(check, &QCheckBox::toggled,
                              edit, &QDoubleSpinBox::setEnabled);
      }
      edit->setEnabled(check->isChecked());
      return edit;
    }
    case colman::DataType::Duration: {
      QLineEdit* edit = qobject_cast<QLineEdit*>(widget);
      if(!edit) {
        edit = new QLineEdit;
        table.setCellWidget(row, col, edit);
        edit->setText(content.toString());
        QApplication::connect(check, &QCheckBox::toggled,
                              edit, &QLineEdit::setEnabled);
      }
      QRegularExpression rx("[0-9][0-9]*:[0-9][0-9]");
      QValidator* validator = new QRegularExpressionValidator(rx, edit);
      edit->setValidator(validator);
      edit->setEnabled(check->isChecked());
      return edit;
    } break;
    default: {
      QSpinBox* edit = qobject_cast<QSpinBox*>(widget);
      if(!edit) {
        edit = new QSpinBox;
        table.setCellWidget(row, col, edit);
        edit->setValue(content.toInt());
        QApplication::connect(check, &QCheckBox::toggled,
                              edit, &QSpinBox::setEnabled);
      }
      edit->setMaximum(99999999);
      edit->setMinimum(-99999999);
      edit->setEnabled(check->isChecked());
      return edit;
    }
    }
  }


  /**
   * Creates or returns a QPushButton "edit" cell widget for a Table-type
   * attribute.  The button opens a nested table dialog when clicked.
   * @param name    attribute name stored as the "table_name" property on the
   *                button.
   * @param table   table widget containing the cell.
   * @param row     row index of the cell.
   * @param col     column index of the cell.
   * @param content initial "table_info" QVariant stored as a button property.
   * @return pointer to the cell widget.
   */
  QWidget* set_table_widget_type(QString const& name,
                                 QTableWidget& table,
                                 int row, int col,
                                 QVariant const& content)
  {
    QWidget* widget = table.cellWidget(row, col);
    QPushButton* but = qobject_cast<QPushButton*>(widget);
    if(!but) {
      but = new QPushButton(QApplication::tr("edit"), &table);
      but->setProperty("table_info", content);
      but->setProperty("table_name", name);
      QApplication::connect(but, &QPushButton::clicked,
                            [but]() {
                              type_table_dialog(*but);
                            });
      table.setCellWidget(row, col, but);
    }
    return widget;
  }


  /// minimum or maximum widget to value
  /**
   * Reads the value from a min/max constraint widget.
   * @param widget widget to read.
   * @param type   data type of the constraint.
   * @return value as QVariant, or invalid QVariant if widget is null.
   */
  QVariant widget_to_value(QWidget* widget, colman::DataType type)
  {
    if((widget)) {
      switch(type) {
      case colman::DataType::String:
      case colman::DataType::ShortString:
      case colman::DataType::Integer:
      case colman::DataType::Table: {
        QSpinBox* sb = qobject_cast<QSpinBox*>(widget);
        if((sb)) {
          return QVariant(sb->value());
        }
      } break;
      case colman::DataType::Date: {
        QDateEdit* de = qobject_cast<QDateEdit*>(widget);
        if((de)) {
          return QVariant(de->date());
        }
      } break;
      case colman::DataType::Time: {
        QTimeEdit* te = qobject_cast<QTimeEdit*>(widget);
        if((te)) {
          return QVariant(te->time());
        }
      } break;
      case colman::DataType::Float: {
        QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox*>(widget);
        if((sb)) {
          return QVariant(sb->value());
        }
      } break;
      case colman::DataType::Duration: {
        QLineEdit* edit = qobject_cast<QLineEdit*>(widget);
        if((edit)) {
          return QVariant(edit->text());
        }
      }
      default:
        break;
      }
    }
    return QVariant();
  }


  // -----------------------------------------------------------------------
  // Row data helpers for the field table (common + type-specific rows)
  // -----------------------------------------------------------------------

  /// Represents one row in the type-editor fields table.
  struct RowData
  {
    bool is_common_ = false;
    QString common_id_;        ///< non-empty if is_common_
    colman::KeyInfo key_info_; ///< valid if !is_common_
  };

  /// Returns the translatable display name for a common field sentinel.
  QString common_field_display_name(QString const& id)
  {
    if(id == "__name__") {
      return QApplication::tr("Item name");
    }
    if(id == "__barcode__") {
      return QApplication::tr("Colman barcode");
    }
    if(id == "__thumbnail__") {
      return QApplication::tr("Thumbnail");
    }
    if(id == "__school_item__") {
      return QApplication::tr("School item");
    }
    if(id == "__place__") {
      return QApplication::tr("Place");
    }
    if(id == "__sig_top__") {
      return QApplication::tr("Signature top");
    }
    if(id == "__sig_bottom__") {
      return QApplication::tr("Signature bottom");
    }
    if(id == "__description__") {
      return QApplication::tr("Description");
    }
    if(id == "__keywords__") {
      return QApplication::tr("Keywords");
    }
    return id;
  }

  /// Creates an editable QComboBox pre-populated with MARC21 field names.
  /**
   * The combo is editable so the user can type a custom field name.
   * Each MARC21 item stores its id (e.g. "100$a") in Qt::UserRole
   * and the clean display name (e.g. "Author") in Qt::UserRole+1.
   * Group-header items are disabled.
   * @param current_display   display name to show when no MARC21 tag matches.
   * @param current_marc21_tag MARC21 id to pre-select; empty means custom.
   */
  QComboBox* make_marc21_name_combo(QString const& current_display = {},
                                    QString const& current_marc21_tag = {})
  {
    auto* combo = new QComboBox;
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    combo->setProperty("is_common_field", false);
    // First item: empty placeholder for custom field entry.
    combo->addItem("", QString());
    QString cur_cat;
    for(auto const& field : colman::marc21_fields()) {
      QString const tr_cat = colman::marc21_translated_category(field);
      if(field.category != cur_cat) {
        cur_cat = field.category;
        combo->addItem("── " + tr_cat + " ──", QString());
        auto* mdl = qobject_cast<QStandardItemModel*>(combo->model());
        if(mdl) {
          auto* it = mdl->item(combo->count() - 1);
          if(it) {
            it->setEnabled(false);
            it->setForeground(QColor(128, 128, 128));
          }
        }
      }
      QString const id = colman::marc21_id(field);
      QString const tr_name = colman::marc21_translated_display_name(field);
      combo->addItem(tr_name + " (" + id + ")", id);
      combo->setItemData(combo->count() - 1, tr_name, Qt::UserRole + 1);
      combo->setItemData(combo->count() - 1,
                         field.description, Qt::ToolTipRole);
    }
    if(!current_marc21_tag.isEmpty()) {
      int const idx = combo->findData(current_marc21_tag, Qt::UserRole);
      if(idx >= 0) {
        combo->setCurrentIndex(idx);
        // Override the edit text with the type-specific label when one exists.
        // The MARC tag remains stored in UserRole for import purposes.
        if(!current_display.isEmpty()) {
          combo->lineEdit()->setText(current_display);
        }
      }
      else {
        combo->setCurrentText(current_display);
      }
    }
    else if(!current_display.isEmpty()) {
      combo->setCurrentText(current_display);
    }
    return combo;
  }


  /// Returns the user-visible display name from a MARC21 field name combo.
  /**
   * When the edit text has been customised (differs from the MARC default),
   * the custom text is returned.  Otherwise the clean MARC name is returned
   * ("Author" instead of "Author (100$a)").  For non-MARC items the verbatim
   * edit text is returned.
   */
  QString get_combo_display_name(QComboBox const* combo)
  {
    if(!combo) {
      return {};
    }
    QString const tag = combo->currentData(Qt::UserRole).toString();
    if(!tag.isEmpty()) {
      int const idx = combo->currentIndex();
      QString const clean = combo->itemData(idx, Qt::UserRole + 1).toString();
      QString const edited = combo->currentText();
      if(!edited.isEmpty() && edited != clean &&
         edited != clean + " (" + tag + ")") {
        return edited;
      }
      if(!clean.isEmpty()) {
        return clean;
      }
    }
    return combo->currentText();
  }


  /// Returns the MARC21 id stored for the currently selected combo item.
  QString get_combo_marc21_tag(QComboBox const* combo)
  {
    if(!combo) {
      return {};
    }
    return combo->currentData(Qt::UserRole).toString();
  }


  // Forward declaration (defined below after add_type_specific_row).
  void add_type_specific_row(QTableWidget&, int, colman::KeyInfo const&,
                             colman::AppData*, int);


  /// Fills the field table from a MARC21 template.
  /**
   * If type-specific rows already exist the user is asked whether to replace
   * or append them.
   * @param table         the field-definition QTableWidget.
   * @param template_name the template to apply (matched by name).
   */
  void apply_marc21_template(QTableWidget& table, QString const& template_name)
  {
    colman::Marc21Template const* tmpl = nullptr;
    for(auto const& t : colman::marc21_templates()) {
      if(t.name == template_name) {
        tmpl = &t;
        break;
      }
    }
    if(!tmpl) {
      return;
    }

    bool has_type_specific = false;
    for(int r = 0; r < table.rowCount(); ++r) {
      if(qobject_cast<QComboBox*>(table.cellWidget(r, 0))) {
        has_type_specific = true;
        break;
      }
    }
    bool do_replace = false;
    if(has_type_specific) {
      QMessageBox msg(&table);
      msg.setWindowTitle(QApplication::tr("Collection Manager"));
      msg.setText(
          QApplication::tr("Replace existing fields with the template, "
                           "or append the template fields?"));
      QPushButton* replace_btn =
          msg.addButton(QApplication::tr("Replace"), QMessageBox::AcceptRole);
      QPushButton* append_btn =
          msg.addButton(QApplication::tr("Append"), QMessageBox::AcceptRole);
      msg.addButton(QMessageBox::Cancel);
      msg.exec();
      if(msg.clickedButton() == replace_btn) {
        do_replace = true;
      }
      else if(msg.clickedButton() != append_btn) {
        return;
      }
    }
    if(do_replace) {
      for(int r = table.rowCount() - 1; r >= 0; --r) {
        if(qobject_cast<QComboBox*>(table.cellWidget(r, 0))) {
          table.removeRow(r);
        }
      }
    }
    // Build list of MARC21 tags already covered by common fields.
    QVector<QString> covered;
    for(auto const* s : {"__name__", "__thumbnail__", "__school_item__",
                         "__place__", "__sig_top__", "__sig_bottom__",
                         "__description__", "__keywords__"}) {
      auto const t = colman::marc21_common_field_tag(s);
      if(!t.isEmpty() && !covered.contains(t)) {
        covered.push_back(t);
      }
    }
    for(auto const& field_id : tmpl->field_ids) {
      if(covered.contains(field_id)) {
        continue;
      }
      colman::Marc21Field const* f = colman::find_marc21_field(field_id);
      if(!f) {
        continue;
      }
      colman::KeyInfo ki;
      ki.first = colman::marc21_translated_display_name(*f);
      ki.second.type_ = f->suggested_type;
      ki.second.info_["marc21_tag"] = field_id;
      ki.second.info_["is_repeatable"] = f->is_repeatable;
      int const rows = table.rowCount();
      table.setRowCount(rows + 1);
      add_type_specific_row(table, rows, ki, nullptr, -1);
    }
  }


  /// Reads one type-specific row from \a table at \a r into a KeyInfo.
  colman::KeyInfo read_type_specific_row(QTableWidget& table, int r)
  {
    auto* name_cb = qobject_cast<QComboBox*>(table.cellWidget(r, 0));
    auto* combo = qobject_cast<QComboBox*>(table.cellWidget(r, 1));
    auto* use_min = qobject_cast<QCheckBox*>(table.cellWidget(r, 2));
    auto* minimum = table.cellWidget(r, 3);
    auto* use_max = qobject_cast<QCheckBox*>(table.cellWidget(r, 4));
    auto* maximum = table.cellWidget(r, 5);
    auto* edit_btn = qobject_cast<QPushButton*>(table.cellWidget(r, 6));
    auto* rep = qobject_cast<QCheckBox*>(table.cellWidget(r, 7));
    colman::KeyInfo ki;
    if(name_cb && combo && use_min && minimum && use_max && maximum) {
      int const dtype = combo->itemData(combo->currentIndex()).toInt();
      QVariant const db_col = name_cb->property("db_column");
      if(db_col.isValid()) {
        ki.first = db_col.toString();
      }
      else {
        ki.first = get_combo_display_name(name_cb);
      }
      ki.second.type_ = (colman::DataType)dtype;
      ki.second.info_["marc21_tag"] = get_combo_marc21_tag(name_cb);
      ki.second.info_["minimum"] =
          widget_to_value(minimum, (colman::DataType)dtype);
      ki.second.info_["use_minimum"] = QVariant(use_min->isChecked());
      ki.second.info_["maximum"] =
          widget_to_value(maximum, (colman::DataType)dtype);
      ki.second.info_["use_maximum"] = QVariant(use_max->isChecked());
      if(edit_btn) {
        ki.second.info_["table_info"] = edit_btn->property("table_info");
      }
      ki.second.info_["is_repeatable"] = rep ? rep->isChecked() : false;
    }
    return ki;
  }

  /// Reads all rows from \a table into a RowData vector.
  std::vector<RowData> read_all_rows(QTableWidget& table)
  {
    std::vector<RowData> result;
    for(int r = 0; r < table.rowCount(); ++r) {
      auto* w0 = table.cellWidget(r, 0);
      RowData rd;
      // Common field: QLineEdit with is_common_field=true.
      auto* le = qobject_cast<QLineEdit*>(w0);
      if(le && le->property("is_common_field").toBool()) {
        rd.is_common_ = true;
        rd.common_id_ = le->property("common_field_id").toString();
        result.push_back(rd);
        continue;
      }
      // Type-specific field: editable QComboBox.
      if(!qobject_cast<QComboBox*>(w0)) {
        continue;
      }
      rd.key_info_ = read_type_specific_row(table, r);
      result.push_back(rd);
    }
    return result;
  }

  /// Adds a single greyed-out, non-removable common-field row to \a table at row \a r.
  void add_common_field_row(QTableWidget& table, int r, QString const& id)
  {
    auto* name_edit = new QLineEdit;
    QString display = common_field_display_name(id);
    QString const tag = colman::marc21_common_field_tag(id);
    if(!tag.isEmpty()) {
      display += " (" + tag + ")";
    }
    name_edit->setText(display);
    name_edit->setReadOnly(true);
    name_edit->setProperty("is_common_field", true);
    name_edit->setProperty("common_field_id", id);
    QPalette pal = name_edit->palette();
    pal.setColor(QPalette::Base, QColor(210, 210, 210));
    name_edit->setPalette(pal);
    table.setCellWidget(r, 0, name_edit);
    for(int col = 1; col < table.columnCount(); ++col) {
      auto* lbl = new QLabel;
      lbl->setEnabled(false);
      table.setCellWidget(r, col, lbl);
    }
  }

  /// Adds a single editable type-specific row to \a table at row \a r.
  void add_type_specific_row(QTableWidget& table, int r,
                             colman::KeyInfo const& key,
                             colman::AppData* data = nullptr,
                             int type_id = -1)
  {
    QString display_name = key.first;
    QString marc21_tag = key.second.info_["marc21_tag"].toString();
    if(data && type_id >= 0) {
      QString trans = data->queries_->get_type_field_label(
          type_id, key.first, colman::system_language());
      if(trans.isEmpty()) {
        trans = data->queries_->get_type_field_label(
            type_id, key.first, QLocale::AnyLanguage);
      }
      if(!trans.isEmpty()) {
        display_name = trans;
      }
    }
    auto* name_cb = make_marc21_name_combo(display_name, marc21_tag);
    if(data && type_id >= 0 && display_name != key.first) {
      name_cb->setProperty("db_column", key.first);
      name_cb->setToolTip(
          QApplication::tr("Database column: %1").arg(key.first));
    }
    table.setCellWidget(r, 0, name_cb);
    auto* type_cb = new QComboBox;
    table.setCellWidget(r, 1, type_cb);
    type_cb->addItem(QApplication::tr("short text"),
                     (int)colman::DataType::ShortString);
    type_cb->addItem(QApplication::tr("long text"),
                     (int)colman::DataType::String);
    type_cb->addItem(QApplication::tr("date"), (int)colman::DataType::Date);
    type_cb->addItem(QApplication::tr("time"), (int)colman::DataType::Time);
    type_cb->addItem(QApplication::tr("duration"),
                     (int)colman::DataType::Duration);
    type_cb->addItem(QApplication::tr("integer"),
                     (int)colman::DataType::Integer);
    type_cb->addItem(QApplication::tr("integer ranges"),
                     (int)colman::DataType::IntegerRanges);
    type_cb->addItem(QApplication::tr("float"), (int)colman::DataType::Float);
    type_cb->addItem(QApplication::tr("table"), (int)colman::DataType::Table);
    auto* use_minimum = new QCheckBox;
    table.setCellWidget(r, 2, use_minimum);
    auto* use_maximum = new QCheckBox;
    table.setCellWidget(r, 4, use_maximum);
    use_minimum->setChecked(key.second.info_["use_minimum"].toBool());
    use_maximum->setChecked(key.second.info_["use_maximum"].toBool());
    int const ni = type_cb->count();
    for(int i = 0; i < ni; ++i) {
      if(type_cb->itemData(i).toInt() == (int)key.second.type_) {
        type_cb->setCurrentIndex(i);
        break;
      }
    }
    set_widget_type(table, r, 3, key.second.type_, use_minimum,
                    key.second.info_["minimum"]);
    set_widget_type(table, r, 5, key.second.type_, use_maximum,
                    key.second.info_["maximum"]);
    if(key.second.type_ == colman::DataType::Table) {
      set_table_widget_type(display_name, table, r, 6,
                            key.second.info_["table_info"]);
    }
    else {
      table.setCellWidget(r, 6, nullptr);
    }
    auto* repeatable = new QCheckBox;
    repeatable->setChecked(colman::is_repeatable(key.second));
    table.setCellWidget(r, 7, repeatable);
    QApplication::connect(type_cb, &QComboBox::currentTextChanged,
                          [&table](QString const&) {
                            update_min_max(table);
                          });
    // Auto-set data type when a MARC21 field is selected from the dropdown.
    QApplication::connect(
        name_cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [name_cb, type_cb](int) {
          QString const id = get_combo_marc21_tag(name_cb);
          colman::Marc21Field const* f = colman::find_marc21_field(id);
          if(!f) {
            return;
          }
          int const n = type_cb->count();
          for(int i = 0; i < n; ++i) {
            if(type_cb->itemData(i).toInt() == (int)f->suggested_type) {
              type_cb->setCurrentIndex(i);
              break;
            }
          }
        });
  }

  /// Rebuilds \a table from a RowData vector (clears the table first).
  void all_rows_to_widgets(std::vector<RowData> const& rows, QTableWidget& table,
                           colman::AppData* data = nullptr,
                           int type_id = -1)
  {
    table.setRowCount(0);
    table.setRowCount((int)rows.size());
    for(int r = 0; r < (int)rows.size(); ++r) {
      auto const& rd = rows[r];
      if(rd.is_common_) {
        add_common_field_row(table, r, rd.common_id_);
      }
      else {
        add_type_specific_row(table, r, rd.key_info_, data, type_id);
      }
    }
  }

  // -----------------------------------------------------------------------

  /**
   * Reads all attribute rows from the table widget and fills keys.
   * Rows marked as common fields (is_common_field property) are skipped.
   * @param table widget containing the attribute definitions.
   * @param keys  output key vector (cleared first).
   */
  void widgets_to_key_vector(QTableWidget& table, colman::KeyVector& keys)
  {
    keys.clear();
    int const rows = table.rowCount();
    for(int r = 0; r < rows; ++r) {
      auto* w0 = table.cellWidget(r, 0);
      // Skip common-field rows (QLineEdit with is_common_field=true).
      auto* le = qobject_cast<QLineEdit*>(w0);
      if(le && le->property("is_common_field").toBool()) {
        continue;
      }
      // Type-specific field: editable QComboBox.
      auto* name_cb = qobject_cast<QComboBox*>(w0);
      auto* combo = qobject_cast<QComboBox*>(table.cellWidget(r, 1));
      auto* use_min = qobject_cast<QCheckBox*>(table.cellWidget(r, 2));
      auto* minimum = table.cellWidget(r, 3);
      auto* use_max = qobject_cast<QCheckBox*>(table.cellWidget(r, 4));
      auto* maximum = table.cellWidget(r, 5);
      auto* edit_button = qobject_cast<QPushButton*>(table.cellWidget(r, 6));
      if(name_cb && combo && use_min && minimum && use_max && maximum) {
        QVariant const db_col_prop = name_cb->property("db_column");
        QString const key = db_col_prop.isValid()
                                ? db_col_prop.toString()
                                : get_combo_display_name(name_cb);
        int const dtype = combo->itemData(combo->currentIndex()).toInt();
        if(!key.isEmpty()) {
          colman::DataTypeInfo dtinfo;
          dtinfo.type_ = (colman::DataType)dtype;
          dtinfo.info_["marc21_tag"] = get_combo_marc21_tag(name_cb);
          dtinfo.info_["minimum"] =
              widget_to_value(minimum, (colman::DataType)dtype);
          dtinfo.info_["use_minimum"] = use_min->isChecked();
          dtinfo.info_["maximum"] =
              widget_to_value(maximum, (colman::DataType)dtype);
          dtinfo.info_["use_maximum"] = use_max->isChecked();
          if(edit_button) {
            dtinfo.info_["table_info"] = edit_button->property("table_info");
          }
          auto* rep = qobject_cast<QCheckBox*>(table.cellWidget(r, 7));
          dtinfo.info_["is_repeatable"] = rep ? rep->isChecked() : false;
          keys.push_back({key, dtinfo});
        }
      }
    }
  }


  /// Builds a column → display-name map from the field table widgets.
  /**
   * For existing fields the column name comes from the db_column property;
   * for newly added fields it is derived via string_to_column_name().
   * @param table the field-definition QTableWidget
   * @return map from sanitized column name to user-visible display name
   */
  QMap<QString, QString> widgets_to_display_names(QTableWidget const& table)
  {
    QMap<QString, QString> result;
    int const rows = table.rowCount();
    for(int r = 0; r < rows; ++r) {
      auto* w0 = table.cellWidget(r, 0);
      // Skip common-field rows.
      auto* le = qobject_cast<QLineEdit const*>(w0);
      if(le && le->property("is_common_field").toBool()) {
        continue;
      }
      auto* name_cb = qobject_cast<QComboBox const*>(w0);
      if(!name_cb) {
        continue;
      }
      QVariant const db_col_prop = name_cb->property("db_column");
      QString const display = get_combo_display_name(name_cb);
      QString const col = db_col_prop.isValid()
                              ? db_col_prop.toString()
                              : colman::string_to_column_name(display);
      if(!col.isEmpty()) {
        result.insert(col, display);
      }
    }
    return result;
  }


  /**
   * Serializes a KeyVector into a flat QList<QVariant> for storage.
   * @param keys vector to serialize.
   * @return QVariant wrapping the list.
   */
  QVariant key_vector_to_variant(colman::KeyVector const& keys)
  {
    QList<QVariant> list;
    for(auto const& key : keys) {
      list.push_back(key.first);
      list.push_back((int)key.second.type_);
      list.push_back(key.second.info_);
    }
    return QVariant::fromValue(list);
  }


  /**
   * Updates all min/max cell widgets in every row to match the currently
   * selected DataType.
   * @param table widget whose rows to update.
   */
  void update_min_max(QTableWidget& table)
  {
    int const rows = table.rowCount();
    for(int r = 0; r < rows; ++r) {
      auto* w0 = table.cellWidget(r, 0);
      // Skip common-field rows (QLineEdit with is_common_field=true).
      auto* le = qobject_cast<QLineEdit*>(w0);
      if(le && le->property("is_common_field").toBool()) {
        continue;
      }
      auto* name_cb = qobject_cast<QComboBox*>(w0);
      if(!name_cb) {
        continue;
      }
      auto* combo = qobject_cast<QComboBox*>(table.cellWidget(r, 1));
      if(!combo) {
        continue;
      }
      int const type = combo->itemData(combo->currentIndex()).toInt();
      auto* use_min = qobject_cast<QCheckBox*>(table.cellWidget(r, 2));
      auto* use_max = qobject_cast<QCheckBox*>(table.cellWidget(r, 4));
      set_widget_type(table, r, 3, (colman::DataType)type, use_min, QVariant());
      set_widget_type(table, r, 5, (colman::DataType)type, use_max, QVariant());
      if((colman::DataType)type == colman::DataType::Table) {
        set_table_widget_type(get_combo_display_name(name_cb),
                              table, r, 6, QVariant());
      }
      else {
        table.setCellWidget(r, 6, nullptr);
      }
    }
  }


  /**
   * Appends a new attribute row with default widgets to the table.
   * @param table widget to add a row to.
   */
  void add_field(QTableWidget& table)
  {
    colman::KeyInfo empty;
    empty.second.type_ = colman::DataType::ShortString;
    int const rows = table.rowCount();
    table.setRowCount(rows + 1);
    add_type_specific_row(table, rows, empty);
    update_min_max(table);
  }


  /**
   * Removes the currently selected row from the table.
   * @param table widget to remove a row from.
   */
  void remove_field(QTableWidget& table)
  {
    auto* model = table.selectionModel();
    if((model) && model->hasSelection()) {
      auto const rows = model->selectedRows();
      if(!rows.empty()) {
        auto const index = rows.front();
        auto* w0 = table.cellWidget(index.row(), 0);
        // Prevent removing common-field placeholder rows.
        auto* le = qobject_cast<QLineEdit*>(w0);
        if(le && le->property("is_common_field").toBool()) {
          return;
        }
        table.removeRow(index.row());
      }
    }
  }


  /// Moves the selected row one position up in the table.
  void move_field_up(QTableWidget& table)
  {
    auto* model = table.selectionModel();
    if(!model || !model->hasSelection()) {
      return;
    }
    int const row = model->selectedRows().front().row();
    if(row <= 0) {
      return;
    }
    auto rows = read_all_rows(table);
    std::swap(rows[std::size_t(row - 1)], rows[std::size_t(row)]);
    all_rows_to_widgets(rows, table);
    table.selectRow(row - 1);
  }

  /// Moves the selected row one position down in the table.
  void move_field_down(QTableWidget& table)
  {
    auto* model = table.selectionModel();
    if(!model || !model->hasSelection()) {
      return;
    }
    int const row = model->selectedRows().front().row();
    if(row >= table.rowCount() - 1) {
      return;
    }
    auto rows = read_all_rows(table);
    std::swap(rows[std::size_t(row)], rows[std::size_t(row + 1)]);
    all_rows_to_widgets(rows, table);
    table.selectRow(row + 1);
  }

  /**
   * Removes all rows from the table.
   * @param table widget to clear.
   */
  void remove_all_fields(QTableWidget& table)
  {
    while(table.rowCount() > 0) {
      table.removeRow(table.rowCount() - 1);
    }
  }


  /**
   * Populates the table widget with rows built from the given key vector.
   * @param keys  source attribute definitions.
   * @param table widget to populate (cleared first).
   */
  void key_vector_to_widgets(colman::KeyVector const& keys, QTableWidget& table)
  {
    std::vector<RowData> rows;
    rows.reserve(keys.size());
    for(auto const& key : keys) {
      RowData rd;
      rd.key_info_ = key;
      rows.push_back(rd);
    }
    all_rows_to_widgets(rows, table);
  }


  /// edits type table and stores result in "table_info" property of \a button
  /**
   * Opens a dialog to edit a nested table attribute definition.  The result is
   * stored in the "table_info" property of button.
   * @param button button whose "table_info" and "table_name" properties are
   *               read and, after acceptance, updated.
   */
  void type_table_dialog(QPushButton& button)
  {
    QDialog diag;
    TypeTableDialogUi dialog;
    dialog.setup_ui(diag);
    dialog.table_name_->setText(button.property("table_name").toString());
    dialog.table_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    colman::KeyVector
        keys = colman::variant_to_key_vector(button.property("table_info"));
    key_vector_to_widgets(keys, *dialog.table_);
    QApplication::connect(dialog.add_row_, &QPushButton::clicked,
                          [&dialog]() {
                            add_field(*dialog.table_);
                          });
    QApplication::connect(dialog.remove_row_, &QPushButton::clicked,
                          [&dialog]() {
                            remove_field(*dialog.table_);
                          });
    QApplication::connect(dialog.remove_all_, &QPushButton::clicked,
                          [&dialog]() {
                            remove_all_fields(*dialog.table_);
                          });
    QApplication::connect(dialog.move_up_, &QPushButton::clicked,
                          [&dialog]() {
                            move_field_up(*dialog.table_);
                          });
    QApplication::connect(dialog.move_down_, &QPushButton::clicked,
                          [&dialog]() {
                            move_field_down(*dialog.table_);
                          });
    if(diag.exec() == QDialog::Accepted) {
      colman::KeyVector keys;
      widgets_to_key_vector(*dialog.table_, keys);
      button.setProperty("table_info", key_vector_to_variant(keys));
    }
  }


  /**
   * Extracts the complete field ordering (common and type-specific) from the
   * table widget as a QStringList for passing to save_field_order().
   * @param table the field-definition table widget.
   * @return list of field sentinels/names in display order.
   */
  QStringList widgets_to_field_order(QTableWidget& table)
  {
    QStringList result;
    int const rows = table.rowCount();
    for(int r = 0; r < rows; ++r) {
      auto* w0 = table.cellWidget(r, 0);
      // Common field: QLineEdit with is_common_field=true.
      auto* le = qobject_cast<QLineEdit*>(w0);
      if(le && le->property("is_common_field").toBool()) {
        result << le->property("common_field_id").toString();
        continue;
      }
      // Type-specific field: editable QComboBox.
      auto* name_cb = qobject_cast<QComboBox*>(w0);
      if(!name_cb) {
        continue;
      }
      QVariant const db_col = name_cb->property("db_column");
      QString const key = db_col.isValid()
                              ? db_col.toString()
                              : get_combo_display_name(name_cb);
      if(!key.isEmpty()) {
        result << key;
      }
    }
    return result;
  }


  /**
   * Populates the table from a KeyVector and an explicit field order list.
   * Common field sentinels (starting with __) become greyed-out placeholder
   * rows; type-specific fields become editable rows.
   * Keys not present in field_order are appended at the end.
   * @param keys       type-specific attribute definitions.
   * @param field_order combined ordering from get_combined_field_order().
   * @param table      widget to populate (cleared first).
   */
  void key_vector_to_widgets(colman::KeyVector const& keys,
                             QStringList const& field_order,
                             QTableWidget& table,
                             colman::AppData* data = nullptr,
                             int type_id = -1)
  {
    QMap<QString, colman::KeyInfo> key_map;
    for(auto const& k : keys) {
      key_map[k.first] = k;
    }
    std::vector<RowData> rows;
    rows.reserve(std::size_t(field_order.size()));
    for(auto const& field : field_order) {
      if(field.startsWith("__")) {
        RowData rd;
        rd.is_common_ = true;
        rd.common_id_ = field;
        rows.push_back(rd);
      }
      else {
        auto it = key_map.find(field);
        if(it != key_map.end()) {
          RowData rd;
          rd.key_info_ = {field, it.value().second};
          rows.push_back(rd);
        }
      }
    }
    for(auto const& k : keys) {
      bool found = false;
      for(auto const& f : field_order) {
        if(f == k.first) {
          found = true;
          break;
        }
      }
      if(!found) {
        RowData rd;
        rd.key_info_ = k;
        rows.push_back(rd);
      }
    }
    all_rows_to_widgets(rows, table, data, type_id);
  }


  /**
   * Builds a Type from the name and fields widgets.
   * @param data central application state.
   * @return the constructed Type.
   */
  colman::Type widgets_to_type(colman::AppData& data)
  {
    colman::Type type;
    type.set_name(data.mw_.item_type_name_->text());
    colman::KeyVector keys;
    widgets_to_key_vector(*data.mw_.item_type_fields_, keys);
    type.set_keys(keys);
    type.set_media_kind(static_cast<colman::MediaKind>(
        data.mw_.item_type_media_kind_->currentData().toInt()));
    return type;
  }


  /**
   * Overload of widgets_to_type that also looks up the existing type id by
   * name.
   * @param data central application state.
   * @param type output Type object populated from widgets.
   * @return database id of the type with the current name, or -1 if not found.
   */
  int widgets_to_type(colman::AppData& data, colman::Type& type)
  {
    type.set_name(data.mw_.item_type_name_->text());
    int const type_id = data.queries_->get_type_id(type.get_name());
    type = widgets_to_type(data);
    return type_id;
  }


  /**
   * Populates the name line edit and fields table from type.
   * When type_id >= 0 the combined field order (common + type-specific) is
   * loaded from the database and used to interleave common-field placeholder
   * rows with editable type-specific rows.
   * @param data    central application state.
   * @param type    source type.
   * @param type_id database id of the type; -1 for a new/empty type.
   */
  void type_to_widgets(colman::AppData& data, colman::Type const& type,
                       int type_id)
  {
    QString const display_name = (type_id >= 0
                                      ? data.queries_->get_type_locale(type_id)
                                      : type.get_name());
    data.mw_.item_type_name_->setText(display_name);
    colman::MediaKind const kind =
        (type_id >= 0 ? data.queries_->media_kind_for_type_id(type_id)
                      : type.get_media_kind());
    int const kind_index =
        data.mw_.item_type_media_kind_->findData(static_cast<int>(kind));
    data.mw_.item_type_media_kind_->setCurrentIndex(
        kind_index >= 0 ? kind_index : 0);
    QStringList const field_order =
        (type_id >= 0
             ? data.queries_->get_combined_field_order(type_id)
             : data.queries_->get_default_common_field_order());
    key_vector_to_widgets(type.get_keys(), field_order,
                          *data.mw_.item_type_fields_,
                          &data, type_id);
  }


  /**
   * Enables the insert button if the name field is non-empty and no type is
   * currently selected.
   * @param data central application state.
   */
  void check_insert_type(colman::AppData& data)
  {
    if(data.mw_.item_type_name_->text().size() > 0 && !data.mw_.item_type_type_view_->selectionModel()->hasSelection())
    {
      data.mw_.item_type_insert_->setEnabled(true);
    }
    else {
      data.mw_.item_type_insert_->setEnabled(false);
    }
  }


  /**
   * Enables the change button if a type is selected and the name field is
   * non-empty.
   * @param data central application state.
   */
  void check_change_type(colman::AppData& data)
  {
    if(data.mw_.item_type_type_view_->selectionModel()->hasSelection() && data.mw_.item_type_name_->text().size() > 0)
    {
      data.mw_.item_type_change_->setEnabled(true);
    }
    else {
      data.mw_.item_type_change_->setEnabled(false);
    }
  }


  /**
   * Enables the remove button if a type is currently selected.
   * @param data central application state.
   */
  void check_remove_type(colman::AppData& data)
  {
    data.mw_.item_type_remove_->setEnabled(
        data.mw_.item_type_type_view_->selectionModel()->hasSelection());
  }


  /**
   * Calls all three check_* functions to update every button state at once.
   * @param data central application state.
   */
  void check_all(colman::AppData& data)
  {
    check_insert_type(data);
    check_change_type(data);
    check_remove_type(data);
  }


  /**
   * Loads the type with the given id from the database and populates the
   * widgets; clears the widgets if id is negative.
   * @param data central application state.
   * @param id   database id of the type to show; pass a negative value to
   *             clear the widgets.
   */
  void show_type(colman::AppData& data, int id)
  {
    colman::Type type;
    if(id >= 0) {
      type = data.queries_->get_type(id);
    }
    type_to_widgets(data, type, id);
  }


  /**
   * Reloads the types query model and updates the type view column header
   * labels.
   * @param data central application state.
   */
  void reset_type_query(colman::AppData& data)
  {
    data.type_model_.set_types(data.queries_->query_types(-1));
    data.mw_.item_type_type_view_->setColumnHidden(0, true);
    data.mw_.item_type_type_view_->setColumnHidden(1, true);
  }


  /**
   * Shows the type with the given id, scrolls to it in the type view, and
   * updates all button states.
   * @param data central application state.
   * @param id   type id to select; pass a negative value to deselect.
   */
  void select_type(colman::AppData& data, int id)
  {
    show_type(data, id);
    if(id >= 0) {
      colman::select_and_scroll_to(data.mw_.item_type_type_view_, 0, id);
    }
    else {
      data.mw_.item_type_type_view_->clearSelection();
    }
    check_all(data);
  }


  /**
   * Discards all widget edits by reloading the currently selected type from
   * the database.
   * @param data central application state.
   */
  void revert(colman::AppData& data)
  {
    auto* model = data.mw_.item_type_type_view_->selectionModel();
    if((model) && model->hasSelection()) {
      auto const rows = model->selectedRows();
      if(!rows.empty()) {
        auto const index = rows.front();
        int const id = index.data(Qt::DisplayRole).toInt();
        show_type(data, id);
        check_all(data);
        return;
      }
    }
    show_type(data, -1);
  }


  /**
   * Compares two key vectors and reports whether applying the change would
   * cause data loss.
   * @param old_kv key vector of the existing type.
   * @param new_kv key vector of the new (desired) type.
   * @return true, iff values may be lost due to the change from old to new.
   * @todo what, if item type changes, e.g. from table to integer?
   */
  bool key_vector_diff(colman::KeyVector const& old_kv,
                       colman::KeyVector const& new_kv)
  {
    if(old_kv.size() > new_kv.size()) {
      return true;
    }

    std::set<QString> keys;
    for(auto const& kv : new_kv) {
      keys.insert(kv.first);
    }
    for(auto const& kv : old_kv) {
      if(!keys.count(kv.first)) {
        return true;
      }
    }
    return false;
  }


  /**
   * Clears all view-model instances held by AppData.
   * @param data central application state.
   */
  void clear_models(colman::AppData& data)
  {
    data.combined_history_model_.set_rows({});
    data.ibc_item_model_.set_rows({});
    data.acquisition_item_model_.set_rows({});
    data.lending_person_model_.set_persons({});
    data.lending_item_model_.set_rows({});
    data.liquidation_item_model_.set_rows({});
    data.return_item_model_.set_rows({});
    data.return_lending_model_.set_rows({});
    data.pbc_person_model_.set_persons({});
    data.person_model_.set_persons({});
    data.type_model_.set_types({});
  }


  /**
   * Called when the type view selection changes; shows the newly selected type,
   * or clears the widgets if the selection is empty.
   * @param data     central application state.
   * @param selected new selection.
   */
  void selection_changed(colman::AppData& data,
                         QItemSelection const& selected)
  {
    auto const index_list = selected.indexes();
    if(!index_list.isEmpty()) {
      QModelIndex const& model_index = index_list.front();
      QAbstractItemModel const* model = model_index.model();
      if((model)) {
        int const row = model_index.row();
        QVariant const
            id = model->data(model->index(row, 0), Qt::DisplayRole);
        show_type(data, id.toInt());
      }
    }
    else {
      show_type(data, -1);
    }
    check_all(data);
  }


  /**
   * Installs a fresh QSortFilterProxyModel on the type view, resets the types
   * query, and reconnects the selection-changed signal.
   * @param data central application state.
   */
  void reset_proxy(colman::AppData& data)
  {
    QSortFilterProxyModel*
        sqlproxy = new QSortFilterProxyModel(data.mw_.item_type_type_view_);
    sqlproxy->setSourceModel(&data.type_model_);
    data.mw_.item_type_type_view_->setModel(sqlproxy);
    data.mw_.item_type_type_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    reset_type_query(data);
    QApplication::connect(data.mw_.item_type_type_view_->selectionModel(),
                          &QItemSelectionModel::selectionChanged,
                          [&data](QItemSelection const& selected,
                                  QItemSelection const&) {
                            selection_changed(data, selected);
                          });
  }


  /**
     Updates an existing item type with new data after user confirmation.

   Because changing a type clears the entire undo history, a warning dialog
   is shown before proceeding.
   @param data Application state containing the database and undo stack.
   @param id Database id of the type to update.
   @param type New type data to store.
   @return true if the user confirmed and the update succeeded, false otherwise.
   */
  bool change_type(colman::AppData& data, int id, colman::Type const& type,
                   QMap<QString, QString> const& display_names)
  {
    if(colman::information_yes_no(
           nullptr,
           QApplication::tr("Collection Manager"),
           QApplication::tr("Warning: changing a type will remove the "
                            "complete undo history, i.e. you can not "
                            "undo this change. Proceed anyway?")) == QMessageBox::Yes)
    {
      if(data.queries_->change_type(id, type, display_names)) {
        return true;
      }
    }
    return false;
  }


  /**
   * Reads the widgets, compares the result with the stored type, warns the
   * user if data loss would occur, writes a backup, calls change_type, and
   * reselects the type in the view.
   * @param data central application state.
   */
  void change(colman::AppData& data)
  {
    int const type_id = colman::selected_id(data.mw_.item_type_type_view_);
    if(type_id >= 0) {
      colman::Type const new_type = widgets_to_type(data);
      QMap<QString, QString> const dn =
          widgets_to_display_names(*data.mw_.item_type_fields_);
      QStringList const field_order =
          widgets_to_field_order(*data.mw_.item_type_fields_);
      colman::Type const old_type = data.queries_->get_type(type_id);
      if(key_vector_diff(old_type.get_keys(), new_type.get_keys())) {
        int const choice = colman::warning_yes_no(
            data.mw_.item_type_page_,
            QApplication::tr("Collection Manager"),
            QApplication::tr("Changing this item type will cause loss of data "
                             "that can not be undone. "
                             "Please consider to add a new type instead and "
                             "make a backup of your data, first! "
                             "Do you really want to change this type now?"));
        switch(choice) {
        case QMessageBox::Yes:
          clear_models(data);
          data.mw_.item_type_type_view_->setModel(nullptr);
          ::change_type(data, type_id, new_type, dn);
          data.queries_->save_field_order(type_id, field_order);
          reset_proxy(data);
          select_type(data, type_id);
          break;
        default:
          break;
        }
      }
      else {
        // we add information and change the type but we will not lose info
        clear_models(data);
        data.mw_.item_type_type_view_->setModel(nullptr);
        ::change_type(data, type_id, new_type, dn);
        data.queries_->save_field_order(type_id, field_order);
        reset_proxy(data);
        select_type(data, type_id);
      }
    }
    check_all(data);
  }


  /**
     Inserts a new item type and returns its database id.

     writes a backup if necessary.
     @param data Application state containing the database and undo stack.
     @param type Data of the type to insert.
     @return Database id of the newly inserted type, or -1 on failure.
   */
  int add_new_type(colman::AppData& data, colman::Type const& type)
  {
    QString const tbl = "_" + colman::string_to_column_name(type.get_name());
    return data.queries_->add_new_type(type, tbl);
  }


  /**
   * Reads the widgets, checks for a duplicate name, writes a backup, calls
   * add_new_type, refreshes the query, and selects the newly created type.
   * @param data central application state.
   */
  void insert(colman::AppData& data)
  {
    colman::Type type;
    int type_id = widgets_to_type(data, type);
    if(type_id >= 0) {
      QMessageBox::warning(data.mw_.item_type_page_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("There is already an item type "
                                            "with name '%1'. Please select "
                                            "another name!")
                               .arg(type.get_name()));
    }
    else {
      type_id = ::add_new_type(data, type);
      if(type_id < 0) {
        QMessageBox::warning(data.mw_.item_type_page_,
                             QApplication::tr("Collection Manager"),
                             QApplication::tr("Could not insert new type"));
      }
      else {
        reset_type_query(data);
        select_type(data, type_id);
      }
    }
    check_all(data);
  }


  /**
   * Writes a backup and calls the SQL remove_type function; shows a warning
   * dialog on failure.
   * @param data    central application state.
   * @param type_id id of the type to remove.
   */
  void remove_type(colman::AppData& data, int type_id)
  {
    if(!data.queries_->remove_type(type_id)) {
      QMessageBox::warning(data.mw_.item_type_page_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("Could not remove type"));
    }
  }


  /**
   * Asks the user to confirm, clears all models, detaches the type view,
   * writes a backup, removes the type, reinstalls the proxy model, and clears
   * the widgets.
   * @param data central application state.
   */
  void remove(colman::AppData& data)
  {
    int const type_id = colman::selected_id(data.mw_.item_type_type_view_);
    if(type_id >= 0) {
      int const choice = colman::warning_yes_no(
          data.mw_.item_type_page_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("Removing an item type also removes corresponding "
                           "items, acquisitions, liquidations, lendings and "
                           "returns. "
                           "Do you really want to remove this type?"));
      switch(choice) {
      case QMessageBox::Yes:
        clear_models(data);
        data.mw_.item_type_type_view_->setModel(nullptr);
        ::remove_type(data, type_id);
        reset_proxy(data);
        show_type(data, -1);
        break;
      default:
        break;
      }
    }
    check_all(data);
  }


  /**
   * Clears the mode-switch functions, navigates back to the central page, and
   * releases the type model.
   * @param data central application state.
   */
  void leave_item_type_page(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.type_model_.set_types({});
  }


  /**
   * Switches to the item type page: clears mode-switch functions, resets the
   * types query, clears the selection, and updates all button states.
   * @param data central application state.
   */
  void enter_item_type_page(colman::AppData& data)
  {
    reset_type_query(data);
    show_type(data, -1);
    data.mw_.item_type_type_view_->clearSelection();
    check_all(data);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.item_type_page_);
  }


} // namespace


/**
 * Refreshes the item type page after an undo or redo operation.
 * @param data central application state.
 */
void colman::refresh_item_type_page(colman::AppData& data)
{
  enter_item_type_page(data);
}


/**
 * One-time setup for the item type page: installs a proxy sort model, an
 * event filter on the type view viewport, and connects all UI signals to their
 * handler lambdas.
 * @param app  Qt application object.
 * @param data central application state.
 */
void colman::setup_item_type_page(QApplication& app, colman::AppData& data)
{
  // Add page icon alongside existing heading label (vbox[0]=back, vbox[1]=label)
  {
    auto* vbox = qobject_cast<QVBoxLayout*>(
        data.mw_.item_type_page_->layout());
    QLayoutItem* heading_item = vbox->takeAt(1);
    QWidget* heading_widget = heading_item->widget();
    delete heading_item;
    QHBoxLayout* title_row = new QHBoxLayout;
    QLabel* icon_lbl = new QLabel(data.mw_.item_type_page_);
    QPixmap const pm(":/icons/owls/edit_types.jpeg");
    if(!pm.isNull()) {
      icon_lbl->setPixmap(
          pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      icon_lbl->setFixedSize(52, 52);
      icon_lbl->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    }
    title_row->addWidget(icon_lbl);
    title_row->addWidget(heading_widget);
    title_row->addStretch();
    vbox->insertLayout(1, title_row);
  }

  QSortFilterProxyModel*
      sqlproxy = new QSortFilterProxyModel(data.mw_.item_type_type_view_);
  sqlproxy->setSourceModel(&data.type_model_);
  data.mw_.item_type_type_view_->setModel(sqlproxy);
  data.mw_.item_type_type_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  data.mw_.item_type_type_view_->viewport()->installEventFilter(new ClearSelectionFilter(data.mw_.item_type_type_view_));
  reset_type_query(data);
  // Populate the search-category (media kind) selector.  The stored item data
  // is the MediaKind enum value; it maps the type to an internet search
  // category independently of the type's (possibly localised) name.
  {
    auto* cb = data.mw_.item_type_media_kind_;
    cb->clear();
    cb->addItem(QApplication::tr("none"),
                static_cast<int>(colman::MediaKind::Unknown));
    cb->addItem(QApplication::tr("book"),
                static_cast<int>(colman::MediaKind::Book));
    cb->addItem(QApplication::tr("CD"),
                static_cast<int>(colman::MediaKind::Cd));
    cb->addItem(QApplication::tr("DVD"),
                static_cast<int>(colman::MediaKind::Dvd));
    cb->addItem(QApplication::tr("Blu-ray"),
                static_cast<int>(colman::MediaKind::BluRay));
    cb->addItem(QApplication::tr("board game"),
                static_cast<int>(colman::MediaKind::BoardGame));
  }
  data.mw_.item_type_fields_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  app.connect(data.item_type_button_, &QToolButton::clicked,
              [&data]() {
                enter_item_type_page(data);
              });
  app.connect(data.mw_.item_type_add_field_, &QPushButton::clicked,
              [&data]() {
                add_field(*data.mw_.item_type_fields_);
              });
  app.connect(data.mw_.item_type_remove_field_, &QPushButton::clicked,
              [&data]() {
                remove_field(*data.mw_.item_type_fields_);
              });
  app.connect(data.mw_.item_type_move_up_, &QPushButton::clicked,
              [&data]() {
                move_field_up(*data.mw_.item_type_fields_);
              });
  app.connect(data.mw_.item_type_move_down_, &QPushButton::clicked,
              [&data]() {
                move_field_down(*data.mw_.item_type_fields_);
              });
  app.connect(data.mw_.item_type_change_, &QPushButton::clicked,
              [&data]() {
                change(data);
              });
  app.connect(data.mw_.item_type_revert_, &QPushButton::clicked,
              [&data]() {
                revert(data);
              });
  app.connect(data.mw_.item_type_insert_, &QPushButton::clicked,
              [&data]() {
                insert(data);
              });
  app.connect(data.mw_.item_type_remove_, &QPushButton::clicked,
              [&data]() {
                remove(data);
              });
  app.connect(data.mw_.item_type_name_, &QLineEdit::editingFinished,
              [&data]() {
                check_all(data);
              });
  app.connect(data.mw_.item_type_type_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [&data](QItemSelection const& selected,
                      QItemSelection const&) {
                selection_changed(data, selected);
              });
  app.connect(data.mw_.item_type_back_, &QPushButton::clicked,
              [&data]() {
                leave_item_type_page(data);
              });
  {
    auto* menu = new QMenu(data.mw_.item_type_marc21_template_);
    for(auto const& tmpl : colman::marc21_templates()) {
      QString const tname = tmpl.name;
      QString const label =
          QApplication::tr(tname.toUtf8().constData());
      menu->addAction(label, [&data, tname]() {
        apply_marc21_template(*data.mw_.item_type_fields_, tname);
      });
    }
    data.mw_.item_type_marc21_template_->setMenu(menu);
  }
}

/* EOF */
