// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// EAN product barcode queries for CDs, DVDs and Blu-rays.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_EAN_QUERIES_HPP
#  define COLMAN_EAN_QUERIES_HPP

#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_SRU_SERVER_HPP
#    include "colman_sru_server.hpp"
#  endif
#  ifndef SYSTEM_QJSONOBJECT
#    include <QJsonObject>
#    define SYSTEM_QJSONOBJECT
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
  /// Returns true if \a ean is a syntactically valid product EAN-13.
  /** An EAN-13 that starts with 978 or 979 is an ISBN and returns false. */
  bool is_product_ean(QString const& ean);

  /// Returns true if \a upc is a syntactically valid 12-digit UPC-A.
  bool is_upc_a(QString const& upc);

  /// Blocking EAN look-up helper using MusicBrainz and SRU servers.
  /**
     Sends parallel HTTP requests to MusicBrainz and/or configured SRU
     servers.  query_ean() blocks until all replies have arrived (each
     individual request enforces its own network timeout).  The results are
     merged and returned by get_item_data().

     MusicBrainz covers music releases (CDs) world-wide.
     SRU servers (e.g. DNB) cover physical media including CDs, DVDs, and
     Blu-rays.
  */
  class EanQueries
  {
  public:
    /// Constructs the EAN query helper.
    EanQueries() = default;
    /// Destroys the helper.
    ~EanQueries() = default;

    /// Enables or disables the MusicBrainz service.
    void use_musicbrainz(bool b = true);
    /// Sets the list of SRU servers to query.
    void set_sru_servers(QVector<SruServer> const& servers);

    /// Starts parallel queries for \a ean and blocks until all have finished.
    /** @return true if at least one query was started */
    bool query_ean(QString const& ean);
    /// Returns the merged item data from all completed queries.
    Item get_item_data() const;
    /// Returns the URL of the best available thumbnail image.
    QString get_thumbnail_url() const;
    /// Downloads and returns the thumbnail pixmap (blocking).
    QPixmap get_thumbnail() const;

  private:
    QString ean_;               ///< EAN being queried
    mutable QString cover_url_; ///< URL of cover/product image
    Item result_;               ///< merged query result
    bool use_mb_ = false;       ///< query MusicBrainz
    QVector<SruServer> sru_servers_; ///< SRU servers to query
  };

  QByteArray query_ean_musicbrainz(QString const& ean);
  Item musicbrainz_to_item(QByteArray const& json, QString& cover_url);

  /// Sends a synchronous title search to MusicBrainz.
  QByteArray query_title_musicbrainz(QString const& title);
  /// Parses one MusicBrainz release object into an Item.
  Item musicbrainz_release_to_item(QJsonObject const& release,
                                   QString& cover_url);

  /// Compatibility wrapper: delegates to marc21xml_to_av_item().
  inline Item dnb_ean_to_item(QByteArray const& data)
  {
    return marc21xml_to_av_item(data);
  }

} // namespace colman

#endif /* #ifndef COLMAN_EAN_QUERIES_HPP */

/* EOF */
