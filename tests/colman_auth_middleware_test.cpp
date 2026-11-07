// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Unit tests for AuthMiddleware.
/**
   Uses an in-memory SQLQueries database as the IQueries backend and
   a JwtValidator loaded with a pre-generated test JWKS, so no network
   access is required.
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
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
#  define COLMAN_PASSWORDS_HPP
#endif
#ifndef THIRDPARTY_JWT_CPP_JWT_H
#  include <jwt-cpp/jwt.h>
#  define THIRDPARTY_JWT_CPP_JWT_H
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_OPTIONAL
#  include <optional>
#  define SYSTEM_OPTIONAL
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

  struct TokenParams
  {
    std::string issuer = kIssuer;
    std::string subject = "user-sub-abc-123";
    std::string kid = "test-key-1";
    std::string private_key = kPrivKey;
    int expires_offset = 3600;
    std::string audience;
  };

  std::string make_token(TokenParams const& p = {})
  {
    auto now = std::chrono::system_clock::now();
    auto builder =
        jwt::create()
            .set_issuer(p.issuer)
            .set_subject(p.subject)
            .set_key_id(p.kid)
            .set_issued_at(now)
            .set_expires_at(now + std::chrono::seconds(p.expires_offset));
    if(!p.audience.empty()) {
      builder.set_audience(p.audience);
    }
    return builder.sign(jwt::algorithm::rs256("", p.private_key, "", ""));
  }

  QString bearer(std::string const& token)
  {
    return "Bearer " + QString::fromStdString(token);
  }

// ---------------------------------------------------------------------------
// Test class
// ---------------------------------------------------------------------------

  class AuthMiddlewareTest : public QObject
  {
    Q_OBJECT

  public:
    AuthMiddlewareTest() = default;

  private:
    std::optional<colman::SQLQueries> q_;
    QString conn_name_;
    int conn_counter_ = 0;

    std::unique_ptr<colman::JwtValidator> validator_;
    std::unique_ptr<colman::AuthMiddleware> auth_;

    void setup_db()
    {
      conn_name_ = QString("auth_mw_test_%1").arg(conn_counter_++);
      q_.emplace(conn_name_);
      QVERIFY(q_->create_tables());
      QVERIFY(q_->create_collection(colman::make_test_collection()));
      QVERIFY(q_->add_administrator());
    }

    void teardown_db()
    {
      auth_.reset();
      validator_.reset();
      q_.reset();
    }

    void setup_auth()
    {
      colman::JwtValidator::Config cfg;
      cfg.issuer = QString::fromUtf8(kIssuer);
      validator_.reset(new colman::JwtValidator{cfg});
      QVERIFY(validator_->load_jwks(QString::fromUtf8(kTestJwks)));
      auth_.reset(new colman::AuthMiddleware{*validator_, *q_});
    }

  private Q_SLOTS:

  // ----------------------------------------------------------------
  // http_status mapping (static, no DB needed)
  // ----------------------------------------------------------------

    void test_http_status_ok_is_200()
    {
      using E = colman::AuthMiddleware::AuthError;
      QCOMPARE(colman::AuthMiddleware::http_status(E::Ok), 200);
    }

    void test_http_status_missing_token_is_401()
    {
      using E = colman::AuthMiddleware::AuthError;
      QCOMPARE(colman::AuthMiddleware::http_status(E::MissingToken), 401);
    }

    void test_http_status_invalid_token_is_401()
    {
      using E = colman::AuthMiddleware::AuthError;
      QCOMPARE(colman::AuthMiddleware::http_status(E::InvalidToken), 401);
    }

    void test_http_status_no_account_is_403()
    {
      using E = colman::AuthMiddleware::AuthError;
      QCOMPARE(colman::AuthMiddleware::http_status(E::NoAccount), 403);
    }

    void test_http_status_forbidden_is_403()
    {
      using E = colman::AuthMiddleware::AuthError;
      QCOMPARE(colman::AuthMiddleware::http_status(E::Forbidden), 403);
    }

  // ----------------------------------------------------------------
  // MissingToken
  // ----------------------------------------------------------------

    void test_empty_header_returns_missing_token()
    {
      setup_db();
      setup_auth();
      auto const r = auth_->authenticate("", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::MissingToken);
      teardown_db();
    }

    void test_unknown_scheme_returns_missing_token()
    {
      setup_db();
      setup_auth();
      auto const r = auth_->authenticate("Digest abc123", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::MissingToken);
      teardown_db();
    }

    void test_bearer_without_token_returns_missing_token()
    {
      setup_db();
      setup_auth();
      auto const r = auth_->authenticate("Bearer ", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::MissingToken);
      teardown_db();
    }

  // ----------------------------------------------------------------
  // InvalidToken
  // ----------------------------------------------------------------

    void test_malformed_jwt_returns_invalid_token()
    {
      setup_db();
      setup_auth();
      auto const r = auth_->authenticate("Bearer not.a.jwt", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::InvalidToken);
      teardown_db();
    }

    void test_expired_token_returns_invalid_token()
    {
      setup_db();
      setup_auth();
      TokenParams p;
      p.expires_offset = -3600;
      auto const r = auth_->authenticate(bearer(make_token(p)), "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::InvalidToken);
      teardown_db();
    }

    void test_wrong_issuer_returns_invalid_token()
    {
      setup_db();
      setup_auth();
      TokenParams p;
      p.issuer = "https://untrusted.example.test/";
      auto const r = auth_->authenticate(bearer(make_token(p)), "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::InvalidToken);
      teardown_db();
    }

  // ----------------------------------------------------------------
  // NoAccount
  // ----------------------------------------------------------------

    void test_valid_token_without_linked_account_returns_no_account()
    {
      setup_db();
      setup_auth();
    // sub does not match any person's external_id
      auto const r = auth_->authenticate(bearer(make_token()), "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::NoAccount);
      teardown_db();
    }

  // ----------------------------------------------------------------
  // Ok
  // ----------------------------------------------------------------

    void test_valid_token_with_permission_returns_ok()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "user-sub-abc-123"));
      QVERIFY(q_->set_permission(id, "view_items", true));
      auto const r = auth_->authenticate(bearer(make_token()), "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Ok);
      teardown_db();
    }

    void test_ok_result_carries_correct_person_id()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "user-sub-abc-123"));
      QVERIFY(q_->set_permission(id, "view_items", true));
      auto const r = auth_->authenticate(bearer(make_token()), "view_items");
      QCOMPARE(r.person_id, id);
      teardown_db();
    }

    void test_empty_required_permission_skips_permission_check()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "user-sub-abc-123"));
    // person has no special permissions — but permission check is skipped
      auto const r = auth_->authenticate(bearer(make_token()), "");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Ok);
      QCOMPARE(r.person_id, id);
      teardown_db();
    }

    void test_administrator_can_authenticate_via_external_id()
    {
      setup_db();
      setup_auth();
    // Link the admin account (person_id = 0) to an Authentik sub.
      QVERIFY(q_->set_person_external_id(0, "admin-sub-000"));
      TokenParams p;
      p.subject = "admin-sub-000";
      auto const r = auth_->authenticate(bearer(make_token(p)), "edit_persons");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Ok);
      QCOMPARE(r.person_id, 0);
      teardown_db();
    }

  // ----------------------------------------------------------------
  // Forbidden
  // ----------------------------------------------------------------

    void test_valid_token_without_permission_returns_forbidden()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "user-sub-abc-123"));
    // person has default permissions — collect_lendings is false
      auto const r =
          auth_->authenticate(bearer(make_token()), "collect_lendings");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Forbidden);
      teardown_db();
    }

    void test_forbidden_result_has_no_person_id()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_person_external_id(id, "user-sub-abc-123"));
      auto const r =
          auth_->authenticate(bearer(make_token()), "collect_lendings");
      QCOMPARE(r.person_id, -1);
      teardown_db();
    }

  // ----------------------------------------------------------------
  // Basic Auth
  // ----------------------------------------------------------------

    void test_basic_auth_valid_credentials_returns_ok()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_permission(id, "view_items", true));
      QVERIFY(q_->set_local_credentials(
          id, "alice", colman::hash_password("secret")));
    // "alice:secret" base64 = YWxpY2U6c2VjcmV0
      auto const r =
          auth_->authenticate("Basic YWxpY2U6c2VjcmV0", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Ok);
      QCOMPARE(r.person_id, id);
      teardown_db();
    }

    void test_basic_auth_wrong_password_returns_invalid_token()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_local_credentials(
          id, "bob", colman::hash_password("correct")));
    // "bob:wrong" base64 = Ym9iOndyb25n
      auto const r =
          auth_->authenticate("Basic Ym9iOndyb25n", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::InvalidToken);
      teardown_db();
    }

    void test_basic_auth_unknown_user_returns_invalid_token()
    {
      setup_db();
      setup_auth();
    // "nobody:pass" base64 = bm9ib2R5OnBhc3M=
      auto const r =
          auth_->authenticate("Basic bm9ib2R5OnBhc3M=", "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::InvalidToken);
      teardown_db();
    }

    void test_basic_auth_without_permission_returns_forbidden()
    {
      setup_db();
      setup_auth();
      int const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      QVERIFY(q_->set_local_credentials(
          id, "carol", colman::hash_password("pass")));
    // "carol:pass" base64 = Y2Fyb2w6cGFzcw==
      auto const r =
          auth_->authenticate("Basic Y2Fyb2w6cGFzcw==", "collect_lendings");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Forbidden);
      teardown_db();
    }

    void test_bearer_rejected_when_no_validator()
    {
      setup_db();
      colman::AuthMiddleware local_auth(*q_);
      auto const r = local_auth.authenticate(bearer(make_token()), "view_items");
      QCOMPARE(r.error, colman::AuthMiddleware::AuthError::InvalidToken);
      teardown_db();
    }

  // ----------------------------------------------------------------
  // Barcode login
  // ----------------------------------------------------------------

    void test_barcode_login_accepts_both_barcode_formats()
    {
      setup_db();
      auto col = q_->read_collection();
      col.set_barcode_prefix("54861");
      QVERIFY(q_->change_collection(col));
      auto const id = q_->add_new_person(colman::make_test_person());
      QVERIFY(id >= 0);
      colman::AuthMiddleware local_auth(*q_, true);
      auto const barcodes = {colman::compute_person_barcode(col, id),
                             colman::legacy_person_barcode("54861", id)};
      for(auto const& barcode : barcodes) {
        auto const header =
            "Basic " + QString::fromLatin1((barcode + ":").toUtf8().toBase64());
        auto const r = local_auth.authenticate(header, QString());
        QCOMPARE(r.error, colman::AuthMiddleware::AuthError::Ok);
        QCOMPARE(r.person_id, id);
      }
      teardown_db();
    }

  }; // class AuthMiddlewareTest

} // anonymous namespace

QTEST_GUILESS_MAIN(AuthMiddlewareTest)
#include "colman_auth_middleware_test.moc"

/* EOF */
