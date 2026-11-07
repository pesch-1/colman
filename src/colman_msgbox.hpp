// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Helpers for QMessageBox dialogs with translated button labels.
/**
   @file
   @author Christoph Pesch
*/
#ifndef COLMAN_MSGBOX_HPP
#  define COLMAN_MSGBOX_HPP

#  ifndef SYSTEM_QMESSAGEBOX
#    include <QMessageBox>
#    define SYSTEM_QMESSAGEBOX
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{

/// Shows a Yes/No question dialog with translated button labels.
/// @return QMessageBox::Yes or QMessageBox::No.
  int question_yes_no(QWidget* parent,
                      QString const& title,
                      QString const& text,
                      int default_btn = QMessageBox::No);

/// Shows a Yes/No information dialog with translated button labels.
/// @return QMessageBox::Yes or QMessageBox::No.
  int information_yes_no(QWidget* parent,
                         QString const& title,
                         QString const& text,
                         int default_btn = QMessageBox::No);

/// Shows a Yes/No warning dialog with translated button labels.
/// @return QMessageBox::Yes or QMessageBox::No.
  int warning_yes_no(QWidget* parent,
                     QString const& title,
                     QString const& text,
                     int default_btn = QMessageBox::No);

/// Shows a Yes/No/Cancel warning dialog with translated button labels.
/// @return QMessageBox::Yes, QMessageBox::No, or QMessageBox::Cancel.
  int warning_yes_no_cancel(QWidget* parent,
                            QString const& title,
                            QString const& text,
                            int default_btn = QMessageBox::Cancel);

} // namespace colman

#endif /* #ifndef COLMAN_MSGBOX_HPP */

/* EOF */
