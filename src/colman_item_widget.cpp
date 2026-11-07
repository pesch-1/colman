// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PersonWidget.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item_widget.hpp"
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_SRU_SERVER_HPP
#  include "colman_sru_server.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_ITEM_TABLE_HPP
#  include "colman_item_table.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_PREVIEW_FILE_DIALOG_HPP
#  include "colman_preview_file_dialog.hpp"
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_QCOMPLETER
#  include <QCompleter>
#  define SYSTEM_QCOMPLETER
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QABSTRACTSCROLLAREA
#  include <QAbstractScrollArea>
#  define SYSTEM_QABSTRACTSCROLLAREA
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QDESKTOPSERVICES
#  include <QDesktopServices>
#  define SYSTEM_QDESKTOPSERVICES
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
#ifndef COLMAN_PARTIAL_DATE_EDIT_HPP
#  include "colman_partial_date_edit.hpp"
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif
#ifndef SYSTEM_QMAP
#  include <QMap>
#  define SYSTEM_QMAP
#endif
#ifndef SYSTEM_QBUFFER
#  include <QBuffer>
#  define SYSTEM_QBUFFER
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef COLMAN_BGG_QUERIES_HPP
#  include "colman_bgg_queries.hpp"
#endif
#ifndef COLMAN_NAME_QUERIES_HPP
#  include "colman_name_queries.hpp"
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_FUNCTIONAL
#  include <functional>
#  define SYSTEM_FUNCTIONAL
#endif
#ifndef SYSTEM_OPTIONAL
#  include <optional>
#  define SYSTEM_OPTIONAL
#endif
#ifndef COLMAN_OPENGTINDB_HPP
#  include "colman_opengtindb.hpp"
#  define COLMAN_OPENGTINDB_HPP
#endif
#ifndef COLMAN_AMAZON_BROWSER_HPP
#  include "colman_amazon_browser.hpp"
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QGUIAPPLICATION
#  include <QGuiApplication>
#  define SYSTEM_QGUIAPPLICATION
#endif
#ifndef SYSTEM_QSCREEN
#  include <QScreen>
#  define SYSTEM_QSCREEN
#endif
#ifndef SYSTEM_QPROGRESSDIALOG
#  include <QProgressDialog>
#  define SYSTEM_QPROGRESSDIALOG
#endif
#ifndef SYSTEM_QTCONCURRENTRUN
#  include <QtConcurrentRun>
#  define SYSTEM_QTCONCURRENTRUN
#endif
#ifndef SYSTEM_QFUTUREWATCHER
#  include <QFutureWatcher>
#  define SYSTEM_QFUTUREWATCHER
#endif
#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif

namespace
{
  /// Lets an input field react to the wheel only while it is focused.
  /**
    Combo boxes, spin boxes and the partial-date editor normally consume
    wheel events whenever the pointer happens to hover over them.  While the
    user scrolls the field list this is disruptive: values change at random
    and, worst of all, the date editor's stepBy() pops up a modal calendar
    as soon as the field drifts under the cursor, blocking further scrolling.

    This filter forwards wheel events to the nearest scroll area as long as
    the guarded widget is not the active field.  Only after the user clicks
    into the field (giving it focus) does the wheel act on the field itself;
    clicking elsewhere returns control to the list.  Pair it with
    Qt::StrongFocus so the wheel never grabs focus on its own.
  */
  class WheelGuardFilter : public QObject
  {
  public:
    explicit WheelGuardFilter(QObject* parent = nullptr)
        : QObject(parent) {}
    bool eventFilter(QObject* obj, QEvent* event) override
    {
      if(event->type() == QEvent::Wheel) {
        QWidget* w = qobject_cast<QWidget*>(obj);
        if(w && w->hasFocus()) {
          // The field is selected: let it handle the wheel itself.
          return false;
        }
        QWidget* p = w ? w->parentWidget() : nullptr;
        while(p) {
          if(auto* sa = qobject_cast<QAbstractScrollArea*>(p)) {
            QCoreApplication::sendEvent(sa->viewport(), event);
            return true;
          }
          p = p->parentWidget();
        }
        return true;
      }
      return QObject::eventFilter(obj, event);
    }
  };

  /// Attaches an auto-completer to a line edit using distinct values from a DB
  /// column.
  void attach_completer(QLineEdit& edit, QStringList values)
  {
    edit.setCompleter(new QCompleter(std::move(values), &edit));
  }

  /// Removes all dynamically added type-specific widgets from the item grid.
  /**
    Identifies type-specific rows by the "db_column" property on the col-0
    label (set by fill_row_label). Works regardless of which grid rows the
    type-specific fields currently occupy.
    @param item_grid The grid layout to clear of type-specific widgets.
  */
  void remove_item_widgets(QGridLayout* item_grid)
  {
    int const rows = item_grid->rowCount();
    int const cols = item_grid->columnCount();
    std::vector<QWidget*> to_delete;
    for(int r = 0; r < rows; ++r) {
      QLayoutItem* it0 = item_grid->itemAtPosition(r, 0);
      if(!it0) {
        continue;
      }
      QWidget* w0 = it0->widget();
      if(!w0 || w0->property("db_column").isNull()) {
        continue;
      }
      to_delete.push_back(w0);
      for(int c = 1; c < cols; ++c) {
        QLayoutItem* it = item_grid->itemAtPosition(r, c);
        if(it && it->widget()) {
          to_delete.push_back(it->widget());
        }
      }
    }
    for(auto* w : to_delete) {
      item_grid->removeWidget(w);
      w->hide();
      w->deleteLater();
    }
  }

  /// Detaches all common-field widgets from the grid without deleting them.
  /**
    Called before re-adding them in configured order.  Each widget is hidden
    so it does not appear until explicitly re-shown by add_common_field().
    The thumbnail HBoxLayout is found by identity scan.
    @param grid  item grid layout.
    @param w     generated UI struct owning the common-field widgets.
  */
  void remove_common_fields(QGridLayout& grid, colman::ItemWidgetUi& w,
                            QWidget* keywords_widget)
  {
    auto remove_w = [&grid](QWidget* widget) {
      if(widget) {
        grid.removeWidget(widget);
        widget->hide();
      }
    };
    remove_w(w.label_21_);
    remove_w(w.name_);
    remove_w(w.label_22_);
    for(int i = 0; i < grid.count(); ++i) {
      QLayoutItem* it = grid.itemAt(i);
      if(it && it->layout() == w.thumbnail_layout_) {
        grid.removeItem(it);
        break;
      }
    }
    remove_w(w.is_schoolitem_);
    remove_w(w.label_100_);
    remove_w(w.subject_);
    remove_w(w.label_101_);
    remove_w(w.academic_year_);
    remove_w(w.label_102_);
    remove_w(w.school_type_);
    remove_w(w.label_23_);
    remove_w(w.place_);
    remove_w(w.label_sig_top_);
    remove_w(w.signature_top_);
    remove_w(w.label_sig_bottom_);
    remove_w(w.signature_bottom_);
    remove_w(w.label_24_);
    remove_w(w.description_);
    remove_w(w.label_keywords_);
    remove_w(w.keywords_placeholder_);
    remove_w(keywords_widget);
    remove_w(w.label_barcode_);
    remove_w(w.barcode_display_);
  }

  /// Re-adds one common-field block to the grid at @p row and returns next row.
  /**
    Each sentinel maps to a fixed set of widgets in @p w.  The school-item
    block adds four rows (checkbox + three sub-fields); all others add one.
    Sub-field visibility for __school_item__ follows the checkbox state.
    @param grid      item grid layout.
    @param w         generated UI struct owning the common-field widgets.
    @param sentinel  common-field identifier (e.g. "__name__").
    @param row       grid row to start at.
    @return the next available grid row after this block.
  */
  int add_common_field(QGridLayout& grid, colman::ItemWidgetUi& w,
                       QString const& sentinel, int row,
                       QWidget* keywords_widget)
  {
    if(sentinel == "__name__") {
      grid.addWidget(w.label_21_, row, 0);
      grid.addWidget(w.name_, row, 1);
      w.label_21_->show();
      w.name_->show();
      return row + 1;
    }
    if(sentinel == "__barcode__") {
      grid.addWidget(w.label_barcode_, row, 0);
      grid.addWidget(w.barcode_display_, row, 1);
      w.label_barcode_->show();
      w.barcode_display_->show();
      return row + 1;
    }
    if(sentinel == "__thumbnail__") {
      grid.addWidget(w.label_22_, row, 0);
      grid.addLayout(w.thumbnail_layout_, row, 1);
      w.label_22_->show();
      w.thumbnail_->show();
      w.browse_thumbnail_->show();
      w.scan_thumbnail_->show();
      w.amazon_thumbnail_->show();
      w.remove_thumbnail_->show();
      return row + 1;
    }
    if(sentinel == "__school_item__") {
      grid.addWidget(w.is_schoolitem_, row, 1);
      w.is_schoolitem_->show();
      ++row;
      bool const s = w.is_schoolitem_->isChecked();
      grid.addWidget(w.label_100_, row, 0);
      grid.addWidget(w.subject_, row, 1);
      w.label_100_->setVisible(s);
      w.subject_->setVisible(s);
      ++row;
      grid.addWidget(w.label_101_, row, 0);
      grid.addWidget(w.academic_year_, row, 1);
      w.label_101_->setVisible(s);
      w.academic_year_->setVisible(s);
      ++row;
      grid.addWidget(w.label_102_, row, 0);
      grid.addWidget(w.school_type_, row, 1);
      w.label_102_->setVisible(s);
      w.school_type_->setVisible(s);
      return row + 1;
    }
    if(sentinel == "__place__") {
      grid.addWidget(w.label_23_, row, 0);
      grid.addWidget(w.place_, row, 1);
      w.label_23_->show();
      w.place_->show();
      return row + 1;
    }
    if(sentinel == "__sig_top__") {
      grid.addWidget(w.label_sig_top_, row, 0);
      grid.addWidget(w.signature_top_, row, 1);
      w.label_sig_top_->show();
      w.signature_top_->show();
      return row + 1;
    }
    if(sentinel == "__sig_bottom__") {
      grid.addWidget(w.label_sig_bottom_, row, 0);
      grid.addWidget(w.signature_bottom_, row, 1);
      w.label_sig_bottom_->show();
      w.signature_bottom_->show();
      return row + 1;
    }
    if(sentinel == "__description__") {
      grid.addWidget(w.label_24_, row, 0);
      grid.addWidget(w.description_, row, 1);
      w.label_24_->show();
      w.description_->show();
      return row + 1;
    }
    if(sentinel == "__keywords__") {
      grid.addWidget(w.label_keywords_, row, 0);
      grid.addWidget(keywords_widget, row, 1);
      w.label_keywords_->show();
      keywords_widget->show();
      return row + 1;
    }
    return row;
  }

  /// Returns the grid row of the type-specific field with the given key.
  /**
    Searches col-0 labels for one whose "db_column" property matches @p key.
    @param grid the item grid layout.
    @param key  database column name to search for.
    @return the matching row index, or -1 if not found.
  */
  int find_type_specific_row(QGridLayout& grid, QString const& key)
  {
    int const rows = grid.rowCount();
    for(int r = 0; r < rows; ++r) {
      QLayoutItem* it = grid.itemAtPosition(r, 0);
      if(!it) {
        continue;
      }
      QWidget* w = it->widget();
      if(w && w->property("db_column").toString() == key) {
        return r;
      }
    }
    return -1;
  }

  /// Resets the thumbnail label to an empty state and clears its stored ID.
  /**
    Sets the thumbnail property `thumbnail_id` to -1 and clears the
    displayed text/pixmap of the thumbnail label.
    @param widget The UI widget whose thumbnail label is to be cleared.
  */
  void clear_thumbnail(colman::ItemWidgetUi& widget)
  {
    widget.thumbnail_->setProperty("thumbnail_id", -1);
    widget.thumbnail_->setText(" ");
    widget.thumbnail_->setText("");
    widget.zoom_thumbnail_->setEnabled(false);
    widget.remove_thumbnail_->setEnabled(false);
  }

  /// Reads the string value from a type-specific attribute widget.
  /**
    Handles PartialDateEdit, QLineEdit, QPlainTextEdit, and QPushButton
    (table data). Returns an empty string for unknown widget types.
    @param widget the widget to read; may be nullptr (returns "").
    @return the current value as a string.
  */
  QString read_attribute_widget_value(QWidget* widget)
  {
    if(!widget) {
      return {};
    }
    colman::PartialDateEdit* const pde =
        qobject_cast<colman::PartialDateEdit*>(widget);
    if(pde) {
      return pde->dateString();
    }
    QLineEdit* line = qobject_cast<QLineEdit*>(widget);
    if(line) {
      return line->text();
    }
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(widget);
    if(edit) {
      return edit->toPlainText();
    }
    QPushButton* button = qobject_cast<QPushButton*>(widget);
    if(button) {
      QVariant const var = button->property("table_data");
      QByteArray data;
      QDataStream stream(&data, QIODevice::WriteOnly);
      stream << var;
      data = data.toBase64();
      return QString(data);
    }
    return {};
  }

  /// Sets the value of a type-specific attribute widget from a string.
  /**
    Handles PartialDateEdit, QLineEdit, and QPlainTextEdit. Table-data
    QPushButton widgets are intentionally not restored (the serialised form is
    complex and table fields are rarely shared across types). Has no effect for
    null or unrecognised widget types.
    @param widget the widget to write to; may be nullptr (no-op).
    @param value  the string to restore into the widget.
  */
  void set_attribute_widget_value(QWidget* widget, QString const& value)
  {
    if(!widget || value.isEmpty()) {
      return;
    }
    colman::PartialDateEdit* const pde =
        qobject_cast<colman::PartialDateEdit*>(widget);
    if(pde) {
      pde->setDateString(value);
      return;
    }
    QLineEdit* line = qobject_cast<QLineEdit*>(widget);
    if(line) {
      line->setText(value);
      return;
    }
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(widget);
    if(edit) {
      edit->setPlainText(value);
    }
  }

  /// Resets a common-field or type-specific attribute widget to its empty
  /// state.
  /**
    Handles ListValueWidget, QLineEdit, QPlainTextEdit, PartialDateEdit,
    QComboBox, QSpinBox, and table-data QPushButton widgets. Has no effect for
    null or unrecognised widget types (notably QCheckBox, which the caller
    resets to a type-specific default).
    @param widget the widget to clear; may be nullptr (no-op).
  */
  void clear_attribute_widget(QWidget* widget)
  {
    if(!widget) {
      return;
    }
    auto* const lv = qobject_cast<colman::ListValueWidget*>(widget);
    if(lv) {
      lv->set_values({});
      return;
    }
    auto* const line = qobject_cast<QLineEdit*>(widget);
    if(line) {
      line->blockSignals(true);
      line->setText("");
      line->setProperty("last_text", "");
      line->blockSignals(false);
      return;
    }
    auto* const edit = qobject_cast<QPlainTextEdit*>(widget);
    if(edit) {
      edit->setPlainText("");
      return;
    }
    auto* const pde = qobject_cast<colman::PartialDateEdit*>(widget);
    if(pde) {
      pde->setDateString("");
      return;
    }
    auto* const combo = qobject_cast<QComboBox*>(widget);
    if(combo) {
      if(combo->isEditable()) {
        combo->clearEditText();
      }
      else {
        combo->setCurrentIndex(-1);
      }
      return;
    }
    auto* const spin = qobject_cast<QSpinBox*>(widget);
    if(spin) {
      spin->setValue(0);
      return;
    }
    auto* const button = qobject_cast<QPushButton*>(widget);
    if(button) {
      button->setProperty("table_data", QVariant());
    }
  }

  /// Returns a short author/artist label for a search result, if available.
  /**
    Online title searches populate "authors" for books (Open Library,
    Google Books, SRU) and "artist" for recordings (MusicBrainz).  This
    helper returns whichever list is present, joined for display, so the
    selection list can tell apart items that share the same title.  Returns
    an empty string when no author information is available.

    @param item  The full record of a single search result.
    @return Joined author/artist string, or empty if none is known.
  */
  QString name_result_author(colman::Item const& item)
  {
    auto const& lists = item.get_list_values();
    for(auto const& key : {QString("authors"), QString("artist")}) {
      auto const it = lists.find(key);
      if(it != lists.end() && !it->second.isEmpty()) {
        return it->second.join(", ");
      }
    }
    return QString();
  }

} // namespace

/// Attaches auto-completers for the place, subject, and school-type fields.
/**
  Sets up QCompleter instances on the place, subject, and school_type line
  edits using distinct values already stored in the database. Has no effect
  if no AppData is associated.
*/
void colman::ItemWidget::set_completers()
{
  if((data_)) {
    attach_completer(*widget_.place_,
                     data_->queries_->query_item_field_completions("place"));
    attach_completer(*widget_.subject_,
                     data_->queries_->query_school_item_field_completions(
                         "subject"));
    attach_completer(*widget_.school_type_,
                     data_->queries_->query_school_item_field_completions(
                         "type"));
    widget_.signature_top_->clear();
    for(auto const& name : data_->queries_->read_signature_top_categories()) {
      widget_.signature_top_->addItem(name);
    }
    refresh_signature_bottom();
  }
}

/// Detaches auto-completers from the place, subject, and school-type fields.
/**
  Sets the completer on the place, subject, and school_type line edits to
  nullptr, effectively removing any previously installed completers. Has no
  effect if no AppData is associated.
*/
void colman::ItemWidget::remove_completers()
{
  if((data_)) {
    widget_.place_->setCompleter(nullptr);
    widget_.subject_->setCompleter(nullptr);
    widget_.school_type_->setCompleter(nullptr);
  }
}

/// Repopulates signature_bottom_ with subcategories matching the current
/// signature_top_ value.
/**
  Queries signature_bottom_categories for all entries whose top_category
  equals the current signature_top_ text and fills signature_bottom_ with
  the results.  Has no effect if no AppData is associated.
*/
void colman::ItemWidget::refresh_signature_bottom()
{
  if(!data_) {
    return;
  }
  QString const top = widget_.signature_top_->currentText();
  widget_.signature_bottom_->clear();
  for(auto const& name : data_->queries_->read_signature_bottom_categories(top))
  {
    widget_.signature_bottom_->addItem(name);
  }
}

/// Writes the default signatures into the signature combo boxes.
/**
  Sets signature_top_ to default_signature_top_, repopulates the bottom
  categories for it and sets signature_bottom_ to default_signature_bottom_.
  With empty defaults both combo boxes end up with empty edit text.
*/
void colman::ItemWidget::apply_default_signatures()
{
  widget_.signature_top_->setCurrentText(default_signature_top_);
  refresh_signature_bottom();
  widget_.signature_bottom_->setCurrentText(default_signature_bottom_);
}

/// Sets the signatures pre-filled whenever the fields are reset.
/**
  Only stores the values; they take effect on the next clear_fields() or
  item-type switch, so input currently being edited is not overwritten.
  @param top    default top signature (empty for none).
  @param bottom default bottom signature (empty for none).
*/
void colman::ItemWidget::set_default_signatures(QString const& top,
                                                QString const& bottom)
{
  default_signature_top_ = top;
  default_signature_bottom_ = bottom;
}

/// Hides the type selector combo box.
void colman::ItemWidget::hide_type_selector()
{
  widget_.item_type_box_->hide();
}

/// Repopulates the item-type combo box from the database and selects the
/// default.
/**
  Queries all available item types from the database, fills the type combo
  box, then selects the collection's default item type and sets the school-
  item checkbox to its default value. Has no effect if no AppData is
  associated.
*/
void colman::ItemWidget::reset_item_type()
{
  if((data_)) {
    auto const types = data_->queries_->get_type_locale();
    colman::Collection const collection = data_->queries_->read_collection();
    int default_type_id = 1;
    QString default_type = data_->queries_->get_type_name(default_type_id);
    {
      colman::AutoBlockSignals b(widget_.item_type_box_);
      widget_.item_type_box_->clear();
      for(auto const& type : types) {
        widget_.item_type_box_->addItem(type.second, type.first);
        if(type.first == collection.get_default_item_type()) {
          default_type = type.second;
          default_type_id = type.first;
        }
      }
    }
    select_item_type(*data_, default_type_id, default_type);
    widget_.is_schoolitem_->setChecked(collection.get_default_is_school_item());
  }
}

/// Repopulates the item type combo box and selects the given type.
/**
  Identical to reset_item_type() but selects @p type_id instead of the
  collection's default type. Falls back to the default when @p type_id is
  not found in the database.
  @param type_id Database identifier of the type to pre-select.
*/
void colman::ItemWidget::reset_item_type(int type_id)
{
  if(!(data_)) {
    return;
  }
  auto const types = data_->queries_->get_type_locale();
  colman::Collection const collection = data_->queries_->read_collection();
  int selected_id = type_id;
  QString selected_trans;
  bool found = false;
  int fallback_id = collection.get_default_item_type();
  QString fallback_trans;
  {
    colman::AutoBlockSignals b(widget_.item_type_box_);
    widget_.item_type_box_->clear();
    for(auto const& t : types) {
      widget_.item_type_box_->addItem(t.second, t.first);
      if(t.first == type_id) {
        found = true;
        selected_trans = t.second;
      }
      if(t.first == fallback_id) {
        fallback_trans = t.second;
      }
    }
  }
  if(!found) {
    selected_id = fallback_id;
    selected_trans = fallback_trans;
  }
  select_item_type(*data_, selected_id, selected_trans);
  widget_.is_schoolitem_->setChecked(collection.get_default_is_school_item());
}

/// Switches the widget to display fields for the given item type.
/**
  Removes any existing type-specific attribute rows, clears all common
  fields, then adds new rows for each attribute defined by the type
  identified by @p type_id. Fields are created read-only or editable
  according to the current read-only state. Updates the combo box selection
  without emitting extra signals.
  @param data    Application data holding the database connection.
  @param type_id Database identifier of the item type to switch to.
  @param trans   Translated display name for the selected type, used to
                 update the combo box text.
*/
void colman::ItemWidget::select_item_type(colman::AppData& data,
                                          int type_id,
                                          QString const& trans)
{
  remove_item_widgets(widget_.item_grid_);
  remove_common_fields(*widget_.item_grid_, widget_, keywords_lv_);
  last_lookup_key_.clear();
  last_lookup_thumbnail_url_.clear();
  clear_thumbnail(widget_);
  widget_.name_->setText("");
  widget_.barcode_display_->setText("");
  widget_.place_->setText("");
  apply_default_signatures();
  widget_.description_->setPlainText("");
  keywords_lv_->set_values({});
  widget_.subject_->setText("");
  widget_.school_type_->setText("");
  widget_.academic_year_->setText("");
  QStringList const field_order =
      data.queries_->get_combined_field_order(type_id);
  colman::Type type = data.queries_->get_type(type_id);
  QMap<QString, colman::KeyInfo const*> key_map;
  for(auto const& key : type.get_keys()) {
    key_map[key.first] = &key;
  }
  int row = 0;
  for(auto const& field : field_order) {
    if(field.startsWith("__")) {
      row = add_common_field(*widget_.item_grid_, widget_, field, row,
                             keywords_lv_);
    }
    else {
      auto it = key_map.find(field);
      if(it != key_map.end()) {
        if(colman::is_repeatable(it.value()->second)) {
          if(read_only_) {
            colman::fill_row_ro(data, widget_.item_grid_, row++,
                                it.value()->first, QStringList{},
                                type.get_id(), it.value()->second);
          }
          else {
            colman::fill_row_rw(
                data, widget_.item_grid_, row++, it.value()->first,
                QStringList{}, type.get_id(), it.value()->second,
                [this](colman::AppData&) { this->some_field_changed(); });
          }
        }
        else {
          if(read_only_) {
            colman::fill_row_ro(data, widget_.item_grid_, row++,
                                it.value()->first, "", type.get_id(),
                                it.value()->second);
          }
          else {
            colman::fill_row_rw(
                data, widget_.item_grid_, row++, it.value()->first, "",
                type.get_id(), it.value()->second,
                [this](colman::AppData&) { this->some_field_changed(); });
          }
        }
      }
    }
  }
  {
    QWidget* prev = nullptr;
    int const n = widget_.item_grid_->rowCount();
    for(int r = 0; r < n; ++r) {
      auto* li = widget_.item_grid_->itemAtPosition(r, 1);
      if(!li) {
        continue;
      }
      QWidget* w = li->widget();
      if(!w || !(w->focusPolicy() & Qt::TabFocus)) {
        continue;
      }
      if(prev) {
        QWidget::setTabOrder(prev, w);
      }
      prev = w;
    }
  }
  install_wheel_guards_in_grid();
  // The grid was rebuilt: rows were removed and re-added, including the nested
  // thumbnail layout via removeItem()/addLayout().  QGridLayout caches its size
  // hints, and re-adding a nested layout leaves that cache stale.  Without an
  // explicit invalidate the stale geometry survives a later hide/show, so when
  // the page is shown again (e.g. returning from the internet-search page after
  // a "not found") the composite rows — thumbnail buttons and list fields —
  // collapse to a sliver while their labels keep normal size.  Recompute now.
  widget_.item_grid_->invalidate();
  widget_.item_widget_->adjustSize();
  colman::AutoBlockSignals b(widget_.item_type_box_);
  widget_.item_type_box_->setCurrentText(trans);
}

/// Captures all current field values for a later restore.
/**
  Saves the common fields (name, place, description, keywords, signatures,
  and the school-item block), the school-item checkbox state, the loaded
  thumbnail id, and every type-specific attribute row keyed by its database
  column. Used together with restore_field_values() to preserve user input
  across an item-type switch.
  @return snapshot suitable for restore_field_values().
*/
colman::ItemWidget::SavedFieldValues
colman::ItemWidget::save_field_values() const
{
  SavedFieldValues saved;
  saved.name_ = widget_.name_->text();
  saved.place_ = widget_.place_->text();
  saved.description_ = widget_.description_->toPlainText();
  saved.keywords_ = keywords_lv_->get_values();
  saved.signature_top_ = widget_.signature_top_->currentText();
  saved.signature_bottom_ = widget_.signature_bottom_->currentText();
  saved.subject_ = widget_.subject_->text();
  saved.school_type_ = widget_.school_type_->text();
  saved.academic_year_ = widget_.academic_year_->text();
  saved.is_schoolitem_ = widget_.is_schoolitem_->isChecked();
  saved.thumbnail_id_ = widget_.thumbnail_->property("thumbnail_id").toInt();
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* li0 = widget_.item_grid_->itemAtPosition(r, 0);
    auto* li1 = widget_.item_grid_->itemAtPosition(r, 1);
    if(!(li0) || !(li1)) {
      continue;
    }
    auto* lbl = qobject_cast<QLabel*>(li0->widget());
    if(!lbl) {
      continue;
    }
    QString const key = lbl->property("db_column").toString();
    if(key.isEmpty()) {
      continue;
    }
    auto* lv = qobject_cast<colman::ListValueWidget*>(li1->widget());
    if(lv) {
      saved.attribute_list_values_[key] = lv->get_values();
    }
    else {
      saved.attribute_values_[key] =
          read_attribute_widget_value(li1->widget());
    }
  }
  return saved;
}

/// Writes back the field values captured by save_field_values().
/**
  Restores non-empty common fields, the school-item checkbox state, all
  type-specific attributes whose database column exists in the current
  type's rows, and the thumbnail. Values for columns the current type does
  not define are silently dropped; empty saved values never overwrite
  field defaults.
  @param saved snapshot returned by save_field_values().
*/
void colman::ItemWidget::restore_field_values(SavedFieldValues const& saved)
{
  if(!saved.name_.isEmpty()) {
    widget_.name_->setText(saved.name_);
  }
  if(!saved.place_.isEmpty()) {
    widget_.place_->setText(saved.place_);
  }
  if(!saved.description_.isEmpty()) {
    widget_.description_->setPlainText(saved.description_);
  }
  if(!saved.keywords_.isEmpty()) {
    keywords_lv_->set_values(saved.keywords_);
  }
  if(!saved.signature_top_.isEmpty()) {
    widget_.signature_top_->setCurrentText(saved.signature_top_);
  }
  if(!saved.signature_bottom_.isEmpty()) {
    widget_.signature_bottom_->setCurrentText(saved.signature_bottom_);
  }
  if(!saved.subject_.isEmpty()) {
    widget_.subject_->setText(saved.subject_);
  }
  if(!saved.school_type_.isEmpty()) {
    widget_.school_type_->setText(saved.school_type_);
  }
  if(!saved.academic_year_.isEmpty()) {
    widget_.academic_year_->setText(saved.academic_year_);
  }
  widget_.is_schoolitem_->setChecked(saved.is_schoolitem_);
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* li0 = widget_.item_grid_->itemAtPosition(r, 0);
    auto* li1 = widget_.item_grid_->itemAtPosition(r, 1);
    if(!(li0) || !(li1)) {
      continue;
    }
    auto* lbl = qobject_cast<QLabel*>(li0->widget());
    if(!lbl) {
      continue;
    }
    QString const key = lbl->property("db_column").toString();
    if(key.isEmpty()) {
      continue;
    }
    auto* lv = qobject_cast<colman::ListValueWidget*>(li1->widget());
    if(lv) {
      auto const it = saved.attribute_list_values_.find(key);
      if(it != saved.attribute_list_values_.end() && !it->second.isEmpty()) {
        lv->set_values(it->second);
      }
    }
    else {
      auto const it = saved.attribute_values_.find(key);
      if(it != saved.attribute_values_.end() && !it->second.isEmpty()) {
        set_attribute_widget_value(li1->widget(), it->second);
      }
    }
  }
  if(saved.thumbnail_id_ > 0) {
    use_thumbnail(saved.thumbnail_id_);
  }
}

/**
   Constructs the ItemWidget instance.

   Sets up the generated UI, installs an academic-year input validator, and
   connects all child-widget signals to the appropriate slots.
   @param parent Optional parent widget; ownership follows Qt conventions.
*/
colman::ItemWidget::ItemWidget(QWidget* parent)
    : QWidget(parent)
{
  widget_.setup_ui(this);
  keywords_lv_ = new colman::ListValueWidget(this);
  wheel_guard_ = new WheelGuardFilter(this);
  install_wheel_guard(widget_.item_type_box_);
  install_wheel_guard(widget_.signature_top_);
  install_wheel_guard(widget_.signature_bottom_);
  install_wheel_guard(widget_.description_);
  QRegularExpression rx(
      "((\\d{1,2}|\\d{1,2}-\\d{1,2})(\\,\\s*(\\d{1,2}|\\d{1,2}-\\d{1,2}))*)?");
  QRegularExpressionValidator* validator =
      new QRegularExpressionValidator(rx, widget_.academic_year_);
  widget_.academic_year_->setValidator(validator);
  connect(widget_.name_, &QLineEdit::editingFinished, this,
          &ItemWidget::some_field_changed);
  connect(widget_.subject_, &QLineEdit::editingFinished, this,
          &ItemWidget::some_field_changed);
  connect(widget_.academic_year_, &QLineEdit::editingFinished, this,
          &ItemWidget::some_field_changed);
  connect(widget_.school_type_, &QLineEdit::editingFinished, this,
          &ItemWidget::some_field_changed);
  connect(widget_.place_, &QLineEdit::editingFinished, this,
          &ItemWidget::some_field_changed);
  connect(widget_.signature_top_, &QComboBox::currentTextChanged, this,
          &ItemWidget::some_field_changed);
  connect(widget_.signature_bottom_, &QComboBox::currentTextChanged, this,
          &ItemWidget::some_field_changed);
  connect(widget_.signature_top_, QOverload<int>::of(&QComboBox::activated),
          [this](int) { this->refresh_signature_bottom(); });
  connect(widget_.description_, &QPlainTextEdit::textChanged, this,
          &ItemWidget::some_field_changed);
  connect(keywords_lv_, &colman::ListValueWidget::value_changed, this,
          &ItemWidget::some_field_changed);
  connect(widget_.is_schoolitem_, &QCheckBox::toggled, this,
          &ItemWidget::set_is_schoolitem);
  connect(widget_.browse_thumbnail_, &QPushButton::clicked, this,
          &ItemWidget::browse_thumbnail);
  connect(widget_.scan_thumbnail_, &QPushButton::clicked, this,
          &ItemWidget::scan_thumbnail);
  connect(widget_.amazon_thumbnail_, &QPushButton::clicked, this,
          &ItemWidget::amazon_thumbnail);
  connect(widget_.zoom_thumbnail_, &QPushButton::clicked, this,
          &ItemWidget::zoom_thumbnail);
  connect(widget_.remove_thumbnail_, &QPushButton::clicked, this,
          &ItemWidget::remove_thumbnail);
}

/**
  Sets @p w to strong focus so the wheel cannot select it, and installs the
  shared wheel guard so that wheel events scroll the field list until the
  user clicks into the field.
  @param w input widget to guard, or nullptr (ignored).
*/
void colman::ItemWidget::install_wheel_guard(QWidget* w)
{
  if(!w || w->property("wheel_guarded").toBool()) {
    return;
  }
  w->setProperty("wheel_guarded", true);
  w->setFocusPolicy(Qt::StrongFocus);
  w->installEventFilter(wheel_guard_);
}

/**
  Guards every combo box, spin box and partial-date editor inside the scroll
  area so that scrolling the field list no longer changes their value or pops
  up the calendar dialog by accident.  Called after each type switch since
  the type-specific fields are recreated; install_wheel_guard() is idempotent,
  so common-field combos that survive the rebuild are guarded only once.
*/
void colman::ItemWidget::install_wheel_guards_in_grid()
{
  for(auto* c : widget_.item_widget_->findChildren<QComboBox*>()) {
    install_wheel_guard(c);
  }
  for(auto* s : widget_.item_widget_->findChildren<QAbstractSpinBox*>()) {
    install_wheel_guard(s);
  }
}

/// Destroys the ItemWidget instance.

/// Associates application data with this widget and finalises signal wiring.
/**
  Stores a pointer to @p data and connects the item-type combo box's
  currentTextChanged signal so that switching the type refreshes the
  type-specific attribute rows. Must be called once after construction
  before any other interaction with the widget.
  @param data Application data holding the database connection and UI state.
*/
void colman::ItemWidget::setup(AppData& data)
{
  data_ = &data;
  connect(widget_.item_type_box_, &QComboBox::currentTextChanged,
          [&data, this](QString const& trans) {
            // Preserve field values across the type switch so matching
            // fields keep their content (e.g. when the user corrects
            // DVD → Blu-ray).
            SavedFieldValues const saved = save_field_values();
            int const new_type_id =
                widget_.item_type_box_->currentData().toInt();
            this->select_item_type(data, new_type_id, trans);
            this->restore_field_values(saved);
          });

  int const uid = data.current_data_collector_;
  bool const can_admin =
      uid >= 0 && data.queries_->has_permission(uid, "admin_permissions");
  if(can_admin) {
    auto* grp = new QGroupBox(
        QCoreApplication::tr("Reservation Settings"), this);
    auto* vbox = new QVBoxLayout(grp);

    auto* allow_row = new QHBoxLayout;
    allow_row->addWidget(
        new QLabel(QCoreApplication::tr("Allow reservations:"), grp));
    item_allow_reservations_combo_ = new QComboBox(grp);
    item_allow_reservations_combo_->addItem(
        QCoreApplication::tr("Default (collection setting)"), -1);
    item_allow_reservations_combo_->addItem(
        QCoreApplication::tr("Allowed"), 1);
    item_allow_reservations_combo_->addItem(
        QCoreApplication::tr("Not allowed"), 0);
    allow_row->addWidget(item_allow_reservations_combo_);
    allow_row->addStretch(1);
    vbox->addLayout(allow_row);

    auto* period_row = new QHBoxLayout;
    item_reservation_period_override_check_ =
        new QCheckBox(QCoreApplication::tr("Override period:"), grp);
    period_row->addWidget(item_reservation_period_override_check_);
    item_reservation_period_days_spin_ = new QSpinBox(grp);
    item_reservation_period_days_spin_->setRange(1, 365);
    item_reservation_period_days_spin_->setValue(14);
    item_reservation_period_days_spin_->setEnabled(false);
    period_row->addWidget(item_reservation_period_days_spin_);
    period_row->addStretch(1);
    vbox->addLayout(period_row);
    install_wheel_guard(item_allow_reservations_combo_);
    install_wheel_guard(item_reservation_period_days_spin_);

    connect(item_reservation_period_override_check_, &QCheckBox::toggled,
            item_reservation_period_days_spin_, &QSpinBox::setEnabled);
    connect(item_allow_reservations_combo_,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ItemWidget::some_field_changed);
    connect(item_reservation_period_override_check_, &QCheckBox::toggled,
            this, &ItemWidget::some_field_changed);
    connect(item_reservation_period_days_spin_,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ItemWidget::some_field_changed);

    auto* main_layout = qobject_cast<QVBoxLayout*>(layout());
    if(main_layout) {
      main_layout->addWidget(grp);
    }
  }
}

/// Slot called whenever any editable field in the widget changes value.
/**
  Emits the field_changed() signal to notify observers that the widget
  content has been modified by the user.
*/
void colman::ItemWidget::some_field_changed()
{
  Q_EMIT field_changed();
}

/// Switches the widget between read-only and editable mode.
/**
  Should be called before filling data, since additional type-specific
  fields are configured as read-only or editable at creation time only.
  When @p ro changes from its current value, all input widgets are updated
  accordingly and the type combo box is always left disabled.
  @param ro True to make all fields read-only; false to make them editable.
*/
void colman::ItemWidget::set_read_only(bool ro)
{
  if(read_only_ != ro) {
    read_only_ = ro;
    widget_.browse_thumbnail_->setHidden(ro);
    widget_.scan_thumbnail_->setHidden(ro);
    widget_.amazon_thumbnail_->setHidden(ro);
    widget_.remove_thumbnail_->setHidden(ro);
    widget_.name_->setReadOnly(ro);
    widget_.is_schoolitem_->setDisabled(ro);
    widget_.subject_->setReadOnly(ro);
    widget_.academic_year_->setReadOnly(ro);
    widget_.school_type_->setReadOnly(ro);
    widget_.place_->setReadOnly(ro);
    widget_.signature_top_->setDisabled(ro);
    widget_.signature_bottom_->setDisabled(ro);
    widget_.description_->setReadOnly(ro);
    keywords_lv_->set_read_only(ro);
    widget_.item_type_box_->setEnabled(false);
    // Type-specific list fields (authors, ISBN10/13, DDC, …) bake in their
    // read-only state when they are built by fill_row_rw/fill_row_ro and are
    // not among the fixed widgets above.  Sync them here as well, so toggling
    // read-only after the fields already exist does not leave their input row
    // and +/- buttons in the wrong state — e.g. returning to the acquisition
    // select page from the copy-count step rebuilds the fields read-only and
    // only afterwards calls set_read_only(false).
    int const rows = widget_.item_grid_->rowCount();
    for(int r = 0; r < rows; ++r) {
      QLayoutItem* const cell = widget_.item_grid_->itemAtPosition(r, 1);
      if(!cell) {
        continue;
      }
      if(auto* lv = qobject_cast<colman::ListValueWidget*>(cell->widget())) {
        lv->set_read_only(ro);
      }
    }
  }
}

/// Populates the form fields from an internet lookup result.
/**
  Lookup services never know the collection's shelf signatures, so an empty
  signature in @p item is replaced by the one currently shown in the widget
  (the acquisition default or what the user already typed) before the item
  is displayed via item_to_fields().  A non-empty signature delivered by the
  lookup takes precedence.  Likewise lookups never mark an item as school
  item, so a checked school-item box (e.g. the collection default) is kept.
  @param item the item retrieved by the lookup.
*/
void colman::ItemWidget::lookup_result_to_fields(Item item)
{
  if(item.get_signature_top().trimmed().isEmpty()) {
    item.set_signature_top(widget_.signature_top_->currentText());
  }
  if(item.get_signature_bottom().trimmed().isEmpty()) {
    item.set_signature_bottom(widget_.signature_bottom_->currentText());
  }
  if(widget_.is_schoolitem_->isChecked()) {
    item.set_is_school_item(true);
  }
  item_to_fields(item, "");
}

/// Populates all widget fields from an Item object.
/**
  Transfers the data held in @p item into the corresponding UI controls.
  If @p type_name is non-empty the type combo box is updated to match it.
  Type-specific attribute rows are filled from the item's key-value map,
  and the thumbnail is loaded from the database when a valid thumbnail ID
  is present.
  @param item      The item whose data is displayed in the widget.
  @param type_name The internal type name used to select the combo box
                   entry; pass an empty string to leave the selection
                   unchanged.
*/
void colman::ItemWidget::item_to_fields(Item const& item,
                                        QString const& type_name)
{
  if(!type_name.isEmpty()) {
    // a stored item is loaded: forget lookups done for a previous item
    last_lookup_key_.clear();
    last_lookup_thumbnail_url_.clear();
    QString const trans = data_->queries_->get_type_locale(type_name);
    widget_.item_type_box_->setCurrentText(trans);
  }
  widget_.name_->setText(item.get_name());
  widget_.barcode_display_->setText(item.get_barcode());
  widget_.place_->setText(item.get_place());
  widget_.signature_top_->setCurrentText(item.get_signature_top());
  refresh_signature_bottom();
  widget_.signature_bottom_->setCurrentText(item.get_signature_bottom());
  widget_.subject_->setText(item.get_subject());
  widget_.academic_year_->setText(
      academic_years_to_string(item.get_academic_year()));
  widget_.school_type_->setText(item.get_type());
  widget_.description_->setPlainText(item.get_description());
  keywords_lv_->set_values(item.get_keywords());
  widget_.is_schoolitem_->setChecked(item.get_is_school_item());
  if(item_allow_reservations_combo_) {
    int const ar = item.get_allow_reservations();
    int idx = 0;
    if(ar == 1) {
      idx = 1;
    }
    else if(ar == 0) {
      idx = 2;
    }
    item_allow_reservations_combo_->setCurrentIndex(idx);
  }
  if(item_reservation_period_override_check_ && item_reservation_period_days_spin_) {
    int const rpd = item.get_reservation_period_days();
    bool const override_set = (rpd >= 0);
    item_reservation_period_override_check_->setChecked(override_set);
    item_reservation_period_days_spin_->setEnabled(override_set);
    if(override_set) {
      item_reservation_period_days_spin_->setValue(rpd);
    }
  }
  StringMap const& values = item.get_values();
  if(!data_) {
    return;
  }
  colman::Type type = data_->queries_->get_type(item.get_type_id());
  for(auto const& key : type.get_keys()) {
    int const row = find_type_specific_row(*widget_.item_grid_, key.first);
    if(row < 0) {
      continue;
    }
    if(colman::is_repeatable(key.second)) {
      QStringList list_value;
      auto const lit = item.get_list_values().find(key.first);
      if(lit != item.get_list_values().end()) {
        list_value = lit->second;
      }
      if(read_only_) {
        colman::fill_row_ro(*data_, widget_.item_grid_, row, key.first,
                            list_value, type.get_id(), key.second);
      }
      else {
        colman::fill_row_rw(
            *data_, widget_.item_grid_, row, key.first, list_value,
            type.get_id(), key.second,
            [this](colman::AppData&) { this->some_field_changed(); });
      }
      continue;
    }
    QString value;
    auto const it = values.find(key.first);
    if(it != values.end()) {
      value = it->second;
    }
    if(read_only_) {
      colman::fill_row_ro(*data_, widget_.item_grid_, row, key.first, value,
                          type.get_id(), key.second);
    }
    else {
      colman::fill_row_rw(
          *data_, widget_.item_grid_, row, key.first, value,
          type.get_id(), key.second,
          [this](colman::AppData&) { this->some_field_changed(); });
    }
  }
  widget_.thumbnail_->setProperty("thumbnail_id", item.get_thumbnail_id());
  if(item.get_thumbnail_id() > 0) {
    QByteArray png;
    if(data_->queries_->get_thumbnail(item.get_thumbnail_id(), png)) {
      QPixmap pm;
      pm.loadFromData(png);
      widget_.thumbnail_->setPixmap(pm);
      widget_.zoom_thumbnail_->setEnabled(true);
      widget_.remove_thumbnail_->setEnabled(true);
    }
    else {
      clear_thumbnail(widget_);
    }
  }
  else {
    clear_thumbnail(widget_);
  }
  widget_.browse_thumbnail_->setHidden(read_only_);
  widget_.scan_thumbnail_->setHidden(read_only_);
  widget_.amazon_thumbnail_->setHidden(read_only_);
  widget_.remove_thumbnail_->setHidden(read_only_);
}

/// Clears all editable fields in the widget to their empty/default state.
/**
  Resets the thumbnail and every editable widget in the grid layout
  (line edits, text edits, the publication-date editor, the signature combo
  boxes, spin boxes, list-value widgets, and table-data buttons) via
  clear_attribute_widget(), and restores the school-item checkbox to the
  collection's default value.
*/
void colman::ItemWidget::clear_fields()
{
  last_lookup_key_.clear();
  last_lookup_thumbnail_url_.clear();
  clear_thumbnail(widget_);
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* item = widget_.item_grid_->itemAtPosition(r, 1);
    if((item)) {
      clear_attribute_widget(item->widget());
    }
  }
  apply_default_signatures();
  if((data_)) {
    colman::Collection const col = data_->queries_->read_collection();
    widget_.is_schoolitem_->setChecked(col.get_default_is_school_item());
  }
}

namespace
{
  /// Performs a blocking internet ISBN lookup and fills the widget with the
  /// Runs fn() in a background thread while showing an indeterminate progress
  /// dialog, then returns the result.
  template<typename Fn>
  auto run_with_progress(QWidget* parent, Fn fn) -> decltype(fn())
  {
    using R = decltype(fn());
    QProgressDialog dlg(
        QApplication::tr("Searching..."), QString(), 0, 0, parent);
    dlg.setWindowModality(Qt::ApplicationModal);
    dlg.setMinimumDuration(0);
    dlg.setValue(0);
    auto future = QtConcurrent::run(std::move(fn));
    QFutureWatcher<R> watcher;
    QEventLoop loop;
    QObject::connect(
        &watcher, &QFutureWatcher<R>::finished, &loop, &QEventLoop::quit);
    watcher.setFuture(future);
    loop.exec();
    return future.result();
  }

  /// result.
  /**
    Reads current field values into a temporary Item, sends an asynchronous
    ISBN query via AppData::isbn_queries_, waits for the response in a local
    event loop, and merges the returned book data back into the widget.
    Optionally downloads and stores a thumbnail if the corresponding setting
    is enabled and no local thumbnail is found first.
    @param data   Application data holding the network query object and DB.
    @param widget The item widget to populate with the retrieved data.
    @param isbn   ISBN-13 string used for the lookup.
    @return True if the query was sent and data was received; false otherwise.
  */
  bool internet_isbn_lookup(colman::AppData& data, colman::ItemWidget& widget,
                            QString const& isbn)
  {
    bool retval = false;
    // Read only the type_id from the widget — the internet result replaces
    // everything else, so no full validation is needed here.
    int const type_id = widget.get_type_id();
    {
      QSettings s;
      data.isbn_queries_.set_googleapis_key(
          s.value("googleapis_key", "").toString());
      data.isbn_queries_.use_openlibrary(
          s.value("use_openlibrary", false).toBool());
      data.isbn_queries_.use_googleapis(
          s.value("use_googleapis", false).toBool());
      data.isbn_queries_.set_sru_servers(colman::load_sru_servers());
    }
    data.w_.setEnabled(false);
    bool const query_started = run_with_progress(
        &data.w_, [&]() { return data.isbn_queries_.query_isbn(isbn); });
    data.w_.setEnabled(true);
    if(query_started) {
      // Internet data is authoritative: use it as the new item, preserving
      // only the type_id from the widget (the API never supplies it).
      colman::Item item = data.isbn_queries_.get_book_data();
      if(item.get_name().isEmpty()) {
        QMessageBox::information(
            &data.w_,
            QApplication::tr("Collection Manager"),
            QApplication::tr(
                "The internet search found no results for this barcode."));
      }
      else {
        if(item.get_type_id() < 0) {
          item.set_type_id(type_id);
        }
        retval = true;
        QSettings settings;
        if(settings.value("download_thumbnails", false).toBool() &&
           (item.get_thumbnail_id() < 0 ||
            !data.queries_->have_local_thumbnail(item.get_thumbnail_id())))
        {
          // first we look for a thumbnail in the local directory:
          int tid = data.queries_->use_local_thumbnail(QString("isbn_") + isbn);
          if(tid < 0) {
            // no local file, now we perform some internet lookups:
            QString const thumbnail_url = data.isbn_queries_.get_thumbnail_url();
            if(!thumbnail_url.isEmpty()) {
              QPixmap const pm = data.isbn_queries_.get_thumbnail();
              if(pm.width() > 0) {
                QByteArray png;
                QBuffer buf(&png);
                buf.open(QIODevice::WriteOnly);
                pm.save(&buf, "PNG");
                tid = data.queries_->store_thumbnail(
                    png, QString("isbn_") + isbn, thumbnail_url, false);
              }
            }
          }
          if(tid >= 0) {
            item.set_thumbnail_id(tid);
          }
        }
        widget.lookup_result_to_fields(item);
      }
    }
    return retval;
  }

  /// Returns true if the opengtindb category name suggests a non-collection item.
  /** Known non-library categories contain keywords like "lebensmittel",
      "drogerie", "kosmetik", "reinigung", or "haushalt" (case-insensitive). */
  bool is_non_collection_category(QString const& category)
  {
    QString const lower = category.toLower();
    static QStringList const NonCollectionKeywords = {
        "lebensmittel", "getränk", "drogerie", "kosmetik",
        "reinigung",    "haushalt", "waschmittel", "kleidung",
        "textil",       "tiernahrung", "baumarkt"};
    for(auto const& kw : NonCollectionKeywords) {
      if(lower.contains(kw)) {
        return true;
      }
    }
    return false;
  }

  /// Performs a blocking internet EAN lookup and fills the widget with the
  /// result.
  /**
    Reads current field values into a temporary Item, sends parallel EAN
    queries via AppData::ean_queries_, waits for the response and merges the
    returned data back into the widget.  Optionally downloads and stores a
    thumbnail if the corresponding setting is enabled and no local thumbnail
    is found first.
    @param data   Application data holding the network query object and DB.
    @param widget The item widget to populate with the retrieved data.
    @param ean    EAN-13 string used for the lookup.
    @return True if the query was sent and data was received; false otherwise.
  */
  bool internet_ean_lookup(colman::AppData& data, colman::ItemWidget& widget,
                           QString const& ean)
  {
    int const type_id = widget.get_type_id();
    {
      QSettings s;
      data.ean_queries_.use_musicbrainz(
          s.value("use_musicbrainz", false).toBool());
      data.ean_queries_.set_sru_servers(colman::load_sru_servers());
    }
    // Read settings for the opengtindb fallback on the main thread before
    // handing off to the background worker.
    bool const use_opengtindb =
        QSettings().value("use_opengtindb", false).toBool();
    QString const opengtindb_userid =
        QSettings().value("opengtindb_userid", "").toString();

    data.w_.setEnabled(false);
    colman::Item item = run_with_progress(
        &data.w_, [&]() -> colman::Item {
          colman::Item result;
          if(data.ean_queries_.query_ean(ean)) {
            result = data.ean_queries_.get_item_data();
          }
          // SRU/MusicBrainz found nothing: try opengtindb as fallback.
          if(result.get_name().isEmpty() && use_opengtindb) {
            auto const opt =
                colman::query_opengtindb_ean(opengtindb_userid, ean);
            if(opt.has_value()) {
              result = *opt;
            }
          }
          return result;
        });
    data.w_.setEnabled(true);

    if(item.get_name().isEmpty()) {
      QMessageBox::information(
          &data.w_,
          QApplication::tr("Collection Manager"),
          QApplication::tr(
              "The internet search found no results for this barcode."));
      return false;
    }
    // Warn if the opengtindb category suggests a non-collection item.
    {
      auto const& vals = item.get_values();
      auto const cat_it = vals.find("opengtindb_category");
      if(cat_it != vals.end() &&
         is_non_collection_category(cat_it->second)) {
        QMessageBox::warning(
            &data.w_,
            QApplication::tr("Collection Manager"),
            QApplication::tr(
                "\"%1\" (category: %2) does not appear to belong "
                "in the library collection.")
                .arg(item.get_name(), cat_it->second));
      }
    }
    if(item.get_type_id() < 0) {
      item.set_type_id(type_id);
    }
    item.set_value("ean", ean);
    QSettings settings;
    if(settings.value("download_thumbnails", false).toBool() &&
       (item.get_thumbnail_id() < 0 ||
        !data.queries_->have_local_thumbnail(item.get_thumbnail_id())))
    {
      int tid = data.queries_->use_local_thumbnail(QString("ean_") + ean);
      if(tid < 0) {
        QString const thumbnail_url = data.ean_queries_.get_thumbnail_url();
        if(!thumbnail_url.isEmpty()) {
          QPixmap const pm = data.ean_queries_.get_thumbnail();
          if(pm.width() > 0) {
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            pm.save(&buf, "PNG");
            tid = data.queries_->store_thumbnail(
                png, QString("ean_") + ean, thumbnail_url, false);
          }
        }
      }
      if(tid >= 0) {
        item.set_thumbnail_id(tid);
      }
    }
    widget.lookup_result_to_fields(item);
    return true;
  }

/// Static configuration strings for a SearchSelectPage.
  struct SearchSelectConfig
  {
    QString title_;        ///< heading shown at the top of the page
    QString placeholder_;  ///< placeholder text of the query field
    QString no_results_;   ///< info when nothing was found (%1 = query)
    QString fetch_failed_; ///< warning when fetching the selected item fails
  };

/// One result row: a display label plus an opaque token for the fetch hook.
  struct SearchSelectRow
  {
    QString label_;
    int token_ = 0;
  };

/// Reusable page for a text search with a result list and detail fetch.
/**
   Replaces the former BggSearchDialog and NameSearchDialog, which shared the
   same structure.  The provider-specific behaviour is injected via two
   hooks: \a search runs the query and returns the result rows; \a fetch
   retrieves the full Item for a selected row's token.  The page is created
   per lookup and deletes itself when the user applies a result or goes back.
*/
  class SearchSelectPage : public QWidget
  {
  public:
    using SearchHook =
        std::function<std::vector<SearchSelectRow>(QString const&)>;
    using FetchHook = std::function<std::optional<colman::Item>(int)>;

    SearchSelectPage(SearchSelectConfig config,
                     SearchHook search,
                     FetchHook fetch,
                     QString const& initial_query,
                     QWidget* parent = nullptr)
        : QWidget(parent), config_(std::move(config)),
          search_(std::move(search)), fetch_(std::move(fetch))
    {
      auto* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

      auto* back_btn = new QPushButton(
          QApplication::translate("SearchSelectPage", "Back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer->addWidget(back_btn);

      auto* heading = new QLabel("<b>" + config_.title_ + "</b>", this);
      heading->setTextFormat(Qt::RichText);
      outer->addWidget(heading);

      auto* search_row = new QHBoxLayout;
      query_edit_ = new QLineEdit(initial_query, this);
      query_edit_->setPlaceholderText(config_.placeholder_);
      search_button_ = new QPushButton(
          QApplication::translate("SearchSelectPage", "Search"), this);
      search_row->addWidget(
          new QLabel(QApplication::translate("SearchSelectPage", "Title:"),
                     this));
      search_row->addWidget(query_edit_, 1);
      search_row->addWidget(search_button_);
      outer->addLayout(search_row);

      result_list_ = new QListWidget(this);
      result_list_->setAlternatingRowColors(true);
      outer->addWidget(result_list_, 1);

      button_box_ = new QDialogButtonBox(
          QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
          Qt::Horizontal, this);
      button_box_->button(QDialogButtonBox::Ok)->setEnabled(false);
      button_box_->button(QDialogButtonBox::Ok)
          ->setText(QApplication::translate("SearchSelectPage", "Apply"));
      outer->addWidget(button_box_);

      connect(search_button_, &QPushButton::clicked,
              this, [this]() { on_search(); });
      connect(query_edit_, &QLineEdit::returnPressed,
              this, [this]() { on_search(); });
      connect(result_list_, &QListWidget::itemSelectionChanged,
              this, [this]() { on_selection_changed(); });
      connect(result_list_, &QListWidget::itemDoubleClicked,
              this, [this]() { on_accept(); });
      connect(button_box_, &QDialogButtonBox::accepted,
              this, [this]() { on_accept(); });
      connect(button_box_, &QDialogButtonBox::rejected,
              this, [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
      connect(back_btn, &QPushButton::clicked,
              this, [this]() {
                if(on_back_) {
                  on_back_();
                }
              });

      if(!initial_query.isEmpty()) {
        // Auto-search once the page is shown for the first time.
        QTimer::singleShot(0, this, [this]() { on_search(); });
      }
    }

  /// Sets the callback invoked with the selected Item when the user applies.
    void set_on_result(std::function<void(colman::Item const&)> on_result)
    {
      on_result_ = std::move(on_result);
    }

  /// Sets the callback invoked when the user cancels or applies a result.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  private:
  /// Runs the search hook for the current query and fills the result list.
    void on_search()
    {
      QString const name = query_edit_->text().trimmed();
      if(name.isEmpty()) {
        return;
      }
      search_button_->setEnabled(false);
      query_edit_->setEnabled(false);

      result_list_->clear();
      button_box_->button(QDialogButtonBox::Ok)->setEnabled(false);
      for(auto const& row : search_(name)) {
        auto* item = new QListWidgetItem(row.label_, result_list_);
        item->setData(Qt::UserRole, row.token_);
      }

      query_edit_->setEnabled(true);
      search_button_->setEnabled(true);

      if(result_list_->count() == 0) {
        QMessageBox::information(
            this, QApplication::translate("SearchSelectPage", "No results"),
            config_.no_results_.arg(name));
      }
    }

  /// Enables the Apply button only while a result row is selected.
    void on_selection_changed()
    {
      button_box_->button(QDialogButtonBox::Ok)
          ->setEnabled(!result_list_->selectedItems().isEmpty());
    }

  /// Fetches the selected row's Item and reports it via the result callback.
    void on_accept()
    {
      QList<QListWidgetItem*> const selected = result_list_->selectedItems();
      if(selected.isEmpty()) {
        return;
      }
      int const token = selected.first()->data(Qt::UserRole).toInt();
      search_button_->setEnabled(false);
      query_edit_->setEnabled(false);
      result_list_->setEnabled(false);
      button_box_->setEnabled(false);

      auto const item = fetch_(token);
      if(!item) {
        search_button_->setEnabled(true);
        query_edit_->setEnabled(true);
        result_list_->setEnabled(true);
        button_box_->setEnabled(true);
        QMessageBox::warning(
            this,
            QApplication::translate("SearchSelectPage", "Lookup failed"),
            config_.fetch_failed_);
        return;
      }
      if(on_result_) {
        on_result_(*item);
      }
      if(on_back_) {
        on_back_();
      }
    }

    SearchSelectConfig config_;
    SearchHook search_;
    FetchHook fetch_;
    std::function<void(colman::Item const&)> on_result_;
    std::function<void()> on_back_;
    QLineEdit* query_edit_ = nullptr;
    QPushButton* search_button_ = nullptr;
    QListWidget* result_list_ = nullptr;
    QDialogButtonBox* button_box_ = nullptr;
  };

/// Shows a transient SearchSelectPage on the main window's page stack.
/**
   Captures the currently visible page as the return target.  When the user
   applies a result, \a on_result is invoked before returning; cancelling
   just returns.  The page deletes itself afterwards.  Does nothing while a
   SearchSelectPage is already visible, so that two lookups triggered by the
   same user action (e.g. editingFinished of a search field followed by a
   button click) do not stack two search pages on top of each other.
*/
  void run_search_select_page(
      colman::AppData& data,
      SearchSelectConfig config,
      SearchSelectPage::SearchHook search,
      SearchSelectPage::FetchHook fetch,
      QString const& initial_query,
      std::function<void(colman::Item const&)> on_result)
  {
    QWidget* const back = data.mw_.pages_stack_->currentWidget();
    if(dynamic_cast<SearchSelectPage*>(back)) {
      return;
    }
    auto* page = new SearchSelectPage(
        std::move(config), std::move(search), std::move(fetch),
        initial_query);
    data.mw_.pages_stack_->addWidget(page);
    auto go_back = [&data, page, back]() {
      data.mw_.pages_stack_->setCurrentWidget(back);
      data.mw_.pages_stack_->removeWidget(page);
      page->deleteLater();
    };
    page->set_on_result(std::move(on_result));
    page->set_on_back(go_back);
    data.mw_.pages_stack_->setCurrentWidget(page);
  }

} // namespace

/// Performs an internet lookup using the given ISBN, EAN, or UPC string.
/**
  Detects whether @p search is an ISBN-13, ISBN-10, EAN-13 product barcode,
  or UPC-A string.  ISBN-10 values are converted to ISBN-13 before the query
  is issued.  UPC-A values are converted to EAN-13 by prepending "0".
  Delegates the actual network request to the file-local helper functions.
  @param search The barcode string to look up.
  @return True if a lookup was performed and data was received; false if the
          string is not a recognised barcode or no AppData is associated.
*/
bool colman::ItemWidget::internet_lookup_string(QString const& search)
{
  if((data_)) {
    bool const is_board_game =
        data_->queries_->media_kind_for_type_id(get_type_id()) ==
        colman::MediaKind::BoardGame;
    if(colman::is_product_ean(search)) {
      last_lookup_key_ = search;
      if(is_board_game) {
        QSettings s;
        if(s.value("use_opengtindb", false).toBool()) {
          auto const opt = colman::query_opengtindb_ean(
              s.value("opengtindb_userid", "").toString(), search);
          if(opt.has_value()) {
            colman::Item item = *opt;
            item.set_type_id(get_type_id());
            item.set_value("ean", search);
            lookup_result_to_fields(item);
            return true;
          }
        }
        return false;
      }
      bool const found = internet_ean_lookup(*data_, *this, search);
      if(found) {
        last_lookup_thumbnail_url_ = data_->ean_queries_.get_thumbnail_url();
      }
      return found;
    }
    else if(colman::is_upc_a(search)) {
      QString const ean13 = QString("0") + search;
      last_lookup_key_ = ean13;
      if(is_board_game) {
        QSettings s;
        if(s.value("use_opengtindb", false).toBool()) {
          auto const opt = colman::query_opengtindb_ean(
              s.value("opengtindb_userid", "").toString(), ean13);
          if(opt.has_value()) {
            colman::Item item = *opt;
            item.set_type_id(get_type_id());
            item.set_value("ean", ean13);
            lookup_result_to_fields(item);
            return true;
          }
        }
        return false;
      }
      bool const found = internet_ean_lookup(*data_, *this, ean13);
      if(found) {
        last_lookup_thumbnail_url_ = data_->ean_queries_.get_thumbnail_url();
      }
      return found;
    }
    else if(colman::is_isbn13(search)) {
      last_lookup_key_ = search;
      bool const found = internet_isbn_lookup(*data_, *this, search);
      if(found) {
        last_lookup_thumbnail_url_ = data_->isbn_queries_.get_thumbnail_url();
      }
      return found;
    }
    else if(colman::is_isbn10(search)) {
      QString const isbn13 = isbn10_to_isbn13(search);
      last_lookup_key_ = isbn13;
      bool const found = internet_isbn_lookup(*data_, *this, isbn13);
      if(found) {
        last_lookup_thumbnail_url_ = data_->isbn_queries_.get_thumbnail_url();
      }
      return found;
    }
  }
  return false;
}

/// Opens the BGG search page with \a name as the initial search query.
/**
  Configures the BGG service from QSettings, then shows a SearchSelectPage
  pre-filled with \a name.  When the user applies a result the selected
  game's data is written into the widget; when \a ean is non-empty it is
  also stored as the item's EAN field.  The lookup is asynchronous: this
  returns immediately after navigating to the search page.
  @param name  Initial BGG search query.
  @param ean   Optional EAN-13 to store on the retrieved item.
*/
void colman::ItemWidget::internet_bgg_name_lookup(QString const& name,
                                                  QString const& ean)
{
  if(!data_) {
    return;
  }
  {
    QSettings s;
    if(!s.value("use_bgg", false).toBool()) {
      return;
    }
    data_->bgg_queries_.set_bgg_token(
        s.value("bgg_token", "").toString());
  }

  SearchSelectConfig config;
  config.title_ =
      QApplication::translate("SearchSelectPage", "BoardGameGeek Search");
  config.placeholder_ =
      QApplication::translate("SearchSelectPage", "Game title…");
  config.no_results_ = QApplication::translate(
      "SearchSelectPage", "No board games found for \"%1\".");
  config.fetch_failed_ = QApplication::translate(
      "SearchSelectPage",
      "Could not retrieve game details from BoardGameGeek.");

  auto search = [this](QString const& query) {
    std::vector<SearchSelectRow> rows;
    data_->bgg_queries_.query_name(query);
    for(auto const& r : data_->bgg_queries_.get_search_results()) {
      QString label = r.name_;
      if(r.year_ > 0) {
        label += QString(" (%1)").arg(r.year_);
      }
      rows.push_back({label, r.id_});
    }
    return rows;
  };
  auto fetch = [this](int token) {
    return data_->bgg_queries_.get_item(token);
  };
  auto on_result = [this, ean](colman::Item const& selected) {
    colman::Item item = selected;
    if(item.get_type_id() < 0) {
      item.set_type_id(get_type_id());
    }
    if(!ean.isEmpty()) {
      item.set_value("ean", ean);
    }
    lookup_result_to_fields(item);
  };

  run_search_select_page(*data_, std::move(config), search, fetch, name,
                         on_result);
}

/// Triggers an internet lookup appropriate for the current item type.
/**
  For board games: opens a BGG text search dialog pre-filled with the
  current name field content.  For all other types: scans the attribute
  grid for an "ISBN13" or "ean" field and triggers the corresponding
  barcode lookup; when neither is filled in, falls back to a multi-source
  title search with the current name field content.
  Has no effect if no AppData is associated.
  @param fallback_name Search text used for the BGG or title search when the
                       name field is empty.
*/
void colman::ItemWidget::internet_lookup(QString const& fallback_name)
{
  if(!data_) {
    return;
  }
  QString name = widget_.name_->text().trimmed();
  if(name.isEmpty()) {
    name = fallback_name;
  }
  if(data_->queries_->media_kind_for_type_id(get_type_id()) ==
     colman::MediaKind::BoardGame) {
    QString ean;
    int const grid_rows = widget_.item_grid_->rowCount();
    for(int r = 0; r < grid_rows; ++r) {
      auto* gi = widget_.item_grid_->itemAtPosition(r, 0);
      if(!gi) {
        continue;
      }
      auto* lbl = qobject_cast<QLabel*>(gi->widget());
      if(!lbl || lbl->property("db_column").toString() != "ean") {
        continue;
      }
      gi = widget_.item_grid_->itemAtPosition(r, 1);
      if(!gi) {
        break;
      }
      auto* edit = qobject_cast<QLineEdit*>(gi->widget());
      if(edit) {
        ean = edit->text();
      }
      break;
    }
    internet_bgg_name_lookup(name, ean);
    return;
  }
  // Collect the first valid ISBN-13 and the EAN field content from the grid.
  QString first_isbn;
  QString ean;
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* item = widget_.item_grid_->itemAtPosition(r, 0);
    if(!item) {
      continue;
    }
    QLabel* label = qobject_cast<QLabel*>(item->widget());
    if(!label) {
      continue;
    }
    QString const col = label->property("db_column").toString();
    if(col == "ISBN13" && first_isbn.isEmpty()) {
      item = widget_.item_grid_->itemAtPosition(r, 1);
      if(!item) {
        continue;
      }
      colman::ListValueWidget* lv =
          qobject_cast<colman::ListValueWidget*>(item->widget());
      if(lv) {
        for(QString const& isbn : lv->get_values()) {
          if(colman::is_isbn13(isbn)) {
            first_isbn = isbn;
            break;
          }
        }
      }
    }
    else if(col == "ean" && ean.isEmpty()) {
      item = widget_.item_grid_->itemAtPosition(r, 1);
      if(!item) {
        continue;
      }
      QLineEdit* edit = qobject_cast<QLineEdit*>(item->widget());
      if(edit) {
        ean = edit->text();
      }
    }
  }
  if(!first_isbn.isEmpty()) {
    internet_isbn_lookup(*data_, *this, first_isbn);
    return;
  }
  if(!ean.isEmpty()) {
    internet_lookup_string(ean);
    return;
  }
  internet_name_lookup(name);
}

/// Collects all widget field values into an Item object.
/**
  Reads the current values from every UI control — type, name, place,
  description, school-item flag, and all type-specific attribute
  rows — and stores them in @p item. Displays a warning dialog and returns
  false if any field contains input that fails its validator.
  @param item Output parameter that receives the collected field values.
  @return True if all fields are valid and @p item was fully populated;
          false if any field contains invalid input.
*/
bool colman::ItemWidget::fields_to_item(Item& item) const
{
  item.set_type_id(widget_.item_type_box_->currentData().toInt());
  item.set_name(widget_.name_->text());
  item.set_place(widget_.place_->text());
  item.set_signature_top(widget_.signature_top_->currentText());
  item.set_signature_bottom(widget_.signature_bottom_->currentText());
  item.set_description(widget_.description_->toPlainText());
  item.set_keywords(keywords_lv_->get_values());
  item.set_is_school_item(widget_.is_schoolitem_->isChecked());
  if(item.get_is_school_item()) {
    item.set_subject(widget_.subject_->text());
    item.set_academic_year(
        string_to_academic_years(widget_.academic_year_->text()));
    if(!academic_years_valid(item.get_academic_year())) {
      QMessageBox::warning(
          widget_.item_widget_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("The academic year range is invalid: the first "
                           "grade must not be greater than the second "
                           "(e.g. 5-7, 9)."));
      return false;
    }
    item.set_type(widget_.school_type_->text());
  }
  else {
    item.set_subject("");
    item.clear_academic_year();
    item.set_type("");
  }
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* item0 = widget_.item_grid_->itemAtPosition(r, 0);
    auto* item1 = widget_.item_grid_->itemAtPosition(r, 1);
    if((item0) && (item1)) {
      QWidget* widget0 = item0->widget();
      QWidget* widget1 = item1->widget();
      QLabel* label = qobject_cast<QLabel*>(widget0);
      if((label) && (widget1)) {
        QString const key = label->property("db_column").toString();
        if(!key.isEmpty()) {
          colman::ListValueWidget* lv =
              qobject_cast<colman::ListValueWidget*>(widget1);
          if(lv) {
            item.set_list_value(key, lv->get_values());
            continue;
          }
          colman::PartialDateEdit* const pde =
              qobject_cast<colman::PartialDateEdit*>(widget1);
          if(pde) {
            if(!pde->isValidInput()) {
              QMessageBox::warning(widget_.item_widget_,
                                   QApplication::tr("Collection Manager"),
                                   QApplication::tr("input for '%1' is invalid")
                                       .arg(label->text()));
              return false;
            }
          }
          else {
            QLineEdit* line = qobject_cast<QLineEdit*>(widget1);
            if(line && !line->text().isEmpty() && !line->hasAcceptableInput()) {
              QMessageBox::warning(widget_.item_widget_,
                                   QApplication::tr("Collection Manager"),
                                   QApplication::tr("input for '%1' is invalid")
                                       .arg(label->text()));
              return false;
            }
          }
          item.set_value(key, read_attribute_widget_value(widget1));
        }
      }
    }
  }
  item.set_thumbnail_id(widget_.thumbnail_->property("thumbnail_id").toInt());
  if(item_allow_reservations_combo_) {
    item.set_allow_reservations(
        item_allow_reservations_combo_->currentData().toInt());
  }
  else {
    item.set_allow_reservations(-1);
  }
  if(item_reservation_period_override_check_ && item_reservation_period_days_spin_) {
    item.set_reservation_period_days(
        item_reservation_period_override_check_->isChecked()
            ? item_reservation_period_days_spin_->value()
            : -1);
  }
  else {
    item.set_reservation_period_days(-1);
  }
  return true;
}

/// Copies the current field values from this widget into another ItemWidget.
/**
  Reads the fields of this widget into a temporary Item and then populates
  @p target using item_to_fields(). Does nothing if @p target is null or
  if fields_to_item() fails.
  @param target The destination widget to receive the copied data.
  @return True if the copy succeeded; false if @p target is null or the
          field collection failed.
*/
bool colman::ItemWidget::copy_to(ItemWidget* target) const
{
  if((target)) {
    Item item;
    if(fields_to_item(item)) {
      int const type_id = item.get_type_id();
      QString type_name = data_->queries_->get_type_name(type_id);
      target->item_to_fields(item, type_name);
      return true;
    }
  }
  return false;
}

/// Returns the item type ID currently selected in the type combo box.
/**
  @return the type_id stored in the current combo-box item's data role.
*/
int colman::ItemWidget::get_type_id() const
{
  return widget_.item_type_box_->currentData().toInt();
}

bool colman::ItemWidget::has_name() const
{
  return widget_.name_->text().size() > 0;
}

/// Checks whether the widget fields differ from the given Item's stored data.
/**
  Compares each UI field — name, place, description, school-item
  flag, thumbnail ID, and when the school-item box is checked also subject,
  academic year, and school type — against the corresponding values in
  @p item. Also compares all type-specific attribute rows.
  @param item The item to compare the current field values against.
  @return True if at least one field differs from @p item; false if they
          are identical.
*/
bool colman::ItemWidget::modified(Item const& item) const
{
  if(widget_.name_->text() != item.get_name() ||
     widget_.place_->text() != item.get_place() ||
     widget_.signature_top_->currentText() != item.get_signature_top() ||
     widget_.signature_bottom_->currentText() != item.get_signature_bottom() ||
     widget_.description_->toPlainText() != item.get_description() ||
     keywords_lv_->get_values() != item.get_keywords() ||
     widget_.is_schoolitem_->isChecked() != item.get_is_school_item() ||
     widget_.thumbnail_->property("thumbnail_id").toInt() !=
         item.get_thumbnail_id())
  {
    return true;
  }
  if(widget_.is_schoolitem_->isChecked()) {
    if(widget_.subject_->text() != item.get_subject() ||
       string_to_academic_years(widget_.academic_year_->text()) !=
           item.get_academic_year() ||
       widget_.school_type_->text() != item.get_type())
    {
      return true;
    }
  }
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* item0 = widget_.item_grid_->itemAtPosition(r, 0);
    auto* item1 = widget_.item_grid_->itemAtPosition(r, 1);
    if((item0) && (item1)) {
      QWidget* widget0 = item0->widget();
      QWidget* widget1 = item1->widget();
      QLabel* label = qobject_cast<QLabel*>(widget0);
      if((label) && (widget1)) {
        QString const key = label->property("db_column").toString();
        if(!key.isEmpty()) {
          colman::ListValueWidget* lv =
              qobject_cast<colman::ListValueWidget*>(widget1);
          if(lv) {
            auto const lit = item.get_list_values().find(key);
            QStringList const stored = (lit != item.get_list_values().end())
                                           ? lit->second
                                           : QStringList{};
            if(stored != lv->get_values()) {
              return true;
            }
            continue;
          }
          QString const value = read_attribute_widget_value(widget1);
          auto const it = item.get_values().find(key);
          if(it == item.get_values().end() || it->second != value) {
            return true;
          }
        }
      }
    }
  }
  if(item_allow_reservations_combo_) {
    if(item_allow_reservations_combo_->currentData().toInt() !=
       item.get_allow_reservations()) {
      return true;
    }
  }
  if(item_reservation_period_override_check_ && item_reservation_period_days_spin_) {
    int const stored = item.get_reservation_period_days();
    bool const override_checked = item_reservation_period_override_check_->isChecked();
    bool const item_has_override = (stored >= 0);
    if(override_checked != item_has_override) {
      return true;
    }
    if(override_checked && item_reservation_period_days_spin_->value() != stored) {
      return true;
    }
  }
  return false;
}

/// Sets the school-item checkbox to the given state.
/**
  @param b True to check the school-item checkbox; false to uncheck it.
*/
void colman::ItemWidget::set_is_schoolitem(bool b)
{
  widget_.is_schoolitem_->setChecked(b);
}

namespace
{
  /// Opens a file dialog to let the user pick a thumbnail image and stores it.
  /**
    Shows a PreviewFileDialog filtered to common image formats. If the user
    selects a file and the image loads successfully, the image is scaled to
    fit within 128×180 pixels (preserving aspect ratio) if it is too large.
    The user is asked whether to overwrite when a thumbnail for the same key
    already exists on disk. The thumbnail is then stored in the database with
    the key @p prefix + "_" + @p value.
    @param data   Application data holding the database connection.
    @param widget The UI widget used as parent for message boxes.
    @param prefix Key prefix identifying the thumbnail category (e.g. "isbn").
    @param value  Key suffix identifying the specific item (e.g. ISBN number).
    @return The database ID of the newly stored thumbnail, or -1 if the user
            cancelled or the image could not be loaded.
  */
  int local_thumbnail_search(colman::AppData& data, colman::ItemWidgetUi& widget,
                             QString const& prefix, QString const& value)
  {
    QSettings settings;
    QByteArray state = settings.value("open/thumbnail").toByteArray();
    colman::PreviewFileDialog diag(nullptr, QApplication::tr("open thumbnail"),
                                   "");
    QStringList filters;
    filters << QApplication::tr("image files (*.svg *.png *.xpm *.jpg *.jpeg)")
            << QApplication::tr("any files (*)");
    diag.setNameFilters(filters);
    diag.setFileMode(QFileDialog::ExistingFile);
    (void)diag.restoreState(state);
    if(diag.exec() == colman::PreviewFileDialog::Accepted) {
      QStringList file_names = diag.selectedFiles();
      QByteArray const state = diag.saveState();
      settings.setValue("open/thumbnail", state);
      if(!file_names.empty()) {
        QString const fname = file_names.front();
        QPixmap pm;
        if(pm.load(fname) && pm.width() > 0 && pm.height() > 0) {
          if(pm.width() > 180 || pm.height() > 220) {
            pm = pm.scaled(128, 180, Qt::KeepAspectRatio);
          }
          QString const dfname =
              colman::key_to_thumbnail_path(prefix + "_" + value,
                                            data.thumbnail_dir());
          if(!QFileInfo::exists(dfname) ||
             colman::question_yes_no(
                 widget.item_widget_, QApplication::tr("Collection Manager"),
                 prefix + " " + value + " " +
                     QApplication::tr(
                         "already has a thumbnail. Overwrite it?")) ==
                 QMessageBox::Yes)
          {
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            if(!pm.save(&buf, "PNG") || png.isEmpty()) {
              QMessageBox::warning(
                  widget.item_widget_,
                  QApplication::tr("Collection Manager"),
                  QApplication::tr(
                      "Could not encode the selected image."));
              return -1;
            }
            int const tid = data.queries_->store_thumbnail(
                png, prefix + "_" + value, "", true);
            if(tid < 0) {
              QMessageBox::warning(
                  widget.item_widget_,
                  QApplication::tr("Collection Manager"),
                  QApplication::tr(
                      "Could not store the thumbnail."));
            }
            return tid;
          }
        }
      }
    }
    return -1;
  }

  /// Converts an arbitrary text string to a filesystem-safe filename fragment.
  /**
    ASCII letters and digits are kept as-is; spaces and punctuation are
    replaced with underscores; all other characters (including non-ASCII)
    are replaced with '@'.
    @param text The input string to convert.
    @return A new string suitable for use as part of a filename.
  */
  QString text_to_fname(QString const& text)
  {
    QString retval;
    int const len = text.size();
    retval.reserve(len);
    for(int i = 0; i < len; ++i) {
      if(text[i].unicode() < 128 && text[i].isLetterOrNumber()) {
        retval.push_back(text[i]);
      }
      else if(text[i].isSpace() || text[i].isPunct()) {
        retval.push_back('_');
      }
      else {
        retval.push_back('@');
      }
    }
    return retval;
  }

} // namespace

/// Opens the multi-source title search page with \a name as the query.
/**
  Determines the media kind from the current item type, configures the
  title search services from QSettings, and shows a SearchSelectPage
  pre-filled with \a name.  When the user applies a result the selected
  record is written into the widget; when \a ean is non-empty it is also
  stored as the item's EAN field.  When the download_thumbnails setting is
  enabled, a cover image is stored under a name-based thumbnail key.  The
  lookup is asynchronous: this returns immediately after navigating to the
  search page.
  @param name  Initial search query for the title search.
  @param ean   Optional EAN-13 to store on the retrieved item.
*/
void colman::ItemWidget::internet_name_lookup(QString const& name,
                                              QString const& ean)
{
  if(!data_) {
    return;
  }
  colman::MediaKind const kind =
      data_->queries_->media_kind_for_type_id(get_type_id());
  if(!colman::name_search_available(kind)) {
    return;
  }
  {
    QSettings s;
    data_->name_queries_.use_openlibrary(
        s.value("use_openlibrary", false).toBool());
    data_->name_queries_.use_googleapis(
        s.value("use_googleapis", false).toBool());
    data_->name_queries_.set_googleapis_key(
        s.value("googleapis_key", "").toString());
    data_->name_queries_.use_musicbrainz(
        s.value("use_musicbrainz", false).toBool());
    data_->name_queries_.set_sru_servers(colman::load_sru_servers());
  }

  SearchSelectConfig config;
  config.title_ =
      QApplication::translate("SearchSelectPage", "Title Search");
  config.placeholder_ =
      QApplication::translate("SearchSelectPage", "Title…");
  config.no_results_ = QApplication::translate(
      "SearchSelectPage", "No results found for \"%1\".");
  config.fetch_failed_ = QApplication::translate(
      "SearchSelectPage",
      "Could not retrieve details from the selected source.");

  auto search = [this, kind](QString const& query) {
    std::vector<SearchSelectRow> rows;
    data_->name_queries_.query_name(query, kind);
    int index = 0;
    for(auto const& r : data_->name_queries_.get_search_results()) {
      QString label = r.name_;
      QString const author = name_result_author(r.item_);
      if(!author.isEmpty()) {
        label += QString(" — %1").arg(author);
      }
      if(r.year_ > 0) {
        label += QString(" (%1)").arg(r.year_);
      }
      if(!r.source_.isEmpty()) {
        label += QString(" — %1").arg(r.source_);
      }
      rows.push_back({label, index});
      ++index;
    }
    return rows;
  };
  auto fetch = [this](int token) {
    return data_->name_queries_.get_item(token);
  };
  auto on_result = [this, ean](colman::Item const& selected) {
    colman::Item item = selected;
    if(item.get_type_id() < 0) {
      item.set_type_id(get_type_id());
    }
    if(!ean.isEmpty()) {
      item.set_value("ean", ean);
    }
    QSettings settings;
    if(settings.value("download_thumbnails", false).toBool() &&
       (item.get_thumbnail_id() < 0 ||
        !data_->queries_->have_local_thumbnail(item.get_thumbnail_id())))
    {
      QString const key = QString("name_") + text_to_fname(item.get_name());
      // first we look for a thumbnail in the local directory:
      int tid = data_->queries_->use_local_thumbnail(key);
      if(tid < 0) {
        // no local file, use the cover URL of the selected search result:
        QString const thumbnail_url =
            data_->name_queries_.get_thumbnail_url();
        if(!thumbnail_url.isEmpty()) {
          QPixmap const pm = data_->name_queries_.get_thumbnail();
          if(pm.width() > 0) {
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            pm.save(&buf, "PNG");
            tid = data_->queries_->store_thumbnail(png, key, thumbnail_url,
                                                   false);
          }
        }
      }
      if(tid >= 0) {
        item.set_thumbnail_id(tid);
      }
    }
    lookup_result_to_fields(item);
    last_lookup_thumbnail_url_ = data_->name_queries_.get_thumbnail_url();
  };

  run_search_select_page(*data_, std::move(config), search, fetch, name,
                         on_result);
}

/// Loads a thumbnail from the database and displays it in the widget.
/**
  Retrieves the thumbnail identified by @p tid from the database and sets
  it as the pixmap on the thumbnail label. Also stores @p tid as the
  label's thumbnail_id property and emits field_changed(). Has no effect
  if @p tid is not positive or if no AppData is associated.
  @param tid Database ID of the thumbnail to load and display.
*/
void colman::ItemWidget::use_thumbnail(int tid)
{
  if(tid > 0 && (data_)) {
    QByteArray png;
    if(data_->queries_->get_thumbnail(tid, png)) {
      QPixmap pm;
      pm.loadFromData(png);
      widget_.thumbnail_->setPixmap(pm);
      widget_.thumbnail_->setProperty("thumbnail_id", tid);
      widget_.zoom_thumbnail_->setEnabled(true);
      widget_.remove_thumbnail_->setEnabled(true);
      some_field_changed();
    }
  }
}

/// Opens a file browser to select and associate a thumbnail with the item.
/**
  Prefers to key the thumbnail on the ISBN-13 field when one is present and
  valid. Falls back to keying on the item name when no ISBN is available.
  In both cases the user is offered any already-stored local thumbnail first;
  if they decline or none exists, a file-open dialog is shown. Has no effect
  if no AppData is associated.
*/
void colman::ItemWidget::browse_thumbnail()
{
  if(!data_) {
    return;
  }
  // first look whether ISBN13 field can be used:
  int const rows = widget_.item_grid_->rowCount();
  for(int r = 0; r < rows; ++r) {
    auto* item = widget_.item_grid_->itemAtPosition(r, 0);
    if((item)) {
      QLabel* label = qobject_cast<QLabel*>(item->widget());
      if(label && label->property("db_column").toString() == "ISBN13") {
        item = widget_.item_grid_->itemAtPosition(r, 1);
        if((item)) {
          colman::ListValueWidget* lv =
              qobject_cast<colman::ListValueWidget*>(item->widget());
          if(lv) {
            for(QString const& isbn : lv->get_values()) {
              if(colman::is_isbn13(isbn)) {
                int tid = data_->queries_->use_local_thumbnail(
                    QString("isbn_") + isbn);
                if(tid >= 0 &&
                   colman::question_yes_no(
                       widget_.item_widget_,
                       QApplication::tr("Collection Manager"),
                       QApplication::tr("found a thumbnail for the ISBN. "
                                        "Use it?")) == QMessageBox::Yes)
                {
                  use_thumbnail(tid);
                }
                else {
                  tid =
                      local_thumbnail_search(*data_, widget_, "isbn", isbn);
                  use_thumbnail(tid);
                }
                return;
              }
            }
          }
        }
      }
    }
  }
  // now try to work without ISBN:
  QString const text = widget_.name_->text();
  if(text.isEmpty()) {
    QMessageBox::warning(widget_.item_widget_,
                         QApplication::tr("Collection Manager"),
                         QApplication::tr("Need a name or an ISBN to store a "
                                          "thumbnail"));
  }
  else {
    int tid = data_->queries_->use_local_thumbnail(QString("name_") +
                                                   text_to_fname(text));
    if(tid >= 0 &&
       colman::question_yes_no(
           widget_.item_widget_, QApplication::tr("Collection Manager"),
           QApplication::tr("found a thumbnail for the item name. "
                            "Use it?")) == QMessageBox::Yes)
    {
      use_thumbnail(tid);
    }
    else {
      int const tid =
          local_thumbnail_search(*data_, widget_, "name", text_to_fname(text));
      use_thumbnail(tid);
    }
  }
}

/// Slot for the scan-thumbnail button; scanner support is not yet implemented.
/**
  Currently shows an informational message. Intended to acquire a thumbnail
  image from a connected scanner device in a future implementation.
*/
void colman::ItemWidget::scan_thumbnail()
{
  QMessageBox::information(
      this, QApplication::tr("Collection Manager"),
      QApplication::tr("Scanner support is not yet implemented."));
}

/// Fetches a cover image online and stores it as the item thumbnail.
/**
  If the item has no thumbnail yet, first tries to download the cover URL
  that an internet lookup found for the current item.  If the item already
  has a thumbnail, no URL is available or the download fails, opens an
  embedded Amazon.de browser where the user can click any product image to
  adopt it.

  When no internet lookup has been performed yet the search term falls back
  to the first valid ISBN-13 found in the form grid, or to the item name.
  Does nothing when no AppData is associated.
*/
void colman::ItemWidget::amazon_thumbnail()
{
  if(!data_) {
    return;
  }
  // Determine the search term: prefer the key from the last internet lookup.
  // Fall back to the first ISBN-13 in the grid, then the item name.
  QString search_key = last_lookup_key_;
  if(search_key.isEmpty()) {
    int const rows = widget_.item_grid_->rowCount();
    for(int r = 0; r < rows && search_key.isEmpty(); ++r) {
      auto* li = widget_.item_grid_->itemAtPosition(r, 0);
      if(li) {
        auto* lbl = qobject_cast<QLabel*>(li->widget());
        if(lbl && lbl->property("db_column").toString() == "ISBN13") {
          auto* vi = widget_.item_grid_->itemAtPosition(r, 1);
          if(vi) {
            auto* lv = qobject_cast<colman::ListValueWidget*>(vi->widget());
            if(lv) {
              for(QString const& isbn : lv->get_values()) {
                if(colman::is_isbn13(isbn)) {
                  search_key = isbn;
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  if(search_key.isEmpty()) {
    search_key = widget_.name_->text();
  }
  if(search_key.isEmpty()) {
    return;
  }
  // 1. Try the cover URL found by an internet lookup for this very item.
  // The query objects in AppData are shared and keep the URL of whatever
  // was looked up last, so they must not be consulted directly.  When the
  // item already shows a thumbnail, the user wants a different one: go
  // straight to the browser.
  QString const url =
      widget_.thumbnail_->property("thumbnail_id").toInt() > 0
          ? QString()
          : last_lookup_thumbnail_url_;
  if(!url.isEmpty()) {
    setCursor(Qt::WaitCursor);
    QByteArray const raw = colman::synchronous_get(url);
    unsetCursor();
    QPixmap pm;
    if(pm.loadFromData(raw) && pm.width() > 0) {
      QString const key =
          colman::is_isbn13(search_key)
              ? (QString("isbn_") + search_key)
              : (QString("ean_") + search_key);
      QByteArray png;
      QBuffer buf(&png);
      buf.open(QIODevice::WriteOnly);
      pm.scaled(128, 180, Qt::KeepAspectRatio).save(&buf, "PNG");
      int const tid = data_->queries_->store_thumbnail(png, key, url, true);
      use_thumbnail(tid);
      return;
    }
  }
  // 2. No cached URL or download failed: open the embedded Amazon browser
  // as a page on the main window's stack.
  colman::open_amazon_browser_page(*data_, search_key,
  [this, search_key](QPixmap const& pm)
  {
    QString const key =
        colman::is_isbn13(search_key)
            ? (QString("isbn_") + search_key)
            : (QString("ean_") + search_key);
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    pm.scaled(128, 180, Qt::KeepAspectRatio).save(&buf, "PNG");
    int const tid =
        data_->queries_->store_thumbnail(png, key, QString(), true);
    use_thumbnail(tid);
  });
}

/// Opens a dialog showing the thumbnail at an enlarged size.
/**
  Scales the stored pixmap to at most 80 % of the available screen area while
  preserving the aspect ratio.  Has no effect when no thumbnail is set.
*/
void colman::ItemWidget::zoom_thumbnail()
{
  auto const pm = widget_.thumbnail_->pixmap();
  if(pm.isNull()) {
    return;
  }
  auto* dlg = new QDialog(this);
  dlg->setWindowTitle(tr("Cover image"));
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  auto const screen =
      QGuiApplication::primaryScreen()->availableGeometry().size();
  auto const max_size =
      QSize(screen.width() * 4 / 5, screen.height() * 4 / 5);
  auto const scaled =
      pm.scaled(max_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  auto* label = new QLabel(dlg);
  label->setPixmap(scaled);
  label->setAlignment(Qt::AlignCenter);
  auto* layout = new QVBoxLayout(dlg);
  layout->addWidget(label);
  dlg->adjustSize();
  dlg->exec();
}

/// Removes the current thumbnail from the item form.
/**
  Clears the thumbnail label and its stored thumbnail id so that saving
  the item stores it without a thumbnail, and emits field_changed().
  The stored image itself remains in the database for other items.
  Has no effect when no thumbnail is set.
*/
void colman::ItemWidget::remove_thumbnail()
{
  if(widget_.thumbnail_->property("thumbnail_id").toInt() <= 0 &&
     widget_.thumbnail_->pixmap().isNull())
  {
    return;
  }
  clear_thumbnail(widget_);
  some_field_changed();
}

/* EOF */
