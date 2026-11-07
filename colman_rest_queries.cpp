// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// RestQueries implementation.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_REST_QUERIES_HPP
#  include "colman_rest_queries.hpp"
#endif
#ifndef COLMAN_JSON_HPP
#  include "colman_json.hpp"
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
#ifndef SYSTEM_QNETWORKREPLY
#  include <QNetworkReply>
#  define SYSTEM_QNETWORKREPLY
#endif
#ifndef SYSTEM_QNETWORKREQUEST
#  include <QNetworkRequest>
#  define SYSTEM_QNETWORKREQUEST
#endif
#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
#ifndef SYSTEM_QURLQUERY
#  include <QUrlQuery>
#  define SYSTEM_QURLQUERY
#endif
#ifndef SYSTEM_QSSLERROR
#  include <QSslError>
#  define SYSTEM_QSSLERROR
#endif
#ifndef SYSTEM_QSSLCERTIFICATE
#  include <QSslCertificate>
#  define SYSTEM_QSSLCERTIFICATE
#endif
#ifndef SYSTEM_QCRYPTOGRAPHICHASH
#  include <QCryptographicHash>
#  define SYSTEM_QCRYPTOGRAPHICHASH
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef COLMAN_TYPE_HPP
#  include "colman_type.hpp"
#endif
#ifndef COLMAN_PERSON_HPP
#  include "colman_person.hpp"
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif

namespace
{

  /// Returns the language code of the user interface for Accept-Language.
  /**
     Mirrors how QTranslator::load(QLocale::system(), ...) picks the
     application translation: the first UI language (LANGUAGE before LANG)
     for which colman has a translation, or English, the source language.
     The server then returns type and field names in the language the user
     actually sees.
     @return two-letter language code, e.g. "de" or "en"
  */
  QByteArray ui_language_code()
  {
    for(QString const& ui : QLocale::system().uiLanguages()) {
      QString const code = QLocale(ui).name().section('_', 0, 0);
      if(code == QLatin1String("en") ||
         QFile::exists(":/translations/colman_" + code + ".qm"))
      {
        return code.toLatin1();
      }
    }
    return QByteArrayLiteral("en");
  }

  QByteArray json_body(QJsonObject const& obj)
  {
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
  }

  QByteArray json_body(QJsonArray const& arr)
  {
    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
  }

  QJsonObject parse_obj(QByteArray const& data)
  {
    return QJsonDocument::fromJson(data).object();
  }

  QJsonArray parse_arr(QByteArray const& data)
  {
    return QJsonDocument::fromJson(data).array();
  }

  QString str_val(QJsonObject const& obj, QString const& key)
  {
    return obj.value(key).toString();
  }

  int int_val(QJsonObject const& obj, QString const& key)
  {
    return obj.value(key).toInt();
  }

  bool bool_val(QJsonObject const& obj, QString const& key)
  {
    return obj.value(key).toBool();
  }

  QString date_str(QDate const& d)
  {
    if(!d.isValid()) {
      return {};
    }
    return d.toString(Qt::ISODate);
  }

  QDate parse_date(QString const& s)
  {
    if(s.isEmpty()) {
      return {};
    }
    return QDate::fromString(s, Qt::ISODate);
  }

  QString datetime_str(QDateTime const& dt)
  {
    if(!dt.isValid()) {
      return {};
    }
    return dt.toString(Qt::ISODate);
  }

  QDateTime parse_datetime(QString const& s)
  {
    if(s.isEmpty()) {
      return {};
    }
    return QDateTime::fromString(s, Qt::ISODate);
  }

  colman::StartPagePersonListModel::Row
  start_page_person_row(QJsonObject const& o)
  {
    colman::StartPagePersonListModel::Row r;
    r.id = int_val(o, QStringLiteral("id"));
    r.lastname = str_val(o, QStringLiteral("lastname"));
    r.firstname = str_val(o, QStringLiteral("firstname"));
    r.title = str_val(o, QStringLiteral("title"));
    r.has_password = bool_val(o, QStringLiteral("has_password"));
    return r;
  }

  colman::ItemListModel::Row item_list_row(QJsonObject const& o)
  {
    colman::ItemListModel::Row r;
    r.id = int_val(o, QStringLiteral("id"));
    r.name = str_val(o, QStringLiteral("name"));
    r.count = int_val(o, QStringLiteral("count"));
    r.available = int_val(o, QStringLiteral("available"));
    r.type_translated =
        str_val(o, QStringLiteral("type_translated"));
    r.type_name = str_val(o, QStringLiteral("type_name"));
    r.collector = str_val(o, QStringLiteral("collector"));
    return r;
  }

  colman::OpenLendingListModel::Row open_lending_row(QJsonObject const& o)
  {
    colman::OpenLendingListModel::Row r;
    r.id = int_val(o, QStringLiteral("id"));
    r.lent_count = int_val(o, QStringLiteral("lent_count"));
    r.returned_count =
        int_val(o, QStringLiteral("returned_count"));
    r.datetime = parse_datetime(
        str_val(o, QStringLiteral("datetime")));
    r.duedate = parse_date(
        str_val(o, QStringLiteral("duedate")));
    r.remarks = str_val(o, QStringLiteral("remarks"));
    r.person_name = str_val(o, QStringLiteral("person_name"));
    return r;
  }

  colman::HistoryCombinedModel::Row history_row(QJsonObject const& o)
  {
    colman::HistoryCombinedModel::Row r;
    r.id = int_val(o, QStringLiteral("id"));
    r.datetime = parse_datetime(
        str_val(o, QStringLiteral("datetime")));
    r.action = str_val(o, QStringLiteral("action"));
    r.item_name = str_val(o, QStringLiteral("item_name"));
    r.count = int_val(o, QStringLiteral("count"));
    r.user_name = str_val(o, QStringLiteral("user_name"));
    r.collector = str_val(o, QStringLiteral("collector"));
    r.item_id = int_val(o, QStringLiteral("item_id"));
    r.person_id = int_val(o, QStringLiteral("person_id"));
    r.remarks = str_val(o, QStringLiteral("remarks"));
    r.collector_id = int_val(o, QStringLiteral("collector_id"));
    r.duedate = parse_date(
        str_val(o, QStringLiteral("duedate")));
    return r;
  }

  QJsonObject history_row_to_json(colman::HistoryCombinedModel::Row const& r)
  {
    QJsonObject o;
    o[QStringLiteral("id")] = r.id;
    o[QStringLiteral("datetime")] = datetime_str(r.datetime);
    o[QStringLiteral("action")] = r.action;
    o[QStringLiteral("item_name")] = r.item_name;
    o[QStringLiteral("count")] = r.count;
    o[QStringLiteral("user_name")] = r.user_name;
    o[QStringLiteral("collector")] = r.collector;
    o[QStringLiteral("item_id")] = r.item_id;
    o[QStringLiteral("person_id")] = r.person_id;
    o[QStringLiteral("remarks")] = r.remarks;
    o[QStringLiteral("collector_id")] = r.collector_id;
    o[QStringLiteral("duedate")] = date_str(r.duedate);
    return o;
  }

  colman::ChangePasswordsPersonListModel::Row
  change_passwords_row(QJsonObject const& o)
  {
    colman::ChangePasswordsPersonListModel::Row r;
    r.id = int_val(o, QStringLiteral("id"));
    r.lastname = str_val(o, QStringLiteral("lastname"));
    r.firstname = str_val(o, QStringLiteral("firstname"));
    r.title = str_val(o, QStringLiteral("title"));
    r.username = str_val(o, QStringLiteral("username"));
    return r;
  }

  colman::DueLendingListModel::Row due_lending_row(QJsonObject const& o)
  {
    colman::DueLendingListModel::Row r;
    r.lending_id = int_val(o, QStringLiteral("lending_id"));
    r.item_id = int_val(o, QStringLiteral("item_id"));
    r.person_id = int_val(o, QStringLiteral("person_id"));
    r.duedate = parse_date(
        str_val(o, QStringLiteral("duedate")));
    r.person_name = str_val(o, QStringLiteral("person_name"));
    // The server appends " (class)" to person_name for older clients;
    // strip it again since the class is shown in a column of its own.
    r.school_class = str_val(o, QStringLiteral("school_class"));
    QString const suffix = " (" + r.school_class + ")";
    if(!r.school_class.isEmpty() && r.person_name.endsWith(suffix)) {
      r.person_name.chop(suffix.size());
    }
    r.outstanding_count =
        int_val(o, QStringLiteral("outstanding_count"));
    r.item_name = str_val(o, QStringLiteral("item_name"));
    return r;
  }

  colman::IQueries::LendingInfo lending_info(QJsonObject const& o)
  {
    colman::IQueries::LendingInfo info;
    info.lending_id_ = int_val(o, QStringLiteral("lending_id"));
    info.lending_count_ =
        int_val(o, QStringLiteral("lending_count"));
    info.item_name_ = str_val(o, QStringLiteral("item_name"));
    return info;
  }

  colman::IQueries::LendingPersonInfo
  lending_person_info(QJsonObject const& o)
  {
    colman::IQueries::LendingPersonInfo info;
    info.lending_id_ =
        int_val(o, QStringLiteral("lending_id"));
    info.person_last_name_ =
        str_val(o, QStringLiteral("person_last_name"));
    info.person_first_name_ =
        str_val(o, QStringLiteral("person_first_name"));
    info.person_title_ =
        str_val(o, QStringLiteral("person_title"));
    return info;
  }

  colman::IQueries::SearchItemRow
  search_item_row(QJsonObject const& o)
  {
    colman::IQueries::SearchItemRow r;
    r.id = int_val(o, QStringLiteral("id"));
    r.name = str_val(o, QStringLiteral("name"));
    r.type_locale = str_val(o, QStringLiteral("type_locale"));
    r.count = int_val(o, QStringLiteral("count"));
    r.available = int_val(o, QStringLiteral("available"));
    r.type_id = int_val(o, QStringLiteral("type_id"));
    return r;
  }

  colman::IQueries::ItemTypeThumbnail
  item_type_thumbnail(QJsonObject const& o)
  {
    colman::IQueries::ItemTypeThumbnail t;
    t.item_id_ = int_val(o, QStringLiteral("item_id"));
    t.item_name_ = str_val(o, QStringLiteral("item_name"));
    t.type_name_ = str_val(o, QStringLiteral("type_name"));
    t.thumb_id_ = int_val(o, QStringLiteral("thumb_id"));
    return t;
  }

  colman::IQueries::AffectedPerson
  affected_person(QJsonObject const& o)
  {
    colman::IQueries::AffectedPerson p;
    p.person_id_ = int_val(o, QStringLiteral("person_id"));
    p.display_name_ =
        str_val(o, QStringLiteral("display_name"));
    p.old_class_ = str_val(o, QStringLiteral("old_class"));
    p.new_class_ = str_val(o, QStringLiteral("new_class"));
    return p;
  }

  colman::IQueries::UpdateResult
  update_result_from_status(int status)
  {
    if(status == 200) {
      return colman::IQueries::UpdateResult::Ok;
    }
    if(status == 412) {
      return colman::IQueries::UpdateResult::VersionMismatch;
    }
    if(status == 404) {
      return colman::IQueries::UpdateResult::NotFound;
    }
    return colman::IQueries::UpdateResult::Error;
  }

  QString error_message_from_body(QByteArray const& body)
  {
    auto obj = QJsonDocument::fromJson(body).object();
    auto msg = obj[QStringLiteral("error")].toString();
    if(msg.isEmpty()) {
      msg = obj[QStringLiteral("message")].toString();
    }
    return msg;
  }

} // namespace

namespace colman
{

  RestQueries::RestQueries()
  {
    QObject::connect(
        &nam_, &QNetworkAccessManager::sslErrors,
        [](QNetworkReply* reply, QList<QSslError> const& errors) {
          auto const cert = errors.isEmpty()
              ? QSslCertificate{}
              : errors.first().certificate();
          if(cert.isNull()) {
            return;
          }
          auto const fingerprint =
              cert.digest(QCryptographicHash::Sha256);
          auto const host = reply->url().host();
          QSettings s;
          auto const key =
              QStringLiteral("trusted_cert/%1").arg(host);
          if(s.value(key).toByteArray() == fingerprint) {
            reply->ignoreSslErrors();
            return;
          }
          auto const fp_str =
              QString::fromLatin1(fingerprint.toHex(':'));
          auto const btn = QMessageBox::warning(
              nullptr,
              QApplication::tr("Untrusted Certificate"),
              QApplication::tr(
                  "The server \"%1\" uses a certificate that "
                  "could not be verified.\n\n"
                  "Subject: %2\n"
                  "Valid:    %3 – %4\n"
                  "SHA-256:  %5\n\n"
                  "Trust this server?")
                  .arg(host,
                       cert.subjectDisplayName(),
                       cert.effectiveDate()
                           .toLocalTime()
                           .toString(Qt::ISODate),
                       cert.expiryDate()
                           .toLocalTime()
                           .toString(Qt::ISODate),
                       fp_str),
              QMessageBox::Yes | QMessageBox::No,
              QMessageBox::No);
          if(btn == QMessageBox::Yes) {
            s.setValue(key, fingerprint);
            reply->ignoreSslErrors();
          }
        });
  }

  void RestQueries::set_config(RestConfig const& config)
  {
    config_ = config;
  }

  RestConfig const& RestQueries::config() const
  {
    return config_;
  }

/// Sends GET /api/collection; returns status and any JSON error message.
  RestReply RestQueries::test_connection()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/collection"));
    RestReply result;
    result.status = r.status;
    if(r.status != 0 && r.status != 200) {
      result.error_message = error_message_from_body(r.body);
    }
    return result;
  }

/// Sends GET /api/version and parses major/minor from the JSON response.
/** Returns {0,0} when the request fails or the response cannot be parsed. */
  ApiVersion RestQueries::fetch_api_version()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/version"));
    if(r.status != 200) {
      return {};
    }
    auto obj = QJsonDocument::fromJson(r.body).object();
    ApiVersion v;
    v.major_ = obj[QStringLiteral("major")].toInt();
    v.minor_ = obj[QStringLiteral("minor")].toInt();
    v.name_ = obj[QStringLiteral("name")].toString();
    return v;
  }

/// Sends GET /api/me and returns the logged-in user's person_id.
/** Returns -1 on network error or unexpected response.
    Also caches person_id and permissions for use in get_user_permissions(). */
  int RestQueries::fetch_logged_in_person_id()
  {
    auto r = send(QStringLiteral("GET"), QStringLiteral("/api/me"));
    if(r.status != 200) {
      return -1;
    }
    auto obj = QJsonDocument::fromJson(r.body).object();
    logged_in_person_id_ = obj[QStringLiteral("person_id")].toInt(-1);
    logged_in_permissions_ =
        obj[QStringLiteral("permissions")].toObject();
    return logged_in_person_id_;
  }

  bool RestQueries::is_open() const
  {
    return !config_.base_url.isEmpty();
  }

/** Builds a QNetworkRequest for \a path, attaches auth and content-type
    headers, dispatches the request via the shared QNetworkAccessManager,
    and runs a local QEventLoop until the reply finishes or the configured
    timeout elapses.  If the timer fires before the reply finishes the
    reply is aborted and status 0 is returned.
*/
  RestQueries::Reply
  RestQueries::send(QString const& method,
                    QString const& path,
                    QByteArray const& body,
                    QString const& content_type)
  {
    auto url = QUrl(config_.base_url + path);
    QNetworkRequest req(url);
    if(!config_.token.isEmpty()) {
      req.setRawHeader(
          "Authorization",
          QStringLiteral("Bearer %1").arg(config_.token).toUtf8());
    }
    else {
      QByteArray const creds =
          (config_.username + ":" + config_.password).toUtf8();
      req.setRawHeader("Authorization", "Basic " + creds.toBase64());
    }
    req.setRawHeader("Accept-Language", ui_language_code());
    if(!body.isEmpty()) {
      req.setHeader(QNetworkRequest::ContentTypeHeader,
                    content_type);
    }
    QNetworkReply* reply = nullptr;
    if(method == QStringLiteral("GET")) {
      reply = nam_.get(req);
    }
    else if(method == QStringLiteral("POST")) {
      reply = nam_.post(req, body);
    }
    else if(method == QStringLiteral("PUT")) {
      reply = nam_.put(req, body);
    }
    else if(method == QStringLiteral("DELETE")) {
      if(body.isEmpty()) {
        reply = nam_.deleteResource(req);
      }
      else {
        req.setHeader(QNetworkRequest::ContentTypeHeader,
                      content_type);
        reply = nam_.sendCustomRequest(req, "DELETE", body);
      }
    }
    else {
      reply = nam_.sendCustomRequest(
          req, method.toLatin1(), body);
    }
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished,
                     &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout,
                     &loop, &QEventLoop::quit);
    timer.start(config_.timeout_ms);
    loop.exec();
    Reply result;
    if(!reply->isFinished()) {
      reply->abort();
      reply->deleteLater();
      return result;
    }
    auto attr = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute);
    result.status = attr.toInt();
    result.body = reply->readAll();
    result.etag = reply->rawHeader("ETag");
    reply->deleteLater();
    return result;
  }

/** Calls send() after inserting an @c If-Match header whose value is the
    decimal string representation of \a expected_version.
*/
  RestQueries::Reply
  RestQueries::send_versioned(QString const& method,
                              QString const& path,
                              QByteArray const& body,
                              int expected_version)
  {
    auto url = QUrl(config_.base_url + path);
    QNetworkRequest req(url);
    if(!config_.token.isEmpty()) {
      req.setRawHeader(
          "Authorization",
          QStringLiteral("Bearer %1").arg(config_.token).toUtf8());
    }
    else {
      QByteArray const creds =
          (config_.username + ":" + config_.password).toUtf8();
      req.setRawHeader("Authorization", "Basic " + creds.toBase64());
    }
    req.setRawHeader(
        "If-Match",
        QString::number(expected_version).toUtf8());
    req.setRawHeader("Accept-Language", ui_language_code());
    if(!body.isEmpty()) {
      req.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/json"));
    }
    QNetworkReply* reply = nullptr;
    if(method == QStringLiteral("PUT")) {
      reply = nam_.put(req, body);
    }
    else {
      reply = nam_.sendCustomRequest(
          req, method.toLatin1(), body);
    }
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished,
                     &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout,
                     &loop, &QEventLoop::quit);
    timer.start(config_.timeout_ms);
    loop.exec();
    Reply result;
    if(!reply->isFinished()) {
      reply->abort();
      reply->deleteLater();
      return result;
    }
    auto attr = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute);
    result.status = attr.toInt();
    result.body = reply->readAll();
    reply->deleteLater();
    return result;
  }

// ---------------------------------------------------------------------------
// Collection
// ---------------------------------------------------------------------------

  Collection RestQueries::read_collection()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/collection"));
    if(r.status != 200) {
      return {};
    }
    auto o = parse_obj(r.body);
    Collection c;
    c.set_name(str_val(o, QStringLiteral("name")));
    c.set_organization(
        str_val(o, QStringLiteral("organization")));
    c.set_responsible(
        str_val(o, QStringLiteral("responsible")));
    c.set_barcode_prefix(
        str_val(o, QStringLiteral("barcode_prefix")));
    c.set_use_lending_period(
        o[QStringLiteral("use_lending_period")].toBool());
    c.set_lending_period_years(
        o[QStringLiteral("lending_period_years")].toInt());
    c.set_lending_period_months(
        o[QStringLiteral("lending_period_months")].toInt());
    c.set_lending_period_days(
        o[QStringLiteral("lending_period_days")].toInt());
    c.set_allow_reservations(
        o[QStringLiteral("allow_reservations")].toBool());
    c.set_reservation_period_days(
        o[QStringLiteral("reservation_period_days")].toInt());
    c.set_default_item_type(
        o[QStringLiteral("default_item_type")].toInt());
    c.set_default_is_school_item(
        o[QStringLiteral("default_is_school_item")].toBool());
    auto logo_b64 = str_val(o, QStringLiteral("logo"));
    if(!logo_b64.isEmpty()) {
      c.set_logo_raw(
          QByteArray::fromBase64(logo_b64.toLatin1()));
    }
    c.set_grade_range_min(
        o[QStringLiteral("grade_range_min")].toInt(1));
    c.set_grade_range_max(
        o[QStringLiteral("grade_range_max")].toInt(13));
    auto parse_elem = [](QJsonObject const& eo,
                         IdCardElementType t) {
      IdCardElement e;
      e.type_ = t;
      e.orientation_ =
          eo[QStringLiteral("orientation")].toInt();
      e.x_mm_ = eo[QStringLiteral("x_mm")].toDouble();
      e.y_mm_ = eo[QStringLiteral("y_mm")].toDouble();
      e.width_mm_ =
          eo[QStringLiteral("width_mm")].toDouble();
      e.height_mm_ =
          eo[QStringLiteral("height_mm")].toDouble();
      return e;
    };
    auto parse_layout = [&](QString const& key) {
      IdCardLayout lay;
      auto const lo = o[key].toObject();
      lay.card_width_mm_ =
          lo[QStringLiteral("card_width_mm")].toDouble();
      lay.card_height_mm_ =
          lo[QStringLiteral("card_height_mm")].toDouble();
      lay.has_logo_ =
          lo[QStringLiteral("has_logo")].toBool();
      lay.barcode_ = parse_elem(
          lo[QStringLiteral("barcode")].toObject(),
          IdCardElementType::Barcode);
      lay.text_ = parse_elem(
          lo[QStringLiteral("text")].toObject(),
          IdCardElementType::Text);
      lay.logo_ = parse_elem(
          lo[QStringLiteral("logo")].toObject(),
          IdCardElementType::Logo);
      return lay;
    };
    // The person ID-card layout is exposed read-only by the server so a
    // freshly installed client can seed its local copy once.  Item-card and
    // signature-label layouts are kept purely locally and no longer fetched.
    if(o.contains(QStringLiteral("id_card_layout"))) {
      c.set_id_card_layout(
          parse_layout(QStringLiteral("id_card_layout")));
    }
    return c;
  }

  bool RestQueries::change_collection(Collection const& collection)
  {
    QJsonObject o;
    o[QStringLiteral("name")] = collection.get_name();
    o[QStringLiteral("organization")] =
        collection.get_organization();
    o[QStringLiteral("responsible")] =
        collection.get_responsible();
    o[QStringLiteral("barcode_prefix")] =
        collection.get_barcode_prefix();
    o[QStringLiteral("use_lending_period")] =
        collection.get_use_lending_period();
    o[QStringLiteral("lending_period_years")] =
        collection.get_lending_period_years();
    o[QStringLiteral("lending_period_months")] =
        collection.get_lending_period_months();
    o[QStringLiteral("lending_period_days")] =
        collection.get_lending_period_days();
    o[QStringLiteral("allow_reservations")] =
        collection.get_allow_reservations();
    o[QStringLiteral("reservation_period_days")] =
        collection.get_reservation_period_days();
    o[QStringLiteral("default_item_type")] =
        collection.get_default_item_type();
    o[QStringLiteral("default_is_school_item")] =
        collection.get_default_is_school_item();
    o[QStringLiteral("logo")] = QString::fromLatin1(
        collection.get_logo_raw().toBase64());
    o[QStringLiteral("grade_range_min")] =
        collection.get_grade_range_min();
    o[QStringLiteral("grade_range_max")] =
        collection.get_grade_range_max();
    auto r = send(QStringLiteral("PUT"),
                  QStringLiteral("/api/collection"),
                  json_body(o));
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Signature categories
// ---------------------------------------------------------------------------

  std::vector<QString>
  RestQueries::read_signature_top_categories()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/signatures/top"));
    if(r.status != 200) {
      return {};
    }
    std::vector<QString> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toString());
    }
    return result;
  }

  bool RestQueries::add_signature_top_category(
      QString const& name)
  {
    QJsonObject o;
    o[QStringLiteral("name")] = name;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/signatures/top"),
                  json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_signature_top_category(
      QString const& name)
  {
    auto path = QStringLiteral("/api/signatures/top/") +
                QUrl::toPercentEncoding(name);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  std::vector<QString>
  RestQueries::read_signature_bottom_categories(
      QString const& top)
  {
  // An empty top means "all bottom categories regardless of top"; that
  // has its own route because <arg> does not match an empty path segment.
    auto path =
        top.isEmpty()
            ? QStringLiteral("/api/signatures/bottom")
            : QStringLiteral("/api/signatures/top/") +
                  QUrl::toPercentEncoding(top) +
                  QStringLiteral("/bottom");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<QString> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toString());
    }
    return result;
  }

  bool RestQueries::add_signature_bottom_category(
      QString const& top, QString const& name)
  {
    auto path =
        QStringLiteral("/api/signatures/top/") +
        QUrl::toPercentEncoding(top) +
        QStringLiteral("/bottom");
    QJsonObject o;
    o[QStringLiteral("name")] = name;
    auto r = send(QStringLiteral("POST"), path, json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_signature_bottom_category(
      QString const& top, QString const& name)
  {
    auto path =
        QStringLiteral("/api/signatures/top/") +
        QUrl::toPercentEncoding(top) +
        QStringLiteral("/bottom/") +
        QUrl::toPercentEncoding(name);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Item types
// ---------------------------------------------------------------------------

  int RestQueries::add_new_type(Type const& type,
                                QString const& tablename)
  {
    QJsonObject o;
    o[QStringLiteral("name")] = type.get_name();
    o[QStringLiteral("tablename")] = tablename;
    o[QStringLiteral("media_kind")] =
        static_cast<int>(type.get_media_kind());
    QJsonArray keys_arr;
    for(auto const& k : type.get_keys()) {
      QJsonObject ko;
      ko[QStringLiteral("column")] = k.first;
      ko[QStringLiteral("data_type")] = (int)k.second.type_;
      ko[QStringLiteral("info")] =
          QJsonObject::fromVariantMap(k.second.info_);
      keys_arr.append(ko);
    }
    o[QStringLiteral("keys")] = keys_arr;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/types"),
                  json_body(o));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  int RestQueries::get_type_id(QString const& type)
  {
    auto path =
        QStringLiteral("/api/types/by-name/") +
        QUrl::toPercentEncoding(type);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  int RestQueries::type_id_for_media_kind(MediaKind kind)
  {
    if(kind == MediaKind::Unknown) {
      return -1;
    }
    auto r = send(QStringLiteral("GET"), QStringLiteral("/api/types"));
    if(r.status != 200) {
      return -1;
    }
    // The listing is ordered by name, so scan for the lowest id whose
    // media_kind matches (lowest id wins, as on the SQL side).
    int best = -1;
    for(auto const& v : parse_arr(r.body)) {
      auto o = v.toObject();
      if(o[QStringLiteral("media_kind")].toInt() == static_cast<int>(kind)) {
        int const id = o[QStringLiteral("id")].toInt();
        if(best < 0 || id < best) {
          best = id;
        }
      }
    }
    return best;
  }

  MediaKind RestQueries::media_kind_for_type_id(int type_id)
  {
    return get_type(type_id).get_media_kind();
  }

  Type RestQueries::get_type(int type_id)
  {
    auto path = QStringLiteral("/api/types/") +
                QString::number(type_id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    auto o = parse_obj(r.body);
    Type t;
    t.set_id(type_id);
    t.set_name(str_val(o, QStringLiteral("name")));
    t.set_media_kind(static_cast<MediaKind>(
        int_val(o, QStringLiteral("media_kind"))));
    for(auto const& v : o[QStringLiteral("keys")].toArray()) {
      auto ko = v.toObject();
      KeyInfo ki;
      ki.first = ko[QStringLiteral("column")].toString();
      ki.second.type_ =
          DataType(ko[QStringLiteral("data_type")].toInt());
      ki.second.info_ =
          ko[QStringLiteral("info")].toObject().toVariantMap();
      t.add_key(ki);
    }
    return t;
  }

  QString RestQueries::get_type_name(int type_id)
  {
    auto path = QStringLiteral("/api/types/") +
                QString::number(type_id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    return str_val(parse_obj(r.body), QStringLiteral("name"));
  }

  std::vector<std::pair<int, QString>>
  RestQueries::get_type_locale(QLocale::Language /*lang*/)
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/types/locale"));
    if(r.status != 200) {
      return {};
    }
    std::vector<std::pair<int, QString>> result;
    for(auto const& v : parse_arr(r.body)) {
      auto o = v.toObject();
      result.emplace_back(
          int_val(o, QStringLiteral("id")),
          str_val(o, QStringLiteral("name")));
    }
    return result;
  }

  QString RestQueries::get_type_locale(int type_id, QLocale::Language /*lang*/)
  {
    auto path =
        QStringLiteral("/api/types/") +
        QString::number(type_id) +
        QStringLiteral("/locale");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    return str_val(parse_obj(r.body), QStringLiteral("value"));
  }

  QString RestQueries::get_type_locale(
      QString const& type, QLocale::Language /*lang*/)
  {
    auto path =
        QStringLiteral("/api/types/by-name/") +
        QUrl::toPercentEncoding(type) +
        QStringLiteral("/locale");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    return str_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::change_type(
      int type_id, Type const& new_type,
      QMap<QString, QString> const& display_names)
  {
    auto path = QStringLiteral("/api/types/") +
                QString::number(type_id);
    QJsonObject o;
    o[QStringLiteral("name")] = new_type.get_name();
    o[QStringLiteral("media_kind")] =
        static_cast<int>(new_type.get_media_kind());
    QJsonObject dn;
    for(auto it = display_names.cbegin();
        it != display_names.cend(); ++it) {
      dn[it.key()] = it.value();
    }
    o[QStringLiteral("display_names")] = dn;
    QJsonArray keys_arr;
    for(auto const& k : new_type.get_keys()) {
      QJsonObject ko;
      ko[QStringLiteral("column")] = k.first;
      ko[QStringLiteral("data_type")] = (int)k.second.type_;
      ko[QStringLiteral("info")] =
          QJsonObject::fromVariantMap(k.second.info_);
      keys_arr.append(ko);
    }
    o[QStringLiteral("keys")] = keys_arr;
    auto r = send(QStringLiteral("PUT"), path, json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_type(int type_id)
  {
    auto path = QStringLiteral("/api/types/") +
                QString::number(type_id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  QStringList RestQueries::get_combined_field_order(int type_id)
  {
    auto path =
        QStringLiteral("/api/types/") +
        QString::number(type_id) +
        QStringLiteral("/field-order");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  QStringList RestQueries::get_default_common_field_order()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral(
                      "/api/types/field-order/default"));
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  bool RestQueries::save_field_order(
      int type_id, QStringList const& field_order)
  {
    auto path =
        QStringLiteral("/api/types/") +
        QString::number(type_id) +
        QStringLiteral("/field-order");
    QJsonArray arr;
    for(auto const& s : field_order) {
      arr.append(s);
    }
    auto r = send(QStringLiteral("PUT"), path, json_body(arr));
    return r.status >= 200 && r.status < 300;
  }

  std::vector<QString> RestQueries::get_type_names()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/types/names"));
    if(r.status != 200) {
      return {};
    }
    std::vector<QString> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toString());
    }
    return result;
  }

  QString RestQueries::get_type_field_label(
      int type_id, QString const& field,
      QLocale::Language lang)
  {
    auto it = type_label_cache_.find(type_id);
    if(it == type_label_cache_.end()) {
      auto path =
          QStringLiteral("/api/types/") + QString::number(type_id);
      auto const lang_int = static_cast<int>(lang);
      if(lang_int != 0) {
        path += QStringLiteral("?lang=") + QString::number(lang_int);
      }
      auto r = send(QStringLiteral("GET"), path);
      if(r.status != 200) {
        return {};
      }
      auto o = parse_obj(r.body);
      QMap<QString, QString> labels;
      for(auto const& v : o[QStringLiteral("keys")].toArray()) {
        auto ko = v.toObject();
        labels[ko[QStringLiteral("column")].toString()] =
            ko[QStringLiteral("label")].toString();
      }
      it = type_label_cache_.insert(type_id, labels);
    }
    return it.value().value(field);
  }

  std::vector<IQueries::TypeFieldMeta>
  RestQueries::get_type_field_metas(int type_id)
  {
    auto path =
        QStringLiteral("/api/types/") +
        QString::number(type_id) +
        QStringLiteral("/field-metas");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<TypeFieldMeta> result;
    for(auto const& v : parse_arr(r.body)) {
      auto o = v.toObject();
      TypeFieldMeta m;
      m.column = str_val(o, QStringLiteral("column"));
      m.marc21_tag =
          str_val(o, QStringLiteral("marc21_tag"));
      result.push_back(m);
    }
    return result;
  }

// ---------------------------------------------------------------------------
// Items
// ---------------------------------------------------------------------------

  int RestQueries::add_new_item(Item const& item)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/items"),
                  json_body(to_json(item)));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  bool RestQueries::get_item(int id, Item& item,
                             QString& type_name)
  {
    auto path = QStringLiteral("/api/items/") +
                QString::number(id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    auto o = parse_obj(r.body);
    type_name = str_val(o, QStringLiteral("type_name"));
    if(!from_json(o, item)) {
      return false;
    }
    item.set_version(r.etag.toInt());
    return true;
  }

  bool RestQueries::remove_item(int id)
  {
    auto path = QStringLiteral("/api/items/") +
                QString::number(id) +
                QStringLiteral("?force=false");
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::force_remove_item(int id)
  {
    auto path = QStringLiteral("/api/items/") +
                QString::number(id) +
                QStringLiteral("?force=true");
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::change_item(int id, Item const& item)
  {
    auto path = QStringLiteral("/api/items/") +
                QString::number(id);
    auto get_r = send(QStringLiteral("GET"), path);
    if(get_r.status != 200) {
      return false;
    }
    auto const version = get_r.etag.toInt();
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(item)), version);
    return r.status >= 200 && r.status < 300;
  }

  unsigned int RestQueries::available(int item_id)
  {
    auto path =
        QStringLiteral("/api/items/") +
        QString::number(item_id) +
        QStringLiteral("/available");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return 0;
    }
    return static_cast<unsigned int>(
        int_val(parse_obj(r.body), QStringLiteral("value")));
  }

  unsigned int RestQueries::stock_size(int item_id)
  {
    auto path =
        QStringLiteral("/api/items/") +
        QString::number(item_id) +
        QStringLiteral("/stock");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return 0;
    }
    return static_cast<unsigned int>(
        int_val(parse_obj(r.body), QStringLiteral("value")));
  }

  bool RestQueries::have_lendings(int item_id)
  {
    auto path =
        QStringLiteral("/api/items/") +
        QString::number(item_id) +
        QStringLiteral("/have-lendings");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return bool_val(parse_obj(r.body), QStringLiteral("value"));
  }

  std::vector<std::pair<int, int>>
  RestQueries::open_lendings_for_item(int item_id)
  {
    auto path =
        QStringLiteral("/api/items/") +
        QString::number(item_id) +
        QStringLiteral("/open-lendings");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<std::pair<int, int>> result;
    for(auto const& v : parse_arr(r.body)) {
      auto o = v.toObject();
      result.emplace_back(
          int_val(o, QStringLiteral("lending_id")),
          int_val(o, QStringLiteral("person_id")));
    }
    return result;
  }

  std::vector<std::pair<int, int>>
  RestQueries::open_lendings_for_person(int person_id)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/open-lendings");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<std::pair<int, int>> result;
    for(auto const& v : parse_arr(r.body)) {
      auto o = v.toObject();
      result.emplace_back(
          int_val(o, QStringLiteral("lending_id")),
          int_val(o, QStringLiteral("item_id")));
    }
    return result;
  }

  std::vector<int> RestQueries::items_by_isbn(QString const& isbn)
  {
    auto path =
        QStringLiteral("/api/items/by-isbn/") +
        QUrl::toPercentEncoding(isbn);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<int> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toInt());
    }
    return result;
  }

  std::vector<int> RestQueries::items_by_ean(QString const& ean)
  {
    auto path =
        QStringLiteral("/api/items/by-ean/") +
        QUrl::toPercentEncoding(ean);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<int> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toInt());
    }
    return result;
  }

  QString RestQueries::compute_item_barcode(
      int type_id, int item_id)
  {
    auto path =
        QStringLiteral("/api/items/barcode?type_id=") +
        QString::number(type_id) +
        QStringLiteral("&item_id=") +
        QString::number(item_id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    return str_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::parse_item_barcode(
      QString const& barcode, int& type_id, int& item_id)
  {
    QJsonObject o;
    o[QStringLiteral("barcode")] = barcode;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/items/barcode/parse"),
                  json_body(o));
    if(r.status != 200) {
      return false;
    }
    auto resp = parse_obj(r.body);
    type_id = int_val(resp, QStringLiteral("type_id"));
    item_id = int_val(resp, QStringLiteral("item_id"));
    return true;
  }

  bool RestQueries::is_item_barcode(QString const& barcode)
  {
    auto path =
        QStringLiteral("/api/items/barcode/check?barcode=") +
        QUrl::toPercentEncoding(barcode);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return bool_val(parse_obj(r.body), QStringLiteral("value"));
  }

  std::vector<std::pair<int, QString>>
  RestQueries::get_items_ordered_by_name()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral(
                      "/api/items/ordered-by-name"));
    if(r.status != 200) {
      return {};
    }
    std::vector<std::pair<int, QString>> result;
    for(auto const& v : parse_arr(r.body)) {
      auto o = v.toObject();
      result.emplace_back(
          int_val(o, QStringLiteral("id")),
          str_val(o, QStringLiteral("name")));
    }
    return result;
  }

  std::vector<int>
  RestQueries::get_recently_acquired_item_ids(int count)
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/items/recently-acquired?count=%1")
                      .arg(count));
    if(r.status != 200) {
      return {};
    }
    std::vector<int> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toObject().value(QStringLiteral("id")).toInt());
    }
    return result;
  }

  std::vector<int>
  RestQueries::get_items_acquired_between(QDateTime const& from,
                                          QDateTime const& to)
  {
    auto path =
        QStringLiteral("/api/items/acquired-between?from=%1&to=%2")
            .arg(QString::fromUtf8(
                     QUrl::toPercentEncoding(from.toString(Qt::ISODate))),
                 QString::fromUtf8(
                     QUrl::toPercentEncoding(to.toString(Qt::ISODate))));
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<int> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toObject().value(QStringLiteral("id")).toInt());
    }
    return result;
  }

  std::vector<IQueries::ItemTypeThumbnail>
  RestQueries::get_items_with_type_and_thumbnails()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral(
                      "/api/items/with-thumbnails"));
    if(r.status != 200) {
      return {};
    }
    std::vector<ItemTypeThumbnail> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(item_type_thumbnail(v.toObject()));
    }
    return result;
  }

// ---------------------------------------------------------------------------
// Acquisitions
// ---------------------------------------------------------------------------

  int RestQueries::add_new_acquisition(
      Acquisition const& acquisition)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/acquisitions"),
                  json_body(to_json(acquisition)));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  bool RestQueries::get_acquisition(int id,
                                    Acquisition& acquisition)
  {
    auto path = QStringLiteral("/api/acquisitions/") +
                QString::number(id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return from_json(parse_obj(r.body), acquisition);
  }

  bool RestQueries::change_acquisition(
      int id, Acquisition const& acquisition)
  {
    auto path = QStringLiteral("/api/acquisitions/") +
                QString::number(id);
    auto r = send(QStringLiteral("PUT"), path,
                  json_body(to_json(acquisition)));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_acquisition(int id)
  {
    auto path = QStringLiteral("/api/acquisitions/") +
                QString::number(id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Liquidations
// ---------------------------------------------------------------------------

  int RestQueries::add_new_liquidation(
      Liquidation const& liquidation)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/liquidations"),
                  json_body(to_json(liquidation)));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  bool RestQueries::get_liquidation(int id,
                                    Liquidation& liquidation)
  {
    auto path = QStringLiteral("/api/liquidations/") +
                QString::number(id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return from_json(parse_obj(r.body), liquidation);
  }

  bool RestQueries::change_liquidation(
      int id, Liquidation const& liquidation)
  {
    auto path = QStringLiteral("/api/liquidations/") +
                QString::number(id);
    auto r = send(QStringLiteral("PUT"), path,
                  json_body(to_json(liquidation)));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_liquidation(int id)
  {
    auto path = QStringLiteral("/api/liquidations/") +
                QString::number(id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Returns
// ---------------------------------------------------------------------------

  int RestQueries::add_new_return(Return const& ret)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/returns"),
                  json_body(to_json(ret)));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  bool RestQueries::get_return(int return_id, Return& ret)
  {
    auto path = QStringLiteral("/api/returns/") +
                QString::number(return_id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return from_json(parse_obj(r.body), ret);
  }

  bool RestQueries::change_return(int return_id,
                                  Return const& ret)
  {
    auto path = QStringLiteral("/api/returns/") +
                QString::number(return_id);
    auto r = send(QStringLiteral("PUT"), path,
                  json_body(to_json(ret)));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_return(int return_id)
  {
    auto path = QStringLiteral("/api/returns/") +
                QString::number(return_id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Lendings
// ---------------------------------------------------------------------------

  int RestQueries::add_new_lending(Lending const& lending)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/lendings"),
                  json_body(to_json(lending)));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  IQueries::GroupLendingResult RestQueries::add_group_lending(
      Lending const& lending, std::vector<int> const& person_ids)
  {
    using Status = GroupLendingResult::Status;
    GroupLendingResult result;
    QJsonArray ids;
    for(int const id : person_ids) {
      ids.append(id);
    }
    QJsonObject o;
    o[QStringLiteral("lending")] = to_json(lending);
    o[QStringLiteral("person_ids")] = ids;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/lendings/batch"),
                  json_body(o));
    if(r.status == 404) {
      result.status_ = Status::Unsupported;
      return result;
    }
    if(r.status == 409) {
      auto const err = parse_obj(r.body);
      if(err[QStringLiteral("error")].toString() ==
         QStringLiteral("already_lent"))
      {
        result.status_ = Status::AlreadyLent;
        result.person_id_ = int_val(err, QStringLiteral("person_id"));
      }
      else {
        result.status_ = Status::NotEnoughAvailable;
        result.lendable_ = int_val(err, QStringLiteral("available_count"));
      }
      return result;
    }
    if(r.status != 200 && r.status != 201) {
      return result;
    }
    for(auto const& v :
        parse_obj(r.body)[QStringLiteral("ids")].toArray()) {
      result.lending_ids_.push_back(v.toInt());
    }
    result.status_ = Status::Ok;
    return result;
  }

  IQueries::GroupReturnResult RestQueries::add_group_return(
      int item_id, std::vector<int> const& person_ids, Return const& tmpl)
  {
    using Status = GroupReturnResult::Status;
    GroupReturnResult result;
    QJsonArray ids;
    for(int const id : person_ids) {
      ids.append(id);
    }
    QJsonObject o;
    o[QStringLiteral("item_id")] = item_id;
    o[QStringLiteral("person_ids")] = ids;
    o[QStringLiteral("return")] = to_json(tmpl);
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/returns/batch"),
                  json_body(o));
    if(r.status == 404) {
      result.status_ = Status::Unsupported;
      return result;
    }
    if(r.status == 409) {
      result.status_ = Status::NotLent;
      result.person_id_ =
          int_val(parse_obj(r.body), QStringLiteral("person_id"));
      return result;
    }
    if(r.status != 200 && r.status != 201) {
      return result;
    }
    auto const obj = parse_obj(r.body);
    for(auto const& v : obj[QStringLiteral("ids")].toArray()) {
      result.return_ids_.push_back(v.toInt());
    }
    result.returned_count_ = int_val(obj, QStringLiteral("count"));
    result.status_ = Status::Ok;
    return result;
  }

  bool RestQueries::get_lending(int id, Lending& lending)
  {
    auto path = QStringLiteral("/api/lendings/") +
                QString::number(id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return from_json(parse_obj(r.body), lending);
  }

  bool RestQueries::change_lending(int id,
                                   Lending const& lending)
  {
    auto path = QStringLiteral("/api/lendings/") +
                QString::number(id);
    auto r = send(QStringLiteral("PUT"), path,
                  json_body(to_json(lending)));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_lending(int id)
  {
    auto path = QStringLiteral("/api/lendings/") +
                QString::number(id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::lending_balance(int lending_id,
                                    std::size_t& lent,
                                    std::size_t& returned)
  {
    auto path =
        QStringLiteral("/api/lendings/") +
        QString::number(lending_id) +
        QStringLiteral("/balance");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    auto o = parse_obj(r.body);
    lent = static_cast<std::size_t>(
        int_val(o, QStringLiteral("lent")));
    returned = static_cast<std::size_t>(
        int_val(o, QStringLiteral("returned")));
    return true;
  }

  std::vector<IQueries::LendingInfo>
  RestQueries::query_lendings(int person_id)
  {
    auto path =
        QStringLiteral("/api/lendings/person/") +
        QString::number(person_id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<LendingInfo> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(lending_info(v.toObject()));
    }
    return result;
  }

// ---------------------------------------------------------------------------
// Persons
// ---------------------------------------------------------------------------

  int RestQueries::add_new_person(Person const& person)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/persons"),
                  json_body(to_json(person)));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  bool RestQueries::get_person(int id, Person& person)
  {
    auto path = QStringLiteral("/api/persons/") +
                QString::number(id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    if(!from_json(parse_obj(r.body), person)) {
      return false;
    }
    person.set_version(r.etag.toInt());
    return true;
  }

  int RestQueries::get_person(QString const& barcode)
  {
    auto path =
        QStringLiteral("/api/persons/by-barcode/") +
        QUrl::toPercentEncoding(barcode);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::change_person(int id, Person const& person)
  {
    auto path = QStringLiteral("/api/persons/") +
                QString::number(id);
    // Fetch the current version to use as the optimistic-lock token.
    // The server requires a matching If-Match header on every PUT.
    auto get_r = send(QStringLiteral("GET"), path);
    if(get_r.status != 200) {
      return false;
    }
    auto const version = get_r.etag.toInt();
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(person)), version);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::email_known(QString const& /*emails*/)
  {
    return false;
  }

  bool RestQueries::username_known(QString const& /*username*/)
  {
    return false;
  }

  bool RestQueries::person_known(QString const& lastname,
                                 QString const& firstname)
  {
    auto path =
        QStringLiteral("/api/persons/known?lastname=") +
        QUrl::toPercentEncoding(lastname) +
        QStringLiteral("&firstname=") +
        QUrl::toPercentEncoding(firstname);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return bool_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::remove_person(int id)
  {
    auto path = QStringLiteral("/api/persons/") +
                QString::number(id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::has_non_admin_persons()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral(
                      "/api/persons/has-non-admin"));
    if(r.status != 200) {
      return false;
    }
    return bool_val(parse_obj(r.body), QStringLiteral("value"));
  }

  QString RestQueries::compute_person_barcode(int person_id)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/barcode");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    return str_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::is_person_barcode(QString const& str)
  {
    auto path =
        QStringLiteral(
            "/api/persons/barcode/check?barcode=") +
        QUrl::toPercentEncoding(str);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return bool_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::parse_person_barcode(
      QString const& barcode, int& person_id)
  {
    QJsonObject o;
    o[QStringLiteral("barcode")] = barcode;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral(
                      "/api/persons/barcode/parse"),
                  json_body(o));
    if(r.status != 200) {
      return false;
    }
    person_id = int_val(
        parse_obj(r.body), QStringLiteral("person_id"));
    return true;
  }

  int RestQueries::get_person_by_external_id(
      QString const& sub)
  {
    auto path =
        QStringLiteral("/api/persons/by-external-id/") +
        QUrl::toPercentEncoding(sub);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("value"));
  }

  bool RestQueries::set_person_external_id(
      int person_id, QString const& sub)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/external-id");
    QJsonObject o;
    o[QStringLiteral("sub")] = sub;
    auto r = send(QStringLiteral("PUT"), path, json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  int RestQueries::check_local_credentials(
      QString const& /*username*/, QString const& /*password*/)
  {
    return -1; // managed server-side only
  }

  bool RestQueries::set_local_credentials(
      int person_id,
      QString const& username,
      QString const& password_hash)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/password");
    QJsonObject body;
    body[QStringLiteral("username")] = username;
    body[QStringLiteral("password")] = password_hash;
    auto r = send(QStringLiteral("POST"), path, json_body(body));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::delete_local_credentials(int person_id)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/password");
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  QString RestQueries::get_username(int person_id)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/username");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    return parse_obj(r.body)[QStringLiteral("username")].toString();
  }

  bool RestQueries::set_username(int person_id, QString const& username)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/username");
    QJsonObject body;
    body[QStringLiteral("username")] = username;
    auto r = send(QStringLiteral("PUT"), path, json_body(body));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_username(int person_id)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/username");
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Permissions
// ---------------------------------------------------------------------------

  std::vector<QString> RestQueries::get_permissions() const
  {
    auto& self = const_cast<RestQueries&>(*this);
    auto r = self.send(QStringLiteral("GET"),
                       QStringLiteral("/api/permissions"));
    if(r.status != 200) {
      return {};
    }
    std::vector<QString> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toString());
    }
    return result;
  }

  std::vector<bool>
  RestQueries::get_user_permissions(int person_id)
  {
    if(person_id == logged_in_person_id_ &&
       !logged_in_permissions_.isEmpty()) {
      auto const cols = get_permissions();
      std::vector<bool> result;
      for(auto const& col : cols) {
        result.push_back(logged_in_permissions_[col].toBool());
      }
      return result;
    }
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/permissions");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<bool> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toBool());
    }
    return result;
  }

  bool RestQueries::create_default_permissions(int person_id)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/permissions/defaults");
    auto r = send(QStringLiteral("POST"), path, {});
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::has_permission(int person_id,
                                   QString const& column)
  {
    auto perms = get_user_permissions(person_id);
    auto cols = get_permissions();
    for(std::size_t i = 0; i < cols.size() && i < perms.size();
        ++i) {
      if(cols[i] == column) {
        return perms[i];
      }
    }
    return false;
  }

  bool RestQueries::set_permission(int person_id,
                                   QString const& column,
                                   bool value)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/permissions/") +
        QUrl::toPercentEncoding(column);
    QJsonObject o;
    o[QStringLiteral("value")] = value;
    auto r = send(QStringLiteral("PUT"), path, json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_permissions(int person_id)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/permissions");
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::is_reserve_only(int /*person_id*/)
  {
    return false;
  }

  bool RestQueries::set_password(int person_id, QString const& hash)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/password");
    QJsonObject body;
    body[QStringLiteral("password")] = hash;
    auto r = send(QStringLiteral("POST"), path, json_body(body));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::get_password(int person_id, QString& hash)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/password");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    if(!parse_obj(r.body)[QStringLiteral("has_password")].toBool()) {
      return false;
    }
    hash = QStringLiteral("set");
    return true;
  }

  bool RestQueries::has_password(int person_id)
  {
    auto const path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/password");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return false;
    }
    return parse_obj(r.body)[QStringLiteral("has_password")].toBool();
  }

  bool RestQueries::remove_password(int person_id)
  {
    auto path =
        QStringLiteral("/api/persons/") +
        QString::number(person_id) +
        QStringLiteral("/password");
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::change_own_password(int /*person_id*/,
                                        QString const& old_password,
                                        QString const& new_hash)
  {
    // The server identifies the caller from the Authorization header, so the
    // person_id argument is ignored here; the old password is verified
    // server-side against the stored hash.
    QJsonObject body;
    body[QStringLiteral("old_password")] = old_password;
    body[QStringLiteral("new_password")] = new_hash;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/me/password"), json_body(body));
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Thumbnails
// ---------------------------------------------------------------------------

  int RestQueries::store_thumbnail(QByteArray const& png,
                                   QString const& key,
                                   QString const& url,
                                   bool overwrite)
  {
    QJsonObject o;
    o[QStringLiteral("png")] =
        QString::fromLatin1(png.toBase64());
    o[QStringLiteral("key")] = key;
    o[QStringLiteral("url")] = url;
    o[QStringLiteral("overwrite")] = overwrite;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/thumbnails"),
                  json_body(o));
    if(r.status != 200 && r.status != 201) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("id"));
  }

  bool RestQueries::get_thumbnail(int thumbnail_id,
                                  QByteArray& png)
  {
    auto path =
        QStringLiteral("/api/thumbnails/") +
        QString::number(thumbnail_id);
    auto r = send(QStringLiteral("GET"), path, {},
                  QStringLiteral("image/png"));
    if(r.status != 200) {
      return false;
    }
    png = r.body;
    return true;
  }

// ---------------------------------------------------------------------------
// View-model queries
// ---------------------------------------------------------------------------

  std::vector<Type> RestQueries::query_types(int type_id,
                                             QLocale::Language /*lang*/)
  {
    QString path;
    if(type_id == -1) {
      path = QStringLiteral("/api/types");
    }
    else {
      path =
          QStringLiteral("/api/types/") +
          QString::number(type_id) +
          QStringLiteral("/query");
    }
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<Type> result;
    for(auto const& v : parse_arr(r.body)) {
      Type t;
      auto o = v.toObject();
      t.set_id(o[QStringLiteral("id")].toInt());
      t.set_name(str_val(o, QStringLiteral("name")));
      result.push_back(t);
    }
    return result;
  }

  std::vector<Person> RestQueries::query_persons(
      int person_id, QString const& search)
  {
    auto path =
        QStringLiteral("/api/persons?id=") +
        QString::number(person_id) +
        QStringLiteral("&search=") +
        QUrl::toPercentEncoding(search);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<Person> result;
    for(auto const& v : parse_arr(r.body)) {
      Person p;
      from_json(v.toObject(), p);
      result.push_back(p);
    }
    return result;
  }

  std::vector<int>
  RestQueries::get_recently_created_person_ids(int count)
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/persons/recently-created?count=%1")
                      .arg(count));
    if(r.status != 200) {
      return {};
    }
    std::vector<int> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(v.toObject().value(QStringLiteral("id")).toInt());
    }
    return result;
  }

  std::vector<Person> RestQueries::query_lending_persons(
      int item_id, int lending_id,
      QString const& person_search,
      QString const& item_search)
  {
    QJsonObject o;
    o[QStringLiteral("item_id")] = item_id;
    o[QStringLiteral("lending_id")] = lending_id;
    o[QStringLiteral("person_search")] = person_search;
    o[QStringLiteral("item_search")] = item_search;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral(
                      "/api/persons/lending-query"),
                  json_body(o));
    if(r.status != 200) {
      return {};
    }
    std::vector<Person> result;
    for(auto const& v : parse_arr(r.body)) {
      Person p;
      from_json(v.toObject(), p);
      result.push_back(p);
    }
    return result;
  }

  std::vector<StartPagePersonListModel::Row>
  RestQueries::query_start_page_persons(QString const& search)
  {
    auto path =
        QStringLiteral("/api/persons/start-page?search=") +
        QUrl::toPercentEncoding(search);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<StartPagePersonListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(
          start_page_person_row(v.toObject()));
    }
    return result;
  }

  int RestQueries::count_items()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/items/count"));
    if(r.status != 200) {
      return 0;
    }
    auto const obj = QJsonDocument::fromJson(r.body).object();
    return obj.value(QStringLiteral("count")).toInt(0);
  }

  std::vector<ItemListModel::Row>
  RestQueries::query_items(int item_id,
                           QString const& search,
                           QLocale::Language /*lang*/)
  {
    auto path =
        QStringLiteral("/api/items?id=") +
        QString::number(item_id) +
        QStringLiteral("&search=") +
        QUrl::toPercentEncoding(search);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<ItemListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(item_list_row(v.toObject()));
    }
    return result;
  }

  std::vector<ItemListModel::Row>
  RestQueries::query_acquired_items(int item_id,
                                    QString const& search,
                                    int limit,
                                    int offset,
                                    QLocale::Language /*lang*/)
  {
    auto path =
        QStringLiteral("/api/items/acquired?id=") +
        QString::number(item_id) +
        QStringLiteral("&search=") +
        QUrl::toPercentEncoding(search) +
        QStringLiteral("&limit=") + QString::number(limit) +
        QStringLiteral("&offset=") + QString::number(offset);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<ItemListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(item_list_row(v.toObject()));
    }
    return result;
  }

  std::vector<ItemListModel::Row>
  RestQueries::query_available_items(int item_id,
                                     QString const& search,
                                     int limit,
                                     int offset,
                                     QLocale::Language /*lang*/)
  {
    auto path =
        QStringLiteral("/api/items/available?id=") +
        QString::number(item_id) +
        QStringLiteral("&search=") +
        QUrl::toPercentEncoding(search) +
        QStringLiteral("&limit=") + QString::number(limit) +
        QStringLiteral("&offset=") + QString::number(offset);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<ItemListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(item_list_row(v.toObject()));
    }
    return result;
  }

  std::vector<ItemListModel::Row>
  RestQueries::query_lent_items(
      int person_id, int lending_id,
      QString const& person_search,
      QString const& item_search,
      QLocale::Language /*lang*/)
  {
    QJsonObject o;
    o[QStringLiteral("person_id")] = person_id;
    o[QStringLiteral("lending_id")] = lending_id;
    o[QStringLiteral("person_search")] = person_search;
    o[QStringLiteral("item_search")] = item_search;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/items/lent-query"),
                  json_body(o));
    if(r.status != 200) {
      return {};
    }
    std::vector<ItemListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(item_list_row(v.toObject()));
    }
    return result;
  }

  std::vector<ItemListModel::Row>
  RestQueries::query_items_with_type(int type_id,
                                     QString const& search,
                                     int limit,
                                     int offset,
                                     QLocale::Language /*lang*/)
  {
    auto path =
        QStringLiteral("/api/items/with-type?type_id=") +
        QString::number(type_id) +
        QStringLiteral("&search=") +
        QUrl::toPercentEncoding(search) +
        QStringLiteral("&limit=") + QString::number(limit) +
        QStringLiteral("&offset=") + QString::number(offset);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<ItemListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(item_list_row(v.toObject()));
    }
    return result;
  }

  std::vector<IQueries::SearchItemRow>
  RestQueries::query_items_for_search(
      QString const& general,
      QString const& name,
      QString const& subject,
      QString const& description,
      QString const& keywords,
      int type_id,
      std::vector<TypeFieldFilter> const& type_fields,
      int grade_min,
      int grade_max,
      int limit,
      int offset,
      QLocale::Language /*lang*/)
  {
    QJsonObject o;
    o[QStringLiteral("general")] = general;
    o[QStringLiteral("name")] = name;
    o[QStringLiteral("subject")] = subject;
    o[QStringLiteral("description")] = description;
    o[QStringLiteral("keywords")] = keywords;
    o[QStringLiteral("type_id")] = type_id;
    QJsonArray fa;
    for(auto const& f : type_fields) {
      QJsonObject fo;
      fo[QStringLiteral("column")] = f.column_;
      fo[QStringLiteral("value")] = f.value_;
      fa.append(fo);
    }
    o[QStringLiteral("type_fields")] = fa;
    o[QStringLiteral("grade_min")] = grade_min;
    o[QStringLiteral("grade_max")] = grade_max;
    o[QStringLiteral("limit")] = limit;
    o[QStringLiteral("offset")] = offset;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/items/search"),
                  json_body(o));
    if(r.status != 200) {
      return {};
    }
    std::vector<SearchItemRow> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(search_item_row(v.toObject()));
    }
    return result;
  }

  std::vector<OpenLendingListModel::Row>
  RestQueries::query_open_lendings(int person_id, int item_id)
  {
    auto path =
        QStringLiteral(
            "/api/lendings/open?person_id=") +
        QString::number(person_id) +
        QStringLiteral("&item_id=") +
        QString::number(item_id);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<OpenLendingListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(open_lending_row(v.toObject()));
    }
    return result;
  }

  std::vector<IQueries::LendingPersonInfo>
  RestQueries::query_item_lendings(int item_id)
  {
    auto path =
        QStringLiteral("/api/items/") +
        QString::number(item_id) +
        QStringLiteral("/item-lendings");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<LendingPersonInfo> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(lending_person_info(v.toObject()));
    }
    return result;
  }

  QStringList RestQueries::query_type_field_completions(
      int type_id, QString const& field)
  {
    auto path =
        QStringLiteral("/api/types/") +
        QString::number(type_id) +
        QStringLiteral("/completions/") +
        QUrl::toPercentEncoding(field);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  QStringList RestQueries::query_person_field_completions(
      QString const& field)
  {
    auto path =
        QStringLiteral("/api/persons/completions/") +
        QUrl::toPercentEncoding(field);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  QStringList RestQueries::query_item_field_completions(
      QString const& field)
  {
    auto path =
        QStringLiteral("/api/items/completions/") +
        QUrl::toPercentEncoding(field);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  QStringList RestQueries::query_school_item_field_completions(
      QString const& field)
  {
    auto path =
        QStringLiteral("/api/items/school-completions/") +
        QUrl::toPercentEncoding(field);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  std::vector<HistoryCombinedModel::Row>
  RestQueries::query_combined_history(bool is_admin,
                                      QDate const& from_date,
                                      QDate const& to_date,
                                      bool show_all,
                                      int limit,
                                      int offset,
                                      QString const& action_filter)
  {
    auto path =
        QStringLiteral("/api/history?is_admin=") +
        (is_admin ? QStringLiteral("true")
                  : QStringLiteral("false")) +
        QStringLiteral("&from=") +
        date_str(from_date) +
        QStringLiteral("&to=") +
        date_str(to_date) +
        QStringLiteral("&show_all=") +
        (show_all ? QStringLiteral("true") : QStringLiteral("false")) +
        QStringLiteral("&limit=") + QString::number(limit) +
        QStringLiteral("&offset=") + QString::number(offset) +
        QStringLiteral("&action=") +
        QUrl::toPercentEncoding(action_filter);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<HistoryCombinedModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(history_row(v.toObject()));
    }
    return result;
  }

  std::vector<ChangePasswordsPersonListModel::Row>
  RestQueries::query_change_passwords_persons()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral(
                      "/api/persons/change-passwords"));
    if(r.status != 200) {
      return {};
    }
    std::vector<ChangePasswordsPersonListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(change_passwords_row(v.toObject()));
    }
    return result;
  }

  std::vector<HistoryCombinedModel::Row>
  RestQueries::find_conflicting_actions(
      HistoryCombinedModel::Row const& action)
  {
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/history/conflicts"),
                  json_body(history_row_to_json(action)));
    if(r.status != 200) {
      return {};
    }
    std::vector<HistoryCombinedModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(history_row(v.toObject()));
    }
    return result;
  }

  bool RestQueries::batch_delete_history_actions(
      std::vector<HistoryCombinedModel::Row> const& actions)
  {
    QJsonArray arr;
    for(auto const& a : actions) {
      arr.append(history_row_to_json(a));
    }
    auto r = send(QStringLiteral("DELETE"),
                  QStringLiteral("/api/history"),
                  json_body(arr));
    return r.status >= 200 && r.status < 300;
  }

  std::vector<HistoryCombinedModel::Row>
  RestQueries::find_change_conflicts(
      HistoryCombinedModel::Row const& old_action,
      HistoryCombinedModel::Row const& new_action)
  {
    QJsonObject o;
    o[QStringLiteral("old")] =
        history_row_to_json(old_action);
    o[QStringLiteral("new")] =
        history_row_to_json(new_action);
    auto r = send(
        QStringLiteral("POST"),
        QStringLiteral("/api/history/change-conflicts"),
        json_body(o));
    if(r.status != 200) {
      return {};
    }
    std::vector<HistoryCombinedModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(history_row(v.toObject()));
    }
    return result;
  }

  bool RestQueries::apply_action_change(
      HistoryCombinedModel::Row const& old_action,
      HistoryCombinedModel::Row const& new_action)
  {
    QJsonObject o;
    o[QStringLiteral("old")] =
        history_row_to_json(old_action);
    o[QStringLiteral("new")] =
        history_row_to_json(new_action);
    auto r = send(QStringLiteral("PUT"),
                  QStringLiteral("/api/history"),
                  json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  std::vector<DueLendingListModel::Row>
  RestQueries::query_due_lendings(
      QDate const& from_date, QDate const& to_date,
      int person_id,
      QString const& school_class_filter,
      QString const& person_name_filter)
  {
    auto path =
        QStringLiteral("/api/lendings/due?from=") +
        date_str(from_date) +
        QStringLiteral("&to=") +
        date_str(to_date) +
        QStringLiteral("&person_id=") +
        QString::number(person_id) +
        QStringLiteral("&school_class=") +
        QUrl::toPercentEncoding(school_class_filter) +
        QStringLiteral("&person_name=") +
        QUrl::toPercentEncoding(person_name_filter);
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<DueLendingListModel::Row> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(due_lending_row(v.toObject()));
    }
    return result;
  }

  bool RestQueries::extend_lendings(std::vector<int> const& lending_ids,
                                    QDate const& duedate)
  {
    QJsonArray ids;
    for(int const id : lending_ids) {
      ids.append(id);
    }
    QJsonObject o;
    o[QStringLiteral("lending_ids")] = ids;
    o[QStringLiteral("duedate")] = date_str(duedate);
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/lendings/extend"),
                  json_body(o));
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Class management
// ---------------------------------------------------------------------------

  QStringList RestQueries::load_class_names()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/classes"));
    if(r.status != 200) {
      return {};
    }
    QStringList result;
    for(auto const& v : parse_arr(r.body)) {
      result << v.toString();
    }
    return result;
  }

  bool RestQueries::add_class_name(QString const& name)
  {
    QJsonObject o;
    o[QStringLiteral("name")] = name;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/classes"),
                  json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::remove_class_name(QString const& name)
  {
    auto path =
        QStringLiteral("/api/classes/") +
        QUrl::toPercentEncoding(name);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  bool RestQueries::rename_class_name(
      QString const& old_name, QString const& new_name)
  {
    auto path =
        QStringLiteral("/api/classes/") +
        QUrl::toPercentEncoding(old_name);
    QJsonObject o;
    o[QStringLiteral("new_name")] = new_name;
    auto r = send(QStringLiteral("PUT"), path, json_body(o));
    return r.status >= 200 && r.status < 300;
  }

  std::vector<IQueries::AffectedPerson>
  RestQueries::get_affected_persons(
      QMap<QString, QString> const& old_to_new)
  {
    QJsonObject o;
    for(auto it = old_to_new.cbegin();
        it != old_to_new.cend(); ++it) {
      o[it.key()] = it.value();
    }
    auto r = send(
        QStringLiteral("POST"),
        QStringLiteral("/api/classes/affected-persons"),
        json_body(o));
    if(r.status != 200) {
      return {};
    }
    std::vector<AffectedPerson> result;
    for(auto const& v : parse_arr(r.body)) {
      result.push_back(affected_person(v.toObject()));
    }
    return result;
  }

  bool RestQueries::apply_school_year_changes(
      QMap<int, QString> const& person_id_to_new_class,
      QList<int> const& persons_to_delete,
      QStringList const& new_class_list)
  {
    QJsonObject changes;
    for(auto it = person_id_to_new_class.cbegin();
        it != person_id_to_new_class.cend(); ++it) {
      changes[QString::number(it.key())] = it.value();
    }
    QJsonArray del;
    for(auto id : persons_to_delete) {
      del.append(id);
    }
    QJsonArray cls;
    for(auto const& c : new_class_list) {
      cls.append(c);
    }
    QJsonObject o;
    o[QStringLiteral("person_id_to_new_class")] = changes;
    o[QStringLiteral("persons_to_delete")] = del;
    o[QStringLiteral("new_class_list")] = cls;
    auto r = send(
        QStringLiteral("POST"),
        QStringLiteral("/api/classes/year-changes"),
        json_body(o));
    return r.status >= 200 && r.status < 300;
  }

// ---------------------------------------------------------------------------
// Concurrency / versioned updates
// ---------------------------------------------------------------------------

  int RestQueries::get_version(VersionedEntity entity, int id)
  {
    QString path;
    switch(entity) {
    case VersionedEntity::Item:
      path = QStringLiteral("/api/items/") +
             QString::number(id) +
             QStringLiteral("/version");
      break;
    case VersionedEntity::Person:
      path = QStringLiteral("/api/persons/") +
             QString::number(id) +
             QStringLiteral("/version");
      break;
    case VersionedEntity::Lending:
      path = QStringLiteral("/api/lendings/") +
             QString::number(id) +
             QStringLiteral("/version");
      break;
    case VersionedEntity::Return:
      path = QStringLiteral("/api/returns/") +
             QString::number(id) +
             QStringLiteral("/version");
      break;
    case VersionedEntity::Acquisition:
      path = QStringLiteral("/api/acquisitions/") +
             QString::number(id) +
             QStringLiteral("/version");
      break;
    case VersionedEntity::Liquidation:
      path = QStringLiteral("/api/liquidations/") +
             QString::number(id) +
             QStringLiteral("/version");
      break;
    }
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return -1;
    }
    return int_val(parse_obj(r.body), QStringLiteral("value"));
  }

  IQueries::TableVersions RestQueries::query_table_versions()
  {
    auto r = send(QStringLiteral("GET"),
                  QStringLiteral("/api/table-versions"));
    if(r.status != 200) {
      return {};
    }
    auto const obj = parse_obj(r.body);
    TableVersions result;
    for(auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
      result.insert(it.key(), it.value().toInt());
    }
    return result;
  }

  IQueries::UpdateResult RestQueries::change_item_versioned(
      int id, Item const& item, int expected_version)
  {
    auto path = QStringLiteral("/api/items/") +
                QString::number(id);
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(item)),
                            expected_version);
    return update_result_from_status(r.status);
  }

  IQueries::UpdateResult RestQueries::change_person_versioned(
      int id, Person const& person, int expected_version)
  {
    auto path = QStringLiteral("/api/persons/") +
                QString::number(id);
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(person)),
                            expected_version);
    return update_result_from_status(r.status);
  }

  IQueries::UpdateResult RestQueries::change_lending_versioned(
      int id, Lending const& lending, int expected_version)
  {
    auto path = QStringLiteral("/api/lendings/") +
                QString::number(id);
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(lending)),
                            expected_version);
    return update_result_from_status(r.status);
  }

  IQueries::UpdateResult RestQueries::change_return_versioned(
      int return_id, Return const& ret,
      int expected_version)
  {
    auto path = QStringLiteral("/api/returns/") +
                QString::number(return_id);
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(ret)),
                            expected_version);
    return update_result_from_status(r.status);
  }

  IQueries::UpdateResult
  RestQueries::change_acquisition_versioned(
      int id, Acquisition const& acquisition,
      int expected_version)
  {
    auto path = QStringLiteral("/api/acquisitions/") +
                QString::number(id);
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(acquisition)),
                            expected_version);
    return update_result_from_status(r.status);
  }

  IQueries::UpdateResult
  RestQueries::change_liquidation_versioned(
      int id, Liquidation const& liquidation,
      int expected_version)
  {
    auto path = QStringLiteral("/api/liquidations/") +
                QString::number(id);
    auto r = send_versioned(QStringLiteral("PUT"), path,
                            json_body(to_json(liquidation)),
                            expected_version);
    return update_result_from_status(r.status);
  }

// -----------------------------------------------------------------------
// Reservations
// -----------------------------------------------------------------------

  colman::IQueries::ReservationResult
  colman::RestQueries::add_reservation(int item_id, int person_id,
                                       int count,
                                       QDate const& expires_at,
                                       int /*data_collector_id*/,
                                       int& new_id)
  {
    Reservation res;
    res.item_id = item_id;
    res.person_id = person_id;
    res.count = count;
    res.expires_at = expires_at;
    auto r = send(QStringLiteral("POST"),
                  QStringLiteral("/api/reservations"),
                  json_body(to_json(res)));
    if(r.status >= 200 && r.status < 300) {
      new_id = int_val(parse_obj(r.body), QStringLiteral("id"));
      return ReservationResult::Ok;
    }
    if(r.status == 409) {
      return ReservationResult::AlreadyExists;
    }
    if(r.status == 422) {
      return ReservationResult::NotEnoughAvailable;
    }
    return ReservationResult::Error;
  }

  bool colman::RestQueries::remove_reservation(int reservation_id)
  {
    auto path = QStringLiteral("/api/reservations/") +
                QString::number(reservation_id);
    auto r = send(QStringLiteral("DELETE"), path);
    return r.status >= 200 && r.status < 300;
  }

  std::vector<colman::IQueries::Reservation>
  colman::RestQueries::get_reservations_for_item(int item_id)
  {
    auto path = QStringLiteral("/api/items/") +
                QString::number(item_id) +
                QStringLiteral("/reservations");
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<Reservation> result;
    for(auto const& v : parse_arr(r.body)) {
      Reservation res;
      if(from_json(v.toObject(), res)) {
        result.push_back(res);
      }
    }
    return result;
  }

  bool colman::RestQueries::get_reservation(int item_id,
                                            int person_id,
                                            Reservation& r)
  {
    for(auto const& res : get_reservations_for_item(item_id)) {
      if(res.person_id == person_id) {
        r = res;
        return true;
      }
    }
    return false;
  }

  std::vector<colman::IQueries::Reservation>
  colman::RestQueries::get_reservations(int person_id)
  {
    QString path = QStringLiteral("/api/reservations");
    if(person_id >= 0) {
      path += QStringLiteral("?person_id=") +
              QString::number(person_id);
    }
    auto r = send(QStringLiteral("GET"), path);
    if(r.status != 200) {
      return {};
    }
    std::vector<Reservation> result;
    for(auto const& v : parse_arr(r.body)) {
      Reservation res;
      if(from_json(v.toObject(), res)) {
        result.push_back(res);
      }
    }
    return result;
  }

} // namespace colman

/* EOF */
