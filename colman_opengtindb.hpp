// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// opengtindb.org EAN look-up helper.
/**
   @file
   @author Christoph Pesch

   Provides a single entry point, query_opengtindb_ean(), that resolves an
   EAN-13 via opengtindb.org and returns an Item with name, description, and
   publisher when found.  The lower-level helpers query_opengtindb() and
   opengtindb_to_item() are also exposed so that unit tests can exercise the
   parser without network access.
*/

#ifndef COLMAN_OPENGTINDB_HPP
#  define COLMAN_OPENGTINDB_HPP

#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef SYSTEM_OPTIONAL
#    include <optional>
#    define SYSTEM_OPTIONAL
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Sends a synchronous EAN request to opengtindb.org and returns the body.
  /** @param ean     EAN-13 to query.
      @param userid  User ID (queryid) for opengtindb.org. */
  QByteArray query_opengtindb(QString const& ean, QString const& userid);

  /// Parses an opengtindb.org text/plain response into an Item.
  /** Fills name (detailname stripped of trailing article codes, or name),
      description (descr, or descr_en as fallback), and publisher (vendor).
      Returns a default Item if the response is empty or contains an error. */
  Item opengtindb_to_item(QByteArray const& data);

  /// Looks up \a ean on opengtindb.org and returns an Item if a name is found.
  /**
     Calls query_opengtindb() followed by opengtindb_to_item().  Returns
     std::nullopt when \a userid or \a ean is empty, the server returns an
     error, or the parsed item has no name.
     @param userid  opengtindb.org user ID (queryid).
     @param ean     EAN-13 to resolve.
     @return Item with at least a name set, or std::nullopt on failure.
  */
  std::optional<Item> query_opengtindb_ean(QString const& userid,
                                           QString const& ean);

} // namespace colman

#endif /* #ifndef COLMAN_OPENGTINDB_HPP */

/* EOF */
