// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Integration tests for RestQueries against a live RestServer.
/**
   Server and client run in the same thread.  RestQueries::send() blocks
   over an inner QEventLoop; QHttpServer processes incoming requests during
   that loop.  No QThread is needed.

   Because the REST layer mandates versioned updates (PUT endpoints always
   call change_*_versioned on the server), the tests obtain the current
   version directly from sql_ before performing updates.
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_REST_QUERIES_HPP
#  include "colman_rest_queries.hpp"
#endif
#ifndef COLMAN_REST_API_VERSION_HPP
#  include "colman_rest_api_version.hpp"
#endif
#ifndef COLMAN_REST_SERVER_HPP
#  include "colman_rest_server.hpp"
#endif
#ifndef COLMAN_AUTH_MIDDLEWARE_HPP
#  include "colman_auth_middleware.hpp"
#endif
#ifndef COLMAN_JWT_VALIDATOR_HPP
#  include "colman_jwt_validator.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif
#ifndef THIRDPARTY_JWT_CPP_JWT_H
#  include <jwt-cpp/jwt.h>
#  define THIRDPARTY_JWT_CPP_JWT_H
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif

namespace
{

// ---------------------------------------------------------------------------
// Test key material (RSA-2048, generated offline, test use only)
// ---------------------------------------------------------------------------

  char const* const kPrivKey = R"(-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC3Um5omIvG086l
N3FE2kYHFD/8hNML3TU9YYwZQ8mMFrs4X1kHzi5Qa/NENQx/HLiJyVCCSyJMY7zB
1eYSf8K12n6LtIf8+5jWdjWJPyUkl+OwDnLo1C1nmGRX5xRROgVKxUs3/K/MOdYp
Tb/AwlYoCuoaxKyK/+fOoAPYaRdAkEgQ1XqWBeqZLMdEAPiwEilJD/amL14IaA8S
Lcw2mjl6zSeW8BlXUp3xeNlXMMoednGGV1LkuT9vTVQAu3OSna5dEa/eD4bq3lxs
cuvZZOrjNWzaqg8LyP9uwXydqkowaAIaOx5lbB14qSXQj4yrXnYZtmknUjJAbBOx
nldWVp3RAgMBAAECggEADsdJKcr0Z7au/8AwtCuBfjQadW2yws/WKSjaPbYqgw3Z
EXIUUA4Fy0tsjMjXBU9ILClrwVkzvmNC9gBboA2nIXhn/7SxysnwmAdnCPWrdZ/+
BbUge9cRAvs0yaZSC7tI7m5pwi1vcaAO/Pzs0qoH/onktfw5aao4In7hWHqeWoCz
yGoorwZNw/5Znz2MCYtSbueTBIUqZYiIaNl7mx5rjQlQ8OdpQgpbVr18QlZG9CpU
xqpm8TzAtyL+a83g2x54Nf0f0nUTFyVG1KsTVgfGbcC9OPZv+nuC15JTwu4YlqSK
5aJ0szHsYTbNuR5Exm/oOSIJPFXG3NCoC450Nt00kQKBgQD9HGqq+aDG+rp9tEN2
lWFvj26Ra+7VwrnAxXXgIs+1E2KUXKWSgd7H/BidKS5TpV0aYJf6n0QPDGiZ660X
KRnA31j7CgSXkNk8J1bi/vfP/VLzqfPkIxw9HyIStTAmnh7q+NJRZsgf/1jv4bUD
DKlcIUkmxuA2MWidJjkm9uOZ+QKBgQC5ahfTtWYCZpWUy67cgTpc8m51+e5ns2YJ
t6CnYBH0tnPwrLZz0Uq6P+iDzgrtpqUftw4wIbZ10MqbWOo64aTQC2l3o0QAJItS
yXgM6xbehYxF6kMpl6V8tqd0BdLSpu7glfaD9zV5A/JFg92I62QKYMlOU7/at5Zv
FZCXbvJYmQKBgCu9Bl3HR1Y7K5DfXq0WQUEi+wtcKGh9/tVmCemSemwjDmzu2ZnI
oXPTh3lZDW1c7QrK2FhxvzQOg+DdTgrriky/C8RNxd9fI3r2nVgakT4622PJ4aBx
xMZ3I88xykFSH6uhFc8FQN3upTLGwR3myTlncxmlUkdrIFXPrh6imVAZAoGABv5h
d7LX7i9AZSRprBeTw/3A/OI4g+DYcHVkseuFwd0/8kwRdbznPK88bfxPnTcz80ZM
WrRq1XbSrZlk/GOWe+JyT/jwj7DRMIY46QPVEP+oktqLPf5iV5nsJwwxL5DsYbxG
WCPy+GWVtdmLeaanV1zfOv0dt3pFSltd4Nhme8ECgYEA8cJuh9QPRNQ7BBcQNfMv
gtI1buvLP9j1hif3VHbCDRQCVSy1D1y4/HE2guJB7d7CzAlt9S7LD5y42dgmtPPm
c3JQ/TVnll9NydRiGSSpm/CnEJezZwMgmdiFdThT8Tw5tXddAUsQglc85CoUtxUe
/e8PAc2w0i0+aq+9368Ouk0=
-----END PRIVATE KEY-----)";

  char const* const kTestJwks = R"({
  "keys": [{
    "kty": "RSA",
    "kid": "test-key-1",
    "use": "sig",
    "alg": "RS256",
    "n": "t1JuaJiLxtPOpTdxRNpGBxQ__ITTC901PWGMGUPJjBa7OF9ZB84uUGvzRDUMfxy4iclQgksiTGO8wdXmEn_Ctdp-i7SH_PuY1nY1iT8lJJfjsA5y6NQtZ5hkV-cUUToFSsVLN_yvzDnWKU2_wMJWKArqGsSsiv_nzqAD2GkXQJBIENV6lgXqmSzHRAD4sBIpSQ_2pi9eCGgPEi3MNpo5es0nlvAZV1Kd8XjZVzDKHnZxhldS5Lk_b01UALtzkp2uXRGv3g-G6t5cbHLr2WTq4zVs2qoPC8j_bsF8napKMGgCGjseZWwdeKkl0I-Mq152GbZpJ1IyQGwTsZ5XVlad0Q",
    "e": "AQAB"
  }]
})";

  char const* const kIssuer = "https://auth.example.test/colman/";
  char const* const kSubject = "user-sub-abc-123";

  std::string make_token()
  {
    auto now = std::chrono::system_clock::now();
    return jwt::create()
        .set_issuer(kIssuer)
        .set_subject(kSubject)
        .set_key_id("test-key-1")
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::seconds(3600))
        .sign(jwt::algorithm::rs256("", kPrivKey, "", ""));
  }

// ---------------------------------------------------------------------------
// 1×1 white PNG (67 bytes, hardcoded)
// ---------------------------------------------------------------------------

  QByteArray test_png()
  {
    static char const kPng[] =
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52"
        "\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90\x77\x53"
        "\xde\x00\x00\x00\x0c\x49\x44\x41\x54\x08\xd7\x63\xf8\xcf\xc0\x00"
        "\x00\x00\x02\x00\x01\xe2\x21\xbc\x33\x00\x00\x00\x00\x49\x45\x4e"
        "\x44\xae\x42\x60\x82";
    return QByteArray(kPng, 67);
  }

// ---------------------------------------------------------------------------
// Test class
// ---------------------------------------------------------------------------

/// Integration tests for RestQueries communicating with RestServer.
  class RestQueriesTest : public QObject
  {
    Q_OBJECT

  public:
    RestQueriesTest() = default;

  private:
    std::unique_ptr<colman::SQLQueries> sql_;
    std::unique_ptr<colman::JwtValidator> validator_;
    std::unique_ptr<colman::AuthMiddleware> auth_;
    std::unique_ptr<colman::RestServer> server_;
    std::unique_ptr<colman::RestQueries> client_;
    QString conn_name_;
    int conn_counter_ = 0;

  // ------------------------------------------------------------------
  // Fixture
  // ------------------------------------------------------------------

    void setup()
    {
      conn_name_ = QString("rest_test_%1").arg(conn_counter_++);
      sql_ = std::make_unique<colman::SQLQueries>(conn_name_);
      QVERIFY(sql_->create_tables());
      QVERIFY(sql_->create_collection(colman::make_test_collection()));
      QVERIFY(sql_->add_administrator());
      QVERIFY(sql_->set_person_external_id(0, QString::fromUtf8(kSubject)));
      for(auto const& perm : sql_->get_permissions()) {
        sql_->set_permission(0, perm, true);
      }

      colman::JwtValidator::Config cfg;
      cfg.issuer = QString::fromUtf8(kIssuer);
      validator_ = std::make_unique<colman::JwtValidator>(cfg);
      QVERIFY(validator_->load_jwks(QString::fromUtf8(kTestJwks)));

      auth_ = std::make_unique<colman::AuthMiddleware>(*validator_, *sql_);
      server_ = std::make_unique<colman::RestServer>(*sql_, *auth_);
      quint16 const port = server_->listen_on_local_port();
      QVERIFY(port > 0);

      colman::RestConfig config;
      config.base_url = QString("http://127.0.0.1:%1").arg(port);
      config.token = QString::fromStdString(make_token());
      config.timeout_ms = 5000;
      client_ = std::make_unique<colman::RestQueries>();
      client_->set_config(config);
    }

    void teardown()
    {
      client_.reset();
      server_.reset();
      auth_.reset();
      validator_.reset();
      sql_.reset();
    }

  // ------------------------------------------------------------------
  // Scenario helpers
  // ------------------------------------------------------------------

  /// Adds a new Book type via the client; returns its type_id.
    int add_type()
    {
      return client_->add_new_type(colman::make_book_type());
    }

  /// Adds a new item of \a type_id named "Test Book"; returns item_id.
    int add_item(int type_id)
    {
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Test Book");
      return client_->add_new_item(item);
    }

  /// Acquires \a count copies of \a item_id; returns acquisition_id.
    int acquire(int item_id, int count = 5)
    {
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(static_cast<unsigned int>(count));
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      return client_->add_new_acquisition(acq);
    }

  /// Adds a new test person; returns person_id.
    int add_person()
    {
      return client_->add_new_person(colman::make_test_person());
    }

  /// Lends \a count copies of \a item_id to \a person_id; returns lending_id.
    int lend(int item_id, int person_id, unsigned int count = 3)
    {
      colman::Lending l;
      l.set_item_id(item_id);
      l.set_person_id(person_id);
      l.set_count(count);
      l.set_date_time(QDateTime::currentDateTimeUtc());
      return client_->add_new_lending(l);
    }

  /// Returns \a count copies from lending \a lending_id; returns return_id.
    int ret(int lending_id, unsigned int count = 2)
    {
      colman::Return r;
      r.set_lending_id(lending_id);
      r.set_count(count);
      r.set_date_time(QDateTime::currentDateTimeUtc());
      return client_->add_new_return(r);
    }

  /// Returns current version of \a id for \a entity from the SQL layer.
    int sql_version(colman::IQueries::VersionedEntity entity, int id)
    {
      return sql_->get_version(entity, id);
    }

  private Q_SLOTS:

  // ==================================================================
  // 0. api_version
  // ==================================================================

    void api_version()
    {
      setup();
      auto v = client_->fetch_api_version();
      QCOMPARE(v.major_, colman::ApiMajorVersion);
      QCOMPARE(v.minor_, colman::ApiMinorVersion);
      teardown();
    }

  // ==================================================================
  // 0b. signature_bottom_categories
  // ==================================================================

  /// An empty top must return all bottom categories regardless of top
  /// (regression: the dedicated "all bottoms" route, since the <arg>
  /// placeholder never matches an empty path segment).  A concrete top
  /// filters; a top without subcategories (the seeded "Picture books")
  /// yields an empty list.
    void signature_bottom_categories()
    {
      setup();
      auto const contains =
          [](std::vector<QString> const& v, QString const& s) {
            for(auto const& e : v) {
              if(e == s) {
                return true;
              }
            }
            return false;
          };

      auto const all =
          client_->read_signature_bottom_categories(QString());
      QVERIFY(!all.empty());
      QVERIFY(contains(all, QString("Adventure")));
      QVERIFY(contains(all, QString("Class newspaper")));

      auto const stories =
          client_->read_signature_bottom_categories("Stories");
      QVERIFY(contains(stories, QString("Adventure")));
      QVERIFY(!contains(stories, QString("Class newspaper")));

      auto const picture =
          client_->read_signature_bottom_categories("Picture books");
      QVERIFY(picture.empty());
      teardown();
    }

  // ==================================================================
  // 1. collection_round_trip
  // ==================================================================

    void collection_round_trip()
    {
      setup();
      auto col = client_->read_collection();
      QCOMPARE(col.get_name(), QString("Test Library"));

      col.set_name("New Library Name");
      QVERIFY(client_->change_collection(col));

      auto col2 = client_->read_collection();
      QCOMPARE(col2.get_name(), QString("New Library Name"));
      teardown();
    }

  // ==================================================================
  // 2. type_crud
  // ==================================================================

    void type_crud()
    {
      setup();
      int type_id = client_->add_new_type(colman::make_book_type());
      QVERIFY(type_id >= 0);

      auto t = client_->get_type(type_id);
      QCOMPARE(t.get_id(), type_id);
      QCOMPARE(t.get_name(), QString("Book"));

    // Add a second type and check total count.
      colman::KeyVector keys;
      keys.push_back({"issn", {colman::DataType::ShortString, {}}});
      colman::Type mag_type("Magazine", keys);
      int mag_id = client_->add_new_type(mag_type);
      QVERIFY(mag_id >= 0);
      auto types = client_->query_types(-1);
      QCOMPARE(static_cast<int>(types.size()), 2);

    // change_type
      colman::Type renamed("BookRenamed", colman::make_book_type().get_keys());
      QMap<QString, QString> display_names;
      QVERIFY(client_->change_type(type_id, renamed, display_names));

    // remove_type
      QVERIFY(client_->remove_type(type_id));
      auto types_after = client_->query_types(-1);
      QCOMPARE(static_cast<int>(types_after.size()), 1);
      teardown();
    }

    /// media_kind round-trips through the REST layer, and the search-category
    /// resolution helpers work against the server.
    void media_kind_over_rest()
    {
      setup();
      colman::KeyVector keys;
      keys.push_back({"director", {colman::DataType::ShortString, {}}});
      colman::Type dvd("dvd", keys);
      dvd.set_media_kind(colman::MediaKind::Dvd);
      int const id = client_->add_new_type(dvd);
      QVERIFY(id >= 0);

      // get_type and query_types carry the media kind.
      QCOMPARE(client_->get_type(id).get_media_kind(),
               colman::MediaKind::Dvd);
      QCOMPARE(client_->media_kind_for_type_id(id), colman::MediaKind::Dvd);
      QCOMPARE(client_->type_id_for_media_kind(colman::MediaKind::Dvd), id);
      QCOMPARE(client_->type_id_for_media_kind(colman::MediaKind::BoardGame),
               -1);

      // change_type updates the kind.
      colman::Type changed = client_->get_type(id);
      changed.set_media_kind(colman::MediaKind::BluRay);
      QVERIFY(client_->change_type(id, changed, {}));
      QCOMPARE(client_->media_kind_for_type_id(id),
               colman::MediaKind::BluRay);
      teardown();
    }

  // ==================================================================
  // 3. item_crud
  // ==================================================================

    void item_crud()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);

      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);

      colman::Item item;
      QString type_name;
      QVERIFY(client_->get_item(item_id, item, type_name));
      QCOMPARE(item.get_name(), QString("Test Book"));

    // The REST PUT route uses change_item_versioned; obtain version from SQL.
      item.set_name("Changed Book");
      auto ver = sql_version(
          colman::IQueries::VersionedEntity::Item, item_id);
      QVERIFY(ver >= 1);
      auto result = client_->change_item_versioned(item_id, item, ver);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      colman::Item item2;
      QString type_name2;
      QVERIFY(client_->get_item(item_id, item2, type_name2));
      QCOMPARE(item2.get_name(), QString("Changed Book"));

      QVERIFY(client_->remove_item(item_id));
      auto items = client_->query_items(-1, QString());
      QCOMPARE(static_cast<int>(items.size()), 0);
      teardown();
    }

  // ==================================================================
  // 3a. item_change_academic_year — grade level survives a REST round-trip
  // ==================================================================

  /// Editing a school item's grade level via the REST client must persist.
    void item_change_academic_year()
    {
      setup();
      int const type_id = add_type();
      QVERIFY(type_id >= 0);

      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("School Book");
      item.set_is_school_item(true);
      item.set_academic_year(colman::IntPairVec{{5, 7}});
      int const id = client_->add_new_item(item);
      QVERIFY(id >= 0);

      colman::Item loaded;
      QString tn;
      QVERIFY(client_->get_item(id, loaded, tn));
      QCOMPARE(colman::academic_years_to_string(loaded.get_academic_year()),
               QString("5-7"));

      loaded.set_academic_year(colman::IntPairVec{{8, 9}});
      QVERIFY(client_->change_item(id, loaded));

      colman::Item reloaded;
      QVERIFY(client_->get_item(id, reloaded, tn));
      QCOMPARE(colman::academic_years_to_string(reloaded.get_academic_year()),
               QString("8-9"));
      teardown();
    }

  // ==================================================================
  // 3b. item_invalid_academic_year — server rejects with 400
  // ==================================================================

  /// Posting/putting an item with a reversed grade range (first > second)
  /// must be rejected by the server, so the client reports failure.
    void item_invalid_academic_year()
    {
      setup();
      int const type_id = add_type();
      QVERIFY(type_id >= 0);

    // A well-formed range is accepted.
      colman::Item good;
      good.set_type_id(type_id);
      good.set_name("Good Item");
      good.set_is_school_item(true);
      good.set_academic_year(colman::IntPairVec{{5, 7}});
      int const good_id = client_->add_new_item(good);
      QVERIFY(good_id >= 0);

    // A reversed range (9 > 5) is rejected on create.
      colman::Item bad;
      bad.set_type_id(type_id);
      bad.set_name("Bad Item");
      bad.set_is_school_item(true);
      bad.set_academic_year(colman::IntPairVec{{9, 5}});
      QCOMPARE(client_->add_new_item(bad), -1);

    // The same reversed range is also rejected on update.
      colman::Item update = good;
      update.set_academic_year(colman::IntPairVec{{8, 3}});
      auto const ver = sql_version(
          colman::IQueries::VersionedEntity::Item, good_id);
      QVERIFY(ver >= 1);
      auto const result =
          client_->change_item_versioned(good_id, update, ver);
      QVERIFY(result != colman::IQueries::UpdateResult::Ok);

      teardown();
    }

  // ==================================================================
  // 4. item_versioned_update
  // ==================================================================

    void item_versioned_update()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);

      int version = sql_version(
          colman::IQueries::VersionedEntity::Item, item_id);
      QVERIFY(version >= 1);

      colman::Item item;
      QString type_name;
      QVERIFY(client_->get_item(item_id, item, type_name));

      auto result = client_->change_item_versioned(item_id, item, version);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

    // Old version should now cause a mismatch.
      auto result2 = client_->change_item_versioned(item_id, item, version);
      QCOMPARE(result2, colman::IQueries::UpdateResult::VersionMismatch);

    // Non-existent id.
      auto result3 = client_->change_item_versioned(9999, item, 1);
      QCOMPARE(result3, colman::IQueries::UpdateResult::NotFound);
      teardown();
    }

  // ==================================================================
  // 5. item_queries
  // ==================================================================

    void item_queries()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      int acq_id = acquire(item_id, 5);
      QVERIFY(acq_id >= 0);

      auto all = client_->query_items(-1, QString());
      QCOMPARE(static_cast<int>(all.size()), 1);

      auto with_search = client_->query_items(-1, "Test");
      QCOMPARE(static_cast<int>(with_search.size()), 1);

      auto no_match = client_->query_items(-1, "XXXX");
      QCOMPARE(static_cast<int>(no_match.size()), 0);

      auto acquired = client_->query_acquired_items(-1, QString());
      QCOMPARE(static_cast<int>(acquired.size()), 1);

      QCOMPARE(client_->stock_size(item_id), 5u);
      QCOMPARE(client_->available(item_id), 5u);
      teardown();
    }

  // ==================================================================
  // 6. item_barcode
  // ==================================================================

    void item_barcode()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);

      QString barcode = client_->compute_item_barcode(type_id, item_id);
      QVERIFY(!barcode.isEmpty());

      int parsed_type_id = -1;
      int parsed_item_id = -1;
      QVERIFY(
          client_->parse_item_barcode(barcode, parsed_type_id, parsed_item_id));
      QCOMPARE(parsed_item_id, item_id);

      QVERIFY(client_->is_item_barcode(barcode));
      QVERIFY(!client_->is_item_barcode("INVALID"));
      teardown();
    }

  // ==================================================================
  // 7. person_crud
  // ==================================================================

    void person_crud()
    {
      setup();
      int person_id = client_->add_new_person(colman::make_test_person());
      QVERIFY(person_id >= 0);

      colman::Person person;
      QVERIFY(client_->get_person(person_id, person));
      QCOMPARE(person.get_lastname(), QString("Doe"));

    // The REST PUT route uses change_person_versioned.
      person.set_lastname("Smith");
      auto ver = sql_version(
          colman::IQueries::VersionedEntity::Person, person_id);
      QVERIFY(ver >= 1);
      auto result = client_->change_person_versioned(person_id, person, ver);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      colman::Person person2;
      QVERIFY(client_->get_person(person_id, person2));
      QCOMPARE(person2.get_lastname(), QString("Smith"));

      QVERIFY(client_->remove_person(person_id));
    // Only the admin remains; query_persons(-1) excludes the administrator.
      auto persons = client_->query_persons(-1, QString());
      QCOMPARE(static_cast<int>(persons.size()), 0);
      teardown();
    }

  // ==================================================================
  // 8. person_versioned_update
  // ==================================================================

    void person_versioned_update()
    {
      setup();
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      int version = sql_version(
          colman::IQueries::VersionedEntity::Person, person_id);
      QVERIFY(version >= 1);

      colman::Person person;
      QVERIFY(client_->get_person(person_id, person));

      auto result = client_->change_person_versioned(person_id, person, version);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      auto result2 = client_->change_person_versioned(person_id, person, version);
      QCOMPARE(result2, colman::IQueries::UpdateResult::VersionMismatch);
      teardown();
    }

  // ==================================================================
  // 9. person_queries
  // ==================================================================

    void person_queries()
    {
      setup();
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      auto all = client_->query_persons(-1, QString());
      QVERIFY(static_cast<int>(all.size()) >= 1);

      auto by_name = client_->query_persons(-1, "Doe");
      QVERIFY(static_cast<int>(by_name.size()) >= 1);

      auto start_page = client_->query_start_page_persons(QString());
      QVERIFY(static_cast<int>(start_page.size()) >= 1);
      teardown();
    }

  // ==================================================================
  // 10. lending_crud
  // ==================================================================

    void lending_crud()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      int acq_id = acquire(item_id, 10);
      QVERIFY(acq_id >= 0);
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      int lending_id = lend(item_id, person_id);
      QVERIFY(lending_id >= 0);

      colman::Lending lending;
      QVERIFY(client_->get_lending(lending_id, lending));
      QCOMPARE(lending.get_item_id(), item_id);

    // The REST PUT route uses change_lending_versioned.
      lending.set_remarks("a remark");
      auto ver = sql_version(
          colman::IQueries::VersionedEntity::Lending, lending_id);
      QVERIFY(ver >= 1);
      auto result = client_->change_lending_versioned(lending_id, lending, ver);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      colman::Lending lending2;
      QVERIFY(client_->get_lending(lending_id, lending2));
      QCOMPARE(lending2.get_remarks(), QString("a remark"));

      QVERIFY(client_->remove_lending(lending_id));
      teardown();
    }

  // ==================================================================
  // 11. lending_balance
  // ==================================================================

    void lending_balance()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 10) >= 0);
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      int lending_id = lend(item_id, person_id, 5);
      QVERIFY(lending_id >= 0);
      QVERIFY(ret(lending_id, 3) >= 0);

      std::size_t lent = 0;
      std::size_t returned = 0;
      QVERIFY(client_->lending_balance(lending_id, lent, returned));
      QCOMPARE(lent, std::size_t{5});
      QCOMPARE(returned, std::size_t{3});
      teardown();
    }

  // ==================================================================
  // 12. lending_versioned_update
  // ==================================================================

    void lending_versioned_update()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 10) >= 0);
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      int lending_id = lend(item_id, person_id);
      QVERIFY(lending_id >= 0);

      int version = sql_version(
          colman::IQueries::VersionedEntity::Lending, lending_id);
      QVERIFY(version >= 1);

      colman::Lending l;
      QVERIFY(client_->get_lending(lending_id, l));

      auto result = client_->change_lending_versioned(lending_id, l, version);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      auto result2 = client_->change_lending_versioned(lending_id, l, version);
      QCOMPARE(result2, colman::IQueries::UpdateResult::VersionMismatch);
      teardown();
    }

  // ==================================================================
  // 13. return_crud
  // ==================================================================

    void return_crud()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 10) >= 0);
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      int lending_id = lend(item_id, person_id, 5);
      QVERIFY(lending_id >= 0);

      int return_id = ret(lending_id, 2);
      QVERIFY(return_id >= 0);

      colman::Return r;
      QVERIFY(client_->get_return(return_id, r));
      QCOMPARE(r.get_lending_id(), lending_id);

    // Update via versioned route.
      r.set_remarks("return note");
      auto ver = sql_version(
          colman::IQueries::VersionedEntity::Return, return_id);
      QVERIFY(ver >= 1);
      auto result = client_->change_return_versioned(return_id, r, ver);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      QVERIFY(client_->remove_return(return_id));
      teardown();
    }

  // ==================================================================
  // 14. acquisition_crud
  // ==================================================================

    void acquisition_crud()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);

      int acq_id = acquire(item_id, 7);
      QVERIFY(acq_id >= 0);

      colman::Acquisition acq;
      QVERIFY(client_->get_acquisition(acq_id, acq));
      QCOMPARE(acq.get_item_id(), item_id);

    // Update via versioned route.
      acq.set_count(10);
      auto ver = sql_version(
          colman::IQueries::VersionedEntity::Acquisition, acq_id);
      QVERIFY(ver >= 1);
      auto result = client_->change_acquisition_versioned(acq_id, acq, ver);
      QCOMPARE(result, colman::IQueries::UpdateResult::Ok);

      QVERIFY(client_->remove_acquisition(acq_id));
      teardown();
    }

  // ==================================================================
  // 15. liquidation_crud
  // ==================================================================

    void liquidation_crud()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 10) >= 0);

      colman::Liquidation liq;
      liq.set_item_id(item_id);
      liq.set_count(3);
      liq.set_date_time(QDateTime::currentDateTimeUtc());
      int liq_id = client_->add_new_liquidation(liq);
      QVERIFY(liq_id >= 0);

      colman::Liquidation liq2;
      QVERIFY(client_->get_liquidation(liq_id, liq2));
      QCOMPARE(liq2.get_item_id(), item_id);

      QVERIFY(client_->remove_liquidation(liq_id));
      teardown();
    }

  // ==================================================================
  // 16. history_query
  // ==================================================================

    void history_query()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      int acq_id = acquire(item_id, 10);
      QVERIFY(acq_id >= 0);
      int person_id = add_person();
      QVERIFY(person_id >= 0);
      int lending_id = lend(item_id, person_id, 3);
      QVERIFY(lending_id >= 0);
      int return_id = ret(lending_id, 2);
      QVERIFY(return_id >= 0);

      auto rows = client_->query_combined_history(true);
      QVERIFY(static_cast<int>(rows.size()) >= 3);

      QStringList known_actions = {"acquisition", "lending", "return",
                                   "liquidation"};
      for(auto const& row : rows) {
        QVERIFY(known_actions.contains(row.action));
      }

      auto lendings = client_->query_combined_history(
          true, QDate(), QDate(), true, 0, 0, "lending");
      QCOMPARE(static_cast<int>(lendings.size()), 1);
      QCOMPARE(lendings.front().action, QString("lending"));
      QCOMPARE(lendings.front().id, lending_id);
      auto returns = client_->query_combined_history(
          true, QDate(), QDate(), true, 0, 0, "return");
      QCOMPARE(static_cast<int>(returns.size()), 1);
      QCOMPARE(returns.front().id, return_id);
      teardown();
    }

    void extend_lendings()
    {
      setup();
      int type_id = add_type();
      QVERIFY(type_id >= 0);
      int item_id = add_item(type_id);
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 10) >= 0);
      int person_id = add_person();
      QVERIFY(person_id >= 0);
      int lending1 = lend(item_id, person_id, 1);
      int lending2 = lend(item_id, person_id, 1);
      QVERIFY(lending1 >= 0);
      QVERIFY(lending2 >= 0);

      QDate const due = QDate::currentDate().addDays(30);
      QVERIFY(client_->extend_lendings({lending1, lending2}, due));
      auto rows = client_->query_due_lendings(QDate(), QDate(), person_id);
      QCOMPARE(static_cast<int>(rows.size()), 2);
      for(auto const& row : rows) {
        QCOMPARE(row.duedate, due);
      }
      // an unknown lending makes the whole change fail
      QVERIFY(!client_->extend_lendings({lending1, 999999},
                                        due.addDays(1)));
      rows = client_->query_due_lendings(QDate(), QDate(), person_id);
      for(auto const& row : rows) {
        QCOMPARE(row.duedate, due);
      }
      teardown();
    }

    void group_lending()
    {
      using Status = colman::IQueries::GroupLendingResult::Status;
      setup();
      int const item_id = add_item(add_type());
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 3) >= 0);
      int const p1 = add_person();
      int const p2 = add_person();
      int const p3 = add_person();
      int const p4 = add_person();
      colman::Lending tmpl;
      tmpl.set_item_id(item_id);
      tmpl.set_duedate(QDate::currentDate().addDays(30));
      tmpl.set_remarks("class set");

      // more borrowers than copies: rejected, nothing written
      auto res = client_->add_group_lending(tmpl, {p1, p2, p3, p4});
      QCOMPARE(res.status_, Status::NotEnoughAvailable);
      QCOMPARE(res.lendable_, 3);
      QCOMPARE(client_->available(item_id), 3u);

      res = client_->add_group_lending(tmpl, {p1, p2});
      QCOMPARE(res.status_, Status::Ok);
      QCOMPARE(static_cast<int>(res.lending_ids_.size()), 2);
      QCOMPARE(client_->available(item_id), 1u);
      colman::Lending got;
      QVERIFY(client_->get_lending(res.lending_ids_[0], got));
      QCOMPARE(got.get_person_id(), p1);
      QCOMPARE(got.get_count(), 1u);
      QCOMPARE(got.get_duedate(), tmpl.get_duedate());

      // a current holder is rejected
      res = client_->add_group_lending(tmpl, {p3, p2});
      QCOMPARE(res.status_, Status::AlreadyLent);
      QCOMPARE(res.person_id_, p2);
      QCOMPARE(client_->available(item_id), 1u);
      teardown();
    }

    void group_return()
    {
      using Status = colman::IQueries::GroupReturnResult::Status;
      setup();
      int const item_id = add_item(add_type());
      QVERIFY(item_id >= 0);
      QVERIFY(acquire(item_id, 5) >= 0);
      int const p1 = add_person();
      int const p2 = add_person();
      int const p3 = add_person();
      QVERIFY(lend(item_id, p1, 2) >= 0);
      QVERIFY(lend(item_id, p2, 1) >= 0);
      QCOMPARE(client_->available(item_id), 2u);

      colman::Return tmpl;
      tmpl.set_remarks("collected");
      // a person without open lending: rejected, nothing written
      auto res = client_->add_group_return(item_id, {p1, p3}, tmpl);
      QCOMPARE(res.status_, Status::NotLent);
      QCOMPARE(res.person_id_, p3);
      QCOMPARE(client_->available(item_id), 2u);

      res = client_->add_group_return(item_id, {p1, p2}, tmpl);
      QCOMPARE(res.status_, Status::Ok);
      QCOMPARE(static_cast<int>(res.return_ids_.size()), 2);
      QCOMPARE(res.returned_count_, 3);
      QCOMPARE(client_->available(item_id), 5u);
      teardown();
    }

  // ==================================================================
  // 17. class_management
  // ==================================================================

    void class_management()
    {
      setup();
      auto names = client_->load_class_names();
      QCOMPARE(static_cast<int>(names.size()), 0);

      QVERIFY(client_->add_class_name("5A"));
      QVERIFY(client_->add_class_name("5B"));
      names = client_->load_class_names();
      QCOMPARE(static_cast<int>(names.size()), 2);

      QVERIFY(client_->rename_class_name("5A", "6A"));
      names = client_->load_class_names();
      QVERIFY(names.contains("6A"));
      QVERIFY(!names.contains("5A"));

      QVERIFY(client_->remove_class_name("6A"));
      names = client_->load_class_names();
      QCOMPARE(static_cast<int>(names.size()), 1);
      teardown();
    }

  // ==================================================================
  // 18. thumbnail_round_trip
  // ==================================================================

    void thumbnail_round_trip()
    {
      setup();
      QByteArray png = test_png();
      int thumb_id = client_->store_thumbnail(png, "test-key", "-1");
      QVERIFY(thumb_id >= 0);

      QByteArray retrieved;
      QVERIFY(client_->get_thumbnail(thumb_id, retrieved));
      QCOMPARE(retrieved, png);
      teardown();
    }

  // ==================================================================
  // 21. person_permissions
  // ==================================================================

    void person_permissions()
    {
      setup();
      int person_id = add_person();
      QVERIFY(person_id >= 0);

      auto cols = client_->get_permissions();
      QVERIFY(!cols.empty());

    // Default: edit_persons is false for a non-admin person.
      auto perms = client_->get_user_permissions(person_id);
      QCOMPARE(static_cast<int>(perms.size()),
               static_cast<int>(cols.size()));
      QVERIFY(!perms.empty());
      QVERIFY(!perms[0]); // edit_persons (index 0) is false

    // Enable edit_persons and verify
      QVERIFY(
          client_->set_permission(
              person_id, QStringLiteral("edit_persons"), true));
      auto perms2 = client_->get_user_permissions(person_id);
      QVERIFY(!perms2.empty());
      QVERIFY(perms2[0]); // edit_persons now true

    // Remove permissions; get_user_permissions recreates defaults on next call.
      QVERIFY(client_->remove_permissions(person_id));
      auto perms3 = client_->get_user_permissions(person_id);
      QVERIFY(!perms3.empty());
      QVERIFY(!perms3[0]); // edit_persons back to false

      teardown();
    }

  // ==================================================================
  // 22. person_external_id
  // ==================================================================

    void person_external_id()
    {
      setup();
      int person_id = add_person();
      QVERIFY(person_id >= 0);

    // Link person to an OIDC subject
      QVERIFY(client_->set_person_external_id(
          person_id, QStringLiteral("test-sub-42")));

    // Look up by external ID
      int found = client_->get_person_by_external_id(
          QStringLiteral("test-sub-42"));
      QCOMPARE(found, person_id);

      teardown();
    }

  // ==================================================================
  // 23. person_username
  // ==================================================================

    void person_username()
    {
      setup();
      int person_id = add_person();
      QVERIFY(person_id >= 0);

    // Initially no username.
      QVERIFY(client_->get_username(person_id).isEmpty());

    // Set username.
      QVERIFY(client_->set_username(person_id, QStringLiteral("alice")));
      QCOMPARE(client_->get_username(person_id), QStringLiteral("alice"));

    // Update username.
      QVERIFY(client_->set_username(person_id, QStringLiteral("alice2")));
      QCOMPARE(client_->get_username(person_id), QStringLiteral("alice2"));

    // Duplicate username is rejected (409 Conflict).
      int person_id2 = add_person();
      QVERIFY(person_id2 >= 0);
      QVERIFY(!client_->set_username(person_id2, QStringLiteral("alice2")));
      QVERIFY(client_->get_username(person_id2).isEmpty());

    // Remove username.
      QVERIFY(client_->remove_username(person_id));
      QVERIFY(client_->get_username(person_id).isEmpty());

    // After removal the username becomes available again.
      QVERIFY(client_->set_username(person_id2, QStringLiteral("alice2")));
      QCOMPARE(client_->get_username(person_id2), QStringLiteral("alice2"));

    // change-passwords list includes username.
      auto rows = client_->query_change_passwords_persons();
      bool found = false;
      for(auto const& r : rows) {
        if(r.id == person_id2) {
          QCOMPARE(r.username, QStringLiteral("alice2"));
          found = true;
        }
      }
      QVERIFY(found);

      teardown();
    }

  // ==================================================================
  // 24. reservations
  // ==================================================================

  /// Adds \a count copies of \a item_id via an SQL acquisition.
    int sql_acquire(int item_id, unsigned int count = 5)
    {
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(count);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      return sql_->add_new_acquisition(acq);
    }

    void reservations()
    {
      setup();
      int const type_id = add_type();
      int const item_id = add_item(type_id);
      QVERIFY(sql_acquire(item_id, 3) >= 0);
      int const person_id = add_person();
      QVERIFY(person_id >= 0);

    // add_reservation creates the reservation via POST /api/reservations.
      int res_id = 0;
      auto const result = client_->add_reservation(
          item_id, person_id, 1,
          QDate::currentDate().addDays(14),
          0, res_id);
      QCOMPARE(result, colman::IQueries::ReservationResult::Ok);
      QVERIFY(res_id > 0);

    // A second reservation for the same person tops up the existing one.
      int dup_id = 0;
      QCOMPARE(client_->add_reservation(
                   item_id, person_id, 1,
                   QDate::currentDate().addDays(14), 0, dup_id),
               colman::IQueries::ReservationResult::Ok);
      QCOMPARE(dup_id, res_id);

    // Reserving more copies than remain reports NotEnoughAvailable.
      int const other_person = add_person();
      QVERIFY(other_person >= 0);
      int over_id = 0;
      QCOMPARE(client_->add_reservation(
                   item_id, other_person, 3,
                   QDate::currentDate().addDays(14), 0, over_id),
               colman::IQueries::ReservationResult::NotEnoughAvailable);

    // get_reservations returns the topped-up reservation for this person.
      auto ress = client_->get_reservations(person_id);
      QCOMPARE(static_cast<int>(ress.size()), 1);
      QCOMPARE(ress[0].id, res_id);
      QCOMPARE(ress[0].item_id, item_id);
      QCOMPARE(ress[0].person_id, person_id);
      QCOMPARE(ress[0].count, 2);

    // get_reservations_for_item returns it as well.
      auto item_ress = client_->get_reservations_for_item(item_id);
      QCOMPARE(static_cast<int>(item_ress.size()), 1);
      QCOMPARE(item_ress[0].id, res_id);

    // get_reservation finds it by item and person; misses return false.
      colman::IQueries::Reservation one;
      QVERIFY(client_->get_reservation(item_id, person_id, one));
      QCOMPARE(one.id, res_id);
      QVERIFY(!client_->get_reservation(item_id, other_person, one));

    // get_reservations for a different person returns empty.
      auto other_ress = client_->get_reservations(person_id + 9999);
      QVERIFY(other_ress.empty());

    // remove_reservation deletes the record.
      QVERIFY(client_->remove_reservation(res_id));

    // After removal get_reservations returns empty.
      ress = client_->get_reservations(person_id);
      QVERIFY(ress.empty());

    // remove_reservation on a non-existent id returns false.
      QVERIFY(!client_->remove_reservation(res_id));

      teardown();
    }

  }; // class RestQueriesTest

} // anonymous namespace

QTEST_MAIN(RestQueriesTest)
#include "colman_rest_queries_test.moc"

/* EOF */
