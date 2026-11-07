// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PartialDateEdit.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PARTIAL_DATE_EDIT_HPP
#  define COLMAN_PARTIAL_DATE_EDIT_HPP

#  ifndef SYSTEM_QABSTRACTSPINBOX
#    include <QAbstractSpinBox>
#    define SYSTEM_QABSTRACTSPINBOX
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

class QWheelEvent;

namespace colman
{

  /// Input widget for an optional partial date (day, month, year).
  /**
     Behaves and looks like QDateEdit but allows incomplete dates:
     year only, month and year, or full day/month/year.  The spin
     buttons open a calendar dialog to pick a full date.
     \c setReadOnly() is inherited from QAbstractSpinBox.

     The stored value is a string in one of the formats:
     - ""               empty
     - "yyyy"           year only
     - "m.yyyy"         month and year
     - "d.m.yyyy"       full date

     Emits dateChanged() whenever the text is modified.
  */
  class PartialDateEdit : public QAbstractSpinBox
  {
    Q_OBJECT

  Q_SIGNALS:
    /// Emitted when the date field text is modified.
    void dateChanged();

  public:
    /// Constructs the widget with the given @p parent.
    explicit PartialDateEdit(QWidget* parent = nullptr);

    /// Populates the field from a partial date string.
    /**
       Accepts "", "yyyy", "m.yyyy", or "d.m.yyyy".
       @param s date string to parse and display.
    */
    void setDateString(QString const& s);

    /// Returns the current value as a partial date string.
    /**
       Returns "" when the field is empty, "yyyy" when only the year
       is set, "m.yyyy" when month and year are set, and "d.m.yyyy"
       when all three fields are set.
    */
    QString dateString() const;

    /// Returns true if the current input is a valid partial date.
    /**
       Invalid combinations: day set without month, month set without
       year, or a full date that does not exist in the calendar.
    */
    bool isValidInput() const;

  protected:
    /// Accepts strings containing only digits and at most two dots.
    QValidator::State validate(QString& text, int& pos) const override;
    /// Opens a calendar dialog; the chosen date fills all three parts.
    void stepBy(int steps) override;
    /// Keeps both spin buttons enabled at all times.
    StepEnabled stepEnabled() const override;
    /// Ignores mouse-wheel events so scrolling the surrounding list never
    /// opens the calendar; the event bubbles up to the scroll area instead.
    /// The calendar is opened only by an explicit click on a step button.
    void wheelEvent(QWheelEvent* event) override;
  };

} // namespace colman

#endif /* #ifndef COLMAN_PARTIAL_DATE_EDIT_HPP */

/* EOF */
