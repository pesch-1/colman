// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PersonCheckListWidget.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person_check_list_widget.hpp"
#ifndef COLMAN_PERSON_HPP
#  include "colman_person.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
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
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QTREEWIDGET
#  include <QTreeWidget>
#  define SYSTEM_QTREEWIDGET
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{
  int constexpr LastnameColumn = 0;  ///< also holds the person id
  int constexpr FirstnameColumn = 1; ///<
  int constexpr ClassColumn = 2;     ///<
  int constexpr NoteColumn = 3;      ///<
  int constexpr IdRole = Qt::UserRole; ///< person id
} // namespace


/**
   Constructs the PersonCheckListWidget instance.
   @param parent parent widget
*/
colman::PersonCheckListWidget::PersonCheckListWidget(QWidget* parent)
    : QWidget(parent)
{
  build_ui();
}


/// Builds the search row, buttons and tree.
void colman::PersonCheckListWidget::build_ui()
{
  auto* outer_layout = new QVBoxLayout(this);
  outer_layout->setContentsMargins(0, 0, 0, 0);
  auto* search_row = new QHBoxLayout;
  auto* search_label = new QLabel(tr("name/barcode:"), this);
  search_ = new QLineEdit(this);
  search_label->setBuddy(search_);
  search_row->addWidget(search_label);
  search_row->addWidget(search_);
  auto* class_label = new QLabel(tr("Class:"), this);
  class_combo_ = new QComboBox(this);
  class_combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  class_label->setBuddy(class_combo_);
  search_row->addWidget(class_label);
  search_row->addWidget(class_combo_);
  select_all_ = new QPushButton(tr("Select all"), this);
  search_row->addWidget(select_all_);
  deselect_all_ = new QPushButton(tr("Deselect all"), this);
  search_row->addWidget(deselect_all_);
  outer_layout->addLayout(search_row);

  tree_ = new QTreeWidget(this);
  tree_->setAlternatingRowColors(true);
  tree_->setRootIsDecorated(false);
  tree_->setColumnCount(4);
  tree_->setHeaderLabels(
      {tr("Last name"), tr("First name"), tr("Class"), tr("Note")});
  tree_->setSortingEnabled(true);
  tree_->sortByColumn(LastnameColumn, Qt::AscendingOrder);
  outer_layout->addWidget(tree_);

  connect(search_, &QLineEdit::textChanged, this,
          [this](QString const&) { apply_filter(); });
  connect(search_, &QLineEdit::returnPressed, this,
          [this]() { Q_EMIT search_return_pressed(search_->text()); });
  connect(class_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int) { apply_filter(); });
  connect(select_all_, &QPushButton::clicked, this,
          [this]() { select_all_visible(); });
  connect(deselect_all_, &QPushButton::clicked, this,
          [this]() { deselect_all_visible(); });
  connect(tree_, &QTreeWidget::currentItemChanged, this,
          [this](QTreeWidgetItem* current, QTreeWidgetItem*) {
            Q_EMIT current_changed(
                current ? current->data(LastnameColumn, IdRole).toInt() : -1);
          });
  connect(tree_, &QTreeWidget::itemChanged, this,
          [this](QTreeWidgetItem* item, int column) {
            on_item_changed(item, column);
          });
}


/// Replaces the listed persons; all check states are cleared.
/**
   Keeps the class filter selection if the class is still offered.
   @param persons     persons to list
   @param class_names class names offered by the class filter
*/
void colman::PersonCheckListWidget::set_persons(
    std::vector<Person> const& persons, QStringList const& class_names)
{
  {
    colman::AutoBlockSignals b(class_combo_);
    auto const current = class_combo_->currentData();
    class_combo_->clear();
    class_combo_->addItem(tr("all classes"), QVariant());
    class_combo_->addItem(tr("no class"), QString(""));
    for(auto const& name : class_names) {
      class_combo_->addItem(name, name);
    }
    auto index = 0;
    if(current.isValid()) {
      index = std::max(0, class_combo_->findData(current));
    }
    class_combo_->setCurrentIndex(index);
  }
  updating_ = true;
  tree_->setSortingEnabled(false);
  tree_->clear();
  disabled_ids_.clear();
  for(auto const& person : persons) {
    auto* ti = new QTreeWidgetItem;
    ti->setFlags(ti->flags() | Qt::ItemIsUserCheckable);
    ti->setCheckState(LastnameColumn, Qt::Unchecked);
    ti->setText(LastnameColumn, person.get_lastname());
    ti->setText(FirstnameColumn, person.get_firstname());
    ti->setText(ClassColumn, person.get_school_class());
    ti->setData(LastnameColumn, IdRole, person.get_id());
    tree_->addTopLevelItem(ti);
  }
  tree_->setSortingEnabled(true);
  tree_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  updating_ = false;
  apply_filter();
  Q_EMIT checked_changed(0);
}


/// Marks persons that can not be checked (shown grayed out).
/**
   Re-enables all other rows and unchecks the disabled ones.
   @param ids ids of the persons to disable
*/
void colman::PersonCheckListWidget::set_disabled_ids(QSet<int> const& ids)
{
  disabled_ids_ = ids;
  updating_ = true;
  bool changed = false;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    bool const disabled =
        ids.contains(ti->data(LastnameColumn, IdRole).toInt());
    if(disabled && ti->checkState(LastnameColumn) == Qt::Checked) {
      ti->setCheckState(LastnameColumn, Qt::Unchecked);
      changed = true;
    }
    auto flags = ti->flags();
    if(disabled) {
      flags &= ~Qt::ItemIsEnabled;
    }
    else {
      flags |= Qt::ItemIsEnabled;
    }
    ti->setFlags(flags);
  }
  updating_ = false;
  if(changed) {
    Q_EMIT checked_changed(checked_count());
  }
}


/// Sets a note shown in the last column for some persons.
/**
   Rows without an entry in \a notes get an empty note.
   @param notes person id → note text
*/
void colman::PersonCheckListWidget::set_notes(
    QHash<int, QString> const& notes)
{
  updating_ = true;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    ti->setText(NoteColumn,
                notes.value(ti->data(LastnameColumn, IdRole).toInt()));
  }
  updating_ = false;
}


/// Sets the function consulted before a person gets checked.
/**
   @param guard returns false to veto checking the given person id
*/
void colman::PersonCheckListWidget::set_check_guard(
    std::function<bool(int)> guard)
{
  guard_ = std::move(guard);
}


/// Returns the ids of all checked persons (also hidden ones).
/**
   @return person ids in display order
*/
QList<int> colman::PersonCheckListWidget::checked_ids() const
{
  QList<int> result;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto const* ti = tree_->topLevelItem(i);
    if(ti->checkState(LastnameColumn) == Qt::Checked) {
      result.append(ti->data(LastnameColumn, IdRole).toInt());
    }
  }
  return result;
}


/// Returns the number of checked persons.
int colman::PersonCheckListWidget::checked_count() const
{
  auto count = 0;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    if(tree_->topLevelItem(i)->checkState(LastnameColumn) == Qt::Checked) {
      ++count;
    }
  }
  return count;
}


/// Returns the number of listed persons (also hidden ones).
int colman::PersonCheckListWidget::person_count() const
{
  return tree_->topLevelItemCount();
}


/// Checks exactly the listed persons in \a ids (guard not consulted).
/**
   Disabled persons stay unchecked.
   @param ids ids of the persons to check; all others are unchecked
*/
void colman::PersonCheckListWidget::set_checked_ids(QSet<int> const& ids)
{
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    int const id = ti->data(LastnameColumn, IdRole).toInt();
    set_checked_silently(ti, ids.contains(id) && !disabled_ids_.contains(id));
  }
  Q_EMIT checked_changed(checked_count());
}


/// Returns the checked persons in display order.
/**
   Only id, last name, first name and school class are set.
   @return the checked persons
*/
std::vector<colman::Person>
colman::PersonCheckListWidget::checked_persons() const
{
  std::vector<Person> result;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto const* ti = tree_->topLevelItem(i);
    if(ti->checkState(LastnameColumn) != Qt::Checked) {
      continue;
    }
    Person p;
    p.set_id(ti->data(LastnameColumn, IdRole).toInt());
    p.set_lastname(ti->text(LastnameColumn));
    p.set_firstname(ti->text(FirstnameColumn));
    p.set_school_class(ti->text(ClassColumn));
    result.push_back(std::move(p));
  }
  return result;
}


/// Checks the person with \a person_id and scrolls to it.
/**
   The row is made visible if the current filter hides it.
   @param person_id the person to check
   @return outcome of the request
*/
colman::PersonCheckListWidget::CheckResult
colman::PersonCheckListWidget::check_person(int person_id)
{
  auto* ti = find_row(person_id);
  if(ti == nullptr) {
    return CheckResult::Unknown;
  }
  ti->setHidden(false);
  tree_->scrollToItem(ti);
  tree_->setCurrentItem(ti);
  if(disabled_ids_.contains(person_id)) {
    return CheckResult::Disabled;
  }
  if(ti->checkState(LastnameColumn) == Qt::Checked) {
    return CheckResult::AlreadyChecked;
  }
  if(guard_ && !guard_(person_id)) {
    return CheckResult::Refused;
  }
  set_checked_silently(ti, true);
  Q_EMIT checked_changed(checked_count());
  return CheckResult::Checked;
}


/// Checks every visible, enabled row the guard allows.
/**
   Rows are processed in display order, so with a limited number of
   copies the topmost persons are checked first.
   @return number of visible rows that were refused by the guard
*/
int colman::PersonCheckListWidget::select_all_visible()
{
  auto refused = 0;
  auto changed = false;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    int const id = ti->data(LastnameColumn, IdRole).toInt();
    if(ti->isHidden() || disabled_ids_.contains(id) ||
       ti->checkState(LastnameColumn) == Qt::Checked)
    {
      continue;
    }
    if(guard_ && !guard_(id)) {
      ++refused;
      continue;
    }
    set_checked_silently(ti, true);
    changed = true;
  }
  if(changed) {
    Q_EMIT checked_changed(checked_count());
  }
  if(refused > 0) {
    Q_EMIT check_refused(-1);
  }
  return refused;
}


/// Unchecks every visible row.
void colman::PersonCheckListWidget::deselect_all_visible()
{
  auto changed = false;
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    if(!ti->isHidden() && ti->checkState(LastnameColumn) == Qt::Checked) {
      set_checked_silently(ti, false);
      changed = true;
    }
  }
  if(changed) {
    Q_EMIT checked_changed(checked_count());
  }
}


/// Restricts the list to the school class \a name.
/**
   @param name the school class to show
   @return false if the class filter does not offer \a name
*/
bool colman::PersonCheckListWidget::set_class_filter(QString const& name)
{
  int const index = class_combo_->findData(name);
  if(index < 0) {
    return false;
  }
  class_combo_->setCurrentIndex(index);
  return true;
}


/// Clears the search field.
void colman::PersonCheckListWidget::clear_search_field()
{
  search_->clear();
}


/// Moves the keyboard focus to the search field.
void colman::PersonCheckListWidget::focus_search_field()
{
  search_->setFocus();
}


/// Shows/hides rows according to the search text and class filter.
/**
   The search text matches a prefix of the last or first name.
*/
void colman::PersonCheckListWidget::apply_filter()
{
  auto const s = search_->text().trimmed();
  auto const cls = class_combo_->currentData();
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    bool const name_match =
        s.isEmpty() ||
        ti->text(LastnameColumn).startsWith(s, Qt::CaseInsensitive) ||
        ti->text(FirstnameColumn).startsWith(s, Qt::CaseInsensitive);
    bool const class_match =
        !cls.isValid() || ti->text(ClassColumn) == cls.toString();
    ti->setHidden(!(name_match && class_match));
  }
}


/// Handles a check state change made by the user.
/**
   Asks the guard before accepting a new check; a vetoed check is
   reverted and reported via check_refused().
   @param item   the changed row
   @param column the changed column
*/
void colman::PersonCheckListWidget::on_item_changed(QTreeWidgetItem* item,
                                                    int column)
{
  if(updating_ || column != LastnameColumn) {
    return;
  }
  int const id = item->data(LastnameColumn, IdRole).toInt();
  if(item->checkState(LastnameColumn) == Qt::Checked) {
    // The guard must judge the selection without this person.
    set_checked_silently(item, false);
    if(guard_ && !guard_(id)) {
      Q_EMIT check_refused(id);
      return;
    }
    set_checked_silently(item, true);
  }
  Q_EMIT checked_changed(checked_count());
}


/// Returns the tree row of \a person_id, or nullptr.
QTreeWidgetItem* colman::PersonCheckListWidget::find_row(
    int person_id) const
{
  for(int i = 0; i < tree_->topLevelItemCount(); ++i) {
    auto* ti = tree_->topLevelItem(i);
    if(ti->data(LastnameColumn, IdRole).toInt() == person_id) {
      return ti;
    }
  }
  return nullptr;
}


/// Sets the check state of \a item without re-entering the guard.
void colman::PersonCheckListWidget::set_checked_silently(
    QTreeWidgetItem* item, bool checked)
{
  updating_ = true;
  item->setCheckState(LastnameColumn, checked ? Qt::Checked : Qt::Unchecked);
  updating_ = false;
}


/* EOF */
