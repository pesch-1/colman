// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for the built-in SQLite driver (colman_sqlite_driver.hpp).
/**
   Checks value binding and conversion, result navigation, transactions,
   schema queries and encryption with key changes.
   @file
   @author Christoph Pesch
 */

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_SQLITE_DRIVER_HPP
#  include "colman_sqlite_driver.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QSQLDATABASE
#  include <QSqlDatabase>
#  define SYSTEM_QSQLDATABASE
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSQLRECORD
#  include <QSqlRecord>
#  define SYSTEM_QSQLRECORD
#endif
#ifndef SYSTEM_QTEMPORARYDIR
#  include <QTemporaryDir>
#  define SYSTEM_QTEMPORARYDIR
#endif

namespace
{
  /// Passphrase with non-ASCII characters (must be passed as UTF-8).
  char constexpr Key[] = "Geheim äöü €";

  /// Qt Test class for the built-in SQLite driver.
  class SqliteDriverTest : public QObject
  {
    Q_OBJECT

  private:
    int conn_counter_ = 0; ///< monotone counter for unique connection names

    /// Opens a connection to \a file (default: in-memory database).
    QSqlDatabase open_db(QString const& file = QStringLiteral(":memory:"))
    {
      colman::register_sqlite_driver();
      auto db = QSqlDatabase::addDatabase(
          colman::SqliteDriverName,
          QStringLiteral("sqlite_driver_test_%1").arg(conn_counter_++));
      db.setDatabaseName(file);
      if(!db.open()) {
        qWarning() << db.lastError();
      }
      return db;
    }

    /// Closes \a db and removes its connection.
    void close_db(QSqlDatabase& db)
    {
      QString const name = db.connectionName();
      db.close();
      db = QSqlDatabase();
      QSqlDatabase::removeDatabase(name);
    }

    /// Returns the number of rows in table \a table, or -1 on error.
    int count_rows(QSqlDatabase const& db, QString const& table)
    {
      QSqlQuery query(db);
      if(!query.exec(QStringLiteral("SELECT count(*) FROM ") + table) ||
         !query.next())
      {
        return -1;
      }
      return query.value(0).toInt();
    }

  private Q_SLOTS:
    void test_driver_is_registered()
    {
      colman::register_sqlite_driver();
      colman::register_sqlite_driver();
      QVERIFY(QSqlDatabase::isDriverAvailable(colman::SqliteDriverName));
    }

    void test_values_round_trip()
    {
      auto db = open_db();
      QVERIFY(db.isOpen());
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE t (id INTEGER PRIMARY KEY, "
                         "s TEXT, n INTEGER, d REAL, b BLOB)"));
      QByteArray const blob("a\0b", 3);
      QVERIFY(query.prepare(
          "INSERT INTO t (s, n, d, b) VALUES (:s, :n, :d, :b)"));
      query.bindValue(":s", QStringLiteral("Grüße"));
      query.bindValue(":n", 4711);
      query.bindValue(":d", 2.5);
      query.bindValue(":b", blob);
      QVERIFY(query.exec());
      QCOMPARE(query.lastInsertId().toInt(), 1);
      QCOMPARE(query.numRowsAffected(), 1);

      QVERIFY(query.exec("SELECT s, n, d, b FROM t"));
      QVERIFY(query.isSelect());
      QVERIFY(query.next());
      QCOMPARE(query.value(0).toString(), QStringLiteral("Grüße"));
      QCOMPARE(query.value(1).toLongLong(), 4711);
      QCOMPARE(query.value(2).toDouble(), 2.5);
      QCOMPARE(query.value(3).toByteArray(), blob);
      QVERIFY(!query.next());
      close_db(db);
    }

    void test_null_values()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE t (s TEXT, e TEXT)"));
      QVERIFY(query.prepare("INSERT INTO t (s, e) VALUES (?, ?)"));
      query.addBindValue(QString());
      query.addBindValue(QStringLiteral(""));
      QVERIFY(query.exec());
      QVERIFY(query.exec("SELECT s, e, s IS NULL, e IS NULL FROM t"));
      QVERIFY(query.next());
      QVERIFY(query.isNull(0));
      QVERIFY(query.value(0).isNull());
      QVERIFY(!query.isNull(1));
      QCOMPARE(query.value(2).toInt(), 1);
      QCOMPARE(query.value(3).toInt(), 0);
      close_db(db);
    }

    void test_repeated_named_placeholder()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.prepare("SELECT :x || '-' || :x"));
      query.bindValue(":x", QStringLiteral("ab"));
      QVERIFY(query.exec());
      QVERIFY(query.next());
      QCOMPARE(query.value(0).toString(), QStringLiteral("ab-ab"));
      close_db(db);
    }

    void test_date_time_formats()
    {
      // Same text formats as Qt's QSQLITE driver.
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.prepare("SELECT ?, ?"));
      query.addBindValue(QDateTime(QDate(2026, 1, 2), QTime(3, 4, 5, 6)));
      query.addBindValue(QDate(2026, 1, 2));
      QVERIFY(query.exec());
      QVERIFY(query.next());
      QCOMPARE(query.value(0).toString(),
               QStringLiteral("2026-01-02T03:04:05.006"));
      QCOMPARE(query.value(1).toString(), QStringLiteral("2026-01-02"));
      close_db(db);
    }

    void test_navigation()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE t (n INTEGER)"));
      QVERIFY(query.exec("INSERT INTO t VALUES (10), (20), (30)"));
      QVERIFY(query.exec("SELECT n FROM t ORDER BY n"));
      QVERIFY(query.next());
      QVERIFY(query.next());
      QCOMPARE(query.value(0).toInt(), 20);
      QVERIFY(query.first());
      QCOMPARE(query.value(0).toInt(), 10);
      QVERIFY(query.seek(2));
      QCOMPARE(query.value(0).toInt(), 30);
      QVERIFY(query.previous());
      QCOMPARE(query.value(0).toInt(), 20);
      QVERIFY(query.last());
      QCOMPARE(query.at(), 2);
      QCOMPARE(query.value(0).toInt(), 30);
      QVERIFY(!query.next());
      QCOMPARE(query.size(), -1);
      close_db(db);
    }

    void test_errors()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(!query.exec("SELECT 1; SELECT 2"));
      QVERIFY(!query.exec("SELECT * FROM missing_table"));
      QVERIFY(query.lastError().isValid());
      QVERIFY(query.exec("CREATE TABLE t (n INTEGER UNIQUE)"));
      QVERIFY(query.exec("INSERT INTO t VALUES (1)"));
      QVERIFY(!query.exec("INSERT INTO t VALUES (1)"));
      QVERIFY(query.lastError().databaseText().contains("UNIQUE"));
      close_db(db);
    }

    void test_transactions()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE t (n INTEGER)"));
      QVERIFY(db.transaction());
      QVERIFY(query.exec("INSERT INTO t VALUES (1)"));
      QVERIFY(db.rollback());
      QCOMPARE(count_rows(db, "t"), 0);
      QVERIFY(db.transaction());
      QVERIFY(query.exec("INSERT INTO t VALUES (1)"));
      QVERIFY(db.commit());
      QCOMPARE(count_rows(db, "t"), 1);
      close_db(db);
    }

    void test_schema()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE items (id INTEGER, name TEXT)"));
      QVERIFY(query.exec("CREATE VIEW v AS SELECT name FROM items"));
      QVERIFY(db.tables().contains("items"));
      QVERIFY(!db.tables().contains("v"));
      QVERIFY(db.tables(QSql::Views).contains("v"));
      QSqlRecord const rec = db.record("items");
      QCOMPARE(rec.count(), 2);
      QCOMPARE(rec.fieldName(0), QStringLiteral("id"));
      QCOMPARE(rec.fieldName(1), QStringLiteral("name"));
      QCOMPARE(db.record("missing").count(), 0);
      QVERIFY(query.exec("SELECT name, id FROM items"));
      QCOMPARE(query.record().fieldName(0), QStringLiteral("name"));
      close_db(db);
    }

    void test_fts5_available()
    {
      auto db = open_db();
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE VIRTUAL TABLE f USING fts5(text)"));
      close_db(db);
    }

    void test_encryption()
    {
      QTemporaryDir dir;
      QVERIFY(dir.isValid());
      QString const file = dir.filePath("test.col");
      QString const key = QString::fromUtf8(Key);
      QString const new_key = QStringLiteral("other key");

      // Create, fill and encrypt.
      auto db = open_db(file);
      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE t (n INTEGER)"));
      QVERIFY(query.exec("INSERT INTO t VALUES (42)"));
      query = QSqlQuery();
      QVERIFY(!colman::is_file_encrypted(file));
      QVERIFY(colman::change_database_key(db, key));
      close_db(db);
      QVERIFY(colman::is_file_encrypted(file));

      // Without or with a wrong key the data cannot be read.
      db = open_db(file);
      QCOMPARE(count_rows(db, "t"), -1);
      close_db(db);
      db = open_db(file);
      QVERIFY(!colman::set_database_key(db, "wrong"));
      close_db(db);
      db = open_db(file);
      QVERIFY(!colman::set_database_key(db, key.left(key.size() - 1)));
      close_db(db);

      // The right key unlocks it; then change the key.
      db = open_db(file);
      QVERIFY(colman::set_database_key(db, key));
      QCOMPARE(count_rows(db, "t"), 1);
      QVERIFY(colman::change_database_key(db, new_key));
      close_db(db);
      db = open_db(file);
      QVERIFY(!colman::set_database_key(db, key));
      close_db(db);
      db = open_db(file);
      QVERIFY(colman::set_database_key(db, new_key));
      QCOMPARE(count_rows(db, "t"), 1);

      // Remove the encryption.
      QVERIFY(colman::change_database_key(db, QString()));
      close_db(db);
      QVERIFY(!colman::is_file_encrypted(file));
      db = open_db(file);
      QCOMPARE(count_rows(db, "t"), 1);
      close_db(db);
    }

    void test_collection_encryption()
    {
      // The encryption workflow of the user interface, via SQLQueries.
      QTemporaryDir dir;
      QVERIFY(dir.isValid());
      QString const file = dir.filePath("collection.col");
      QString const key = QString::fromUtf8(Key);
      QString const new_key = QStringLiteral("new key");
      {
        colman::SQLQueries q(colman::SQLQueries::FileDb{});
        auto col = colman::make_test_collection();
        QVERIFY(q.create_new_database(col, file, key));
        QVERIFY(q.supports_encryption());
        QVERIFY(q.is_encrypted());
        QVERIFY(!q.encrypt_database(new_key));
      }
      QVERIFY(colman::is_file_encrypted(file));
      {
        colman::SQLQueries q(colman::SQLQueries::FileDb{});
        QVERIFY(!q.load_database(file));
      }
      {
        colman::SQLQueries q(colman::SQLQueries::FileDb{});
        QVERIFY(!q.load_database(file, new_key));
      }
      {
        colman::SQLQueries q(colman::SQLQueries::FileDb{});
        QVERIFY(q.load_database(file, key));
        QVERIFY(q.is_encrypted());
        QVERIFY(!q.change_encryption_key(new_key, new_key));
        QVERIFY(q.change_encryption_key(key, new_key));
        QVERIFY(!q.remove_encryption(key));
      }
      {
        colman::SQLQueries q(colman::SQLQueries::FileDb{});
        QVERIFY(q.load_database(file, new_key));
        QVERIFY(q.remove_encryption(new_key));
        QVERIFY(!q.is_encrypted());
      }
      QVERIFY(!colman::is_file_encrypted(file));
      {
        colman::SQLQueries q(colman::SQLQueries::FileDb{});
        QVERIFY(q.load_database(file));
        QVERIFY(q.encrypt_database(key));
      }
      QVERIFY(colman::is_file_encrypted(file));
    }
  };

} // anonymous namespace

QTEST_GUILESS_MAIN(SqliteDriverTest)
#include "colman_sqlite_driver_test.moc"

/* EOF */
