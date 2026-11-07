// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// BoardGameGeek and opengtindb.org queries for board games.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_bgg_queries.hpp"
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
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
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
#ifndef SYSTEM_QDOMELEMENT
#  include <QDomElement>
#  define SYSTEM_QDOMELEMENT
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  define SYSTEM_QREGULAREXPRESSION
#endif

namespace
{
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


  /// Strips trailing MARC-like punctuation from \a s.
  QString strip_punct(QString s)
  {
    while(!s.isEmpty()) {
      QChar const c = s.back();
      if(c == ' ' || c == '.' || c == ',' || c == '/' || c == ':') {
        s.chop(1);
      }
      else {
        break;
      }
    }
    return s;
  }

  /// Decodes HTML entities and removes HTML tags from \a html.
  QString strip_html(QString html)
  {
    // Replace common entities before stripping tags.
    html.replace("&amp;", "&");
    html.replace("&lt;", "<");
    html.replace("&gt;", ">");
    html.replace("&quot;", "\"");
    html.replace("&#10;", "\n");
    html.replace("&mdash;", "—");
    html.replace("&ndash;", "–");
    html.replace("&rsquo;", "’");
    html.replace("&lsquo;", "‘");
    html.replace("&rdquo;", "”");
    html.replace("&ldquo;", "“");
    QRegularExpression const tag("<[^>]*>");
    html.remove(tag);
    return html.trimmed();
  }

} // namespace


/// Sends a synchronous text search to BoardGameGeek and returns the raw XML.
/**
  @param name  Search query string.
  @return Raw XML from the BGG search API, or an empty QByteArray on failure.
*/
QByteArray colman::query_bgg_search(QString const& name,
                                    QString const& token)
{
  // No "www." — BGG redirects to the apex domain; querying it directly
  // saves the redirect round trip.
  QUrl url("https://boardgamegeek.com/xmlapi2/search");
  QUrlQuery query;
  query.addQueryItem("query", name);
  query.addQueryItem("type", "boardgame");
  url.setQuery(query);
  return synchronous_get(url.toString(),
                         "Colman/1.0 (school library management)",
                         token);
}

/// Parses a BGG search XML response and returns the list of results.
/**
  Extracts the BGG ID, primary name, and year from each <item> element.

  @param xml  Raw XML from query_bgg_search().
  @return Parsed search results, sorted alphabetically by name.
*/
QVector<colman::BggSearchResult>
colman::bgg_search_to_results(QByteArray const& xml)
{
  QVector<BggSearchResult> results;
  if(xml.isEmpty()) {
    return results;
  }
  QDomDocument doc;
  if(!doc.setContent(xml)) {
    return results;
  }
  QDomElement const root = doc.documentElement();
  QDomElement item = root.firstChildElement("item");
  while(!item.isNull()) {
    BggSearchResult r;
    bool ok = false;
    r.id_ = item.attribute("id").toInt(&ok);
    if(!ok || r.id_ <= 0) {
      item = item.nextSiblingElement("item");
      continue;
    }
    QDomElement name_el = item.firstChildElement("name");
    while(!name_el.isNull()) {
      if(name_el.attribute("type") == "primary") {
        r.name_ = name_el.attribute("value").trimmed();
        break;
      }
      name_el = name_el.nextSiblingElement("name");
    }
    if(r.name_.isEmpty()) {
      item = item.nextSiblingElement("item");
      continue;
    }
    QDomElement const year_el = item.firstChildElement("yearpublished");
    if(!year_el.isNull()) {
      r.year_ = year_el.attribute("value").toInt();
    }
    results.append(r);
    item = item.nextSiblingElement("item");
  }
  return results;
}

/// Sends a synchronous thing-detail request to BGG and returns the raw XML.
/**
  BGG's thing endpoint may return a 202-style message body while data is
  being prepared.  The function retries up to three times with a 2-second
  delay between attempts when it detects such a response.
  @param bgg_id  BGG game ID to fetch.
  @return Raw XML from the BGG thing API, or an empty QByteArray on failure.
*/
QByteArray colman::query_bgg_thing(int bgg_id, QString const& token)
{
  // No "www." — see query_bgg_search().
  QString const url =
      QString("https://boardgamegeek.com/xmlapi2/thing?id=%1").arg(bgg_id);
  QString const ua = "Colman/1.0 (school library management)";
  for(int attempt = 0; attempt < 3; ++attempt) {
    QByteArray const result = synchronous_get(url, ua, token);
    if(!result.trimmed().startsWith("<message>")) {
      return result;
    }
    QEventLoop wait;
    QTimer delay;
    delay.setSingleShot(true);
    wait.connect(&delay, &QTimer::timeout, &wait, &QEventLoop::quit);
    delay.start(2000);
    wait.exec();
  }
  return QByteArray();
}

/// Parses a BGG thing XML response into an Item.
/**
  Extracts: primary name, year, min/max players, min age, playing time,
  publisher, designers, artists, categories, and description.

  @param xml  Raw XML from query_bgg_thing().
  @return Populated Item, or a default Item on parse error.
*/
colman::Item colman::bgg_thing_to_item(QByteArray const& xml)
{
  Item item;
  if(xml.isEmpty()) {
    return item;
  }
  QDomDocument doc;
  if(!doc.setContent(xml)) {
    return item;
  }
  QDomElement const root = doc.documentElement();
  QDomElement const thing = root.firstChildElement("item");
  if(thing.isNull()) {
    return item;
  }

  // Title: prefer German alternate name, fall back to primary.
  QString primary_name;
  QString german_name;
  QDomElement name_el = thing.firstChildElement("name");
  while(!name_el.isNull()) {
    QString const val = name_el.attribute("value").trimmed();
    if(name_el.attribute("type") == "primary") {
      primary_name = val;
    }
    else if(name_el.attribute("type") == "alternate" && !val.isEmpty()) {
      // Heuristic: alternate names with German characters preferred.
      if(german_name.isEmpty()) {
        for(QChar const c : val) {
          if(c == QChar(0xE4) || c == QChar(0xF6) || c == QChar(0xFC) ||
             c == QChar(0xC4) || c == QChar(0xD6) || c == QChar(0xDC) ||
             c == QChar(0xDF))
          {
            german_name = val;
            break;
          }
        }
      }
    }
    name_el = name_el.nextSiblingElement("name");
  }
  QString const title = german_name.isEmpty() ? primary_name : german_name;
  if(!title.isEmpty()) {
    item.set_name(title);
  }

  auto int_attr = [&thing](QString const& tag) -> QString {
    QDomElement const el = thing.firstChildElement(tag);
    if(el.isNull()) {
      return QString();
    }
    QString const val = el.attribute("value");
    return val.isEmpty() ? QString() : val;
  };

  QString const year = int_attr("yearpublished");
  if(!year.isEmpty()) {
    item.set_value("publication_date", year);
  }
  QString const min_p = int_attr("minplayers");
  if(!min_p.isEmpty()) {
    item.set_value("min_players", min_p);
  }
  QString const max_p = int_attr("maxplayers");
  if(!max_p.isEmpty()) {
    item.set_value("max_players", max_p);
  }
  QString const min_age = int_attr("minage");
  if(!min_age.isEmpty()) {
    item.set_value("min_age", min_age);
  }
  QString const play_time = int_attr("playingtime");
  if(!play_time.isEmpty()) {
    item.set_value("playing_time", play_time);
  }

  // description
  QDomElement const desc_el = thing.firstChildElement("description");
  if(!desc_el.isNull()) {
    QString const desc = strip_html(desc_el.text());
    if(!desc.isEmpty()) {
      item.set_description(desc);
    }
  }

  // links: publishers, designers, artists, categories
  QStringList designers;
  QStringList artists;
  QStringList genres;
  QString publisher;

  QDomElement link = thing.firstChildElement("link");
  while(!link.isNull()) {
    QString const type = link.attribute("type");
    QString const val = strip_punct(link.attribute("value").trimmed());
    if(!val.isEmpty()) {
      if(type == "boardgamepublisher") {
        if(publisher.isEmpty()) {
          publisher = val;
        }
      }
      else if(type == "boardgamedesigner") {
        designers.append(val);
      }
      else if(type == "boardgameartist") {
        artists.append(val);
      }
      else if(type == "boardgamecategory") {
        genres.append(val);
      }
    }
    link = link.nextSiblingElement("link");
  }

  if(!publisher.isEmpty()) {
    item.set_value("publisher", publisher);
  }
  if(!designers.isEmpty()) {
    item.set_list_value("designer", designers);
  }
  if(!artists.isEmpty()) {
    item.set_list_value("artist", artists);
  }
  if(!genres.isEmpty()) {
    item.set_list_value("genre", genres);
  }

  return item;
}


/// Sets the Bearer token for BGG XML API authentication.
/** @param token  Token string; empty means unauthenticated. */
void colman::BggQueries::set_bgg_token(QString const& token)
{
  bgg_token_ = token;
}

/// Returns the BGG search results from the last query_name() call.
QVector<colman::BggSearchResult> colman::BggQueries::get_search_results() const
{
  return results_;
}

/**
  Searches BGG for board games matching \a name.  Results are stored and
  accessible via get_search_results().

  @param name  Search query string.
  @return true if at least one result was returned.
*/
bool colman::BggQueries::query_name(QString const& name)
{
  results_.clear();
  if(name.trimmed().isEmpty()) {
    return false;
  }
  QByteArray const xml = query_bgg_search(name.trimmed(), bgg_token_);
  results_ = bgg_search_to_results(xml);
  return !results_.isEmpty();
}

/**
  Fetches the full game record for \a bgg_id from BGG.

  @param bgg_id  BGG game ID.
  @return Item on success, std::nullopt if retrieval or parsing failed.
*/
std::optional<colman::Item> colman::BggQueries::get_item(int bgg_id)
{
  QByteArray const xml = query_bgg_thing(bgg_id, bgg_token_);
  auto item = bgg_thing_to_item(xml);
  if(item.get_name().isEmpty()) {
    return std::nullopt;
  }
  return item;
}

/* EOF */
