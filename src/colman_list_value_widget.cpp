// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class ListValueWidget.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_list_value_widget.hpp"
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif


colman::ListValueWidget::ListValueWidget(QWidget* parent)
    : QWidget(parent),
      list_(new QListWidget(this)),
      input_(new QLineEdit(this)),
      add_(new QPushButton("+", this)),
      remove_(new QPushButton("-", this))
{
  auto* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->addWidget(list_);
  auto* hbox = new QHBoxLayout;
  hbox->addWidget(input_);
  hbox->addWidget(add_);
  hbox->addWidget(remove_);
  vbox->addLayout(hbox);

  add_->setToolTip(QApplication::tr("Add value"));
  remove_->setToolTip(QApplication::tr("Remove value"));

  QObject::connect(add_, &QPushButton::clicked, this, [this]() {
    QString const text = input_->text().trimmed();
    if(!text.isEmpty()) {
      list_->addItem(text);
      input_->clear();
      Q_EMIT value_changed();
    }
  });
  QObject::connect(remove_, &QPushButton::clicked, this, [this]() {
    auto* item = list_->currentItem();
    if(item) {
      delete item;
      Q_EMIT value_changed();
    }
  });
  QObject::connect(input_, &QLineEdit::returnPressed, add_, &QPushButton::click);
}


/**
   Replaces the displayed list with \a values.
   @param values new list of values.
*/
void colman::ListValueWidget::set_values(QStringList const& values)
{
  list_->clear();
  for(auto const& v : values) {
    list_->addItem(v);
  }
}


/**
   Returns the current list of values in display order.
   @return list of value strings.
*/
QStringList colman::ListValueWidget::get_values() const
{
  QStringList result;
  int const n = list_->count();
  result.reserve(n);
  for(int i = 0; i < n; ++i) {
    result.append(list_->item(i)->text());
  }
  return result;
}


/**
   Switches between read-only and editable mode.
   @param ro true to hide the input and +/- buttons.
*/
void colman::ListValueWidget::set_read_only(bool ro)
{
  input_->setVisible(!ro);
  add_->setVisible(!ro);
  remove_->setVisible(!ro);
}


/* EOF */
