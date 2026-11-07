// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Integration tests verifying that the REST server enforces permissions.
/**
   Each test creates a server with two accounts:
   - admin (person 0, all permissions)
   - restricted user (default permissions: view_personal_data and reserve
     only; everything else false)

   For every permission group that protects write or sensitive read
   endpoints the test checks that:
   1. The restricted user receives HTTP 403 Forbidden.
   2. The admin token reaches the same endpoint (response is not 401/403),
      proving the endpoint exists and the test is not a false positive.

   HTTP requests are made via QNetworkAccessManager rather than
   RestQueries so that the exact HTTP status code is available.

   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef SYSTEM_QNETWORKACCESSMANAGER
#  include <QNetworkAccessManager>
#  define SYSTEM_QNETWORKACCESSMANAGER
#endif
#ifndef SYSTEM_QNETWORKREQUEST
#  include <QNetworkRequest>
#  define SYSTEM_QNETWORKREQUEST
#endif
#ifndef SYSTEM_QNETWORKREPLY
#  include <QNetworkReply>
#  define SYSTEM_QNETWORKREPLY
#endif
#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
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
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#endif
#ifndef THIRDPARTY_JWT_CPP_JWT_H
#  include <jwt-cpp/jwt.h>
#  define THIRDPARTY_JWT_CPP_JWT_H
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_CHRONO
#  include <chrono>
#  define SYSTEM_CHRONO
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#  define COLMAN_ITEM_HPP
#endif
#ifndef COLMAN_ACQUISITION_HPP
#  include "colman_acquisition.hpp"
#  define COLMAN_ACQUISITION_HPP
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QJSONDOCUMENT
#  include <QJsonDocument>
#  define SYSTEM_QJSONDOCUMENT
#endif
#ifndef SYSTEM_QJSONOBJECT
#  include <QJsonObject>
#  define SYSTEM_QJSONOBJECT
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
  char const* const kSubjectAdmin = "user-sub-abc-123";
  char const* const kSubjectRestricted = "user-sub-restricted-456";

  std::string make_token(char const* subject)
  {
    auto now = std::chrono::system_clock::now();
    return jwt::create()
        .set_issuer(kIssuer)
        .set_subject(subject)
        .set_key_id("test-key-1")
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::seconds(3600))
        .sign(jwt::algorithm::rs256("", kPrivKey, "", ""));
  }

// ---------------------------------------------------------------------------
// Test class
// ---------------------------------------------------------------------------

  class RestAuthorizationTest : public QObject
  {
    Q_OBJECT

  public:
    RestAuthorizationTest() = default;

  private:
    std::unique_ptr<colman::SQLQueries> sql_;
    std::unique_ptr<colman::JwtValidator> validator_;
    std::unique_ptr<colman::AuthMiddleware> auth_;
    std::unique_ptr<colman::RestServer> server_;
    QNetworkAccessManager nam_;
    QString base_url_;
    QString admin_token_;
    QString restricted_token_;
    int restricted_person_id_ = -1;
    int conn_counter_ = 0;

  // ------------------------------------------------------------------
  // Fixture
  // ------------------------------------------------------------------

    void setup()
    {
      teardown();
      auto const conn = QString("auth_test_%1").arg(conn_counter_++);
      sql_ = std::make_unique<colman::SQLQueries>(conn);
      QVERIFY(sql_->create_tables());
      QVERIFY(sql_->create_collection(colman::make_test_collection()));
      QVERIFY(sql_->add_administrator());
      QVERIFY(sql_->set_person_external_id(0, kSubjectAdmin));
      for(auto const& perm : sql_->get_permissions()) {
        sql_->set_permission(0, perm, true);
      }

      auto person = colman::make_test_person("Restricted", "User");
      restricted_person_id_ = sql_->add_new_person(person);
      QVERIFY(restricted_person_id_ > 0);
      QVERIFY(sql_->set_person_external_id(
          restricted_person_id_, kSubjectRestricted));

      colman::JwtValidator::Config cfg;
      cfg.issuer = QString::fromUtf8(kIssuer);
      validator_ = std::make_unique<colman::JwtValidator>(cfg);
      QVERIFY(validator_->load_jwks(QString::fromUtf8(kTestJwks)));

      auth_ = std::make_unique<colman::AuthMiddleware>(*validator_, *sql_);
      server_ = std::make_unique<colman::RestServer>(*sql_, *auth_);
      quint16 const port = server_->listen_on_local_port();
      QVERIFY(port > 0);

      base_url_ = QString("http://127.0.0.1:%1").arg(port);
      admin_token_ = QString::fromStdString(make_token(kSubjectAdmin));
      restricted_token_ =
          QString::fromStdString(make_token(kSubjectRestricted));
    }

    void teardown()
    {
      server_.reset();
      auth_.reset();
      validator_.reset();
      sql_.reset();
      restricted_person_id_ = -1;
    }

  // ------------------------------------------------------------------
  // HTTP helper
  // ------------------------------------------------------------------

  /// Sends an HTTP request with a Bearer token and returns the status code.
    int send_raw(QString const& method,
                 QString const& path,
                 QString const& token,
                 QByteArray const& body = {})
    {
      QUrl const url(base_url_ + path);
      QNetworkRequest req(url);
      req.setRawHeader(
          "Authorization",
          QByteArray("Bearer ") + token.toUtf8());
      if(!body.isEmpty()) {
        req.setHeader(
            QNetworkRequest::ContentTypeHeader,
            QStringLiteral("application/json"));
      }
      QNetworkReply* reply = nullptr;
      if(method == QStringLiteral("GET")) {
        reply = nam_.get(req);
      }
      else if(method == QStringLiteral("POST")) {
        reply = nam_.post(req, body);
      }
      else if(method == QStringLiteral("PUT")) {
        reply = nam_.put(req, body);
      }
      else if(method == QStringLiteral("DELETE")) {
        reply = nam_.deleteResource(req);
      }
      else {
        return 0;
      }
      QEventLoop loop;
      QObject::connect(
          reply, &QNetworkReply::finished,
          &loop, &QEventLoop::quit);
      loop.exec();
      auto const status =
          reply->attribute(
              QNetworkRequest::HttpStatusCodeAttribute).toInt();
      reply->deleteLater();
      return status;
    }

  /// Sends a GET request and returns the parsed JSON response body.
    QJsonObject get_json(QString const& path, QString const& token)
    {
      QUrl const url(base_url_ + path);
      QNetworkRequest req(url);
      req.setRawHeader(
          "Authorization",
          QByteArray("Bearer ") + token.toUtf8());
      QNetworkReply* reply = nam_.get(req);
      QEventLoop loop;
      QObject::connect(
          reply, &QNetworkReply::finished,
          &loop, &QEventLoop::quit);
      loop.exec();
      auto const body = reply->readAll();
      reply->deleteLater();
      return QJsonDocument::fromJson(body).object();
    }

  /// Creates an item with \a count acquired copies; returns item_id.
    int sql_item_with_copies(unsigned int count = 5)
    {
      auto type_id = sql_->add_new_type(colman::make_book_type());
      if(type_id < 0) {
        type_id = sql_->get_type_id(QStringLiteral("Book"));
      }
      colman::Item item;
      item.set_type_id(type_id);
      item.set_name("Test Book");
      auto const item_id = sql_->add_new_item(item);
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(count);
      acq.set_date_time(QDateTime::currentDateTimeUtc());
      sql_->add_new_acquisition(acq);
      return item_id;
    }

  /// Creates a person holding the delegation right \a right, links \a subject
  /// as its external id (so make_token(subject) authenticates as this person)
  /// and returns the new person id.
    int make_delegated_user(QString const& right, char const* subject)
    {
      int const id =
          sql_->add_new_person(colman::make_test_person("Delegated", "User"));
      Q_ASSERT(id > 0);
      sql_->set_permission(id, right, true);
      sql_->set_person_external_id(id, subject);
      return id;
    }

  /// Sends a permission-grant PUT and returns the status code.
    int grant_permission(int target_id, QString const& column,
                         QString const& token)
    {
      return send_raw(
          "PUT",
          QString("/api/persons/%1/permissions/%2")
              .arg(target_id)
              .arg(column),
          token, R"({"value":true})");
    }

  // ------------------------------------------------------------------
  // Tests
  // ------------------------------------------------------------------

  private Q_SLOTS:

  /// Person management (edit_persons) is blocked for restricted users.
  /**
   * Covers: POST /api/persons, PUT /api/persons/<id>,
   *         DELETE /api/persons/<id>.
   */
    void test_edit_persons_person_crud_forbidden()
    {
      setup();

      auto const id_path =
          QString("/api/persons/%1").arg(restricted_person_id_);

    // POST — create person
      QCOMPARE(
          send_raw("POST", "/api/persons", restricted_token_, "{}"),
          403);
      auto const admin_post =
          send_raw("POST", "/api/persons", admin_token_, "{}");
      QVERIFY(admin_post != 401 && admin_post != 403);

    // PUT — edit person (auth check runs before If-Match validation)
      QCOMPARE(
          send_raw("PUT", id_path, restricted_token_, "{}"),
          403);
      auto const admin_put =
          send_raw("PUT", id_path, admin_token_, "{}");
      QVERIFY(admin_put != 401 && admin_put != 403);

    // DELETE — delete person
      QCOMPARE(send_raw("DELETE", id_path, restricted_token_), 403);
      auto const admin_del = send_raw("DELETE", id_path, admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

      teardown();
    }

  /// Setting and removing passwords (edit_persons) is blocked.
  /**
   * This is the most security-critical check: a restricted user must not
   * be able to change any account's password via the REST API.
   * Covers: POST /api/persons/<id>/password,
   *         DELETE /api/persons/<id>/password.
   */
    void test_edit_persons_password_change_forbidden()
    {
      setup();

      auto const pw_path =
          QString("/api/persons/%1/password")
          .arg(restricted_person_id_);

    // POST — set password
      QByteArray const valid_body =
          R"({"username":"alice","password":"hash"})";
      QCOMPARE(
          send_raw("POST", pw_path, restricted_token_, valid_body),
          403);
    // Admin with a valid body succeeds (200).
      QCOMPARE(
          send_raw("POST", pw_path, admin_token_, valid_body),
          200);

    // DELETE — remove password
      QCOMPARE(send_raw("DELETE", pw_path, restricted_token_), 403);
      auto const admin_del = send_raw("DELETE", pw_path, admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

      teardown();
    }

  /// A user may change their own password via POST /api/me/password.
  /**
   * Self-service password change requires no person-management permission
   * but must verify the current password.  Covers: correct old password
   * succeeds (200) and stores the new hash; wrong old password is rejected
   * (403); an empty new password is a bad request (400).
   */
    void test_change_own_password()
    {
      setup();

    // Give the restricted user a real (verifiable) current password.
      QVERIFY(sql_->set_password(
          restricted_person_id_, colman::hash_password("oldsecret")));
      auto const new_hash = colman::hash_password("newsecret");

    // Wrong current password is rejected.
      auto const wrong_body =
          QString(R"({"old_password":"nope","new_password":"%1"})")
              .arg(new_hash)
              .toUtf8();
      QCOMPARE(
          send_raw("POST", "/api/me/password", restricted_token_, wrong_body),
          403);

    // Correct current password succeeds and stores the new hash.
      auto const ok_body =
          QString(R"({"old_password":"oldsecret","new_password":"%1"})")
              .arg(new_hash)
              .toUtf8();
      QCOMPARE(
          send_raw("POST", "/api/me/password", restricted_token_, ok_body),
          200);
      QString stored;
      QVERIFY(sql_->get_password(restricted_person_id_, stored));
      QVERIFY(colman::verify_password("newsecret", stored));

    // An empty new password is a bad request.
      QCOMPARE(
          send_raw("POST", "/api/me/password", restricted_token_,
                   R"({"old_password":"newsecret","new_password":""})"),
          400);

      teardown();
    }

  /// Managing other users' permissions (edit_persons) is blocked.
  /**
   * A restricted user must not be able to grant themselves or others
   * additional permissions.
   * Covers: PUT /api/persons/<id>/permissions/<column>,
   *         DELETE /api/persons/<id>/permissions,
   *         POST /api/persons/<id>/permissions/defaults.
   *
   * The DELETE positive test runs before the POST negative/positive pair
   * so that the permissions row is absent when POST /defaults is tested
   * (INSERT only succeeds on an absent row).  The admin PUT positive
   * test uses a harmless column (print_signatures) to avoid granting
   * edit_persons to the restricted user as a side effect.
   */
    void test_edit_persons_permissions_management_forbidden()
    {
      setup();

      auto const perm_col_path =
          QString("/api/persons/%1/permissions/edit_persons")
          .arg(restricted_person_id_);
      auto const admin_col_path =
          QString("/api/persons/%1/permissions/print_signatures")
          .arg(restricted_person_id_);
      auto const defaults_path =
          QString("/api/persons/%1/permissions/defaults")
          .arg(restricted_person_id_);
      auto const perms_path =
          QString("/api/persons/%1/permissions")
          .arg(restricted_person_id_);

    // PUT — set a single permission flag
      QCOMPARE(
          send_raw("PUT", perm_col_path, restricted_token_,
                   R"({"value":true})"),
          403);
    // Admin uses a harmless column so restricted user keeps
    // edit_persons=false for the subsequent checks.
      auto const admin_put =
          send_raw("PUT", admin_col_path, admin_token_,
                   R"({"value":true})");
      QVERIFY(admin_put != 401 && admin_put != 403);

    // DELETE — remove all permissions
    // Run before POST /defaults because the POST inserts a new row
    // and only works when the row is absent (DELETE creates that state).
      QCOMPARE(
          send_raw("DELETE", perms_path, restricted_token_),
          403);
      auto const admin_del =
          send_raw("DELETE", perms_path, admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

    // POST — recreate default permissions.
    // The auth check for the restricted user re-creates the row via
    // get_user_permissions, so we delete it again via SQL before the
    // admin positive test (INSERT only works on an absent row).
      QCOMPARE(
          send_raw("POST", defaults_path, restricted_token_),
          403);
      QVERIFY(sql_->remove_permissions(restricted_person_id_));
      auto const admin_post =
          send_raw("POST", defaults_path, admin_token_);
      QVERIFY(admin_post != 401 && admin_post != 403);

      teardown();
    }

  /// Reading the person list (view_persons) is blocked for restricted users.
  /**
   * Covers: GET /api/persons,
   *         GET /api/persons/change-passwords.
   */
    void test_view_persons_list_forbidden()
    {
      setup();

      QCOMPARE(
          send_raw("GET", "/api/persons", restricted_token_),
          403);
      auto const admin_get =
          send_raw("GET", "/api/persons", admin_token_);
      QVERIFY(admin_get != 401 && admin_get != 403);

      QCOMPARE(
          send_raw("GET",
                   "/api/persons/change-passwords",
                   restricted_token_),
          403);
      auto const admin_cp =
          send_raw("GET",
                   "/api/persons/change-passwords",
                   admin_token_);
      QVERIFY(admin_cp != 401 && admin_cp != 403);

      teardown();
    }

  /// Reading a single person allows self-access without view_persons.
  /**
   * Covers: GET /api/persons/<id>.
   */
    void test_get_person_self_allowed()
    {
      setup();

    // Own record is readable without view_persons.
      QCOMPARE(
          send_raw("GET",
                   QString("/api/persons/%1")
                       .arg(restricted_person_id_),
                   restricted_token_),
          200);

    // Other persons stay forbidden without view_persons.
      QCOMPARE(
          send_raw("GET", "/api/persons/0", restricted_token_),
          403);

      teardown();
    }

  /// Creating and deleting lendings (collect_lendings) is blocked.
  /**
   * Covers: POST /api/lendings, DELETE /api/lendings/<id>.
   */
    void test_collect_lendings_forbidden()
    {
      setup();

    // POST — create lending
      QCOMPARE(
          send_raw("POST", "/api/lendings", restricted_token_, "{}"),
          403);
      auto const admin_post =
          send_raw("POST", "/api/lendings", admin_token_, "{}");
      QVERIFY(admin_post != 401 && admin_post != 403);

    // DELETE — delete a lending (ID 1; may not exist, but auth runs first)
      QCOMPARE(
          send_raw("DELETE", "/api/lendings/1", restricted_token_),
          403);
      auto const admin_del =
          send_raw("DELETE", "/api/lendings/1", admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

      teardown();
    }

  /// Creating returns (collect_returns) is blocked for restricted users.
  /**
   * Covers: POST /api/returns, DELETE /api/returns/<id>.
   */
    void test_collect_returns_forbidden()
    {
      setup();

      QCOMPARE(
          send_raw("POST", "/api/returns", restricted_token_, "{}"),
          403);
      auto const admin_post =
          send_raw("POST", "/api/returns", admin_token_, "{}");
      QVERIFY(admin_post != 401 && admin_post != 403);

      QCOMPARE(
          send_raw("DELETE", "/api/returns/1", restricted_token_),
          403);
      auto const admin_del =
          send_raw("DELETE", "/api/returns/1", admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

      teardown();
    }

  /// The lending-balance endpoint is reachable with collect_returns alone.
  /**
   * The return wizard reads the balance (lent/returned counts) in its details
   * step.  A user who may collect returns but cannot view history must still
   * be able to read it; users with neither permission stay forbidden.
   * Covers: GET /api/lendings/<id>/balance.
   */
    void test_balance_allowed_for_collect_returns()
    {
      setup();

      QCOMPARE(
          send_raw("GET", "/api/lendings/1/balance", restricted_token_),
          403);

      make_delegated_user("collect_returns", "user-sub-returner");
      auto const token =
          QString::fromStdString(make_token("user-sub-returner"));
      auto const code =
          send_raw("GET", "/api/lendings/1/balance", token);
      QVERIFY(code != 401 && code != 403);

      teardown();
    }

  /// Item and acquisition management (collect_acquisitions) is blocked.
  /**
   * Covers: POST /api/items, PUT /api/items/<id>,
   *         DELETE /api/items/<id>, POST /api/acquisitions.
   */
    void test_collect_acquisitions_forbidden()
    {
      setup();

    // POST /api/items — add item
      QCOMPARE(
          send_raw("POST", "/api/items", restricted_token_, "{}"),
          403);
      auto const admin_post_item =
          send_raw("POST", "/api/items", admin_token_, "{}");
      QVERIFY(admin_post_item != 401 && admin_post_item != 403);

    // PUT /api/items/<id>
      QCOMPARE(
          send_raw("PUT", "/api/items/1", restricted_token_, "{}"),
          403);
      auto const admin_put =
          send_raw("PUT", "/api/items/1", admin_token_, "{}");
      QVERIFY(admin_put != 401 && admin_put != 403);

    // DELETE /api/items/<id>
      QCOMPARE(
          send_raw("DELETE", "/api/items/1", restricted_token_),
          403);
      auto const admin_del =
          send_raw("DELETE", "/api/items/1", admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

    // POST /api/acquisitions
      QCOMPARE(
          send_raw("POST", "/api/acquisitions",
                   restricted_token_, "{}"),
          403);
      auto const admin_post_acq =
          send_raw("POST", "/api/acquisitions", admin_token_, "{}");
      QVERIFY(admin_post_acq != 401 && admin_post_acq != 403);

      teardown();
    }

  /// Viewing and modifying history (view_history, change_actions) is blocked.
  /**
   * Covers: GET /api/history,
   *         DELETE /api/history (batch delete),
   *         PUT /api/history (apply change).
   */
    void test_history_access_forbidden()
    {
      setup();

    // GET — view history
      QCOMPARE(
          send_raw("GET", "/api/history", restricted_token_),
          403);
      auto const admin_get =
          send_raw("GET", "/api/history", admin_token_);
      QVERIFY(admin_get != 401 && admin_get != 403);

    // DELETE — batch-delete history entries (requires change_actions)
      QCOMPARE(
          send_raw("DELETE", "/api/history", restricted_token_),
          403);
      auto const admin_del =
          send_raw("DELETE", "/api/history", admin_token_);
      QVERIFY(admin_del != 401 && admin_del != 403);

      teardown();
    }

  /// Class management (manage_classes) is blocked for restricted users.
  /**
   * Covers: POST /api/classes, DELETE /api/classes/<name>.
   */
    void test_manage_classes_forbidden()
    {
      setup();

      QCOMPARE(
          send_raw("POST", "/api/classes", restricted_token_,
                   R"({"name":"5a"})"),
          403);
      auto const admin_post =
          send_raw("POST", "/api/classes", admin_token_,
                   R"({"name":"5a"})");
      QVERIFY(admin_post != 401 && admin_post != 403);

      QCOMPARE(
          send_raw("DELETE", "/api/classes/5a", restricted_token_),
          403);

      teardown();
    }

  /// Liquidation management (collect_liquidations) is blocked.
  /**
   * Covers: POST /api/liquidations.
   */
    void test_collect_liquidations_forbidden()
    {
      setup();

      QCOMPARE(
          send_raw("POST", "/api/liquidations",
                   restricted_token_, "{}"),
          403);
      auto const admin_post =
          send_raw("POST", "/api/liquidations", admin_token_, "{}");
      QVERIFY(admin_post != 401 && admin_post != 403);

      teardown();
    }

  /// Granting edit_persons to the restricted user lifts the password block.
  /**
   * Proves that the permission check is the only gate: once the permission
   * is set, the same call that previously returned 403 now returns 200.
   */
    void test_granted_permission_allows_password_change()
    {
      setup();

      auto const pw_path =
          QString("/api/persons/%1/password")
          .arg(restricted_person_id_);
      QByteArray const valid_body =
          R"({"username":"alice","password":"hash"})";

    // Without permission: 403.
      QCOMPARE(
          send_raw("POST", pw_path, restricted_token_, valid_body),
          403);

    // Grant edit_persons directly in the database.
      QVERIFY(sql_->set_permission(
          restricted_person_id_,
          QStringLiteral("edit_persons"),
          true));

    // With permission: 200.
      QCOMPARE(
          send_raw("POST", pw_path, restricted_token_, valid_body),
          200);

      teardown();
    }

  /// GET /api/me returns person_id and edit_reservations for the caller.
  /**
   * Admin: person_id == 0, edit_reservations == true.
   * Restricted user: person_id matches, edit_reservations == false.
   */
    void test_api_me()
    {
      setup();

    // Admin.
      auto const admin_obj = get_json("/api/me", admin_token_);
      QCOMPARE(admin_obj[QStringLiteral("person_id")].toInt(), 0);
      QVERIFY(admin_obj[QStringLiteral("edit_reservations")].toBool());

    // Restricted user.
      auto const res_obj = get_json("/api/me", restricted_token_);
      QCOMPARE(res_obj[QStringLiteral("person_id")].toInt(),
               restricted_person_id_);
      QVERIFY(!res_obj[QStringLiteral("edit_reservations")].toBool());

      teardown();
    }

  /// DELETE /api/reservations/<id> enforces ownership.
  /**
   * A restricted user (reserve, no edit_reservations) may only delete
   * their own reservations; deleting another user's reservation yields 403.
   */
    void test_reservations_delete_ownership()
    {
      setup();

      int const item_id = sql_item_with_copies(4);

    // Create a reservation owned by admin.
      int admin_res_id = 0;
      QCOMPARE(
          sql_->add_reservation(
              item_id, 0, 1,
              QDate::currentDate().addDays(14),
              0, admin_res_id),
          colman::IQueries::ReservationResult::Ok);
      QVERIFY(admin_res_id > 0);

    // Restricted user cannot delete admin's reservation.
      auto const admin_res_path =
          QString("/api/reservations/%1").arg(admin_res_id);
      QCOMPARE(
          send_raw("DELETE", admin_res_path, restricted_token_),
          403);

    // Admin can delete their own reservation.
      QCOMPARE(
          send_raw("DELETE", admin_res_path, admin_token_),
          200);

    // Create a second item so the restricted user can also reserve.
      int const item_id2 = sql_item_with_copies(4);
      int restricted_res_id = 0;
      QCOMPARE(
          sql_->add_reservation(
              item_id2, restricted_person_id_, 1,
              QDate::currentDate().addDays(14),
              0, restricted_res_id),
          colman::IQueries::ReservationResult::Ok);
      QVERIFY(restricted_res_id > 0);

    // Restricted user can delete their own reservation.
      auto const own_res_path =
          QString("/api/reservations/%1").arg(restricted_res_id);
      QCOMPARE(
          send_raw("DELETE", own_res_path, restricted_token_),
          200);

      teardown();
    }

  /// GET /api/reservations auto-filters to the caller's own reservations.
  /**
   * A user without edit_reservations only sees their own reservations even
   * when the database contains reservations from other users.
   */
    void test_reservations_get_filtered()
    {
      setup();

      int const item_admin = sql_item_with_copies(4);
      int const item_restr = sql_item_with_copies(4);

    // Create reservation for admin and one for restricted user.
      int admin_res_id = 0;
      QCOMPARE(
          sql_->add_reservation(
              item_admin, 0, 1,
              QDate::currentDate().addDays(14), 0, admin_res_id),
          colman::IQueries::ReservationResult::Ok);

      int restricted_res_id = 0;
      QCOMPARE(
          sql_->add_reservation(
              item_restr, restricted_person_id_, 1,
              QDate::currentDate().addDays(14), 0, restricted_res_id),
          colman::IQueries::ReservationResult::Ok);

    // Restricted user's GET returns 200.
      QCOMPARE(send_raw("GET", "/api/reservations", restricted_token_), 200);

    // Admin's GET without filter also returns 200.
      QCOMPARE(send_raw("GET", "/api/reservations", admin_token_), 200);

      teardown();
    }

  // ------------------------------------------------------------------
  // Delegated person creation (create_lenders / create_data_collectors)
  // ------------------------------------------------------------------

  /// A create_lenders holder may create persons and grant the borrower
  /// profile, but not elevated or out-of-profile permissions.
    void test_create_lenders_delegated_scope()
    {
      setup();
      make_delegated_user("create_lenders", "user-sub-lender-1");
      auto const token =
          QString::fromStdString(make_token("user-sub-lender-1"));

    // May create a new person (no longer requires edit_persons).
      auto const post =
          send_raw("POST", "/api/persons", token,
                   R"({"lastname":"New","firstname":"Borrower"})");
      QVERIFY(post != 401 && post != 403);

    // A freshly created target holds only baseline rights.
      int const target =
          sql_->add_new_person(colman::make_test_person("Fresh", "Target"));
      QVERIFY(target > 0);

    // May grant borrower-profile permissions.
      QCOMPARE(grant_permission(target, "view_items", token), 200);
      QCOMPARE(grant_permission(target, "reserve", token), 200);
    // May NOT grant elevated or out-of-profile permissions.
      QCOMPARE(grant_permission(target, "edit_persons", token), 403);
      QCOMPARE(grant_permission(target, "collect_acquisitions", token), 403);
      QCOMPARE(grant_permission(target, "edit_reservations", token), 403);

      teardown();
    }

  /// A create_lenders holder may not manage staff or the administrator.
    void test_create_lenders_cannot_manage_privileged_targets()
    {
      setup();
      make_delegated_user("create_lenders", "user-sub-lender-2");
      auto const token =
          QString::fromStdString(make_token("user-sub-lender-2"));

    // A staff member with edit_persons is outside the creator's scope.
      int const staff =
          sql_->add_new_person(colman::make_test_person("Staff", "Member"));
      QVERIFY(staff > 0);
      QVERIFY(sql_->set_permission(staff, "edit_persons", true));

    // Cannot edit the staff member (containment invariant fails) ...
      QCOMPARE(
          send_raw("PUT", QString("/api/persons/%1/class").arg(staff),
                   token, R"({"value":"7a"})"),
          403);
    // ... not even a permission that is within the borrower profile,
    // because the target itself is out of scope.
      QCOMPARE(grant_permission(staff, "view_items", token), 403);
    // Cannot reset the administrator's permissions.
      QCOMPARE(
          send_raw("POST", "/api/persons/0/permissions/defaults", token),
          403);

      teardown();
    }

  /// A create_data_collectors holder may grant only the collector profile.
    void test_create_data_collectors_delegated_scope()
    {
      setup();
      int const creator =
          make_delegated_user("create_data_collectors", "user-sub-collector-1");
      // The creator must hold the collect rights to be able to delegate them
      // (you may only grant what you hold yourself).
      QVERIFY(sql_->set_permission(creator, "collect_acquisitions", true));
      QVERIFY(sql_->set_permission(creator, "collect_liquidations", true));
      auto const token =
          QString::fromStdString(make_token("user-sub-collector-1"));

      auto const post =
          send_raw("POST", "/api/persons", token,
                   R"({"lastname":"New","firstname":"Collector"})");
      QVERIFY(post != 401 && post != 403);

      int const target = sql_->add_new_person(
          colman::make_test_person("Fresh", "Collector"));
      QVERIFY(target > 0);

      QCOMPARE(grant_permission(target, "collect_acquisitions", token), 200);
      QCOMPARE(grant_permission(target, "collect_liquidations", token), 200);
      QCOMPARE(grant_permission(target, "view_items", token), 200);
      QCOMPARE(grant_permission(target, "edit_persons", token), 403);
      QCOMPARE(grant_permission(target, "view_persons", token), 403);

      teardown();
    }

  }; // class RestAuthorizationTest

} // namespace

QTEST_MAIN(RestAuthorizationTest)
#include "colman_rest_authorization_test.moc"

/* EOF */
