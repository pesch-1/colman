// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// opengtindb.org EAN look-up helper.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_opengtindb.hpp"
#ifndef SYSTEM_QNETWORKACCESSMANAGER
#  include <QNetworkAccessManager>
#  define SYSTEM_QNETWORKACCESSMANAGER
#endif
#ifndef SYSTEM_QNETWORKREPLY
#  include <QNetworkReply>
#  define SYSTEM_QNETWORKREPLY
#endif
#ifndef SYSTEM_QNETWORKREQUEST
#  include <QNetworkRequest>
#  define SYSTEM_QNETWORKREQUEST
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
#ifndef SYSTEM_QURLQUERY
#  include <QUrlQuery>
#  define SYSTEM_QURLQUERY
#endif
#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QAUTHENTICATOR
#  include <QAuthenticator>
#  define SYSTEM_QAUTHENTICATOR
#endif
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  define SYSTEM_QREGULAREXPRESSION
#endif

namespace
{
  /// Decodes a Windows-1252 (CP1252) byte array into a QString.
  /**
    CP1252 maps bytes 0x80–0x9F to printable Unicode characters;
    ISO-8859-1 maps that range to C1 control characters (displayed as
    boxes by Qt).  Undefined CP1252 slots (0x81, 0x8D, 0x8F, 0x90, 0x9D)
    fall back to their Latin-1 code points.
    @param data  Raw bytes in Windows-1252 encoding.
    @return Decoded QString.
  */
  QString from_windows1252(QByteArray const& data)
  {
    static QChar const Cp1252Map[32] = {
      QChar(0x20AC), QChar(0x0081), QChar(0x201A), QChar(0x0192), // 80-83
      QChar(0x201E), QChar(0x2026), QChar(0x2020), QChar(0x2021), // 84-87
      QChar(0x02C6), QChar(0x2030), QChar(0x0160), QChar(0x2039), // 88-8B
      QChar(0x0152), QChar(0x008D), QChar(0x017D), QChar(0x008F), // 8C-8F
      QChar(0x0090), QChar(0x2018), QChar(0x2019), QChar(0x201C), // 90-93
      QChar(0x201D), QChar(0x2022), QChar(0x2013), QChar(0x2014), // 94-97
      QChar(0x02DC), QChar(0x2122), QChar(0x0161), QChar(0x203A), // 98-9B
      QChar(0x0153), QChar(0x009D), QChar(0x017E), QChar(0x0178), // 9C-9F
    };
    QString result;
    result.reserve(data.size());
    for(auto const byte : data) {
      auto const c = static_cast<unsigned char>(byte);
      if(c >= 0x80 && c <= 0x9F) {
        result += Cp1252Map[c - 0x80];
      }
      else {
        result += QChar(c);
      }
    }
    return result;
  }

  /// Performs a synchronous HTTP GET and returns the response body.
  /**
    Returns an empty QByteArray on network error or timeout.
    @param urlstr       URL to retrieve.
    @param user_agent   Optional User-Agent header value.
    @param bearer_token Optional Bearer token for Authorization header.
    @param min_timeout  Lower bound for the timeout in ms; the effective
                        timeout is max(QSettings "timeout", min_timeout).
    @return Response body, or an empty QByteArray on failure.
  */
  QByteArray synchronous_get(QString const& urlstr,
                             QString const& user_agent = QString(),
                             QString const& bearer_token = QString(),
                             int min_timeout = 0)
  {
    QNetworkAccessManager http;
    QEventLoop loop;
    QSettings settings;
    int const timeout = qMax(settings.value("timeout", 5000).toInt(),
                             min_timeout);
    QString const proxy_user = settings.value("proxy_user", "").toString();
    QString const proxy_pass = settings.value("proxy_password", "").toString();
    http.connect(
        &http, &QNetworkAccessManager::proxyAuthenticationRequired,
        [&proxy_user, &proxy_pass](QNetworkProxy const&,
                                   QAuthenticator* auth) {
          auth->setUser(proxy_user);
          auth->setPassword(proxy_pass);
        });
    QTimer timer;
    timer.setSingleShot(true);
    loop.connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QUrl const url(urlstr);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    if(!user_agent.isEmpty()) {
      request.setHeader(QNetworkRequest::UserAgentHeader, user_agent);
    }
    if(!bearer_token.isEmpty()) {
      request.setRawHeader("Authorization",
                           ("Bearer " + bearer_token).toUtf8());
    }
    QNetworkReply* const reply = http.get(request);
    if(!(reply)) {
      return QByteArray();
    }
    loop.connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeout);
    loop.exec();
    QByteArray result;
    if(reply->isFinished() && reply->error() == QNetworkReply::NoError) {
      result = reply->readAll();
    }
    reply->deleteLater();
    return result;
  }

} // namespace


/// Sends a synchronous EAN lookup to opengtindb.org and returns the response.
/**
  @param ean     EAN-13 to look up.
  @param userid  User ID for opengtindb.org (queryid parameter).
  @return Response body (text/plain key=value), or empty on network error.
*/
QByteArray colman::query_opengtindb(QString const& ean, QString const& userid)
{
  QUrl url("https://opengtindb.org/");
  QUrlQuery query;
  query.addQueryItem("ean", ean);
  query.addQueryItem("cmd", "query");
  query.addQueryItem("queryid", userid);
  url.setQuery(query);
  return synchronous_get(
      url.toString(),
      "Colman/1.0 (school library management; contact: colman@example.org)",
      QString(),
      15000);
}

/// Parses an opengtindb.org text/plain response into an Item.
/**
  The API returns key=value pairs (one per line, Windows-1252 encoded).
  Populates name, description, and publisher.  Returns a default Item
  if the response is empty or contains a non-zero error code.

  @param data  Raw response body from query_opengtindb().
  @return Populated Item, or a default Item on error.
*/
colman::Item colman::opengtindb_to_item(QByteArray const& data)
{
  Item item;
  if(data.isEmpty()) {
    return item;
  }
  QString const text = from_windows1252(data);
  QHash<QString, QString> fields;
  for(auto const& line : text.split('\n')) {
    int const eq = line.indexOf('=');
    if(eq > 0) {
      fields[line.left(eq).trimmed()] = line.mid(eq + 1).trimmed();
    }
  }
  QString const err = fields.value("error");
  if(!err.isEmpty() && err != "0") {
    return item;
  }
  // Name: prefer detailname, fall back to name.
  QString name = fields.value("detailname");
  if(name.isEmpty()) {
    name = fields.value("name");
  }
  // Strip trailing parenthetical manufacturer codes, e.g.
  // "Die Siedler von Catan (KOSMOS 6930150)" → "Die Siedler von Catan".
  static QRegularExpression const TrailingParens(R"(\s*\([^)]*\)\s*$)");
  name.remove(TrailingParens);
  if(!name.isEmpty()) {
    item.set_name(name);
  }
  // Description: prefer German, fall back to English.
  QString descr = fields.value("descr");
  if(descr.isEmpty()) {
    descr = fields.value("descr_en");
  }
  if(!descr.isEmpty()) {
    item.set_description(descr);
  }
  // Publisher.
  QString const vendor = fields.value("vendor");
  if(!vendor.isEmpty()) {
    item.set_value("publisher", vendor);
  }
  // Store the product category (opengtindb "name" field) so callers can
  // check whether the item fits the collection.  Not shown in the UI.
  QString const category = fields.value("name");
  if(!category.isEmpty()) {
    item.set_value("opengtindb_category", category);
  }
  return item;
}

/// Looks up \a ean on opengtindb.org and returns an Item if a name was found.
/**
  @param userid  opengtindb.org user ID (queryid).
  @param ean     EAN-13 to resolve.
  @return Item with at least a name set, or std::nullopt on failure.
*/
std::optional<colman::Item>
colman::query_opengtindb_ean(QString const& userid, QString const& ean)
{
  if(userid.isEmpty() || ean.isEmpty()) {
    return std::nullopt;
  }
  Item const item = opengtindb_to_item(query_opengtindb(ean, userid));
  if(item.get_name().isEmpty()) {
    return std::nullopt;
  }
  return item;
}

/* EOF */
