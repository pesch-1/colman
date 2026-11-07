// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Multi-source title (name) queries for books, CDs, DVDs, and Blu-rays.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_name_queries.hpp"
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef COLMAN_EAN_QUERIES_HPP
#  include "colman_ean_queries.hpp"
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
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
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  define SYSTEM_QREGULAREXPRESSION
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
#ifndef SYSTEM_QTCONCURRENTRUN
#  include <QtConcurrentRun>
#  define SYSTEM_QTCONCURRENTRUN
#endif

namespace
{
  /// Maximum number of records requested per title-search source.
  int constexpr MaxSearchRecords = 10;

  /// Extracts the four-digit year from a normalised publication date.
  /**
    @param date  A date string as produced by colman::normalize_date().
    @return The year, or 0 when no four-digit year is present.
  */
  int year_from_date(QString const& date)
  {
    QRegularExpression const year_re("(\\d{4})\\s*$");
    QRegularExpressionMatch const m = year_re.match(date);
    if(m.hasMatch()) {
      return m.captured(1).toInt();
    }
    return 0;
  }

  /// Extracts the year of an item's publication_date value.
  int year_of_item(colman::Item const& item)
  {
    auto const it = item.get_values().find("publication_date");
    if(it == item.get_values().end()) {
      return 0;
    }
    return year_from_date(it->second);
  }

  /// Wraps a parsed item into a NameSearchResult.
  /**
    @param item       The parsed item; its name becomes the result name.
    @param source     Source label shown in the result list.
    @param cover_url  Cover image URL, may be empty.
    @return The populated NameSearchResult.
  */
  colman::NameSearchResult wrap_item(colman::Item const& item,
                                     QString const& source,
                                     QString const& cover_url)
  {
    colman::NameSearchResult result;
    result.name_ = item.get_name();
    result.year_ = year_of_item(item);
    result.source_ = source;
    result.cover_url_ = cover_url;
    result.item_ = item;
    return result;
  }

  /// True when an enabled SRU server supports title search for \a kind.
  /**
    UNIMARC servers (BnF) only support book title searches.
    @param servers  The configured SRU servers.
    @param kind     The requested media kind.
  */
  bool sru_title_search_available(QVector<colman::SruServer> const& servers,
                                  colman::MediaKind kind)
  {
    for(auto const& srv : servers) {
      if(srv.enabled && !srv.title_field.isEmpty() &&
         (kind == colman::MediaKind::Book || !srv.is_unimarc())) {
        return true;
      }
    }
    return false;
  }

  /// Queries Open Library by title and returns the parsed results.
  QVector<colman::NameSearchResult> fetch_title_openlibrary(
      QString const& title)
  {
    auto const data = colman::query_title_openlibrary(title);
    auto const results = colman::openlibrary_search_to_results(data);
    qCDebug(colman_lookup_log) << "[name] Open Library:" << results.size()
                               << "hit(s) for" << title;
    return results;
  }

  /// Queries Google Books by title and returns the parsed results.
  QVector<colman::NameSearchResult> fetch_title_googleapi(
      QString const& title, QString const& key)
  {
    auto const data = colman::query_title_googleapi(title, key);
    auto const results = colman::googleapis_search_to_results(data);
    qCDebug(colman_lookup_log) << "[name] Google Books:" << results.size()
                               << "hit(s) for" << title;
    return results;
  }

  /// Queries MusicBrainz by title and returns the parsed results.
  QVector<colman::NameSearchResult> fetch_title_musicbrainz(
      QString const& title)
  {
    auto const data = colman::query_title_musicbrainz(title);
    auto const results = colman::musicbrainz_search_to_results(data);
    qCDebug(colman_lookup_log) << "[name] MusicBrainz:" << results.size()
                               << "hit(s) for" << title;
    return results;
  }

  /// Queries one SRU server by title and returns the parsed results.
  QVector<colman::NameSearchResult> fetch_title_sru(
      colman::SruServer const& srv,
      QString const& title,
      colman::MediaKind kind)
  {
    QVector<colman::NameSearchResult> results;
    auto const data = colman::query_sru_title(srv, title, MaxSearchRecords);
    if(data.isEmpty()) {
      qCDebug(colman_lookup_log) << "[name]" << srv.name
                                 << ": network error / timeout";
      return results;
    }
    for(auto const& item : colman::sru_title_to_items(srv, data, kind)) {
      results.append(wrap_item(item, srv.name, QString()));
    }
    qCDebug(colman_lookup_log) << "[name]" << srv.name << ":" << results.size()
                               << "hit(s) for" << title;
    return results;
  }

} // namespace


/// Returns true when a title-search service is enabled for \a kind.
/**
  Reads the service settings ("use_openlibrary", "use_googleapis",
  "use_musicbrainz") from QSettings and the configured SRU servers from
  load_sru_servers().

  @param kind  The requested media kind.
  @return true when at least one applicable service is enabled.
*/
bool colman::name_search_available(MediaKind kind)
{
  QSettings settings;
  auto const servers = load_sru_servers();
  bool const sru = sru_title_search_available(servers, kind);
  switch(kind) {
  case MediaKind::Book:
    return sru || settings.value("use_openlibrary", false).toBool() ||
           settings.value("use_googleapis", false).toBool();
  case MediaKind::Cd:
    return sru || settings.value("use_musicbrainz", false).toBool();
  case MediaKind::Dvd:
  case MediaKind::BluRay:
    return sru;
  case MediaKind::BoardGame:
  case MediaKind::Unknown:
    break;
  }
  return false;
}


/// Sends a synchronous title search to Open Library and returns raw JSON.
/**
  Performs a synchronous HTTP GET to the Open Library search API and
  returns the raw JSON response body.  Returns an empty QByteArray on
  network error or timeout.

  @param title  The title string to search for.
  @return Raw JSON from Open Library, or an empty QByteArray on failure.
*/
QByteArray colman::query_title_openlibrary(QString const& title)
{
  return synchronous_get(
      QString("https://openlibrary.org/search.json?title=") +
      QUrl::toPercentEncoding(title.trimmed()) +
      "&limit=" + QString::number(MaxSearchRecords) +
      "&fields=title,author_name,first_publish_year,cover_i,publisher");
}


/// Parses an Open Library search JSON response into search results.
/**
  Builds one shallow result per docs[] entry: title, authors, first
  publication year, first publisher, and cover URL.  Deliberately stores
  no ISBN, because the search documents mix the ISBNs of all editions.

  @param json  Raw JSON bytes from the Open Library search response.
  @return One NameSearchResult per valid docs entry, in response order.
*/
QVector<colman::NameSearchResult>
colman::openlibrary_search_to_results(QByteArray const& json)
{
  QVector<NameSearchResult> results;
  if(json.isEmpty()) {
    return results;
  }
  QJsonParseError error;
  QJsonDocument const doc = QJsonDocument::fromJson(json, &error);
  if(!doc.isObject()) {
    return results;
  }
  for(auto const& entry : doc.object()["docs"].toArray()) {
    QJsonObject const book = entry.toObject();
    QString const title = book["title"].toString().trimmed();
    if(title.isEmpty()) {
      continue;
    }
    Item item;
    item.set_name(title);
    QStringList authors;
    for(auto const& author : book["author_name"].toArray()) {
      QString const name = author.toString().trimmed();
      if(!name.isEmpty()) {
        authors += name;
      }
    }
    if(!authors.isEmpty()) {
      item.set_list_value("authors", authors);
    }
    int const year = book["first_publish_year"].toInt();
    if(year > 0) {
      item.set_value("publication_date", QString::number(year));
    }
    QString const publisher =
        book["publisher"].toArray().first().toString().trimmed();
    if(!publisher.isEmpty()) {
      item.set_value("publisher", publisher);
    }
    QString cover;
    int const cover_id = book["cover_i"].toInt();
    if(cover_id > 0) {
      cover = "https://covers.openlibrary.org/b/id/" +
              QString::number(cover_id) + "-M.jpg";
      item.set_value("cover_url", cover);
    }
    results.append(wrap_item(item, "Open Library", cover));
  }
  return results;
}


/// Parses a Google Books search JSON response into search results.
/**
  Builds one result per items[] volume via googleapis_volume_to_item();
  volumes without a title are skipped.

  @param json  Raw JSON bytes from the Google Books search response.
  @return One NameSearchResult per valid volume, in response order.
*/
QVector<colman::NameSearchResult>
colman::googleapis_search_to_results(QByteArray const& json)
{
  QVector<NameSearchResult> results;
  if(json.isEmpty()) {
    return results;
  }
  QJsonParseError error;
  QJsonDocument const doc = QJsonDocument::fromJson(json, &error);
  if(!doc.isObject()) {
    return results;
  }
  for(auto const& entry : doc.object()["items"].toArray()) {
    QJsonObject const volinfo = entry.toObject()["volumeInfo"].toObject();
    QString cover;
    Item item = googleapis_volume_to_item(volinfo, cover);
    if(item.get_name().isEmpty()) {
      continue;
    }
    if(!cover.isEmpty()) {
      item.set_value("cover_url", cover);
    }
    results.append(wrap_item(item, "Google Books", cover));
  }
  return results;
}


/// Parses a MusicBrainz release search JSON response into search results.
/**
  Builds one result per releases[] entry via
  musicbrainz_release_to_item(); releases without a title are skipped.

  @param json  Raw JSON bytes from the MusicBrainz release search.
  @return One NameSearchResult per valid release, in response order.
*/
QVector<colman::NameSearchResult>
colman::musicbrainz_search_to_results(QByteArray const& json)
{
  QVector<NameSearchResult> results;
  if(json.isEmpty()) {
    return results;
  }
  QJsonParseError error;
  QJsonDocument const doc = QJsonDocument::fromJson(json, &error);
  if(!doc.isObject()) {
    return results;
  }
  for(auto const& entry : doc.object()["releases"].toArray()) {
    QString cover;
    Item item = musicbrainz_release_to_item(entry.toObject(), cover);
    if(item.get_name().isEmpty()) {
      continue;
    }
    if(!cover.isEmpty()) {
      item.set_value("cover_url", cover);
    }
    results.append(wrap_item(item, "MusicBrainz", cover));
  }
  return results;
}


/// Enables or disables the Open Library service (books).
/** @param b  true to enable, false to disable (default: true). */
void colman::NameQueries::use_openlibrary(bool b)
{
  use_ol_ = b;
}

/// Enables or disables the Google Books service (books).
/** @param b  true to enable, false to disable (default: true). */
void colman::NameQueries::use_googleapis(bool b)
{
  use_ga_ = b;
}

/// Sets the optional Google Books API key.
/** @param key  Google Books API key, or empty for anonymous access. */
void colman::NameQueries::set_googleapis_key(QString const& key)
{
  googleapis_key_ = key;
}

/// Enables or disables the MusicBrainz service (CDs).
/** @param b  true to enable, false to disable (default: true). */
void colman::NameQueries::use_musicbrainz(bool b)
{
  use_mb_ = b;
}

/// Sets the list of SRU servers to query.
/** @param servers  List of SruServer objects; only enabled servers with a
    title_field are queried. */
void colman::NameQueries::set_sru_servers(QVector<SruServer> const& servers)
{
  sru_servers_ = servers;
}


/**
  Runs parallel title searches and blocks until all have finished.

  Starts one QtConcurrent thread per applicable service: Open Library
  and Google Books for books, MusicBrainz for CDs, and every enabled
  SRU server with a title index (UNIMARC servers only for books).  The
  combined results keep the dedicated services first, followed by the
  SRU servers in configured order.

  @param name  The title string to search for.
  @param kind  The media kind that determines the applicable services.
  @return True if at least one search was started, false if no service
          is enabled for \a kind.
*/
bool colman::NameQueries::query_name(QString const& name, MediaKind kind)
{
  results_.clear();
  thumbnail_url_ = QString();
  QString const title = name.trimmed();
  if(title.isEmpty() || kind == MediaKind::Unknown) {
    return false;
  }
  bool const use_ol = use_ol_ && kind == MediaKind::Book;
  bool const use_ga = use_ga_ && kind == MediaKind::Book;
  bool const use_mb = use_mb_ && kind == MediaKind::Cd;
  QVector<SruServer> active_sru;
  for(auto const& srv : sru_servers_) {
    if(srv.enabled && !srv.title_field.isEmpty() &&
       (kind == MediaKind::Book || !srv.is_unimarc())) {
      active_sru.append(srv);
    }
  }
  qCDebug(colman_lookup_log) << "[name] lookup title=" << title << "kind="
                             << static_cast<int>(kind) << "ol=" << use_ol
                             << "ga=" << use_ga << "mb=" << use_mb
                             << "sru_servers=" << active_sru.size();
  if(!use_ol && !use_ga && !use_mb && active_sru.isEmpty()) {
    return false;
  }
  QFuture<QVector<NameSearchResult>> f_ol, f_ga, f_mb;
  QVector<QFuture<QVector<NameSearchResult>>> f_sru(active_sru.size());
  if(use_ol) {
    f_ol = QtConcurrent::run(fetch_title_openlibrary, title);
  }
  if(use_ga) {
    f_ga = QtConcurrent::run(fetch_title_googleapi, title, googleapis_key_);
  }
  if(use_mb) {
    f_mb = QtConcurrent::run(fetch_title_musicbrainz, title);
  }
  for(int i = 0; i < active_sru.size(); ++i) {
    auto const srv = active_sru[i];
    f_sru[i] = QtConcurrent::run(fetch_title_sru, srv, title, kind);
  }
  if(use_ol) {
    f_ol.waitForFinished();
    results_ += f_ol.result();
  }
  if(use_ga) {
    f_ga.waitForFinished();
    results_ += f_ga.result();
  }
  if(use_mb) {
    f_mb.waitForFinished();
    results_ += f_mb.result();
  }
  for(auto& f : f_sru) {
    f.waitForFinished();
    results_ += f.result();
  }
  return true;
}


/// Returns all search results from the last query_name() call.
/** @return The combined results of all queried services. */
QVector<colman::NameSearchResult>
colman::NameQueries::get_search_results() const
{
  return results_;
}


/// Returns the full item data for result \a index.
/**
  Also stores the result's cover URL for retrieval via
  get_thumbnail_url(); a non-empty cover URL is copied into the item's
  "cover_url" value.

  @param index  Index into the result list of the last query_name() call.
  @return Item on success, std::nullopt if \a index is out of range.
*/
std::optional<colman::Item> colman::NameQueries::get_item(int index)
{
  if(index < 0 || index >= results_.size()) {
    return std::nullopt;
  }
  NameSearchResult const& result = results_[index];
  thumbnail_url_ = result.cover_url_;
  Item item = result.item_;
  if(!result.cover_url_.isEmpty() &&
     item.get_values().count("cover_url") == 0) {
    item.set_value("cover_url", result.cover_url_);
  }
  return item;
}


/// Returns the cover URL of the result of the last get_item() call.
/** @return The cover image URL, or an empty string when none exists. */
QString colman::NameQueries::get_thumbnail_url() const
{
  return thumbnail_url_;
}


/// Downloads the cover image and returns it as a pixmap.
/**
  Retrieves the cover URL via get_thumbnail_url() and performs a
  synchronous HTTP GET.  Returns a null QPixmap when no URL is
  available, the download times out, or a network error occurs.

  @return A QPixmap containing the downloaded image, or a null QPixmap.
*/
QPixmap colman::NameQueries::get_thumbnail() const
{
  QPixmap pm;
  QString const url = get_thumbnail_url();
  if(!url.isEmpty()) {
    QByteArray const data = synchronous_get(url);
    pm.loadFromData(data);
  }
  return pm;
}

/* EOF */
