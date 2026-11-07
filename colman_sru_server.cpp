// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Generic SRU server abstraction: struct, network utility, parsers,
/// persistence.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sru_server.hpp"
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#endif
#ifndef SYSTEM_QAUTHENTICATOR
#  include <QAuthenticator>
#  define SYSTEM_QAUTHENTICATOR
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QDOMELEMENT
#  include <QDomElement>
#  define SYSTEM_QDOMELEMENT
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
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QNETWORKREPLY
#  include <QNetworkReply>
#  define SYSTEM_QNETWORKREPLY
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
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif

namespace
{
  /// Decodes a Latin-1/Windows-1252 XML byte array into UTF-8.
  /**
    SRU servers (e.g. K10plus) occasionally declare
    @c encoding="iso-8859-1" but encode bytes 0x80–0x9F as Windows-1252.
    ISO-8859-1 maps that range to C1 control characters, which Qt displays
    as outlined boxes.  This function re-encodes such responses as UTF-8
    (correctly mapping the CP1252-specific bytes) and updates the encoding
    declaration so that QDomDocument can parse the result without error.
    Responses with any other declared encoding are returned unchanged.
    @param xml  Raw SRU response bytes.
    @return Bytes suitable for QDomDocument::setContent().
  */
  QByteArray normalize_xml_encoding(QByteArray const& xml)
  {
    QByteArray const prefix = xml.left(256).toLower();
    bool const is_latin1 =
        prefix.contains("encoding=\"iso-8859-1\"") ||
        prefix.contains("encoding='iso-8859-1'") ||
        prefix.contains("encoding=\"windows-1252\"") ||
        prefix.contains("encoding='windows-1252'");
    if(!is_latin1) {
      return xml;
    }
    static QRegularExpression const EncDecl(
        R"(encoding\s*=\s*["'][^"']*["'])",
        QRegularExpression::CaseInsensitiveOption);
    // Servers like k10plus declare ISO-8859-1 but actually send UTF-8.
    // If fromUtf8() introduces no replacement characters the content is
    // valid UTF-8: only the declaration needs patching.
    QString text_utf8 = QString::fromUtf8(xml);
    if(!text_utf8.contains(QChar(0xFFFD))) {
      text_utf8.replace(EncDecl, "encoding=\"UTF-8\"");
      return text_utf8.toUtf8();
    }
    // Content is genuine CP1252/Latin-1: transcode bytes 0x80-0x9F and
    // leave everything else as its Latin-1 Unicode code point.
    static QChar const Cp1252Map[32] = {
      QChar(0x20AC), QChar(0x0081), QChar(0x201A), QChar(0x0192), // 80-83
      QChar(0x201E), QChar(0x2026), QChar(0x2020), QChar(0x2021), // 84-87
      QChar(0x02C6), QChar(0x2030), QChar(0x0160), QChar(0x2039), // 88-8B
      QChar(0x0152), QChar(0x008D), QChar(0x017D), QChar(0x008F), // 8C-8F
      QChar(0x0090), QChar(0x2018), QChar(0x2019), QChar(0x201C), // 90-93
      QChar(0x201D), QChar(0x2022), QChar(0x2013), QChar(0x2014), // 94-97
      QChar(0x02DC), QChar(0x2122), QChar(0x0161), QChar(0x203A), // 98-9B
      QChar(0x0153), QChar(0x009D), QChar(0x017E), QChar(0x0178), // 9C-9F
    };
    QString text;
    text.reserve(xml.size());
    for(auto const byte : xml) {
      auto const c = static_cast<unsigned char>(byte);
      if(c >= 0x80 && c <= 0x9F) {
        text += Cp1252Map[c - 0x80];
      }
      else {
        text += QChar(c);
      }
    }
    text.replace(EncDecl, "encoding=\"UTF-8\"");
    return text.toUtf8();
  }

  /// Returns the default CQL title index for the server at \a endpoint.
  /**
    Used to migrate SRU server lists saved by versions that did not yet
    know the title_field attribute.
    @param endpoint  The endpoint URL of the stored server.
    @return The title_field of the matching default server, or an empty
            string when no default server has this endpoint.
  */
  QString default_title_field_for(QString const& endpoint)
  {
    for(auto const& srv : colman::default_sru_servers()) {
      if(srv.endpoint == endpoint) {
        return srv.title_field;
      }
    }
    return QString();
  }

  /// Returns the tag name of \a e without any namespace prefix.
  QString local_name(QDomElement const& e)
  {
    return e.tagName().section(':', -1);
  }

  /// Returns the first child of \a parent whose local tag name is \a name.
  QDomElement child_named(QDomElement const& parent, QString const& name)
  {
    QDomElement c = parent.firstChildElement();
    while(!c.isNull()) {
      if(local_name(c) == name) {
        return c;
      }
      c = c.nextSiblingElement();
    }
    return QDomElement();
  }

  /// Returns the trimmed text of the subfield of \a df with code \a code.
  QString subfield(QDomElement const& df, char code)
  {
    QDomElement sf = df.firstChildElement();
    while(!sf.isNull()) {
      if(local_name(sf) == "subfield" && sf.attribute("code") == QChar(code)) {
        return sf.text().trimmed();
      }
      sf = sf.nextSiblingElement();
    }
    return QString();
  }

  /// Removes MARC sorting marks and trailing ISBD punctuation from \a s.
  QString strip_marc(QString s)
  {
    s.remove(QChar(0x0098));
    s.remove(QChar(0x009C));
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

  /// Converts an inverted "Family, Forename" string to "Forename Family".
  QString natural_name(QString const& s)
  {
    int const comma = s.indexOf(',');
    if(comma > 0) {
      return s.mid(comma + 1).trimmed() + " " + s.left(comma).trimmed();
    }
    return s;
  }

  /// Extracts all MARC record elements from a raw SRU response.
  /**
    @param data  Raw SRU response bytes.
    @param doc   Document that owns the returned elements; must outlive
                 every use of them.
    @return All recordData/record elements, in response order.
  */
  QVector<QDomElement> sru_marc_records(QByteArray const& data,
                                        QDomDocument& doc)
  {
    QVector<QDomElement> records;
    if(data.isEmpty()) {
      return records;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto const parsed = doc.setContent(normalize_xml_encoding(data));
    if(!parsed) {
      qCDebug(colman_lookup_log) << "SRU parse error:" << parsed.errorMessage;
      return records;
    }
#else
    QString error_message;
    if(!doc.setContent(normalize_xml_encoding(data), &error_message)) {
      qCDebug(colman_lookup_log) << "SRU parse error:" << error_message;
      return records;
    }
#endif
    QDomElement const root = doc.documentElement();
    QDomElement const records_el = child_named(root, "records");
    if(records_el.isNull()) {
      return records;
    }
    QDomElement sru_record = records_el.firstChildElement();
    while(!sru_record.isNull()) {
      if(local_name(sru_record) == "record") {
        QDomElement const record_data = child_named(sru_record, "recordData");
        if(!record_data.isNull()) {
          QDomElement const marc = child_named(record_data, "record");
          if(!marc.isNull()) {
            records.append(marc);
          }
        }
      }
      sru_record = sru_record.nextSiblingElement();
    }
    return records;
  }

  /// Media kind detected in a single MARC21 record.
  /** Knows generic video records that cannot be narrowed to DVD or
      Blu-ray, unlike the public colman::MediaKind. */
  enum struct DetectedKind { Unknown,
                             Book,
                             Cd,
                             Video,
                             Dvd,
                             BluRay };

  /// Detects the media kind of one MARC21 record.
  /**
    Uses the leader (position 06), controlfield 007 (positions 00 and 04),
    and the carrier/extent text of datafields 338 $a and 300 $a.
    @param marc  One MARC21 record element.
    @return The detected kind, DetectedKind::Unknown if undetectable.
  */
  DetectedKind detect_marc21_kind(QDomElement const& marc)
  {
    DetectedKind kind = DetectedKind::Unknown;
    QString carrier_text;
    QDomElement el = marc.firstChildElement();
    while(!el.isNull()) {
      if(local_name(el) == "leader") {
        QString const ldr = el.text();
        if(ldr.size() > 6 && kind == DetectedKind::Unknown) {
          QChar const t = ldr[6];
          if(t == 'a' || t == 't') {
            kind = DetectedKind::Book;
          }
          else if(t == 'j' || t == 'i') {
            kind = DetectedKind::Cd;
          }
          else if(t == 'g') {
            kind = DetectedKind::Video;
          }
        }
      }
      else if(local_name(el) == "controlfield" &&
              el.attribute("tag") == "007") {
        QString const cf = el.text();
        if(!cf.isEmpty()) {
          if(cf[0] == 'v') {
            if(kind == DetectedKind::Unknown) {
              kind = DetectedKind::Video;
            }
            if(cf.size() > 4 && kind == DetectedKind::Video) {
              // Videorecording format: 'v' = DVD, 's' = Blu-ray.
              if(cf[4] == 'v') {
                kind = DetectedKind::Dvd;
              }
              else if(cf[4] == 's') {
                kind = DetectedKind::BluRay;
              }
            }
          }
          else if(cf[0] == 's' && kind == DetectedKind::Unknown) {
            kind = DetectedKind::Cd;
          }
        }
      }
      else if(local_name(el) == "datafield") {
        QString const tag = el.attribute("tag");
        if(tag == "300" || tag == "338") {
          carrier_text += " " + subfield(el, 'a').toLower();
        }
      }
      el = el.nextSiblingElement();
    }
    if(kind == DetectedKind::Video) {
      if(carrier_text.contains("blu-ray") || carrier_text.contains("bluray")) {
        kind = DetectedKind::BluRay;
      }
      else if(carrier_text.contains("dvd")) {
        kind = DetectedKind::Dvd;
      }
    }
    return kind;
  }

  /// Returns true when a record of \a detected kind matches \a requested.
  /**
    Undetectable records match every request; generic video records match
    both DVD and Blu-ray requests.
  */
  bool kind_matches(DetectedKind detected, colman::MediaKind requested)
  {
    switch(detected) {
    case DetectedKind::Unknown:
      return true;
    case DetectedKind::Book:
      return requested == colman::MediaKind::Book;
    case DetectedKind::Cd:
      return requested == colman::MediaKind::Cd;
    case DetectedKind::Video:
      return requested == colman::MediaKind::Dvd ||
             requested == colman::MediaKind::BluRay;
    case DetectedKind::Dvd:
      return requested == colman::MediaKind::Dvd;
    case DetectedKind::BluRay:
      return requested == colman::MediaKind::BluRay;
    }
    return true;
  }

  /// Parses one MARC21 record element into a book item.
  /**
    See colman::marc21xml_to_book_item() for the extracted MARC21 fields.
    @param marc  One MARC21 record element.
    @return A colman::Item populated with the extracted book data.
  */
  colman::Item marc21_record_to_book_item(QDomElement const& marc)
  {
    colman::Item item;
    QString authors;
    QDomElement df = marc.firstChildElement();
    while(!df.isNull()) {
      if(local_name(df) == "datafield") {
        QString const tag = df.attribute("tag");

        if(tag == "020") {
          QString clean;
          for(QChar const c : subfield(df, 'a')) {
            if(c.isDigit() || c.toUpper() == 'X') {
              clean += c.toUpper();
            }
            else if(!clean.isEmpty()) {
              break;
            }
          }
          if(colman::is_isbn13(clean) &&
             item.get_list_values().count("ISBN13") == 0) {
            item.set_list_value("ISBN13", {clean});
          }
          else if(colman::is_isbn10(clean) &&
                  item.get_list_values().count("ISBN10") == 0) {
            item.set_list_value("ISBN10", {clean});
          }
        }
        else if(tag == "082") {
          QString const ddc = strip_marc(subfield(df, 'a'));
          if(!ddc.isEmpty() && item.get_list_values().count("ddc") == 0) {
            item.set_list_value("ddc", {ddc});
          }
        }
        else if(tag == "245") {
          QString const title = strip_marc(subfield(df, 'a'));
          if(!title.isEmpty()) {
            item.set_name(title);
          }
          QString const resp = strip_marc(subfield(df, 'c'));
          if(!resp.isEmpty()) {
            authors = resp;
          }
        }
        else if(tag == "250") {
          QString const edition = strip_marc(subfield(df, 'a'));
          if(!edition.isEmpty()) {
            item.set_value("edition", edition);
          }
        }
        else if(tag == "264" && df.attribute("ind2") == "1") {
          QString const publisher = strip_marc(subfield(df, 'b'));
          QString const date = strip_marc(subfield(df, 'c'));
          if(!publisher.isEmpty()) {
            item.set_value("publisher", publisher);
          }
          if(!date.isEmpty()) {
            item.set_value("publication_date", colman::normalize_date(date));
          }
        }
        else if(tag == "260") {
          if(item.get_values().count("publisher") == 0) {
            QString const publisher = strip_marc(subfield(df, 'b'));
            if(!publisher.isEmpty()) {
              item.set_value("publisher", publisher);
            }
          }
          if(item.get_values().count("publication_date") == 0) {
            QString const date = strip_marc(subfield(df, 'c'));
            if(!date.isEmpty()) {
              item.set_value("publication_date",
                             colman::normalize_date(date));
            }
          }
        }
        else if(tag == "300") {
          QString const extent = subfield(df, 'a');
          QString pages;
          for(int j = extent.size() - 1; j >= 0; --j) {
            if(extent[j].isDigit()) {
              int end = j;
              while(j > 0 && extent[j - 1].isDigit()) {
                --j;
              }
              pages = extent.mid(j, end - j + 1);
              break;
            }
          }
          if(!pages.isEmpty()) {
            item.set_value("pages", pages);
          }
        }
        else if(tag == "520") {
          QString const summary = strip_marc(subfield(df, 'a'));
          if(!summary.isEmpty() && item.get_description().isEmpty()) {
            item.set_description(summary);
          }
        }
        else if(tag == "655") {
          QString const genre = strip_marc(subfield(df, 'a'));
          if(!genre.isEmpty()) {
            auto const it = item.get_values().find("genre");
            if(it == item.get_values().end()) {
              item.set_value("genre", genre);
            }
            else {
              item.set_value("genre", it->second + "; " + genre);
            }
          }
        }
      }
      df = df.nextSiblingElement();
    }

    if(!authors.isEmpty()) {
      item.set_list_value("authors", {authors});
    }
    return item;
  }

  /// Parses one MARC21 record element into an AV media item.
  /**
    See colman::marc21xml_to_av_item() for the extracted MARC21 fields.
    @param marc  One MARC21 record element.
    @return A colman::Item populated with the extracted AV media data.
  */
  colman::Item marc21_record_to_av_item(QDomElement const& marc)
  {
    colman::Item item;
    QDomElement df = marc.firstChildElement();
    while(!df.isNull()) {
      if(local_name(df) == "leader") {
        // MARC21 leader position 06: type of record.
        // 'g' = projected medium (video), 'j'/'i' = sound recording.
        QString const ldr = df.text();
        if(ldr.size() > 6 &&
           item.get_values().count("media_classification") == 0) {
          QChar const t = ldr[6];
          if(t == 'g') {
            item.set_value("media_classification", "video");
          }
          else if(t == 'j' || t == 'i') {
            item.set_value("media_classification", "audio");
          }
        }
      }
      else if(local_name(df) == "controlfield" &&
              df.attribute("tag") == "007") {
        // Position 0: category of material. 'v' = video, 's' = sound.
        QString const cf = df.text();
        if(!cf.isEmpty() &&
           item.get_values().count("media_classification") == 0) {
          if(cf[0] == 'v') {
            item.set_value("media_classification", "video");
          }
          else if(cf[0] == 's') {
            item.set_value("media_classification", "audio");
          }
        }
      }
      else if(local_name(df) == "datafield") {
        QString const tag = df.attribute("tag");

        if(tag == "028") {
          QString const num = strip_marc(subfield(df, 'a'));
          QString const pub = strip_marc(subfield(df, 'b'));
          if(!num.isEmpty() &&
             item.get_values().count("publisher_number") == 0) {
            item.set_value("publisher_number", num);
          }
          if(!pub.isEmpty() && item.get_values().count("publisher") == 0) {
            item.set_value("publisher", pub);
          }
        }
        else if(tag == "041") {
          QString const lang = strip_marc(subfield(df, 'a'));
          if(!lang.isEmpty() && item.get_values().count("language") == 0) {
            item.set_value("language", lang);
          }
        }
        else if(tag == "100") {
          QString const raw = strip_marc(subfield(df, 'a'));
          if(!raw.isEmpty() && item.get_list_values().count("artist") == 0) {
            item.set_list_value("artist", {natural_name(raw)});
          }
        }
        else if(tag == "245") {
          QString title = strip_marc(subfield(df, 'a'));
          if(!title.isEmpty()) {
            QString part = strip_marc(subfield(df, 'n'));
            int const slash = part.indexOf(" /");
            if(slash >= 0) {
              part = strip_marc(part.left(slash));
            }
            if(!part.isEmpty()) {
              title += ": " + part;
            }
            item.set_name(title);
          }
        }
        else if(tag == "264" && df.attribute("ind2") == "1") {
          QString const pub = strip_marc(subfield(df, 'b'));
          QString const date = strip_marc(subfield(df, 'c'));
          if(!pub.isEmpty() && item.get_values().count("publisher") == 0) {
            item.set_value("publisher", pub);
          }
          if(!date.isEmpty()) {
            item.set_value("publication_date", colman::normalize_date(date));
          }
        }
        else if(tag == "260") {
          if(item.get_values().count("publisher") == 0) {
            QString const pub = strip_marc(subfield(df, 'b'));
            if(!pub.isEmpty()) {
              item.set_value("publisher", pub);
            }
          }
          if(item.get_values().count("publication_date") == 0) {
            QString const date = strip_marc(subfield(df, 'c'));
            if(!date.isEmpty()) {
              item.set_value("publication_date",
                             colman::normalize_date(date));
            }
          }
        }
        else if(tag == "300") {
          QString const extent = strip_marc(subfield(df, 'a'));
          if(!extent.isEmpty() && item.get_values().count("extent") == 0) {
            item.set_value("extent", extent);
          }
        }
        else if(tag == "336") {
          QString const ct = strip_marc(subfield(df, 'a'));
          if(!ct.isEmpty() && item.get_values().count("content_type") == 0) {
            item.set_value("content_type", ct);
          }
        }
        else if(tag == "337") {
          QString const mt = strip_marc(subfield(df, 'a'));
          if(!mt.isEmpty() && item.get_values().count("media_type") == 0) {
            item.set_value("media_type", mt);
          }
        }
        else if(tag == "338") {
          QString const ct = strip_marc(subfield(df, 'a'));
          if(!ct.isEmpty() && item.get_values().count("carrier_type") == 0) {
            item.set_value("carrier_type", ct);
          }
        }
        else if(tag == "500") {
          QString const note = strip_marc(subfield(df, 'a'));
          if(!note.isEmpty() && item.get_description().isEmpty()) {
            item.set_description(note);
          }
        }
        else if(tag == "508") {
          QString const credits = strip_marc(subfield(df, 'a'));
          if(!credits.isEmpty() &&
             item.get_list_values().count("director") == 0) {
            item.set_list_value("director", {credits});
          }
        }
        else if(tag == "511") {
          QString const cast = strip_marc(subfield(df, 'a'));
          if(!cast.isEmpty()) {
            auto const it = item.get_values().find("participants");
            if(it == item.get_values().end()) {
              item.set_value("participants", cast);
            }
            else {
              item.set_value("participants", it->second + "; " + cast);
            }
          }
        }
        else if(tag == "520") {
          QString const summary = strip_marc(subfield(df, 'a'));
          if(!summary.isEmpty() && item.get_description().isEmpty()) {
            item.set_description(summary);
          }
        }
        else if(tag == "521") {
          QString const audience = strip_marc(subfield(df, 'a'));
          if(!audience.isEmpty() &&
             item.get_values().count("target_audience") == 0) {
            item.set_value("target_audience", audience);
          }
        }
        else if(tag == "653") {
          QString const kw = subfield(df, 'a').toLower();
          if(!kw.isEmpty() && item.get_values().count("is_game") == 0) {
            if(kw.contains("spiel") || kw.contains("game")) {
              item.set_value("is_game", "1");
            }
          }
        }
        else if(tag == "700") {
          QString const raw = strip_marc(subfield(df, 'a'));
          if(!raw.isEmpty()) {
            QString const role = strip_marc(subfield(df, 'e')).toLower();
            bool const is_director = role.contains("regisseur");
            bool const is_performer = role.contains("schauspieler") ||
                                      role.contains("darsteller") ||
                                      role.contains("sprecher");
            if(is_director) {
              if(item.get_list_values().count("director") == 0) {
                item.set_list_value("director", {natural_name(raw)});
              }
              else {
                item.append_list_value("director", natural_name(raw));
              }
            }
            else if(is_performer) {
              auto const it = item.get_values().find("participants");
              if(it == item.get_values().end()) {
                item.set_value("participants", natural_name(raw));
              }
              else {
                item.set_value("participants",
                               it->second + "; " + natural_name(raw));
              }
            }
            else {
              if(item.get_list_values().count("artist") == 0) {
                item.set_list_value("artist", {natural_name(raw)});
              }
              else {
                item.append_list_value("artist", natural_name(raw));
              }
            }
          }
        }
      }
      df = df.nextSiblingElement();
    }

    return item;
  }

  /// Parses one UNIMARC record element into a book item.
  /**
    See colman::unimarcxml_to_book_item() for the extracted UNIMARC fields.
    @param marc  One UNIMARC record element.
    @return A colman::Item populated with the extracted book data.
  */
  colman::Item unimarc_record_to_book_item(QDomElement const& marc)
  {
    colman::Item item;
    QStringList authors;
    QDomElement df = marc.firstChildElement();
    while(!df.isNull()) {
      if(local_name(df) == "datafield") {
        QString const tag = df.attribute("tag");

        if(tag == "010") {
          QString raw_isbn = subfield(df, 'a');
          QString clean;
          for(QChar const c : raw_isbn) {
            if(c.isDigit() || c.toUpper() == 'X') {
              clean += c.toUpper();
            }
          }
          if(colman::is_isbn13(clean) &&
             item.get_list_values().count("ISBN13") == 0) {
            item.set_list_value("ISBN13", {clean});
          }
          else if(colman::is_isbn10(clean) &&
                  item.get_list_values().count("ISBN10") == 0) {
            item.set_list_value("ISBN10", {clean});
          }
        }
        else if(tag == "200") {
          QString const title = strip_marc(subfield(df, 'a'));
          if(!title.isEmpty()) {
            item.set_name(title);
          }
          QString const author_stmt = strip_marc(subfield(df, 'f'));
          if(!author_stmt.isEmpty()) {
            authors.append(author_stmt);
          }
        }
        else if(tag == "210") {
          QString const publisher = strip_marc(subfield(df, 'c'));
          QString const date = strip_marc(subfield(df, 'd'));
          if(!publisher.isEmpty() &&
             item.get_values().count("publisher") == 0) {
            item.set_value("publisher", publisher);
          }
          if(!date.isEmpty() &&
             item.get_values().count("publication_date") == 0) {
            item.set_value("publication_date", colman::normalize_date(date));
          }
        }
        else if(tag == "215") {
          QString const extent = subfield(df, 'a');
          QString pages;
          for(int j = extent.size() - 1; j >= 0; --j) {
            if(extent[j].isDigit()) {
              int end = j;
              while(j > 0 && extent[j - 1].isDigit()) {
                --j;
              }
              pages = extent.mid(j, end - j + 1);
              break;
            }
          }
          if(!pages.isEmpty() && item.get_values().count("pages") == 0) {
            item.set_value("pages", pages);
          }
        }
        else if(tag == "330") {
          QString const summary = strip_marc(subfield(df, 'a'));
          if(!summary.isEmpty() && item.get_description().isEmpty()) {
            item.set_description(summary);
          }
        }
        else if(tag == "606") {
          QString const subject = strip_marc(subfield(df, 'a'));
          if(!subject.isEmpty()) {
            auto const it = item.get_values().find("genre");
            if(it == item.get_values().end()) {
              item.set_value("genre", subject);
            }
            else {
              item.set_value("genre", it->second + "; " + subject);
            }
          }
        }
        else if(tag == "700" || tag == "701") {
          QString const family = strip_marc(subfield(df, 'a'));
          QString const forename = strip_marc(subfield(df, 'b'));
          if(!family.isEmpty()) {
            QString name;
            if(!forename.isEmpty()) {
              name = forename + " " + family;
            }
            else {
              name = family;
            }
            authors.append(name);
          }
        }
      }
      df = df.nextSiblingElement();
    }

    if(!authors.isEmpty()) {
      item.set_list_value("authors", authors);
    }
    return item;
  }

} // namespace


/// Returns true when the record schema name contains "unimarc" (case-insensitive).
bool colman::SruServer::is_unimarc() const
{
  return record_schema.contains("unimarc", Qt::CaseInsensitive);
}


/// Returns the built-in list of pre-configured SRU servers.
/**
  Only the DNB server is enabled by default.

  @return A list of six pre-configured SruServer objects.
*/
QVector<colman::SruServer> colman::default_sru_servers()
{
  QVector<SruServer> servers;

  SruServer dnb;
  dnb.name = "Deutsche Nationalbibliothek (DNB)";
  dnb.endpoint = "https://services.dnb.de/sru/dnb";
  dnb.isbn_field = "num";
  dnb.ean_field = "";
  dnb.title_field = "tit";
  dnb.record_schema = "MARC21-xml";
  dnb.enabled = true;
  servers.append(dnb);

  SruServer k10plus;
  k10plus.name = "K10plus";
  k10plus.endpoint = "https://sru.k10plus.de/opac-de-627";
  k10plus.isbn_field = "pica.isb";
  k10plus.ean_field = "pica.num";
  k10plus.title_field = "pica.tit";
  k10plus.record_schema = "marcxml";
  k10plus.enabled = true;
  servers.append(k10plus);

  SruServer loc;
  loc.name = "Library of Congress";
  loc.endpoint = "https://lx2.loc.gov/sru/lcdb";
  loc.isbn_field = "bath.isbn";
  loc.ean_field = "";
  loc.title_field = "dc.title";
  loc.record_schema = "marcxml";
  loc.enabled = true;
  servers.append(loc);

  SruServer oenb;
  oenb.name = "Österreichische Nationalbibliothek (ÖNB)";
  oenb.endpoint =
      "https://obv-at-oenb.alma.exlibrisgroup.com/view/sru/43ACC_ONB";
  oenb.isbn_field = "alma.isbn";
  oenb.ean_field = "";
  oenb.title_field = "alma.title";
  oenb.record_schema = "marcxml";
  oenb.enabled = false;
  servers.append(oenb);

  SruServer swisscovery;
  swisscovery.name = "Swisscovery";
  swisscovery.endpoint = "https://swisscovery.slsp.ch/view/sru/41SLSP_NETWORK";
  swisscovery.isbn_field = "isbn";
  swisscovery.ean_field = "";
  swisscovery.title_field = "alma.title";
  swisscovery.record_schema = "marcxml";
  swisscovery.enabled = false;
  servers.append(swisscovery);

  SruServer bnf;
  bnf.name = "Bibliothèque nationale de France (BnF)";
  bnf.endpoint = "http://catalogue.bnf.fr/api/SRU";
  bnf.isbn_field = "bib.isbn";
  bnf.ean_field = "";
  bnf.title_field = "bib.title";
  bnf.record_schema = "unimarcxchange";
  bnf.enabled = false;
  servers.append(bnf);

  return servers;
}


/// Loads the SRU server list from QSettings.
/**
  Falls back to default_sru_servers() when the "sru_servers" key is absent
  or the stored JSON cannot be parsed.

  @return The list of configured SruServer objects.
*/
QVector<colman::SruServer> colman::load_sru_servers()
{
  QSettings settings;
  if(!settings.contains("sru_servers")) {
    return default_sru_servers();
  }
  auto const json = settings.value("sru_servers").toByteArray();
  QJsonParseError err;
  auto const doc = QJsonDocument::fromJson(json, &err);
  if(!doc.isArray()) {
    return default_sru_servers();
  }
  QVector<SruServer> result;
  for(auto const& val : doc.array()) {
    auto const obj = val.toObject();
    SruServer srv;
    srv.name = obj["name"].toString();
    srv.endpoint = obj["endpoint"].toString();
    srv.isbn_field = obj["isbn_field"].toString();
    srv.ean_field = obj["ean_field"].toString();
    if(obj.contains("title_field")) {
      srv.title_field = obj["title_field"].toString();
    }
    else {
      srv.title_field = default_title_field_for(srv.endpoint);
    }
    srv.record_schema = obj["record_schema"].toString();
    srv.enabled = obj["enabled"].toBool();
    result.append(srv);
  }
  return result;
}


/// Saves \a servers to QSettings as compact JSON.
/**
  @param servers  The list of SruServer objects to persist.
*/
void colman::save_sru_servers(QVector<SruServer> const& servers)
{
  QJsonArray arr;
  for(auto const& srv : servers) {
    QJsonObject obj;
    obj["name"] = srv.name;
    obj["endpoint"] = srv.endpoint;
    obj["isbn_field"] = srv.isbn_field;
    obj["ean_field"] = srv.ean_field;
    obj["title_field"] = srv.title_field;
    obj["record_schema"] = srv.record_schema;
    obj["enabled"] = srv.enabled;
    arr.append(obj);
  }
  QSettings settings;
  settings.setValue("sru_servers",
                    QJsonDocument(arr).toJson(QJsonDocument::Compact));
}


/// Performs a synchronous HTTP GET and returns the response body.
/**
  Returns an empty QByteArray on network error or timeout.
  The timeout is read from QSettings key "timeout" (default: 5000 ms).
  An optional \a user_agent string is sent as the User-Agent header.

  @param url        URL to retrieve.
  @param user_agent Optional User-Agent header value; empty = Qt default.
  @return Response body, or an empty QByteArray on failure.
*/
QByteArray colman::synchronous_get(QString const& url,
                                   QString const& user_agent)
{
  QNetworkAccessManager http;
  QEventLoop loop;
  QSettings settings;
  int const timeout = settings.value("timeout", 5000).toInt();
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
  QUrl const qurl(url);
  QNetworkRequest request(qurl);
  request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
  if(!user_agent.isEmpty()) {
    request.setHeader(QNetworkRequest::UserAgentHeader, user_agent);
  }
  QNetworkReply* const reply = http.get(request);
  if(!reply) {
    return QByteArray();
  }
  loop.connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  timer.start(timeout);
  loop.exec();
  QByteArray result;
  if(reply->isFinished() && reply->error() == QNetworkReply::NoError) {
    result = reply->readAll();
  }
  else if(!reply->isFinished()) {
    qCDebug(colman_lookup_log) << "[HTTP] timeout:" << url;
  }
  else {
    qCDebug(colman_lookup_log) << "[HTTP] error" << reply->error()
                               << reply->errorString() << url;
  }
  reply->deleteLater();
  return result;
}


/// Queries \a server for \a number in \a field and returns the raw response.
/**
  @param server  The SRU server to query.
  @param field   CQL index field name (e.g. "num", "bath.isbn").
  @param number  The value to search for (e.g. ISBN or EAN string).
  @return Raw SRU response bytes, or empty on failure.
*/
QByteArray colman::query_sru(SruServer const& server,
                             QString const& field,
                             QString const& number)
{
  return synchronous_get(
      server.endpoint +
      "?version=1.1&operation=searchRetrieve"
      "&maximumRecords=1"
      "&query=" +
      field + "%3D" + number + "&recordSchema=" + server.record_schema);
}


/// Queries \a server for \a title in its title index (multi-record).
/**
  Sends the title as a percent-encoded, quoted CQL phrase so that spaces
  and non-ASCII characters cannot break the CQL query.  UNIMARC servers
  (BnF) reject SRU version 1.1 and are queried with version 1.2; all
  other servers (e.g. DNB) only accept version 1.1.

  @param server       The SRU server to query (uses its title_field).
  @param title        The title string to search for.
  @param max_records  Maximum number of records to request.
  @return Raw SRU response bytes, or empty on failure.
*/
QByteArray colman::query_sru_title(SruServer const& server,
                                   QString const& title,
                                   int max_records)
{
  if(server.title_field.isEmpty() || title.trimmed().isEmpty()) {
    return QByteArray();
  }
  QString const version = server.is_unimarc() ? "1.2" : "1.1";
  return synchronous_get(
      server.endpoint + "?version=" + version +
      "&operation=searchRetrieve"
      "&maximumRecords=" +
      QString::number(max_records) + "&query=" + server.title_field +
      "%3D%22" + QUrl::toPercentEncoding(title.trimmed()) +
      "%22&recordSchema=" + server.record_schema);
}


/// Normalises a raw publication-date string to a form accepted by the date
/// validator: "yyyy", "M.yyyy", or "d.M.yyyy".
/**
  Handles ISO partial dates, common English long-form dates, and MARC21 year
  annotations such as "c2020.", "[2020]", "©2020".

  @param raw  The raw date string from an API response.
  @return A normalised date string, or an empty string if no date could be
          extracted.
*/
QString colman::normalize_date(QString const& raw)
{
  QString const s = raw.trimmed();
  if(s.isEmpty()) {
    return {};
  }

  // ISO-like: starts with yyyy optionally followed by -M[M] and -d[d].
  {
    QRegularExpression const iso(
        "^(\\d{4})(?:-(\\d{1,2})(?:-(\\d{1,2}))?)?$");
    QRegularExpressionMatch const m = iso.match(s);
    if(m.hasMatch()) {
      QString const year = m.captured(1);
      QString month = m.captured(2);
      QString day = m.captured(3);
      if(month.isEmpty()) {
        return year;
      }
      if(month.size() > 1 && month[0] == '0') {
        month = month.mid(1);
      }
      if(day.isEmpty()) {
        return month + "." + year;
      }
      if(day.size() > 1 && day[0] == '0') {
        day = day.mid(1);
      }
      return day + "." + month + "." + year;
    }
  }

  // Common English long-form dates (OpenLibrary).
  QLocale const en_us("en_US");
  static QString const fmts[] = {"MMMM d, yyyy", "d MMMM yyyy",
                                 "MMM d, yyyy", "d MMM yyyy",
                                 "MMMM yyyy", "MMM yyyy"};
  for(QString const& fmt : fmts) {
    QDate const d = en_us.toDate(s, fmt);
    if(d.isValid()) {
      if(fmt == "MMMM yyyy" || fmt == "MMM yyyy") {
        return QString::number(d.month()) + "." + QString::number(d.year());
      }
      return QString("%1.%2.%3")
          .arg(d.day())
          .arg(d.month())
          .arg(d.year());
    }
  }

  // Last resort: extract the first four-digit sequence.
  QRegularExpression const year_only("(\\d{4})");
  QRegularExpressionMatch const m = year_only.match(s);
  if(m.hasMatch()) {
    return m.captured(1);
  }

  return {};
}


/// Parses a MARC21-xml SRU response and returns book item data.
/**
  Extracts title, authors, publisher, publication date, edition, ISBNs, and
  page count from the first MARC21 record in the SRU MARC21-xml response.

  Relevant MARC21 fields:
  - 020 $a  ISBN
  - 082 $a  Dewey Decimal Classification number
  - 245 $a  Title / $c Statement of responsibility (authors)
  - 250 $a  Edition statement
  - 264 ind2=1: $b Publisher / $c Date
  - 260 $b  Publisher / $c Date (older records, fallback)
  - 300 $a  Physical description (page count)
  - 520 $a  Summary / abstract
  - 655 $a  Genre/Form term

  @param data  Raw SRU/MARC21-xml bytes.
  @param isbn  The ISBN that was queried; used as fallback.
  @return A colman::Item populated with the extracted book data.
*/
colman::Item colman::marc21xml_to_book_item(QByteArray const& data,
                                            QString const& isbn)
{
  Item item;
  QDomDocument doc;
  auto const records = sru_marc_records(data, doc);
  if(records.isEmpty()) {
    return item;
  }
  item = marc21_record_to_book_item(records.first());
  if(item.get_list_values().count("ISBN13") == 0 &&
     item.get_list_values().count("ISBN10") == 0 && !isbn.isEmpty()) {
    fill_isbn(isbn, item);
  }
  return item;
}


/// Parses a MARC21-xml SRU response and returns AV media item data.
/**
  Extracts title, director, cast, artist, publisher, publication date,
  language, extent, content/media/carrier type, target audience, and
  description from the first MARC21 record in the SRU MARC21-xml response.

  Relevant MARC21 fields:
  - 028 $a  Publisher/catalogue number; $b Publisher/label name
  - 041 $a  Language code
  - 100 $a  Primary personal name
  - 245 $a  Title; $n Part number (stripped at first " /")
  - 264 ind2=1: $b Publisher / $c Date
  - 260 $b  Publisher / $c Date (older records, fallback)
  - 300 $a  Physical description (extent / running time)
  - 336 $a  Content type (RDA)
  - 337 $a  Media type (RDA)
  - 338 $a  Carrier type (RDA)
  - 500 $a  General note (description fallback)
  - 508 $a  Creation/production credits (director fallback)
  - 511 $a  Cast / performers note (participants fallback)
  - 520 $a  Summary / abstract
  - 521 $a  Target audience (e.g. FSK rating)
  - 653 $a  Subject keywords (game detection)
  - 700 $a+$e  Added personal name; $e role: "regisseur" → director,
               "schauspieler"/"darsteller"/"sprecher" → participants,
               others → artist

  @param data  Raw SRU/MARC21-xml bytes.
  @return A colman::Item populated with the extracted AV media data.
*/
colman::Item colman::marc21xml_to_av_item(QByteArray const& data)
{
  QDomDocument doc;
  auto const records = sru_marc_records(data, doc);
  if(records.isEmpty()) {
    return Item();
  }
  return marc21_record_to_av_item(records.first());
}


/// Parses a UNIMARC-xml SRU response (BnF) and returns book item data.
/**
  Extracts title, authors, publisher, publication date, pages, summary,
  and genre from the first UNIMARC record in the SRU response.

  Relevant UNIMARC fields:
  - 010 $a  ISBN (hyphens stripped)
  - 200 $a  Title, $f Author statement
  - 210 $c  Publisher, $d Date
  - 215 $a  Physical description (rightmost number = page count)
  - 330 $a  Summary / abstract
  - 606 $a  Subject / genre (semicolon-separated if multiple)
  - 700 $a (family) + $b (forename)  Primary personal name
  - 701 $a + $b  Secondary authors (same format)

  @param data  Raw SRU/UNIMARC-xml bytes.
  @return A colman::Item populated with the extracted book data.
*/
colman::Item colman::unimarcxml_to_book_item(QByteArray const& data)
{
  QDomDocument doc;
  auto const records = sru_marc_records(data, doc);
  if(records.isEmpty()) {
    return Item();
  }
  return unimarc_record_to_book_item(records.first());
}


/// Dispatches to the correct book-item parser based on the server schema.
/**
  @param server  The SRU server that produced \a data.
  @param data    Raw SRU response bytes.
  @param isbn    The ISBN that was queried.
  @return A colman::Item populated with the extracted book data.
*/
colman::Item colman::sru_to_book_item(SruServer const& server,
                                      QByteArray const& data,
                                      QString const& isbn)
{
  if(server.is_unimarc()) {
    return unimarcxml_to_book_item(data);
  }
  return marc21xml_to_book_item(data, isbn);
}


/// Dispatches to the correct AV-media parser based on the server schema.
/**
  @param server  The SRU server that produced \a data.
  @param data    Raw SRU response bytes.
  @return A colman::Item populated with the extracted AV media data.
*/
colman::Item colman::sru_to_av_item(SruServer const& server,
                                    QByteArray const& data)
{
  (void)server;
  return marc21xml_to_av_item(data);
}


/// Parses all records of a MARC21-xml SRU response into book items.
/**
  Multi-record variant of marc21xml_to_book_item(); no ISBN fallback is
  applied.

  @param data  Raw SRU/MARC21-xml bytes.
  @return One colman::Item per MARC record in the response.
*/
QVector<colman::Item> colman::marc21xml_to_book_items(QByteArray const& data)
{
  QVector<Item> items;
  QDomDocument doc;
  for(auto const& marc : sru_marc_records(data, doc)) {
    items.append(marc21_record_to_book_item(marc));
  }
  return items;
}


/// Parses all records of a MARC21-xml SRU response into AV media items.
/**
  Multi-record variant of marc21xml_to_av_item().

  @param data  Raw SRU/MARC21-xml bytes.
  @return One colman::Item per MARC record in the response.
*/
QVector<colman::Item> colman::marc21xml_to_av_items(QByteArray const& data)
{
  QVector<Item> items;
  QDomDocument doc;
  for(auto const& marc : sru_marc_records(data, doc)) {
    items.append(marc21_record_to_av_item(marc));
  }
  return items;
}


/// Parses all records of a UNIMARC-xml SRU response into book items.
/**
  Multi-record variant of unimarcxml_to_book_item().

  @param data  Raw SRU/UNIMARC-xml bytes.
  @return One colman::Item per UNIMARC record in the response.
*/
QVector<colman::Item> colman::unimarcxml_to_book_items(QByteArray const& data)
{
  QVector<Item> items;
  QDomDocument doc;
  for(auto const& marc : sru_marc_records(data, doc)) {
    items.append(unimarc_record_to_book_item(marc));
  }
  return items;
}


/// Parses a multi-record SRU title response, filtered by media kind.
/**
  Keeps only records whose detected media kind matches \a kind; records
  whose kind cannot be detected are kept as well, and generic video
  records match both MediaKind::Dvd and MediaKind::BluRay requests.
  Records without a title are dropped.  For UNIMARC servers only
  MediaKind::Book is supported; other kinds yield an empty result.

  @param server  The SRU server that produced \a data.
  @param data    Raw SRU response bytes.
  @param kind    The media kind requested by the title search.
  @return One colman::Item per matching record.
*/
QVector<colman::Item> colman::sru_title_to_items(SruServer const& server,
                                                 QByteArray const& data,
                                                 MediaKind kind)
{
  QVector<Item> items;
  QDomDocument doc;
  if(server.is_unimarc()) {
    if(kind == MediaKind::Book) {
      for(auto const& marc : sru_marc_records(data, doc)) {
        Item item = unimarc_record_to_book_item(marc);
        if(!item.get_name().isEmpty()) {
          items.append(item);
        }
      }
    }
    return items;
  }
  for(auto const& marc : sru_marc_records(data, doc)) {
    if(!kind_matches(detect_marc21_kind(marc), kind)) {
      continue;
    }
    Item item;
    if(kind == MediaKind::Book) {
      item = marc21_record_to_book_item(marc);
    }
    else {
      item = marc21_record_to_av_item(marc);
    }
    if(!item.get_name().isEmpty()) {
      items.append(item);
    }
  }
  return items;
}

/* EOF */
