// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Widget for viewing and editing per-user permissions.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_permissions_widget.hpp"

#ifndef COLMAN_PERMISSION_DEPS_HPP
#  include "colman_permission_deps.hpp"
#endif
#ifndef COLMAN_TRY_CATCH_HPP
#  include "colman_try_catch.hpp"
#endif
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QSTRINGLIST
#  include <QStringList>
#  define SYSTEM_QSTRINGLIST
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

/// Constructs the permissions widget.
/**
 * Builds the UI, populates the person list (unless a fixed person_id
 * is given), and loads initial permission state.  Permissions listed
 * in \a non_grantable_permissions are shown greyed out and never written.
 *
 * @param queries                   database queries (must stay valid)
 * @param read_only                 if true the checkboxes and buttons are
 *                                  disabled
 * @param person_id                 if >= 0, show only this person's
 *                                  permissions and hide the person list
 * @param non_grantable_permissions column names of permissions the user may
 *                                  not grant (shown greyed out)
 * @param embedded                  if true, hide person list, name label, and
 *                                  Apply/Reset buttons (for inline use)
 * @param parent                    optional parent widget
 */
colman::PermissionsWidget::PermissionsWidget(
    IQueries& queries, bool read_only, int person_id,
    QSet<QString> const& non_grantable_permissions, bool embedded,
    QWidget* parent)
    : QWidget(parent), queries_(queries), read_only_(read_only),
      embedded_(embedded), fixed_person_id_(person_id),
      non_grantable_(non_grantable_permissions)
{
  build_ui();

  if(fixed_person_id_ >= 0) {
    person_list_->hide();
    load_permissions(fixed_person_id_);
  }
  else {
    populate_person_list();
    if(person_list_->count() > 0) {
      person_list_->setCurrentRow(0);
    }
  }

  if(read_only_) {
    set_checkboxes_enabled(false);
    apply_btn_->setEnabled(false);
    reset_btn_->setEnabled(false);
  }
}

/// Builds the complete UI layout.
/**
 * Left side: QListWidget with persons (hidden in single-person or
 * embedded mode).  Right side: name label, permission checkboxes in a
 * group box, and Apply/Reset buttons at the bottom.  All permissions
 * are created; rows whose column name is in non_grantable_ start disabled
 * (greyed out) so the user can see but not change them.
 * In embedded mode the person list, name label, and Apply/Reset buttons
 * are hidden so the widget can be placed inline in a larger form.
 */
void colman::PermissionsWidget::build_ui()
{
  QHBoxLayout* main_layout = new QHBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);

  // -- Left: person list --
  person_list_ = new QListWidget;
  person_list_->setMinimumWidth(180);
  main_layout->addWidget(person_list_, 1);

  // -- Right: permissions panel --
  QVBoxLayout* right_layout = new QVBoxLayout;

  name_label_ = new colman::ScalingLabel(nullptr, 2, true);
  right_layout->addWidget(name_label_);

  QGroupBox* group = new QGroupBox(tr("Permissions"));
  perm_form_ = new QFormLayout(group);

  // Create the permission rows.  This may be empty in a REST client that is
  // not yet logged in; reload_columns() rebuilds them after authentication.
  populate_permission_rows();
  // Grey out permissions the user may not grant.
  set_checkboxes_enabled(!read_only_);

  if(embedded_) {
    right_layout->addWidget(group);
  }
  else {
    auto* scroll = new QScrollArea;
    scroll->setWidget(group);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    right_layout->addWidget(scroll, 1);
  }

  // -- Buttons --
  QHBoxLayout* btn_layout = new QHBoxLayout;
  apply_btn_ = new QPushButton(tr("Apply"));
  reset_btn_ = new QPushButton(tr("Reset"));
  btn_layout->addStretch();
  btn_layout->addWidget(apply_btn_);
  btn_layout->addWidget(reset_btn_);
  right_layout->addLayout(btn_layout);

  main_layout->addLayout(right_layout, 2);

  // -- Connections --
  connect(person_list_, &QListWidget::currentRowChanged, this,
          &PermissionsWidget::on_person_selected);
  connect(apply_btn_, &QPushButton::clicked, this,
          &PermissionsWidget::on_apply_clicked);
  connect(reset_btn_, &QPushButton::clicked, this,
          &PermissionsWidget::on_reset_clicked);

  if(embedded_) {
    person_list_->hide();
    name_label_->hide();
    apply_btn_->hide();
    reset_btn_->hide();
    set_checkboxes_enabled(false);
  }
}

/// Creates the labelled permission checkboxes, sorted by label.
/**
 * Reads the permission column list from the database and builds one labelled
 * checkbox per column.  Does nothing if the rows already exist (idempotent) or
 * if the list is currently empty, which happens in a REST client before the
 * user has logged in.  The dependency-cascade and change signals are connected
 * here so they are wired up regardless of when the rows are built.
 */
void colman::PermissionsWidget::populate_permission_rows()
{
  if(!visible_permissions_.empty()) {
    return;
  }
  auto const columns = queries_.get_permissions();
  if(columns.empty()) {
    return;
  }
  for(auto const& col : columns) {
    VisiblePermission vp;
    vp.column = col;
    vp.label = new QLabel(permission_label(col));
    vp.box = new QCheckBox;
    visible_permissions_.push_back(vp);
  }
  std::sort(
      visible_permissions_.begin(), visible_permissions_.end(),
      [this](VisiblePermission const& a, VisiblePermission const& b) {
        return QString::localeAwareCompare(
                   permission_label(a.column),
                   permission_label(b.column)) < 0;
      });
  for(auto const& vp : visible_permissions_) {
    perm_form_->addRow(vp.label, vp.box);
  }
  for(auto const& vp : visible_permissions_) {
    connect(vp.box, &QCheckBox::toggled, this,
            [this, col = vp.column](bool checked) {
              apply_checkbox_dependencies(col, checked);
              Q_EMIT permissions_changed();
            });
  }
}

void colman::PermissionsWidget::reload_columns()
{
  populate_permission_rows();
  // Reflect the current context: editable only when a person is selected and
  // the logged-in user may manage it.
  set_checkboxes_enabled(current_person_id_ > 0 && !read_only_);
}

/// Populates the person list from the database.
/**
 * Queries all persons with id > 0 (excluding the administrator)
 * and adds them to the QListWidget.  Each item stores the person_id
 * as Qt::UserRole data.
 */
void colman::PermissionsWidget::populate_person_list()
{
  person_list_->clear();
  auto const& persons = queries_.query_persons(-1, "");
  for(auto const& person : persons) {
    int const pid = person.get_id();
    QString const name = person.get_lastname() + ", " + person.get_firstname();
    QListWidgetItem* item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, pid);
    person_list_->addItem(item);
  }
}

/// Loads permissions for the given person and updates the checkboxes.
/**
 * Reads all permission flags from the database in a single query
 * and sets each visible checkbox accordingly.  If no permissions row
 * exists yet, default permissions are created first.
 *
 * @param person_id the person whose permissions to load
 */
void colman::PermissionsWidget::load_permissions(int person_id)
{
  COLMAN_TRY
  {
    current_person_id_ = person_id;

    // Update name label.
    {
      auto const& persons = queries_.query_persons(person_id, "");
      for(auto const& person : persons) {
        name_label_->setText(person.get_lastname() + ", " +
                             person.get_firstname());
      }
    }
    auto const columns = queries_.get_permissions();
    auto const& perms = queries_.get_user_permissions(person_id);
    for(auto const& vp : visible_permissions_) {
      auto const it = std::find(columns.begin(), columns.end(), vp.column);
      if(it != columns.end()) {
        int const idx = static_cast<int>(std::distance(columns.begin(), it));
        if(idx < static_cast<int>(perms.size())) {
          QSignalBlocker const b(vp.box);
          vp.box->setChecked(perms[idx]);
        }
      }
    }
  }
  COLMAN_CATCH;
}


/// Enables or disables the permission checkboxes.
/**
 * A checkbox is only enabled when \a enabled is true and its column is not
 * in non_grantable_; permissions the logged-in user may not grant always
 * stay disabled (greyed out) so they are visible but not editable.
 * @param enabled true to enable editable checkboxes, false to disable all
 */
void colman::PermissionsWidget::set_checkboxes_enabled(bool enabled)
{
  for(auto const& vp : visible_permissions_) {
    vp.box->setEnabled(enabled && !non_grantable_.contains(vp.column));
  }
}

/// Called when a person is selected in the list.
/**
 * Loads the permissions for the newly selected person.
 * @param row  the current row index in the person list
 */
void colman::PermissionsWidget::on_person_selected(int row)
{
  if(row < 0) {
    return;
  }
  QListWidgetItem* item = person_list_->item(row);
  if(!item) {
    return;
  }
  int const pid = item->data(Qt::UserRole).toInt();
  load_permissions(pid);
}

/// Writes the current checkbox states to the database.
/**
 * Iterates over all visible permission columns and calls
 * colman::set_permission() for each grantable one.  Non-grantable
 * permissions are left unchanged.  If any write fails (for example because
 * the server rejects it), the user is informed and the checkboxes are
 * reloaded to reflect the actual database state.
 */
void colman::PermissionsWidget::on_apply_clicked()
{
  COLMAN_TRY
  {
    if(current_person_id_ < 0) {
      return;
    }
    QStringList failed;
    for(auto const& vp : visible_permissions_) {
      if(non_grantable_.contains(vp.column)) {
        continue;
      }
      if(!queries_.set_permission(current_person_id_, vp.column,
                                  vp.box->isChecked())) {
        failed.push_back(permission_label(vp.column));
      }
    }
    if(!failed.isEmpty()) {
      QMessageBox::warning(
          this, tr("Could not change permissions"),
          tr("The following permissions could not be changed:\n%1")
              .arg(failed.join(QStringLiteral("\n"))));
      // Reload so the display matches what was actually stored.
      load_permissions(current_person_id_);
    }
  }
  COLMAN_CATCH;
}

/// Reloads permissions from the database, discarding unsaved changes.
void colman::PermissionsWidget::on_reset_clicked()
{
  COLMAN_TRY
  {
    if(current_person_id_ >= 0) {
      load_permissions(current_person_id_);
    }
  }
  COLMAN_CATCH;
}


/// Cascades a checkbox state change to all dependent or prerequisite boxes.
/**
 * When \a checked is true, all permissions that \a column requires
 * (transitively) are also checked.  When \a checked is false, all
 * permissions that require \a column (transitively) are also unchecked.
 * Affected checkboxes have their signals blocked during the update to
 * prevent recursive re-entry.
 *
 * @param column  the permission column name of the checkbox that changed
 * @param checked the new state of that checkbox
 */
void colman::PermissionsWidget::apply_checkbox_dependencies(
    QString const& column, bool checked)
{
  // Compute the full set of columns whose state must also change.
  auto const affected = permission_cascade(column, checked);

  // Apply to all editable checkboxes except the one that triggered this call.
  // Non-grantable (greyed-out) permissions are left untouched; the server
  // applies the same dependency cascade for those when it stores the change.
  for(auto const& vp : visible_permissions_) {
    if(vp.column == column || !affected.contains(vp.column) ||
       non_grantable_.contains(vp.column))
    {
      continue;
    }
    vp.box->blockSignals(true);
    vp.box->setChecked(checked);
    vp.box->blockSignals(false);
  }
}

/// Returns the translated display label for \a column.
/**
 * Maps a database permission column name to its localised display string.
 * Falls back to the raw column name for any unknown column so that newly
 * added permissions are always visible, even before a label is defined.
 * @param column  database column name (e.g. "edit_persons")
 * @return        translated label string
 */
QString colman::PermissionsWidget::permission_label(
    QString const& column) const
{
  if(column == "edit_persons") {
    return tr("Edit persons");
  }
  if(column == "create_lenders") {
    return tr("Create borrowers");
  }
  if(column == "create_data_collectors") {
    return tr("Create data collectors");
  }
  if(column == "collect_lendings") {
    return tr("Record lendings");
  }
  if(column == "collect_returns") {
    return tr("Record returns");
  }
  if(column == "collect_acquisitions") {
    return tr("Record acquisitions");
  }
  if(column == "collect_liquidations") {
    return tr("Record liquidations");
  }
  if(column == "view_history") {
    return tr("View history");
  }
  if(column == "view_persons") {
    return tr("View person lists");
  }
  if(column == "view_items") {
    return tr("View item lists");
  }
  if(column == "edit_preferences") {
    return tr("Edit preferences");
  }
  if(column == "print_ids") {
    return tr("Print ID cards");
  }
  if(column == "print_barcodes") {
    return tr("Print labels");
  }
  if(column == "edit_categories") {
    return tr("Edit categories");
  }
  if(column == "view_personal_data") {
    return tr("View personal data");
  }
  if(column == "manage_label_layouts") {
    return tr("Manage label layouts");
  }
  if(column == "calibrate_printers") {
    return tr("Calibrate printers");
  }
  if(column == "edit_id_layout") {
    return tr("Edit ID card layout");
  }
  if(column == "print_signatures") {
    return tr("Print signature labels");
  }
  if(column == "change_actions") {
    return tr("Change and delete actions");
  }
  if(column == "edit_program_settings") {
    return tr("Edit program settings");
  }
  if(column == "manage_classes") {
    return tr("Manage classes");
  }
  if(column == "reserve") {
    return tr("Reserve media");
  }
  if(column == "edit_reservations") {
    return tr("Edit reservations");
  }
  if(column == "admin_permissions") {
    return tr("Edit item settings");
  }
  return column;
}

/// Writes the current checkbox states to the database immediately.
void colman::PermissionsWidget::save_permissions()
{
  on_apply_clicked();
}

/// Enables or disables editing of the currently shown permissions.
void colman::PermissionsWidget::set_read_only(bool read_only)
{
  read_only_ = read_only;
  if(current_person_id_ > 0) {
    set_checkboxes_enabled(!read_only_);
  }
}

/// Saves current person's permissions then loads permissions for a new person.
/**
 * Called by the person management page when the selection changes.
 * If there is a currently loaded person their pending checkbox state is
 * written to the database before switching.  The non-grantable set is
 * updated so that each call can use the permissions of the currently
 * logged-in data collector.
 *
 * @param person_id      id of the person to display, or <= 0 to clear
 * @param non_grantable  column names of permissions the user may not grant
 */
void colman::PermissionsWidget::load_for_person(
    int person_id, QSet<QString> const& non_grantable)
{
  // Auto-save previous person before switching.
  if(current_person_id_ > 0 && !read_only_) {
    on_apply_clicked();
  }

  // Update the non-grantable set; its rows are greyed out, not hidden.
  non_grantable_ = non_grantable;

  if(person_id > 0) {
    load_permissions(person_id);
    set_checkboxes_enabled(!read_only_);
  }
  else {
    current_person_id_ = -1;
    for(auto const& vp : visible_permissions_) {
      QSignalBlocker const b(vp.box);
      vp.box->setChecked(false);
    }
    set_checkboxes_enabled(false);
  }
}

/* EOF */
