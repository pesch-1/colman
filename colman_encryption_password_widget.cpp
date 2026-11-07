// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of password_strength() and EncryptionPasswordWidget.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_encryption_password_widget.hpp"
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QSTYLE
#  include <QStyle>
#  define SYSTEM_QSTYLE
#endif

int colman::password_strength(QString const& pw, QString& label,
                              QString& color)
{
  int score = 0;
  if(pw.length() >= 8) {
    ++score;
  }
  for(QChar c : pw) {
    if(c.isUpper()) {
      ++score;
      break;
    }
  }
  for(QChar c : pw) {
    if(c.isLower()) {
      ++score;
      break;
    }
  }
  for(QChar c : pw) {
    if(c.isDigit()) {
      ++score;
      break;
    }
  }
  for(QChar c : pw) {
    if(!c.isLetterOrNumber()) {
      ++score;
      break;
    }
  }
  switch(score) {
  case 0:
  case 1:
    label = QApplication::tr("Very weak");
    color = "red";
    break;
  case 2:
    label = QApplication::tr("Weak");
    color = "orange";
    break;
  case 3:
    label = QApplication::tr("Moderate");
    color = "goldenrod";
    break;
  case 4:
    label = QApplication::tr("Strong");
    color = "green";
    break;
  default:
    label = QApplication::tr("Very strong");
    color = "darkgreen";
    break;
  }
  return score;
}

colman::EncryptionPasswordWidget::EncryptionPasswordWidget(QWidget* parent)
    : QWidget(parent)
{
  QFormLayout* form = new QFormLayout(this);
  form->setContentsMargins(0, 0, 0, 0);

  pw_edit_ = new QLineEdit(this);
  pw_edit_->setObjectName("encryption_password_");
  pw_edit_->setEchoMode(QLineEdit::Password);
  confirm_edit_ = new QLineEdit(this);
  confirm_edit_->setObjectName("encryption_confirm_");
  confirm_edit_->setEchoMode(QLineEdit::Password);
  {
    QChar const mask(pw_edit_->style()->styleHint(
        QStyle::SH_LineEdit_PasswordCharacter));
    int const pw_w =
        pw_edit_->fontMetrics().horizontalAdvance(QString(20, mask)) + 8;
    pw_edit_->setMinimumWidth(pw_w);
    confirm_edit_->setMinimumWidth(pw_w);
  }
  form->addRow(QApplication::tr("New password:"), pw_edit_);
  form->addRow(QApplication::tr("Confirm:"), confirm_edit_);

  strength_label_ = new QLabel(this);
  form->addRow(QApplication::tr("Password strength:"), strength_label_);

  connect(pw_edit_, &QLineEdit::textChanged,
          [this]() {
            update_strength();
            Q_EMIT changed();
          });
  connect(confirm_edit_, &QLineEdit::textChanged,
          [this]() { Q_EMIT changed(); });
  connect(pw_edit_, &QLineEdit::returnPressed,
          [this]() { confirm_edit_->setFocus(); });
}

QString colman::EncryptionPasswordWidget::password() const
{
  return pw_edit_->text();
}

bool colman::EncryptionPasswordWidget::is_valid() const
{
  return !pw_edit_->text().isEmpty() &&
         pw_edit_->text() == confirm_edit_->text();
}

void colman::EncryptionPasswordWidget::clear()
{
  pw_edit_->clear();
  confirm_edit_->clear();
  strength_label_->clear();
}

void colman::EncryptionPasswordWidget::update_strength()
{
  if(pw_edit_->text().isEmpty()) {
    strength_label_->clear();
    return;
  }
  QString lbl, col;
  password_strength(pw_edit_->text(), lbl, col);
  strength_label_->setText(
      QString("<span style='color:%1'>%2</span>").arg(col, lbl));
}

/* EOF */
