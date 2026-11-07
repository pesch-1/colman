// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// EAN product barcode queries for CDs, DVDs and Blu-rays.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_ean_queries.hpp"
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_QJSONDOCUMENT
#  include <QJsonDocument>
#  define SYSTEM_QJSONDOCUMENT
#endif
#ifndef SYSTEM_QJSONOBJECT
#  include <QJsonObject>
#  define SYSTEM_QJSONOBJECT
#endif
#ifndef SYSTEM_QJSONARRAY
#  include <QJsonArray>
#  define SYSTEM_QJSONARRAY
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
  /// Queries MusicBrainz and returns the parsed item.
  /**
    Combines query_ean_musicbrainz() and musicbrainz_to_item().  Stores
    the cover image URL in the "cover_url" item value.

    @param ean  The EAN-13 to query.
    @return A colman::Item with the music data and "cover_url" value.
  */
  colman::Item fetch_musicbrainz(QString const& ean)
  {
    QString cover;
    QByteArray const data = colman::query_ean_musicbrainz(ean);
    colman::Item item = colman::musicbrainz_to_item(data, cover);
    if(!cover.isEmpty()) {
      item.set_value("cover_url", cover);
    }
    return item;
  }

} // namespace

/// Returns true if \a ean is a valid product EAN-13 (not an ISBN).
/**
  Uses the same EAN-13 checksum algorithm as is_isbn13().  Returns false
  when \a ean starts with "978" or "979" (those are ISBN prefixes).

  @param ean  The 13-digit string to check.
  @return True for a valid non-ISBN EAN-13; false otherwise.
*/
bool colman::is_product_ean(QString const& ean)
{
  return colman::is_isbn13(ean) && !ean.startsWith("978") &&
         !ean.startsWith("979");
}

/// Returns true if \a upc is a syntactically valid 12-digit UPC-A.
/**
  Prepends "0" to \a upc and checks the result via is_product_ean().
  A valid UPC-A is guaranteed to form a valid EAN-13 when padded.

  @param upc  The 12-digit string to check.
  @return True for a valid UPC-A; false otherwise.
*/
bool colman::is_upc_a(QString const& upc)
{
  if(upc.size() != 12) {
    return false;
  }
  for(QChar const c : upc) {
    if(!c.isDigit()) {
      return false;
    }
  }
  return is_product_ean(QString("0") + upc);
}

/// Enables or disables the MusicBrainz service.
/** @param b  true to enable, false to disable (default: true). */
void colman::EanQueries::use_musicbrainz(bool b)
{
  use_mb_ = b;
}

/// Sets the list of SRU servers to query.
/** @param servers  List of SruServer objects; only enabled ones are queried. */
void colman::EanQueries::set_sru_servers(QVector<SruServer> const& servers)
{
  sru_servers_ = servers;
}

/**
  Runs parallel EAN lookups and blocks until all have finished.

  Starts one QtConcurrent thread per enabled service (MusicBrainz and
  each enabled SRU server that supports EAN lookup), waits for them all,
  then merges the results into result_.  Each individual query enforces its
  own network timeout (from QSettings "timeout").

  @param ean  The EAN-13 to look up.
  @return True if at least one lookup was started, false if no service
          is enabled.
*/
bool colman::EanQueries::query_ean(QString const& ean)
{
  ean_ = ean;
  cover_url_ = QString();
  result_ = Item();
  bool const use_mb = use_mb_;
  QVector<SruServer> active_sru;
  for(auto const& srv : sru_servers_) {
    if(srv.enabled && !srv.ean_field.isEmpty()) {
      active_sru.append(srv);
    }
    else {
      qCDebug(colman_lookup_log) << "[SRU ean] skipping" << srv.name
                                 << "enabled=" << srv.enabled << "ean_field="
                                 << srv.ean_field;
    }
  }
  qCDebug(colman_lookup_log) << "[SRU ean] lookup ean=" << ean << "musicbrainz="
                             << use_mb << "sru_servers=" << active_sru.size();
  if(!use_mb && active_sru.isEmpty()) {
    return false;
  }
  QFuture<Item> f_mb;
  QVector<QFuture<Item>> f_sru(active_sru.size());
  if(use_mb) {
    f_mb = QtConcurrent::run(fetch_musicbrainz, ean);
  }
  for(int i = 0; i < active_sru.size(); ++i) {
    auto const srv = active_sru[i];
    f_sru[i] = QtConcurrent::run([srv, ean]() -> Item {
      qCDebug(colman_lookup_log) << "[SRU ean] querying" << srv.name << "for"
                                 << ean;
      auto const data = colman::query_sru(srv, srv.ean_field, ean);
      if(data.isEmpty()) {
        qCDebug(colman_lookup_log) << "[SRU ean]" << srv.name
                                   << ": network error / timeout";
        return Item();
      }
      auto const item = colman::sru_to_av_item(srv, data);
      if(item.get_name().isEmpty()) {
        qCDebug(colman_lookup_log) << "[SRU ean]" << srv.name << ": no hit";
      }
      else {
        qCDebug(colman_lookup_log) << "[SRU ean]" << srv.name << ": hit -"
                                   << item.get_name();
      }
      return item;
    });
  }
  if(use_mb) {
    f_mb.waitForFinished();
  }
  for(auto& f : f_sru) {
    f.waitForFinished();
  }
  if(use_mb) {
    result_ += f_mb.result();
  }
  for(auto const& f : f_sru) {
    result_ += f.result();
  }
  // Pick the most specific publication date across all sources.
  {
    QString best;
    auto pick = [&best](QFuture<Item> const& f) {
      if(!f.isValid()) {
        return;
      }
      Item const item = f.result();
      auto const it = item.get_values().find("publication_date");
      if(it != item.get_values().end() && it->second.size() > best.size()) {
        best = it->second;
      }
    };
    if(use_mb) {
      pick(f_mb);
    }
    for(auto const& f : f_sru) {
      pick(f);
    }
    if(!best.isEmpty()) {
      result_.set_value("publication_date", best);
    }
  }
  auto const it = result_.get_values().find("cover_url");
  if(it != result_.get_values().end()) {
    cover_url_ = it->second;
  }
  return true;
}

/// Returns the merged item data populated by the last query_ean() call.
/**
  @return A colman::Item containing the combined AV metadata from all
          services that returned a successful response.
*/
colman::Item colman::EanQueries::get_item_data() const
{
  return result_;
}

/// Returns the URL of the best available cover image.
/**
  Returns an empty string when no cover URL is available.

  @return The cover image URL as a string.
*/
QString colman::EanQueries::get_thumbnail_url() const
{
  return cover_url_;
}

/// Downloads the product thumbnail and returns it as a pixmap.
/**
  Retrieves the thumbnail URL via get_thumbnail_url() and performs a
  synchronous HTTP GET.  Returns a null QPixmap when no URL is available,
  the download times out, or a network error occurs.

  @return A QPixmap containing the downloaded image, or a null QPixmap.
*/
QPixmap colman::EanQueries::get_thumbnail() const
{
  QPixmap pm;
  QString const url = get_thumbnail_url();
  if(!url.isEmpty()) {
    QByteArray const data = colman::synchronous_get(url);
    pm.loadFromData(data);
  }
  return pm;
}

/// Sends a synchronous EAN lookup to MusicBrainz and returns the raw response.
/**
  Performs a synchronous HTTP GET to the MusicBrainz release search API
  using the barcode field.  Sets the required User-Agent header.
  Returns an empty QByteArray on network error or timeout.

  @param ean  The EAN-13 to query.
  @return Raw JSON from MusicBrainz, or an empty QByteArray on failure.
*/
QByteArray colman::query_ean_musicbrainz(QString const& ean)
{
  return colman::synchronous_get(
      "https://musicbrainz.org/ws/2/release"
      "?query=barcode%3A" +
          ean + "&fmt=json&limit=5",
      "Colman/1.0 (school library management software)");
}

/// Sends a synchronous title search to MusicBrainz and returns the raw
/// response.
/**
  Performs a synchronous HTTP GET to the MusicBrainz release search API
  using the release (title) field.  Sets the required User-Agent header.
  Returns an empty QByteArray on network error or timeout.

  @param title  The release title to search for.
  @return Raw JSON from MusicBrainz, or an empty QByteArray on failure.
*/
QByteArray colman::query_title_musicbrainz(QString const& title)
{
  return colman::synchronous_get(
      "https://musicbrainz.org/ws/2/release"
      "?query=release%3A%22" +
          QUrl::toPercentEncoding(title.trimmed()) + "%22&fmt=json&limit=10",
      "Colman/1.0 (school library management software)");
}

/// Parses a MusicBrainz release search JSON response and returns item data.
/**
  Extracts title, artist credits, label name and release date from the
  first release in the MusicBrainz response.  Always constructs a Cover
  Art Archive URL from the release MBID; the barcode search API does not
  populate the cover-art-archive field reliably, so availability is only
  determined when the download is attempted.

  @param json       Raw JSON bytes from the MusicBrainz release search.
  @param cover_url  Output parameter filled with the Cover Art Archive URL,
                    or empty if no MBID was found.
  @return A colman::Item populated with the music data.
*/
colman::Item colman::musicbrainz_to_item(QByteArray const& json,
                                         QString& cover_url)
{
  colman::Item item;
  if(json.isEmpty()) {
    return item;
  }
  QJsonParseError error;
  QJsonDocument const doc = QJsonDocument::fromJson(json, &error);
  if(!doc.isObject()) {
    return item;
  }
  QJsonArray const releases = doc.object()["releases"].toArray();
  if(releases.isEmpty()) {
    return item;
  }
  return musicbrainz_release_to_item(releases[0].toObject(), cover_url);
}

/// Parses one MusicBrainz release object into an Item.
/**
  Extracts title, artist credits, label name, catalogue number, and
  release date from a single release object of a MusicBrainz release
  search response.  Always constructs a Cover Art Archive URL from the
  release MBID; availability is only determined when the download is
  attempted.

  @param release    One release JSON object.
  @param cover_url  Output parameter filled with the Cover Art Archive URL,
                    or empty if no MBID was found.
  @return A colman::Item populated with the music data.
*/
colman::Item colman::musicbrainz_release_to_item(QJsonObject const& release,
                                                 QString& cover_url)
{
  colman::Item item;
  QJsonObject const& rel = release;
  QString const title = rel["title"].toString().trimmed();
  if(!title.isEmpty()) {
    item.set_name(title);
  }
  // Build artist string by concatenating name + joinphrase for each credit.
  QString artist;
  for(auto const& credit : rel["artist-credit"].toArray()) {
    QJsonObject const c = credit.toObject();
    QString name = c["name"].toString().trimmed();
    if(name.isEmpty()) {
      name = c["artist"].toObject()["name"].toString().trimmed();
    }
    QString const join = c["joinphrase"].toString();
    if(!name.isEmpty()) {
      artist += name;
      artist += join;
    }
  }
  if(!artist.isEmpty()) {
    item.set_list_value("artist", {artist});
  }
  // Label name from the first label-info entry.
  QJsonArray const linfo = rel["label-info"].toArray();
  if(!linfo.isEmpty()) {
    QString const label =
        linfo[0].toObject()["label"].toObject()["name"].toString().trimmed();
    if(!label.isEmpty()) {
      item.set_value("publisher", label);
    }
    QString const catno =
        linfo[0].toObject()["catalog-number"].toString().trimmed();
    if(!catno.isEmpty()) {
      item.set_value("publisher_number", catno);
    }
  }
  QString const date = colman::normalize_date(rel["date"].toString());
  if(!date.isEmpty()) {
    item.set_value("publication_date", date);
  }
  // The barcode search API does not populate cover-art-archive, so always
  // attempt a CAA download; a 404 is handled gracefully in get_thumbnail().
  QString const mbid = rel["id"].toString();
  if(!mbid.isEmpty()) {
    cover_url = "https://coverartarchive.org/release/" + mbid + "/front-250";
  }
  return item;
}

/* EOF */
