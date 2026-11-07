// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Entry point for the colman REST client.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_REST_QUERIES_HPP
#  include "colman_rest_queries.hpp"
#endif
#ifndef COLMAN_CENTRAL_PAGE_HPP
#  include "colman_central_page.hpp"
#endif
#ifndef COLMAN_CAMERA_SCAN_PAGE_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_ACQUISITION_PAGE_HPP
#  include "colman_acquisition_page.hpp"
#endif
#ifndef COLMAN_LENDING_PAGE_HPP
#  include "colman_lending_page.hpp"
#endif
#ifndef COLMAN_GROUP_LENDING_PAGE_HPP
#  include "colman_group_lending_page.hpp"
#endif
#ifndef COLMAN_LIQUIDATION_PAGE_HPP
#  include "colman_liquidation_page.hpp"
#endif
#ifndef COLMAN_RETURN_PAGE_HPP
#  include "colman_return_page.hpp"
#endif
#ifndef COLMAN_HISTORY_PAGE_HPP
#  include "colman_history_page.hpp"
#endif
#ifndef COLMAN_ITEM_BARCODE_PAGE_HPP
#  include "colman_item_barcode_page.hpp"
#endif
#ifndef COLMAN_ITEM_TYPE_PAGE_HPP
#  include "colman_item_type_page.hpp"
#endif
#ifndef COLMAN_PERSON_BARCODE_PAGE_HPP
#  include "colman_person_barcode_page.hpp"
#endif
#ifndef COLMAN_PERSON_PAGE_HPP
#  include "colman_person_page.hpp"
#endif
#ifndef COLMAN_PERSON_VIEW_PAGE_HPP
#  include "colman_person_view_page.hpp"
#endif
#ifndef COLMAN_PREFERENCES_PAGE_HPP
#  include "colman_preferences_page.hpp"
#endif
#ifndef COLMAN_SEARCH_PAGE_HPP
#  include "colman_search_page.hpp"
#endif
#ifndef COLMAN_SETTINGS_PAGE_HPP
#  include "colman_settings_page.hpp"
#endif
#ifndef COLMAN_CLASS_MANAGEMENT_PAGE_HPP
#  include "colman_class_management_page.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QKEYEVENT
#  include <QKeyEvent>
#  define SYSTEM_QKEYEVENT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QLIBRARYINFO
#  include <QLibraryInfo>
#  define SYSTEM_QLIBRARYINFO
#endif
#ifndef SYSTEM_QNETWORKPROXY
#  include <QNetworkProxy>
#  define SYSTEM_QNETWORKPROXY
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QTRANSLATOR
#  include <QTranslator>
#  define SYSTEM_QTRANSLATOR
#endif
#ifndef SYSTEM_QWHEELEVENT
#  include <QWheelEvent>
#  define SYSTEM_QWHEELEVENT
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_REST_API_VERSION_HPP
#  include "colman_rest_api_version.hpp"
#endif
#ifndef COLMAN_MDNS_BROWSER_HPP
#  include "colman_mdns_browser.hpp"
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif
#ifndef SYSTEM_QSIZE
#  include <QSize>
#  define SYSTEM_QSIZE
#endif
#ifndef COLMAN_START_PAGE_HPP
#  include "colman_start_page.hpp"
#endif
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef SYSTEM_QACTION
#  include <QAction>
#  define SYSTEM_QACTION
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif

namespace
{

/// Maps an HTTP status code to a user-facing login-error message.
  QString friendly_rest_error(int status)
  {
    if(status == 401 || status == 403) {
      return QApplication::tr("Wrong username or password.");
    }
    if(status == 0) {
      return QApplication::tr(
          "Could not connect to server. Check the server URL.");
    }
    return QApplication::tr("Server error (status %1).").arg(status);
  }

/// Builds the technical detail string shown when the user requests details.
  QString rest_error_detail(colman::RestReply const& r)
  {
    if(r.status == 0) {
      return QApplication::tr("No response from server.");
    }
    if(!r.error_message.isEmpty()) {
      return QStringLiteral("HTTP %1 – %2")
          .arg(r.status)
          .arg(r.error_message);
    }
    return QStringLiteral("HTTP %1").arg(r.status);
  }

  void apply_font_size(int pt)
  {
    pt = qBound(8, pt, 24);
    QFont f = QApplication::font();
    f.setPointSize(pt);
    QApplication::setFont(f);
    QSettings().setValue("font_size", pt);
  }

/// Global event filter: Ctrl+Wheel and Ctrl++/−/0 change the application
/// font size.
  class FontSizeFilter : public QObject
  {
  public:
    explicit FontSizeFilter(QObject* parent = nullptr)
        : QObject(parent) {}

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
      if(event->type() == QEvent::Wheel) {
        auto* we = static_cast<QWheelEvent*>(event);
        if(we->modifiers() & Qt::ControlModifier) {
          if(inside_graphics_view(watched)) {
            return false;
          }
          int const delta = we->angleDelta().y() > 0 ? 1 : -1;
          apply_font_size(QApplication::font().pointSize() + delta);
          return true;
        }
      }
      if(event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if(ke->modifiers() & Qt::ControlModifier) {
          int const key = ke->key();
          if(key == Qt::Key_Plus || key == Qt::Key_Equal) {
            apply_font_size(QApplication::font().pointSize() + 1);
            return true;
          }
          if(key == Qt::Key_Minus) {
            apply_font_size(QApplication::font().pointSize() - 1);
            return true;
          }
          if(key == Qt::Key_0) {
            apply_font_size(12);
            return true;
          }
        }
      }
      return false;
    }

  private:
    static bool inside_graphics_view(QObject* obj)
    {
      QWidget* w = qobject_cast<QWidget*>(obj);
      if(!w) {
        w = QApplication::widgetAt(QCursor::pos());
      }
      while(w) {
        if(qobject_cast<QGraphicsView*>(w)) {
          return true;
        }
        w = w->parentWidget();
      }
      return false;
    }
  };

/// Polls /api/table-versions on a timer and refreshes the current page
/// when relevant change counters advance.
  class DbChangePoller
  {
  public:
    explicit DbChangePoller(colman::AppData& data)
        : data_(data)
    {
      QObject::connect(&timer_, &QTimer::timeout,
                       [this]() { on_tick(); });
    }

    void start()
    {
      auto const ms = QSettings().value(
                                     QStringLiteral("poll_interval_s"), 10)
                          .toInt() *
                      1000;
      timer_.start(ms);
    }

    /// Clears the stored versions so the next tick records a fresh baseline
    /// instead of triggering a spurious refresh.
    void reset_versions() { last_versions_.clear(); }

  private:
    void on_tick()
    {
      if(in_flight_) {
        return;
      }
      auto const ms = QSettings().value(
                                     QStringLiteral("poll_interval_s"), 10)
                          .toInt() *
                      1000;
      if(ms != timer_.interval()) {
        timer_.setInterval(ms);
      }
      auto const* current =
          data_.mw_.pages_stack_->currentWidget();
      auto const relevant = relevant_tables(current);
      if(relevant.isEmpty()) {
        return;
      }
      in_flight_ = true;
      auto const new_versions =
          data_.queries_->query_table_versions();
      in_flight_ = false;
      if(new_versions.isEmpty()) {
        if(server_reachable_) {
          server_reachable_ = false;
          QMessageBox::warning(
              &data_.w_,
              QApplication::tr("Server not reachable"),
              QApplication::tr(
                  "The server is currently not reachable. "
                  "The displayed data may be outdated."));
        }
        return;
      }
      if(!server_reachable_) {
        server_reachable_ = true;
      }
      if(last_versions_.isEmpty()) {
        last_versions_ = new_versions;
        return;
      }
      bool changed = false;
      for(auto const& tbl : relevant) {
        auto const new_it = new_versions.find(tbl);
        if(new_it == new_versions.end()) {
          continue;
        }
        auto const old_it = last_versions_.find(tbl);
        if(old_it == last_versions_.end() ||
           old_it.value() != new_it.value()) {
          changed = true;
          break;
        }
      }
      last_versions_ = new_versions;
      if(changed) {
        refresh_current_page(current);
      }
    }

    QStringList relevant_tables(QWidget const* page) const
    {
      auto const& mw = data_.mw_;
      if(page == mw.lending_person_) {
        return {QStringLiteral("persons")};
      }
      if(page == mw.lending_item_) {
        return {QStringLiteral("items"),
                QStringLiteral("acquisitions"),
                QStringLiteral("lendings")};
      }
      if(page == mw.lending_details_ ||
         page == mw.lending_confirm_) {
        return {QStringLiteral("lendings"),
                QStringLiteral("reservations"),
                QStringLiteral("items")};
      }
      if(page == mw.group_lending_item_) {
        return {QStringLiteral("items")};
      }
      if(page == mw.group_lending_persons_) {
        return {QStringLiteral("lendings"),
                QStringLiteral("reservations"),
                QStringLiteral("items")};
      }
      if(page == mw.return_item_) {
        return {QStringLiteral("items"),
                QStringLiteral("lendings")};
      }
      if(page == mw.return_borrower_) {
        return {QStringLiteral("persons"),
                QStringLiteral("lendings")};
      }
      if(page == mw.return_lending_) {
        return {QStringLiteral("lendings")};
      }
      if(page == mw.return_details_ ||
         page == mw.return_confirm_) {
        return {QStringLiteral("lendings"),
                QStringLiteral("returns")};
      }
      if(page == mw.acquisition_select_ ||
         page == mw.acquisition_count_ ||
         page == mw.acquisition_done_) {
        return {QStringLiteral("items"),
                QStringLiteral("acquisitions")};
      }
      if(page == mw.liquidation_item_ ||
         page == mw.liquidation_count_ ||
         page == mw.liquidation_done_) {
        return {QStringLiteral("items"),
                QStringLiteral("liquidations")};
      }
      if(page == mw.person_view_page_) {
        return {QStringLiteral("persons"),
                QStringLiteral("lendings")};
      }
      if(page == mw.history_page_) {
        return {QStringLiteral("items"),
                QStringLiteral("persons"),
                QStringLiteral("lendings"),
                QStringLiteral("returns")};
      }
      if(page == mw.item_type_page_) {
        return {QStringLiteral("types")};
      }
      if(page == data_.search_item_detail_page_) {
        return {QStringLiteral("items"),
                QStringLiteral("lendings"),
                QStringLiteral("returns"),
                QStringLiteral("acquisitions"),
                QStringLiteral("liquidations"),
                QStringLiteral("reservations")};
      }
      return {};
    }

    void refresh_current_page(QWidget const* page)
    {
      auto const& mw = data_.mw_;
      if(page == mw.lending_person_ ||
         page == mw.lending_item_ ||
         page == mw.lending_details_ ||
         page == mw.lending_confirm_) {
        colman::refresh_lending_pages(data_);
      }
      else if(page == mw.group_lending_item_ ||
              page == mw.group_lending_persons_) {
        colman::refresh_group_lending_pages(data_);
      }
      else if(page == mw.return_item_ ||
              page == mw.return_borrower_ ||
              page == mw.return_lending_ ||
              page == mw.return_details_ ||
              page == mw.return_confirm_) {
        colman::refresh_return_pages(data_);
      }
      else if(page == mw.acquisition_select_ ||
              page == mw.acquisition_count_ ||
              page == mw.acquisition_done_) {
        colman::refresh_acquisition_pages(data_);
      }
      else if(page == mw.liquidation_item_ ||
              page == mw.liquidation_count_ ||
              page == mw.liquidation_done_) {
        colman::refresh_liquidation_pages(data_);
      }
      else if(page == mw.person_view_page_) {
        colman::refresh_person_view_page(data_);
      }
      else if(page == mw.history_page_) {
        colman::refresh_history_page(data_);
      }
      else if(page == mw.item_type_page_) {
        colman::refresh_item_type_page(data_);
      }
      else if(page == data_.search_item_detail_page_ &&
              data_.refresh_search_item_detail_) {
        data_.refresh_search_item_detail_();
      }
    }

    colman::AppData& data_;
    QTimer timer_;
    colman::IQueries::TableVersions last_versions_;
    bool in_flight_ = false;
    bool server_reachable_ = true;
  };

/// Sets up the REST login page (used at startup and for re-login).
/**
   Shows a LAN-discovery list, a credential form (server URL, username,
   password) and two buttons: "log in" (authenticates, checks the API
   version and switches to the central page) and "quit program".  The page
   is navigated to by colman::logout() and at startup when no credentials
   were supplied on the command line.
   @param app             Qt application object.
   @param data            central application state.
   @param prefill_server  initial server URL (else the stored one is used).
   @param prefill_username initial username.
   @param prefill_password initial password.
*/
  void setup_rest_login_page(QApplication& app, colman::AppData& data,
                             QString const& prefill_server = QString(),
                             QString const& prefill_username = QString(),
                             QString const& prefill_password = QString())
  {
    auto* page = new QWidget;
    page->setObjectName("login_rest_page_");
    auto* vbox = new QVBoxLayout(page);
    vbox->addStretch(1);

    // -- Logo --
    auto* logo_label = new QLabel(page);
    QPixmap const logo_pix(":/icons/owls/connection2.jpeg");
    if(!logo_pix.isNull()) {
      logo_label->setPixmap(
          logo_pix.scaledToHeight(96, Qt::SmoothTransformation));
      logo_label->setAlignment(Qt::AlignHCenter);
    }
    vbox->addWidget(logo_label);

    // -- LAN discovery --
    auto* disc_group = new QGroupBox(
        QApplication::tr("Discover on LAN"), page);
    auto* disc_inner = new QVBoxLayout(disc_group);
    auto* server_list = new QListWidget(disc_group);
    server_list->setSelectionMode(QAbstractItemView::SingleSelection);
    disc_inner->addWidget(server_list);
    auto* disc_row = new QHBoxLayout;
    auto* refresh_btn = new QPushButton(
        QApplication::tr("Refresh"), disc_group);
    auto* discover_status = new QLabel(
        QApplication::tr("Searching..."), disc_group);
    disc_row->addWidget(refresh_btn);
    disc_row->addWidget(discover_status);
    disc_row->addStretch(1);
    disc_inner->addLayout(disc_row);
    vbox->addWidget(disc_group);

    // -- Credential form --
    auto* group = new QGroupBox(QApplication::tr("Log In"), page);
    auto* form = new QFormLayout(group);
    auto* server_edit = new QLineEdit(group);
    server_edit->setPlaceholderText(QStringLiteral("http://host:8080"));
    auto* user_edit = new QLineEdit(group);
    auto* pw_edit = new QLineEdit(group);
    pw_edit->setEchoMode(QLineEdit::Password);
    auto* eye_act = pw_edit->addAction(
        QIcon::fromTheme(QStringLiteral("view-visible")),
        QLineEdit::TrailingPosition);
    eye_act->setCheckable(true);
    app.connect(eye_act, &QAction::toggled,
                [pw_edit, eye_act](bool show) {
                  pw_edit->setEchoMode(
                      show ? QLineEdit::Normal : QLineEdit::Password);
                  eye_act->setIcon(QIcon::fromTheme(
                      show ? QStringLiteral("view-hidden")
                           : QStringLiteral("view-visible")));
                });
    form->addRow(QApplication::tr("Server:"), server_edit);
    form->addRow(QApplication::tr("Username:"), user_edit);
    form->addRow(QApplication::tr("Password:"), pw_edit);
    auto* error_label = new QLabel(group);
    error_label->setStyleSheet("color: red");
    error_label->setTextInteractionFlags(
        Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    error_label->setVisible(false);
    form->addRow(error_label);
    auto* details_btn = new QPushButton(
        QApplication::tr("Details..."), group);
    details_btn->setVisible(false);
    form->addRow(details_btn);
    auto detail_text = std::make_shared<QString>();
    app.connect(details_btn, &QPushButton::clicked,
                [page, detail_text]() {
                  QMessageBox::information(
                      page,
                      QApplication::tr("Error Details"),
                      *detail_text);
                });
    vbox->addWidget(group);
    vbox->addSpacing(16);

    auto* hbox = new QHBoxLayout;
    hbox->addStretch(1);
    auto* login_btn = new QToolButton(page);
    login_btn->setText(QApplication::tr("log in"));
    login_btn->setIcon(QIcon(":/icons/owls/login.jpeg"));
    login_btn->setIconSize(QSize(128, 128));
    login_btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    login_btn->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(login_btn, 0, Qt::AlignTop);
    hbox->addStretch(1);
    auto* quit_btn = new QToolButton(page);
    quit_btn->setText(QApplication::tr("quit program"));
    quit_btn->setIcon(QIcon(":/icons/owls/exit2.jpeg"));
    quit_btn->setIconSize(QSize(128, 128));
    quit_btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    quit_btn->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(quit_btn, 0, Qt::AlignTop);
    hbox->addStretch(1);
    vbox->addLayout(hbox);
    vbox->addStretch(1);

    data.mw_.pages_stack_->addWidget(page);

    auto* rq0 =
        dynamic_cast<colman::RestQueries*>(data.queries_.get());
    int const timeout_ms = rq0 ? rq0->config().timeout_ms : 10000;

    // -- Initial values --
    server_edit->setText(
        prefill_server.isEmpty()
            ? QSettings().value(
                             QStringLiteral("login_server_url"))
                  .toString()
            : prefill_server);
    user_edit->setText(prefill_username);
    pw_edit->setText(prefill_password);

    auto update_login_enabled =
        [login_btn, server_edit, user_edit, pw_edit]() {
          login_btn->setEnabled(
              !server_edit->text().trimmed().isEmpty() &&
              !user_edit->text().trimmed().isEmpty() &&
              !pw_edit->text().isEmpty());
        };
    app.connect(server_edit, &QLineEdit::textChanged, page,
                [update_login_enabled]() { update_login_enabled(); });
    app.connect(user_edit, &QLineEdit::textChanged, page,
                [update_login_enabled]() { update_login_enabled(); });
    app.connect(pw_edit, &QLineEdit::textChanged, page,
                [update_login_enabled]() { update_login_enabled(); });
    update_login_enabled();

    // -- mDNS discovery --
    auto* no_result_timer = new QTimer(page);
    no_result_timer->setSingleShot(true);
    no_result_timer->setInterval(5000);
    app.connect(no_result_timer, &QTimer::timeout, page,
                [server_list, discover_status]() {
                  if(server_list->count() == 0) {
                    discover_status->setText(
                        QApplication::tr("No servers found."));
                  }
                });
    auto* browser = new colman::MdnsBrowser(page);
    app.connect(browser, &colman::MdnsBrowser::service_found, page,
                [server_list, discover_status, no_result_timer](
                    colman::MdnsBrowser::ServiceInfo const& info) {
                  discover_status->setText(QString());
                  no_result_timer->stop();
                  QString const label =
                      info.name + QStringLiteral("  (") + info.host +
                      QStringLiteral(":") + QString::number(info.port) +
                      QStringLiteral(")");
                  for(int i = 0; i < server_list->count(); ++i) {
                    auto* itm = server_list->item(i);
                    if(itm->data(Qt::UserRole).toString() == info.url()) {
                      itm->setText(label);
                      return;
                    }
                  }
                  auto* itm = new QListWidgetItem(label);
                  itm->setData(Qt::UserRole, info.url());
                  server_list->addItem(itm);
                });
    app.connect(browser, &colman::MdnsBrowser::service_removed, page,
                [server_list](QString const& name) {
                  for(int i = 0; i < server_list->count(); ++i) {
                    auto* itm = server_list->item(i);
                    if(itm->text().startsWith(name)) {
                      delete server_list->takeItem(i);
                      return;
                    }
                  }
                });
    app.connect(server_list, &QListWidget::itemClicked, page,
                [server_edit](QListWidgetItem* itm) {
                  server_edit->setText(itm->data(Qt::UserRole).toString());
                });
    app.connect(refresh_btn, &QPushButton::clicked, page,
                [server_list, discover_status, no_result_timer, browser]() {
                  server_list->clear();
                  discover_status->setText(QApplication::tr("Searching..."));
                  no_result_timer->start();
                  browser->start_browse();
                });

    // Reset fields and (re)start discovery whenever the page is shown.
    // The first show keeps the prefilled values; later shows (re-login)
    // clear the credentials and refresh the server URL from the config.
    auto first_show = std::make_shared<bool>(true);
    app.connect(
        data.mw_.pages_stack_, &QStackedWidget::currentChanged, page,
        [&data, page, server_edit, user_edit, pw_edit, error_label,
         details_btn, server_list, discover_status, no_result_timer,
         browser, update_login_enabled, first_show](int) {
          if(data.mw_.pages_stack_->currentWidget() != page) {
            return;
          }
          if(*first_show) {
            *first_show = false;
          }
          else {
            auto* rq =
                dynamic_cast<colman::RestQueries*>(data.queries_.get());
            if(rq && !rq->config().base_url.isEmpty()) {
              server_edit->setText(rq->config().base_url);
            }
            user_edit->clear();
            pw_edit->clear();
          }
          error_label->setVisible(false);
          details_btn->setVisible(false);
          server_list->clear();
          discover_status->setText(QApplication::tr("Searching..."));
          no_result_timer->start();
          browser->start_browse();
          update_login_enabled();
          if(user_edit->text().isEmpty()) {
            user_edit->setFocus();
          }
          else {
            pw_edit->setFocus();
          }
        });

    app.connect(quit_btn, &QToolButton::clicked,
                []() { QApplication::quit(); });

    // Pressing Return in the password field triggers the login button (a
    // no-op while a login is already in flight, as the button is disabled).
    app.connect(pw_edit, &QLineEdit::returnPressed,
                login_btn, &QAbstractButton::click);

    app.connect(
        login_btn, &QToolButton::clicked,
        [&data, server_edit, user_edit, pw_edit, error_label,
         details_btn, detail_text, login_btn, timeout_ms]() {
          error_label->setVisible(false);
          details_btn->setVisible(false);
          login_btn->setEnabled(false);
          colman::RestConfig cfg;
          cfg.base_url = server_edit->text().trimmed();
          cfg.username = user_edit->text().trimmed();
          cfg.password = pw_edit->text();
          cfg.timeout_ms = timeout_ms;
          auto* rq =
              dynamic_cast<colman::RestQueries*>(data.queries_.get());
          rq->set_config(cfg);
          auto const r = rq->test_connection();
          login_btn->setEnabled(true);
          if(r.status != 200) {
            error_label->setText(friendly_rest_error(r.status));
            error_label->setVisible(true);
            *detail_text = rest_error_detail(r);
            details_btn->setVisible(true);
            if(r.status == 401 || r.status == 403) {
              pw_edit->setFocus();
              pw_edit->selectAll();
            }
            return;
          }
          auto const v = rq->fetch_api_version();
          if(!colman::api_version_compatible(v)) {
            error_label->setText(
                QApplication::tr(
                    "The server's REST API version %1.%2 is not "
                    "compatible with this client (requires version "
                    "%3.x with x ≥ %4).")
                    .arg(v.major_)
                    .arg(v.minor_)
                    .arg(colman::ApiMajorVersion)
                    .arg(colman::ApiMinorVersion));
            error_label->setVisible(true);
            return;
          }
          QSettings().setValue("login_server_url", cfg.base_url);
          auto const pid = rq->fetch_logged_in_person_id();
          data.current_data_collector_ = (pid >= 0) ? pid : 0;
          colman::update_login_status(data);
          data.mw_.pages_stack_->setCurrentWidget(
              data.mw_.central_page_);
        });
  }

} // namespace

/**
   Entry point for the colman REST client.

   Connects to a running colman REST server, sets up all UI pages, and
   shows the central page directly (no local database or start page).

   If none of the credential options are provided, a login dialog is shown.

   Optional arguments:
   - `--server <url>`    Base URL of the colman REST server
   - `--username <name>` Username for HTTP Basic Auth
   - `--password <pw>`   Password for HTTP Basic Auth
   - `--token <jwt>`     Bearer JWT (takes priority over username/password)
   - `--timeout-ms <N>`  HTTP request timeout in milliseconds (default 10000)

   If `--server` and `--token` are both given, JWT is used and the dialog is
   skipped.  If `--server`, `--username`, and `--password` are all given,
   Basic Auth is used and the dialog is skipped.

   @param argc command-line argument count.
   @param argv command-line argument values.
   @return Qt application exit code.
*/
int main(int argc, char* argv[])
{
  colman::set_trace_level(0);
  QCoreApplication::setOrganizationName("cpesch");
  QCoreApplication::setApplicationName("colman-client");
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/icons/owls/colman_logo.jpeg"));

  {
    QTranslator* t_qt = new QTranslator(&app);
    static_cast<void>(
        t_qt->load(QLocale::system(), "qtbase", "_",
                   QLibraryInfo::path(QLibraryInfo::TranslationsPath),
                   ".qm"));
    app.installTranslator(t_qt);
    QTranslator* t = new QTranslator(&app);
    static_cast<void>(
        t->load(QLocale::system(), "colman", "_", ":/translations", ".qm"));
    app.installTranslator(t);
  }

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QStringLiteral("colman REST client"));
  auto const help_opt = parser.addHelpOption();
  QCommandLineOption server_opt(
      QStringLiteral("server"),
      QStringLiteral("Base URL of the colman REST server "
                     "(e.g. https://host:8443)"),
      QStringLiteral("url"));
  QCommandLineOption username_opt(
      QStringLiteral("username"),
      QStringLiteral("Username for HTTP Basic Auth"),
      QStringLiteral("name"));
  QCommandLineOption password_opt(
      QStringLiteral("password"),
      QStringLiteral("Password for HTTP Basic Auth"),
      QStringLiteral("pw"));
  QCommandLineOption token_opt(
      QStringLiteral("token"),
      QStringLiteral("Bearer JWT for authenticating requests "
                     "(takes priority over username/password)"),
      QStringLiteral("jwt"));
  QCommandLineOption timeout_opt(
      QStringLiteral("timeout-ms"),
      QStringLiteral("HTTP request timeout in milliseconds (default 10000)"),
      QStringLiteral("ms"),
      QStringLiteral("10000"));
  parser.addOption(server_opt);
  parser.addOption(username_opt);
  parser.addOption(password_opt);
  parser.addOption(token_opt);
  QCommandLineOption trace_level_opt(
      QStringLiteral("trace-level"),
      QStringLiteral("Print diagnostic traces on the console "
                     "(0 = silent, 2 = internet search; default: 0)"),
      QStringLiteral("number"),
      QStringLiteral("0"));
  parser.addOption(timeout_opt);
  parser.addOption(trace_level_opt);
  // parse() does not call exit() for --help; handle it manually so that
  // return 0 properly unwinds the stack and ~QApplication() is called,
  // avoiding "QThreadStorage: entry N destroyed before end of thread".
  parser.parse(app.arguments());
  if(parser.isSet(help_opt)) {
    fprintf(stdout, "%s", qPrintable(parser.helpText()));
    return 0;
  }
  if(!colman::set_trace_level(parser.value(trace_level_opt))) {
    fprintf(stderr, "Invalid trace level: %s\n",
            qPrintable(parser.value(trace_level_opt)));
    return 1;
  }

  bool const has_jwt =
      parser.isSet(server_opt) && parser.isSet(token_opt);
  bool const has_basic =
      parser.isSet(server_opt) &&
      parser.isSet(username_opt) &&
      parser.isSet(password_opt);

  colman::RestConfig config;
  config.timeout_ms = parser.value(timeout_opt).toInt();

  if(has_jwt) {
    config.base_url = parser.value(server_opt);
    config.token = parser.value(token_opt);
  }
  else if(has_basic) {
    config.base_url = parser.value(server_opt);
    config.username = parser.value(username_opt);
    config.password = parser.value(password_opt);
  }
  // When credentials are supplied on the command line we connect straight
  // away; otherwise the REST login page (set up below) handles authentication
  // in-window, so the queries object starts unconnected.
  bool const have_creds = has_jwt || has_basic;
  auto rq = std::make_unique<colman::RestQueries>();
  if(have_creds) {
    rq->set_config(config);
    auto const v = rq->fetch_api_version();
    if(!colman::api_version_compatible(v)) {
      QMessageBox::critical(
          nullptr,
          QApplication::tr("API Version Mismatch"),
          QApplication::tr(
              "The server's REST API version %1.%2 is not compatible "
              "with this client (requires version %3.x with x ≥ %4).")
              .arg(v.major_)
              .arg(v.minor_)
              .arg(colman::ApiMajorVersion)
              .arg(colman::ApiMinorVersion));
      return 1;
    }
  }


  {
    QSettings settings;
    QVariant const stored = settings.value("font_size");
    if(stored.isValid()) {
      QFont f = app.font();
      f.setPointSize(stored.toInt());
      app.setFont(f);
    }
  }
  app.installEventFilter(new FontSizeFilter(&app));

  int const logged_in_person_id =
      have_creds ? rq->fetch_logged_in_person_id() : -1;
  colman::AppData data(std::move(rq));
  QNetworkProxyFactory::setUseSystemConfiguration(true);
  data.mw_.setup_ui(data.w_);
  data.w_.setMinimumSize(880, 600);
  data.w_.resize(1024, 824);

  data.current_data_collector_ =
      (logged_in_person_id >= 0) ? logged_in_person_id : 0;

  // Set up all UI pages (same order as start_page after opening a file).
  colman::setup_central_page(app, data);
  colman::setup_history_page(app, data);
  colman::setup_person_page(app, data);
  colman::setup_person_view_page(app, data);
  colman::setup_person_barcode_page(app, data);
  colman::setup_item_barcode_page(app, data);
  colman::setup_item_type_page(app, data);
  colman::setup_camera_scan_page(app, data);
  colman::setup_acquisition_pages(app, data);
  colman::setup_liquidation_pages(app, data);
  colman::setup_lending_pages(app, data);
  colman::setup_group_lending_pages(app, data);
  colman::setup_return_pages(app, data);
  colman::setup_settings_page(app, data);
  colman::setup_search_page(app, data);
  colman::setup_class_management_pages(app, data);
  colman::setup_preferences_page(app, data);
  setup_rest_login_page(
      app, data,
      parser.isSet(server_opt) ? parser.value(server_opt) : QString(),
      parser.isSet(username_opt) ? parser.value(username_opt) : QString(),
      parser.isSet(password_opt) ? parser.value(password_opt) : QString());

  DbChangePoller poller(data);
  QObject::connect(
      data.mw_.pages_stack_, &QStackedWidget::currentChanged,
      [&poller](int) { poller.reset_versions(); });
  poller.start();

  if(have_creds) {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
  }
  else {
    // No command-line credentials: show the login page in-window.
    QWidget* login_page =
        data.mw_.pages_stack_->findChild<QWidget*>(
            "login_rest_page_", Qt::FindDirectChildrenOnly);
    if(login_page) {
      data.mw_.pages_stack_->setCurrentWidget(login_page);
    }
  }

  data.w_.show();
  return app.exec();
}

/* EOF */
