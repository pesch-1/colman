// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PersonListWidget.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person_list_widget.hpp"
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_TABLE_MODELS_HPP
#  include "colman_table_models.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  define SYSTEM_QREGULAREXPRESSION
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

/**
   Constructs the PersonListWidget instance.
*/
colman::PersonListWidget::PersonListWidget(QWidget* parent)
    : QWidget(parent),
      last_id_(-1)
{
  build_ui();
}


/// Builds the search row and person table view.
void colman::PersonListWidget::build_ui()
{
  setWindowTitle(tr("Person list"));

  auto* outer_layout = new QVBoxLayout(this);

  auto* search_row = new QHBoxLayout;
  auto* label = new QLabel(tr("name/barcode:"), this);
  search_ = new QLineEdit(this);
  label->setBuddy(search_);
  search_row->addWidget(label);
  search_row->addWidget(search_);
  // Optional school class filter, hidden until enable_class_filter().
  class_label_ = new QLabel(tr("Class:"), this);
  class_combo_ = new QComboBox(this);
  class_combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  class_combo_->addItem(tr("all classes"), QVariant());
  class_label_->setBuddy(class_combo_);
  class_label_->setVisible(false);
  class_combo_->setVisible(false);
  search_row->addWidget(class_label_);
  search_row->addWidget(class_combo_);
  outer_layout->addLayout(search_row);

  table_ = new QTableView(this);
  table_->setAlternatingRowColors(true);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  outer_layout->addWidget(table_);
}


/**
   Implements PersonListWidget.
*/
colman::PersonListWidget::~PersonListWidget()
{
}


/// Enables or disables multi-row selection in the table view.
/**
  @param allow true to enable extended (multi-row) selection,
               false to restrict to single-row selection
*/
void colman::PersonListWidget::allow_multiple_selection(bool allow)
{
  table_->setSelectionMode(allow ? QAbstractItemView::ExtendedSelection
                                 : QAbstractItemView::SingleSelection);
}


/// Sets the function that refreshes the model for a given search text.
/**
  Replaces the current refresh function with @p func.  The function
  receives the current search text and updates the model directly.

  @param func callable taking a search string and updating the model
*/
void colman::PersonListWidget::
    set_query_function(std::function<void(QString const&)> func)
{
  query_func_ = func;
}


/// Re-executes the current query, preserving the existing selection.
/**
  Runs the installed query function with the current search text and
  restores the previously selected person.
  Emits selection_changed() if the selection changes as a result.
*/
void colman::PersonListWidget::refresh_person_query()
{
  if((query_func_)) {
    colman::AutoBlockSignals b1(table_);
    colman::AutoBlockSignals b2(table_->selectionModel());
    int person_id = colman::selected_id(table_);
    query_func_(search_->text());
    apply_hidden_columns();
    if(person_id < 0) {
      person_id = select_if_single_row(table_);
    }
    if(colman::select_and_scroll_to(table_, 0, person_id)) {
      if(person_id != last_id_) {
        last_id_ = person_id;
        Q_EMIT selection_changed(person_id);
      }
    }
    else if(-1 != last_id_) {
      last_id_ = -1;
      Q_EMIT selection_changed(-1);
    }
  }
}


/// Sets up the widget with the given model.
/**
  Configures the table view with a sort proxy, connects the selection
  and search signals, and installs the clear-selection event filter.
  Call set_query_function() and reset_person_query() to populate data.

  @param model the table model supplying person rows
*/
void colman::PersonListWidget::setup(QAbstractTableModel& model)
{
  proxy_ = new QSortFilterProxyModel(table_);
  proxy_->setSourceModel(&model);
  // The class filter matches the school class column of PersonListModel.
  proxy_->setFilterKeyColumn(PersonListModel::ClassColumn);
  table_->setModel(proxy_);
  table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  table_->setSortingEnabled(true);
  table_->viewport()->installEventFilter(new ClearSelectionFilter(table_));
  apply_hidden_columns();
  connect(table_->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this, &PersonListWidget::selected);
  connect(class_combo_,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int) {
            apply_class_filter();
            refresh_person_query();
          });
  connect(search_, &QLineEdit::editingFinished,
          [this]() {
            if(search_->isModified()) {
              search_->setModified(false);
              refresh_person_query();
              Q_EMIT search_text_changed();
            }
          });
}


/// Moves keyboard focus to the search input field.
/**
  Gives focus to the search line edit so the user can immediately
  start typing a search term.
*/
void colman::PersonListWidget::focus_search_field()
{
  search_->setFocus();
}


/// Resets the person list to the unfiltered default.
/**
  Clears any active search filter, re-executes the query with an empty
  search string, and sorts by the name column.
*/
void colman::PersonListWidget::reset_person_query()
{
  if((query_func_)) {
    query_func_("");
    apply_hidden_columns();
    table_->sortByColumn(1, Qt::AscendingOrder);
  }
}


/// Handles a table selection change and emits selection_changed().
/**
  Extracts the person ID from the first index of the new selection.
  Emits selection_changed() with the person ID, or with -1 when the
  selection becomes empty.

  @param sel the newly selected item ranges (deselected range is unused)
*/
void colman::PersonListWidget::selected(QItemSelection const& sel,
                                        QItemSelection const&)
{
  auto const index_list = sel.indexes();
  if(!index_list.isEmpty()) {
    QModelIndex const& model_index = index_list.front();
    QAbstractItemModel const* model = model_index.model();
    if((model)) {
      int const row = model_index.row();
      QVariant const
          id = model->data(model->index(row, 0), Qt::DisplayRole);
      int const person_id = id.toInt();
      if(person_id != last_id_) {
        last_id_ = person_id;
        Q_EMIT selection_changed(person_id);
      }
      return;
    }
  }
  if(-1 != last_id_) {
    last_id_ = -1;
    Q_EMIT selection_changed(-1);
  }
}


/// Selects the person with the given database ID.
/**
  Scrolls the table to the person identified by @p id and selects
  that row.  Passing -1 clears the selection.  Emits
  selection_changed() only when the selection actually changes.

  @param id database ID of the person to select, or -1 to deselect
*/
void colman::PersonListWidget::
    select_person(int id)
{
  if(id != last_id_) {
    last_id_ = id;
    if(id >= 0) {
      colman::select_and_scroll_to(table_, 0, id);
    }
    else {
      table_->clearSelection();
    }
    Q_EMIT selection_changed(id);
  }
}


/// Returns the database ID of the currently selected person.
/**
  @return the person ID of the selected row, or -1 if no row is selected
*/
int colman::PersonListWidget::get_selected_person_id() const
{
  return last_id_;
}


/// Returns the database IDs of all currently selected persons.
/**
  @return set of person IDs for every selected row; empty if nothing
          is selected
*/
std::set<int> colman::PersonListWidget::get_selected_person_ids() const
{
  return selected_ids(table_);
}


/// Selects the only available person if exactly one row is shown.
/**
  Convenience wrapper around select_if_single_row().  Has no effect
  when the table contains zero or more than one row.

  @return the selected person ID, or -1 if the row count is not exactly one
*/
int colman::PersonListWidget::select_if_single_person()
{
  return select_if_single_row(table_);
}


/// Returns true if exactly one person row is currently shown.
/**
  @return true when the table contains exactly one row, false otherwise
*/
bool colman::PersonListWidget::single_person() const
{
  return single_row(table_);
}


/// Clears the search field and refreshes the person list.
/**
  Empties the search line edit and calls refresh_person_query() so
  that the full, unfiltered list of persons is shown again.
*/
void colman::PersonListWidget::clear_search_field()
{
  search_->clear();
  refresh_person_query();
}


/// Filters the list to the given search text and refreshes the view.
/**
  Sets the search field to @p text (if it differs from the current
  content) and calls refresh_person_query() to apply the filter.

  @param text  the search string to apply
*/
void colman::PersonListWidget::search(QString const& text)
{
  if(text != search_->text()) {
    search_->setText(text);
  }
  refresh_person_query();
}


/// Returns the current text of the search input field.
/**
  @return the search string currently entered in the search line edit
*/
QString colman::PersonListWidget::get_search_string() const
{
  return search_->text();
}


/// Selects all rows in the person table.
/**
  Forwards to QAbstractItemView::selectAll() on the internal table.
*/
void colman::PersonListWidget::select_all()
{
  table_->selectAll();
}


/// Clears the current table selection.
/**
  Forwards to QAbstractItemView::clearSelection() on the internal table.
*/
void colman::PersonListWidget::clear_selection()
{
  table_->clearSelection();
}


/// Shows or hides the school class filter combo box.
/**
  Hiding the filter resets it to "all classes" so that no invisible
  restriction remains active.

  @param enable true to show the class filter, false to hide it
*/
void colman::PersonListWidget::enable_class_filter(bool enable)
{
  class_label_->setVisible(enable);
  class_combo_->setVisible(enable);
  if(!enable) {
    class_combo_->setCurrentIndex(0);
  }
}


/// Replaces the class names offered by the class filter.
/**
  The combo box offers "all classes", "no class" and then every
  non-empty name of @p names.  The current filter is kept when its
  class is still offered; otherwise it falls back to "all classes",
  which re-filters the list.

  @param names the school class names
*/
void colman::PersonListWidget::set_class_names(QStringList const& names)
{
  auto const current = class_combo_->currentData();
  {
    QSignalBlocker blk(class_combo_);
    class_combo_->clear();
    class_combo_->addItem(tr("all classes"), QVariant());
    class_combo_->addItem(tr("no class"), QString(""));
    for(auto const& name : names) {
      if(!name.isEmpty()) {
        class_combo_->addItem(name, name);
      }
    }
    int index = 0;
    if(current.isValid()) {
      index = std::max(0, class_combo_->findData(current));
    }
    class_combo_->setCurrentIndex(index);
  }
  if(class_combo_->currentData() != current) {
    apply_class_filter();
    refresh_person_query();
  }
}


/// Hides a model column permanently, also after the model is reloaded.
/**
  A model reset makes the header forget hidden sections, so the column
  is remembered and hidden again after every query.

  @param column model column to hide
*/
void colman::PersonListWidget::hide_column(int column)
{
  hidden_columns_.insert(column);
  apply_hidden_columns();
}


/// Hides all remembered columns in the table view.
void colman::PersonListWidget::apply_hidden_columns()
{
  for(int const column : hidden_columns_) {
    table_->setColumnHidden(column, true);
  }
}


/// Restricts the proxy to persons of the selected school class.
/**
  "all classes" (invalid item data) removes the restriction; any
  other entry, including "no class" (empty string), matches the
  school class column exactly.
*/
void colman::PersonListWidget::apply_class_filter()
{
  if((proxy_)) {
    auto const cls = class_combo_->currentData();
    if(cls.isValid()) {
      proxy_->setFilterRegularExpression(QRegularExpression(
          "^" + QRegularExpression::escape(cls.toString()) + "$"));
    }
    else {
      proxy_->setFilterRegularExpression(QRegularExpression());
    }
  }
}


/* EOF */
