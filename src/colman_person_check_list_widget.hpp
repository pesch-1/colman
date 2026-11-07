// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PersonCheckListWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERSON_CHECK_LIST_WIDGET_HPP
#  define COLMAN_PERSON_CHECK_LIST_WIDGET_HPP

#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QHASH
#    include <QHash>
#    define SYSTEM_QHASH
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QSET
#    include <QSet>
#    define SYSTEM_QSET
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif

class QComboBox;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace colman
{
  class Person;

  /// Checkable list of persons with name search and class filter.
  /**
     Each person row carries a check box.  The list can be narrowed by a
     name search and a school class filter; "Select all" and
     "Deselect all" act on the visible rows only.  Hidden rows keep their
     check state, so persons of several classes can be combined.

     A guard function may veto checking a person (e.g. when no more copies
     are left); vetoed checks are reverted and reported via
     check_refused().
  */
  class PersonCheckListWidget : public QWidget
  {
    Q_OBJECT

  Q_SIGNALS:
    /// Emitted whenever the set of checked persons changes.
    /** @param count number of checked persons */
    void checked_changed(int count);
    /// Emitted when the guard vetoed checking \a person_id.
    void check_refused(int person_id);
    /// Emitted when the current (highlighted) row changes.
    /** @param person_id id of the current person, or -1 for none */
    void current_changed(int person_id);
    /// Emitted when Return is pressed in the search field.
    /** @param text the current search text */
    void search_return_pressed(QString const& text);

  public:
    /// Outcome of check_person().
    enum struct CheckResult {
      Checked,        ///< the person is now checked
      AlreadyChecked, ///< the person was checked before
      Unknown,        ///< no such person in the list
      Disabled,       ///< the person can not be checked
      Refused         ///< the guard vetoed the check
    };

    /// Constructs the widget with the given parent.
    explicit PersonCheckListWidget(QWidget* parent = nullptr);

    /// Replaces the listed persons; all check states are cleared.
    /**
       @param persons     persons to list
       @param class_names class names offered by the class filter
    */
    void set_persons(std::vector<Person> const& persons,
                     QStringList const& class_names);
    /// Marks persons that can not be checked (shown grayed out).
    /** Unchecks them if necessary. */
    void set_disabled_ids(QSet<int> const& ids);
    /// Sets a note shown in the last column for some persons.
    void set_notes(QHash<int, QString> const& notes);
    /// Sets the function consulted before a person gets checked.
    /**
       @param guard returns false to veto checking the given person id;
                    an empty function allows every check
    */
    void set_check_guard(std::function<bool(int)> guard);
    /// Returns the ids of all checked persons (also hidden ones).
    /** In display order. */
    QList<int> checked_ids() const;
    /// Returns the number of checked persons.
    int checked_count() const;
    /// Returns the number of listed persons (also hidden ones).
    int person_count() const;
    /// Checks exactly the listed persons in \a ids (guard not consulted).
    void set_checked_ids(QSet<int> const& ids);
    /// Returns the checked persons in display order.
    /** Only id, last name, first name and school class are set. */
    std::vector<Person> checked_persons() const;
    /// Checks the person with \a person_id and scrolls to it.
    CheckResult check_person(int person_id);
    /// Checks every visible, enabled row the guard allows.
    /** @return number of visible rows that were refused by the guard */
    int select_all_visible();
    /// Unchecks every visible row.
    void deselect_all_visible();
    /// Restricts the list to the school class \a name.
    /** @return false if the class filter does not offer \a name */
    bool set_class_filter(QString const& name);
    /// Clears the search field.
    void clear_search_field();
    /// Moves the keyboard focus to the search field.
    void focus_search_field();

  private:
    /// Builds the search row, buttons and tree.
    void build_ui();
    /// Shows/hides rows according to the search text and class filter.
    void apply_filter();
    /// Handles a check state change made by the user.
    void on_item_changed(QTreeWidgetItem* item, int column);
    /// Returns the tree row of \a person_id, or nullptr.
    QTreeWidgetItem* find_row(int person_id) const;
    /// Sets the check state of \a item without re-entering the guard.
    void set_checked_silently(QTreeWidgetItem* item, bool checked);

    QLineEdit* search_ = nullptr;          ///< name/barcode search
    QComboBox* class_combo_ = nullptr;     ///< class filter
    QPushButton* select_all_ = nullptr;    ///< checks all visible rows
    QPushButton* deselect_all_ = nullptr;  ///< unchecks all visible rows
    QTreeWidget* tree_ = nullptr;          ///< the person rows
    std::function<bool(int)> guard_;       ///< veto for checking
    QSet<int> disabled_ids_;               ///< persons that can't be checked
    bool updating_ = false;                ///< suppresses itemChanged handling
  };

} // namespace colman

#endif /* #ifndef COLMAN_PERSON_CHECK_LIST_WIDGET_HPP */
/* EOF */
