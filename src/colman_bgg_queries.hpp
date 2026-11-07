// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// BoardGameGeek queries for board games.
/**
   @file
   @author Christoph Pesch

   Provides query_name() for text searches on BoardGameGeek and get_item()
   to fetch full game details by BGG ID.

   @todo barcodelookup.com as alternative EAN source
*/

#ifndef COLMAN_BGG_QUERIES_HPP
#  define COLMAN_BGG_QUERIES_HPP

#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_OPTIONAL
#    include <optional>
#    define SYSTEM_OPTIONAL
#  endif
#  ifndef SYSTEM_QVECTOR
#    include <QVector>
#    define SYSTEM_QVECTOR
#  endif

namespace colman
{
  /// One entry from a BoardGameGeek text search.
  struct BggSearchResult
  {
    int id_ = 0;   ///< BGG game ID
    QString name_; ///< primary game name
    int year_ = 0; ///< year of publication (0 if unknown)
  };

  /// Blocking BGG look-up helper for board games.
  /**
     query_name() searches BoardGameGeek by text query and stores results
     for retrieval via get_search_results().  Call get_item() afterwards to
     fetch the full game record for a specific BGG ID.
  */
  class BggQueries
  {
  public:
    /// Constructs the BGG query helper.
    BggQueries() = default;
    /// Destroys the helper.
    ~BggQueries() = default;

    /// Sets the Bearer token for BGG XML API authentication.
    void set_bgg_token(QString const& token);

    /// Searches BGG by text query \a name.
    /** @return true if BGG returned at least one hit. */
    bool query_name(QString const& name);

    /// Returns all search results from the last query_name() call.
    QVector<BggSearchResult> get_search_results() const;

    /// Fetches full game details for \a bgg_id from BGG.
    /** @return Item on success, std::nullopt if retrieval or parsing failed. */
    std::optional<Item> get_item(int bgg_id);

  private:
    QString bgg_token_;                ///< Bearer token for BGG XML API
    QVector<BggSearchResult> results_; ///< search results
  };


  /// Sends a synchronous text search to BGG and returns the raw XML.
  /** @param token  Optional Bearer token for BGG API authentication. */
  QByteArray query_bgg_search(QString const& name,
                              QString const& token = QString());

  /// Parses a BGG search XML response into a list of BggSearchResult entries.
  QVector<BggSearchResult> bgg_search_to_results(QByteArray const& xml);

  /// Sends a synchronous thing-detail request to BGG and returns the raw XML.
  /** @param token  Optional Bearer token for BGG API authentication. */
  QByteArray query_bgg_thing(int bgg_id, QString const& token = QString());

  /// Parses a BGG thing XML response into an Item.
  Item bgg_thing_to_item(QByteArray const& xml);

} // namespace colman

#endif /* #ifndef COLMAN_BGG_QUERIES_HPP */

/* EOF */
