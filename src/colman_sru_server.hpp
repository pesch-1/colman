// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Generic SRU server abstraction: struct, network utility, parsers,
/// persistence.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SRU_SERVER_HPP
#  define COLMAN_SRU_SERVER_HPP

#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_MEDIA_KIND_HPP
#    include "colman_media_kind.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QVECTOR
#    include <QVector>
#    define SYSTEM_QVECTOR
#  endif

namespace colman
{
  /// Describes a single SRU library catalogue server.
  struct SruServer
  {
    QString name;          ///< display name shown in preferences
    QString endpoint;      ///< base URL without query string
    QString isbn_field;    ///< CQL index for ISBN; empty = not supported
    QString ean_field;     ///< CQL index for EAN; empty = not supported
    QString title_field;   ///< CQL index for title; empty = not supported
    QString record_schema; ///< "MARC21-xml", "marcxml", "unimarcxchange", etc.
    bool enabled = false;  ///<  server is queried when true

    /// Returns true when the record schema contains "unimarc" (case-insensitive).
    bool is_unimarc() const;
  };

  /// Returns the built-in list of pre-configured SRU servers.
  QVector<SruServer> default_sru_servers();

  /// Loads the SRU server list from QSettings; falls back to
  /// default_sru_servers() when the key is absent or unparseable.
  QVector<SruServer> load_sru_servers();

  /// Saves \a servers to QSettings as compact JSON.
  void save_sru_servers(QVector<SruServer> const& servers);

  /// Performs a synchronous HTTP GET and returns the response body.
  /**
    Returns an empty QByteArray on network error or timeout.
    @param url        URL to retrieve.
    @param user_agent Optional User-Agent header; empty = Qt default.
    @return Response body, or an empty QByteArray on failure.
  */
  QByteArray synchronous_get(QString const& url,
                             QString const& user_agent = QString());

  /// Queries \a server for \a number in \a field and returns the raw response.
  QByteArray query_sru(SruServer const& server,
                       QString const& field,
                       QString const& number);

  /// Queries \a server for \a title in its title index (multi-record).
  /**
    The title is sent as a percent-encoded, quoted CQL phrase.
    @param server       The SRU server to query (uses its title_field).
    @param title        The title string to search for.
    @param max_records  Maximum number of records to request.
    @return Raw SRU response bytes, or empty on failure.
  */
  QByteArray query_sru_title(SruServer const& server,
                             QString const& title,
                             int max_records);

  /// Normalises a raw publication-date string to a validator-accepted form.
  /**
    Handles ISO partial dates, English long-form dates, and MARC21 year
    annotations such as "c2020.", "[2020]", "©2020".
    @param raw  The raw date string from an API response.
    @return A normalised date string, or an empty string if no date can be
            extracted.
  */
  QString normalize_date(QString const& raw);

  /// Parses a MARC21-xml SRU response and returns book item data.
  /**
    @param data  Raw SRU/MARC21-xml bytes.
    @param isbn  The ISBN that was queried; used as fallback when none is
                 found in the response.
    @return A colman::Item populated with the extracted book data.
  */
  Item marc21xml_to_book_item(QByteArray const& data, QString const& isbn);

  /// Parses a MARC21-xml SRU response and returns AV media item data.
  /**
    @param data  Raw SRU/MARC21-xml bytes.
    @return A colman::Item populated with the extracted AV media data.
  */
  Item marc21xml_to_av_item(QByteArray const& data);

  /// Parses a UNIMARC-xml SRU response (BnF) and returns book item data.
  /**
    @param data  Raw SRU/UNIMARC-xml bytes.
    @return A colman::Item populated with the extracted book data.
  */
  Item unimarcxml_to_book_item(QByteArray const& data);

  /// Parses all records of a MARC21-xml SRU response into book items.
  /**
    @param data  Raw SRU/MARC21-xml bytes.
    @return One colman::Item per MARC record in the response.
  */
  QVector<Item> marc21xml_to_book_items(QByteArray const& data);

  /// Parses all records of a MARC21-xml SRU response into AV media items.
  /**
    @param data  Raw SRU/MARC21-xml bytes.
    @return One colman::Item per MARC record in the response.
  */
  QVector<Item> marc21xml_to_av_items(QByteArray const& data);

  /// Parses all records of a UNIMARC-xml SRU response into book items.
  /**
    @param data  Raw SRU/UNIMARC-xml bytes.
    @return One colman::Item per UNIMARC record in the response.
  */
  QVector<Item> unimarcxml_to_book_items(QByteArray const& data);

  /// Parses a multi-record SRU title response, filtered by media kind.
  /**
    Keeps only records whose detected media kind matches \a kind; records
    whose kind cannot be detected are kept as well.  Generic video records
    match both MediaKind::Dvd and MediaKind::BluRay requests.  For UNIMARC
    servers only MediaKind::Book is supported; other kinds yield an empty
    result.
    @param server  The SRU server that produced \a data.
    @param data    Raw SRU response bytes.
    @param kind    The media kind requested by the title search.
    @return One colman::Item per matching record.
  */
  QVector<Item> sru_title_to_items(SruServer const& server,
                                   QByteArray const& data,
                                   MediaKind kind);

  /// Dispatches to the correct parser based on the server schema.
  /**
    Calls unimarcxml_to_book_item() for UNIMARC schemas and
    marc21xml_to_book_item() for all others.
    @param server  The SRU server that produced \a data.
    @param data    Raw SRU response bytes.
    @param isbn    The ISBN that was queried.
    @return A colman::Item populated with the extracted book data.
  */
  Item sru_to_book_item(SruServer const& server,
                        QByteArray const& data,
                        QString const& isbn);

  /// Dispatches to the correct AV-media parser based on the server schema.
  /**
    @param server  The SRU server that produced \a data.
    @param data    Raw SRU response bytes.
    @return A colman::Item populated with the extracted AV media data.
  */
  Item sru_to_av_item(SruServer const& server, QByteArray const& data);

} // namespace colman

#endif /* #ifndef COLMAN_SRU_SERVER_HPP */

/* EOF */
