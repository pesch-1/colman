// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// acquisition wizard: setup, page navigation, and item/acquisition DB
/// ops.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_acquisition_page.hpp"
#ifndef COLMAN_CAMERA_SCAN_DIALOG_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_SRU_SERVER_HPP
#  include "colman_sru_server.hpp"
#endif
#ifndef COLMAN_NAME_QUERIES_HPP
#  include "colman_name_queries.hpp"
#endif
#ifndef COLMAN_ITEM_BARCODE_PAGE_HPP
#  include "colman_item_barcode_page.hpp"
#endif
#ifndef COLMAN_SIGNATURE_PRINT_PAGE_HPP
#  include "colman_signature_print_page.hpp"
#endif
#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  include "colman_clear_selection_filter.hpp"
#endif
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_TABLEVIEW_HELPERS_HPP
#  include "colman_tableview_helpers.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QBUFFER
#  include <QBuffer>
#  define SYSTEM_QBUFFER
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef COLMAN_OPENGTINDB_HPP
#  include "colman_opengtindb.hpp"
#  define COLMAN_OPENGTINDB_HPP
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif
#ifndef SYSTEM_QCAMERADEVICE
#  include <QCameraDevice>
#  define SYSTEM_QCAMERADEVICE
#endif
#ifndef SYSTEM_QMEDIADEVICES
#  include <QMediaDevices>
#  define SYSTEM_QMEDIADEVICES
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
namespace
{
  void goto_select_page(colman::AppData& data, int type_id = -1);
  void goto_count_page(colman::AppData& data);
  void goto_done_page(colman::AppData& data);
  void update_acquisition_item_count(colman::AppData& data);
  void acquisition_load_more(colman::AppData& data);

  /**
   * Populates the type filter ComboBox on the select page with all available item
   * types. Selects @p select_type_id if >= 0, otherwise the collection's
   * default type.
   * @param data          Central application state.
   * @param select_type_id DB id of the type to pre-select, or -1 for default.
   */
  void populate_type_combo(colman::AppData& data, int select_type_id = -1)
  {
    auto const types = data.queries_->get_type_locale();
    colman::Collection const collection = data.queries_->read_collection();
    int const default_type = (select_type_id >= 0)
                                 ? select_type_id
                                 : collection.get_default_item_type();
    colman::AutoBlockSignals b(data.mw_.acquisition_select_type_);
    data.mw_.acquisition_select_type_->clear();
    int default_index = 0;
    for(int i = 0; i < static_cast<int>(types.size()); ++i) {
      data.mw_.acquisition_select_type_->addItem(types[i].second,
                                                 types[i].first);
      if(types[i].first == default_type) {
        default_index = i;
      }
    }
    data.mw_.acquisition_select_type_->setCurrentIndex(default_index);
  }

  /// Updates column visibility for the acquisition item table.
  void setup_acquisition_item_columns(colman::AppData& data)
  {
    bool const is_admin = (data.current_data_collector_ == 0);
    auto* view = data.mw_.acquisition_select_items_view_;
    view->setColumnHidden(0, true);
    if(data.queries_->is_locale_supported()) {
      view->setColumnHidden(4, false);
      view->setColumnHidden(5, true);
    }
    else {
      view->setColumnHidden(4, true);
      view->setColumnHidden(5, false);
    }
    view->setColumnHidden(6, !is_admin);
  }


  /// Updates the acquisition item count label and load-more visibility.
  void update_acquisition_item_count(colman::AppData& data)
  {
    auto const n = data.acquisition_item_model_.rowCount();
    data.mw_.acquisition_select_item_count_->setText(
        QApplication::tr("%n item(s) shown", "", n));
    data.mw_.acquisition_select_load_more_->setVisible(
        data.acquisition_item_has_more_);
  }


  /**
   * Reloads the item table on the select page by querying items that match the
   * currently selected type and the current search field text. Refreshes the
   * column headers afterwards.
   * @param data Central application state.
   */
  void refresh_item_query(colman::AppData& data)
  {
    auto const type_id =
        data.mw_.acquisition_select_type_->currentData().toInt();
    QString const& search = data.mw_.acquisition_select_search_->text();
    data.acquisition_item_offset_ = 0;
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    auto rows = data.queries_->query_items_with_type(
        type_id, search, lim, 0);
    data.acquisition_item_has_more_ =
        lim > 0 && static_cast<int>(rows.size()) > lim;
    if(data.acquisition_item_has_more_) {
      rows.pop_back();
    }
    data.acquisition_item_model_.set_rows(std::move(rows));
    setup_acquisition_item_columns(data);
    update_acquisition_item_count(data);
  }


  /// Loads the next page of acquisition items and appends them to the model.
  void acquisition_load_more(colman::AppData& data)
  {
    auto const type_id =
        data.mw_.acquisition_select_type_->currentData().toInt();
    QString const& search = data.mw_.acquisition_select_search_->text();
    auto const lim =
        QSettings().value("search_threshold", 1000).toInt();
    data.acquisition_item_offset_ += lim;
    auto rows = data.queries_->query_items_with_type(
        type_id, search, lim, data.acquisition_item_offset_);
    data.acquisition_item_has_more_ =
        static_cast<int>(rows.size()) > lim;
    if(data.acquisition_item_has_more_) {
      rows.pop_back();
    }
    data.acquisition_item_model_.append_rows(std::move(rows));
    update_acquisition_item_count(data);
  }

  /**
   * Returns the QSettings group holding the remembered acquisition
   * signatures of the currently open collection.  The collection is
   * identified by its name and barcode prefix (hex-encoded, so that no
   * character acts as a QSettings group separator); this also works for
   * REST clients, which have no local database file.
   * @param data Central application state.
   * @return QSettings group name.
   */
  QString signature_settings_group(colman::AppData& data)
  {
    colman::Collection const col = data.queries_->read_collection();
    QString const id = col.get_name() + QChar('\n') + col.get_barcode_prefix();
    return QString("acquisition_signature/") +
           QString::fromLatin1(id.toUtf8().toHex());
  }

  /**
   * Loads the remembered signatures of the current collection from
   * QSettings and makes them the defaults of the acquisition item widget.
   * @param data Central application state.
   */
  void load_default_signatures(colman::AppData& data)
  {
    QSettings settings;
    settings.beginGroup(signature_settings_group(data));
    data.mw_.acquisition_select_item_->set_default_signatures(
        settings.value("top").toString(), settings.value("bottom").toString());
  }

  /**
   * Remembers the signatures of a newly acquired item as defaults for the
   * next acquisition of the current collection.  Empty signatures are stored
   * as well, so clearing the fields also clears the default.
   * @param data Central application state.
   * @param item The newly created item.
   */
  void store_default_signatures(colman::AppData& data, colman::Item const& item)
  {
    QString const top = item.get_signature_top().trimmed();
    QString const bottom = item.get_signature_bottom().trimmed();
    QSettings settings;
    settings.beginGroup(signature_settings_group(data));
    settings.setValue("top", top);
    settings.setValue("bottom", bottom);
    data.mw_.acquisition_select_item_->set_default_signatures(top, bottom);
  }

  /**
   * Displays the select page (existing-item search) of the acquisition wizard.
   * Populates the type ComboBox (pre-selecting @p type_id or the collection
   * default), refreshes the item table, resets the item detail widget, and
   * moves focus to the search field.
   * @param data    Central application state.
   * @param type_id DB id of the type to pre-select, or -1 for default.
   */
  void goto_select_page(colman::AppData& data, int type_id)
  {
    populate_type_combo(data, type_id);
    refresh_item_query(data);
    int const selected_type =
        data.mw_.acquisition_select_type_->currentData().toInt();
    data.mw_.acquisition_select_item_->reset_item_type(selected_type);
    data.mw_.acquisition_select_item_->set_read_only(false);
    data.mw_.acquisition_select_new_item_->hide();
    data.mw_.acquisition_select_restore_->hide();
    data.mw_.acquisition_select_change_item_->hide();
    data.mw_.acquisition_select_more_copies_->hide();
    data.mw_.acquisition_select_search_->setFocus();
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.acquisition_select_);
  }

  /**
   * Navigates to the select page and pre-fills the item widget with @p item.
   *
   * goto_select_page() resets the item widget, so lookup_result_to_fields()
   * must be called afterwards.  Empty signatures in @p item keep the
   * remembered acquisition default.  The "New Item" button is made visible
   * when the item has a name so the user can proceed without further
   * editing.
   * @param data    Central application state.
   * @param item    Item data to display (type_id must already be set).
   * @param type_id DB id of the item type.
   */
  void show_select_page_with_item(colman::AppData& data, colman::Item const& item,
                                  int type_id)
  {
    goto_select_page(data, type_id);
    data.mw_.acquisition_select_item_->lookup_result_to_fields(item);
    data.mw_.acquisition_select_new_item_->setVisible(
        data.mw_.acquisition_select_item_->has_name());
  }

  /**
   * Looks up @p ean in the local database. On a match, updates the search
   * field with the item name, navigates to the select page with the correct
   * item type pre-selected, and selects the item row.
   * @param data Central application state.
   * @param ean  Normalised EAN-13 string to look up.
   * @return True when a local match was found and the wizard was navigated.
   */
  bool navigate_to_local_ean_item(colman::AppData& data, QString const& ean)
  {
    auto const local_ids = data.queries_->items_by_ean(ean);
    if(local_ids.empty()) {
      return false;
    }
    int const item_id = local_ids[0];
    colman::Item item;
    QString type_name;
    if(!data.queries_->get_item(item_id, item, type_name)) {
      return false;
    }
    QString const name = item.get_name();
    QMessageBox msgbox(data.mw_.centralwidget);
    msgbox.setWindowTitle(QObject::tr("Item already exists"));
    msgbox.setText(
        QObject::tr("An item with this barcode already exists:\n\"%1\"\n\n"
                    "Add copies of the existing item, or create a new "
                    "separate item?")
            .arg(name));
    auto* add_copies_btn =
        msgbox.addButton(QObject::tr("Add copies"), QMessageBox::AcceptRole);
    msgbox.addButton(
        QObject::tr("New separate item"), QMessageBox::RejectRole);
    msgbox.setDefaultButton(add_copies_btn);
    msgbox.exec();
    if(msgbox.clickedButton() == add_copies_btn) {
      if(name.isEmpty()) {
        data.mw_.acquisition_select_search_->clear();
      }
      else {
        data.mw_.acquisition_select_search_->setText(name);
      }
      goto_select_page(data, item.get_type_id());
      colman::select_and_scroll_to(
          data.mw_.acquisition_select_items_view_, 0, item_id);
    }
    else {
      show_select_page_with_item(data, item, item.get_type_id());
    }
    return true;
  }

  /**
   * Refreshes the item table using the current search field text.  If no items
   * match and the search string is a valid barcode or ISBN, an internet lookup
   * is triggered and the item widget on the select page is populated with the result.
   * If exactly one item matches it is selected automatically.
   * @param data Central application state.
   */
  void search_barcode(colman::AppData& data)
  {
    refresh_item_query(data);
    int const rows = data.acquisition_item_model_.rowCount();
    if(0 == rows) {
      QString const search = data.mw_.acquisition_select_search_->text();
      bool const is_ean = colman::is_product_ean(search);
      bool const is_upc = !is_ean && colman::is_upc_a(search);
      bool const is_isbn =
          !is_ean && !is_upc && (colman::is_isbn10(search) || colman::is_isbn13(search));
      if(is_isbn || is_ean || is_upc) {
        QString const lookup = is_upc ? (QString("0") + search) : search;
        if((is_ean || is_upc) && navigate_to_local_ean_item(data, lookup)) {
          return;
        }
        if(is_isbn) {
          data.mw_.acquisition_select_item_->reset_item_type();
        }
        else {
          int const type_id =
              data.mw_.acquisition_select_type_->currentData().toInt();
          data.mw_.acquisition_select_item_->reset_item_type(type_id);
        }
        data.mw_.acquisition_select_item_->internet_lookup_string(lookup);
      }
      else {
        // Free-text: run BGG lookup when board_game type is selected and
        // BGG service is enabled in preferences; for book/CD/DVD/Blu-ray
        // types run the multi-source title search instead.
        int const type_id =
            data.mw_.acquisition_select_type_->currentData().toInt();
        bool const is_board_game =
            data.queries_->media_kind_for_type_id(type_id) ==
            colman::MediaKind::BoardGame;
        QSettings bgg_settings;
        if(is_board_game && !search.trimmed().isEmpty() &&
           bgg_settings.value("use_bgg", false).toBool()) {
          data.mw_.acquisition_select_item_->reset_item_type(type_id);
          data.mw_.acquisition_select_item_->internet_bgg_name_lookup(
              search.trimmed());
        }
        else if(!search.trimmed().isEmpty()) {
          colman::MediaKind const kind =
              data.queries_->media_kind_for_type_id(type_id);
          if(colman::name_search_available(kind)) {
            data.mw_.acquisition_select_item_->reset_item_type(type_id);
            data.mw_.acquisition_select_item_->internet_name_lookup(
                search.trimmed());
          }
        }
      }
    }
    else if(1 == rows) {
      data.mw_.acquisition_select_items_view_->selectRow(0);
    }
  }

  /**
   * Exits the acquisition wizard and returns to the central start page.
   * Clears the item query model.
   * functions.
   * @param data Central application state.
   */
  void leave_acquisition_pages(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.acquisition_item_model_.set_rows({});
  }

  /**
   * Clears the item detail widget on the select page and deselects any row in the item
   * table. Called when the table selection becomes empty.
   * @param data Central application state.
   */
  void deselect_item(colman::AppData& data)
  {
    data.mw_.acquisition_select_item_->clear_fields();
    data.mw_.acquisition_select_item_->set_read_only(false);
    data.mw_.acquisition_select_items_view_->clearSelection();
  }

  /**
   * Loads the item identified by @p id from the database and populates the
   * item detail widget on the select page. Also scrolls the item table to the selected
   * row. Shows a warning dialog and clears the search field when the item is
   * not found in the database.
   * @param data Central application state.
   * @param id   Database ID of the item to display.
   */
  void select_item(colman::AppData& data, int id)
  {
    colman::Item item;
    QString type_name;
    if(!data.queries_->get_item(id, item, type_name)) {
      QMessageBox::warning(data.mw_.acquisition_select_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("unknown item"));
      data.mw_.acquisition_select_search_->setText("");
      return;
    }
    data.mw_.acquisition_select_item_->item_to_fields(item, type_name);
    data.mw_.acquisition_select_item_->set_read_only(true);
    colman::select_and_scroll_to(data.mw_.acquisition_select_items_view_, 0,
                                 id);
  }

  /**
   * Displays the count page (acquisition details) of the acquisition wizard.
   * Reads the item name either from the database (when an existing item is
   * selected on the select page) or from the item widget on the select page, sets the
   * acquisition date to today, and pre-fills the copy count with 1.
   * @param data Central application state.
   */
  void goto_count_page(colman::AppData& data)
  {
    int const item_id =
        colman::selected_id(data.mw_.acquisition_select_items_view_);
    colman::Item item;
    if(item_id > 0) {
      QString type_name;
      data.queries_->get_item(item_id, item, type_name);
    }
    else {
      if(!data.mw_.acquisition_select_item_->fields_to_item(item)) {
        return;
      }
    }
    data.mw_.acquisition_count_item_name_->setText(item.get_name());
    data.mw_.acquisition_count_datetime_->hide();
    data.mw_.label_14->hide();
    data.mw_.acquisition_count_count_->setValue(1);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.acquisition_count_);
  }

  /**
     Inserts a new item and returns its database id.
     writes a backup if necessary.
     @param data Application state containing the database.
     @param item Data of the item to insert.
     @return Database id of the newly inserted item, or -1 on failure.
   */
  int add_new_item(colman::AppData& data, colman::Item const& item)
  {
    colman::Item i = item;
    i.set_data_collector_id(data.current_data_collector_);
    auto const item_id = data.queries_->add_new_item(i);
    return item_id;
  }

  /// Inserts a new acquisition record and returns its database id.
  /**
     Writes a backup if necesssary.
     @param data Application state containing the database and undo stack.
     @param acquisition Data of the acquisition to insert.
     @return Database id of the newly inserted acquisition, or -1 on failure.
   */
  int add_new_acquisition(colman::AppData& data,
                          colman::Acquisition const& acquisition)
  {
    colman::Acquisition a = acquisition;
    a.set_data_collector_id(data.current_data_collector_);
    return data.queries_->add_new_acquisition(a);
  }

  /**
   * Commits the acquisition to the database and advances to the done page.
   * When an existing item is selected on the select page, only an Acquisition
   * record is inserted; when no item is selected a new Item record is created
   * first (with a backup written before each database mutation). Warning
   * dialogs are shown on insertion failure and the wizard stays on the count page. The
   * completion page then asks whether the user wants to add another
   * acquisition.
   * @param data Central application state.
   */
  void goto_done_page(colman::AppData& data)
  {
    int item_id = colman::selected_id(data.mw_.acquisition_select_items_view_);
    colman::Item item;
    if(item_id > 0) {
      QString type_name;
      data.queries_->get_item(item_id, item, type_name);
    }
    else {
      if(!data.mw_.acquisition_select_item_->fields_to_item(item)) {
        return;
      }
    }
    QDateTime const datetime = QDateTime::currentDateTimeUtc();
    int const count = data.mw_.acquisition_count_count_->value();
    if(count > 0) {
      if(item_id <= 0) {
        item_id = add_new_item(data, item);
        if(item_id < 0) {
          QMessageBox::warning(data.mw_.acquisition_count_,
                               QApplication::tr("Collection Manager"),
                               QApplication::tr("Insertion of new item failed. "
                                                "Missing data?"));
          return;
        }
        store_default_signatures(data, item);
      }
      colman::Acquisition acquisition;
      acquisition.set_item_id(item_id);
      acquisition.set_count(count);
      acquisition.set_date_time(datetime);
      int const aid = ::add_new_acquisition(data, acquisition);
      if(aid < 0) {
        QMessageBox::warning(data.mw_.acquisition_count_,
                             QApplication::tr("Collection Manager"),
                             QApplication::tr("Could not add acquisition."));
        return;
      }
    }
    data.last_acquisition_item_id_ = item_id;
    data.mw_.acquisition_done_print_top_->setEnabled(
        !item.get_signature_top().trimmed().isEmpty());
    data.mw_.acquisition_done_print_bottom_->setEnabled(
        !item.get_signature_bottom().trimmed().isEmpty());
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.acquisition_done_);
  }

  /**
   * Attempts to advance the wizard past type-selection by checking the local
   * database first, then querying the internet (MusicBrainz/DNB), and
   * finally trying a BGG lookup for board games.
   *
   * Local check: calls items_by_ean(); if a match is found, navigates to
   * the select page with the item pre-selected (skipping the type-selection page).
   *
   * Internet EAN lookup: type inference uses the "director" field (→ Blu-ray
   * or DVD) and the "artist" field (→ CD); the "extent" field (MARC21 300)
   * is used to distinguish Blu-ray from DVD when present.  MARC21 tag 653
   * game-related keywords (e.g. "gesellschaftsspiele") set an "is_game" flag.
   *
   * BGG fallback: entered when DNB identified the item as a game ("is_game")
   * or when DNB returned no data at all.  When the DNB title is available it
   * is passed directly; otherwise opengtindb is queried and the dialog is
   * only shown when BGG returns at least one hit.
   *
   * Returns false when all approaches fail; the caller then falls back to
   * step-1 type selection.
   *
   * @param data  Central application state.
   * @param ean   EAN-13 string to look up.
   * @return True when the wizard has been advanced past the type-selection
   *         page.
   */
  bool try_ean_auto_lookup(colman::AppData& data,
                           QString const& ean,
                           QString* prefill = nullptr)
  {
    // Check local database before any internet lookup.
    if(navigate_to_local_ean_item(data, ean)) {
      return true;
    }

    QSettings settings;

    // Internet EAN lookup via MusicBrainz / DNB.
    // dnb_title: title from DNB when non-empty.
    // is_game_from_dnb: true when MARC21 653 keywords indicate a game.
    QString dnb_title;
    bool is_game_from_dnb = false;
    colman::Item dnb_item;

    data.ean_queries_.use_musicbrainz(
        settings.value("use_musicbrainz", false).toBool());
    data.ean_queries_.set_sru_servers(colman::load_sru_servers());
    if(data.ean_queries_.query_ean(ean)) {
      colman::Item item = data.ean_queries_.get_item_data();
      if(!item.get_name().isEmpty()) {
        dnb_title = item.get_name();
        item.set_value("ean", ean);
        dnb_item = item;
        auto const& lvals = item.get_list_values();
        auto const& vals = item.get_values();
        bool const has_director =
            lvals.count("director") > 0 && !lvals.at("director").isEmpty();
        bool const has_participants =
            vals.count("participants") > 0 &&
            !vals.at("participants").isEmpty();
        bool const has_artist =
            lvals.count("artist") > 0 && !lvals.at("artist").isEmpty();
        is_game_from_dnb = vals.count("is_game") > 0;
        // Use physical extent (MARC21 300), carrier type (MARC21 338), and
        // media type (MARC21 337) to distinguish Blu-ray from DVD.
        QString const extent =
            vals.count("extent") > 0 ? vals.at("extent") : QString();
        QString const carrier_type =
            vals.count("carrier_type") > 0
                ? vals.at("carrier_type")
                : QString();
        QString const media_type =
            vals.count("media_type") > 0 ? vals.at("media_type") : QString();
        bool const is_bluray =
            extent.contains("Blu-ray", Qt::CaseInsensitive) ||
            extent.contains("bluray", Qt::CaseInsensitive) ||
            carrier_type.contains("Blu-ray", Qt::CaseInsensitive) ||
            carrier_type.contains("bluray", Qt::CaseInsensitive);
        bool const is_dvd =
            !is_bluray && (extent.contains("DVD", Qt::CaseInsensitive) ||
                           carrier_type.contains("DVD", Qt::CaseInsensitive) ||
                           carrier_type.contains("Videodisk",
                                                 Qt::CaseInsensitive) ||
                           carrier_type.contains("Videodisc",
                                                 Qt::CaseInsensitive));
        QString const media_class =
            vals.count("media_classification") > 0
                ? vals.at("media_classification")
                : QString();
        bool const has_video_signal =
            is_bluray || is_dvd ||
            media_type.contains("Video", Qt::CaseInsensitive) ||
            media_class == "video";
        bool const has_audio_signal = media_class == "audio";
        int const bluray_id =
            data.queries_->type_id_for_media_kind(colman::MediaKind::BluRay);
        int const dvd_id =
            data.queries_->type_id_for_media_kind(colman::MediaKind::Dvd);
        int const cd_id =
            data.queries_->type_id_for_media_kind(colman::MediaKind::Cd);
        int type_id = -1;
        if(has_director || (has_video_signal && !has_artist)) {
          if(is_bluray) {
            type_id = bluray_id;
          }
          if(type_id < 0) {
            type_id = dvd_id;
          }
          if(type_id < 0) {
            type_id = bluray_id;
          }
        }
        if(type_id < 0 && (has_artist || has_audio_signal)) {
          type_id = cd_id;
        }
        // Carrier/extent fallback: infer video type from MARC21 300/338
        // when the 508/700 director credit is absent.
        if(type_id < 0 && !is_game_from_dnb && has_video_signal) {
          if(is_bluray) {
            type_id = bluray_id;
            if(type_id < 0) {
              type_id = dvd_id;
            }
          }
          else {
            type_id = dvd_id;
          }
        }
        // Participants without a director also indicate video content.
        if(type_id < 0 && has_participants && !has_artist) {
          if(is_bluray) {
            type_id = bluray_id;
          }
          if(type_id < 0) {
            type_id = dvd_id;
          }
        }
        if(type_id >= 0) {
          item.set_type_id(type_id);
          if(settings.value("download_thumbnails", false).toBool() &&
             (item.get_thumbnail_id() < 0 ||
              !data.queries_->have_local_thumbnail(item.get_thumbnail_id())))
          {
            int tid =
                data.queries_->use_local_thumbnail(QString("ean_") + ean);
            if(tid < 0) {
              QString const url = data.ean_queries_.get_thumbnail_url();
              if(!url.isEmpty()) {
                QPixmap const pm = data.ean_queries_.get_thumbnail();
                if(pm.width() > 0) {
                  QByteArray png;
                  QBuffer buf(&png);
                  buf.open(QIODevice::WriteOnly);
                  pm.save(&buf, "PNG");
                  tid = data.queries_->store_thumbnail(
                      png, QString("ean_") + ean, url, false);
                }
              }
            }
            if(tid >= 0) {
              item.set_thumbnail_id(tid);
            }
          }
          show_select_page_with_item(data, item, type_id);
          return true;
        }
      }
    }

    // opengtindb fallback: used when DNB identified the item as a game, or
    // when DNB returned no data at all.  Skip when DNB has data with no game
    // markers to avoid false look-ups for unrecognised AV items.
    if(is_game_from_dnb || dnb_title.isEmpty()) {
      int const board_game_id =
          data.queries_->type_id_for_media_kind(colman::MediaKind::BoardGame);
      if(board_game_id >= 0) {
        if(settings.value("use_opengtindb", false).toBool()) {
          auto const opt = colman::query_opengtindb_ean(
              settings.value("opengtindb_userid", "").toString(), ean);
          if(opt.has_value()) {
            colman::Item item = *opt;
            item.set_value("ean", ean);
            show_select_page_with_item(data, item, board_game_id);
            return true;
          }
        }
        // opengtindb disabled or no data: pass best available name as prefill
        // so the step-2 search field shows something meaningful.
        if(prefill && !dnb_title.isEmpty()) {
          *prefill = dnb_title;
        }
      }
    }
    // When DNB/MusicBrainz found the item but no known collection type maps to
    // it, show the data on the select page so the user can pick the type and proceed.
    if(!dnb_item.get_name().isEmpty()) {
      int const fallback_type_id =
          is_game_from_dnb
              ? data.queries_->type_id_for_media_kind(
                    colman::MediaKind::BoardGame)
              : -1;
      show_select_page_with_item(data, dnb_item, fallback_type_id);
      return true;
    }
    if(prefill && prefill->isEmpty() && !dnb_title.isEmpty()) {
      *prefill = dnb_title;
    }

    return false;
  }
} // namespace

/**
 * One-time initialisation of the acquisition wizard pages. Sets up the
 * sort proxy model and column resize behaviour for the item table view on
 * the select page, configures the item widget, installs a ClearSelectionFilter event
 * filter on the table viewport, and connects all wizard navigation buttons
 * (Previous, Cancel, Finish, Yes/No on the completion page), the barcode/ISBN
 * search field, the internet-lookup button, and the main toolbar Acquisition
 * button to their respective handler lambdas.
 * @param app  The Qt application object used for signal/slot connections.
 * @param data Central application state.
 */
void colman::setup_acquisition_pages(QApplication& app,
                                     colman::AppData& data)
{
  QSortFilterProxyModel* sqlproxy =
      new QSortFilterProxyModel(data.mw_.acquisition_select_items_view_);
  sqlproxy->setSourceModel(&data.acquisition_item_model_);
  sqlproxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  data.mw_.acquisition_select_items_view_->setModel(sqlproxy);
  // Scroll smoothly (per pixel) so a wide name column can be
  // panned into view instead of jumping column by column.
  data.mw_.acquisition_select_items_view_->setHorizontalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.acquisition_select_items_view_->setVerticalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  data.mw_.acquisition_select_items_view_->horizontalHeader()
      ->setSectionResizeMode(QHeaderView::ResizeToContents);
  data.mw_.acquisition_select_items_view_->viewport()->installEventFilter(
      new ClearSelectionFilter(data.mw_.acquisition_select_items_view_));
  data.mw_.acquisition_select_items_view_->clearSelection();
  data.mw_.acquisition_select_item_->setup(data);
  data.mw_.acquisition_select_item_->set_read_only(false);
  data.mw_.acquisition_select_item_->hide_type_selector();
  app.connect(data.mw_.acquisition_select_type_,
              QOverload<int>::of(&QComboBox::currentIndexChanged),
              [&data]() {
                // Save the field values before refresh_item_query(): the
                // model reset clears any selection, which in turn clears
                // the item widget via deselect_item().
                auto const saved =
                    data.mw_.acquisition_select_item_->save_field_values();
                refresh_item_query(data);
                int const type_id =
                    data.mw_.acquisition_select_type_->currentData().toInt();
                data.mw_.acquisition_select_item_->reset_item_type(type_id);
                data.mw_.acquisition_select_item_->restore_field_values(
                    saved);
              });
  app.connect(
      data.mw_.acquisition_select_items_view_->selectionModel(),
      &QItemSelectionModel::selectionChanged,
      [&data](QItemSelection const& selected, QItemSelection const&) {
        auto const index_list = selected.indexes();
        bool const has_selection = !index_list.isEmpty();
        if(has_selection) {
          QModelIndex const& model_index = index_list.front();
          QAbstractItemModel const* model = model_index.model();
          if((model)) {
            int const row = model_index.row();
            QVariant const id =
                model->data(model->index(row, 0), Qt::DisplayRole);
            select_item(data, id.toInt());
          }
        }
        else {
          deselect_item(data);
        }
        data.mw_.acquisition_select_more_copies_->setVisible(has_selection);
        data.mw_.acquisition_select_more_copies_->setEnabled(has_selection);
        bool const new_item_enabled =
            !has_selection && data.mw_.acquisition_select_item_->has_name();
        data.mw_.acquisition_select_new_item_->setVisible(new_item_enabled);
      });
  app.connect(
      data.mw_.acquisition_select_item_, &colman::ItemWidget::field_changed,
      [&data]() {
        bool const has_selection =
            data.mw_.acquisition_select_items_view_->selectionModel()
                ->hasSelection();
        data.mw_.acquisition_select_new_item_->setVisible(
            !has_selection && data.mw_.acquisition_select_item_->has_name());
      });
  app.connect(data.mw_.acquisition_select_back_, &QPushButton::clicked,
              [&data]() { leave_acquisition_pages(data); });
  app.connect(data.mw_.acquisition_count_back_, &QPushButton::clicked,
              [&data]() { goto_select_page(data); });
  app.connect(data.mw_.acquisition_select_new_item_, &QPushButton::clicked,
              [&data]() { goto_count_page(data); });
  app.connect(data.mw_.acquisition_select_more_copies_, &QPushButton::clicked,
              [&data]() { goto_count_page(data); });
  app.connect(data.mw_.acquisition_select_load_more_, &QPushButton::clicked,
              [&data]() { acquisition_load_more(data); });
  app.connect(data.mw_.acquisition_select_clear_, &QPushButton::clicked,
              [&data]() {
                deselect_item(data);
                // Without a selection clearSelection() emits no
                // selectionChanged, so hide the buttons explicitly.
                data.mw_.acquisition_select_new_item_->hide();
                data.mw_.acquisition_select_more_copies_->hide();
              });
  app.connect(data.mw_.acquisition_select_internet_search_,
              &QPushButton::clicked, [&data]() {
                // Free text in the search field serves as title when the
                // item's name is empty; barcodes are handled by
                // search_barcode() instead.
                QString search =
                    data.mw_.acquisition_select_search_->text().trimmed();
                if(colman::is_product_ean(search) ||
                   colman::is_upc_a(search) || colman::is_isbn10(search) ||
                   colman::is_isbn13(search))
                {
                  search.clear();
                }
                data.mw_.acquisition_select_item_->internet_lookup(search);
              });
  app.connect(data.mw_.acquisition_count_finish_, &QPushButton::clicked,
              [&data]() { goto_done_page(data); });
  app.connect(data.mw_.acquisition_done_yes_, &QPushButton::clicked,
              [&data]() { colman::enter_acquisition_page(data); });
  app.connect(data.mw_.acquisition_done_no_, &QPushButton::clicked,
              [&data]() { leave_acquisition_pages(data); });
  app.connect(data.mw_.acquisition_done_print_card_, &QPushButton::clicked,
              [&data]() {
                int const item_id = data.last_acquisition_item_id_;
                colman::enter_item_barcode_page_for_item(
                    data, item_id, [](colman::AppData& d) {
                      d.mw_.pages_stack_->setCurrentWidget(
                          d.mw_.acquisition_done_);
                    });
              });
  app.connect(data.mw_.acquisition_done_print_top_, &QPushButton::clicked,
              [&data]() {
                int const item_id = data.last_acquisition_item_id_;
                if(item_id < 1) {
                  return;
                }
                colman::Item item;
                QString type_name;
                if(!data.queries_->get_item(item_id, item, type_name)) {
                  return;
                }
                colman::print_signature_labels(item.get_signature_top(), 1,
                                               data);
              });
  app.connect(data.mw_.acquisition_done_print_bottom_, &QPushButton::clicked,
              [&data]() {
                int const item_id = data.last_acquisition_item_id_;
                if(item_id < 1) {
                  return;
                }
                colman::Item item;
                QString type_name;
                if(!data.queries_->get_item(item_id, item, type_name)) {
                  return;
                }
                colman::print_signature_labels(item.get_signature_bottom(), 1,
                                               data);
              });
  app.connect(data.mw_.acquisition_select_search_, &QLineEdit::editingFinished,
              [&data]() {
                if(data.mw_.acquisition_select_search_->isModified()) {
                  data.mw_.acquisition_select_search_->setModified(false);
                  search_barcode(data);
                }
              });
  data.mw_.acquisition_select_camera_->setIcon(
      QIcon(":/icons/owls/scan_barcode.jpeg"));
  app.connect(data.mw_.acquisition_select_camera_, &QToolButton::clicked,
              [&data]() {
                colman::scan_barcode_into(
                    data, data.mw_.acquisition_select_search_);
              });
  if(QMediaDevices::videoInputs().isEmpty()) {
    data.mw_.acquisition_select_camera_->setEnabled(false);
  }

  app.connect(data.acquisition_button_, &QToolButton::clicked,
              [&data]() { enter_acquisition_page(data); });
}

/**
 * Prepares the acquisition wizard for a fresh transaction by
 * navigating to the select page (type combo is pre-selected when only
 * one item type exists).
 * @param data Central application state.
 */
void colman::enter_acquisition_page(AppData& data)
{
  data.mw_.acquisition_select_item_->set_completers();
  load_default_signatures(data);
  data.mw_.acquisition_select_search_->clear();
  goto_select_page(data);
}

/**
 * Refreshes the currently visible acquisition wizard page.
 * Determines which page is active and re-runs the
 * appropriate query or navigation function so that the displayed data reflects
 * the current database state.
 * @param data Central application state.
 */
void colman::refresh_acquisition_pages(colman::AppData& data)
{
  QWidget const* const current = data.mw_.pages_stack_->currentWidget();
  if(current == data.mw_.acquisition_select_) {
    goto_select_page(data);
    search_barcode(data);
  }
}

/**
   Enters the acquisition wizard pre-populated from \a ai.

   For \a ai.item_id_ >= 0 the item's type is looked up and pre-selected
   in the type ComboBox so that the select page shows the correct item set.
   The wizard advances directly to the select page and the item row is selected.

   For \a ai.item_id_ == -1 the wizard goes to the select page with the
   collection's default type; \a ai.barcode_ is placed in the search
   field and the search is triggered so that an internet look-up fires
   for unknown ISBNs.
   @param data  central application state.
   @param ai    Acquisition action produced by possible_actions().
*/
void colman::enter_acquisition_page_for_action(
    colman::AppData& data, colman::ActionInfo const& ai)
{
  data.mw_.acquisition_select_item_->set_completers();
  load_default_signatures(data);

  if(ai.item_id_ >= 0) {
    colman::Item item;
    QString type_name;
    int item_type_id = -1;
    if(data.queries_->get_item(ai.item_id_, item, type_name)) {
      item_type_id = item.get_type_id();
    }
    goto_select_page(data, item_type_id);
    colman::select_and_scroll_to(data.mw_.acquisition_select_items_view_, 0,
                                 ai.item_id_);
  }
  else {
    bool is_isbn = false;
    bool is_possible_ean = false;
    QString prefill;
    if(!ai.barcode_.isEmpty()) {
      // is_isbn13() only checks the check digit, not the 978/979 prefix.
      // Product EANs can accidentally pass the checksum, so exclude them.
      is_possible_ean = colman::is_product_ean(ai.barcode_) ||
                        colman::is_upc_a(ai.barcode_);
      is_isbn = !is_possible_ean && (colman::is_isbn10(ai.barcode_) ||
                                     colman::is_isbn13(ai.barcode_));
      if(!is_isbn) {
        // For product EAN/UPC try immediate lookup to infer the type.
        QString ean;
        if(colman::is_product_ean(ai.barcode_)) {
          ean = ai.barcode_;
        }
        else if(colman::is_upc_a(ai.barcode_)) {
          ean = QString("0") + ai.barcode_;
        }
        bool const auto_ok = try_ean_auto_lookup(data, ean, &prefill);
        if(auto_ok) {
          return;
        }
      }
    }
    goto_select_page(data);
    if(!ai.barcode_.isEmpty()) {
      // Prefer a human-readable name from an internet lookup over the raw EAN.
      QString const search_text =
          (prefill.isEmpty() || is_isbn) ? ai.barcode_ : prefill;
      data.mw_.acquisition_select_search_->setText(search_text);
      // try_ean_auto_lookup returned false (no hit), so trigger the
      // internet search via the search field regardless of barcode type.
      if(data.mw_.pages_stack_->currentWidget() == data.mw_.acquisition_select_) {
        data.mw_.acquisition_select_search_->setModified(true);
        Q_EMIT data.mw_.acquisition_select_search_->editingFinished();
      }
    }
  }
}

/* EOF */
