// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// preferences page setup function
/**
   @file
   @author Christoph Pesch
*/

#include "colman_preferences_page.hpp"
#ifndef COLMAN_SRU_SERVER_HPP
#  include "colman_sru_server.hpp"
#endif
#ifndef COLMAN_CODE128_ITEM_HPP
#  include "colman_code128_item.hpp"
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QFILEDIALOG
#  include <QFileDialog>
#  define SYSTEM_QFILEDIALOG
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QSVGRENDERER
#  include <QSvgRenderer>
#  define SYSTEM_QSVGRENDERER
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif


namespace
{
  /// Fills the SRU server table widget with the given server list.
  /**
    @param data     Application state holding the main window UI.
    @param servers  The list of SRU servers to display.
  */
  void fill_sru_table(colman::AppData& data,
                      QVector<colman::SruServer> const& servers)
  {
    auto* tbl = data.mw_.pref_sru_servers_;
    tbl->setRowCount(0);
    for(auto const& srv : servers) {
      int const row = tbl->rowCount();
      tbl->insertRow(row);
      auto* chk = new QTableWidgetItem();
      chk->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      chk->setCheckState(srv.enabled ? Qt::Checked : Qt::Unchecked);
      tbl->setItem(row, 0, chk);
      tbl->setItem(row, 1, new QTableWidgetItem(srv.name));
      tbl->setItem(row, 2, new QTableWidgetItem(srv.endpoint));
      tbl->setItem(row, 3, new QTableWidgetItem(srv.isbn_field));
      tbl->setItem(row, 4, new QTableWidgetItem(srv.ean_field));
      tbl->setItem(row, 5, new QTableWidgetItem(srv.record_schema));
      tbl->setItem(row, 6, new QTableWidgetItem(srv.title_field));
    }
  }

  /// Reads the SRU server table widget and returns the server list.
  /**
    @param data  Application state holding the main window UI.
    @return The list of SRU servers as currently shown in the table.
  */
  QVector<colman::SruServer> sru_from_table(colman::AppData& data)
  {
    QVector<colman::SruServer> result;
    auto* tbl = data.mw_.pref_sru_servers_;
    for(int row = 0; row < tbl->rowCount(); ++row) {
      auto text = [tbl, row](int col) -> QString {
        auto* it = tbl->item(row, col);
        return it ? it->text().trimmed() : QString();
      };
      colman::SruServer srv;
      auto* chk = tbl->item(row, 0);
      srv.enabled = chk && chk->checkState() == Qt::Checked;
      srv.name = text(1);
      srv.endpoint = text(2);
      srv.isbn_field = text(3);
      srv.ean_field = text(4);
      srv.record_schema = text(5);
      srv.title_field = text(6);
      if(!srv.endpoint.isEmpty()) {
        result.append(srv);
      }
    }
    return result;
  }

  /// Populates all preference widgets with values read from QSettings.
  /**
    Reads the stored settings for thumbnail directory, API keys, network
    timeout, and ISBN search provider flags, then writes them into the
    corresponding UI widgets on the preferences page.
    @param data Application state holding the main window UI.
  */
  void fill_widgets(colman::AppData& data)
  {
    QSettings settings;
    QString const dirname = settings.value("thumbnail_dir", "").toString();
    QString const googleapis_key =
        settings.value("googleapis_key", "").toString();
    int const timeout = settings.value("timeout", 5000).toInt();
    int const poll_interval =
        settings.value("poll_interval_s", 10).toInt();
    data.mw_.pref_thumbnail_dir_->setText(dirname);
    data.mw_.pref_googleapis_key_->setText(googleapis_key);
    data.mw_.pref_timeout_->setValue(timeout / 1000);
    data.mw_.pref_poll_interval_->setValue(poll_interval);
    data.mw_.pref_search_openlibrary_
        ->setChecked(settings.value("use_openlibrary", false).toBool());
    data.mw_.pref_search_googleapis_
        ->setChecked(settings.value("use_googleapis", false).toBool());
    fill_sru_table(data, colman::load_sru_servers());
    data.mw_.pref_search_musicbrainz_
        ->setChecked(settings.value("use_musicbrainz", false).toBool());
    data.mw_.pref_search_bgg_
        ->setChecked(settings.value("use_bgg", false).toBool());
    data.mw_.pref_bgg_token_->setText(
        settings.value("bgg_token", "").toString());
    data.mw_.pref_search_opengtindb_
        ->setChecked(settings.value("use_opengtindb", false).toBool());
    data.mw_.pref_opengtindb_userid_->setText(
        settings.value("opengtindb_userid", "").toString());
    data.mw_.pref_proxy_user_->setText(
        settings.value("proxy_user", "").toString());
    data.mw_.pref_proxy_password_->setText(
        settings.value("proxy_password", "").toString());
    data.mw_.pref_download_thumbnails_
        ->setChecked(settings.value("download_thumbnails", false).toBool());
    data.mw_.pref_search_threshold_->setValue(
        settings.value("search_threshold", 1000).toInt());
    data.mw_.pref_barcode_unit_->setValue(
        settings.value("barcode_unit_mm", colman::DefaultBarcodeUnitMm)
            .toDouble());
  }


  void widgets_to_settings(colman::AppData& data)
  {
    QSettings settings;
    QString const dirname = data.mw_.pref_thumbnail_dir_->text();
    QString const googleapis_key = data.mw_.pref_googleapis_key_->text();
    int const timeout = data.mw_.pref_timeout_->value() * 1000;
    int const poll_interval = data.mw_.pref_poll_interval_->value();
    settings.setValue("thumbnail_dir", dirname);
    data.set_thumbnail_dir(dirname);
    settings.setValue("googleapis_key", googleapis_key);
    settings.setValue("timeout", timeout);
    settings.setValue("poll_interval_s", poll_interval);
    settings.setValue("use_openlibrary",
                      data.mw_.pref_search_openlibrary_->isChecked());
    settings.setValue("use_googleapis",
                      data.mw_.pref_search_googleapis_->isChecked());
    colman::save_sru_servers(sru_from_table(data));
    settings.setValue("use_musicbrainz",
                      data.mw_.pref_search_musicbrainz_->isChecked());
    settings.setValue("use_bgg",
                      data.mw_.pref_search_bgg_->isChecked());
    settings.setValue("bgg_token",
                      data.mw_.pref_bgg_token_->text().trimmed());
    settings.setValue("use_opengtindb",
                      data.mw_.pref_search_opengtindb_->isChecked());
    settings.setValue("opengtindb_userid",
                      data.mw_.pref_opengtindb_userid_->text().trimmed());
    settings.setValue("proxy_user",
                      data.mw_.pref_proxy_user_->text().trimmed());
    settings.setValue("proxy_password",
                      data.mw_.pref_proxy_password_->text());
    settings.setValue("download_thumbnails",
                      data.mw_.pref_download_thumbnails_->isChecked());
    settings.setValue("search_threshold",
                      data.mw_.pref_search_threshold_->value());
    settings.setValue("barcode_unit_mm",
                      data.mw_.pref_barcode_unit_->value());
  }


  bool widgets_changed(colman::AppData& data)
  {
    QSettings settings;
    QString const dirname = settings.value("thumbnail_dir", "").toString();
    QString const googleapis_key =
        settings.value("googleapis_key", "").toString();
    int const timeout = settings.value("timeout", 5000).toInt();
    bool const download_th =
        settings.value("download_thumbnails", false).toBool();
    int const poll_interval = settings.value("poll_interval_s", 10).toInt();

    QString const dirname2 = data.mw_.pref_thumbnail_dir_->text();
    QString const googleapis_key2 = data.mw_.pref_googleapis_key_->text();
    int const timeout2 = data.mw_.pref_timeout_->value() * 1000;
    bool const download_th2 =
        data.mw_.pref_download_thumbnails_->isChecked();
    int const poll_interval2 = data.mw_.pref_poll_interval_->value();

    // Compare SRU table with saved servers.
    auto const saved_sru = colman::load_sru_servers();
    auto const table_sru = sru_from_table(data);
    bool sru_changed = saved_sru.size() != table_sru.size();
    if(!sru_changed) {
      for(int i = 0; i < saved_sru.size(); ++i) {
        if(saved_sru[i].enabled != table_sru[i].enabled ||
           saved_sru[i].name != table_sru[i].name ||
           saved_sru[i].endpoint != table_sru[i].endpoint ||
           saved_sru[i].isbn_field != table_sru[i].isbn_field ||
           saved_sru[i].ean_field != table_sru[i].ean_field ||
           saved_sru[i].title_field != table_sru[i].title_field ||
           saved_sru[i].record_schema != table_sru[i].record_schema) {
          sru_changed = true;
          break;
        }
      }
    }

    return timeout != timeout2 ||
           dirname != dirname2 ||
           googleapis_key != googleapis_key2 ||
           download_th != download_th2 ||
           poll_interval != poll_interval2 ||
           sru_changed ||
           (data.mw_.pref_search_openlibrary_->isChecked() !=
            settings.value("use_openlibrary", false).toBool()) ||
           (data.mw_.pref_search_googleapis_->isChecked() !=
            settings.value("use_googleapis", false).toBool()) ||
           (data.mw_.pref_search_musicbrainz_->isChecked() !=
            settings.value("use_musicbrainz", false).toBool()) ||
           (data.mw_.pref_search_bgg_->isChecked() !=
            settings.value("use_bgg", false).toBool()) ||
           (data.mw_.pref_bgg_token_->text().trimmed() !=
            settings.value("bgg_token", "").toString()) ||
           (data.mw_.pref_search_opengtindb_->isChecked() !=
            settings.value("use_opengtindb", false).toBool()) ||
           (data.mw_.pref_opengtindb_userid_->text().trimmed() !=
            settings.value("opengtindb_userid", "").toString()) ||
           (data.mw_.pref_proxy_user_->text().trimmed() !=
            settings.value("proxy_user", "").toString()) ||
           (data.mw_.pref_proxy_password_->text() !=
            settings.value("proxy_password", "").toString()) ||
           (data.mw_.pref_search_threshold_->value() !=
            settings.value("search_threshold", 1000).toInt()) ||
           (data.mw_.pref_barcode_unit_->value() !=
            settings.value("barcode_unit_mm", colman::DefaultBarcodeUnitMm)
                .toDouble());
  }

} // namespace


void colman::enter_preferences_tab(colman::AppData& data)
{
  fill_widgets(data);
}

bool colman::preferences_tab_changed(colman::AppData& data)
{
  return widgets_changed(data);
}

void colman::apply_preferences_tab(colman::AppData& data)
{
  widgets_to_settings(data);
}


/**
   Initializes the preferences page; to be called once at program start.
   Connects the revert, apply, and browse-thumbnail signals.
   The button-click and back-button connections are handled by the
   combined settings page in setup_central_page().
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_preferences_page(QApplication& app, colman::AppData& data)
{
  data.mw_.preferences_back_->setVisible(false);

  // Configure the SRU server table.
  auto* tbl = data.mw_.pref_sru_servers_;
  tbl->setColumnCount(7);
  tbl->setHorizontalHeaderLabels({QApplication::tr("Enable"),
                                  QApplication::tr("Name"),
                                  QApplication::tr("Endpoint"),
                                  QApplication::tr("ISBN field"),
                                  QApplication::tr("EAN field"),
                                  QApplication::tr("Schema"),
                                  QApplication::tr("Title field")});
  tbl->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  tbl->setSelectionBehavior(QAbstractItemView::SelectRows);

  app.connect(data.mw_.pref_sru_add_, &QPushButton::clicked, [&data]() {
    auto* t = data.mw_.pref_sru_servers_;
    int const row = t->rowCount();
    t->insertRow(row);
    auto* chk = new QTableWidgetItem();
    chk->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    chk->setCheckState(Qt::Unchecked);
    t->setItem(row, 0, chk);
    t->setItem(row, 1,
               new QTableWidgetItem(QApplication::tr("New SRU server")));
    t->setItem(row, 2, new QTableWidgetItem(""));
    t->setItem(row, 3, new QTableWidgetItem(""));
    t->setItem(row, 4, new QTableWidgetItem(""));
    t->setItem(row, 5, new QTableWidgetItem("MARC21-xml"));
    t->setItem(row, 6, new QTableWidgetItem(""));
  });

  app.connect(data.mw_.pref_sru_remove_, &QPushButton::clicked, [&data]() {
    auto* t = data.mw_.pref_sru_servers_;
    auto const sel = t->selectedItems();
    if(!sel.isEmpty()) {
      t->removeRow(sel.first()->row());
    }
  });

  app.connect(data.mw_.pref_thumbnail_dir_browse_, &QPushButton::clicked,
              [&data]() {
                QString const dir = QFileDialog::getExistingDirectory(
                    data.mw_.preferences_page_,
                    QApplication::tr("Select thumbnail directory"),
                    data.mw_.pref_thumbnail_dir_->text());
                if(!dir.isEmpty()) {
                  data.mw_.pref_thumbnail_dir_->setText(dir);
                }
              });
  app.connect(data.mw_.pref_revert_, &QPushButton::clicked,
              [&data]() {
                fill_widgets(data);
              });
  app.connect(data.mw_.pref_apply_, &QPushButton::clicked,
              [&data]() {
                widgets_to_settings(data);
              });

  // Render the "Powered by BGG" logo at half its native size.
  QSvgRenderer renderer(
      QString(":/icons/thirdparty/Color/SVG/powered-by-bgg-rgb.svg"));
  int const LogoH = 38;
  int const logo_w = qRound(renderer.defaultSize().width() * LogoH
                            / static_cast<double>(
                                renderer.defaultSize().height()));
  QPixmap logo(logo_w, LogoH);
  logo.fill(Qt::transparent);
  QPainter painter(&logo);
  renderer.render(&painter);
  data.mw_.pref_bgg_logo_->setPixmap(logo);
}

/* EOF */
