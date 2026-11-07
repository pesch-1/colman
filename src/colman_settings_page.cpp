// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// settings page setup function
/**
   @file
   @author Christoph Pesch
*/

#include "colman_settings_page.hpp"
#ifndef COLMAN_PIXMAP_LABEL_HPP
#  include "colman_pixmap_label.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_PREVIEW_FILE_DIALOG_HPP
#  include "colman_preview_file_dialog.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QINPUTDIALOG
#  include <QInputDialog>
#  define SYSTEM_QINPUTDIALOG
#endif

namespace
{
  /**
     Reads the collection record from the database and populates all settings
     page widgets (name, organization, responsible, lending period, logo,
     default type and school-item flag).
     @param data central application state.
  */
  void fill_widgets(colman::AppData& data)
  {
    colman::Collection const col = data.queries_->read_collection();
    data.mw_.settings_collection_name_->setText(col.get_name());
    data.mw_.settings_organization_->setText(col.get_organization());
    data.mw_.settings_responsible_->setText(col.get_responsible());
    data.mw_.settings_use_lending_period_->setChecked(col.get_use_lending_period());
    data.mw_.settings_lending_period_years_->setValue(col.get_lending_period_years());
    data.mw_.settings_lending_period_months_
        ->setValue(col.get_lending_period_months());
    data.mw_.settings_lending_period_days_->setValue(col.get_lending_period_days());
    data.mw_.settings_lending_period_years_->setEnabled(col.get_use_lending_period());
    data.mw_.settings_lending_period_months_
        ->setEnabled(col.get_use_lending_period());
    data.mw_.settings_lending_period_days_->setEnabled(col.get_use_lending_period());
    data.mw_.settings_allow_reservations_->setChecked(col.get_allow_reservations());
    data.mw_.settings_reservation_period_days_
        ->setValue(col.get_reservation_period_days());
    data.mw_.settings_reservation_period_days_
        ->setEnabled(col.get_allow_reservations());
    data.mw_.settings_logo_->setText("");
    data.mw_.settings_logo_pixmap_->setText(" ");
    data.mw_.settings_logo_pixmap_->setText("");
    data.mw_.settings_logo_pixmap_->setProperty("logo_raw", QByteArray());
    if(!col.get_logo_raw().isEmpty()) {
      QPixmap logo;
      logo.loadFromData(col.get_logo_raw());
      data.mw_.settings_use_logo_->setChecked(true);
      data.mw_.settings_logo_pixmap_->set_original_pixmap(logo);
    }
    else {
      data.mw_.settings_use_logo_->setChecked(false);
      data.mw_.settings_logo_pixmap_->setText(" ");
      data.mw_.settings_logo_pixmap_->setText("");
    }
    std::vector<std::pair<int, QString>> const
        type_locale = data.queries_->get_type_locale();
    data.mw_.settings_default_type_->clear();
    int found_idx = -1;
    for(auto const& type : type_locale) {
      data.mw_.settings_default_type_->addItem(type.second, type.first);
      if(type.first == col.get_default_item_type()) {
        found_idx = data.mw_.settings_default_type_->count() - 1;
      }
    }
    colman::AutoBlockSignals b(data.mw_.settings_default_type_);
    data.mw_.settings_default_type_->setCurrentIndex(
        found_idx >= 0 ? found_idx : 0);
    data.mw_.settings_default_is_schoolitem_
        ->setChecked(col.get_default_is_school_item());
    data.mw_.settings_sig_top_categories_->clear();
    for(QString const& cat :
        data.queries_->read_signature_top_categories()) {
      data.mw_.settings_sig_top_categories_->addItem(cat);
    }
    data.mw_.settings_sig_bottom_categories_->clear();
  }


  /**
     Reads all settings page widgets, builds a Collection object, and writes
     it to the database.  Writes a backup first; shows a warning on failure.
     @param data central application state.
  */
  void widgets_to_settings(colman::AppData& data)
  {
    colman::Collection col = data.queries_->read_collection();
    col.set_name(data.mw_.settings_collection_name_->text());
    col.set_organization(data.mw_.settings_organization_->text());
    col.set_responsible(data.mw_.settings_responsible_->text());
    col.set_use_lending_period(data.mw_.settings_use_lending_period_->isChecked());
    col.set_lending_period_years(data.mw_.settings_lending_period_years_->value());
    col.set_lending_period_months(data.mw_.settings_lending_period_months_->value());
    col.set_lending_period_days(data.mw_.settings_lending_period_days_->value());
    col.set_allow_reservations(data.mw_.settings_allow_reservations_->isChecked());
    col.set_reservation_period_days(data.mw_.settings_reservation_period_days_->value());
    if(data.mw_.settings_use_logo_->isChecked()) {
      QByteArray const new_raw = data.mw_.settings_logo_pixmap_->property("logo_raw").toByteArray();
      if(!new_raw.isEmpty()) {
        col.set_logo_raw(new_raw);
      }
      // else: col already carries logo_raw_ from read_collection
    }
    else {
      col.set_logo_raw(QByteArray());
    }
    col.set_default_item_type(data.mw_.settings_default_type_->currentData().toInt());
    col.set_default_is_school_item(data.mw_.settings_default_is_schoolitem_->isChecked());
    if(!data.queries_->change_collection(col)) {
      QMessageBox::warning(data.mw_.settings_page_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("Could not change settings."));
    }
  }


  /**
     Compares all settings page widget values against the stored collection
     record.  Returns true if any widget value differs from the stored value.
     @param data central application state.
     @return true if at least one widget differs from the stored settings.
  */
  bool widgets_changed(colman::AppData& data)
  {
    colman::Collection const col = data.queries_->read_collection();
    QPixmap pm = (data.mw_.settings_use_logo_->isChecked()
                      ? data.mw_.settings_logo_pixmap_->pixmap()
                      : QPixmap());
    if(col.get_name() != data.mw_.settings_collection_name_->text()) {
      return true;
    }
    if(col.get_organization() != data.mw_.settings_organization_->text()) {
      return true;
    }
    if(col.get_responsible() != data.mw_.settings_responsible_->text()) {
      return true;
    }
    if(col.get_use_lending_period() !=
       data.mw_.settings_use_lending_period_->isChecked()) {
      return true;
    }
    if(col.get_lending_period_years() !=
       data.mw_.settings_lending_period_years_->value()) {
      return true;
    }
    if(col.get_lending_period_months() !=
       data.mw_.settings_lending_period_months_->value()) {
      return true;
    }
    if(col.get_lending_period_days() !=
       data.mw_.settings_lending_period_days_->value()) {
      return true;
    }
    if(col.get_allow_reservations() !=
       data.mw_.settings_allow_reservations_->isChecked()) {
      return true;
    }
    if(col.get_reservation_period_days() !=
       data.mw_.settings_reservation_period_days_->value()) {
      return true;
    }
    {
      // default_item_type <= 0 means "not set in DB": treat index 0 as unchanged
      bool const type_unset = col.get_default_item_type() <= 0;
      bool const type_changed =
          type_unset
              ? data.mw_.settings_default_type_->currentIndex() != 0
              : col.get_default_item_type() !=
                    data.mw_.settings_default_type_->currentData().toInt();
      if(type_changed) {
        return true;
      }
    }
    if(col.get_default_is_school_item() !=
       data.mw_.settings_default_is_schoolitem_->isChecked()) {
      return true;
    }
    if(pm.isNull() && !col.get_logo_raw().isEmpty()) {
      return true;
    }
    if(data.mw_.settings_use_logo_->isChecked() &&
       col.get_logo_raw().isEmpty()) {
      return true;
    }
    if(data.mw_.settings_use_logo_->isChecked() &&
       !data.mw_.settings_logo_pixmap_->property("logo_raw")
            .toByteArray()
            .isEmpty()) {
      return true;
    }
    return false;
  }


  /**
     Opens a file-chooser dialog to select a new logo image.  On acceptance
     loads the chosen file as a QPixmap and shows it in the settings page.
     @param data central application state.
  */
  void browse_for_logo(colman::AppData& data)
  {
    QSettings settings;
    QByteArray state = settings.value("open/logo").toByteArray();
    colman::PreviewFileDialog
        diag(nullptr, QApplication::tr("Open Logo"), "");
    QStringList filters;
    filters << QApplication::tr("image files (*.svg *.png *.xpm *.jpg *.jpeg)")
            << QApplication::tr("any files (*)");
    diag.setNameFilters(filters);
    diag.setFileMode(QFileDialog::ExistingFile);
    (void)diag.restoreState(state);
    if(diag.exec()) {
      QStringList file_names = diag.selectedFiles();
      QByteArray const state = diag.saveState();
      settings.setValue("open/logo", state);
      if(!file_names.empty()) {
        QString const fname = file_names.front();
        QFile f(fname);
        if(f.open(QIODevice::ReadOnly)) {
          QByteArray const raw = f.readAll();
          QPixmap pm;
          if(pm.loadFromData(raw)) {
            data.mw_.settings_logo_pixmap_->set_original_pixmap(pm);
            data.mw_.settings_logo_->setText(fname);
            data.mw_.settings_logo_pixmap_->setProperty("logo_raw", raw);
          }
        }
      }
    }
  }

} // namespace


/// Populates all collection-settings widgets from the database.
void colman::enter_settings_tab(colman::AppData& data)
{
  fill_widgets(data);
}

/// Returns true if any settings widget value differs from the stored value.
bool colman::settings_tab_changed(colman::AppData& data)
{
  return widgets_changed(data);
}

/// Writes the current settings-widget values to the database.
void colman::apply_settings_tab(colman::AppData& data)
{
  widgets_to_settings(data);
}


/**
   Initializes the settings page; to be called once at program start.
   Connects the revert, apply, browse-logo, and category signals.
   The button-click and back-button connections are handled by the
   combined settings page in setup_central_page().
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_settings_page(QApplication& app, colman::AppData& data)
{
  data.mw_.settings_back_->setVisible(false);
  app.connect(data.mw_.settings_revert_, &QPushButton::clicked,
              [&data]() {
                fill_widgets(data);
              });
  app.connect(data.mw_.settings_apply_, &QPushButton::clicked,
              [&data]() {
                widgets_to_settings(data);
              });
  app.connect(data.mw_.settings_browse_logo_, &QPushButton::clicked,
              [&data]() {
                browse_for_logo(data);
              });
  app.connect(data.mw_.settings_add_sig_top_cat_, &QPushButton::clicked,
              [&data]() {
                bool ok = false;
                QString name = QInputDialog::getText(
                    data.mw_.settings_page_,
                    QApplication::tr("Collection Manager"),
                    QApplication::tr("Category name:"),
                    QLineEdit::Normal, QString(), &ok);
                if(!ok || name.trimmed().isEmpty()) {
                  return;
                }
                name = name.trimmed();
                data.queries_->add_signature_top_category(name);
                data.mw_.settings_sig_top_categories_->clear();
                for(QString const& cat :
                    data.queries_->read_signature_top_categories()) {
                  data.mw_.settings_sig_top_categories_->addItem(cat);
                }
              });
  app.connect(data.mw_.settings_remove_sig_top_cat_, &QPushButton::clicked,
              [&data]() {
                QListWidgetItem const* item =
                    data.mw_.settings_sig_top_categories_->currentItem();
                if(!item) {
                  return;
                }
                data.queries_->remove_signature_top_category(item->text());
                data.mw_.settings_sig_top_categories_->clear();
                for(QString const& cat :
                    data.queries_->read_signature_top_categories()) {
                  data.mw_.settings_sig_top_categories_->addItem(cat);
                }
                data.mw_.settings_sig_bottom_categories_->clear();
              });
  app.connect(data.mw_.settings_sig_top_categories_,
              &QListWidget::currentItemChanged,
              [&data](QListWidgetItem* current, QListWidgetItem*) {
                data.mw_.settings_sig_bottom_categories_->clear();
                if(!current) {
                  return;
                }
                for(QString const& cat :
                    data.queries_->read_signature_bottom_categories(
                        current->text())) {
                  data.mw_.settings_sig_bottom_categories_->addItem(cat);
                }
              });
  app.connect(data.mw_.settings_add_sig_bottom_cat_, &QPushButton::clicked,
              [&data]() {
                QListWidgetItem const* top_item =
                    data.mw_.settings_sig_top_categories_->currentItem();
                if(!top_item) {
                  return;
                }
                QString const top = top_item->text();
                bool ok = false;
                QString name = QInputDialog::getText(
                    data.mw_.settings_page_,
                    QApplication::tr("Collection Manager"),
                    QApplication::tr("Subcategory name:"),
                    QLineEdit::Normal, QString(), &ok);
                if(!ok || name.trimmed().isEmpty()) {
                  return;
                }
                name = name.trimmed();
                data.queries_->add_signature_bottom_category(top, name);
                data.mw_.settings_sig_bottom_categories_->clear();
                for(QString const& cat :
                    data.queries_->read_signature_bottom_categories(top)) {
                  data.mw_.settings_sig_bottom_categories_->addItem(cat);
                }
              });
  app.connect(data.mw_.settings_remove_sig_bottom_cat_, &QPushButton::clicked,
              [&data]() {
                QListWidgetItem const* top_item =
                    data.mw_.settings_sig_top_categories_->currentItem();
                if(!top_item) {
                  return;
                }
                QString const top = top_item->text();
                QListWidgetItem const* bottom_item =
                    data.mw_.settings_sig_bottom_categories_->currentItem();
                if(!bottom_item) {
                  return;
                }
                data.queries_->remove_signature_bottom_category(
                    top, bottom_item->text());
                data.mw_.settings_sig_bottom_categories_->clear();
                for(QString const& cat :
                    data.queries_->read_signature_bottom_categories(top)) {
                  data.mw_.settings_sig_bottom_categories_->addItem(cat);
                }
              });
}

/* EOF */
