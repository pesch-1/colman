// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_REST_SERVER_HPP
#  include "colman_rest_server.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#  define COLMAN_DB_UTIL_HPP
#endif
#ifndef SYSTEM_QHTTPSERVERREQUEST
#  include <QHttpServerRequest>
#  define SYSTEM_QHTTPSERVERREQUEST
#endif
#ifndef SYSTEM_QHTTPSERVERRESPONSE
#  include <QHttpServerResponse>
#  define SYSTEM_QHTTPSERVERRESPONSE
#endif
#ifndef SYSTEM_QTCPSERVER
#  include <QTcpServer>
#  define SYSTEM_QTCPSERVER
#endif
#ifndef SYSTEM_QSSLSERVER
#  include <QSslServer>
#  define SYSTEM_QSSLSERVER
#endif
#ifndef SYSTEM_QSSLCONFIGURATION
#  include <QSslConfiguration>
#  define SYSTEM_QSSLCONFIGURATION
#endif
#ifndef SYSTEM_QSSLCERTIFICATE
#  include <QSslCertificate>
#  define SYSTEM_QSSLCERTIFICATE
#endif
#ifndef SYSTEM_QSSLKEY
#  include <QSslKey>
#  define SYSTEM_QSSLKEY
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QHOSTADDRESS
#  include <QHostAddress>
#  define SYSTEM_QHOSTADDRESS
#endif
#ifndef SYSTEM_QDEBUG
#  include <QDebug>
#  define SYSTEM_QDEBUG
#endif
#ifndef SYSTEM_QJSONARRAY
#  include <QJsonArray>
#  define SYSTEM_QJSONARRAY
#endif
#ifndef SYSTEM_QJSONDOCUMENT
#  include <QJsonDocument>
#  define SYSTEM_QJSONDOCUMENT
#endif
#ifndef SYSTEM_QJSONOBJECT
#  include <QJsonObject>
#  define SYSTEM_QJSONOBJECT
#endif
#ifndef SYSTEM_QURLQUERY
#  include <QUrlQuery>
#  define SYSTEM_QURLQUERY
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#  ifndef SYSTEM_QHTTPHEADERS
#    include <QHttpHeaders>
#    define SYSTEM_QHTTPHEADERS
#  endif
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef COLMAN_AUTH_MIDDLEWARE_HPP
#  include "colman_auth_middleware.hpp"
#endif
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef COLMAN_PERMISSION_DEPS_HPP
#  include "colman_permission_deps.hpp"
#endif
#ifndef COLMAN_ID_CARD_LAYOUT_HPP
#  include "colman_id_card_layout.hpp"
#endif
#ifndef COLMAN_JSON_HPP
#  include "colman_json.hpp"
#endif
#ifndef COLMAN_REST_API_VERSION_HPP
#  include "colman_rest_api_version.hpp"
#endif

namespace
{

  using SC = QHttpServerResponse::StatusCode;
  using Method = QHttpServerRequest::Method;

  QLocale::Language language_from_request(QHttpServerRequest const& req)
  {
    auto const hdr = QString::fromLatin1(req.value("Accept-Language"))
                         .split(',').first()
                         .split(';').first()
                         .trimmed();
    if(hdr.isEmpty()) {
      return QLocale::AnyLanguage;
    }
    auto const lang = QLocale(hdr).language();
    if(lang == QLocale::C || lang == QLocale::AnyLanguage) {
      return QLocale::AnyLanguage;
    }
    return lang;
  }

  QHttpServerResponse json_resp(
      QJsonObject const& obj,
      SC status = SC::Ok)
  {
    return QHttpServerResponse(obj, status);
  }

  /// Returns a JSON response with an ETag header set to \a version.
  /**
   * Used for responses that support optimistic concurrency control so clients
   * can send the value back as the `If-Match` header on subsequent PUT
   * requests.
   * @param obj      the JSON body
   * @param version  value from the row's `version` column in the database
   * @return a 200 OK JSON response with `ETag: <version>`
   */
  QHttpServerResponse json_resp_with_etag(QJsonObject const& obj, int version)
  {
    QHttpServerResponse resp(obj);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    auto h = resp.headers();
    h.replaceOrAppend(QHttpHeaders::WellKnownHeader::ETag,
                      QString::number(version).toUtf8());
    resp.setHeaders(std::move(h));
#else
    resp.setHeader(QByteArray("ETag"), QString::number(version).toUtf8());
#endif
    return resp;
  }

  QHttpServerResponse json_resp(QJsonArray const& arr)
  {
    return QHttpServerResponse(arr);
  }

  QHttpServerResponse json_id(int id)
  {
    return json_resp(
        QJsonObject{{QStringLiteral("id"), id}});
  }

  QHttpServerResponse json_value(int v)
  {
    return json_resp(
        QJsonObject{{QStringLiteral("value"), v}});
  }

  QHttpServerResponse json_value(bool v)
  {
    return json_resp(
        QJsonObject{{QStringLiteral("value"), v}});
  }

  QHttpServerResponse json_value(QString const& v)
  {
    return json_resp(
        QJsonObject{{QStringLiteral("value"), v}});
  }

/// Returns a JSON error response: {"error": \a msg} with \a status.
  QHttpServerResponse json_error(QString const& msg, SC status)
  {
    return json_resp(
        QJsonObject{{QStringLiteral("error"), msg}}, status);
  }

/// Converts a query-item string to int;
/// returns \a default_val when the string is empty.
  int query_int(QString const& s, int default_val = 0)
  {
    if(s.isEmpty()) {
      return default_val;
    }
    return s.toInt();
  }

  int if_match_version(QHttpServerRequest const& req)
  {
    auto const v = req.value("If-Match");
    if(v.isEmpty()) {
      return -1;
    }
    return QString::fromUtf8(v).toInt();
  }

  QJsonObject parse_body(QHttpServerRequest const& req)
  {
    return QJsonDocument::fromJson(req.body()).object();
  }

  QJsonArray parse_body_array(
      QHttpServerRequest const& req)
  {
    return QJsonDocument::fromJson(req.body()).array();
  }

  QHttpServerResponse versioned_response(
      colman::IQueries::UpdateResult result)
  {
    using R = colman::IQueries::UpdateResult;
    switch(result) {
    case R::Ok:
      return SC::Ok;
    case R::VersionMismatch:
      return json_error(
          QStringLiteral("Version conflict, please reload"),
          SC::PreconditionFailed);
    case R::NotFound:
      return json_error(
          QStringLiteral("Not found"), SC::NotFound);
    default:
      return json_error(
          QStringLiteral("Internal server error"),
          SC::InternalServerError);
    }
  }

  QJsonObject item_list_row_to_json(
      colman::ItemListModel::Row const& r)
  {
    QJsonObject o;
    o[QStringLiteral("id")] = r.id;
    o[QStringLiteral("name")] = r.name;
    o[QStringLiteral("count")] = r.count;
    o[QStringLiteral("available")] = r.available;
    o[QStringLiteral("type_translated")] =
        r.type_translated;
    o[QStringLiteral("type_name")] = r.type_name;
    o[QStringLiteral("collector")] = r.collector;
    return o;
  }

  QJsonObject open_lending_row_to_json(
      colman::OpenLendingListModel::Row const& r)
  {
    QJsonObject o;
    o[QStringLiteral("id")] = r.id;
    o[QStringLiteral("lent_count")] = r.lent_count;
    o[QStringLiteral("returned_count")] =
        r.returned_count;
    o[QStringLiteral("datetime")] =
        r.datetime.toString(Qt::ISODate);
    o[QStringLiteral("duedate")] =
        r.duedate.toString(Qt::ISODate);
    o[QStringLiteral("remarks")] = r.remarks;
    o[QStringLiteral("person_name")] = r.person_name;
    return o;
  }

  QJsonObject history_row_to_json(
      colman::HistoryCombinedModel::Row const& r)
  {
    QJsonObject o;
    o[QStringLiteral("id")] = r.id;
    o[QStringLiteral("datetime")] =
        r.datetime.toString(Qt::ISODate);
    o[QStringLiteral("action")] = r.action;
    o[QStringLiteral("item_name")] = r.item_name;
    o[QStringLiteral("count")] = r.count;
    o[QStringLiteral("user_name")] = r.user_name;
    o[QStringLiteral("collector")] = r.collector;
    o[QStringLiteral("item_id")] = r.item_id;
    o[QStringLiteral("person_id")] = r.person_id;
    o[QStringLiteral("remarks")] = r.remarks;
    o[QStringLiteral("collector_id")] = r.collector_id;
    o[QStringLiteral("duedate")] =
        r.duedate.toString(Qt::ISODate);
    return o;
  }

  colman::HistoryCombinedModel::Row history_row_from_json(
      QJsonObject const& o)
  {
    colman::HistoryCombinedModel::Row r;
    r.id = o[QStringLiteral("id")].toInt();
    r.datetime = QDateTime::fromString(
        o[QStringLiteral("datetime")].toString(),
        Qt::ISODate);
    r.action = o[QStringLiteral("action")].toString();
    r.item_name =
        o[QStringLiteral("item_name")].toString();
    r.count = o[QStringLiteral("count")].toInt();
    r.user_name =
        o[QStringLiteral("user_name")].toString();
    r.collector =
        o[QStringLiteral("collector")].toString();
    r.item_id = o[QStringLiteral("item_id")].toInt();
    r.person_id =
        o[QStringLiteral("person_id")].toInt();
    r.remarks = o[QStringLiteral("remarks")].toString();
    r.collector_id =
        o[QStringLiteral("collector_id")].toInt();
    r.duedate = QDate::fromString(
        o[QStringLiteral("duedate")].toString(),
        Qt::ISODate);
    return r;
  }

  QJsonObject start_page_row_to_json(
      colman::StartPagePersonListModel::Row const& r)
  {
    QJsonObject o;
    o[QStringLiteral("id")] = r.id;
    o[QStringLiteral("lastname")] = r.lastname;
    o[QStringLiteral("firstname")] = r.firstname;
    o[QStringLiteral("title")] = r.title;
    o[QStringLiteral("has_password")] = r.has_password;
    return o;
  }

  QJsonObject due_lending_row_to_json(
      colman::DueLendingListModel::Row const& r)
  {
    QJsonObject o;
    o[QStringLiteral("lending_id")] = r.lending_id;
    o[QStringLiteral("item_id")] = r.item_id;
    o[QStringLiteral("person_id")] = r.person_id;
    o[QStringLiteral("duedate")] =
        r.duedate.toString(Qt::ISODate);
    // person_name keeps the "name (class)" format of API minor <= 3 for
    // older clients; school_class (minor 4) carries the class separately.
    o[QStringLiteral("person_name")] = r.person_with_class();
    o[QStringLiteral("school_class")] = r.school_class;
    o[QStringLiteral("outstanding_count")] =
        r.outstanding_count;
    o[QStringLiteral("item_name")] = r.item_name;
    return o;
  }

  QJsonObject search_item_row_to_json(
      colman::IQueries::SearchItemRow const& r)
  {
    QJsonObject o;
    o[QStringLiteral("id")] = r.id;
    o[QStringLiteral("name")] = r.name;
    o[QStringLiteral("type_locale")] = r.type_locale;
    o[QStringLiteral("count")] = r.count;
    o[QStringLiteral("available")] = r.available;
    o[QStringLiteral("type_id")] = r.type_id;
    return o;
  }

  QJsonObject lending_info_to_json(
      colman::IQueries::LendingInfo const& info)
  {
    QJsonObject o;
    o[QStringLiteral("lending_id")] = info.lending_id_;
    o[QStringLiteral("lending_count")] =
        info.lending_count_;
    o[QStringLiteral("item_name")] = info.item_name_;
    return o;
  }

  QJsonObject lending_person_info_to_json(
      colman::IQueries::LendingPersonInfo const& info)
  {
    QJsonObject o;
    o[QStringLiteral("lending_id")] = info.lending_id_;
    o[QStringLiteral("person_last_name")] =
        info.person_last_name_;
    o[QStringLiteral("person_first_name")] =
        info.person_first_name_;
    o[QStringLiteral("person_title")] =
        info.person_title_;
    return o;
  }

  QJsonObject affected_person_to_json(
      colman::IQueries::AffectedPerson const& p)
  {
    QJsonObject o;
    o[QStringLiteral("person_id")] = p.person_id_;
    o[QStringLiteral("display_name")] = p.display_name_;
    o[QStringLiteral("old_class")] = p.old_class_;
    o[QStringLiteral("new_class")] = p.new_class_;
    return o;
  }

  QJsonObject id_card_element_to_json(
      colman::IdCardElement const& e)
  {
    QJsonObject o;
    o[QStringLiteral("orientation")] = e.orientation_;
    o[QStringLiteral("x_mm")] = e.x_mm_;
    o[QStringLiteral("y_mm")] = e.y_mm_;
    o[QStringLiteral("width_mm")] = e.width_mm_;
    o[QStringLiteral("height_mm")] = e.height_mm_;
    return o;
  }

  QJsonObject id_card_layout_to_json(
      colman::IdCardLayout const& lay)
  {
    QJsonObject o;
    o[QStringLiteral("card_width_mm")] = lay.card_width_mm_;
    o[QStringLiteral("card_height_mm")] = lay.card_height_mm_;
    o[QStringLiteral("has_logo")] = lay.has_logo_;
    o[QStringLiteral("barcode")] =
        id_card_element_to_json(lay.barcode_);
    o[QStringLiteral("text")] =
        id_card_element_to_json(lay.text_);
    o[QStringLiteral("logo")] =
        id_card_element_to_json(lay.logo_);
    return o;
  }

} // namespace

namespace colman
{

  RestServer::RestServer(IQueries& queries,
                         AuthMiddleware& auth,
                         QObject* parent)
      : QObject(parent), queries_(queries), auth_(auth)
  {
  }

/** Creates a QTcpServer, calls setup_routes(), and
    binds the TCP server to QHttpServer. */
  bool RestServer::listen(quint16 port)
  {
    setup_routes();
    auto* tcp = new QTcpServer(this);
    if(!tcp->listen(QHostAddress::Any, port)) {
      return false;
    }
    server_.bind(tcp);
    return true;
  }

/** Loads PEM certificate and key, creates a QSslServer, and
    binds it to QHttpServer for HTTPS. */
  bool RestServer::listen_tls(quint16 port,
                               QString const& cert_path,
                               QString const& key_path)
  {
    QFile cert_file(cert_path);
    if(!cert_file.open(QIODevice::ReadOnly)) {
      qCritical() << "Cannot open certificate file:" << cert_path;
      return false;
    }
    QSslCertificate const cert(&cert_file, QSsl::Pem);
    if(cert.isNull()) {
      qCritical() << "Failed to parse certificate:" << cert_path;
      return false;
    }

    QFile key_file(key_path);
    if(!key_file.open(QIODevice::ReadOnly)) {
      qCritical() << "Cannot open private key file:" << key_path;
      return false;
    }
    QSslKey const key(&key_file, QSsl::Rsa, QSsl::Pem);
    if(key.isNull()) {
      qCritical() << "Failed to parse private key (expected RSA PEM):"
                  << key_path;
      return false;
    }

    QSslConfiguration ssl_cfg = QSslConfiguration::defaultConfiguration();
    ssl_cfg.setLocalCertificate(cert);
    ssl_cfg.setPrivateKey(key);
    ssl_cfg.setProtocol(QSsl::TlsV1_2OrLater);

    setup_routes();
    auto* ssl = new QSslServer(this);
    ssl->setSslConfiguration(ssl_cfg);
    if(!ssl->listen(QHostAddress::Any, port)) {
      qCritical() << "QSslServer failed to listen on port" << port;
      return false;
    }
    server_.bind(ssl);
    return true;
  }

/// Starts listening on localhost with an OS-assigned port.
  quint16 RestServer::listen_on_local_port()
  {
    setup_routes();
    auto* tcp = new QTcpServer(this);
    if(!tcp->listen(QHostAddress::LocalHost, 0)) {
      return 0;
    }
    quint16 const port = tcp->serverPort();
    server_.bind(tcp);
    return port;
  }

/// Returns the underlying QHttpServer.
  QHttpServer& RestServer::http_server()
  {
    return server_;
  }

/// Returns the first TCP server port, or 0 if not listening.
  quint16 RestServer::server_port() const
  {
    auto const servers = server_.servers();
    if(servers.isEmpty()) {
      return 0;
    }
    return servers.first()->serverPort();
  }

/** Returns a JSON error response whose status and message match \a e. */
  QHttpServerResponse RestServer::auth_error(
      AuthMiddleware::AuthError e) const
  {
    using AE = AuthMiddleware::AuthError;
    switch(e) {
    case AE::MissingToken:
      return json_error(
          QStringLiteral("Authentication required"),
          SC::Unauthorized);
    case AE::InvalidToken:
      return json_error(
          QStringLiteral("Invalid or expired credentials"),
          SC::Unauthorized);
    case AE::NoAccount:
      return json_error(
          QStringLiteral("No account linked to these credentials"),
          SC::Forbidden);
    case AE::Forbidden:
      return json_error(
          QStringLiteral("You do not have permission for this action"),
          SC::Forbidden);
    default:
      return json_error(
          QStringLiteral("Authentication error"),
          SC::Unauthorized);
    }
  }

/** Registers all /api/ routes on server_. Fixed-path
    sub-routes are registered before parameterised ones
    to avoid shadowing. */
  void RestServer::setup_routes()
  {
  // ---------------------------------------------------------
  // Version (no authentication required)
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/version"),
        Method::Get,
        [this](QHttpServerRequest const&) -> QHttpServerResponse {
          QJsonObject o;
          o[QStringLiteral("major")] = ApiMajorVersion;
          o[QStringLiteral("minor")] = ApiMinorVersion;
          o[QStringLiteral("name")] =
              queries_.read_collection().get_server_name();
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/me"),
        Method::Get,
        [this](QHttpServerRequest const& req) -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          QJsonObject o;
          o[QStringLiteral("person_id")] = ar.person_id;
          o[QStringLiteral("edit_reservations")] =
              queries_.has_permission(
                  ar.person_id, QStringLiteral("edit_reservations"));
          auto const cols = queries_.get_permissions();
          auto const perms = queries_.get_user_permissions(ar.person_id);
          QJsonObject perms_obj;
          for(std::size_t i = 0;
              i < static_cast<std::size_t>(cols.size()) &&
              i < perms.size();
              ++i) {
            perms_obj[cols[i]] = perms[i];
          }
          o[QStringLiteral("permissions")] = perms_obj;
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/me/password"),
        Method::Post,
        [this](QHttpServerRequest const& req) -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto old_password =
              obj[QStringLiteral("old_password")].toString();
          auto new_password =
              obj[QStringLiteral("new_password")].toString();
          if(new_password.isEmpty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!queries_.change_own_password(
                 ar.person_id, old_password, new_password)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/table-versions"),
        Method::Get,
        [this](QHttpServerRequest const&) -> QHttpServerResponse {
          QJsonObject o;
          auto const versions = queries_.query_table_versions();
          for(auto it = versions.cbegin(); it != versions.cend(); ++it) {
            o[it.key()] = it.value();
          }
          return json_resp(o);
        });

  // ---------------------------------------------------------
  // Collection
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/collection"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto c = queries_.read_collection();
          QJsonObject o;
          o[QStringLiteral("name")] = c.get_name();
          o[QStringLiteral("organization")] =
              c.get_organization();
          o[QStringLiteral("responsible")] =
              c.get_responsible();
          o[QStringLiteral("barcode_prefix")] =
              c.get_barcode_prefix();
          o[QStringLiteral("use_lending_period")] =
              c.get_use_lending_period();
          o[QStringLiteral("lending_period_years")] =
              c.get_lending_period_years();
          o[QStringLiteral("lending_period_months")] =
              c.get_lending_period_months();
          o[QStringLiteral("lending_period_days")] =
              c.get_lending_period_days();
          o[QStringLiteral("allow_reservations")] =
              c.get_allow_reservations();
          o[QStringLiteral("reservation_period_days")] =
              c.get_reservation_period_days();
          o[QStringLiteral("default_item_type")] =
              c.get_default_item_type();
          o[QStringLiteral("default_is_school_item")] =
              c.get_default_is_school_item();
          o[QStringLiteral("logo")] = QString::fromLatin1(
              c.get_logo_raw().toBase64());
          o[QStringLiteral("grade_range_min")] =
              c.get_grade_range_min();
          o[QStringLiteral("grade_range_max")] =
              c.get_grade_range_max();
          // Person ID-card layout is exposed read-only so a freshly
          // installed client can seed its local copy once; clients keep
          // their own card/label layouts locally from then on.
          o[QStringLiteral("id_card_layout")] =
              id_card_layout_to_json(c.get_id_card_layout());
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/collection"),
        Method::Put,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_preferences"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Collection c = queries_.read_collection();
          c.set_name(
              obj[QStringLiteral("name")].toString());
          c.set_organization(
              obj[QStringLiteral("organization")]
                  .toString());
          c.set_responsible(
              obj[QStringLiteral("responsible")].toString());
          c.set_barcode_prefix(
              obj[QStringLiteral("barcode_prefix")]
                  .toString());
          c.set_use_lending_period(
              obj[QStringLiteral("use_lending_period")]
                  .toBool());
          c.set_lending_period_years(
              obj[QStringLiteral("lending_period_years")]
                  .toInt());
          c.set_lending_period_months(
              obj[QStringLiteral("lending_period_months")]
                  .toInt());
          c.set_lending_period_days(
              obj[QStringLiteral("lending_period_days")]
                  .toInt());
          c.set_allow_reservations(
              obj[QStringLiteral("allow_reservations")]
                  .toBool());
          c.set_reservation_period_days(
              obj[QStringLiteral("reservation_period_days")]
                  .toInt());
          c.set_default_item_type(
              obj[QStringLiteral("default_item_type")]
                  .toInt());
          c.set_default_is_school_item(
              obj[QStringLiteral("default_is_school_item")]
                  .toBool());
          if(obj.contains(QStringLiteral("logo"))) {
            c.set_logo_raw(QByteArray::fromBase64(
                obj[QStringLiteral("logo")]
                    .toString()
                    .toLatin1()));
          }
          if(obj.contains(QStringLiteral("grade_range_min"))) {
            c.set_grade_range_min(
                obj[QStringLiteral("grade_range_min")].toInt());
          }
          if(obj.contains(QStringLiteral("grade_range_max"))) {
            c.set_grade_range_max(
                obj[QStringLiteral("grade_range_max")].toInt());
          }
          queries_.change_collection(c);
          return SC::Ok;
        });

    // Card and label layouts are kept locally per client; the former
    // PUT endpoints for persisting them on the server were removed in
    // REST API 2.0.

  // ---------------------------------------------------------
  // Signature categories — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/signatures/top"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto cats =
              queries_.read_signature_top_categories();
          QJsonArray arr;
          for(auto const& s : cats) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/signatures/top"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto name =
              obj[QStringLiteral("name")].toString();
          queries_.add_signature_top_category(name);
          return SC::Created;
        });

    server_.route(
        QStringLiteral(
            "/api/signatures/top/by-name/<arg>"),
        Method::Get,
        [this](QString const& name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto locale =
              queries_.get_type_locale(name,
                                       language_from_request(req));
          return json_value(locale);
        });

    server_.route(
        QStringLiteral("/api/signatures/bottom"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto cats =
              queries_.read_signature_bottom_categories(QString());
          QJsonArray arr;
          for(auto const& s : cats) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/signatures/top/<arg>/bottom"),
        Method::Get,
        [this](QString const& top,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto cats =
              queries_.read_signature_bottom_categories(top);
          QJsonArray arr;
          for(auto const& s : cats) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/signatures/top/<arg>/bottom"),
        Method::Post,
        [this](QString const& top,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto name =
              obj[QStringLiteral("name")].toString();
          queries_.add_signature_bottom_category(top, name);
          return SC::Created;
        });

    server_.route(
        QStringLiteral(
            "/api/signatures/top/<arg>/bottom/<arg>"),
        Method::Delete,
        [this](QString const& top, QString const& name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_signature_bottom_category(
              top, name);
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral("/api/signatures/top/<arg>"),
        Method::Delete,
        [this](QString const& name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_signature_top_category(name);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Types — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/types/locale"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto pairs =
              queries_.get_type_locale(language_from_request(req));
          QJsonArray arr;
          for(auto const& p : pairs) {
            QJsonObject o;
            o[QStringLiteral("id")] = p.first;
            o[QStringLiteral("name")] = p.second;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types/names"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto names = queries_.get_type_names();
          QJsonArray arr;
          for(auto const& n : names) {
            arr.append(n);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types/field-order/default"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto order =
              queries_.get_default_common_field_order();
          QJsonArray arr;
          for(auto const& s : order) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types/by-name/<arg>/locale"),
        Method::Get,
        [this](QString const& name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto locale =
              queries_.get_type_locale(name,
                                       language_from_request(req));
          return json_value(locale);
        });

    server_.route(
        QStringLiteral("/api/types/by-name/<arg>"),
        Method::Get,
        [this](QString const& name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto id = queries_.get_type_id(name);
          if(id < 0) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          auto t = queries_.get_type(id);
          QJsonObject o;
          o[QStringLiteral("id")] = id;
          o[QStringLiteral("name")] = t.get_name();
          o[QStringLiteral("media_kind")] =
              static_cast<int>(t.get_media_kind());
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/types"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto types = queries_.query_types(-1, language_from_request(req));
          QJsonArray arr;
          for(auto const& t : types) {
            QJsonObject o;
            o[QStringLiteral("id")] = t.get_id();
            o[QStringLiteral("name")] = t.get_name();
            o[QStringLiteral("media_kind")] =
                static_cast<int>(t.get_media_kind());
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Type t;
          t.set_name(
              obj[QStringLiteral("name")].toString());
          t.set_media_kind(static_cast<MediaKind>(
              obj[QStringLiteral("media_kind")].toInt()));
          auto tablename =
              obj[QStringLiteral("tablename")].toString();
          for(auto const& v :
              obj[QStringLiteral("keys")].toArray()) {
            auto ko = v.toObject();
            KeyInfo ki;
            ki.first =
                ko[QStringLiteral("column")].toString();
            ki.second.type_ = DataType(
                ko[QStringLiteral("data_type")].toInt());
            ki.second.info_ =
                ko[QStringLiteral("info")]
                    .toObject()
                    .toVariantMap();
            t.add_key(ki);
          }
          auto id =
              queries_.add_new_type(t, tablename);
          return json_id(id);
        });

    server_.route(
        QStringLiteral(
            "/api/types/<arg>/field-order"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto order =
              queries_.get_combined_field_order(id);
          QJsonArray arr;
          for(auto const& s : order) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/types/<arg>/field-order"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto arr = parse_body_array(req);
          QStringList order;
          for(auto const& v : arr) {
            order << v.toString();
          }
          queries_.save_field_order(id, order);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/types/<arg>/locale"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto locale =
              queries_.get_type_locale(id,
                                       language_from_request(req));
          return json_value(locale);
        });

    server_.route(
        QStringLiteral("/api/types/<arg>/attributes"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          QMap<QString, QString> display_names;
          for(auto it = obj.constBegin();
              it != obj.constEnd(); ++it) {
            display_names[it.key()] =
                it.value().toString();
          }
          auto t = queries_.get_type(id);
          queries_.change_type(id, t, display_names);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral(
            "/api/types/<arg>/completions/<arg>"),
        Method::Get,
        [this](int id, QString const& field,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto completions =
              queries_.query_type_field_completions(
                  id, field);
          QJsonArray arr;
          for(auto const& s : completions) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types/<arg>/field-metas"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto metas =
              queries_.get_type_field_metas(id);
          QJsonArray arr;
          for(auto const& m : metas) {
            QJsonObject o;
            o[QStringLiteral("column")] = m.column;
            o[QStringLiteral("marc21_tag")] = m.marc21_tag;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types/<arg>/query"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto types = queries_.query_types(id, language_from_request(req));
          QJsonArray arr;
          for(auto const& t : types) {
            QJsonObject o;
            o[QStringLiteral("id")] = t.get_id();
            o[QStringLiteral("name")] = t.get_name();
            o[QStringLiteral("media_kind")] =
                static_cast<int>(t.get_media_kind());
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/types/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto t = queries_.get_type(id);
          QJsonObject o;
          o[QStringLiteral("id")] = id;
          o[QStringLiteral("name")] = t.get_name();
          o[QStringLiteral("media_kind")] =
              static_cast<int>(t.get_media_kind());
          auto const qparams = QUrlQuery(req.url().query());
          auto const lang_param_int =
              qparams.queryItemValue(
                  QStringLiteral("lang")).toInt();
          QLocale::Language lang;
          if(lang_param_int != 0) {
            lang = static_cast<QLocale::Language>(lang_param_int);
          } else {
            auto const req_lang = language_from_request(req);
            lang = (req_lang == QLocale::AnyLanguage)
                       ? system_language()
                       : req_lang;
          }
          bool const has_lang =
              lang != QLocale::AnyLanguage && lang != QLocale::C;
          QJsonArray keys_arr;
          for(auto const& k : t.get_keys()) {
            QJsonObject ko;
            ko[QStringLiteral("column")] = k.first;
            ko[QStringLiteral("data_type")] = (int)k.second.type_;
            ko[QStringLiteral("info")] =
                QJsonObject::fromVariantMap(k.second.info_);
            QString lbl;
            if(has_lang) {
              lbl = queries_.get_type_field_label(
                  id, k.first, lang);
            }
            ko[QStringLiteral("label")] =
                lbl.isEmpty() ? k.first : lbl;
            keys_arr.append(ko);
          }
          o[QStringLiteral("keys")] = keys_arr;
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/types/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Type t;
          t.set_name(
              obj[QStringLiteral("name")].toString());
          t.set_media_kind(static_cast<MediaKind>(
              obj[QStringLiteral("media_kind")].toInt()));
          for(auto const& v :
              obj[QStringLiteral("keys")].toArray()) {
            auto ko = v.toObject();
            KeyInfo ki;
            ki.first =
                ko[QStringLiteral("column")].toString();
            ki.second.type_ = DataType(
                ko[QStringLiteral("data_type")].toInt());
            ki.second.info_ =
                ko[QStringLiteral("info")]
                    .toObject()
                    .toVariantMap();
            t.add_key(ki);
          }
          QMap<QString, QString> display_names;
          auto dn =
              obj[QStringLiteral("display_names")]
                  .toObject();
          for(auto it = dn.constBegin();
              it != dn.constEnd(); ++it) {
            display_names[it.key()] =
                it.value().toString();
          }
          queries_.change_type(id, t, display_names);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/types/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("edit_categories"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_type(id);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Items — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/items/barcode/parse"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto barcode =
              obj[QStringLiteral("barcode")].toString();
          int type_id = 0;
          int item_id = 0;
          if(!queries_.parse_item_barcode(
                 barcode, type_id, item_id)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          QJsonObject o;
          o[QStringLiteral("type_id")] = type_id;
          o[QStringLiteral("item_id")] = item_id;
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/items/barcode/check"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto barcode = query.queryItemValue(
              QStringLiteral("barcode"));
          auto ok = queries_.is_item_barcode(barcode);
          return json_value(ok);
        });

    server_.route(
        QStringLiteral("/api/items/barcode"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto type_id = query.queryItemValue(
                                  QStringLiteral("type_id"))
                             .toInt();
          auto item_id = query.queryItemValue(
                                  QStringLiteral("item_id"))
                             .toInt();
          auto bc = queries_.compute_item_barcode(
              type_id, item_id);
          return json_value(bc);
        });

    server_.route(
        QStringLiteral("/api/items/by-isbn/<arg>"),
        Method::Get,
        [this](QString const& isbn,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto ids = queries_.items_by_isbn(isbn);
          QJsonArray arr;
          for(auto const id : ids) {
            arr.append(id);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/by-ean/<arg>"),
        Method::Get,
        [this](QString const& ean,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto ids = queries_.items_by_ean(ean);
          QJsonArray arr;
          for(auto const id : ids) {
            arr.append(id);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/lent-query"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto person_id =
              obj[QStringLiteral("person_id")].toInt();
          auto lending_id =
              obj[QStringLiteral("lending_id")].toInt();
          auto person_search =
              obj[QStringLiteral("person_search")]
                  .toString();
          auto item_search =
              obj[QStringLiteral("item_search")].toString();
          auto rows = queries_.query_lent_items(
              person_id, lending_id,
              person_search, item_search,
              language_from_request(req));
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(item_list_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/with-type"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto type_id = query.queryItemValue(
                                  QStringLiteral("type_id"))
                             .toInt();
          auto search = query.queryItemValue(
              QStringLiteral("search"));
          auto limit = query_int(
              query.queryItemValue(QStringLiteral("limit")));
          auto offset = query_int(
              query.queryItemValue(QStringLiteral("offset")));
          auto rows = queries_.query_items_with_type(
              type_id, search, limit, offset,
              language_from_request(req));
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(item_list_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/search"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")), {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!queries_.has_permission(
                 ar.person_id, QStringLiteral("view_items")) &&
             !queries_.has_permission(
                 ar.person_id, QStringLiteral("reserve"))) {
            return json_error(QStringLiteral(
                "You do not have permission for this action"),
                SC::Forbidden);
          }
          auto obj = parse_body(req);
          auto general =
              obj[QStringLiteral("general")].toString();
          auto name =
              obj[QStringLiteral("name")].toString();
          auto subject =
              obj[QStringLiteral("subject")].toString();
          auto description =
              obj[QStringLiteral("description")].toString();
          auto keywords =
              obj[QStringLiteral("keywords")].toString();
          auto type_id =
              obj[QStringLiteral("type_id")].toInt(-1);
          std::vector<IQueries::TypeFieldFilter> type_fields;
          for(auto const& v :
              obj[QStringLiteral("type_fields")].toArray()) {
            auto fo = v.toObject();
            IQueries::TypeFieldFilter f;
            f.column_ =
                fo[QStringLiteral("column")].toString();
            f.value_ =
                fo[QStringLiteral("value")].toString();
            type_fields.push_back(f);
          }
          auto grade_min =
              obj[QStringLiteral("grade_min")].toInt(0);
          auto grade_max =
              obj[QStringLiteral("grade_max")].toInt(0);
          auto limit = obj[QStringLiteral("limit")].toInt(0);
          auto offset = obj[QStringLiteral("offset")].toInt(0);
          auto rows = queries_.query_items_for_search(
              general, name, subject, description,
              keywords, type_id, type_fields,
              grade_min, grade_max, limit, offset,
              language_from_request(req));
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(search_item_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/completions/<arg>"),
        Method::Get,
        [this](QString const& field,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto completions =
              queries_.query_item_field_completions(field);
          QJsonArray arr;
          for(auto const& s : completions) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/items/school-completions/<arg>"),
        Method::Get,
        [this](QString const& field,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto completions =
              queries_
                  .query_school_item_field_completions(field);
          QJsonArray arr;
          for(auto const& s : completions) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/acquired"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto id = query_int(query.queryItemValue(
                                  QStringLiteral("id")),
                              -1);
          auto search = query.queryItemValue(
              QStringLiteral("search"));
          auto limit = query_int(
              query.queryItemValue(QStringLiteral("limit")));
          auto offset = query_int(
              query.queryItemValue(QStringLiteral("offset")));
          auto rows =
              queries_.query_acquired_items(id, search,
                                            limit, offset,
                                            language_from_request(req));
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(item_list_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/available"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto id = query_int(query.queryItemValue(
                                  QStringLiteral("id")),
                              -1);
          auto search = query.queryItemValue(
              QStringLiteral("search"));
          auto limit = query_int(
              query.queryItemValue(QStringLiteral("limit")));
          auto offset = query_int(
              query.queryItemValue(QStringLiteral("offset")));
          auto rows =
              queries_.query_available_items(id, search,
                                             limit, offset,
                                             language_from_request(req));
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(item_list_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/count"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          QJsonObject obj;
          obj[QStringLiteral("count")] = queries_.count_items();
          return json_resp(obj);
        });

    server_.route(
        QStringLiteral(
            "/api/items/ordered-by-name"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto pairs =
              queries_.get_items_ordered_by_name();
          QJsonArray arr;
          for(auto const& p : pairs) {
            QJsonObject o;
            o[QStringLiteral("id")] = p.first;
            o[QStringLiteral("name")] = p.second;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/recently-acquired"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto const qparams = QUrlQuery(req.url().query());
          int const count =
              qparams.queryItemValue(QStringLiteral("count")).toInt();
          auto ids = queries_.get_recently_acquired_item_ids(count);
          QJsonArray arr;
          for(int const id : ids) {
            QJsonObject o;
            o[QStringLiteral("id")] = id;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/acquired-between"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto const qparams = QUrlQuery(req.url().query());
          auto const from = QDateTime::fromString(
              qparams.queryItemValue(QStringLiteral("from"),
                                     QUrl::FullyDecoded),
              Qt::ISODate);
          auto const to = QDateTime::fromString(
              qparams.queryItemValue(QStringLiteral("to"),
                                     QUrl::FullyDecoded),
              Qt::ISODate);
          auto ids = queries_.get_items_acquired_between(from, to);
          QJsonArray arr;
          for(int const id : ids) {
            QJsonObject o;
            o[QStringLiteral("id")] = id;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/with-thumbnails"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto rows =
              queries_.get_items_with_type_and_thumbnails();
          QJsonArray arr;
          for(auto const& r : rows) {
            QJsonObject o;
            o[QStringLiteral("item_id")] = r.item_id_;
            o[QStringLiteral("item_name")] = r.item_name_;
            o[QStringLiteral("type_name")] = r.type_name_;
            o[QStringLiteral("thumb_id")] = r.thumb_id_;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/<arg>/stock"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto n = queries_.stock_size(id);
          return json_value(static_cast<int>(n));
        });

    server_.route(
        QStringLiteral("/api/items/<arg>/available"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto n = queries_.available(id);
          return json_value(static_cast<int>(n));
        });

    server_.route(
        QStringLiteral(
            "/api/items/<arg>/have-lendings"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto ok = queries_.have_lendings(id);
          return json_value(ok);
        });

    server_.route(
        QStringLiteral(
            "/api/items/<arg>/open-lendings"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto pairs =
              queries_.open_lendings_for_item(id);
          QJsonArray arr;
          for(auto const& p : pairs) {
            QJsonObject o;
            o[QStringLiteral("lending_id")] = p.first;
            o[QStringLiteral("person_id")] = p.second;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/items/<arg>/item-lendings"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto infos =
              queries_.query_item_lendings(id);
          QJsonArray arr;
          for(auto const& info : infos) {
            arr.append(lending_person_info_to_json(info));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          Item item;
          QString type_name;
          if(!queries_.get_item(id, item, type_name)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          auto o = to_json(item);
          o[QStringLiteral("type_name")] =
              queries_.get_type_locale(type_name, language_from_request(req));
          return json_resp_with_etag(o, item.get_version());
        });

    server_.route(
        QStringLiteral("/api/items"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")), {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!queries_.has_permission(
                 ar.person_id, QStringLiteral("view_items")) &&
             !queries_.has_permission(
                 ar.person_id, QStringLiteral("reserve"))) {
            return json_error(QStringLiteral(
                "You do not have permission for this action"),
                SC::Forbidden);
          }
          auto query = QUrlQuery(req.url().query());
          auto id = query_int(query.queryItemValue(
                                  QStringLiteral("id")),
                              -1);
          auto search = query.queryItemValue(
              QStringLiteral("search"));
          auto rows = queries_.query_items(id, search,
                                          language_from_request(req));
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(item_list_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Item item;
          if(!from_json(obj, item)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!academic_years_valid(item.get_academic_year())) {
            return json_error(
                QStringLiteral("Invalid academic year range"),
                SC::BadRequest);
          }
          auto id = queries_.add_new_item(item);
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/items/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Item item;
          if(!from_json(obj, item)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!academic_years_valid(item.get_academic_year())) {
            return json_error(
                QStringLiteral("Invalid academic year range"),
                SC::BadRequest);
          }
          auto ver = if_match_version(req);
          auto result =
              queries_.change_item_versioned(
                  id, item, ver);
          return versioned_response(result);
        });

    server_.route(
        QStringLiteral("/api/items/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto force_str = query.queryItemValue(
              QStringLiteral("force"));
          if(force_str == QStringLiteral("true")) {
            queries_.force_remove_item(id);
          }
          else {
            queries_.remove_item(id);
          }
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Thumbnails
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/thumbnails"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto png = QByteArray::fromBase64(
              obj[QStringLiteral("png")]
                  .toString()
                  .toLatin1());
          auto key =
              obj[QStringLiteral("key")].toString();
          auto url =
              obj[QStringLiteral("url")].toString();
          auto overwrite =
              obj[QStringLiteral("overwrite")].toBool();
          auto id = queries_.store_thumbnail(
              png, key, url, overwrite);
          if(id < 0) {
            return json_error(QStringLiteral("Internal server error"),
                SC::InternalServerError);
          }
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/thumbnails/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_items"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          QByteArray png;
          if(!queries_.get_thumbnail(id, png)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return QHttpServerResponse(
              QByteArrayLiteral("image/png"), png);
        });

  // ---------------------------------------------------------
  // Persons — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/persons/start-page"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto search = query.queryItemValue(
              QStringLiteral("search"));
          auto rows =
              queries_.query_start_page_persons(search);
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(start_page_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/persons/by-barcode/<arg>"),
        Method::Get,
        [this](QString const& barcode,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto id = queries_.get_person(barcode);
          if(id < 0) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          Person p;
          if(!queries_.get_person(id, p)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_resp(to_json(p));
        });

    server_.route(
        QStringLiteral("/api/persons/known"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto lastname = query.queryItemValue(
              QStringLiteral("lastname"));
          auto firstname = query.queryItemValue(
              QStringLiteral("firstname"));
          auto known = queries_.person_known(
              lastname, firstname);
          return json_value(known ? 1 : 0);
        });

    server_.route(
        QStringLiteral("/api/persons/recently-created"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto const qparams = QUrlQuery(req.url().query());
          int const count =
              qparams.queryItemValue(QStringLiteral("count")).toInt();
          auto ids = queries_.get_recently_created_person_ids(count);
          QJsonArray arr;
          for(int const id : ids) {
            QJsonObject o;
            o[QStringLiteral("id")] = id;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/persons/by-external-id/<arg>"),
        Method::Get,
        [this](QString const& ext_id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto id =
              queries_.get_person_by_external_id(ext_id);
          if(id < 0) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_value(id);
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/external-id"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto obj = parse_body(req);
          auto sub =
              obj[QStringLiteral("sub")].toString();
          if(!queries_.set_person_external_id(id, sub)) {
            return json_error(QStringLiteral("Internal server error"),
                SC::InternalServerError);
          }
          return SC::Ok;
        });

    server_.route(
        QStringLiteral(
            "/api/persons/completions/<arg>"),
        Method::Get,
        [this](QString const& field,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto completions =
              queries_.query_person_field_completions(field);
          QJsonArray arr;
          for(auto const& s : completions) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/persons/lending-query"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto item_id =
              obj[QStringLiteral("item_id")].toInt();
          auto lending_id =
              obj[QStringLiteral("lending_id")].toInt();
          auto person_search =
              obj[QStringLiteral("person_search")]
                  .toString();
          auto item_search =
              obj[QStringLiteral("item_search")].toString();
          auto rows = queries_.query_lending_persons(
              item_id, lending_id,
              person_search, item_search);
          QJsonArray arr;
          for(auto const& p : rows) {
            arr.append(to_json(p));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/persons/change-passwords"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto rows =
              queries_.query_change_passwords_persons();
          QJsonArray arr;
          for(auto const& r : rows) {
            QJsonObject o;
            o[QStringLiteral("id")] = r.id;
            o[QStringLiteral("lastname")] = r.lastname;
            o[QStringLiteral("firstname")] = r.firstname;
            o[QStringLiteral("title")] = r.title;
            o[QStringLiteral("username")] = r.username;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/persons/<arg>/open-lendings"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto pairs =
              queries_.open_lendings_for_person(id);
          QJsonArray arr;
          for(auto const& p : pairs) {
            QJsonObject o;
            o[QStringLiteral("lending_id")] = p.first;
            o[QStringLiteral("item_id")] = p.second;
            arr.append(o);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/barcode"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          return json_value(queries_.compute_person_barcode(id));
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/permissions"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto perms =
              queries_.get_user_permissions(id);
          QJsonArray arr;
          for(auto const b : perms) {
            arr.append(static_cast<bool>(b));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/persons/<arg>/permissions/defaults"),
        Method::Post,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          if(!queries_.create_default_permissions(id)) {
            return json_error(QStringLiteral("Internal server error"),
                SC::InternalServerError);
          }
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral(
            "/api/persons/<arg>/permissions/<arg>"),
        Method::Put,
        [this](int id, QString const& column,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          // The caller must be allowed to manage this target and to grant
          // the specific permission column (scope enforcement).
          if(!may_manage_target(queries_, ar.person_id, id) ||
             !may_grant_permission(queries_, ar.person_id, column)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto obj = parse_body(req);
          auto val =
              obj[QStringLiteral("value")].toBool();
          queries_.set_permission(id, column, val);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/permissions"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          queries_.remove_permissions(id);
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/password"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          QJsonObject o;
          o[QStringLiteral("has_password")] =
              queries_.has_password(id);
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/password"),
        Method::Post,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto obj = parse_body(req);
          auto username = obj[QStringLiteral("username")].toString();
          auto hash = obj[QStringLiteral("password")].toString();
          if(hash.isEmpty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(username.isEmpty()) {
            queries_.set_password(id, hash);
          }
          else {
            queries_.set_local_credentials(id, username, hash);
          }
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/password"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          queries_.remove_password(id);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/username"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto const u = queries_.get_username(id);
          QJsonObject o;
          if(u.isEmpty()) {
            o[QStringLiteral("username")] = QJsonValue::Null;
          }
          else {
            o[QStringLiteral("username")] = u;
          }
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/username"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto const username =
              parse_body(req)[QStringLiteral("username")].toString();
          if(username.isEmpty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!queries_.set_username(id, username)) {
            return json_error(
                QStringLiteral("Conflict"), SC::Conflict);
          }
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/username"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          queries_.remove_username(id);
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/class"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          Person p;
          if(!queries_.get_person(id, p)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_value(p.get_title());
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>/class"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          Person p;
          if(!queries_.get_person(id, p)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          auto obj = parse_body(req);
          p.set_school_class(
              obj[QStringLiteral("value")].toString());
          queries_.change_person(id, p);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
        // Reading one's own person record needs no view_persons
        // permission so that e.g. reserve-only users can display
        // their own name.
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(ar.person_id != id &&
             !queries_.has_permission(
                 ar.person_id, QStringLiteral("view_persons"))) {
            return json_error(
                QStringLiteral(
                    "You do not have permission for this action"),
                SC::Forbidden);
          }
          Person p;
          if(!queries_.get_person(id, p)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_resp_with_etag(to_json(p), p.get_version());
        });

    server_.route(
        QStringLiteral("/api/persons"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_persons"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto id = query_int(query.queryItemValue(
                                  QStringLiteral("id")),
                              -1);
          auto search = query.queryItemValue(
              QStringLiteral("search"));
          auto persons =
              queries_.query_persons(id, search);
          QJsonArray arr;
          for(auto const& p : persons) {
            arr.append(to_json(p));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/persons"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_persons(queries_, ar.person_id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto obj = parse_body(req);
          Person p;
          if(!from_json(obj, p)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto id = queries_.add_new_person(p);
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto obj = parse_body(req);
          Person p;
          if(!from_json(obj, p)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto ver = if_match_version(req);
          auto result =
              queries_.change_person_versioned(
                  id, p, ver);
          return versioned_response(result);
        });

    server_.route(
        QStringLiteral("/api/persons/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QString());
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          if(!may_manage_target(queries_, ar.person_id, id)) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          queries_.remove_person(id);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Permissions
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/permissions"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto perms = queries_.get_permissions();
          QJsonArray arr;
          for(auto const& s : perms) {
            arr.append(s);
          }
          return json_resp(arr);
        });

  // ---------------------------------------------------------
  // Lendings — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/lendings/open"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          bool const has_view_history = queries_.has_permission(
              ar.person_id, QStringLiteral("view_history"));
          bool const has_collect_returns = queries_.has_permission(
              ar.person_id, QStringLiteral("collect_returns"));
          if(!has_view_history && !has_collect_returns) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto query = QUrlQuery(req.url().query());
          auto person_id = query_int(
              query.queryItemValue(
                  QStringLiteral("person_id")),
              -1);
          auto item_id = query_int(
              query.queryItemValue(
                  QStringLiteral("item_id")),
              -1);
          auto rows = queries_.query_open_lendings(
              person_id, item_id);
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(open_lending_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/lendings/due"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto from_str = query.queryItemValue(
              QStringLiteral("from"));
          auto to_str = query.queryItemValue(
              QStringLiteral("to"));
          auto person_id = query_int(
              query.queryItemValue(
                  QStringLiteral("person_id")),
              -1);
          auto school_class = query.queryItemValue(
              QStringLiteral("school_class"),
              QUrl::FullyDecoded);
          auto person_name = query.queryItemValue(
              QStringLiteral("person_name"),
              QUrl::FullyDecoded);
          bool const has_view_history = queries_.has_permission(
              ar.person_id,
              QStringLiteral("view_history"));
          bool const has_collect_returns = queries_.has_permission(
              ar.person_id,
              QStringLiteral("collect_returns"));
          // Lenders need a person's open lendings to extend them.
          bool const has_collect_lendings = queries_.has_permission(
              ar.person_id,
              QStringLiteral("collect_lendings"));
          bool const can_see = has_view_history
              || has_collect_returns
              || has_collect_lendings
              || person_id == ar.person_id;
          if(!can_see) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          auto from_date =
              QDate::fromString(from_str, Qt::ISODate);
          auto to_date =
              QDate::fromString(to_str, Qt::ISODate);
          auto rows = queries_.query_due_lendings(
              from_date, to_date,
              person_id, school_class, person_name);
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(due_lending_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/lendings/extend"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_lendings"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          std::vector<int> ids;
          for(auto const& v :
              obj[QStringLiteral("lending_ids")].toArray()) {
            ids.push_back(v.toInt());
          }
          if(ids.empty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto const duedate = QDate::fromString(
              obj[QStringLiteral("duedate")].toString(), Qt::ISODate);
          if(!queries_.extend_lendings(ids, duedate)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral("/api/lendings/batch"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_lendings"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Lending l;
          if(!from_json(obj[QStringLiteral("lending")].toObject(), l)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          std::vector<int> person_ids;
          for(auto const& v :
              obj[QStringLiteral("person_ids")].toArray()) {
            person_ids.push_back(v.toInt());
          }
          if(person_ids.empty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!l.get_date_time().isValid()) {
            l.set_date_time(QDateTime::currentDateTimeUtc());
          }
          using Status = IQueries::GroupLendingResult::Status;
          auto const res = queries_.add_group_lending(l, person_ids);
          if(res.status_ == Status::NotEnoughAvailable) {
            QJsonObject err;
            err.insert("error", QStringLiteral("not_available"));
            err.insert("available_count", res.lendable_);
            return QHttpServerResponse(QJsonDocument(err).toJson(),
                                       SC::Conflict);
          }
          if(res.status_ == Status::AlreadyLent) {
            QJsonObject err;
            err.insert("error", QStringLiteral("already_lent"));
            err.insert("person_id", res.person_id_);
            return QHttpServerResponse(QJsonDocument(err).toJson(),
                                       SC::Conflict);
          }
          if(res.status_ != Status::Ok) {
            return json_error(QStringLiteral("Internal error"),
                              SC::InternalServerError);
          }
          QJsonArray ids;
          for(int const id : res.lending_ids_) {
            ids.append(id);
          }
          QJsonObject o;
          o[QStringLiteral("ids")] = ids;
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/lendings/person/<arg>"),
        Method::Get,
        [this](int person_id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_history"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto infos =
              queries_.query_lendings(person_id);
          QJsonArray arr;
          for(auto const& info : infos) {
            arr.append(lending_info_to_json(info));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/lendings/<arg>/balance"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          // The balance is needed both to view history and to perform a
          // return, so accept either permission (mirrors /api/lendings/open).
          bool const has_view_history = queries_.has_permission(
              ar.person_id, QStringLiteral("view_history"));
          bool const has_collect_returns = queries_.has_permission(
              ar.person_id, QStringLiteral("collect_returns"));
          if(!has_view_history && !has_collect_returns) {
            return auth_error(AuthMiddleware::AuthError::Forbidden);
          }
          std::size_t lent = 0;
          std::size_t returned = 0;
          if(!queries_.lending_balance(
                 id, lent, returned)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          QJsonObject o;
          o[QStringLiteral("lent")] =
              static_cast<int>(lent);
          o[QStringLiteral("returned")] =
              static_cast<int>(returned);
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/lendings/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_history"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          Lending l;
          if(!queries_.get_lending(id, l)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_resp(to_json(l));
        });

    server_.route(
        QStringLiteral("/api/lendings"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_history"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto person_id = query_int(
              query.queryItemValue(
                  QStringLiteral("person_id")),
              -1);
          auto item_id = query_int(
              query.queryItemValue(
                  QStringLiteral("item_id")),
              -1);
          auto rows = queries_.query_open_lendings(
              person_id, item_id);
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(open_lending_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/lendings"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_lendings"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Lending l;
          if(!from_json(obj, l)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!l.get_date_time().isValid()) {
            l.set_date_time(QDateTime::currentDateTimeUtc());
          }
          auto id = queries_.add_new_lending(l);
          if(id == -2) {
            int const avail =
                static_cast<int>(queries_.available(l.get_item_id()));
            auto const ress =
                queries_.get_reservations_for_item(l.get_item_id());
            int not_reserved = avail;
            for(auto const& r : ress) {
              if(r.person_id != l.get_person_id()) {
                not_reserved -= r.count;
              }
            }
            not_reserved = std::max(0, not_reserved);
            QJsonObject err;
            err.insert("error", QStringLiteral("reserved"));
            err.insert("not_reserved_count", not_reserved);
            return QHttpServerResponse(QJsonDocument(err).toJson(),
                                       QHttpServerResponse::StatusCode::Conflict);
          }
          if(id < 0) {
            // Not enough copies available (e.g. already lent out). Never
            // report this as success.
            QJsonObject err;
            err.insert("error", QStringLiteral("not_available"));
            err.insert(
                "available_count",
                static_cast<int>(queries_.available(l.get_item_id())));
            return QHttpServerResponse(QJsonDocument(err).toJson(),
                                       QHttpServerResponse::StatusCode::Conflict);
          }
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/lendings/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_lendings"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Lending l;
          if(!from_json(obj, l)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto ver = if_match_version(req);
          auto result =
              queries_.change_lending_versioned(
                  id, l, ver);
          return versioned_response(result);
        });

    server_.route(
        QStringLiteral("/api/lendings/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_lendings"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_lending(id);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Returns
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/returns"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_returns"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Return ret;
          if(!from_json(obj, ret)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto id = queries_.add_new_return(ret);
          if(id < 0) {
            return json_error(
                QStringLiteral("Return rejected (invalid count or "
                               "nothing outstanding)"),
                SC::Conflict);
          }
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/returns/batch"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_returns"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Return tmpl;
          if(!from_json(obj[QStringLiteral("return")].toObject(), tmpl)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          int const item_id = obj[QStringLiteral("item_id")].toInt();
          std::vector<int> person_ids;
          for(auto const& v :
              obj[QStringLiteral("person_ids")].toArray()) {
            person_ids.push_back(v.toInt());
          }
          if(item_id <= 0 || person_ids.empty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(!tmpl.get_date_time().isValid()) {
            tmpl.set_date_time(QDateTime::currentDateTimeUtc());
          }
          using Status = IQueries::GroupReturnResult::Status;
          auto const res =
              queries_.add_group_return(item_id, person_ids, tmpl);
          if(res.status_ == Status::NotLent) {
            QJsonObject err;
            err.insert("error", QStringLiteral("not_lent"));
            err.insert("person_id", res.person_id_);
            return QHttpServerResponse(QJsonDocument(err).toJson(),
                                       SC::Conflict);
          }
          if(res.status_ != Status::Ok) {
            return json_error(QStringLiteral("Internal error"),
                              SC::InternalServerError);
          }
          QJsonArray ids;
          for(int const id : res.return_ids_) {
            ids.append(id);
          }
          QJsonObject o;
          o[QStringLiteral("ids")] = ids;
          o[QStringLiteral("count")] = res.returned_count_;
          return json_resp(o);
        });

    server_.route(
        QStringLiteral("/api/returns/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_returns"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          Return ret;
          if(!queries_.get_return(id, ret)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_resp(to_json(ret));
        });

    server_.route(
        QStringLiteral("/api/returns/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_returns"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Return ret;
          if(!from_json(obj, ret)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto ver = if_match_version(req);
          auto result =
              queries_.change_return_versioned(
                  id, ret, ver);
          return versioned_response(result);
        });

    server_.route(
        QStringLiteral("/api/returns/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_returns"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_return(id);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Acquisitions
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/acquisitions"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Acquisition acq;
          if(!from_json(obj, acq)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto id = queries_.add_new_acquisition(acq);
          if(id < 0) {
            return json_error(
                QStringLiteral("Acquisition rejected"), SC::Conflict);
          }
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/acquisitions/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          Acquisition acq;
          if(!queries_.get_acquisition(id, acq)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_resp(to_json(acq));
        });

    server_.route(
        QStringLiteral("/api/acquisitions/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Acquisition acq;
          if(!from_json(obj, acq)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto ver = if_match_version(req);
          auto result =
              queries_.change_acquisition_versioned(
                  id, acq, ver);
          return versioned_response(result);
        });

    server_.route(
        QStringLiteral("/api/acquisitions/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_acquisitions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_acquisition(id);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Liquidations
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/liquidations"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_liquidations"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Liquidation liq;
          if(!from_json(obj, liq)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto id = queries_.add_new_liquidation(liq);
          if(id < 0) {
            return json_error(
                QStringLiteral("Liquidation rejected (not enough in "
                               "stock)"),
                SC::Conflict);
          }
          return json_id(id);
        });

    server_.route(
        QStringLiteral("/api/liquidations/<arg>"),
        Method::Get,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_liquidations"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          Liquidation liq;
          if(!queries_.get_liquidation(id, liq)) {
            return json_error(
                QStringLiteral("Not found"), SC::NotFound);
          }
          return json_resp(to_json(liq));
        });

    server_.route(
        QStringLiteral("/api/liquidations/<arg>"),
        Method::Put,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_liquidations"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          Liquidation liq;
          if(!from_json(obj, liq)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          auto ver = if_match_version(req);
          auto result =
              queries_.change_liquidation_versioned(
                  id, liq, ver);
          return versioned_response(result);
        });

    server_.route(
        QStringLiteral("/api/liquidations/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("collect_liquidations"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_liquidation(id);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // History — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral("/api/history/conflicts"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("change_actions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto action = history_row_from_json(obj);
          auto conflicts =
              queries_.find_conflicting_actions(action);
          QJsonArray arr;
          for(auto const& r : conflicts) {
            arr.append(history_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral(
            "/api/history/change-conflicts"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("change_actions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto old_action = history_row_from_json(
              obj[QStringLiteral("old")].toObject());
          auto new_action = history_row_from_json(
              obj[QStringLiteral("new")].toObject());
          auto conflicts =
              queries_.find_change_conflicts(
                  old_action, new_action);
          QJsonArray arr;
          for(auto const& r : conflicts) {
            arr.append(history_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/history"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("view_history"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto is_admin_str = query.queryItemValue(
              QStringLiteral("is_admin"));
          auto is_admin =
              (is_admin_str == QStringLiteral("true"));
          auto from_str = query.queryItemValue(
              QStringLiteral("from"));
          auto to_str = query.queryItemValue(
              QStringLiteral("to"));
          auto from_date =
              QDate::fromString(from_str, Qt::ISODate);
          auto to_date =
              QDate::fromString(to_str, Qt::ISODate);
          auto show_all_str = query.queryItemValue(
              QStringLiteral("show_all"));
          bool const show_all =
              (show_all_str == QStringLiteral("true"));
          auto limit = query_int(
              query.queryItemValue(QStringLiteral("limit")));
          auto offset = query_int(
              query.queryItemValue(QStringLiteral("offset")));
          auto action = query.queryItemValue(
              QStringLiteral("action"));
          auto rows = queries_.query_combined_history(
              is_admin, from_date, to_date, show_all,
              limit, offset, action);
          QJsonArray arr;
          for(auto const& r : rows) {
            arr.append(history_row_to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/history"),
        Method::Put,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("change_actions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto old_action = history_row_from_json(
              obj[QStringLiteral("old")].toObject());
          auto new_action = history_row_from_json(
              obj[QStringLiteral("new")].toObject());
          queries_.apply_action_change(
              old_action, new_action);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/history"),
        Method::Delete,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("change_actions"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto arr = parse_body_array(req);
          std::vector<HistoryCombinedModel::Row> actions;
          for(auto const& v : arr) {
            actions.push_back(
                history_row_from_json(v.toObject()));
          }
          queries_.batch_delete_history_actions(actions);
          return SC::NoContent;
        });

  // ---------------------------------------------------------
  // Classes — fixed paths first
  // ---------------------------------------------------------

    server_.route(
        QStringLiteral(
            "/api/classes/affected-persons"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("manage_classes"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          QMap<QString, QString> old_to_new;
          for(auto it = obj.constBegin();
              it != obj.constEnd(); ++it) {
            old_to_new[it.key()] =
                it.value().toString();
          }
          auto persons =
              queries_.get_affected_persons(old_to_new);
          QJsonArray arr;
          for(auto const& p : persons) {
            arr.append(affected_person_to_json(p));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/classes/year-changes"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("manage_classes"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          QMap<int, QString> person_to_class;
          auto pm =
              obj[QStringLiteral("person_id_to_new_class")]
                  .toObject();
          for(auto it = pm.constBegin();
              it != pm.constEnd(); ++it) {
            person_to_class[it.key().toInt()] =
                it.value().toString();
          }
          QList<int> to_delete;
          for(auto const& v :
              obj[QStringLiteral("persons_to_delete")]
                  .toArray()) {
            to_delete.append(v.toInt());
          }
          QStringList new_class_list;
          for(auto const& v :
              obj[QStringLiteral("new_class_list")]
                  .toArray()) {
            new_class_list << v.toString();
          }
          queries_.apply_school_year_changes(
              person_to_class, to_delete, new_class_list);
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral("/api/classes/<arg>"),
        Method::Put,
        [this](QString const& old_name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("manage_classes"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto new_name =
              obj[QStringLiteral("new_name")].toString();
          queries_.rename_class_name(old_name, new_name);
          return SC::Ok;
        });

    server_.route(
        QStringLiteral("/api/classes/<arg>"),
        Method::Delete,
        [this](QString const& name,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("manage_classes"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          queries_.remove_class_name(name);
          return SC::NoContent;
        });

    server_.route(
        QStringLiteral("/api/classes"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              {});
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto names = queries_.load_class_names();
          QJsonArray arr;
          for(auto const& s : names) {
            arr.append(s);
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/classes"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("manage_classes"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          auto name =
              obj[QStringLiteral("name")].toString().trimmed();
          if(name.isEmpty()) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          queries_.add_class_name(name);
          return SC::Created;
        });

  // ---- Reservations -------------------------------------------------------

    server_.route(
        QStringLiteral("/api/reservations"),
        Method::Get,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("reserve"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto query = QUrlQuery(req.url().query());
          auto person_id = query_int(
              query.queryItemValue(QStringLiteral("person_id")), -1);
          auto item_id = query_int(
              query.queryItemValue(QStringLiteral("item_id")), -1);
          if(item_id >= 0) {
            auto ress = queries_.get_reservations_for_item(item_id);
            QJsonArray arr;
            for(auto const& r : ress) {
              arr.append(to_json(r));
            }
            return json_resp(arr);
          }
          bool const can_edit = queries_.has_permission(
              ar.person_id, QStringLiteral("edit_reservations"));
          auto const pid =
              can_edit ? person_id : ar.person_id;
          auto ress = queries_.get_reservations(pid);
          QJsonArray arr;
          for(auto const& r : ress) {
            arr.append(to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/items/<arg>/reservations"),
        Method::Get,
        [this](int item_id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("reserve"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto ress = queries_.get_reservations_for_item(item_id);
          QJsonArray arr;
          for(auto const& r : ress) {
            arr.append(to_json(r));
          }
          return json_resp(arr);
        });

    server_.route(
        QStringLiteral("/api/reservations"),
        Method::Post,
        [this](QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("reserve"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          auto obj = parse_body(req);
          IQueries::Reservation r;
          if(!from_json(obj, r)) {
            return json_error(
                QStringLiteral("Bad request"), SC::BadRequest);
          }
          if(queries_.is_reserve_only(ar.person_id)) {
            r.person_id = ar.person_id;
          }
          {
            Item item;
            QString tmp;
            queries_.get_item(r.item_id, item, tmp);
            Collection const col = queries_.read_collection();
            bool item_allows = col.get_allow_reservations();
            if(item.get_allow_reservations() != -1) {
              item_allows = (item.get_allow_reservations() != 0);
            }
            if(!item_allows) {
              return QHttpServerResponse(
                  QJsonObject{{QStringLiteral("error"),
                               QStringLiteral("reservations_disabled_for_item")}},
                  SC::Forbidden);
            }
          }
          int new_id = 0;
          auto result = queries_.add_reservation(
              r.item_id, r.person_id, r.count, r.expires_at,
              ar.person_id, new_id);
          switch(result) {
          case IQueries::ReservationResult::Ok:
            return json_id(new_id);
          case IQueries::ReservationResult::AlreadyExists:
            return QHttpServerResponse(SC::Conflict);
          case IQueries::ReservationResult::NotEnoughAvailable:
            return QHttpServerResponse(SC::UnprocessableEntity);
          case IQueries::ReservationResult::Error:
            return QHttpServerResponse(SC::InternalServerError);
          }
          return QHttpServerResponse(SC::InternalServerError);
        });

    server_.route(
        QStringLiteral("/api/reservations/<arg>"),
        Method::Delete,
        [this](int id,
               QHttpServerRequest const& req)
            -> QHttpServerResponse {
          auto ar = auth_.authenticate(
              QString::fromUtf8(req.value("Authorization")),
              QStringLiteral("reserve"));
          if(ar.error != AuthMiddleware::AuthError::Ok) {
            return auth_error(ar.error);
          }
          bool const can_edit = queries_.has_permission(
              ar.person_id, QStringLiteral("edit_reservations"));
          if(!can_edit) {
            auto ress = queries_.get_reservations(ar.person_id);
            bool owned = false;
            for(auto const& res : ress) {
              if(res.id == id) {
                owned = true;
                break;
              }
            }
            if(!owned) {
              return json_error(QStringLiteral(
                "You do not have permission for this action"),
                SC::Forbidden);
            }
          }
          return queries_.remove_reservation(id) ? SC::Ok : SC::NotFound;
        });
    setup_extra_routes();
  }

} // namespace colman

/* EOF */
