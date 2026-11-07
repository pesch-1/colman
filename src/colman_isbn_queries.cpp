// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// ISBN queries
/**
   @file
   @author Christoph Pesch
*/

#include "colman_isbn_queries.hpp"
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef COLMAN_SRU_SERVER_HPP
#  include "colman_sru_server.hpp"
#endif
#ifndef COLMAN_ASSERT_HPP
#  include "colman_assert.hpp"
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
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


/// Checks whether a string is a valid ISBN-13.
/**
  Validates the length, digit content, and check digit of the given
  string according to the ISBN-13 standard.

  @param isbn13 The string to validate.
  @return True if @p isbn13 is a valid ISBN-13, false otherwise.
*/
bool colman::is_isbn13(QString const& isbn13)
{
  if(isbn13.size() != 13) {
    return false;
  }
  int check = 0;
  for(int i = 0; i < 11; i += 2) {
    int const tmp1 = isbn13[i].digitValue();
    int const tmp2 = isbn13[i + 1].digitValue();
    if(tmp1 < 0 || tmp2 < 0) {
      return false;
    }
    check += tmp1 + 3 * tmp2;
  }
  int const tmp = isbn13[12].digitValue();
  if(tmp < 0) {
    return false;
  }
  check += tmp;
  return ((check % 10) == 0);
}


/// Checks whether a string is a valid ISBN-10.
/**
  Validates the length, digit content (including the optional trailing
  'X' check character), and check digit of the given string according
  to the ISBN-10 standard.

  @param isbn10 The string to validate.
  @return True if @p isbn10 is a valid ISBN-10, false otherwise.
*/
bool colman::is_isbn10(QString const& isbn10)
{
  if(isbn10.size() != 10) {
    return false;
  }
  int check = 0;
  for(int i = 0; i < 10; ++i) {
    bool const is_check_x = (i == 9 && isbn10[i] == 'X');
    int const tmp = (is_check_x ? 10 : isbn10[i].digitValue());
    if(tmp < 0) {
      return false;
    }
    check += tmp * (10 - i);
  }
  return ((check % 11) == 0);
}


/// Converts a valid ISBN-10 to its ISBN-13 equivalent.
/**
  Prepends the "978" prefix to @p isbn10, drops the original check
  digit, and computes the correct ISBN-13 check digit.  Asserts that
  @p isbn10 is a valid ISBN-10.

  @param isbn10 A valid ISBN-10 string.
  @return The corresponding 13-digit ISBN string.
*/
QString colman::isbn10_to_isbn13(QString const& isbn10)
{
  COLMAN_VERIFY(is_isbn10(isbn10)).msg("no isbn-10");
  QString isbn13("978");
  isbn13 += isbn10;
  int check = 0;
  for(int i = 0; i < 11; i += 2) {
    int const tmp1 = isbn13[i].digitValue();
    int const tmp2 = isbn13[i + 1].digitValue();
    COLMAN_VERIFY(tmp1 >= 0 && tmp2 >= 0)
    (isbn13.toStdString())
        .msg("no isbn");
    check += tmp1 + 3 * tmp2;
  }
  check = (10 - (check % 10)) % 10;
  isbn13[12] = QChar('0' + check);
  return isbn13;
}


/// Converts a valid ISBN-13 to its ISBN-10 equivalent.
/**
  Only ISBN-13 numbers with the "978" prefix can be represented as
  ISBN-10.  For those, the function strips the prefix, recomputes the
  check digit (using 'X' when the check value equals 10), and returns
  the result.  For all other prefixes an empty string is returned.
  Asserts that @p isbn13 is a valid ISBN-13.

  @param isbn13 A valid ISBN-13 string.
  @return The corresponding ISBN-10 string, or an empty string if the
          ISBN-13 cannot be represented as ISBN-10.
*/
QString colman::isbn13_to_isbn10(QString const& isbn13)
{
  COLMAN_VERIFY(is_isbn13(isbn13)).msg("no isbn-13");
  if(isbn13.startsWith("978")) {
    QString isbn10 = isbn13.right(10);
    int check = 0;
    for(int i = 0; i < 9; ++i) {
      int const tmp = isbn10[i].digitValue();
      check += tmp * (10 - i);
    }
    check = (11 - (check % 11)) % 11;
    if(check < 10) {
      isbn10[9] = QChar('0' + check);
    }
    else {
      isbn10[9] = 'X';
    }
    return isbn10;
  }
  return QString();
}


/// Fills both ISBN-10 and ISBN-13 fields of an item from a single ISBN string.
/**
  Determines whether @p isbn is an ISBN-10 or ISBN-13 and sets both
  the "ISBN10" and "ISBN13" values on @p item by converting between
  the two formats as needed.  Does nothing if @p isbn is neither a
  valid ISBN-10 nor a valid ISBN-13.

  @param isbn  A valid ISBN-10 or ISBN-13 string.
  @param item  The item whose ISBN fields are to be populated.
*/
void colman::fill_isbn(QString const& isbn, colman::Item& item)
{
  if(is_isbn13(isbn)) {
    item.set_list_value("ISBN13", {isbn});
    item.set_list_value("ISBN10", {isbn13_to_isbn10(isbn)});
  }
  else if(is_isbn10(isbn)) {
    item.set_list_value("ISBN10", {isbn});
    item.set_list_value("ISBN13", {isbn10_to_isbn13(isbn)});
  }
}


namespace
{
  /// Queries Open Library and returns the parsed book item.
  /**
    Combines query_isbn_openlibrary() and openlibrary_to_item().  Stores
    the cover image URL in the "cover_url" item value.

    @param isbn  The ISBN to query (ISBN-10 or ISBN-13).
    @return A colman::Item with the book data and "cover_url" value.
  */
  colman::Item fetch_openlibrary(QString const& isbn)
  {
    QString cover;
    QByteArray const data = colman::query_isbn_openlibrary(isbn);
    colman::Item item = colman::openlibrary_to_item(data, cover);
    if(!cover.isEmpty()) {
      item.set_value("cover_url", cover);
    }
    return item;
  }

  /// Queries the Google Books API and returns the parsed book item.
  /**
    Combines query_isbn_googleapi() and googleapis_to_item().  Stores
    the cover image URL in the "cover_url" item value.

    @param isbn  The ISBN to query (ISBN-10 or ISBN-13).
    @return A colman::Item with the book data and "cover_url" value.
  */
  colman::Item fetch_googleapi(QString const& isbn, QString const& key)
  {
    QString cover;
    QByteArray const data = colman::query_isbn_googleapi(isbn, key);
    colman::Item item = colman::googleapis_to_item(isbn, data, cover);
    if(!cover.isEmpty()) {
      item.set_value("cover_url", cover);
    }
    return item;
  }

} // namespace


/// Sets the optional Google Books API key.
/**
  If non-empty, the key is appended as @c &key=... to each request URL.
  @param key  Google Books API key, or empty string for anonymous access.
*/
void colman::IsbnQueries::set_googleapis_key(QString const& key)
{
  googleapis_key_ = key;
}


/// Enables or disables the Open Library service.
/** @param b  true to enable, false to disable (default: true). */
void colman::IsbnQueries::use_openlibrary(bool b)
{
  use_ol_ = b;
}

/// Enables or disables the Google Books service.
/** @param b  true to enable, false to disable (default: true). */
void colman::IsbnQueries::use_googleapis(bool b)
{
  use_ga_ = b;
}

/// Sets the list of SRU servers to query.
/** @param servers  List of SruServer objects; only enabled ones are queried. */
void colman::IsbnQueries::set_sru_servers(QVector<SruServer> const& servers)
{
  sru_servers_ = servers;
}


/**
  Runs parallel ISBN lookups and blocks until all have finished.

  Starts one QtConcurrent thread per enabled service (Open Library,
  Google Books, and each enabled SRU server that supports ISBN lookup),
  waits for them all, then merges the results into result_.  Each
  individual query enforces its own network timeout (from QSettings
  "timeout").

  @param isbn  The ISBN to look up (ISBN-10 or ISBN-13).
  @return True if at least one lookup was started, false if no service
          is enabled.
*/
bool colman::IsbnQueries::query_isbn(QString const& isbn)
{
  isbn_ = isbn;
  cover_url_ = QString();
  result_ = Item();
  bool const use_ol = use_ol_;
  bool const use_ga = use_ga_;
  QVector<SruServer> active_sru;
  for(auto const& srv : sru_servers_) {
    if(srv.enabled && !srv.isbn_field.isEmpty()) {
      active_sru.append(srv);
    }
    else {
      qCDebug(colman_lookup_log) << "[SRU isbn] skipping" << srv.name
                                 << "enabled=" << srv.enabled << "isbn_field="
                                 << srv.isbn_field;
    }
  }
  qCDebug(colman_lookup_log) << "[SRU isbn] lookup isbn=" << isbn << "ol="
                             << use_ol << "ga=" << use_ga << "sru_servers="
                             << active_sru.size();
  if(!use_ol && !use_ga && active_sru.isEmpty()) {
    return false;
  }
  QFuture<Item> f_ol, f_ga;
  QVector<QFuture<Item>> f_sru(active_sru.size());
  if(use_ol) {
    f_ol = QtConcurrent::run(fetch_openlibrary, isbn);
  }
  if(use_ga) {
    f_ga = QtConcurrent::run(fetch_googleapi, isbn, googleapis_key_);
  }
  for(int i = 0; i < active_sru.size(); ++i) {
    auto const srv = active_sru[i];
    f_sru[i] = QtConcurrent::run([srv, isbn]() -> Item {
      qCDebug(colman_lookup_log) << "[SRU isbn] querying" << srv.name << "for"
                                 << isbn;
      auto const data = colman::query_sru(srv, srv.isbn_field, isbn);
      if(data.isEmpty()) {
        qCDebug(colman_lookup_log) << "[SRU isbn]" << srv.name
                                   << ": network error / timeout";
        return Item();
      }
      auto const item = colman::sru_to_book_item(srv, data, isbn);
      if(item.get_name().isEmpty()) {
        qCDebug(colman_lookup_log) << "[SRU isbn]" << srv.name << ": no hit";
      }
      else {
        qCDebug(colman_lookup_log) << "[SRU isbn]" << srv.name << ": hit -"
                                   << item.get_name();
      }
      return item;
    });
  }
  if(use_ol) {
    f_ol.waitForFinished();
  }
  if(use_ga) {
    f_ga.waitForFinished();
  }
  for(auto& f : f_sru) {
    f.waitForFinished();
  }
  fill_isbn(isbn, result_);
  if(use_ol) {
    result_ += f_ol.result();
  }
  if(use_ga) {
    result_ += f_ga.result();
  }
  for(auto const& f : f_sru) {
    result_ += f.result();
  }
  // Pick the most specific publication date across all sources.
  // operator+= keeps the first non-empty value, so a year-only date from one
  // source can block a more specific date from another.  A longer string
  // means a more specific date (e.g. "15.1.2020" > "1.2020" > "2020").
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
    if(use_ol) {
      pick(f_ol);
    }
    if(use_ga) {
      pick(f_ga);
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


/// Returns the merged book data populated by the last query_isbn() call.
/**
  @return A colman::Item containing the combined book metadata from
          all services that returned a successful response.
*/
colman::Item colman::IsbnQueries::get_book_data() const
{
  return result_;
}


/// Downloads the book cover thumbnail and returns it as a pixmap.
/**
  Retrieves the thumbnail URL via get_thumbnail_url() and performs a
  synchronous HTTP GET using an internal QEventLoop.  The download is
  subject to the timeout configured in the application settings.
  Returns a null QPixmap when no URL is available, the download times
  out, or a network error occurs.

  @return A QPixmap containing the downloaded cover image, or a null
          QPixmap if the image could not be retrieved.
*/
QPixmap colman::IsbnQueries::get_thumbnail() const
{
  QPixmap pm;
  QString const url = get_thumbnail_url();
  if(!url.isEmpty()) {
    QByteArray const data = colman::synchronous_get(url);
    pm.loadFromData(data);
  }
  return pm;
}


/// Returns the URL of the best available cover image.
/**
  Returns the cover image URL gathered from any service that provided
  one (stored as "cover_url" in the merged item).  Returns an empty
  string when no cover URL is available.

  @return The cover image URL as a string, or an empty string if
          no cover URL is available.
*/
QString colman::IsbnQueries::get_thumbnail_url() const
{
  return cover_url_;
}


/**
  Constructs an IsbnValidator that accepts ISBNs of the given type.

  @param type    The ISBN format to validate (IsbnType::Isbn10 or
                 IsbnType::Isbn13).
  @param parent  The parent QObject, passed to the QValidator base
                 class constructor.
*/
colman::IsbnValidator::IsbnValidator(IsbnType type, QObject* parent)
    : QValidator(parent),
      type_(type)
{
}


/**
  Removes all characters from @p input that are not digits or 'X'.

  Called by Qt when the user finishes editing and the current input is
  not acceptable.  Modifies @p input in place.

  @param input  The string to fix up; modified in place.
*/
void colman::IsbnValidator::fixup(QString& input) const
{
  int n = input.size();
  for(int i = 0; i < n;) {
    if(!(input[i].isDigit() || input[i] == QChar('X'))) {
      input.remove(i, 1);
      --n;
    }
    else {
      ++i;
    }
  }
}


/**
  Validates the current content of @p input against the expected ISBN
  format.

  Returns QValidator::Acceptable when the string is a complete, valid
  ISBN of the configured type, QValidator::Intermediate while the
  string could still become valid with further input, and
  QValidator::Invalid when the string cannot possibly become valid.

  @param input  The string being edited; may be modified by the
                validator (e.g. to normalise case).
  @param pos    The current cursor position within @p input.
  @return The validation state of the current input.
*/
QValidator::State colman::IsbnValidator::validate(QString& input, int& pos) const
{
  int const len = input.size();
  if(pos > 0 && !(input[pos - 1].isDigit() || input[pos - 1] == 'X')) {
    return QValidator::Invalid;
  }
  if(type_ == IsbnType::Isbn10) {
    if(len == 10) {
      return is_isbn10(input)
                 ? QValidator::Acceptable
                 : (pos > 9 ? QValidator::Invalid : QValidator::Intermediate);
    }
    else if(len == 11 && pos < 11) {
      return QValidator::Intermediate;
    }
    else if(len < 10) {
      return QValidator::Intermediate;
    }
    else {
      return QValidator::Invalid;
    }
  }
  else if(type_ == IsbnType::Isbn13) {
    if(len == 13) {
      return is_isbn13(input)
                 ? QValidator::Acceptable
                 : (pos > 12 ? QValidator::Invalid : QValidator::Intermediate);
    }
    else if(len == 14 && pos < 14) {
      return QValidator::Intermediate;
    }
    else if(len < 13) {
      return QValidator::Intermediate;
    }
    else {
      return QValidator::Invalid;
    }
  }
  return QValidator::Intermediate;
}


/// Parses an Open Library JSON response and returns the extracted item data.
/**
   Parses the JSON returned by the Open Library Books API and fills a
   colman::Item with ISBN-10, ISBN-13, title, authors, publisher, page
   count, and publication date.  Also extracts the medium-size cover
   URL into @p cover.  Returns a default-constructed item if the
   response cannot be parsed.

    @param json   Raw JSON bytes from the Open Library API response.
    @param cover  Output parameter filled with the URL of the medium
    cover image, if available.
    @return A colman::Item populated with the book data found in @p json.
 */
colman::Item colman::openlibrary_to_item(QByteArray const& json, QString& cover)
{
  //QTextStream(stdout) << json << "\n";
  colman::Item item;
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(json, &error);
  if(doc.isObject()) {
    QJsonObject const root = doc.object();
    for(auto const& book : root) {
      QJsonObject const ids = book.toObject()["identifiers"].toObject();
      auto const first = [](QJsonArray const& a) -> QString {
        return a.isEmpty() ? QString() : a[0].toString();
      };
      QString const isbn10 = first(ids["isbn_10"].toArray());
      QString const isbn13 = first(ids["isbn_13"].toArray());
      QString const& title = book.toObject()["title"].toString();
      int const pages = book.toObject()["number_of_pages"].toInt();
      QStringList authors;
      for(auto const& a : book.toObject()["authors"].toArray()) {
        QString const name = a.toObject()["name"].toString();
        if(!name.isEmpty()) {
          authors += name;
        }
      }
      QString publishers;
      QJsonArray const jpublishers = book.toObject()["publishers"].toArray();
      int const np = jpublishers.size();
      for(int i = 0; i < np; ++i) {
        if(i > 0) {
          publishers += ", ";
        }
        publishers += jpublishers[i].toObject()["name"].toString();
      }
      QString const published = book.toObject()["publish_date"].toString();
      cover = book.toObject()["cover"].toObject()["medium"].toString();
      item.set_name(title);
      if(!authors.isEmpty()) {
        item.set_list_value("authors", authors);
      }
      if(pages > 0) {
        item.set_value("pages", QString().setNum(pages));
      }
      item.set_value("publication_date", normalize_date(published));
      item.set_value("publisher", publishers);
      item.set_list_value("ISBN13", {isbn13});
      item.set_list_value("ISBN10", {isbn10});
      // description can be a plain string or {"type":…, "value":"…"}.
      QJsonValue const jdesc = book.toObject()["description"];
      QString description;
      if(jdesc.isString()) {
        description = jdesc.toString();
      }
      else if(jdesc.isObject()) {
        description = jdesc.toObject()["value"].toString();
      }
      if(!description.isEmpty()) {
        item.set_description(description);
      }
      QJsonArray const jddc = book.toObject()["dewey_decimal_class"].toArray();
      if(!jddc.isEmpty()) {
        item.set_list_value("ddc", {jddc[0].toString()});
      }
      QString genres;
      for(auto const& g : book.toObject()["genres"].toArray()) {
        if(!genres.isEmpty()) {
          genres += "; ";
        }
        genres += g.toString();
      }
      if(!genres.isEmpty()) {
        item.set_value("genre", genres);
      }
      break; // use only the first book entry
    }
  }
  return item;
}


/// Parses a Google Books API JSON response and returns the extracted item data.
/**
   Parses the JSON returned by the Google Books API, finds the volume
   entry whose ISBN-10 or ISBN-13 matches @p isbn, and fills a
   colman::Item with title, authors, description, page count,
   publication date, and ISBN values.  Also extracts the thumbnail URL
   into @p thumbnail_url.  Returns a default-constructed item if no
   matching entry is found or the response cannot be parsed.

    @param isbn           The ISBN that was queried (ISBN-10 or ISBN-13).
    @param json           Raw JSON bytes from the Google Books API response.
    @param thumbnail_url  Output parameter filled with the thumbnail image
    URL of the matching volume, if available.
    @return A colman::Item populated with the book data for the matching
    volume, or a default-constructed item if none matched.
 */
colman::Item colman::googleapis_to_item(QString const& isbn,
                                        QByteArray const& json,
                                        QString& thumbnail_url)
{
  colman::Item item;
  if(json.isEmpty()) {
    return item;
  }
  QJsonParseError error;
  QJsonDocument const doc = QJsonDocument::fromJson(json, &error);
  if(doc.isObject()) {
    QJsonObject const root = doc.object();
    QJsonArray const jitems = root["items"].toArray();
    for(auto const& it : jitems) {
      QJsonObject const volinfo = it.toObject()["volumeInfo"].toObject();
      QString thumbnail;
      colman::Item const candidate =
          googleapis_volume_to_item(volinfo, thumbnail);
      auto const& lists = candidate.get_list_values();
      auto const isbn10_it = lists.find("ISBN10");
      auto const isbn13_it = lists.find("ISBN13");
      bool const matches =
          (isbn10_it != lists.end() && isbn10_it->second.contains(isbn)) ||
          (isbn13_it != lists.end() && isbn13_it->second.contains(isbn));
      if(matches) {
        item = candidate;
        thumbnail_url = thumbnail;
        break;
      }
    }
  }
  return item;
}


/// Parses one Google Books volumeInfo object into an Item.
/**
   Extracts title (with subtitle), authors, ISBNs, publisher, publication
   date, page count, genre, and description from a single volumeInfo
   object of a Google Books API response.  The thumbnail image URL is
   stored in @p thumbnail_url.

   @param volinfo        One volumeInfo JSON object.
   @param thumbnail_url  Output parameter filled with the thumbnail image
                         URL of the volume, if available.
   @return A colman::Item populated with the extracted book data.
*/
colman::Item colman::googleapis_volume_to_item(QJsonObject const& volinfo,
                                               QString& thumbnail_url)
{
  colman::Item item;
  QString isbn10, isbn13;
  for(auto const& identifier : volinfo["industryIdentifiers"].toArray()) {
    QJsonObject const id = identifier.toObject();
    QString const type = id["type"].toString();
    if(type == "ISBN_10") {
      isbn10 = id["identifier"].toString();
    }
    else if(type == "ISBN_13") {
      isbn13 = id["identifier"].toString();
    }
  }
  QString title = volinfo["title"].toString();
  if(title.isEmpty()) {
    return item;
  }
  QString const subtitle = volinfo["subtitle"].toString();
  if(!subtitle.isEmpty()) {
    title += " : ";
    title += subtitle;
  }
  QStringList authors;
  for(auto const& author : volinfo["authors"].toArray()) {
    QString const name = author.toString();
    if(!name.isEmpty()) {
      authors += name;
    }
  }
  QString categories;
  for(auto const& c : volinfo["categories"].toArray()) {
    if(!categories.isEmpty()) {
      categories += "; ";
    }
    categories += c.toString();
  }
  int const pageCount = volinfo["pageCount"].toInt();
  thumbnail_url = volinfo["imageLinks"].toObject()["thumbnail"].toString();
  item.set_name(title);
  if(!authors.isEmpty()) {
    item.set_list_value("authors", authors);
  }
  item.set_value("publisher", volinfo["publisher"].toString());
  item.set_value("publication_date",
                 normalize_date(volinfo["publishedDate"].toString()));
  if(!isbn10.isEmpty()) {
    item.set_list_value("ISBN10", {isbn10});
  }
  if(!isbn13.isEmpty()) {
    item.set_list_value("ISBN13", {isbn13});
  }
  if(pageCount > 0) {
    item.set_value("pages", QString::number(pageCount));
  }
  if(!categories.isEmpty()) {
    item.set_value("genre", categories);
  }
  QString const description = volinfo["description"].toString();
  if(!description.isEmpty()) {
    item.set_description(description);
  }
  return item;
}


/// Sends a synchronous ISBN lookup to Open Library and returns the raw
/// response.
/**
  Performs a synchronous HTTP GET to the Open Library Books API and
  returns the raw JSON response body.  Returns an empty QByteArray on
  network error or timeout.

  @param isbn  The ISBN to query (ISBN-10 or ISBN-13).
  @return Raw JSON from Open Library, or an empty QByteArray on failure.
*/
QByteArray colman::query_isbn_openlibrary(QString const& isbn)
{
  return colman::synchronous_get(
      "https://openlibrary.org/api/books?bibkeys=ISBN:" + isbn +
      "&format=json&jscmd=data");
}


/// Sends a synchronous ISBN lookup to the Google Books API and returns
/// the raw response.
/**
  Performs a synchronous HTTP GET to the Google Books API and returns
  the raw JSON response body.  Returns an empty QByteArray on network
  error or timeout.

  @param isbn  The ISBN to query (ISBN-10 or ISBN-13).
  @return Raw JSON from Google Books API, or an empty QByteArray on
          failure.
*/
QByteArray colman::query_isbn_googleapi(QString const& isbn,
                                        QString const& key)
{
  QString urlstr("https://www.googleapis.com/books/v1/volumes?q=isbn:");
  urlstr += isbn;
  if(!key.isEmpty()) {
    urlstr += "&key=";
    urlstr += key;
  }
  return colman::synchronous_get(urlstr);
}


/// Sends a synchronous title search to the Google Books API and returns
/// the raw response.
/**
  Performs a synchronous HTTP GET to the Google Books API, searching for
  volumes whose title contains \a title, and returns the raw JSON
  response body.  Returns an empty QByteArray on network error or
  timeout.

  @param title  The title string to search for.
  @param key    Optional Google Books API key.
  @return Raw JSON from the Google Books API, or an empty QByteArray on
          failure.
*/
QByteArray colman::query_title_googleapi(QString const& title,
                                         QString const& key)
{
  QString urlstr("https://www.googleapis.com/books/v1/volumes"
                 "?q=intitle:%22");
  urlstr += QUrl::toPercentEncoding(title.trimmed());
  urlstr += "%22&maxResults=10";
  if(!key.isEmpty()) {
    urlstr += "&key=";
    urlstr += key;
  }
  return colman::synchronous_get(urlstr);
}


#if 0
int main(int argc, char* argv[])
{
  if(argc > 1){
    COLMAN_TRY{
      COLMAN_VERIFY(colman::is_isbn10(argv[1]) || colman::is_isbn13(argv[1]))
        (argv[1]).msg("no isbn");
      QApplication app(argc, argv);
      QNetworkProxyFactory::setUseSystemConfiguration(true);
      colman::IsbnQueries queries;
      queries.set_isbndb_access_key("XRV3B9FH");
      queries.set_librarything_api_key("c698b06e36c7c45f1b611ade2ea42c8b");
      QEventLoop loop;
      colman::Item item;
      loop.connect(&queries, &colman::IsbnQueries::book_data_available,
                   &loop, &QEventLoop::quit);
      queries.query_isbn(argv[1]);
      loop.exec();
      colman::print_item(queries.get_book_data());
      //return app.exec();
    } COLMAN_CATCH;
  }
  return 0;
}
#endif

/* EOF */
