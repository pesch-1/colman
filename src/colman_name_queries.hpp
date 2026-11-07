// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Multi-source title (name) queries for books, CDs, DVDs, and Blu-rays.
/**
   @file
   @author Christoph Pesch

   Provides NameQueries::query_name() for online title searches and
   NameQueries::get_item() to fetch the full record of a selected search
   result.  Depending on the media kind, Open Library, Google Books,
   MusicBrainz, and the configured SRU servers are queried in parallel.
*/

#ifndef COLMAN_NAME_QUERIES_HPP
#  define COLMAN_NAME_QUERIES_HPP

#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_SRU_SERVER_HPP
#    include "colman_sru_server.hpp"
#  endif
#  ifndef SYSTEM_OPTIONAL
#    include <optional>
#    define SYSTEM_OPTIONAL
#  endif
#  ifndef SYSTEM_QPIXMAP
#    include <QPixmap>
#    define SYSTEM_QPIXMAP
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QVECTOR
#    include <QVector>
#    define SYSTEM_QVECTOR
#  endif

namespace colman
{
  /// One entry from a multi-source title search.
  struct NameSearchResult
  {
    QString name_;      ///< title shown in the result list
    int year_ = 0;      ///< publication year (0 if unknown)
    QString source_;    ///< source label, e.g. "Open Library", "DNB"
    QString cover_url_; ///< cover image URL, may be empty
    Item item_;         ///< full item data
  };

  /// Returns true when a title-search service is enabled for \a kind.
  /** Reads the service settings from QSettings and the configured SRU
      servers from load_sru_servers(). */
  bool name_search_available(MediaKind kind);

  /// Sends a synchronous title search to Open Library.
  QByteArray query_title_openlibrary(QString const& title);
  /// Parses an Open Library search JSON response into search results.
  QVector<NameSearchResult> openlibrary_search_to_results(
      QByteArray const& json);
  /// Parses a Google Books search JSON response into search results.
  QVector<NameSearchResult> googleapis_search_to_results(
      QByteArray const& json);
  /// Parses a MusicBrainz release search JSON response into search results.
  QVector<NameSearchResult> musicbrainz_search_to_results(
      QByteArray const& json);

  /// Blocking multi-source title look-up helper.
  /**
     query_name() runs parallel title searches on all services applicable
     to the requested media kind and stores the combined results for
     retrieval via get_search_results().  Call get_item() afterwards to
     obtain the full record of a specific result.
  */
  class NameQueries
  {
  public:
    /// Constructs the title query helper.
    NameQueries() = default;
    /// Destroys the helper.
    ~NameQueries() = default;

    /// Enables or disables the Open Library service (books).
    void use_openlibrary(bool b = true);
    /// Enables or disables the Google Books service (books).
    void use_googleapis(bool b = true);
    /// Sets the optional Google Books API key.
    void set_googleapis_key(QString const& key);
    /// Enables or disables the MusicBrainz service (CDs).
    void use_musicbrainz(bool b = true);
    /// Sets the list of SRU servers to query.
    void set_sru_servers(QVector<SruServer> const& servers);

    /// Runs parallel title searches on all services applicable to \a kind.
    /** Blocks until all queries have finished.
        @return false when no applicable service is enabled. */
    bool query_name(QString const& name, MediaKind kind);

    /// Returns all search results from the last query_name() call.
    QVector<NameSearchResult> get_search_results() const;

    /// Returns the full item data for result \a index.
    /** @return Item on success, std::nullopt if \a index is out of
                range. */
    std::optional<Item> get_item(int index);

    /// Returns the cover URL of the result of the last get_item() call.
    QString get_thumbnail_url() const;
    /// Downloads and returns the cover pixmap (blocking).
    QPixmap get_thumbnail() const;

  private:
    bool use_ol_ = false;               ///< query Open Library
    bool use_ga_ = false;               ///< query Google Books
    bool use_mb_ = false;               ///< query MusicBrainz
    QString googleapis_key_;            ///< optional Google Books API key
    QVector<SruServer> sru_servers_;    ///< SRU servers to query
    QVector<NameSearchResult> results_; ///< results of last query_name()
    QString thumbnail_url_;             ///< cover URL of last get_item()
  };

} // namespace colman

#endif /* #ifndef COLMAN_NAME_QUERIES_HPP */

/* EOF */
