// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of colman msgbox helpers.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_msgbox.hpp"
#ifndef SYSTEM_QABSTRACTBUTTON
#  include <QAbstractButton>
#  define SYSTEM_QABSTRACTBUTTON
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif

namespace
{

/// Returns the question-owl icon scaled for use in a QMessageBox.
  QPixmap msgbox_icon()
  {
    return QPixmap(":/icons/owls/question.jpeg")
        .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }

/// Builds and executes a Yes/No message box with translated button labels.
  int exec_yes_no(QWidget* parent,
                  QString const& title,
                  QString const& text,
                  int default_btn)
  {
    QMessageBox mb(QMessageBox::NoIcon, title, text,
                   QMessageBox::Yes | QMessageBox::No, parent);
    mb.setIconPixmap(msgbox_icon());
    if(auto* b = mb.button(QMessageBox::Yes)) {
      b->setText(QApplication::tr("Yes"));
    }
    if(auto* b = mb.button(QMessageBox::No)) {
      b->setText(QApplication::tr("No"));
    }
    mb.setDefaultButton(
        static_cast<QMessageBox::StandardButton>(default_btn));
    return mb.exec();
  }

} // namespace


int colman::question_yes_no(QWidget* parent,
                            QString const& title,
                            QString const& text,
                            int default_btn)
{
  return exec_yes_no(parent, title, text, default_btn);
}


int colman::information_yes_no(QWidget* parent,
                               QString const& title,
                               QString const& text,
                               int default_btn)
{
  return exec_yes_no(parent, title, text, default_btn);
}


int colman::warning_yes_no(QWidget* parent,
                           QString const& title,
                           QString const& text,
                           int default_btn)
{
  return exec_yes_no(parent, title, text, default_btn);
}


int colman::warning_yes_no_cancel(QWidget* parent,
                                  QString const& title,
                                  QString const& text,
                                  int default_btn)
{
  QMessageBox mb(QMessageBox::NoIcon, title, text,
                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                 parent);
  mb.setIconPixmap(msgbox_icon());
  if(auto* b = mb.button(QMessageBox::Yes)) {
    b->setText(QApplication::tr("Yes"));
  }
  if(auto* b = mb.button(QMessageBox::No)) {
    b->setText(QApplication::tr("No"));
  }
  if(auto* b = mb.button(QMessageBox::Cancel)) {
    b->setText(QApplication::tr("Cancel"));
  }
  mb.setDefaultButton(
      static_cast<QMessageBox::StandardButton>(default_btn));
  return mb.exec();
}

/* EOF */
