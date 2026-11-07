// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// ISBN queries
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ISBN_QUERIES_HPP
#  define COLMAN_ISBN_QUERIES_HPP

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
#  ifndef SYSTEM_QVALIDATOR
#    include <QValidator>
#    define SYSTEM_QVALIDATOR
#  endif
#  ifndef SYSTEM_QVECTOR
#    include <QVector>
#    define SYSTEM_QVECTOR
#  endif

namespace colman
{
  /// Blocking ISBN look-up helper using multiple online data sources.
  /**
     Sends parallel HTTP requests to OpenLibrary, Google Books, and
     configured SRU servers.  query_isbn() blocks until all replies have
     arrived (each individual request enforces its own network timeout).
     The results are merged and returned by get_book_data().

     @todo EAN-Search.org
     @todo WorldCat.org, ISBN-Suche.net, Buchhandel.de, eurobuch.com ???
  */
  class IsbnQueries
  {
  public:
    /// Constructs the ISBN query helper.
    IsbnQueries() = default;
    /// Destroys the helper.
    ~IsbnQueries() = default;

    /// Sets the optional Google Books API key.
    void set_googleapis_key(QString const& key);

    /// Enables or disables the Open Library service.
    void use_openlibrary(bool b = true);
    /// Enables or disables the Google Books service.
    void use_googleapis(bool b = true);
    /// Sets the list of SRU servers to query.
    void set_sru_servers(QVector<SruServer> const& servers);

    /// Starts parallel queries for \a isbn and blocks until all have finished.
    /** @return true if at least one query was started */
    bool query_isbn(QString const& isbn);
    /// Returns the merged book data from all completed queries.
    Item get_book_data() const;
    /// Returns the URL of the best available thumbnail image.
    QString get_thumbnail_url() const;
    /// Downloads and returns the thumbnail pixmap (blocking).
    QPixmap get_thumbnail() const;

  private:
    QString isbn_;                  ///< ISBN being queried
    mutable QString cover_url_;     ///< URL of cover image
    Item result_;                   ///< merged query result
    QString googleapis_key_;        ///< optional Google Books API key
    bool use_ol_ = false;           ///< query Open Library
    bool use_ga_ = false;           ///< query Google Books
    QVector<SruServer> sru_servers_; ///< SRU servers to query
  };


  /// Returns true if \a isbn13 is a syntactically valid ISBN-13 string.
  bool is_isbn13(QString const& isbn13);
  /// Returns true if \a isbn10 is a syntactically valid ISBN-10 string.
  bool is_isbn10(QString const& isbn10);
  /// Converts an ISBN-10 string to its ISBN-13 equivalent.
  QString isbn10_to_isbn13(QString const& isbn10);
  /// Converts an ISBN-13 string to its ISBN-10 equivalent, if possible.
  QString isbn13_to_isbn10(QString const& isbn13);

  /// Stores the canonical ISBN forms in \a item's values map.
  void fill_isbn(QString const& isbn, Item& item);


  /// Selects whether an IsbnValidator accepts ISBN-10 or ISBN-13 strings.
  enum struct IsbnType { Isbn10 = 1,
                         Isbn13 = 2 };

  /// QValidator that accepts only syntactically correct ISBN strings.
  /**
     Validates input according to the selected IsbnType and attempts to
     fix up partial or incorrectly formatted ISBN strings.
  */
  class IsbnValidator : public QValidator
  {
    Q_OBJECT

  public:
    /// Constructs a validator for the given \a type (default: ISBN-13).
    IsbnValidator(IsbnType type = IsbnType::Isbn13, QObject* parent = nullptr);
    /// Attempts to auto-correct \a input to a valid ISBN.
    void fixup(QString& input) const override;
    /// Returns Acceptable, Intermediate, or Invalid for the current \a input.
    State validate(QString& input, int& pos) const override;

  private:
    IsbnType type_; ///< the accepted ISBN type
  };


  QByteArray query_isbn_openlibrary(QString const& isbn);
  colman::Item openlibrary_to_item(QByteArray const& xml, QString& cover);

  QByteArray query_isbn_googleapi(QString const& isbn,
                                  QString const& key = QString());
  colman::Item googleapis_to_item(QString const& isbn,
                                  QByteArray const& xml,
                                  QString& thumbnail_url);

  /// Sends a synchronous title search to the Google Books API.
  QByteArray query_title_googleapi(QString const& title,
                                   QString const& key = QString());
  /// Parses one Google Books volumeInfo object into an Item.
  colman::Item googleapis_volume_to_item(QJsonObject const& volinfo,
                                         QString& thumbnail_url);


} // namespace colman

#endif /* #ifndef COLMAN_ISBN_QUERIES_HPP */

/* EOF */
