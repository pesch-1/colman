// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of the create-collection step flow.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_create_collection_page.hpp"
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef COLMAN_ENCRYPTION_PASSWORD_WIDGET_HPP
#  include "colman_encryption_password_widget.hpp"
#endif
#ifndef COLMAN_PREVIEW_FILE_DIALOG_HPP
#  include "colman_preview_file_dialog.hpp"
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QFILEDIALOG
#  include <QFileDialog>
#  define SYSTEM_QFILEDIALOG
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QFRAME
#  include <QFrame>
#  define SYSTEM_QFRAME
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QSQLDATABASE
#  include <QSqlDatabase>
#  define SYSTEM_QSQLDATABASE
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif

namespace
{
  /// Transient multi-step flow for creating a new collection.
  /**
     Builds its step pages on the main window's page stack, drives navigation
     (top Back button, bottom Next/Finish button gated on per-step validity),
     performs the database creation on Finish, and removes itself afterwards.
  */
  class CreateCollectionFlow : public QObject
  {
  public:
    CreateCollectionFlow(colman::AppData& data,
                         std::function<void(QString const&)> on_created)
        : QObject(&data.w_),
          data_(data),
          on_created_(std::move(on_created))
    {
      return_page_ = data_.mw_.pages_stack_->currentWidget();
      // The cipher driver name mirrors CipherDriver in colman_sql_queries.cpp.
      // Checked statically here because supports_encryption() only reports the
      // driver of an *open* connection, and no collection is open yet; the
      // value matches what supports_encryption() returns once the new database
      // has been created (it opens with the same preferred driver).
      has_encryption_ =
          QSqlDatabase::isDriverAvailable("SQLITE_MULTIPLE_CIPHERS");
      build_pages();
    }

    /// Resets all fields and shows the first step.
    void start()
    {
      reset_fields();
      current_ = 0;
      show_step();
    }

  private:
    /// Wraps \a content in a step page (top Back, content, bottom Next/Finish).
    void add_step(QWidget* content, std::function<bool()> validator,
                  bool is_last)
    {
      auto* page = new QWidget;
      auto* vbox = new QVBoxLayout(page);

      auto* back_btn = new QPushButton(QApplication::tr("back"), page);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);
      connect(back_btn, &QPushButton::clicked,
              [this]() { go_back(); });

      vbox->addSpacing(24);
      auto* logo = new QLabel(page);
      QPixmap const pm(":/icons/owls/colman_logo.jpeg");
      if(!pm.isNull()) {
        logo->setPixmap(pm.scaledToHeight(200, Qt::SmoothTransformation));
        logo->setAlignment(Qt::AlignHCenter);
      }
      vbox->addWidget(logo);
      vbox->addSpacing(24);

      vbox->addWidget(content, 1);

      auto* btn_hbox = new QHBoxLayout;
      auto* next_btn = new QPushButton(
          is_last ? QApplication::tr("Finish") : QApplication::tr("Next"), page);
      next_btn->setObjectName("create_collection_next_");
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(next_btn);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      if(is_last) {
        connect(next_btn, &QPushButton::clicked, [this]() { finish(); });
      }
      else {
        connect(next_btn, &QPushButton::clicked, [this]() { go_next(); });
      }

      data_.mw_.pages_stack_->addWidget(page);
      steps_.push_back(page);
      next_btns_.push_back(next_btn);
      validators_.push_back(std::move(validator));
    }

    /// Builds all step pages and wires per-step validity updates.
    void build_pages()
    {
      // The file step comes last so that the create/overwrite check runs on
      // that step: if the user declines overwriting, they stay where they can
      // change the file name.  Encryption and admin steps only collect input;
      // the database is created, encrypted and given its admin password in
      // finish() (in that order) once the file name is confirmed.
      build_name_step();
      build_logo_step();
      build_lending_step();
      build_school_step();
      if(has_encryption_) {
        build_encryption_step();
      }
      build_admin_step();
      build_file_step();
    }

    void build_name_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("Create a new collection") + "</b>", content);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr(
              "Please specify a name for your collection, the organization "
              "and a responsible person."),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      auto* form = new QFormLayout;
      name_edit_ = new QLineEdit(content);
      org_edit_ = new QLineEdit(content);
      resp_edit_ = new QLineEdit(content);
      name_edit_->setObjectName("create_collection_name_");
      org_edit_->setObjectName("create_collection_organization_");
      resp_edit_->setObjectName("create_collection_responsible_");
      form->addRow(QApplication::tr("collection name:"), name_edit_);
      form->addRow(QApplication::tr("organization:"), org_edit_);
      form->addRow(QApplication::tr("responsible:"), resp_edit_);
      vbox->addLayout(form);
      vbox->addStretch(1);

      connect(name_edit_, &QLineEdit::textChanged, [this]() { update_nav(); });
      connect(org_edit_, &QLineEdit::textChanged, [this]() { update_nav(); });
      connect(name_edit_, &QLineEdit::returnPressed,
              [this]() { advance_focus(org_edit_); });
      connect(org_edit_, &QLineEdit::returnPressed,
              [this]() { advance_focus(resp_edit_); });
      connect(resp_edit_, &QLineEdit::returnPressed,
              [this]() { advance_focus(nullptr); });

      add_step(content,
               [this]() {
                 return !name_edit_->text().isEmpty() &&
                        !org_edit_->text().isEmpty();
               },
               false);
    }

    void build_logo_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("Collection Logo") + "</b>", content);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr(
              "Do you want to use a logo? It will be printed on lending "
              "cards."),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      auto* row = new QHBoxLayout;
      logo_preview_ = new QLabel(content);
      logo_preview_->setMinimumSize(100, 150);
      logo_preview_->setFrameShape(QFrame::StyledPanel);
      logo_preview_->setFrameShadow(QFrame::Sunken);
      logo_preview_->setScaledContents(true);
      row->addWidget(logo_preview_);

      auto* right = new QVBoxLayout;
      auto* lbl = new QLabel(QApplication::tr("logo file:"), content);
      lbl->setAlignment(Qt::AlignCenter);
      right->addWidget(lbl);
      logo_edit_ = new QLineEdit(content);
      logo_edit_->setObjectName("create_collection_logo_");
      right->addWidget(logo_edit_);
      auto* browse = new QPushButton(QApplication::tr("browse"), content);
      right->addWidget(browse);
      right->addStretch(1);
      row->addLayout(right);
      vbox->addLayout(row);
      vbox->addStretch(1);

      connect(browse, &QPushButton::clicked, [this]() { browse_logo(); });
      connect(logo_edit_, &QLineEdit::textChanged, [this]() { update_nav(); });
      connect(logo_edit_, &QLineEdit::editingFinished,
              [this]() {
                if(logo_edit_->isModified()) {
                  logo_edit_->setModified(false);
                  display_logo(logo_edit_->text());
                }
              });
      connect(logo_edit_, &QLineEdit::returnPressed,
              [this]() { advance_focus(nullptr); });

      add_step(content,
               [this]() {
                 if(logo_edit_->text().isEmpty()) {
                   return true;
                 }
                 QPixmap pm;
                 return pm.load(logo_edit_->text());
               },
               false);
    }

    void build_lending_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("Lending Period") + "</b>", content);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr("Do you want to specify a default lending period?"),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      use_period_ = new QCheckBox(
          QApplication::tr("use default lending period"), content);
      use_period_->setObjectName("create_collection_use_period_");
      vbox->addWidget(use_period_);

      auto* period_row = new QHBoxLayout;
      years_ = new QSpinBox(content);
      years_->setSuffix(QApplication::tr(" year(s)"));
      months_ = new QSpinBox(content);
      months_->setObjectName("create_collection_months_");
      months_->setSuffix(QApplication::tr(" month(s)"));
      days_ = new QSpinBox(content);
      days_->setSuffix(QApplication::tr(" day(s)"));
      days_->setMaximum(999);
      period_row->addWidget(years_);
      period_row->addWidget(months_);
      period_row->addWidget(days_);
      vbox->addLayout(period_row);

      allow_res_ = new QCheckBox(
          QApplication::tr("Allow reservations"), content);
      vbox->addWidget(allow_res_);

      auto* res_row = new QHBoxLayout;
      res_days_ = new QSpinBox(content);
      res_days_->setSuffix(QApplication::tr(" day(s)"));
      res_days_->setMinimum(1);
      res_days_->setMaximum(999);
      res_row->addWidget(res_days_);
      res_row->addStretch(1);
      vbox->addLayout(res_row);
      vbox->addStretch(1);

      connect(use_period_, &QCheckBox::toggled,
              [this](bool on) {
                years_->setEnabled(on);
                months_->setEnabled(on);
                days_->setEnabled(on);
              });
      connect(allow_res_, &QCheckBox::toggled,
              [this](bool on) { res_days_->setEnabled(on); });

      add_step(content, []() { return true; }, false);
    }

    void build_school_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("School item support") + "</b>", content);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr(
              "Are most of the items school items? School items will be "
              "assigned a subject, an academic year and a type like workbook "
              "or teacher's manual."),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      school_ = new QCheckBox(
          QApplication::tr("default item type is school item"), content);
      school_->setObjectName("create_collection_school_");
      vbox->addWidget(school_);
      vbox->addStretch(1);

      add_step(content, []() { return true; }, false);
    }

    void build_file_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("Collection file") + "</b>", content);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr(
              "Please specify a filename to store your collection data "
              "into:"),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      file_edit_ = new QLineEdit(content);
      file_edit_->setObjectName("create_collection_file_");
      vbox->addWidget(file_edit_);

      auto* row = new QHBoxLayout;
      row->addStretch(1);
      auto* browse = new QPushButton(QApplication::tr("browse"), content);
      row->addWidget(browse);
      vbox->addLayout(row);
      vbox->addStretch(1);

      connect(browse, &QPushButton::clicked, [this]() { browse_file(); });
      connect(file_edit_, &QLineEdit::textChanged, [this]() { update_nav(); });
      connect(file_edit_, &QLineEdit::returnPressed,
              [this]() { advance_focus(nullptr); });

      add_step(content, [this]() { return file_path_valid(); }, true);
    }

    void build_encryption_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("Encrypt Collection") + "</b>", content);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr(
              "You can protect this collection with a password.\n"
              "An encrypted collection cannot be read without the password."),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      encrypt_check_ = new QCheckBox(
          QApplication::tr("Encrypt this collection"), content);
      encrypt_check_->setObjectName("create_collection_encrypt_");
      vbox->addWidget(encrypt_check_);

      enc_pw_ = new colman::EncryptionPasswordWidget(content);
      enc_pw_->setObjectName("create_collection_encryption_password_");
      enc_pw_->setEnabled(false);
      vbox->addWidget(enc_pw_);
      vbox->addStretch(1);

      connect(encrypt_check_, &QCheckBox::toggled,
              [this](bool on) {
                enc_pw_->setEnabled(on);
                update_nav();
              });
      connect(enc_pw_, &colman::EncryptionPasswordWidget::changed,
              [this]() { update_nav(); });

      add_step(content,
               [this]() {
                 return !encrypt_check_->isChecked() || enc_pw_->is_valid();
               },
               false);
    }

    void build_admin_step()
    {
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      auto* title = new QLabel(
          "<b>" + QApplication::tr("Set Administrator Password") + "</b>",
          content);
      title->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(title);
      auto* hint = new QLabel(
          QApplication::tr(
              "Please set a password for the Administrator account."),
          content);
      hint->setWordWrap(true);
      vbox->addWidget(hint);

      auto* form = new QFormLayout;
      admin_pw_ = new QLineEdit(content);
      admin_pw_->setEchoMode(QLineEdit::Password);
      admin_confirm_ = new QLineEdit(content);
      admin_pw_->setObjectName("create_collection_admin_password_");
      admin_confirm_->setObjectName("create_collection_admin_confirm_");
      admin_confirm_->setEchoMode(QLineEdit::Password);
      form->addRow(QApplication::tr("Password:"), admin_pw_);
      form->addRow(QApplication::tr("Confirm:"), admin_confirm_);
      vbox->addLayout(form);
      vbox->addStretch(1);

      connect(admin_pw_, &QLineEdit::textChanged, [this]() { update_nav(); });
      connect(admin_confirm_, &QLineEdit::textChanged,
              [this]() { update_nav(); });
      connect(admin_pw_, &QLineEdit::returnPressed,
              [this]() { advance_focus(admin_confirm_); });
      connect(admin_confirm_, &QLineEdit::returnPressed,
              [this]() { advance_focus(nullptr); });

      add_step(content,
               [this]() {
                 return !admin_pw_->text().isEmpty() &&
                        admin_pw_->text() == admin_confirm_->text();
               },
               false);
    }

    /// Returns true when the collection file path is usable.
    bool file_path_valid() const
    {
      if(file_edit_->text().isEmpty()) {
        return false;
      }
      QFileInfo info(file_edit_->text());
      return info.dir().exists() && !(info.exists() && !info.isWritable());
    }

    void reset_fields()
    {
      name_edit_->clear();
      org_edit_->clear();
      resp_edit_->clear();
      logo_edit_->clear();
      logo_preview_->clear();
      use_period_->setChecked(false);
      years_->setValue(0);
      months_->setValue(1);
      days_->setValue(0);
      years_->setEnabled(false);
      months_->setEnabled(false);
      days_->setEnabled(false);
      allow_res_->setChecked(true);
      res_days_->setValue(14);
      res_days_->setEnabled(true);
      school_->setChecked(false);
      file_edit_->clear();
      if(has_encryption_) {
        encrypt_check_->setChecked(false);
        enc_pw_->clear();
        enc_pw_->setEnabled(false);
      }
      admin_pw_->clear();
      admin_confirm_->clear();
    }

    void show_step()
    {
      data_.mw_.pages_stack_->setCurrentWidget(steps_[current_]);
      update_nav();
    }

    void update_nav()
    {
      next_btns_[current_]->setEnabled(validators_[current_]());
    }

    /// Moves focus to \a next_field on Return, or activates the current step's
    /// Next/Finish button when there is no further field on the step.
    void advance_focus(QWidget* next_field)
    {
      if(next_field) {
        next_field->setFocus();
      }
      else if(next_btns_[current_]->isEnabled()) {
        next_btns_[current_]->click();
      }
    }

    void go_next()
    {
      if(current_ + 1 < static_cast<int>(steps_.size())) {
        ++current_;
        show_step();
      }
    }

    void go_back()
    {
      if(current_ == 0) {
        cancel();
      }
      else {
        --current_;
        show_step();
      }
    }

    void browse_logo()
    {
      QSettings settings;
      QByteArray state = settings.value("open/logo").toByteArray();
      colman::PreviewFileDialog diag(
          nullptr, QApplication::tr("Open Logo"), "");
      QStringList filters;
      filters << QApplication::tr("image files (*.svg *.png *.xpm *.jpg)")
              << QApplication::tr("any files (*)");
      diag.setNameFilters(filters);
      diag.setFileMode(QFileDialog::ExistingFile);
      (void)diag.restoreState(state);
      if(diag.exec()) {
        QStringList file_names = diag.selectedFiles();
        settings.setValue("open/logo", diag.saveState());
        if(!file_names.empty()) {
          logo_edit_->setText(file_names.front());
          display_logo(file_names.front());
          update_nav();
        }
      }
    }

    void display_logo(QString const& fname)
    {
      if(fname.isEmpty()) {
        return;
      }
      QPixmap pm;
      if(pm.load(fname)) {
        logo_preview_->setPixmap(pm);
      }
      else {
        QMessageBox::warning(steps_[current_],
                             QApplication::tr("Collection Manager"),
                             QApplication::tr("Could not read logo."));
      }
    }

    void browse_file()
    {
      QString fname = QFileDialog::getSaveFileName(
          steps_[current_], QApplication::tr("Save File"), "untitled.col",
          QApplication::tr("collection files (*.col *.db);;any files (*)"),
          nullptr, QFileDialog::DontConfirmOverwrite);
      if(!fname.isEmpty()) {
        if(QFileInfo(fname).suffix().isEmpty()) {
          fname += ".col";
        }
        file_edit_->setText(fname);
      }
    }

    void finish()
    {
      colman::Collection collection;
      collection.set_name(name_edit_->text());
      collection.set_organization(org_edit_->text());
      collection.set_responsible(resp_edit_->text());
      collection.set_use_lending_period(use_period_->isChecked());
      collection.set_lending_period_years(years_->value());
      collection.set_lending_period_months(months_->value());
      collection.set_lending_period_days(days_->value());
      collection.set_allow_reservations(allow_res_->isChecked());
      collection.set_reservation_period_days(res_days_->value());
      QString const lfname = logo_edit_->text();
      if(!lfname.isEmpty()) {
        QFile lf(lfname);
        if(lf.open(QIODevice::ReadOnly)) {
          collection.set_logo_raw(lf.readAll());
        }
      }
      collection.set_default_is_school_item(school_->isChecked());

      QString const fname = file_edit_->text();
      QFileInfo info(fname);
      if(info.exists()) {
        if(colman::warning_yes_no(
               steps_[current_],
               QApplication::tr("Collection Manager"),
               QApplication::tr("file already exists! "
                                "do you want to overwrite it?")) !=
           QMessageBox::Yes) {
          return;
        }
        QFile(fname).remove();
      }
      if(!data_.queries_->create_new_database(collection, fname)) {
        QMessageBox::warning(steps_[current_],
                             QApplication::tr("Collection Manager"),
                             QApplication::tr("could not create database!"));
        return;
      }
      if(has_encryption_ && encrypt_check_->isChecked()) {
        if(!data_.queries_->encrypt_database(enc_pw_->password())) {
          QMessageBox::warning(
              steps_[current_],
              QApplication::tr("Collection Manager"),
              QApplication::tr("Encryption operation failed."));
          return;
        }
      }
      QString const hash = colman::hash_password(admin_pw_->text());
      if(hash.isEmpty() ||
         !data_.queries_->set_local_credentials(0, "admin", hash)) {
        QMessageBox::warning(
            steps_[current_],
            QApplication::tr("Collection Manager"),
            QApplication::tr("could not set administrator password!"));
        return;
      }

      auto cb = on_created_;
      cleanup();
      if(cb) {
        cb(fname);
      }
    }

    void cancel()
    {
      data_.mw_.pages_stack_->setCurrentWidget(return_page_);
      cleanup();
    }

    /// Removes the step pages from the stack and schedules deletion.
    void cleanup()
    {
      for(QWidget* page : steps_) {
        data_.mw_.pages_stack_->removeWidget(page);
        page->deleteLater();
      }
      steps_.clear();
      deleteLater();
    }

    colman::AppData& data_;
    std::function<void(QString const&)> on_created_;
    QWidget* return_page_ = nullptr;
    bool has_encryption_ = false;
    int current_ = 0;

    std::vector<QWidget*> steps_;
    std::vector<QPushButton*> next_btns_;
    std::vector<std::function<bool()>> validators_;

    QLineEdit* name_edit_ = nullptr;
    QLineEdit* org_edit_ = nullptr;
    QLineEdit* resp_edit_ = nullptr;
    QLineEdit* logo_edit_ = nullptr;
    QLabel* logo_preview_ = nullptr;
    QCheckBox* use_period_ = nullptr;
    QSpinBox* years_ = nullptr;
    QSpinBox* months_ = nullptr;
    QSpinBox* days_ = nullptr;
    QCheckBox* allow_res_ = nullptr;
    QSpinBox* res_days_ = nullptr;
    QCheckBox* school_ = nullptr;
    QLineEdit* file_edit_ = nullptr;
    QCheckBox* encrypt_check_ = nullptr;
    colman::EncryptionPasswordWidget* enc_pw_ = nullptr;
    QLineEdit* admin_pw_ = nullptr;
    QLineEdit* admin_confirm_ = nullptr;
  };

} // namespace

void colman::enter_create_collection_flow(
    colman::AppData& data, std::function<void(QString const&)> on_created)
{
  auto* flow = new CreateCollectionFlow(data, std::move(on_created));
  flow->start();
}

/* EOF */
