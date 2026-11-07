// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Central page and change-passwords page setup functions.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_central_page.hpp"
#ifndef COLMAN_ACTIONS_HPP
#  include "colman_actions.hpp"
#endif
#ifndef COLMAN_CLASS_NAMES_HPP
#  include "colman_class_names.hpp"
#endif
#ifndef COLMAN_CAMERA_SCAN_DIALOG_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_ACQUISITION_PAGE_HPP
#  include "colman_acquisition_page.hpp"
#endif
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef COLMAN_LENDING_PAGE_HPP
#  include "colman_lending_page.hpp"
#endif
#ifndef COLMAN_RETURN_PAGE_HPP
#  include "colman_return_page.hpp"
#endif
#ifndef COLMAN_LIQUIDATION_PAGE_HPP
#  include "colman_liquidation_page.hpp"
#endif
#ifndef COLMAN_ID_EDITOR_HPP
#  include "colman_id_editor.hpp"
#endif
#ifndef COLMAN_ITEM_CARD_EDITOR_HPP
#  include "colman_item_card_editor.hpp"
#endif
#ifndef COLMAN_SIGNATURE_LABEL_EDITOR_HPP
#  include "colman_signature_label_editor.hpp"
#endif
#ifndef COLMAN_SIGNATURE_PRINT_PAGE_HPP
#  include "colman_signature_print_page.hpp"
#endif
#ifndef COLMAN_START_PAGE_HPP
#  include "colman_start_page.hpp"
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#endif
#ifndef SYSTEM_FUNCTIONAL
#  include <functional>
#  define SYSTEM_FUNCTIONAL
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMAYOUT
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif
#ifndef SYSTEM_QITEMSELECTIONMODEL
#  include <QItemSelectionModel>
#  define SYSTEM_QITEMSELECTIONMODEL
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QCAMERADEVICE
#  include <QCameraDevice>
#  define SYSTEM_QCAMERADEVICE
#endif
#ifndef SYSTEM_QMEDIADEVICES
#  include <QMediaDevices>
#  define SYSTEM_QMEDIADEVICES
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSIZE
#  include <QSize>
#  define SYSTEM_QSIZE
#endif
#ifndef COLMAN_TABLE_MODELS_HPP
#  include "colman_table_models.hpp"
#endif
#ifndef COLMAN_EXTEND_LENDING_DIALOG_HPP
#  include "colman_extend_lending_dialog.hpp"
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef COLMAN_PRINTER_CALIBRATION_WIDGET_HPP
#  include "colman_printer_calibration_widget.hpp"
#  define COLMAN_PRINTER_CALIBRATION_WIDGET_HPP
#endif
#ifndef COLMAN_SETTINGS_PAGE_HPP
#  include "colman_settings_page.hpp"
#endif
#ifndef COLMAN_ENCRYPTION_PASSWORD_WIDGET_HPP
#  include "colman_encryption_password_widget.hpp"
#endif
#ifndef COLMAN_PREFERENCES_PAGE_HPP
#  include "colman_preferences_page.hpp"
#endif
#ifndef SYSTEM_QTABWIDGET
#  include <QTabWidget>
#  define SYSTEM_QTABWIDGET
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QCOMMANDLINKBUTTON
#  include <QCommandLinkButton>
#  define SYSTEM_QCOMMANDLINKBUTTON
#endif
#ifndef SYSTEM_QMOUSEEVENT
#  include <QMouseEvent>
#  define SYSTEM_QMOUSEEVENT
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QKEYEVENT
#  include <QKeyEvent>
#  define SYSTEM_QKEYEVENT
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSIGNALBLOCKER
#  include <QSignalBlocker>
#  define SYSTEM_QSIGNALBLOCKER
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef COLMAN_TILE_BUTTON_HPP
#  include "colman_tile_button.hpp"
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QMENU
#  include <QMenu>
#  define SYSTEM_QMENU
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QTEXTDOCUMENT
#  include <QTextDocument>
#  define SYSTEM_QTEXTDOCUMENT
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  include <QPageLayout>
#  include <QMarginsF>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QPRINTPREVIEWDIALOG
#  include <QPrintPreviewDialog>
#  define SYSTEM_QPRINTPREVIEWDIALOG
#endif
#ifndef SYSTEM_QSCREEN
#  include <QScreen>
#  define SYSTEM_QSCREEN
#endif
#ifndef SYSTEM_QSTYLE
#  include <QStyle>
#  define SYSTEM_QSTYLE
#endif


namespace
{

/// Appends a heading row (48×48 icon + bold title) to @p vbox.
/**
   Used by programmatic page classes to show a small version of the
   central-page button icon directly below the back button.
   @param vbox      layout to append to
   @param icon_path resource path of the icon
   @param title     title text (will be rendered bold, +2pt)
   @param parent    parent widget for the labels
*/
  void add_page_heading(QVBoxLayout* vbox,
                        QString const& icon_path,
                        QString const& title,
                        QWidget* parent)
  {
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setContentsMargins(4, 4, 4, 4);
    QLabel* icon_lbl = new QLabel(parent);
    QPixmap const pm(icon_path);
    if(!pm.isNull()) {
      icon_lbl->setPixmap(
          pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      icon_lbl->setFixedSize(52, 52);
      icon_lbl->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    }
    hbox->addWidget(icon_lbl);
    QLabel* title_lbl = new colman::ScalingLabel("<b>" + title + "</b>", parent, 2);
    hbox->addWidget(title_lbl, 1);
    vbox->addLayout(hbox);
  }


/// Page for a regular user to change their own password.
/**
   Prompts for the current password before accepting a new one.  A
   wrong current password locks the input for three seconds.  The OK
   button is enabled only when all three fields are filled and the two
   new-password fields match.  Calls on_back_() after a successful change
   or when the Back button is pressed.
*/
  class ChangePasswordPage : public QWidget
  {
  public:
  /// Constructs the page for the currently logged-in user in @p data.
  /**
     @param data    central application state
     @param on_back called after a successful change or on Back
     @param parent  optional parent widget
  */
    explicit ChangePasswordPage(colman::AppData& data,
                                std::function<void()> on_back,
                                QWidget* parent = nullptr)
        : QWidget(parent), data_(data), on_back_(on_back)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

      QVBoxLayout* icon_vbox = new QVBoxLayout;
      icon_vbox->setSpacing(0);
      icon_vbox->setContentsMargins(0, 0, 0, 0);
      QLabel* img = new QLabel(this);
      QPixmap const pm(":/icons/owls/change_password.jpeg");
      if(!pm.isNull()) {
        img->setPixmap(pm.scaledToHeight(120, Qt::SmoothTransformation));
        img->setAlignment(Qt::AlignHCenter);
        img->setFixedHeight(120);
        icon_vbox->addWidget(img);
      }
      QLabel* pw_title = new QLabel(
          "<b>" + QApplication::tr("Change Password") + "</b>", this);
      pw_title->setAlignment(Qt::AlignHCenter);
      icon_vbox->addWidget(pw_title);
      vbox->addLayout(icon_vbox);

      QFormLayout* form = new QFormLayout;
      old_pw_ = new QLineEdit(this);
      old_pw_->setEchoMode(QLineEdit::Password);
      new_pw_ = new QLineEdit(this);
      new_pw_->setEchoMode(QLineEdit::Password);
      confirm_pw_ = new QLineEdit(this);
      confirm_pw_->setEchoMode(QLineEdit::Password);
      {
        QChar const mask(old_pw_->style()->styleHint(
            QStyle::SH_LineEdit_PasswordCharacter));
        int const pw_w =
            old_pw_->fontMetrics().horizontalAdvance(QString(20, mask)) + 8;
        old_pw_->setMinimumWidth(pw_w);
        new_pw_->setMinimumWidth(pw_w);
        confirm_pw_->setMinimumWidth(pw_w);
      }
      form->addRow(QApplication::tr("Current password:"), old_pw_);
      form->addRow(QApplication::tr("New password:"), new_pw_);
      form->addRow(QApplication::tr("Confirm:"), confirm_pw_);
      vbox->addLayout(form);

      error_label_ = new QLabel(this);
      error_label_->setStyleSheet("color: red;");
      error_label_->setAlignment(Qt::AlignHCenter);
      error_label_->setVisible(false);
      vbox->addWidget(error_label_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("OK"), this);
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(old_pw_, &QLineEdit::textChanged,
              [this]() { check_ok_btn(); });
      connect(new_pw_, &QLineEdit::textChanged,
              [this]() { check_ok_btn(); });
      connect(confirm_pw_, &QLineEdit::textChanged,
              [this]() { check_ok_btn(); });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() { try_change(); });
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
    }

  /// Resets all fields for a fresh attempt and focuses the first field.
    void enter()
    {
      old_pw_->setEnabled(true);
      new_pw_->setEnabled(true);
      confirm_pw_->setEnabled(true);
      old_pw_->clear();
      new_pw_->clear();
      confirm_pw_->clear();
      error_label_->setVisible(false);
      ok_btn_->setEnabled(false);
      old_pw_->setFocus();
    }

  private:
  /// Enables OK when all fields are filled and new passwords match.
    void check_ok_btn()
    {
      bool const ok =
          !old_pw_->text().isEmpty() &&
          !new_pw_->text().isEmpty() &&
          new_pw_->text() == confirm_pw_->text();
      ok_btn_->setEnabled(ok);
      if(!new_pw_->text().isEmpty() &&
         !confirm_pw_->text().isEmpty() &&
         new_pw_->text() != confirm_pw_->text())
      {
        error_label_->setText(
            QApplication::tr("Passwords do not match."));
        error_label_->setVisible(true);
      }
      else {
        error_label_->setVisible(false);
      }
    }

  /// Verifies the current password; on success sets the new one.
    void try_change()
    {
      int const id = data_.current_data_collector_;
      QString const new_hash = colman::hash_password(new_pw_->text());
      if(data_.queries_->change_own_password(id, old_pw_->text(), new_hash)) {
        if(on_back_) {
          on_back_();
        }
      }
      else {
        error_label_->setText(QApplication::tr("Wrong password!"));
        error_label_->setVisible(true);
        old_pw_->setEnabled(false);
        new_pw_->setEnabled(false);
        confirm_pw_->setEnabled(false);
        ok_btn_->setEnabled(false);
        QTimer::singleShot(3000, old_pw_,
                           [this]() {
                             old_pw_->setEnabled(true);
                             old_pw_->clear();
                             old_pw_->setFocus();
                             new_pw_->setEnabled(true);
                             confirm_pw_->setEnabled(true);
                             ok_btn_->setEnabled(false);
                             error_label_->setVisible(false);
                           });
      }
    }

    colman::AppData& data_;
    std::function<void()> on_back_;
    QLineEdit* old_pw_ = nullptr;
    QLineEdit* new_pw_ = nullptr;
    QLineEdit* confirm_pw_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };


/// Page showing the current user's active reservations with cancel buttons.
/**
   Replaces the former ManageReservationsDialog. The back button at the
   top returns to the central page.
*/
  class ManageReservationsPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param data    central application state
     @param on_back called when the Back button is pressed
     @param parent  optional parent widget
  */
    explicit ManageReservationsPage(colman::AppData& data,
                                    std::function<void()> on_back,
                                    QWidget* parent = nullptr)
        : QWidget(parent), data_(data)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      QHBoxLayout* hdr_hbox = new QHBoxLayout;
      hdr_hbox->setContentsMargins(4, 4, 4, 4);
      QLabel* owl_lbl = new QLabel(this);
      QPixmap const pm(":/icons/owls/reserve2.jpeg");
      if(!pm.isNull()) {
        owl_lbl->setPixmap(pm.scaledToHeight(64, Qt::SmoothTransformation));
        owl_lbl->setFixedHeight(64);
      }
      hdr_hbox->addWidget(owl_lbl);
      QLabel* title_lbl = new colman::ScalingLabel(
          "<b>" + QApplication::tr("My reservations") + "</b>", this, 4);
      hdr_hbox->addWidget(title_lbl, 1);
      vbox->addLayout(hdr_hbox);

      table_ = new QTableWidget(0, 3, this);
      table_->setHorizontalHeaderLabels(
          {QApplication::tr("Item"),
           QApplication::tr("Expires"),
           QString()});
      table_->setSelectionMode(QAbstractItemView::NoSelection);
      table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
      table_->horizontalHeader()->setSectionResizeMode(
          0, QHeaderView::Stretch);
      table_->horizontalHeader()->setSectionResizeMode(
          1, QHeaderView::ResizeToContents);
      table_->horizontalHeader()->setSectionResizeMode(
          2, QHeaderView::ResizeToContents);
      table_->verticalHeader()->setVisible(false);
      vbox->addWidget(table_, 1);

      connect(back_btn, &QPushButton::clicked,
              [on_back]() { on_back(); });
    }

  /// Refreshes the reservation list for a new visit.
    void enter()
    {
      load();
    }

  private:
  /// Fetches and displays the current user's active reservations.
    void load()
    {
      table_->setRowCount(0);
      auto const ress = data_.queries_->get_reservations(
          data_.current_data_collector_);
      for(auto const& res : ress) {
        colman::Item item;
        QString unused;
        data_.queries_->get_item(res.item_id, item, unused);
        int const row = table_->rowCount();
        table_->insertRow(row);

        auto* name_item = new QTableWidgetItem(item.get_name());
        name_item->setFlags(name_item->flags() & ~Qt::ItemIsEditable);
        table_->setItem(row, 0, name_item);

        QString const date_str =
            QLocale().toString(res.expires_at, QLocale::ShortFormat);
        auto* date_item = new QTableWidgetItem(date_str);
        date_item->setFlags(date_item->flags() & ~Qt::ItemIsEditable);
        table_->setItem(row, 1, date_item);

        auto* btn = new QPushButton(
            QApplication::tr("Cancel reservation"), this);
        int const res_id = res.id;
        connect(btn, &QPushButton::clicked,
                [this, res_id, btn]() {
                  auto const ans = QMessageBox::question(
                      this,
                      QApplication::tr("Cancel reservation"),
                      QApplication::tr("Cancel this reservation?"));
                  if(ans != QMessageBox::Yes) {
                    return;
                  }
                  btn->setEnabled(false);
                  if(!data_.queries_->remove_reservation(res_id)) {
                    QMessageBox::warning(
                        this,
                        QApplication::tr("Error"),
                        QApplication::tr("Could not cancel reservation."));
                    btn->setEnabled(true);
                    return;
                  }
                  load();
                });
        table_->setCellWidget(row, 2, btn);
      }
      if(table_->rowCount() == 0) {
        table_->setRowCount(1);
        auto* empty = new QTableWidgetItem(
            QApplication::tr("No active reservations."));
        empty->setFlags(empty->flags() & ~Qt::ItemIsEditable);
        table_->setItem(0, 0, empty);
        table_->setSpan(0, 0, 1, 3);
      }
    }

    colman::AppData& data_;
    QTableWidget* table_ = nullptr;
  };


/// Page for the administrator to view and change all user passwords.
/**
   Shows a list of all users (including Administrator) on the left.
   The right side holds the owl image, two password input fields, a
   "Change Password" button and a "Delete Password" button.  All
   right-side controls are disabled until a person is selected.
   Each destructive action is guarded by a confirmation dialog.
*/
  class ChangePasswordsPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param data    central application state
     @param on_back called when the Back button is pressed
     @param parent  optional parent widget
  */
    explicit ChangePasswordsPage(colman::AppData& data,
                                 std::function<void()> on_back,
                                 QWidget* parent = nullptr)
        : QWidget(parent),
          data_(data),
          on_back_(std::move(on_back))
    {
      QVBoxLayout* outer_vbox = new QVBoxLayout(this);
      outer_vbox->setContentsMargins(0, 0, 0, 0);
      outer_vbox->setSpacing(0);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer_vbox->addWidget(back_btn_);

      QHBoxLayout* hbox = new QHBoxLayout;
      outer_vbox->addLayout(hbox, 1);

    // --- left: person list ---
      table_ = new QTableView(this);
      table_->setModel(&model_);
      table_->setSelectionBehavior(QAbstractItemView::SelectRows);
      table_->setSelectionMode(QAbstractItemView::SingleSelection);
      table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
      table_->horizontalHeader()->setSectionResizeMode(
          QHeaderView::ResizeToContents);
      table_->verticalHeader()->setVisible(false);
      table_->setColumnHidden(0, true);
      hbox->addWidget(table_, 1);

    // --- right panel ---
      QVBoxLayout* right_vbox = new QVBoxLayout;
      right_vbox->addStretch(1);

      QVBoxLayout* icon_vbox = new QVBoxLayout;
      icon_vbox->setSpacing(0);
      icon_vbox->setContentsMargins(0, 0, 0, 0);
      QLabel* img = new QLabel(this);
      QPixmap const pm(":/icons/owls/edit_passwords.jpeg");
      if(!pm.isNull()) {
        img->setPixmap(pm.scaledToHeight(120, Qt::SmoothTransformation));
        img->setAlignment(Qt::AlignHCenter);
        img->setFixedHeight(120);
      }
      icon_vbox->addWidget(img);
      QLabel* pw_title = new QLabel(
          "<b>" + QApplication::tr("Change Passwords") + "</b>", this);
      pw_title->setAlignment(Qt::AlignHCenter);
      icon_vbox->addWidget(pw_title);
      right_vbox->addLayout(icon_vbox);

      name_label_ = new colman::ScalingLabel(this, 4, true);
      name_label_->setAlignment(Qt::AlignHCenter);
      right_vbox->addWidget(name_label_);

    // --- username section ---
      QFormLayout* uname_form = new QFormLayout;
      username_ = new QLineEdit(this);
      username_->setEnabled(false);
      uname_form->addRow(QApplication::tr("Username:"), username_);
      right_vbox->addLayout(uname_form);

      QHBoxLayout* uname_btn_hbox = new QHBoxLayout;
      remove_username_btn_ =
          new QPushButton(QApplication::tr("Remove Username"), this);
      remove_username_btn_->setEnabled(false);
      set_username_btn_ =
          new QPushButton(QApplication::tr("Set Username"), this);
      set_username_btn_->setEnabled(false);
      uname_btn_hbox->addStretch(1);
      uname_btn_hbox->addWidget(remove_username_btn_);
      uname_btn_hbox->addWidget(set_username_btn_);
      right_vbox->addLayout(uname_btn_hbox);

    // --- password section ---
      QFormLayout* form = new QFormLayout;
      new_pw_ = new QLineEdit(this);
      new_pw_->setEchoMode(QLineEdit::Password);
      new_pw_->setEnabled(false);
      confirm_pw_ = new QLineEdit(this);
      confirm_pw_->setEchoMode(QLineEdit::Password);
      confirm_pw_->setEnabled(false);
      {
        QChar const mask(new_pw_->style()->styleHint(
            QStyle::SH_LineEdit_PasswordCharacter));
        int const pw_w =
            new_pw_->fontMetrics().horizontalAdvance(QString(20, mask)) + 8;
        new_pw_->setMinimumWidth(pw_w);
        confirm_pw_->setMinimumWidth(pw_w);
      }
      form->addRow(QApplication::tr("New password:"), new_pw_);
      form->addRow(QApplication::tr("Confirm:"), confirm_pw_);
      right_vbox->addLayout(form);

      error_label_ = new QLabel(this);
      error_label_->setStyleSheet("color: red;");
      error_label_->setAlignment(Qt::AlignHCenter);
      error_label_->setVisible(false);
      right_vbox->addWidget(error_label_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      delete_pw_btn_ =
          new QPushButton(QApplication::tr("Delete Password"), this);
      delete_pw_btn_->setEnabled(false);
      change_btn_ =
          new QPushButton(QApplication::tr("Change Password"), this);
      change_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(delete_pw_btn_);
      btn_hbox->addWidget(change_btn_);
      right_vbox->addLayout(btn_hbox);
      right_vbox->addStretch(1);

      hbox->addLayout(right_vbox, 1);

    // --- signal connections ---
      connect(table_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [this](QItemSelection const&, QItemSelection const&) {
                on_selection_changed();
              });
      connect(username_, &QLineEdit::textChanged,
              [this]() { check_username_btn(); });
      connect(set_username_btn_, &QPushButton::clicked,
              [this]() { do_set_username(); });
      connect(remove_username_btn_, &QPushButton::clicked,
              [this]() { do_remove_username(); });
      connect(new_pw_, &QLineEdit::textChanged,
              [this]() { check_change_btn(); });
      connect(confirm_pw_, &QLineEdit::textChanged,
              [this]() { check_change_btn(); });
      connect(change_btn_, &QPushButton::clicked,
              [this]() { do_change_password(); });
      connect(delete_pw_btn_, &QPushButton::clicked,
              [this]() { do_delete_password(); });
      connect(back_btn_, &QPushButton::clicked,
              [this]() { on_back_(); });
    }

  /// Refreshes the person list and resets all controls for a new visit.
    void enter()
    {
      model_.set_rows(data_.queries_->query_change_passwords_persons());
      table_->clearSelection();
      clear_right_controls();
    }

  private:
  /// Returns the person_id of the selected row, or -1 if none.
    int selected_id() const
    {
      QModelIndexList const sel =
          table_->selectionModel()->selectedRows();
      if(sel.isEmpty()) {
        return -1;
      }
      return model_.data(model_.index(sel.first().row(), 0)).toInt();
    }

  /// Resets all right-side controls to the initial disabled/empty state.
    void clear_right_controls()
    {
      name_label_->clear();
      current_username_.clear();
      username_->clear();
      username_->setEnabled(false);
      set_username_btn_->setEnabled(false);
      remove_username_btn_->setEnabled(false);
      new_pw_->clear();
      new_pw_->setEnabled(false);
      confirm_pw_->clear();
      confirm_pw_->setEnabled(false);
      delete_pw_btn_->setEnabled(false);
      change_btn_->setEnabled(false);
      error_label_->setVisible(false);
    }

  /// Returns "Firstname Lastname" for regular users or "Administrator".
    QString person_display_name(int id) const
    {
      if(id == 0) {
        return QApplication::tr("Administrator");
      }
      for(int row = 0; row < model_.rowCount(); ++row) {
        if(model_.data(model_.index(row, 0)).toInt() == id) {
          QString const fn =
              model_.data(model_.index(row, 2)).toString();
          QString const ln =
              model_.data(model_.index(row, 1)).toString();
          return (fn + " " + ln).trimmed();
        }
      }
      return QString::number(id);
    }

  /// Enables/disables right-side controls based on the selected person.
    void on_selection_changed()
    {
      QModelIndexList const sel =
          table_->selectionModel()->selectedRows();
      int const id = sel.isEmpty() ? -1 : model_.data(model_.index(sel.first().row(), 0)).toInt();
      error_label_->setVisible(false);
      new_pw_->clear();
      confirm_pw_->clear();
      if(id < 0) {
        clear_right_controls();
        return;
      }
      name_label_->setText(person_display_name(id));
      current_username_ =
          model_.data(model_.index(sel.first().row(), 4)).toString();
      username_->setText(current_username_);
      username_->setEnabled(true);
      set_username_btn_->setEnabled(false);
      remove_username_btn_->setEnabled(!current_username_.isEmpty());
      new_pw_->setEnabled(true);
      confirm_pw_->setEnabled(true);
      change_btn_->setEnabled(false);
      QString hash;
      delete_pw_btn_->setEnabled(
          data_.queries_->get_password(id, hash));
    }

  /// Enables/disables Set Username button.
    void check_username_btn()
    {
      set_username_btn_->setEnabled(
          selected_id() >= 0 && !username_->text().isEmpty());
    }

  /// Enables/disables the Change button; shows mismatch error if needed.
    void check_change_btn()
    {
      bool const fields_ok =
          !new_pw_->text().isEmpty() &&
          new_pw_->text() == confirm_pw_->text();
      change_btn_->setEnabled(fields_ok && selected_id() >= 0);
      if(!new_pw_->text().isEmpty() &&
         !confirm_pw_->text().isEmpty() &&
         new_pw_->text() != confirm_pw_->text())
      {
        error_label_->setText(
            QApplication::tr("Passwords do not match."));
        error_label_->setVisible(true);
      }
      else {
        error_label_->setVisible(false);
      }
    }

  /// Stores the username entered in the username field.
    void do_set_username()
    {
      int const id = selected_id();
      if(id < 0) {
        return;
      }
      if(!data_.queries_->set_username(id, username_->text())) {
        error_label_->setText(
            QApplication::tr("Username already taken."));
        error_label_->setVisible(true);
        return;
      }
      current_username_ = username_->text();
      model_.update_username(id, current_username_);
      remove_username_btn_->setEnabled(true);
      error_label_->setVisible(false);
    }

  /// Confirms with the user, then removes the username.
    void do_remove_username()
    {
      int const id = selected_id();
      if(id < 0) {
        return;
      }
      if(colman::question_yes_no(
             this,
             QApplication::tr("Collection Manager"),
             QApplication::tr("Remove username for %1?")
                 .arg(person_display_name(id))) != QMessageBox::Yes)
      {
        return;
      }
      if(!data_.queries_->remove_username(id)) {
        error_label_->setText(
            QApplication::tr("Could not remove username."));
        error_label_->setVisible(true);
        return;
      }
      current_username_.clear();
      username_->clear();
      model_.update_username(id, QString());
      remove_username_btn_->setEnabled(false);
      error_label_->setVisible(false);
    }

  /// Confirms with the user, then hashes and stores the new password.
    void do_change_password()
    {
      int const id = selected_id();
      if(id < 0) {
        return;
      }
      if(colman::question_yes_no(
             this,
             QApplication::tr("Collection Manager"),
             QApplication::tr("Change password for %1?")
                 .arg(person_display_name(id))) != QMessageBox::Yes)
      {
        return;
      }
      QString const hash = colman::hash_password(new_pw_->text());
      if(data_.queries_->set_password(id, hash)) {
        new_pw_->clear();
        confirm_pw_->clear();
        change_btn_->setEnabled(false);
        QString h;
        delete_pw_btn_->setEnabled(
            data_.queries_->get_password(id, h));
      }
      else {
        error_label_->setText(
            QApplication::tr("Could not set password."));
        error_label_->setVisible(true);
      }
    }

  /// Confirms with the user, then removes the password.
    void do_delete_password()
    {
      int const id = selected_id();
      if(id < 0) {
        return;
      }
      if(colman::question_yes_no(
             this,
             QApplication::tr("Collection Manager"),
             QApplication::tr("Delete password for %1?")
                 .arg(person_display_name(id))) != QMessageBox::Yes)
      {
        return;
      }
      if(data_.queries_->remove_password(id)) {
        delete_pw_btn_->setEnabled(false);
        new_pw_->clear();
        confirm_pw_->clear();
        change_btn_->setEnabled(false);
      }
      else {
        error_label_->setText(
            QApplication::tr("Could not delete password."));
        error_label_->setVisible(true);
      }
    }

    colman::AppData& data_;
    std::function<void()> on_back_;
    colman::ChangePasswordsPersonListModel model_;
    QString current_username_;
    QTableView* table_ = nullptr;
    QLabel* name_label_ = nullptr;
    QLineEdit* username_ = nullptr;
    QPushButton* set_username_btn_ = nullptr;
    QPushButton* remove_username_btn_ = nullptr;
    QLineEdit* new_pw_ = nullptr;
    QLineEdit* confirm_pw_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* delete_pw_btn_ = nullptr;
    QPushButton* change_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };

/// Page for calibrating the print position of a specific printer.
/**
   Shows a full-width Back button at the top and the
   PrinterCalibrationWidget below.  The page owns a PageLayout loaded
   from QSettings (same keys as the item-barcode page) so the
   calibration sheet preview matches the actual label sheet.
*/
  class PrinterCalibrationPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param on_back  Called when the Back button is pressed.
     @param parent   Optional parent widget.
  */
    explicit PrinterCalibrationPage(std::function<void()> on_back,
                                    QWidget* parent = nullptr)
        : QWidget(parent)
    {
      load_layout();
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      add_page_heading(vbox,
                       ":/icons/owls/printer_calibration.jpeg",
                       QApplication::tr("calibrate printer"),
                       this);

      cal_widget_ = new colman::PrinterCalibrationWidget(layout_, this);
      vbox->addWidget(cal_widget_, 1);

      connect(back_btn, &QPushButton::clicked,
              [on_back]() { on_back(); });
    }

  private:
  /// Loads the label sheet layout from QSettings.
    void load_layout()
    {
      QSettings s;
      layout_.set_page_width(
          s.value("page_width",
                  layout_.get_page_width())
              .toDouble());
      layout_.set_page_height(
          s.value("page_height",
                  layout_.get_page_height())
              .toDouble());
      layout_.set_printer_top_margin(
          s.value("printer_top_margin",
                  layout_.get_printer_top_margin())
              .toDouble());
      layout_.set_printer_bottom_margin(
          s.value("printer_bottom_margin",
                  layout_.get_printer_bottom_margin())
              .toDouble());
      layout_.set_printer_left_margin(
          s.value("printer_left_margin",
                  layout_.get_printer_left_margin())
              .toDouble());
      layout_.set_printer_right_margin(
          s.value("printer_right_margin",
                  layout_.get_printer_right_margin())
              .toDouble());
      layout_.set_rows(
          s.value("label_rows", layout_.get_rows()).toInt());
      layout_.set_columns(
          s.value("label_columns", layout_.get_columns()).toInt());
      layout_.set_label_width(
          s.value("label_width", layout_.get_label_width()).toDouble());
      layout_.set_label_height(
          s.value("label_height", layout_.get_label_height()).toDouble());
      layout_.set_label_distance_x(
          s.value("label_distance_x",
                  layout_.get_label_distance_x())
              .toDouble());
      layout_.set_label_distance_y(
          s.value("label_distance_y",
                  layout_.get_label_distance_y())
              .toDouble());
    }

    colman::PageLayout layout_;
    colman::PrinterCalibrationWidget* cal_widget_ = nullptr;
  };


/// Page for editing the ID card layout inside the main window.
/**
   Shows a full-width Back button at the top and the IdEditor below.
   The editor is created lazily on first show to avoid SQL queries
   before a collection is opened.
*/
  class IdEditorPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param queries  database queries
     @param on_back called when the Back button is pressed.
     @param parent  optional parent widget.
  */
    explicit IdEditorPage(colman::IQueries& queries,
                          std::function<void()> on_back,
                          QWidget* parent = nullptr)
        : QWidget(parent), queries_(queries),
          on_back_(std::move(on_back))
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      add_page_heading(vbox,
                       ":/icons/owls/edit_id.jpeg",
                       QApplication::tr("edit ID layout"),
                       this);

      content_vbox_ = new QVBoxLayout;
      vbox->addLayout(content_vbox_, 1);

      connect(back_btn, &QPushButton::clicked,
              [this]() { on_back_(); });
    }

  /// Discards the lazily created editor so the next show rebuilds it.
    void clear_editor()
    {
      if(editor_) {
        editor_->deleteLater();
        editor_ = nullptr;
      }
    }

  protected:
    void showEvent(QShowEvent* e) override
    {
      QWidget::showEvent(e);
      if(!editor_) {
        editor_ = new colman::IdEditor(queries_, this);
        content_vbox_->addWidget(editor_);
      }
    }

  private:
    colman::IQueries& queries_;
    std::function<void()> on_back_;
    colman::IdEditor* editor_ = nullptr;
    QVBoxLayout* content_vbox_ = nullptr;
  };


/// Page for editing the item card layout inside the main window.
/**
   Shows a full-width Back button at the top and the ItemCardEditor below.
   The editor is created lazily on first show to avoid SQL queries
   before a collection is opened.
*/
  class ItemCardEditorPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param queries database queries
     @param data    central application state; back navigation uses
                    data.invoke_ice_back().
     @param parent  optional parent widget.
  */
    explicit ItemCardEditorPage(colman::IQueries& queries,
                                colman::AppData& data,
                                QWidget* parent = nullptr)
        : QWidget(parent), queries_(queries), data_(data)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      add_page_heading(vbox,
                       ":/icons/owls/edit_item_card.jpeg",
                       QApplication::tr("edit item card layout"),
                       this);

      content_vbox_ = new QVBoxLayout;
      vbox->addLayout(content_vbox_, 1);

      connect(back_btn, &QPushButton::clicked,
              [this]() { data_.invoke_ice_back(); });
    }

  /// Discards the lazily created editor so the next show rebuilds it.
    void clear_editor()
    {
      if(editor_) {
        editor_->deleteLater();
        editor_ = nullptr;
      }
    }

  protected:
    void showEvent(QShowEvent* e) override
    {
      QWidget::showEvent(e);
      if(!editor_) {
        editor_ = new colman::ItemCardEditor(queries_, this);
        content_vbox_->addWidget(editor_);
      }
    }

  private:
    colman::IQueries& queries_;
    colman::AppData& data_;
    colman::ItemCardEditor* editor_ = nullptr;
    QVBoxLayout* content_vbox_ = nullptr;
  };


/// Page for editing the signature label layout inside the main window.
/**
   Shows a full-width Back button at the top and the SignatureLabelEditor below.
   The editor is created lazily on first show to avoid SQL queries
   before a collection is opened.
*/
  class SignatureLabelEditorPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param queries database/REST queries for layout persistence.
     @param on_back called when the Back button is pressed.
     @param parent  optional parent widget.
  */
    explicit SignatureLabelEditorPage(colman::IQueries& queries,
                                      std::function<void()> on_back,
                                      QWidget* parent = nullptr)
        : QWidget(parent), queries_(queries)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      add_page_heading(vbox,
                       ":/icons/owls/edit_signature_layout.jpeg",
                       QApplication::tr("edit signature layout"),
                       this);

      content_vbox_ = new QVBoxLayout;
      vbox->addLayout(content_vbox_, 1);

      connect(back_btn, &QPushButton::clicked,
              [on_back]() { on_back(); });
    }

  /// Discards the lazily created editor so the next show rebuilds it.
    void clear_editor()
    {
      if(editor_) {
        editor_->deleteLater();
        editor_ = nullptr;
      }
    }

  protected:
    void showEvent(QShowEvent* e) override
    {
      QWidget::showEvent(e);
      if(!editor_) {
        editor_ = new colman::SignatureLabelEditor(queries_, this);
        content_vbox_->addWidget(editor_);
      }
    }

  private:
    colman::IQueries& queries_;
    colman::SignatureLabelEditor* editor_ = nullptr;
    QVBoxLayout* content_vbox_ = nullptr;
  };

  /// Clickable widget showing an icon, bold title, and multi-line description.
  /**
     Replaces QCommandLinkButton whose sizeHint() systematically
     under-estimates the height needed for multi-line descriptions.
  */
  class ActionButton : public QWidget
  {
  public:
    /// Constructs the widget.
    /**
       @param title       Bold first line.
       @param description Optional description; may contain newlines.
       @param icon        Optional icon shown to the left.
       @param on_click    Called on left mouse press.
       @param parent      Optional parent widget.
    */
    explicit ActionButton(QString const& title,
                          QString const& description,
                          QIcon const& icon,
                          std::function<void()> on_click,
                          QWidget* parent = nullptr)
        : QWidget(parent), on_click_(std::move(on_click))
    {
      setCursor(Qt::PointingHandCursor);
      QHBoxLayout* hbox = new QHBoxLayout(this);
      if(!icon.isNull()) {
        QLabel* icon_lbl = new QLabel(this);
        icon_lbl->setPixmap(icon.pixmap(48, 48));
        icon_lbl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        icon_lbl->setFixedSize(56, 56);
        hbox->addWidget(icon_lbl, 0, Qt::AlignTop);
      }
      QVBoxLayout* text_vbox = new QVBoxLayout;
      QLabel* title_lbl = new QLabel(title, this);
      {
        QFont title_font;
        title_font.setBold(true);
        title_lbl->setFont(title_font);
      }
      title_lbl->setWordWrap(true);
      text_vbox->addWidget(title_lbl);
      if(!description.isEmpty()) {
        QLabel* desc_lbl = new QLabel(description, this);
        desc_lbl->setWordWrap(true);
        text_vbox->addWidget(desc_lbl);
      }
      text_vbox->addStretch(1);
      hbox->addLayout(text_vbox, 1);
    }

  protected:
    void mousePressEvent(QMouseEvent* e) override
    {
      if(e->button() == Qt::LeftButton) {
        on_click_();
      }
      QWidget::mousePressEvent(e);
    }

  private:
    std::function<void()> on_click_;
  };


  /// Removes from \a actions every entry the current user is not permitted
  /// to perform.  Admin (uid 0) is always allowed everything.
  void filter_actions(colman::AppData& data,
                      std::vector<colman::ActionInfo>& actions)
  {
    int const uid = data.current_data_collector_;
    if(uid == 0) {
      return;
    }
    auto is_forbidden =
        [&](colman::ActionInfo const& ai) -> bool {
      switch(ai.action_) {
      case colman::Action::Acquisition:
        return !data.queries_->has_permission(
            uid, "collect_acquisitions");
      case colman::Action::Lending:
        return !data.queries_->has_permission(
            uid, "collect_lendings");
      case colman::Action::Return:
        return !data.queries_->has_permission(
            uid, "collect_returns");
      case colman::Action::Liquidation:
        return !data.queries_->has_permission(
            uid, "collect_liquidations");
      case colman::Action::None:
        return true;
      }
      return true;
    };
    actions.erase(
        std::remove_if(actions.begin(), actions.end(), is_forbidden),
        actions.end());
  }

  /// Returns the icon resource path for the given action type.
  QString action_icon_path(colman::Action action)
  {
    switch(action) {
    case colman::Action::Return:
      return ":/icons/owls/returns.jpeg";
    case colman::Action::Lending:
      return ":/icons/owls/lending.jpeg";
    case colman::Action::Acquisition:
      return ":/icons/owls/add_book.jpeg";
    case colman::Action::Liquidation:
      return ":/icons/owls/remove_book.jpeg";
    case colman::Action::None:
      break;
    }
    return {};
  }

  /// Returns the translated title string for the given action type.
  QString action_title(colman::Action action)
  {
    switch(action) {
    case colman::Action::Return:
      return QApplication::tr("Return");
    case colman::Action::Lending:
      return QApplication::tr("Lending");
    case colman::Action::Acquisition:
      return QApplication::tr("Acquisition");
    case colman::Action::Liquidation:
      return QApplication::tr("Liquidation");
    case colman::Action::None:
      break;
    }
    return {};
  }

  /// Builds the description text shown below the title in the action list.
  /**
     Queries item, person, and lending data from the database and formats
     a human-readable description:
     - Return:      "Name — Author\nFirstname Lastname, date"
     - Lending:     "Name — Author"
     - Acquisition: barcode (ISBN)
     - Liquidation: "Name — Author"
  */
  QString action_description(colman::AppData& data,
                             colman::ActionInfo const& ai)
  {
    QString text;
    if(ai.item_id_ >= 0) {
      colman::Item item;
      QString type_name;
      if(data.queries_->get_item(ai.item_id_, item, type_name)) {
        text = item.get_name();
        colman::StringMap const& vals = item.get_values();
        for(char const* key : {"authors", "Autor", "Author", "Verfasser",
                               "author", "Autoren"}) {
          auto it = vals.find(QString(key));
          if(it != vals.end() && !it->second.isEmpty()) {
            text += " \u2014 " + it->second;
            break;
          }
        }
      }
    }
    else if(!ai.barcode_.isEmpty()) {
      text = ai.barcode_;
    }
    if(ai.action_ == colman::Action::Return) {
      if(ai.person_id_ >= 0) {
        colman::Person person;
        if(data.queries_->get_person(ai.person_id_, person)) {
          text += "\n" + person.get_firstname() + " " + person.get_lastname();
        }
      }
      if(ai.lending_id_ >= 0) {
        colman::Lending lending;
        if(data.queries_->get_lending(ai.lending_id_, lending)) {
          text += ", " + QLocale().toString(lending.get_date_time().toLocalTime(),
                                            QLocale::ShortFormat);
        }
      }
    }
    return text;
  }


  /// Navigates to the appropriate page for a single \a action.
  void dispatch_single_action(colman::AppData& data,
                              colman::ActionInfo const& action)
  {
    switch(action.action_) {
    case colman::Action::Acquisition:
      colman::enter_acquisition_page_for_action(data, action);
      return;
    case colman::Action::Lending:
      colman::enter_lend_page_for_action(data, action);
      return;
    case colman::Action::Return:
      colman::enter_return_page_for_action(data, action);
      return;
    case colman::Action::Liquidation:
      colman::enter_liquidation_page_for_action(data, action);
      return;
    case colman::Action::None:
      break;
    }
  }

  /// Shows a page for choosing among multiple possible \a actions.
  /**
     Removes any previously created action selection page from the stack,
     then builds a new page with a full-width back button, a heading, and
     one QCommandLinkButton per action.  Clicking a button dispatches that
     action via dispatch_single_action().
  */
  void dispatch_multiple_actions(
      colman::AppData& data,
      std::vector<colman::ActionInfo> const& actions)
  {
    // Remove any leftover action selection page from a previous call.
    int const stack_size = data.mw_.pages_stack_->count();
    for(int i = 0; i < stack_size; ++i) {
      QWidget* w = data.mw_.pages_stack_->widget(i);
      if(w && w->objectName() == "action_selection_page") {
        data.mw_.pages_stack_->removeWidget(w);
        delete w;
        break;
      }
    }

    QWidget* page = new QWidget;
    page->setObjectName("action_selection_page");
    QVBoxLayout* page_vbox = new QVBoxLayout(page);

    // Back button — full width, consistent with other pages.
    QPushButton* back_btn =
        new QPushButton(QApplication::tr("back"), page);
    page_vbox->addWidget(back_btn);

    // Heading
    QLabel* heading =
        new QLabel(QApplication::tr("What do you want to do?"), page);
    {
      QFont heading_font;
      heading_font.setBold(true);
      heading->setFont(heading_font);
    }
    page_vbox->addWidget(heading);

    // Scrollable list of action buttons.
    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget;
    QVBoxLayout* content_vbox = new QVBoxLayout(content);
    content_vbox->setSpacing(4);

    for(colman::ActionInfo const& ai : actions) {
      QString const icon_path = action_icon_path(ai.action_);
      QIcon const icon = icon_path.isEmpty() ? QIcon() : QIcon(icon_path);
      ActionButton* btn = new ActionButton(
          action_title(ai.action_),
          action_description(data, ai),
          icon,
          [&data, ai]() { dispatch_single_action(data, ai); },
          content);
      content_vbox->addWidget(btn);
    }
    content_vbox->addStretch(1);
    scroll->setWidget(content);
    page_vbox->addWidget(scroll);

    QObject::connect(
        back_btn, &QPushButton::clicked,
        [&data]() {
          data.mw_.pages_stack_->setCurrentWidget(
              data.mw_.central_page_);
        });

    data.mw_.pages_stack_->addWidget(page);
    data.mw_.pages_stack_->setCurrentWidget(page);
  }

  /// Dispatches \a actions to the appropriate handler.
  /**
     - Empty list: no reaction.
     - One action: calls dispatch_single_action().
     - Multiple actions: calls dispatch_multiple_actions().
  */
  void dispatch_actions(colman::AppData& data,
                        std::vector<colman::ActionInfo> const& actions)
  {
    if(actions.empty()) {
      return;
    }
    if(actions.size() == 1) {
      dispatch_single_action(data, actions.front());
      return;
    }
    dispatch_multiple_actions(data, actions);
  }

  /// Central page widget that intercepts barcode scanner input.
  /**
     A barcode reader acting as a USB keyboard sends a sequence of
     printable characters followed by Enter (Return).  This widget
     accumulates those characters in a buffer and calls \a on_barcode
     when Enter is received.
  */
  class CentralPage : public QWidget
  {
  public:
    /// Constructs the page.
    /**
       @param on_barcode  Called with the accumulated string when Enter arrives.
       @param parent      Optional parent widget.
    */
    explicit CentralPage(
        std::function<void(QString const&)> on_barcode,
        QWidget* parent = nullptr)
        : QWidget(parent), on_barcode_(std::move(on_barcode))
    {
      setFocusPolicy(Qt::StrongFocus);
    }

  protected:
    void keyPressEvent(QKeyEvent* e) override
    {
      if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        if(!buffer_.isEmpty()) {
          on_barcode_(buffer_);
          buffer_.clear();
        }
      }
      else {
        QString const text = e->text();
        if(!text.isEmpty()) {
          buffer_ += text;
        }
      }
    }

  private:
    std::function<void(QString const&)> on_barcode_;
    QString buffer_;
  };

  /// Page for enabling encryption: a shared password widget + OK/Back.
  /**
     Calls on_accept_(password) with the chosen password when OK is pressed,
     or on_back_() when Back is pressed.
  */
  class NewEncryptionPage : public QWidget
  {
  public:
    explicit NewEncryptionPage(QWidget* parent = nullptr)
        : QWidget(parent)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

      QLabel* title = new QLabel(
          "<b>" + QApplication::tr("Enable Encryption") + "</b>", this);
      title->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(title);

      QLabel* info = new QLabel(
          QApplication::tr(
              "Enter a password to protect this collection."),
          this);
      info->setWordWrap(true);
      info->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(info);

      pw_widget_ = new colman::EncryptionPasswordWidget(this);
      vbox->addWidget(pw_widget_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("OK"), this);
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(pw_widget_, &colman::EncryptionPasswordWidget::changed,
              [this]() { ok_btn_->setEnabled(pw_widget_->is_valid()); });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() {
                if(on_accept_) {
                  on_accept_(pw_widget_->password());
                }
              });
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
    }

  /// Sets the callback invoked with the chosen password on OK.
    void set_on_accept(std::function<void(QString const&)> on_accept)
    {
      on_accept_ = std::move(on_accept);
    }

  /// Sets the callback invoked when Back is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  private:
    std::function<void(QString const&)> on_accept_;
    std::function<void()> on_back_;
    colman::EncryptionPasswordWidget* pw_widget_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };


  /// Page for changing the encryption key: current password + new-key widget.
  /**
     Calls on_accept_(old_password, new_password) on OK, or on_back_() on Back.
  */
  class ChangeEncryptionKeyPage : public QWidget
  {
  public:
    explicit ChangeEncryptionKeyPage(QWidget* parent = nullptr)
        : QWidget(parent)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

      QLabel* title = new QLabel(
          "<b>" + QApplication::tr("Change Encryption Password") + "</b>",
          this);
      title->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(title);

      QFormLayout* form = new QFormLayout;
      old_pw_edit_ = new QLineEdit(this);
      old_pw_edit_->setEchoMode(QLineEdit::Password);
      form->addRow(QApplication::tr("Current password:"), old_pw_edit_);
      vbox->addLayout(form);

      pw_widget_ = new colman::EncryptionPasswordWidget(this);
      vbox->addWidget(pw_widget_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("OK"), this);
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(old_pw_edit_, &QLineEdit::textChanged,
              [this]() { update_ok(); });
      connect(pw_widget_, &colman::EncryptionPasswordWidget::changed,
              [this]() { update_ok(); });
      connect(old_pw_edit_, &QLineEdit::returnPressed,
              [this]() { pw_widget_->setFocus(); });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() {
                if(on_accept_) {
                  on_accept_(old_pw_edit_->text(), pw_widget_->password());
                }
              });
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
    }

  /// Sets the callback invoked with (old, new) passwords on OK.
    void set_on_accept(
        std::function<void(QString const&, QString const&)> on_accept)
    {
      on_accept_ = std::move(on_accept);
    }

  /// Sets the callback invoked when Back is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  private:
    void update_ok()
    {
      ok_btn_->setEnabled(
          !old_pw_edit_->text().isEmpty() && pw_widget_->is_valid());
    }

    std::function<void(QString const&, QString const&)> on_accept_;
    std::function<void()> on_back_;
    QLineEdit* old_pw_edit_ = nullptr;
    colman::EncryptionPasswordWidget* pw_widget_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };


  /// Page for disabling encryption: asks for the current password.
  /**
     Calls on_accept_(password) on OK, or on_back_() on Back.
  */
  class RemoveEncryptionPage : public QWidget
  {
  public:
    explicit RemoveEncryptionPage(QWidget* parent = nullptr)
        : QWidget(parent)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

      QLabel* title = new QLabel(
          "<b>" + QApplication::tr("Disable Encryption") + "</b>", this);
      title->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(title);

      QLabel* info = new QLabel(
          QApplication::tr(
              "Enter the current password to remove encryption."),
          this);
      info->setWordWrap(true);
      info->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(info);

      QFormLayout* form = new QFormLayout;
      pw_edit_ = new QLineEdit(this);
      pw_edit_->setEchoMode(QLineEdit::Password);
      form->addRow(QApplication::tr("Current password:"), pw_edit_);
      vbox->addLayout(form);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("OK"), this);
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(pw_edit_, &QLineEdit::textChanged,
              [this](QString const& t) { ok_btn_->setEnabled(!t.isEmpty()); });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() {
                if(on_accept_) {
                  on_accept_(pw_edit_->text());
                }
              });
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
    }

  /// Sets the callback invoked with the current password on OK.
    void set_on_accept(std::function<void(QString const&)> on_accept)
    {
      on_accept_ = std::move(on_accept);
    }

  /// Sets the callback invoked when Back is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  private:
    std::function<void(QString const&)> on_accept_;
    std::function<void()> on_back_;
    QLineEdit* pw_edit_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };


  /// Combined settings page with two tabs: collection and program settings.
  /**
     A full-width back button sits at the top.  Below it a QTabWidget
     with "Collection Settings", "Program Settings", "Accessibility", and
     (admin-only) "Encryption" tabs.  The back button checks unsaved
     changes in each tab and offers to apply them before navigating to the
     central page.
  */
  class CombinedSettingsPage : public QWidget
  {
  public:
    /// Constructs the page.
    /**
       @param data    central application state
       @param on_back called when the Back button is pressed (after change check)
       @param parent  optional parent widget
    */
    explicit CombinedSettingsPage(colman::AppData& data,
                                  std::function<void()> on_back,
                                  QWidget* parent = nullptr)
        : QWidget(parent), data_(data), on_back_(std::move(on_back))
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      add_page_heading(vbox,
                       ":/icons/owls/preferences.jpeg",
                       QApplication::tr("Settings"),
                       this);

      tabs_ = new QTabWidget(this);
      tabs_->addTab(data.mw_.settings_page_,
                    QApplication::tr("Collection Settings"));
      tabs_->addTab(data.mw_.preferences_page_,
                    QApplication::tr("Program Settings"));
      tabs_->addTab(build_accessibility_tab(),
                    QApplication::tr("Accessibility"));
      tabs_->addTab(build_encryption_tab(),
                    QApplication::tr("Encryption"));
      vbox->addWidget(tabs_, 1);

      connect(back_btn_, &QPushButton::clicked,
              [this]() { on_back_click(); });
      connect(font_size_spin_,
              QOverload<int>::of(&QSpinBox::valueChanged),
              [](int pt) {
                QFont f = QApplication::font();
                f.setPointSize(pt);
                QApplication::setFont(f);
                QSettings settings;
                settings.setValue("font_size", pt);
              });
    }

    /// Fills tabs from their data sources; hides tabs the user cannot access.
    void enter(int tab = 0)
    {
      int const uid = data_.current_data_collector_;
      bool const is_admin = (uid == 0);
      bool const can_collection = is_admin || (data_.queries_ &&
                                               data_.queries_->has_permission(uid, "edit_preferences"));
      bool const can_program = is_admin || (data_.queries_ &&
                                            data_.queries_->has_permission(uid, "edit_program_settings"));
      tabs_->setTabVisible(0, can_collection);
      tabs_->setTabVisible(1, can_program);
      tabs_->setTabVisible(3, is_admin && data_.queries_ &&
                                  data_.queries_->supports_encryption());
      if(is_admin) {
        update_encryption_tab();
      }
      int const actual_tab = tabs_->isTabVisible(tab) ? tab : 2;
      tabs_->setCurrentIndex(actual_tab);
      if(can_collection) {
        colman::enter_settings_tab(data_);
      }
      if(can_program) {
        colman::enter_preferences_tab(data_);
      }
      {
        QSignalBlocker blk(font_size_spin_);
        font_size_spin_->setValue(QApplication::font().pointSize());
      }
    }

    /// Shows the collection-settings tab.
    void enter_settings() { enter(0); }
    /// Shows the program-settings tab.
    void enter_preferences() { enter(1); }

  private:
    /// Handles the back button: asks to apply unsaved changes.
    void on_back_click()
    {
      bool const s_changed =
          tabs_->isTabVisible(0) && colman::settings_tab_changed(data_);
      bool const p_changed =
          tabs_->isTabVisible(1) && colman::preferences_tab_changed(data_);
      if(s_changed || p_changed) {
        int const choice = colman::warning_yes_no_cancel(
            this,
            QApplication::tr("Collection Manager"),
            QApplication::tr("You have unsaved changes. "
                             "Do you want to apply them now?"));
        if(choice == QMessageBox::Cancel) {
          return;
        }
        if(choice == QMessageBox::Yes) {
          if(s_changed) {
            colman::apply_settings_tab(data_);
          }
          if(p_changed) {
            colman::apply_preferences_tab(data_);
          }
        }
      }
      on_back_();
    }

    /// Builds the Accessibility tab widget with font-size selector.
    QWidget* build_accessibility_tab()
    {
      QWidget* page = new QWidget(this);
      QVBoxLayout* vbox = new QVBoxLayout(page);
      vbox->setContentsMargins(12, 12, 12, 12);
      vbox->addStretch(1);

      QFormLayout* form = new QFormLayout;
      font_size_spin_ = new QSpinBox(page);
      font_size_spin_->setRange(8, 24);
      font_size_spin_->setSuffix(" pt");
      form->addRow(QApplication::tr("Font size:"), font_size_spin_);
      vbox->addLayout(form);

      QLabel* preview = new QLabel(
          QApplication::tr("The quick brown fox jumps over the lazy dog."),
          page);
      preview->setWordWrap(true);
      vbox->addWidget(preview);

      vbox->addStretch(1);
      return page;
    }

    /// Builds the Encryption tab widget.
    QWidget* build_encryption_tab()
    {
      QWidget* page = new QWidget(this);
      QVBoxLayout* vbox = new QVBoxLayout(page);
      vbox->setContentsMargins(12, 12, 12, 12);
      vbox->addStretch(1);

      QFormLayout* form = new QFormLayout;
      enc_status_label_ = new QLabel(page);
      form->addRow(QApplication::tr("Encryption status:"), enc_status_label_);
      vbox->addLayout(form);

      encrypt_btn_ = new QPushButton(
          QApplication::tr("Enable Encryption"), page);
      vbox->addWidget(encrypt_btn_);

      change_key_btn_ = new QPushButton(
          QApplication::tr("Change Password"), page);
      vbox->addWidget(change_key_btn_);

      remove_enc_btn_ = new QPushButton(
          QApplication::tr("Disable Encryption"), page);
      vbox->addWidget(remove_enc_btn_);

      vbox->addStretch(1);

      connect(encrypt_btn_, &QPushButton::clicked,
              [this]() { on_encrypt_click(); });
      connect(change_key_btn_, &QPushButton::clicked,
              [this]() { on_change_key_click(); });
      connect(remove_enc_btn_, &QPushButton::clicked,
              [this]() { on_remove_encryption_click(); });

      return page;
    }

    /// Refreshes the encryption tab to reflect the current encryption state.
    void update_encryption_tab()
    {
      if(!data_.queries_) {
        return;
      }
      bool const enc = data_.queries_->is_encrypted();
      enc_status_label_->setText(
          enc ? QApplication::tr("encrypted")
              : QApplication::tr("not encrypted"));
      encrypt_btn_->setVisible(!enc);
      change_key_btn_->setVisible(enc);
      remove_enc_btn_->setVisible(enc);
    }

    /// Handles "Enable Encryption" button.
    void on_encrypt_click()
    {
      QStackedWidget* stack = data_.mw_.pages_stack_;
      QWidget* const back = stack->currentWidget();
      auto* page = new NewEncryptionPage();
      stack->addWidget(page);
      auto go_back = [stack, page, back]() {
        stack->setCurrentWidget(back);
        stack->removeWidget(page);
        page->deleteLater();
      };
      page->set_on_back(go_back);
      page->set_on_accept([this, go_back](QString const& pw) {
        go_back();
        if(!data_.queries_->encrypt_database(pw)) {
          QMessageBox::warning(
              this,
              QApplication::tr("Collection Manager"),
              QApplication::tr("Encryption operation failed."));
          return;
        }
        QMessageBox::information(
            this,
            QApplication::tr("Collection Manager"),
            QApplication::tr("Encryption enabled successfully."));
        update_encryption_tab();
      });
      stack->setCurrentWidget(page);
    }

    /// Handles "Change Password" button.
    void on_change_key_click()
    {
      QStackedWidget* stack = data_.mw_.pages_stack_;
      QWidget* const back = stack->currentWidget();
      auto* page = new ChangeEncryptionKeyPage();
      stack->addWidget(page);
      auto go_back = [stack, page, back]() {
        stack->setCurrentWidget(back);
        stack->removeWidget(page);
        page->deleteLater();
      };
      page->set_on_back(go_back);
      page->set_on_accept(
          [this, go_back](QString const& old_pw, QString const& new_pw) {
            go_back();
            if(!data_.queries_->change_encryption_key(old_pw, new_pw)) {
              QMessageBox::warning(
                  this,
                  QApplication::tr("Collection Manager"),
                  QApplication::tr("Wrong password or "
                                   "encryption operation failed."));
              return;
            }
            QMessageBox::information(
                this,
                QApplication::tr("Collection Manager"),
                QApplication::tr("Password changed successfully."));
            update_encryption_tab();
          });
      stack->setCurrentWidget(page);
    }

    /// Handles "Disable Encryption" button.
    void on_remove_encryption_click()
    {
      QStackedWidget* stack = data_.mw_.pages_stack_;
      QWidget* const back = stack->currentWidget();
      auto* page = new RemoveEncryptionPage();
      stack->addWidget(page);
      auto go_back = [stack, page, back]() {
        stack->setCurrentWidget(back);
        stack->removeWidget(page);
        page->deleteLater();
      };
      page->set_on_back(go_back);
      page->set_on_accept([this, go_back](QString const& pw) {
        go_back();
        if(!data_.queries_->remove_encryption(pw)) {
          QMessageBox::warning(
              this,
              QApplication::tr("Collection Manager"),
              QApplication::tr("Wrong password or "
                               "encryption operation failed."));
          return;
        }
        QMessageBox::information(
            this,
            QApplication::tr("Collection Manager"),
            QApplication::tr("Encryption disabled successfully."));
        update_encryption_tab();
      });
      stack->setCurrentWidget(page);
    }

    colman::AppData& data_;
    std::function<void()> on_back_;
    QTabWidget* tabs_ = nullptr;
    QPushButton* back_btn_ = nullptr;
    QSpinBox* font_size_spin_ = nullptr;
    QLabel* enc_status_label_ = nullptr;
    QPushButton* encrypt_btn_ = nullptr;
    QPushButton* change_key_btn_ = nullptr;
    QPushButton* remove_enc_btn_ = nullptr;
  };


  /// Page showing all currently open lendings with filter and print support.
  /**
     Shows lendings ordered by due date (earliest first, no-date last).
     Admin and users with view_persons permission see combo-box filters for
     person and school class.  Users without that permission see only their
     own open lendings.  A print button (with larger icon) opens a preview.
     Double-clicking a row navigates to the return wizard.
  */
  class OpenLendingsPage : public QWidget
  {
  public:
    /// Constructs the page.
    /**
       @param data    central application state
       @param on_back called when the Back button is pressed
       @param parent  optional parent widget
    */
    explicit OpenLendingsPage(colman::AppData& data,
                              std::function<void()> on_back,
                              QWidget* parent = nullptr)
        : QWidget(parent), data_(data)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      QHBoxLayout* hdr_hbox = new QHBoxLayout;
      hdr_hbox->setContentsMargins(4, 4, 4, 4);
      QLabel* owl_lbl = new QLabel(this);
      QPixmap const pm(":/icons/owls/reserve2.jpeg");
      if(!pm.isNull()) {
        owl_lbl->setPixmap(pm.scaledToHeight(64, Qt::SmoothTransformation));
        owl_lbl->setFixedHeight(64);
      }
      hdr_hbox->addWidget(owl_lbl);
      QLabel* title_lbl = new colman::ScalingLabel(
          "<b>" + QApplication::tr("Open Lendings") + "</b>", this, 4);
      hdr_hbox->addWidget(title_lbl, 1);
      vbox->addLayout(hdr_hbox);

      // Filter bar — shown only for users with view_persons permission.
      filter_row_ = new QWidget(this);
      QHBoxLayout* filter_hbox = new QHBoxLayout(filter_row_);
      filter_hbox->setContentsMargins(4, 2, 4, 2);
      filter_hbox->addWidget(
          new QLabel(QApplication::tr("Person:"), filter_row_));
      person_combo_ = new QComboBox(filter_row_);
      person_combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      filter_hbox->addWidget(person_combo_, 1);
      filter_hbox->addWidget(
          new QLabel(QApplication::tr("Class:"), filter_row_));
      class_combo_ = new QComboBox(filter_row_);
      class_combo_->setObjectName("open_lendings_class_");
      class_combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      filter_hbox->addWidget(class_combo_, 1);
      vbox->addWidget(filter_row_);

      table_ = new QTableView(this);
      table_->setObjectName("open_lendings_table_");
      table_->setModel(&model_);
      table_->setSelectionBehavior(QAbstractItemView::SelectRows);
      // several rows can be selected to extend their due dates together
      table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
      table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
      table_->horizontalHeader()->setSectionResizeMode(
          QHeaderView::ResizeToContents);
      table_->horizontalHeader()->setStretchLastSection(true);
      table_->verticalHeader()->setVisible(false);
      table_->setColumnHidden(0, true);
      table_->setColumnHidden(6, true);
      table_->setColumnHidden(7, true);
      vbox->addWidget(table_, 1);

      QHBoxLayout* bottom_hbox = new QHBoxLayout;
      bottom_hbox->setContentsMargins(4, 4, 4, 4);
      extend_btn_ = new QPushButton(
          QApplication::tr("extend due date..."), this);
      extend_btn_->setObjectName("open_lendings_extend_");
      extend_btn_->setEnabled(false);
      bottom_hbox->addWidget(extend_btn_);
      bottom_hbox->addStretch(1);
      number_cb_ = new QCheckBox(
          QApplication::tr("Number entries"), this);
      number_cb_->setChecked(
          QSettings().value(
                         QStringLiteral("open_lendings/number_entries"),
                         false)
              .toBool());
      bottom_hbox->addWidget(number_cb_);
      print_btn_ = new QPushButton(
          QIcon(":/icons/owls/printer.jpeg"),
          QApplication::tr("Print"), this);
      print_btn_->setObjectName("open_lendings_print_");
      print_btn_->setIconSize(QSize(48, 48));
      print_btn_->setMinimumHeight(60);
      bottom_hbox->addWidget(print_btn_);
      vbox->addLayout(bottom_hbox);

      connect(back_btn, &QPushButton::clicked,
              [on_back]() { on_back(); });
      connect(person_combo_,
              QOverload<int>::of(&QComboBox::currentIndexChanged),
              [this](int) { refresh(); });
      connect(class_combo_,
              QOverload<int>::of(&QComboBox::currentIndexChanged),
              [this](int) { refresh(); });
      connect(table_, &QTableView::doubleClicked,
              [this](QModelIndex const& idx) {
                on_double_click(idx.row());
              });
      connect(print_btn_, &QPushButton::clicked,
              [this]() { do_print(); });
      connect(table_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [this]() {
                extend_btn_->setEnabled(
                    table_->selectionModel()->hasSelection());
              });
      connect(extend_btn_, &QPushButton::clicked,
              [this]() { extend_selected(); });
    }

    /// Refreshes the page for a new visit, adapting filter visibility.
    void enter()
    {
      int const uid = data_.current_data_collector_;
      bool const can_view =
          uid == 0 ||
          data_.queries_->has_permission(uid, "view_persons");
      filter_row_->setVisible(can_view);
      if(can_view) {
        restricted_person_id_ = -1;
        populate_combos();
      }
      else {
        restricted_person_id_ = uid;
      }
      extend_btn_->setVisible(colman::may_extend_lendings(data_));
      refresh();
    }

  private:
    /// Fills the person and class combo boxes from the database.
    void populate_combos()
    {
      auto const persons = data_.queries_->query_persons(-1, "");
      classless_person_ids_.clear();
      for(auto const& p : persons) {
        if(p.get_school_class().isEmpty()) {
          classless_person_ids_.insert(p.get_id());
        }
      }
      {
        QSignalBlocker blk(person_combo_);
        person_combo_->clear();
        person_combo_->addItem(
            QApplication::tr("all persons"), QVariant(-1));
        for(auto const& p : persons) {
          person_combo_->addItem(
              p.get_lastname() + ", " + p.get_firstname(),
              p.get_id());
        }
      }
      {
        QSignalBlocker blk(class_combo_);
        class_combo_->clear();
        // "all classes" carries invalid data, "no class" an empty string.
        class_combo_->addItem(QApplication::tr("all classes"), QVariant());
        class_combo_->addItem(QApplication::tr("no class"), QString(""));
        // Also offer classes used by persons but missing from the list.
        auto const names = colman::merge_class_names(
            data_.queries_->load_class_names(), persons);
        for(auto const& cls : names) {
          class_combo_->addItem(cls, cls);
        }
      }
    }

    /// Reloads the model applying the current filter state.
    void refresh()
    {
      int pid = -1;
      QVariant cls;
      if(restricted_person_id_ >= 0) {
        pid = restricted_person_id_;
      }
      else if(filter_row_->isVisible()) {
        pid = person_combo_->currentData().toInt();
        cls = class_combo_->currentData();
      }
      // An empty class name means "no filter" to query_due_lendings(), so
      // "no class" is applied here to keep the query (and REST) API as is.
      bool const no_class = cls.isValid() && cls.toString().isEmpty();
      auto rows = data_.queries_->query_due_lendings(
          QDate{}, QDate{}, pid, cls.toString(), QString{});
      if(no_class) {
        rows.erase(std::remove_if(
                       rows.begin(), rows.end(),
                       [this](colman::DueLendingListModel::Row const& row) {
                         return !classless_person_ids_.contains(row.person_id);
                       }),
                   rows.end());
      }
      model_.set_rows(std::move(rows));
    }

    /// Lets the user set a new due date for the selected lendings.
    /**
       The selected rows are already the user's choice, so the dialog only
       asks for the new date.  After a change the table is reloaded and the
       changed lendings stay selected.
    */
    void extend_selected()
    {
      std::vector<colman::DueLendingListModel::Row> rows;
      for(auto const& idx : table_->selectionModel()->selectedRows()) {
        rows.push_back(model_.row_at(idx.row()));
      }
      if(!colman::extend_selected_lendings(this, data_, rows)) {
        return;
      }
      refresh();
      auto* sel = table_->selectionModel();
      for(int r = 0; r < model_.rowCount(); ++r) {
        int const id = model_.row_at(r).lending_id;
        bool const was_selected =
            std::any_of(rows.cbegin(), rows.cend(),
                        [id](colman::DueLendingListModel::Row const& x) {
                          return x.lending_id == id;
                        });
        if(was_selected) {
          sel->select(model_.index(r, 0), QItemSelectionModel::Select |
                                              QItemSelectionModel::Rows);
        }
      }
    }

    /// Navigates to the return wizard for the lending at \a row.
    void on_double_click(int row)
    {
      colman::DueLendingListModel::Row const& r = model_.row_at(row);
      if(r.lending_id <= 0) {
        return;
      }
      colman::ActionInfo ai;
      ai.action_ = colman::Action::Return;
      ai.lending_id_ = r.lending_id;
      ai.item_id_ = r.item_id;
      ai.person_id_ = r.person_id;
      colman::enter_return_page_for_action(data_, ai);
    }

    /// Builds an HTML report from the current model and opens print preview.
    void do_print()
    {
      bool const numbered = number_cb_->isChecked();
      QSettings().setValue(
          QStringLiteral("open_lendings/number_entries"), numbered);

      QString html;
      html += "<html><head><meta charset='utf-8'/></head><body>";
      html += "<h2>" + QApplication::tr("Open Lendings") + "</h2>";
      if(filter_row_->isVisible()) {
        bool const person_filtered = person_combo_->currentIndex() > 0;
        bool const class_filtered = class_combo_->currentIndex() > 0;
        if(person_filtered || class_filtered) {
          html += "<p>";
          if(person_filtered) {
            html += QApplication::tr("Person: %1").arg(person_combo_->currentText().toHtmlEscaped());
          }
          if(class_filtered) {
            if(person_filtered) {
              html += " &nbsp; ";
            }
            html += QApplication::tr("Class: %1").arg(class_combo_->currentText().toHtmlEscaped());
          }
          html += "</p>";
        }
      }
      html +=
          "<table border='1' cellspacing='0' cellpadding='4'"
          " style='border-collapse:collapse;width:100%'>"
          "<tr>";
      if(numbered) {
        html += "<th>#</th>";
      }
      html +=
          "<th>" + QApplication::tr("due date") + "</th>"
                                                  "<th>" +
          QApplication::tr("borrower") + "</th>"
                                         "<th>" +
          QCoreApplication::tr("class") + "</th>"
                                          "<th>" +
          QApplication::tr("count") + "</th>"
                                      "<th>" +
          QApplication::tr("name") + "</th>"
                                     "</tr>";
      int const n = model_.rowCount();
      for(int i = 0; i < n; ++i) {
        colman::DueLendingListModel::Row const& r = model_.row_at(i);
        QString const due = r.duedate.isValid()
                                ? QLocale().toString(r.duedate, QLocale::ShortFormat)
                                : QString{};
        html += "<tr>";
        if(numbered) {
          html += "<td>" + QString::number(i + 1) + "</td>";
        }
        html +=
            "<td>" + due.toHtmlEscaped() + "</td>"
                                           "<td>" +
            r.person_name.toHtmlEscaped() + "</td>"
                                            "<td>" +
            r.school_class.toHtmlEscaped() + "</td>"
                                             "<td>" +
            QString::number(r.outstanding_count) + "</td>"
                                                   "<td>" +
            r.item_name.toHtmlEscaped() + "</td>"
                                          "</tr>";
      }
      html += "</table></body></html>";

      QTextDocument doc;
      doc.setHtml(html);
      QPrinter printer(QPrinter::HighResolution);
      printer.setPageMargins(
          QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
      QPrintPreviewDialog dialog(&printer, this);
      dialog.connect(
          &dialog, &QPrintPreviewDialog::paintRequested,
          [&doc](QPrinter* p) {
            doc.setPageSize(p->pageRect(QPrinter::Point).size());
            doc.print(p);
          });
      dialog.exec();
    }

    colman::AppData& data_;
    colman::DueLendingListModel model_;
    QTableView* table_ = nullptr;
    QWidget* filter_row_ = nullptr;
    QComboBox* person_combo_ = nullptr;
    QComboBox* class_combo_ = nullptr;
    QCheckBox* number_cb_ = nullptr;
    QPushButton* print_btn_ = nullptr;
    QPushButton* extend_btn_ = nullptr;
    int restricted_person_id_ = -1;
    QSet<int> classless_person_ids_;  ///< persons without school class
  };

/// Minimal event filter that calls fn() on QEvent::ApplicationFontChange.
  class FontChangeFilter : public QObject
  {
  public:
    explicit FontChangeFilter(std::function<void()> fn, QObject* parent)
        : QObject(parent), fn_(std::move(fn)) {}
    bool eventFilter(QObject*, QEvent* e) override
    {
      if(e->type() == QEvent::ApplicationFontChange) {
        fn_();
      }
      return false;
    }

  private:
    std::function<void()> fn_;
  };


/// Sets all buttons to the same size (the widest and highest sizeHint).
  void equalize_button_sizes(std::vector<QToolButton*> const& btns)
  {
    int max_w = 0;
    int max_h = 0;
    for(auto* btn : btns) {
      btn->ensurePolished();
      auto const hint = btn->sizeHint();
      max_w = std::max(max_w, hint.width());
      max_h = std::max(max_h, hint.height());
    }
    for(auto* btn : btns) {
      btn->setFixedSize(max_w, max_h);
    }
  }

} // namespace


/**
   Replaces the UI-generated central_page_ with a C++-created widget
   whose button visibility is adjusted per role each time the page is
   shown.  The navigation tiles are recreated and their pointers stored
   in @p data so that the individual page setup functions can connect to
   them.  The tiles are arranged in one row per task area (circulation
   desk, stock, persons and own account, setup); hidden tiles leave no
   gap and rows without visible tiles are hidden.  Two role-dependent
   tiles share one place and are shown/hidden mutually exclusively:
   "Change Passwords" (admin) and "Change Password" (regular user).
   The change-passwords page is also created and added to the stack.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_central_page(QApplication& app, colman::AppData& data)
{
  int const cp_idx =
      data.mw_.pages_stack_->indexOf(data.mw_.central_page_);
  auto dispatch = [&data](QString const& barcode) {
    data.mw_.central_page_->setEnabled(false);
    std::vector<colman::ActionInfo> actions =
        colman::possible_actions(*data.queries_, barcode);
    filter_actions(data, actions);
    dispatch_actions(data, actions);
    if(data.mw_.pages_stack_->currentWidget() ==
       data.mw_.central_page_) {
      data.mw_.central_page_->setEnabled(true);
    }
  };
  CentralPage* cp = new CentralPage(dispatch);
  QVBoxLayout* cp_vbox = new QVBoxLayout(cp);
  cp_vbox->addStretch(1);
  QHBoxLayout* cp_hbox = new QHBoxLayout;
  cp_hbox->addStretch(1);

  auto make_btn =
      [cp](QString const& text, QIcon const& icon) -> QToolButton* {
    QToolButton* btn = new colman::TileButton(cp);
    btn->setText(text);
    btn->setIcon(icon);
    btn->setIconSize(QSize(128, 128));
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    return btn;
  };

  data.lend_button_ =
      make_btn(QApplication::tr("lend item"),
               QIcon(":/icons/owls/lending.jpeg"));
  data.group_lend_button_ =
      make_btn(QApplication::tr("group lending"),
               QIcon(":/icons/owls/lending_group.jpeg"));
  data.group_lend_button_->setToolTip(QApplication::tr(
      "Lend one copy of an item to each member of a group, e.g. a class"));
  data.return_button_ =
      make_btn(QApplication::tr("return item"),
               QIcon(":/icons/owls/returns.jpeg"));
  data.history_button_ =
      make_btn(QApplication::tr("view history"),
               QIcon(":/icons/owls/list_history.jpeg"));
  data.new_person_button_ =
      make_btn(QApplication::tr("manage borrowers"),
               QIcon(":/icons/owls/edit_persons.jpeg"));

  data.open_lendings_button_ =
      make_btn(QApplication::tr("open lendings"),
               QIcon(":/icons/owls/waiting.jpeg"));
  data.acquisition_button_ =
      make_btn(QApplication::tr("acquire"),
               QIcon(":/icons/owls/add_book.jpeg"));
  data.liquidation_button_ =
      make_btn(QApplication::tr("liquidate"),
               QIcon(":/icons/owls/remove_book.jpeg"));
  data.search_button_ =
      make_btn(QApplication::tr("Search"),
               QIcon(":/icons/owls/search.jpeg"));
  data.class_management_button_ =
      make_btn(QApplication::tr("class management"),
               QIcon(":/icons/owls/evolution.jpeg"));

  data.scan_button_ =
      make_btn(QApplication::tr("Barcodes"),
               QIcon(":/icons/owls/film_barcode.jpeg"));

  data.print_id_button_ =
      make_btn(QApplication::tr("print IDs"),
               QIcon(":/icons/owls/print_id.jpeg"));
  data.print_barcode_button_ =
      make_btn(QApplication::tr("print labels"),
               QIcon(":/icons/owls/print_barcode.jpeg"));
  data.my_reservations_button_ =
      make_btn(QApplication::tr("My\nreservations"),
               QIcon(":/icons/owls/reserve2.jpeg"));
  data.print_signatures_button_ =
      make_btn(QApplication::tr("print signatures"),
               QIcon(":/icons/owls/print_signatures.jpeg"));
  data.calibrate_printer_button_ =
      make_btn(QApplication::tr("calibrate printer"),
               QIcon(":/icons/owls/printer_calibration.jpeg"));

  data.edit_id_button_ =
      make_btn(QApplication::tr("edit ID layout"),
               QIcon(":/icons/owls/edit_id.jpeg"));
  data.edit_item_card_button_ =
      make_btn(QApplication::tr("edit item card\nlayout"),
               QIcon(":/icons/owls/edit_item_card.jpeg"));
  data.edit_signature_layout_button_ =
      make_btn(QApplication::tr("edit signature\nlayout"),
               QIcon(":/icons/owls/edit_signature_layout.jpeg"));
  data.settings_button_ =
      make_btn(QApplication::tr("Settings"),
               QIcon(":/icons/owls/preferences.jpeg"));
  data.item_type_button_ =
      make_btn(QApplication::tr("manage categories"),
               QIcon(":/icons/owls/edit_types.jpeg"));

  // Admin-only: "Change Passwords".  User-only: "Change Password".
  // Both share one place in the "persons and own account" row; they are
  // mutually exclusive.
  QToolButton* change_passwords_btn =
      make_btn(QApplication::tr("Change Passwords"),
               QIcon(":/icons/owls/edit_passwords.jpeg"));
  change_passwords_btn->setObjectName("change_passwords_btn_");
  QToolButton* change_password_btn =
      make_btn(QApplication::tr("Change Password"),
               QIcon(":/icons/owls/change_password.jpeg"));

  QToolButton* logout_btn =
      make_btn(QApplication::tr("log out"),
               QIcon(":/icons/owls/logout.jpeg"));
  logout_btn->setObjectName("logout_btn_");

  if(QMediaDevices::videoInputs().isEmpty()) {
    data.scan_button_->setEnabled(false);
    data.scan_button_->setToolTip(
        QApplication::tr("No camera available"));
  }
  else {
    app.connect(data.scan_button_, &QToolButton::clicked,
                [&data, dispatch]() {
                  colman::scan_barcode(data, [dispatch](QString const& barcode) {
                    dispatch(barcode);
                  });
                });
  }

  // All tiles, including those hidden for the current user, get the width
  // of the widest and the height of the highest one, so that the columns
  // of the rows below stay aligned and one- and two-line labels give tiles
  // of equal height whatever the length of the (translated) labels.
  std::vector<QToolButton*> const eq_btns{
        data.lend_button_,
        data.group_lend_button_,
        data.return_button_,
        data.history_button_,
        data.new_person_button_,
        data.class_management_button_,
        data.acquisition_button_,
        data.liquidation_button_,
        data.search_button_,
        data.open_lendings_button_,
        data.my_reservations_button_,
        data.print_id_button_,
        data.print_barcode_button_,
        data.print_signatures_button_,
        data.calibrate_printer_button_,
        data.edit_id_button_,
        data.edit_item_card_button_,
        data.edit_signature_layout_button_,
        data.settings_button_,
        data.item_type_button_,
        data.scan_button_,
        change_passwords_btn,
        change_password_btn,
        logout_btn};
  equalize_button_sizes(eq_btns);
  cp->installEventFilter(new FontChangeFilter(
      [eq_btns]() { equalize_button_sizes(eq_btns); }, cp));

  // One row per task area, the most frequently used area first; within a
  // row the tiles follow the usual order of work.  Tiles hidden for the
  // logged-in user leave no gap: the visible ones move to the left, and
  // since all tiles share one size the columns stay aligned.
  std::vector<std::vector<QToolButton*>> const tile_rows{
      // circulation desk
      {data.lend_button_, data.group_lend_button_, data.return_button_,
       data.open_lendings_button_, data.my_reservations_button_,
       data.search_button_, data.scan_button_},
      // stock
      {data.acquisition_button_, data.liquidation_button_,
       data.print_barcode_button_, data.print_signatures_button_,
       data.history_button_},
      // persons and own account
      {data.new_person_button_, data.print_id_button_,
       data.class_management_button_, change_passwords_btn,
       change_password_btn, logout_btn},
      // setup
      {data.settings_button_, data.item_type_button_, data.edit_id_button_,
       data.edit_item_card_button_, data.edit_signature_layout_button_,
       data.calibrate_printer_button_}};
  QVBoxLayout* rows_vbox = new QVBoxLayout;
  std::vector<QWidget*> row_widgets;
  for(auto const& tiles : tile_rows) {
    QWidget* row_widget = new QWidget(cp);
    QHBoxLayout* row_hbox = new QHBoxLayout(row_widget);
    row_hbox->setContentsMargins(0, 0, 0, 0);
    for(QToolButton* tile : tiles) {
      row_hbox->addWidget(tile, 0, Qt::AlignTop);
    }
    // Rows keep their natural width and start at the left edge, so the
    // block of rows stays centred and the columns stay aligned.
    row_widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    rows_vbox->addWidget(row_widget, 0, Qt::AlignLeft);
    row_widgets.push_back(row_widget);
  }
  // Hides the rows whose tiles are all hidden for the logged-in user.
  auto const update_rows = [tile_rows, row_widgets]() {
    for(std::size_t i = 0; i < tile_rows.size(); ++i) {
      bool const any = std::any_of(
          tile_rows[i].begin(), tile_rows[i].end(),
          [](QToolButton const* tile) { return !tile->isHidden(); });
      row_widgets[i]->setVisible(any);
    }
  };
  cp_hbox->addLayout(rows_vbox);
  cp_hbox->addStretch(1);
  cp_vbox->addLayout(cp_hbox);
  cp_vbox->addStretch(1);

  // Replace the UI-generated central page.
  data.mw_.pages_stack_->removeWidget(data.mw_.central_page_);
  data.mw_.pages_stack_->insertWidget(cp_idx, cp);
  data.mw_.central_page_ = cp;

  // Disable the central page before any navigation action so that
  // button presses queued inside a blocking QEventLoop are ignored.
  // These connections are registered first, so they fire before each
  // button's actual action handler (even those connected in other files).
  {
    auto disable_cp = [&data]() {
      data.mw_.central_page_->setEnabled(false);
    };
    for(auto* btn : {
            static_cast<QAbstractButton*>(data.lend_button_),
            static_cast<QAbstractButton*>(data.group_lend_button_),
            static_cast<QAbstractButton*>(data.return_button_),
            static_cast<QAbstractButton*>(data.acquisition_button_),
            static_cast<QAbstractButton*>(data.liquidation_button_),
            static_cast<QAbstractButton*>(data.history_button_),
            static_cast<QAbstractButton*>(data.new_person_button_),
            static_cast<QAbstractButton*>(data.search_button_),
            static_cast<QAbstractButton*>(data.item_type_button_),
            static_cast<QAbstractButton*>(data.class_management_button_),
            static_cast<QAbstractButton*>(data.print_id_button_),
            static_cast<QAbstractButton*>(data.print_barcode_button_),
            static_cast<QAbstractButton*>(data.print_signatures_button_),
            static_cast<QAbstractButton*>(data.calibrate_printer_button_),
            static_cast<QAbstractButton*>(data.edit_id_button_),
            static_cast<QAbstractButton*>(data.edit_item_card_button_),
            static_cast<QAbstractButton*>(
                data.edit_signature_layout_button_),
            static_cast<QAbstractButton*>(data.settings_button_),
            static_cast<QAbstractButton*>(data.open_lendings_button_),
            static_cast<QAbstractButton*>(change_passwords_btn)}) {
      app.connect(btn, &QAbstractButton::clicked, disable_cp);
    }
  }

  // --- change passwords page ---
  ChangePasswordsPage* change_passwords_page =
      new ChangePasswordsPage(
          data,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(change_passwords_page);

  // --- printer calibration page ---
  PrinterCalibrationPage* cal_page =
      new PrinterCalibrationPage(
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(cal_page);

  // --- ID editor page ---
  IdEditorPage* id_editor_page =
      new IdEditorPage(
          *data.queries_,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(id_editor_page);

  // --- item card editor page ---
  ItemCardEditorPage* item_card_editor_page =
      new ItemCardEditorPage(*data.queries_, data);
  data.item_card_editor_page_ = item_card_editor_page;
  data.mw_.pages_stack_->addWidget(item_card_editor_page);

  // --- signature label editor page ---
  SignatureLabelEditorPage* signature_editor_page =
      new SignatureLabelEditorPage(
          *data.queries_,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(signature_editor_page);

  // Let close_collection() discard the lazily created editors so they
  // are rebuilt consistent with the next opened collection.
  data.set_clear_lazy_editors_function(
      [id_editor_page, item_card_editor_page, signature_editor_page]() {
        id_editor_page->clear_editor();
        item_card_editor_page->clear_editor();
        signature_editor_page->clear_editor();
      });

  // --- signature print page ---
  colman::SignaturePrintPage* signature_print_page =
      new colman::SignaturePrintPage(
          data,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(signature_print_page);

  // --- combined settings page ---
  CombinedSettingsPage* combined_settings_page =
      new CombinedSettingsPage(
          data,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  // settings_page_ and preferences_page_ are now parented to the tab widget;
  // remove them from the stack so Qt does not double-count them.
  data.mw_.pages_stack_->removeWidget(data.mw_.settings_page_);
  data.mw_.pages_stack_->removeWidget(data.mw_.preferences_page_);
  data.mw_.pages_stack_->addWidget(combined_settings_page);

  // --- open lendings page ---
  OpenLendingsPage* open_lendings_page =
      new OpenLendingsPage(
          data,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(open_lendings_page);

  // --- my reservations page ---
  ManageReservationsPage* my_reservations_page =
      new ManageReservationsPage(
          data,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(my_reservations_page);

  ChangePasswordPage* change_password_page =
      new ChangePasswordPage(
          data,
          [&data]() {
            data.mw_.pages_stack_->setCurrentWidget(
                data.mw_.central_page_);
          });
  data.mw_.pages_stack_->addWidget(change_password_page);

  // Adjust button visibility each time the central page is shown.
  // Admin sees all buttons; regular users see only those they have
  // permission for.
  app.connect(
      data.mw_.pages_stack_, &QStackedWidget::currentChanged,
      [&data, cp, change_passwords_btn, change_password_btn, eq_btns,
       update_rows](int) {
        if(data.mw_.pages_stack_->currentWidget() != cp) {
          return;
        }
        int const uid = data.current_data_collector_;
        bool const is_admin = (uid == 0);
        change_passwords_btn->setVisible(is_admin);
        change_password_btn->setVisible(!is_admin);
        if(is_admin) {
          data.new_person_button_->setVisible(true);
          data.item_type_button_->setVisible(true);
          data.settings_button_->setVisible(true);
          data.acquisition_button_->setVisible(true);
          data.lend_button_->setVisible(true);
          data.lend_button_->setText(QApplication::tr("lend item"));
          data.group_lend_button_->setVisible(true);
          data.return_button_->setVisible(true);
          data.liquidation_button_->setVisible(true);
          data.search_button_->setVisible(true);
        // data.person_view_button_->setVisible(true);
          data.print_id_button_->setVisible(true);
          data.print_barcode_button_->setVisible(true);
          data.history_button_->setVisible(true);
          data.edit_id_button_->setVisible(true);
          data.edit_item_card_button_->setVisible(true);
          data.edit_signature_layout_button_->setVisible(true);
          data.print_signatures_button_->setVisible(true);
          data.calibrate_printer_button_->setVisible(true);
          data.open_lendings_button_->setVisible(true);
          data.class_management_button_->setVisible(true);
          data.my_reservations_button_->setVisible(false);
        }
        else {
          // Fetch all permissions in two requests instead of two
          // requests per has() call (~34 calls previously).
          auto const perm_cols = data.queries_->get_permissions();
          auto const perm_vals =
              data.queries_->get_user_permissions(uid);
          auto has = [&](QString const& col) -> bool {
            for(std::size_t i = 0;
                i < perm_cols.size() && i < perm_vals.size(); ++i) {
              if(perm_cols[i] == col) {
                return perm_vals[i];
              }
            }
            return false;
          };
          data.new_person_button_->setVisible(
              has("edit_persons") || has("create_lenders") ||
              has("create_data_collectors"));
          data.item_type_button_->setVisible(has("edit_categories"));
          data.settings_button_->setVisible(true);
          data.acquisition_button_->setVisible(has("collect_acquisitions"));
          data.lend_button_->setVisible(
              has("collect_lendings") || has("reserve"));
          data.lend_button_->setText(
              has("collect_lendings")
                  ? QApplication::tr("lend item")
                  : QApplication::tr("reserve item"));
          data.group_lend_button_->setVisible(has("collect_lendings"));
          data.return_button_->setVisible(has("collect_returns"));
          data.liquidation_button_->setVisible(has("collect_liquidations"));
          data.search_button_->setVisible(has("view_items"));
        // data.person_view_button_->setVisible(has("view_persons"));
          data.print_id_button_->setVisible(has("print_ids"));
          data.print_barcode_button_->setVisible(has("print_barcodes"));
          data.history_button_->setVisible(has("view_history"));
          data.edit_id_button_->setVisible(has("edit_id_layout"));
          data.edit_item_card_button_->setVisible(has("edit_id_layout"));
          data.edit_signature_layout_button_->setVisible(
              has("edit_id_layout"));
          data.print_signatures_button_->setVisible(
              has("print_signatures"));
          data.calibrate_printer_button_->setVisible(false);
          data.open_lendings_button_->setVisible(true);
          data.class_management_button_->setVisible(has("manage_classes"));
          data.my_reservations_button_->setVisible(has("reserve"));
        }
        // The lend tile's label depends on the permissions.
        equalize_button_sizes(eq_btns);
        update_rows();
        cp->setEnabled(true);
        cp->setFocus();
      });

  app.connect(change_passwords_btn, &QToolButton::clicked,
              [&data, change_passwords_page]() {
                change_passwords_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(
                    change_passwords_page);
              });
  app.connect(change_password_btn, &QToolButton::clicked,
              [&data, change_password_page]() {
                change_password_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(change_password_page);
              });
  app.connect(data.edit_id_button_, &QToolButton::clicked,
              [&data, id_editor_page]() {
                data.mw_.pages_stack_->setCurrentWidget(id_editor_page);
              });
  app.connect(data.edit_item_card_button_, &QToolButton::clicked,
              [&data, item_card_editor_page]() {
                data.mw_.pages_stack_->setCurrentWidget(
                    item_card_editor_page);
              });
  app.connect(data.edit_signature_layout_button_, &QToolButton::clicked,
              [&data, signature_editor_page]() {
                data.mw_.pages_stack_->setCurrentWidget(
                    signature_editor_page);
              });
  app.connect(data.print_signatures_button_, &QToolButton::clicked,
              [&data, signature_print_page]() {
                signature_print_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(
                    signature_print_page);
              });
  app.connect(data.calibrate_printer_button_, &QToolButton::clicked,
              [&data, cal_page]() {
                data.mw_.pages_stack_->setCurrentWidget(cal_page);
              });
  app.connect(logout_btn, &QToolButton::clicked,
              [&data]() {
                int const answer = colman::question_yes_no(
                    &data.w_,
                    QApplication::tr("log out"),
                    QApplication::tr("Really log out?"));
                if(answer == QMessageBox::Yes) {
                  colman::logout(data);
                }
              });
  app.connect(data.settings_button_, &QToolButton::clicked,
              [&data, combined_settings_page]() {
                combined_settings_page->enter_settings();
                data.mw_.pages_stack_->setCurrentWidget(
                    combined_settings_page);
              });
  app.connect(data.open_lendings_button_, &QToolButton::clicked,
              [&data, open_lendings_page]() {
                open_lendings_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(
                    open_lendings_page);
              });
  app.connect(data.my_reservations_button_, &QToolButton::clicked,
              [&data, my_reservations_page]() {
                my_reservations_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(
                    my_reservations_page);
              });
}

/* EOF */
