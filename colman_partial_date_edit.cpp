// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PartialDateEdit.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_partial_date_edit.hpp"
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QCALENDARWIDGET
#  include <QCalendarWidget>
#  define SYSTEM_QCALENDARWIDGET
#endif
#ifndef SYSTEM_QWHEELEVENT
#  include <QWheelEvent>
#  define SYSTEM_QWHEELEVENT
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
namespace
{
  /// Splits a partial date string into its numeric components.
  /**
     Sets day, month, year to zero for absent parts.
     Supports "", "yyyy", "m.yyyy", and "d.m.yyyy".
     @param s      input string.
     @param day    receives the day value, or 0 if absent.
     @param month  receives the month value, or 0 if absent.
     @param year   receives the year value, or 0 if absent.
  */
  void parse_date(QString const& s, int& day, int& month, int& year)
  {
    day = 0;
    month = 0;
    year = 0;
    QStringList const parts = s.split('.');
    if(parts.size() == 3) {
      day = parts[0].toInt();
      month = parts[1].toInt();
      year = parts[2].toInt();
    }
    else if(parts.size() == 2) {
      month = parts[0].toInt();
      year = parts[1].toInt();
    }
    else if(parts.size() == 1 && !parts[0].isEmpty()) {
      year = parts[0].toInt();
    }
  }

} // namespace

/**
   Constructs the widget: sets a placeholder text and connects text
   changes to the dateChanged() signal.
   @param parent optional parent widget.
*/
colman::PartialDateEdit::PartialDateEdit(QWidget* parent)
    : QAbstractSpinBox(parent)
{
  lineEdit()->setPlaceholderText(tr("D.M.YYYY"));
  connect(lineEdit(), &QLineEdit::textEdited,
          this, &colman::PartialDateEdit::dateChanged);
}

/**
   Sets the field text to @p s.  Accepted formats: "", "yyyy",
   "m.yyyy", "d.m.yyyy".
   @param s date string to display.
*/
void colman::PartialDateEdit::setDateString(QString const& s)
{
  lineEdit()->setText(s);
}

/**
   Reads the field text and returns a normalized partial date string.
   Returns "" if the field is empty or contains only zeros.
*/
QString colman::PartialDateEdit::dateString() const
{
  int d = 0, m = 0, y = 0;
  parse_date(lineEdit()->text(), d, m, y);
  if(d > 0 && m > 0 && y > 0) {
    return QString::number(d) + "." + QString::number(m) + "." + QString::number(y);
  }
  if(m > 0 && y > 0) {
    return QString::number(m) + "." + QString::number(y);
  }
  if(y > 0) {
    return QString::number(y);
  }
  return QString();
}

/**
   Returns true if the current field value is a valid partial date:
   - empty is valid
   - year alone is valid
   - month + year is valid when month is in 1–12
   - day + month + year is valid when QDate(y, m, d) is valid
   Day without month or month without year is invalid.
*/
bool colman::PartialDateEdit::isValidInput() const
{
  int d = 0, m = 0, y = 0;
  parse_date(lineEdit()->text(), d, m, y);
  if(d > 0 && m == 0) {
    return false;
  }
  if(m > 0 && y == 0) {
    return false;
  }
  if(d > 0 && m > 0 && y > 0 && !QDate(y, m, d).isValid()) {
    return false;
  }
  return true;
}

/**
   Accepts strings consisting of digits and at most two dots.
   Any such string is returned as Intermediate so that the user can
   continue typing; semantic validation happens in isValidInput().
   @param text current text (may be modified by fixup on focus-out).
   @param pos  cursor position (unused).
*/
QValidator::State colman::PartialDateEdit::validate(QString& text,
                                                    int& pos) const
{
  Q_UNUSED(pos);
  if(text.isEmpty()) {
    return QValidator::Acceptable;
  }
  int dots = 0;
  for(QChar const c : text) {
    if(c == '.') {
      ++dots;
    }
    else if(!c.isDigit()) {
      return QValidator::Invalid;
    }
  }
  if(dots > 2) {
    return QValidator::Invalid;
  }
  return QValidator::Intermediate;
}

/**
   Opens a calendar dialog.  The currently entered date (if complete
   and valid) is pre-selected; otherwise today is shown.  On
   acceptance the full d.m.yyyy date is written to the field and
   dateChanged() is emitted.
   @param steps ignored (both spin directions open the same dialog).
*/
void colman::PartialDateEdit::stepBy(int steps)
{
  Q_UNUSED(steps);
  int d = 0, m = 0, y = 0;
  parse_date(lineEdit()->text(), d, m, y);
  QDate date;
  if(d > 0 && m > 0 && y > 0) {
    date = QDate(y, m, d);
  }
  else if(m > 0 && y > 0) {
    date = QDate(y, m, 1);
  }
  else if(y > 0) {
    date = QDate(y, 1, 1);
  }
  if(!date.isValid()) {
    date = QDate::currentDate();
  }
  QDialog dlg(this);
  QVBoxLayout* layout = new QVBoxLayout(&dlg);
  QCalendarWidget* cal = new QCalendarWidget;
  cal->setSelectedDate(date);
  layout->addWidget(cal);
  QDialogButtonBox* box = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
  connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
  layout->addWidget(box);
  if(dlg.exec() == QDialog::Accepted) {
    QDate const sel = cal->selectedDate();
    setDateString(QString::number(sel.day()) + "." + QString::number(sel.month()) + "." + QString::number(sel.year()));
    Q_EMIT dateChanged();
  }
}

/**
   Keeps both spin buttons permanently enabled so the calendar can
   always be opened.
*/
QAbstractSpinBox::StepEnabled colman::PartialDateEdit::stepEnabled() const
{
  return StepUpEnabled | StepDownEnabled;
}

/**
   Ignores the wheel event so that scrolling the surrounding field list does
   not invoke stepBy() and pop up the calendar.  Ignoring lets the event
   propagate to the enclosing scroll area, which scrolls the list instead.
*/
void colman::PartialDateEdit::wheelEvent(QWheelEvent* event)
{
  event->ignore();
}

/* EOF */
