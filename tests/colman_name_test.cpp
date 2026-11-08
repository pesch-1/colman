// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for the multi-source title (name) search helpers.
/**
   Covers the search-response parsers for Open Library, Google Books,
   and MusicBrainz, the multi-record SRU parsing with media-kind
   filtering, the SRU server settings migration, and the NameQueries
   plumbing.  No network access.
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_NAME_QUERIES_HPP
#  include "colman_name_queries.hpp"
#  define COLMAN_NAME_QUERIES_HPP
#endif
#ifndef COLMAN_EAN_QUERIES_HPP
#  include "colman_ean_queries.hpp"
#  define COLMAN_EAN_QUERIES_HPP
#endif
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
#  define COLMAN_ISBN_QUERIES_HPP
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif

namespace
{
  /// Returns the names of all items in \a items.
  QStringList names_of(QVector<colman::Item> const& items)
  {
    QStringList names;
    for(auto const& item : items) {
      names += item.get_name();
    }
    return names;
  }

  /// A six-record MARC21 SRU response with one record per media kind.
  QByteArray marc21_mixed_fixture()
  {
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<searchRetrieveResponse>"
        "<numberOfRecords>7</numberOfRecords>"
        "<records>"

        "<record><recordData><record>"
        "<leader>00000cam a2200000 c 4500</leader>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title Book</subfield>"
        "<subfield code=\"c\">Some Author</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "<record><recordData><record>"
        "<leader>00000cjm a2200000 c 4500</leader>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title CD</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "<record><recordData><record>"
        "<leader>00000cgm a2200000 c 4500</leader>"
        "<controlfield tag=\"007\">vd cvaizq</controlfield>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title DVD</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "<record><recordData><record>"
        "<leader>00000cgm a2200000 c 4500</leader>"
        "<controlfield tag=\"007\">vd csaizs</controlfield>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title BluRay</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "<record><recordData><record>"
        "<leader>00000cgm a2200000 c 4500</leader>"
        "<datafield tag=\"300\">"
        "<subfield code=\"a\">1 Blu-ray Disc (90 min)</subfield>"
        "</datafield>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title BluRayText</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "<record><recordData><record>"
        "<leader>00000cgm a2200000 c 4500</leader>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title Video</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "<record><recordData><record>"
        "<datafield tag=\"245\">"
        "<subfield code=\"a\">Title Unknown</subfield>"
        "</datafield>"
        "</record></recordData></record>"

        "</records>"
        "</searchRetrieveResponse>");
  }

  /// Returns a MARC21 SRU server for filter tests.
  colman::SruServer marc21_server()
  {
    colman::SruServer srv;
    srv.name = "Test Server";
    srv.endpoint = "https://example.org/sru";
    srv.title_field = "tit";
    srv.record_schema = "MARC21-xml";
    srv.enabled = true;
    return srv;
  }

  /// A two-record UNIMARC SRU response (BnF style).
  QByteArray unimarc_fixture()
  {
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<searchRetrieveResponse>"
        "<records>"
        "<record><recordData><record>"
        "<datafield tag=\"200\">"
        "<subfield code=\"a\">Titre Un</subfield>"
        "</datafield>"
        "<datafield tag=\"700\">"
        "<subfield code=\"a\">Hugo</subfield>"
        "<subfield code=\"b\">Victor</subfield>"
        "</datafield>"
        "</record></recordData></record>"
        "<record><recordData><record>"
        "<datafield tag=\"200\">"
        "<subfield code=\"a\">Titre Deux</subfield>"
        "</datafield>"
        "</record></recordData></record>"
        "</records>"
        "</searchRetrieveResponse>");
  }

  /// Returns a UNIMARC SRU server (BnF style) for dispatch tests.
  colman::SruServer unimarc_server()
  {
    colman::SruServer srv = marc21_server();
    srv.record_schema = "unimarcxchange";
    return srv;
  }

} // namespace


namespace colman
{

  class ColmanNameTest : public QObject
  {
    Q_OBJECT

  private Q_SLOTS:

    void initTestCase()
    {
      // Isolate QSettings from the real user configuration; without an
      // organization name QSettings cannot store anything on Windows.
      QCoreApplication::setOrganizationName("colman_name_test");
      QCoreApplication::setApplicationName("colman_name_test");
      QSettings().clear();
    }

    void cleanupTestCase()
    {
      QSettings().clear();
      QSettings().sync();
    }

    // -------------------------------------------------------------------
    // openlibrary_search_to_results

    void test_openlibrary_search_normal()
    {
      QByteArray const json =
          "{\"numFound\":2,\"docs\":["
          "{\"title\":\"Der Hobbit\","
          "\"author_name\":[\"J. R. R. Tolkien\"],"
          "\"first_publish_year\":1937,"
          "\"cover_i\":12345,"
          "\"publisher\":[\"Klett\",\"Other\"]},"
          "{\"title\":\"\"},"
          "{\"author_name\":[\"No Title\"]}"
          "]}";
      auto const results = openlibrary_search_to_results(json);
      QCOMPARE(results.size(), 1);
      QCOMPARE(results[0].name_, QString("Der Hobbit"));
      QCOMPARE(results[0].year_, 1937);
      QCOMPARE(results[0].source_, QString("Open Library"));
      QCOMPARE(results[0].cover_url_,
               QString("https://covers.openlibrary.org/b/id/12345-M.jpg"));
      Item const& item = results[0].item_;
      QCOMPARE(item.get_list_values().at("authors").first(),
               QString("J. R. R. Tolkien"));
      QCOMPARE(item.get_values().at("publisher"), QString("Klett"));
      QCOMPARE(item.get_values().at("publication_date"), QString("1937"));
      // Deliberately no ISBN: search docs mix all editions.
      QVERIFY(item.get_list_values().count("ISBN13") == 0);
      QVERIFY(item.get_list_values().count("ISBN10") == 0);
    }

    void test_openlibrary_search_empty_and_invalid()
    {
      QVERIFY(openlibrary_search_to_results(QByteArray()).isEmpty());
      QVERIFY(openlibrary_search_to_results("not json").isEmpty());
      QVERIFY(openlibrary_search_to_results("{\"docs\":[]}").isEmpty());
    }

    // -------------------------------------------------------------------
    // googleapis_search_to_results

    void test_googleapis_search_normal()
    {
      QByteArray const json =
          "{\"items\":["
          "{\"volumeInfo\":{\"title\":\"Buch Eins\","
          "\"authors\":[\"A1\"],"
          "\"industryIdentifiers\":["
          "{\"type\":\"ISBN_13\",\"identifier\":\"9780596007126\"},"
          "{\"type\":\"ISBN_10\",\"identifier\":\"0596007124\"}],"
          "\"publishedDate\":\"2020-05-01\","
          "\"publisher\":\"Verlag\","
          "\"pageCount\":300,"
          "\"imageLinks\":{\"thumbnail\":\"http://img/1.jpg\"}}},"
          "{\"volumeInfo\":{\"title\":\"Buch Zwei\"}},"
          "{\"volumeInfo\":{}}"
          "]}";
      auto const results = googleapis_search_to_results(json);
      QCOMPARE(results.size(), 2);
      QCOMPARE(results[0].name_, QString("Buch Eins"));
      QCOMPARE(results[0].year_, 2020);
      QCOMPARE(results[0].source_, QString("Google Books"));
      QCOMPARE(results[0].cover_url_, QString("http://img/1.jpg"));
      QCOMPARE(results[0].item_.get_list_values().at("ISBN13").first(),
               QString("9780596007126"));
      QCOMPARE(results[0].item_.get_list_values().at("ISBN10").first(),
               QString("0596007124"));
      QCOMPARE(results[1].name_, QString("Buch Zwei"));
      QCOMPARE(results[1].year_, 0);
    }

    void test_googleapis_search_empty_and_invalid()
    {
      QVERIFY(googleapis_search_to_results(QByteArray()).isEmpty());
      QVERIFY(googleapis_search_to_results("not json").isEmpty());
    }

    /// Regression: googleapis_to_item still matches volumes by ISBN.
    void test_googleapis_to_item_still_matches_isbn()
    {
      QByteArray const json =
          "{\"items\":["
          "{\"volumeInfo\":{\"title\":\"Falsches Buch\","
          "\"industryIdentifiers\":["
          "{\"type\":\"ISBN_13\",\"identifier\":\"9780201616224\"}]}},"
          "{\"volumeInfo\":{\"title\":\"Richtiges Buch\","
          "\"industryIdentifiers\":["
          "{\"type\":\"ISBN_13\",\"identifier\":\"9780596007126\"}],"
          "\"imageLinks\":{\"thumbnail\":\"http://img/2.jpg\"}}}"
          "]}";
      QString thumbnail;
      Item const item =
          googleapis_to_item("9780596007126", json, thumbnail);
      QCOMPARE(item.get_name(), QString("Richtiges Buch"));
      QCOMPARE(thumbnail, QString("http://img/2.jpg"));
      QString other_thumbnail;
      Item const none =
          googleapis_to_item("9999999999999", json, other_thumbnail);
      QVERIFY(none.get_name().isEmpty());
      QVERIFY(other_thumbnail.isEmpty());
    }

    // -------------------------------------------------------------------
    // musicbrainz_search_to_results

    void test_musicbrainz_search_normal()
    {
      QByteArray const json =
          "{\"releases\":["
          "{\"id\":\"mbid-1\",\"title\":\"Album Eins\","
          "\"artist-credit\":["
          "{\"name\":\"Artist A\",\"joinphrase\":\" & \"},"
          "{\"name\":\"Artist B\"}],"
          "\"label-info\":[{\"label\":{\"name\":\"Label X\"},"
          "\"catalog-number\":\"LX-1\"}],"
          "\"date\":\"2001-09-10\"},"
          "{\"title\":\"\"}"
          "]}";
      auto const results = musicbrainz_search_to_results(json);
      QCOMPARE(results.size(), 1);
      QCOMPARE(results[0].name_, QString("Album Eins"));
      QCOMPARE(results[0].year_, 2001);
      QCOMPARE(results[0].source_, QString("MusicBrainz"));
      QCOMPARE(results[0].cover_url_,
               QString("https://coverartarchive.org/release/mbid-1/"
                       "front-250"));
      Item const& item = results[0].item_;
      QCOMPARE(item.get_list_values().at("artist").first(),
               QString("Artist A & Artist B"));
      QCOMPARE(item.get_values().at("publisher"), QString("Label X"));
      QCOMPARE(item.get_values().at("publisher_number"), QString("LX-1"));
    }

    /// Regression: musicbrainz_to_item still returns the first release.
    void test_musicbrainz_to_item_uses_first_release()
    {
      QByteArray const json =
          "{\"releases\":["
          "{\"id\":\"mbid-1\",\"title\":\"Erstes Album\"},"
          "{\"id\":\"mbid-2\",\"title\":\"Zweites Album\"}"
          "]}";
      QString cover;
      Item const item = musicbrainz_to_item(json, cover);
      QCOMPARE(item.get_name(), QString("Erstes Album"));
      QCOMPARE(cover,
               QString("https://coverartarchive.org/release/mbid-1/"
                       "front-250"));
    }

    // -------------------------------------------------------------------
    // Multi-record SRU parsing

    void test_marc21xml_to_book_items_parses_all_records()
    {
      auto const items = marc21xml_to_book_items(marc21_mixed_fixture());
      QCOMPARE(items.size(), 7);
      QCOMPARE(items[0].get_name(), QString("Title Book"));
      QCOMPARE(items[0].get_list_values().at("authors").first(),
               QString("Some Author"));
      QCOMPARE(items[6].get_name(), QString("Title Unknown"));
    }

    void test_marc21xml_to_av_items_parses_all_records()
    {
      auto const items = marc21xml_to_av_items(marc21_mixed_fixture());
      QCOMPARE(items.size(), 7);
      QCOMPARE(items[2].get_name(), QString("Title DVD"));
      QCOMPARE(items[2].get_values().at("media_classification"),
               QString("video"));
    }

    /// Regression: the single-record wrappers still use the first record.
    void test_single_record_wrappers_unchanged()
    {
      Item const book =
          marc21xml_to_book_item(marc21_mixed_fixture(), "9780596007126");
      QCOMPARE(book.get_name(), QString("Title Book"));
      // ISBN fallback is still applied when the record has no ISBN.
      QCOMPARE(book.get_list_values().at("ISBN13").first(),
               QString("9780596007126"));
      Item const av = marc21xml_to_av_item(marc21_mixed_fixture());
      QCOMPARE(av.get_name(), QString("Title Book"));
      QVERIFY(marc21xml_to_book_item(QByteArray(), "9780596007126")
                  .get_name()
                  .isEmpty());
    }

    void test_unimarcxml_to_book_items_parses_all_records()
    {
      auto const items = unimarcxml_to_book_items(unimarc_fixture());
      QCOMPARE(items.size(), 2);
      QCOMPARE(items[0].get_name(), QString("Titre Un"));
      QCOMPARE(items[0].get_list_values().at("authors").first(),
               QString("Victor Hugo"));
      QCOMPARE(items[1].get_name(), QString("Titre Deux"));
    }

    // -------------------------------------------------------------------
    // sru_title_to_items — media kind filtering

    void test_sru_title_filter_book()
    {
      auto const items = sru_title_to_items(
          marc21_server(), marc21_mixed_fixture(), MediaKind::Book);
      QCOMPARE(names_of(items),
               QStringList({"Title Book", "Title Unknown"}));
    }

    void test_sru_title_filter_cd()
    {
      auto const items = sru_title_to_items(
          marc21_server(), marc21_mixed_fixture(), MediaKind::Cd);
      QCOMPARE(names_of(items), QStringList({"Title CD", "Title Unknown"}));
    }

    void test_sru_title_filter_dvd()
    {
      // The explicit Blu-ray records are excluded; the generic video
      // record matches both DVD and Blu-ray requests.
      auto const items = sru_title_to_items(
          marc21_server(), marc21_mixed_fixture(), MediaKind::Dvd);
      QCOMPARE(names_of(items),
               QStringList({"Title DVD", "Title Video", "Title Unknown"}));
    }

    void test_sru_title_filter_bluray()
    {
      auto const items = sru_title_to_items(
          marc21_server(), marc21_mixed_fixture(), MediaKind::BluRay);
      QCOMPARE(names_of(items),
               QStringList({"Title BluRay", "Title BluRayText",
                            "Title Video", "Title Unknown"}));
    }

    void test_sru_title_unimarc_book_only()
    {
      auto const book_items = sru_title_to_items(
          unimarc_server(), unimarc_fixture(), MediaKind::Book);
      QCOMPARE(names_of(book_items),
               QStringList({"Titre Un", "Titre Deux"}));
      QVERIFY(sru_title_to_items(unimarc_server(), unimarc_fixture(),
                                 MediaKind::Cd)
                  .isEmpty());
    }

    // -------------------------------------------------------------------
    // query_sru_title — offline guards

    void test_query_sru_title_without_title_field_is_empty()
    {
      SruServer srv = marc21_server();
      srv.title_field = "";
      QVERIFY(query_sru_title(srv, "Some Title", 10).isEmpty());
      QVERIFY(query_sru_title(marc21_server(), "  ", 10).isEmpty());
    }

    // -------------------------------------------------------------------
    // SRU server settings migration

    void test_load_sru_servers_migrates_title_field()
    {
      QSettings settings;
      settings.setValue(
          "sru_servers",
          QByteArray("[{\"name\":\"DNB\","
                     "\"endpoint\":\"https://services.dnb.de/sru/dnb\","
                     "\"isbn_field\":\"num\",\"ean_field\":\"\","
                     "\"record_schema\":\"MARC21-xml\",\"enabled\":true},"
                     "{\"name\":\"Custom\","
                     "\"endpoint\":\"https://example.org/sru\","
                     "\"isbn_field\":\"isbn\",\"ean_field\":\"\","
                     "\"record_schema\":\"marcxml\",\"enabled\":true}]"));
      auto const servers = load_sru_servers();
      settings.remove("sru_servers");
      QCOMPARE(servers.size(), 2);
      // Known endpoint: title_field taken from the default server list.
      QCOMPARE(servers[0].title_field, QString("tit"));
      // Unknown endpoint: no default available, stays empty.
      QVERIFY(servers[1].title_field.isEmpty());
    }

    void test_save_load_sru_servers_roundtrip_keeps_title_field()
    {
      QSettings settings;
      QVector<SruServer> servers;
      SruServer srv = marc21_server();
      srv.title_field = "custom.title";
      servers.append(srv);
      save_sru_servers(servers);
      auto const loaded = load_sru_servers();
      settings.remove("sru_servers");
      QCOMPARE(loaded.size(), 1);
      QCOMPARE(loaded[0].title_field, QString("custom.title"));
    }

    // -------------------------------------------------------------------
    // NameQueries plumbing (no network)

    void test_name_queries_query_name_false_without_services()
    {
      NameQueries q;
      QVERIFY(!q.query_name("Some Title", MediaKind::Book));
      QVERIFY(q.get_search_results().isEmpty());
    }

    void test_name_queries_query_name_false_for_unknown_kind()
    {
      NameQueries q;
      q.use_openlibrary(true);
      QVERIFY(!q.query_name("Some Title", MediaKind::Unknown));
    }

    void test_name_queries_kind_selects_services()
    {
      // Only book services enabled: CD/DVD searches have no source.
      NameQueries q;
      q.use_openlibrary(true);
      q.use_googleapis(true);
      QVERIFY(!q.query_name("Some Title", MediaKind::Cd));
      QVERIFY(!q.query_name("Some Title", MediaKind::Dvd));
    }

    void test_name_queries_get_item_out_of_range()
    {
      NameQueries q;
      QVERIFY(!q.get_item(0).has_value());
      QVERIFY(!q.get_item(-1).has_value());
      QVERIFY(q.get_thumbnail_url().isEmpty());
    }

    // -------------------------------------------------------------------
    // name_search_available

    void test_name_search_available()
    {
      QSettings settings;
      // All SRU servers disabled, no services enabled.
      settings.setValue(
          "sru_servers",
          QByteArray("[{\"name\":\"DNB\","
                     "\"endpoint\":\"https://services.dnb.de/sru/dnb\","
                     "\"isbn_field\":\"num\",\"ean_field\":\"\","
                     "\"title_field\":\"tit\","
                     "\"record_schema\":\"MARC21-xml\","
                     "\"enabled\":false}]"));
      settings.setValue("use_openlibrary", false);
      settings.setValue("use_googleapis", false);
      settings.setValue("use_musicbrainz", false);
      settings.sync();
      QVERIFY(!name_search_available(MediaKind::Book));
      QVERIFY(!name_search_available(MediaKind::Cd));
      QVERIFY(!name_search_available(MediaKind::Dvd));
      QVERIFY(!name_search_available(MediaKind::BluRay));
      QVERIFY(!name_search_available(MediaKind::Unknown));

      settings.setValue("use_openlibrary", true);
      settings.sync();
      QVERIFY(name_search_available(MediaKind::Book));
      QVERIFY(!name_search_available(MediaKind::Cd));
      QVERIFY(!name_search_available(MediaKind::Dvd));

      // DVD/Blu-ray searches need an SRU server with a title field.
      settings.setValue(
          "sru_servers",
          QByteArray("[{\"name\":\"DNB\","
                     "\"endpoint\":\"https://services.dnb.de/sru/dnb\","
                     "\"isbn_field\":\"num\",\"ean_field\":\"\","
                     "\"title_field\":\"tit\","
                     "\"record_schema\":\"MARC21-xml\","
                     "\"enabled\":true}]"));
      settings.sync();
      QVERIFY(name_search_available(MediaKind::Dvd));
      QVERIFY(name_search_available(MediaKind::BluRay));

      settings.remove("sru_servers");
      settings.remove("use_openlibrary");
      settings.remove("use_googleapis");
      settings.remove("use_musicbrainz");
      settings.sync();
    }
  };

} // namespace colman

QTEST_MAIN(colman::ColmanNameTest)
#include "colman_name_test.moc"

/* EOF */
