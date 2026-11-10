// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests the shutdown of the colman_rest_server executable on SIGTERM.
/**
   Starts the real server binary on a temporary database, sends SIGTERM
   (as systemctl stop does) and checks that the server exits normally and
   removes its database lock file.  A lock left behind under another host
   name would otherwise block the next start.  POSIX only.
   @file
   @author Christoph Pesch
 */

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_TEST_HELPERS_HPP
#  include "colman_test_helpers.hpp"
#endif
#ifndef SYSTEM_QPROCESS
#  include <QProcess>
#  define SYSTEM_QPROCESS
#endif
#ifndef SYSTEM_QTCPSERVER
#  include <QTcpServer>
#  define SYSTEM_QTCPSERVER
#endif
#ifndef SYSTEM_QTEMPORARYDIR
#  include <QTemporaryDir>
#  define SYSTEM_QTEMPORARYDIR
#endif

namespace
{
  /// Milliseconds to wait for the server to start or to stop.
  int constexpr TimeoutMs = 30000;

  /// Qt Test class for the shutdown behaviour of colman_rest_server.
  class RestServerShutdownTest : public QObject
  {
    Q_OBJECT

  private:
    /// Returns a TCP port that is currently free on localhost.
    quint16 free_port()
    {
      QTcpServer probe;
      if(!probe.listen(QHostAddress::LocalHost, 0)) {
        return 0;
      }
      return probe.serverPort();
    }

    /// Waits until \a process has printed \a text; returns false on timeout.
    bool wait_for_output(QProcess& process, QByteArray const& text,
                         QByteArray& output)
    {
      QDeadlineTimer deadline(TimeoutMs);
      while(!output.contains(text)) {
        if(deadline.hasExpired() || process.state() == QProcess::NotRunning) {
          return false;
        }
        process.waitForReadyRead(100);
        output += process.readAll();
      }
      return true;
    }

  private Q_SLOTS:
    /// SIGTERM ends the server normally and removes the database lock.
    void sigterm_removes_lock()
    {
      QTemporaryDir dir;
      QVERIFY(dir.isValid());
      auto const db_file = dir.filePath(QStringLiteral("shutdown.col"));
      auto const lock_file = db_file + QStringLiteral(".lock");
      {
        colman::SQLQueries queries(colman::SQLQueries::FileDb{});
        auto collection = colman::make_test_collection();
        QVERIFY(queries.create_new_database(collection, db_file));
      }
      auto const port = free_port();
      QVERIFY(port != 0);

      QProcess server;
      server.setProcessChannelMode(QProcess::MergedChannels);
      server.start(QStringLiteral(COLMAN_REST_SERVER_PATH),
                   {QStringLiteral("--db"), db_file, QStringLiteral("--port"),
                    QString::number(port)});
      QVERIFY(server.waitForStarted(TimeoutMs));
      QByteArray output;
      QVERIFY2(wait_for_output(server, "Listening on port", output),
               output.constData());
      QVERIFY(QFile::exists(lock_file));

      server.terminate(); // sends SIGTERM
      QVERIFY2(server.waitForFinished(TimeoutMs), output.constData());
      output += server.readAll();
      QVERIFY2(server.exitStatus() == QProcess::NormalExit,
               output.constData());
      QCOMPARE(server.exitCode(), 0);
      QVERIFY2(!QFile::exists(lock_file), output.constData());
    }
  };
} // namespace

QTEST_GUILESS_MAIN(RestServerShutdownTest)
#include "colman_rest_server_shutdown_test.moc"

/* EOF */
