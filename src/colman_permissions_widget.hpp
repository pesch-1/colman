// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Widget for viewing and editing per-user permissions.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERMISSIONS_WIDGET_HPP
#  define COLMAN_PERMISSIONS_WIDGET_HPP

#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSET
#    include <QSet>
#    define SYSTEM_QSET
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif

class QCheckBox;
class QFormLayout;
class QLabel;
class QListWidget;
class QPushButton;

namespace colman
{

  /// Widget for viewing and editing per-user permissions.
  /**
   * Displays a list of persons on the left and the selected person's
   * permission flags on the right.  When a single person_id is given
   * at construction the person list is hidden and only that person's
   * permissions are shown.  The widget can be set to read-only mode
   * to prevent editing.
   *
   * Permissions whose column name appears in \a non_grantable_permissions
   * are shown greyed out (disabled) and are never written to the database,
   * because the logged-in user is not allowed to grant them.
   */
  class PermissionsWidget : public QWidget
  {
    Q_OBJECT

  public:
    /// Constructs the widget.
    /**
     * @param queries                  database queries
     * @param read_only                if true the checkboxes and buttons are
     *                                 disabled
     * @param person_id                if >= 0, show only this person's
     *                                 permissions and hide the person list
     * @param non_grantable_permissions column names of permissions the
     *                                 logged-in user may not grant; shown
     *                                 greyed out and never written
     * @param parent                   optional parent widget
     */
    PermissionsWidget(IQueries& queries,
                      bool read_only = false,
                      int person_id = -1,
                      QSet<QString> const& non_grantable_permissions = {},
                      bool embedded = false,
                      QWidget* parent = nullptr);

    /// Saves the current person's permissions then loads for the new person.
    /**
     * Greys out the checkbox rows listed in \a non_grantable.
     * Pass person_id <= 0 to clear the display.
     * @param person_id      id of the person to show, or <= 0 to clear
     * @param non_grantable  column names of permissions the user may not grant
     */
    void load_for_person(int person_id, QSet<QString> const& non_grantable);

    /// Builds the permission checkboxes if they were not available earlier.
    /**
     * The permission list comes from IQueries::get_permissions(), which a
     * REST client cannot return before the user has logged in.  A widget
     * constructed at startup therefore starts without any rows; call this
     * once the page is entered (after authentication) to populate them.
     * It is a no-op once the rows exist.
     */
    void reload_columns();

    /// Writes the current checkbox states to the database immediately.
    void save_permissions();

    /// Enables or disables editing of the currently shown permissions.
    /**
     * Use this to reflect that the logged-in user may not manage the
     * selected target (the containment invariant): the checkboxes stay
     * visible but become non-editable.  Re-applies the enabled state to the
     * checkboxes of the person currently shown.
     * @param read_only  if true, editing is disabled
     */
    void set_read_only(bool read_only);

  Q_SIGNALS:
    /// Emitted when the user toggles any permission checkbox.
    void permissions_changed();

  private Q_SLOTS:
    void on_person_selected(int row);
    void on_apply_clicked();
    void on_reset_clicked();

  private:
    void build_ui();
    void populate_permission_rows();
    void populate_person_list();
    void load_permissions(int person_id);
    void set_checkboxes_enabled(bool enabled);
    void apply_checkbox_dependencies(QString const& column, bool checked);
    /// Returns the translated display label for \a column.
    QString permission_label(QString const& column) const;

    /// One visible permission: its database column name, label and checkbox.
    struct VisiblePermission
    {
      QString column;           ///< permission column name in the database
      QCheckBox* box = nullptr;
      QLabel* label = nullptr;  ///< form row label
    };

    IQueries& queries_;
    bool read_only_ = false;
    bool embedded_ = false;
    int fixed_person_id_ = -1;
    int current_person_id_ = -1;
    QSet<QString> non_grantable_;

    QListWidget* person_list_ = nullptr;
    QLabel* name_label_ = nullptr;
    QFormLayout* perm_form_ = nullptr;
    std::vector<VisiblePermission> visible_permissions_;
    QPushButton* apply_btn_ = nullptr;
    QPushButton* reset_btn_ = nullptr;
  };

} // namespace colman

#endif /* #ifndef COLMAN_PERMISSIONS_WIDGET_HPP */

/* EOF */
