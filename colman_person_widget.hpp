// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PersonWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERSON_WIDGET_HPP
#  define COLMAN_PERSON_WIDGET_HPP

#  ifndef COLMAN_PERSON_HPP
#    include "colman_person.hpp"
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif

class QComboBox;
class QLineEdit;
class QPlainTextEdit;

namespace colman
{
  /// Qt widget for editing and displaying person contact data.
  /**
     Provides form fields for all Person properties: name, title,
     contact details, and remarks.
     Emits field_changed() whenever the user modifies any field.
  */
  class PersonWidget : public QWidget
  {
    Q_OBJECT

  Q_SIGNALS:
    /// Emitted when any input field is modified.
    void field_changed();

  public Q_SLOTS:
    /// Clears all input fields.
    void clear_fields();

  public:
    /// Constructs the widget with the given parent.
    PersonWidget(QWidget* parent = nullptr);
    /// Destructor.
    virtual ~PersonWidget() = default;

    /// Populates all form fields from \a person.
    /**
       @param person the person whose data is to be displayed
    */
    void person_to_fields(Person const& person);
    /// Reads all form fields into \a person.
    /**
       @param person output person receiving the field values
    */
    void fields_to_person(Person& person) const;

    /// Returns true if any field value differs from \a person.
    /**
       @param person the person to compare against
    */
    bool modified(Person const& person) const;
    /// Returns true if both last name and first name are non-empty.
    bool name_given() const;
    /// Sets all input fields to read-only or editable.
    /**
       @param ro true to make fields read-only
    */
    void set_read_only(bool ro);
    /// Populates the school-class combo-box drop-down with \a names.
    /** Preserves the current text in the editable line-edit.
        @param names sorted list of current class names */
    void set_class_names(QStringList const& names);
    /// Returns the current last name field value.
    QString get_lastname() const;
    /// Returns the current first name field value.
    QString get_firstname() const;

  private:
    void some_field_changed();
    void build_ui();

    QLineEdit* lastname_ = nullptr;
    QLineEdit* firstname_ = nullptr;
    QLineEdit* title_ = nullptr;
    QComboBox* school_class_ = nullptr;
    QLineEdit* emails_ = nullptr;
    QLineEdit* telephone_ = nullptr;
    QLineEdit* street_ = nullptr;
    QLineEdit* zip_ = nullptr;
    QLineEdit* city_ = nullptr;
    QLineEdit* country_ = nullptr;
    QPlainTextEdit* remarks_ = nullptr;
  };

} // namespace colman

#endif /* #ifndef COLMAN_PERSON_WIDGET_HPP */

/* EOF */
