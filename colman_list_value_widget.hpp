// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class ListValueWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_LIST_VALUE_WIDGET_HPP
#  define COLMAN_LIST_VALUE_WIDGET_HPP

#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif

class QListWidget;
class QLineEdit;
class QPushButton;

namespace colman
{
  /// Inline list editor with add and remove buttons.
  /**
     Displays a QListWidget with a QLineEdit and +/- buttons to add or remove
     entries.  Used in the item attribute grid for fields declared as
     repeatable.  Emits value_changed() after every user modification.
  */
  class ListValueWidget : public QWidget
  {
    Q_OBJECT
  Q_SIGNALS:
    /// Emitted after any user modification to the value list.
    void value_changed();

  public:
    explicit ListValueWidget(QWidget* parent = nullptr);
    /// Replaces the displayed list with \a values.
    void set_values(QStringList const& values);
    /// Returns the current list of values.
    QStringList get_values() const;
    /// Switches between read-only (no editing widgets) and editable mode.
    void set_read_only(bool ro);

  private:
    QListWidget* list_;    ///<  the value list
    QLineEdit* input_;   ///<  text input for the next value to add
    QPushButton* add_;     ///<  appends input_ text to list_
    QPushButton* remove_;  ///<  removes the selected list_ entry
  };

} // namespace colman

#endif /* #ifndef COLMAN_LIST_VALUE_WIDGET_HPP */

/* EOF */
