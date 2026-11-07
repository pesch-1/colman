// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Concurrency stress tests for RestServer with many simultaneous clients.
/**
   Each test method spins up a dedicated ServerThread (QHttpServer + event
   loop) and N WorkerThreads that each own their own RestQueries instance.
   The server runs in its own thread so its event loop processes TCP
   connections even while the main thread blocks in QThread::wait().
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef SYSTEM_QTHREAD
#  include <QThread>
#  define SYSTEM_QTHREAD
#endif
#ifndef SYSTEM_QSEMAPHORE
#  include <QSemaphore>
#  define SYSTEM_QSEMAPHORE
#endif
#ifndef SYSTEM_ATOMIC
#  include <atomic>
#  define SYSTEM_ATOMIC
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef THIRDPARTY_JWT_CPP_JWT_H
#  include <jwt-cpp/jwt.h>
#  define THIRDPARTY_JWT_CPP_JWT_H
#endif
#ifndef COLMAN_REST_QUERIES_HPP
#  include "colman_rest_queries.hpp"
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
// ServerThread
// ---------------------------------------------------------------------------

/// Runs a RestServer in its own thread so its event loop stays alive
/// while the main thread blocks in QThread::wait().
  class ServerThread : public QThread
  {
  public:
    explicit ServerThread(QString const& conn_name,
                          QObject* parent = nullptr)
        : QThread(parent), conn_name_(conn_name)
    {}

  /// Blocks until the server is ready; returns the listening port (0 = error).
    quint16 wait_for_port()
    {
      ready_.acquire(1);
      return port_.load();
    }

  /// Returns the type_id that was registered during setup.
    int type_id() const
    {
      return type_id_;
    }

  protected:
    void run() override
    {
      colman::SQLQueries sql(conn_name_);
      if(!sql.create_tables()) {
        ready_.release(1);
        return;
      }
      if(!sql.create_collection(colman::make_test_collection())) {
        ready_.release(1);
        return;
      }
      if(!sql.add_administrator()) {
        ready_.release(1);
        return;
      }
      if(!sql.set_person_external_id(0, QString::fromUtf8(kSubject))) {
        ready_.release(1);
        return;
      }
      for(auto const& p : sql.get_permissions()) {
        sql.set_permission(0, p, true);
      }
      type_id_ = sql.add_new_type(colman::make_book_type());

      colman::JwtValidator::Config vcfg;
      vcfg.issuer = QString::fromUtf8(kIssuer);
      colman::JwtValidator validator(vcfg);
      validator.load_jwks(QString::fromUtf8(kTestJwks));

      colman::AuthMiddleware auth(validator, sql);
      colman::RestServer srv(sql, auth);
      auto const port = srv.listen_on_local_port();
      port_.store(port);
      ready_.release(1);
      if(port == 0) {
        return;
      }
      exec();
    }

  private:
    QString conn_name_;
    QSemaphore ready_{0};
    std::atomic<quint16> port_{0};
    int type_id_ = -1;
  };

// ---------------------------------------------------------------------------
// WriteWorker
// ---------------------------------------------------------------------------

/// Worker thread that repeatedly performs a 5-step write sequence:
/// add_person, add_item, acquire, lend, return.
  class WriteWorker : public QThread
  {
  public:
    WriteWorker(colman::RestConfig const& cfg,
                int type_id,
                int iters,
                QObject* parent = nullptr)
        : QThread(parent), cfg_(cfg), type_id_(type_id), iters_(iters)
    {}

  /// Returns the number of failed write sequences.
    int errors() const
    {
      return errors_.load();
    }

  /// Returns the number of fully successful write sequences.
    int ops() const
    {
      return ops_.load();
    }

  protected:
    void run() override
    {
      colman::RestQueries client;
      client.set_config(cfg_);
      for(int i = 0; i < iters_; ++i) {
        auto const person_id = client.add_new_person(
            colman::make_test_person());
        if(person_id < 0) {
          errors_.fetch_add(1);
          continue;
        }
        colman::Item item;
        item.set_type_id(type_id_);
        item.set_name("StressBook");
        auto const item_id = client.add_new_item(item);
        if(item_id < 0) {
          errors_.fetch_add(1);
          continue;
        }
        colman::Acquisition acq;
        acq.set_item_id(item_id);
        acq.set_count(5u);
        acq.set_date_time(QDateTime::currentDateTimeUtc());
        auto const acq_id = client.add_new_acquisition(acq);
        if(acq_id < 0) {
          errors_.fetch_add(1);
          continue;
        }
        colman::Lending lending;
        lending.set_item_id(item_id);
        lending.set_person_id(person_id);
        lending.set_count(3u);
        lending.set_date_time(QDateTime::currentDateTimeUtc());
        auto const lending_id = client.add_new_lending(lending);
        if(lending_id < 0) {
          errors_.fetch_add(1);
          continue;
        }
        colman::Return ret;
        ret.set_lending_id(lending_id);
        ret.set_count(2u);
        ret.set_date_time(QDateTime::currentDateTimeUtc());
        auto const return_id = client.add_new_return(ret);
        if(return_id < 0) {
          errors_.fetch_add(1);
          continue;
        }
        ops_.fetch_add(1);
      }
    }

  private:
    colman::RestConfig cfg_;
    int type_id_;
    int iters_;
    std::atomic<int> errors_{0};
    std::atomic<int> ops_{0};
  };

// ---------------------------------------------------------------------------
// ReadWorker
// ---------------------------------------------------------------------------

/// Worker thread that repeatedly issues three read queries.
  class ReadWorker : public QThread
  {
  public:
    ReadWorker(colman::RestConfig const& cfg,
               int iters,
               QObject* parent = nullptr)
        : QThread(parent), cfg_(cfg), iters_(iters)
    {}

  /// Returns the number of completed read operations.
    int ops() const
    {
      return ops_.load();
    }

  protected:
    void run() override
    {
      colman::RestQueries client;
      client.set_config(cfg_);
      for(int i = 0; i < iters_; ++i) {
        client.query_items(-1, QString());
        ops_.fetch_add(1);
        client.query_persons(-1, QString());
        ops_.fetch_add(1);
        client.query_combined_history(true);
        ops_.fetch_add(1);
      }
    }

  private:
    colman::RestConfig cfg_;
    int iters_;
    std::atomic<int> ops_{0};
  };

// ---------------------------------------------------------------------------
// Test class
// ---------------------------------------------------------------------------

/// Concurrency stress tests for RestServer.
  class RestStressTest : public QObject
  {
    Q_OBJECT

  public:
    RestStressTest() = default;

  private Q_SLOTS:

  // ==================================================================
  // concurrent_reads
  // ==================================================================

    void concurrent_reads()
    {
      static int counter = 0;
      QString const conn_name =
          QString("stress_test_%1").arg(counter++);

      ServerThread srv(conn_name);
      srv.start();
      auto const port = srv.wait_for_port();
      QVERIFY(port > 0);

    // Pre-populate with 10 items and 10 persons from the main thread.
      colman::RestConfig cfg;
      cfg.base_url = QString("http://127.0.0.1:%1").arg(port);
      cfg.token = QString::fromStdString(make_token());
      cfg.timeout_ms = 10000;

      {
        colman::RestQueries setup_client;
        setup_client.set_config(cfg);
        auto const type_id = srv.type_id();
        QVERIFY(type_id >= 0);
        for(int i = 0; i < 10; ++i) {
          colman::Item item;
          item.set_type_id(type_id);
          item.set_name(QString("PreBook%1").arg(i));
          QVERIFY(setup_client.add_new_item(item) >= 0);
          QVERIFY(
              setup_client.add_new_person(colman::make_test_person()) >= 0);
        }
      }

      int const kThreads = 8;
      int const kIters = 400;
      std::vector<std::unique_ptr<ReadWorker>> workers;
      workers.reserve(static_cast<std::size_t>(kThreads));
      for(int t = 0; t < kThreads; ++t) {
        workers.push_back(
            std::make_unique<ReadWorker>(cfg, kIters));
      }
      for(auto const& w : workers) {
        w->start();
      }
      for(auto const& w : workers) {
        w->wait();
      }

      int total_ops = 0;
      for(auto const& w : workers) {
        total_ops += w->ops();
      }
      QCOMPARE(total_ops, kThreads * kIters * 3);

      srv.quit();
      srv.wait();
    }

  // ==================================================================
  // concurrent_independent_writes
  // ==================================================================

    void concurrent_independent_writes()
    {
      static int counter = 0;
      QString const conn_name =
          QString("stress_test_%1").arg(counter++);

      ServerThread srv(conn_name);
      srv.start();
      auto const port = srv.wait_for_port();
      QVERIFY(port > 0);

      colman::RestConfig cfg;
      cfg.base_url = QString("http://127.0.0.1:%1").arg(port);
      cfg.token = QString::fromStdString(make_token());
      cfg.timeout_ms = 30000;

      int const kThreads = 8;
      int const kIters = 150;
      auto const type_id = srv.type_id();
      QVERIFY(type_id >= 0);

      std::vector<std::unique_ptr<WriteWorker>> workers;
      workers.reserve(static_cast<std::size_t>(kThreads));
      for(int t = 0; t < kThreads; ++t) {
        workers.push_back(
            std::make_unique<WriteWorker>(cfg, type_id, kIters));
      }
      for(auto const& w : workers) {
        w->start();
      }
      for(auto const& w : workers) {
        QVERIFY(w->wait(120000));
      }

      int total_errors = 0;
      int total_ops = 0;
      for(auto const& w : workers) {
        total_errors += w->errors();
        total_ops += w->ops();
      }
      QCOMPARE(total_errors, 0);
      QCOMPARE(total_ops, kThreads * kIters);

    // Verify via a fresh client that persons were actually created.
      colman::RestQueries verify_client;
      verify_client.set_config(cfg);
      auto const persons = verify_client.query_persons(-1, QString());
      QVERIFY(static_cast<int>(persons.size()) >= kThreads * kIters);

      srv.quit();
      srv.wait();
    }

  // ==================================================================
  // concurrent_mixed
  // ==================================================================

    void concurrent_mixed()
    {
      static int counter = 0;
      QString const conn_name =
          QString("stress_test_%1").arg(counter++);

      ServerThread srv(conn_name);
      srv.start();
      auto const port = srv.wait_for_port();
      QVERIFY(port > 0);

      colman::RestConfig cfg;
      cfg.base_url = QString("http://127.0.0.1:%1").arg(port);
      cfg.token = QString::fromStdString(make_token());
      cfg.timeout_ms = 30000;

      int const kWriteThreads = 4;
      int const kReadThreads = 4;
      int const kWriteIters = 100;
      int const kReadIters = 200;
      auto const type_id = srv.type_id();
      QVERIFY(type_id >= 0);

      std::vector<std::unique_ptr<WriteWorker>> writers;
      writers.reserve(static_cast<std::size_t>(kWriteThreads));
      std::vector<std::unique_ptr<ReadWorker>> readers;
      readers.reserve(static_cast<std::size_t>(kReadThreads));

      for(int t = 0; t < kWriteThreads; ++t) {
        writers.push_back(
            std::make_unique<WriteWorker>(cfg, type_id, kWriteIters));
      }
      for(int t = 0; t < kReadThreads; ++t) {
        readers.push_back(
            std::make_unique<ReadWorker>(cfg, kReadIters));
      }

      for(auto const& w : writers) {
        w->start();
      }
      for(auto const& r : readers) {
        r->start();
      }
      for(auto const& w : writers) {
        w->wait();
      }
      for(auto const& r : readers) {
        r->wait();
      }

      int write_errors = 0;
      int write_ops = 0;
      int read_ops = 0;
      for(auto const& w : writers) {
        write_errors += w->errors();
        write_ops += w->ops();
      }
      for(auto const& r : readers) {
        read_ops += r->ops();
      }
      QCOMPARE(write_errors, 0);
      QCOMPARE(write_ops, kWriteThreads * kWriteIters);
      QCOMPARE(read_ops, kReadThreads * kReadIters * 3);

      srv.quit();
      srv.wait();
    }

  }; // class RestStressTest

} // anonymous namespace

QTEST_MAIN(RestStressTest)
#include "colman_rest_stress_test.moc"

/* EOF */
