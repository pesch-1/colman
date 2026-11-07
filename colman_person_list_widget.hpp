// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PersonListWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERSON_LIST_WIDGET_HPP
#  define COLMAN_PERSON_LIST_WIDGET_HPP

#  ifndef SYSTEM_QITEMSELECTION
#    include <QItemSelection>
#    define SYSTEM_QITEMSELECTION
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSQLDATABASE
#    include <QSqlDatabase>
#    define SYSTEM_QSQLDATABASE
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif
#  ifndef SYSTEM_SET
#    include <set>
#    define SYSTEM_SET
#  endif

class QComboBox;
class QLabel;
class QLineEdit;
class QSortFilterProxyModel;
class QTableView;

namespace colman
{
  /// Qt widget displaying a searchable, selectable list of persons.
  /**
     Shows persons from a QAbstractTableModel in a sortable table view.
     Supports single and multi-selection, free-text search,
     and notifies listeners via selection_changed().
  */
  class PersonListWidget : public QWidget
  {
    Q_OBJECT

  Q_SIGNALS:
    /// Emitted when the selected person changes.
    /**
       @param id database ID of the newly selected person, or -1 if none
    */
    void selection_changed(int id);
    /// Emitted when the search text field content changes.
    void search_text_changed();

  public Q_SLOTS:
    /// Selects all rows in the person table.
    void select_all();
    /// Clears the current table selection.
    void clear_selection();

  public:
    /// Constructs the widget with the given parent.
    PersonListWidget(QWidget* parent = nullptr);
    /// Destructor.
    virtual ~PersonListWidget();

    /// Sets up the widget with the given model.
    /**
       Installs a sort proxy, connects search and selection signals.
       Call set_query_function() afterwards to enable data loading.
       @param model the table model supplying person rows
    */
    void setup(QAbstractTableModel& model);
    /// Sets the function that refreshes the model for a given search text.
    /**
       @param func callable taking a search string and updating the model
    */
    void set_query_function(std::function<void(QString const&)> func);
    /// Resets the person list to the unfiltered default.
    void reset_person_query();
    /// Re-executes the current query, preserving the selection.
    void refresh_person_query();
    /// Selects the person with the given database ID.
    /**
       @param id database ID of the person to select
    */
    void select_person(int id);
    /// Returns the database ID of the currently selected person.
    /**
       @return person ID, or -1 if no person is selected
    */
    int get_selected_person_id() const;
    /// Selects the only available person if exactly one is listed.
    /**
       @return the selected person ID, or -1 if not exactly one person
    */
    int select_if_single_person();
    /// Returns all selected person IDs.
    std::set<int> get_selected_person_ids() const;
    /// Filters the list to the given search text and refreshes.
    /**
       @param text the search string
    */
    void search(QString const& text);
    /// Clears the search field and refreshes the person list.
    void clear_search_field();
    /// Returns the current search field text.
    QString get_search_string() const;
    /// Moves keyboard focus to the search input field.
    void focus_search_field();
    /// Enables or disables multi-row selection.
    /**
       @param allow true to enable extended selection
    */
    void allow_multiple_selection(bool allow);
    /// Returns true if exactly one person is listed.
    bool single_person() const;
    /// Shows or hides the school class filter combo box.
    /**
       The filter is hidden by default. Hiding it also resets it to
       "all classes".
       @param enable true to show the class filter
    */
    void enable_class_filter(bool enable);
    /// Hides a model column permanently, also after the model is reloaded.
    /**
       @param column model column to hide
    */
    void hide_column(int column);
    /// Replaces the class names offered by the class filter.
    /**
       Keeps the current filter if its class is still offered; otherwise
       the filter falls back to "all classes".
       @param names the school class names
    */
    void set_class_names(QStringList const& names);

  private:
    void build_ui();
    void selected(QItemSelection const& sel,
                  QItemSelection const&);
    void apply_class_filter();
    void apply_hidden_columns();
    QLineEdit* search_ = nullptr;                        ///< search input field
    QTableView* table_ = nullptr;                        ///< person table view
    QLabel* class_label_ = nullptr;                      ///< label of the class filter
    QComboBox* class_combo_ = nullptr;                   ///< school class filter
    QSortFilterProxyModel* proxy_ = nullptr;             ///< sort and class filter proxy
    std::set<int> hidden_columns_ = {0};                 ///< columns kept hidden (id always)
    int last_id_ = -1;                                     ///< last selected person ID
    std::function<void(QString const&)> query_func_;     ///< refreshes model for search text
  };

} // namespace colman

#endif /* #ifndef COLMAN_PERSON_LIST_WIDGET_HPP */

/* EOF */
