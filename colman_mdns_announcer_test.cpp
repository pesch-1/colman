// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for colman_mdns_announcer.hpp (legacy unicast responses).
/**
   Runs an MdnsAnnouncer on a non-standard port (to avoid contending
   with a system mDNS responder on 5353) and verifies that PTR queries
   sent from an ephemeral port are answered with a unicast response per
   RFC 6762 §6.7.  No multicast reception is required by these tests.
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif
#ifndef COLMAN_MDNS_ANNOUNCER_HPP
#  include "colman_mdns_announcer.hpp"
#endif
#ifndef SYSTEM_QUDPSOCKET
#  include <QUdpSocket>
#  define SYSTEM_QUDPSOCKET
#endif

namespace
{

  quint16 const TestMdnsPort = 25353;

  void write_u16(QByteArray& out, quint16 v)
  {
    out.append(static_cast<char>((v >> 8) & 0xFF));
    out.append(static_cast<char>(v & 0xFF));
  }

  void write_name(QByteArray& out, QString const& name)
  {
    for(auto const& label : name.split('.', Qt::SkipEmptyParts)) {
      auto const encoded = label.toUtf8();
      out.append(static_cast<char>(encoded.size()));
      out.append(encoded);
    }
    out.append('\0');
  }

  quint16 read_u16(QByteArray const& data, int& pos)
  {
    quint16 const v =
        (static_cast<quint8>(data[pos]) << 8) |
        static_cast<quint8>(data[pos + 1]);
    pos += 2;
    return v;
  }

  quint32 read_u32(QByteArray const& data, int& pos)
  {
    quint32 const v =
        (static_cast<quint32>(static_cast<quint8>(data[pos])) << 24) |
        (static_cast<quint32>(static_cast<quint8>(data[pos + 1])) << 16) |
        (static_cast<quint32>(static_cast<quint8>(data[pos + 2])) << 8) |
        static_cast<quint32>(static_cast<quint8>(data[pos + 3]));
    pos += 4;
    return v;
  }

/// Reads a DNS-encoded name without compression pointer support
/// (the announcer never emits compressed names).
  QString read_name(QByteArray const& data, int& pos)
  {
    QStringList labels;
    while(pos < data.size()) {
      auto const len = static_cast<quint8>(data[pos]);
      ++pos;
      if(len == 0) {
        break;
      }
      labels.append(QString::fromUtf8(data.mid(pos, len)));
      pos += len;
    }
    return labels.join('.');
  }

/// Builds a PTR query packet for the given service type.
  QByteArray build_query(quint16 id, QString const& svc_type)
  {
    QByteArray pkt;
    write_u16(pkt, id);
    write_u16(pkt, 0);      // Flags: standard query
    write_u16(pkt, 1);      // QDCOUNT
    write_u16(pkt, 0);      // ANCOUNT
    write_u16(pkt, 0);      // NSCOUNT
    write_u16(pkt, 0);      // ARCOUNT
    write_name(pkt, svc_type);
    write_u16(pkt, 12);     // QTYPE PTR
    write_u16(pkt, 0x0001); // QCLASS IN
    return pkt;
  }

} // namespace

/// Qt Test class exercising MdnsAnnouncer's query response paths.
class ColmanMdnsAnnouncerTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void test_legacy_unicast_response();
  void test_no_response_for_other_service_type();
};

/**
   Sends a PTR query for `_colman._tcp.local` from an ephemeral port and
   verifies the unicast response: echoed ID, repeated question, no
   cache-flush bits, TTL capped at 10 s, and correct PTR/SRV/TXT data.
*/
void ColmanMdnsAnnouncerTest::test_legacy_unicast_response()
{
  colman::MdnsAnnouncer::Config config;
  config.service_name = QStringLiteral("Test Library");
  config.host_name = QStringLiteral("testhost.local");
  config.port = 18080;
  config.ttl_secs = 120;
  config.tls = true;
  config.mdns_port = TestMdnsPort;
  colman::MdnsAnnouncer announcer{config};
  QVERIFY(announcer.start());

  QUdpSocket query_socket;
  QVERIFY(query_socket.bind(QHostAddress::LocalHost, 0));
  auto const query =
      build_query(0x1234, QStringLiteral("_colman._tcp.local"));
  QCOMPARE(query_socket.writeDatagram(query, QHostAddress::LocalHost,
                                      TestMdnsPort),
           static_cast<qint64>(query.size()));

  QTRY_VERIFY_WITH_TIMEOUT(query_socket.hasPendingDatagrams(), 2000);
  QByteArray response;
  response.resize(
      static_cast<int>(query_socket.pendingDatagramSize()));
  QHostAddress sender;
  quint16 sender_port = 0;
  query_socket.readDatagram(response.data(), response.size(),
                            &sender, &sender_port);
  QCOMPARE(sender_port, TestMdnsPort);
  QVERIFY(response.size() > 12);

  int pos = 0;
  QCOMPARE(read_u16(response, pos), quint16{0x1234});  // echoed ID
  auto const flags = read_u16(response, pos);
  QVERIFY(flags & 0x8000);                             // QR=1
  auto const qdcount = read_u16(response, pos);
  QCOMPARE(qdcount, quint16{1});                       // question repeated
  auto const ancount = read_u16(response, pos);
  QVERIFY(ancount >= 3);                               // PTR, SRV, TXT[, A]
  read_u16(response, pos);                             // NSCOUNT
  read_u16(response, pos);                             // ARCOUNT

  // Question section
  QCOMPARE(read_name(response, pos),
           QStringLiteral("_colman._tcp.local"));
  QCOMPARE(read_u16(response, pos), quint16{12});      // QTYPE PTR
  QCOMPARE(read_u16(response, pos), quint16{0x0001});  // QCLASS IN

  bool seen_ptr = false;
  bool seen_srv = false;
  bool seen_txt = false;
  for(int i = 0; i < static_cast<int>(ancount); ++i) {
    auto const rr_name = read_name(response, pos);
    auto const rr_type = read_u16(response, pos);
    auto const rr_class = read_u16(response, pos);
    QCOMPARE(rr_class, quint16{0x0001});               // no cache-flush
    auto const ttl = read_u32(response, pos);
    QVERIFY(ttl <= 10);                                // RFC 6762 §6.7
    auto const rdlen = read_u16(response, pos);
    auto const rdata = response.mid(pos, rdlen);
    pos += rdlen;

    if(rr_type == 12) {
      seen_ptr = true;
      QCOMPARE(rr_name, QStringLiteral("_colman._tcp.local"));
      int rpos = 0;
      QCOMPARE(read_name(rdata, rpos),
               QStringLiteral("Test Library._colman._tcp.local"));
    }
    else if(rr_type == 33) {
      seen_srv = true;
      int rpos = 0;
      read_u16(rdata, rpos);                           // Priority
      read_u16(rdata, rpos);                           // Weight
      QCOMPARE(read_u16(rdata, rpos), quint16{18080});
      QCOMPARE(read_name(rdata, rpos),
               QStringLiteral("testhost.local"));
    }
    else if(rr_type == 16) {
      seen_txt = true;
      QStringList entries;
      int rpos = 0;
      while(rpos < rdata.size()) {
        auto const len = static_cast<quint8>(rdata[rpos]);
        ++rpos;
        entries.append(QString::fromUtf8(rdata.mid(rpos, len)));
        rpos += len;
      }
      QVERIFY(entries.contains(QStringLiteral("name=Test Library")));
      QVERIFY(entries.contains(QStringLiteral("tls=1")));
    }
  }
  QVERIFY(seen_ptr);
  QVERIFY(seen_srv);
  QVERIFY(seen_txt);

  announcer.stop();
}

/// Verifies that PTR queries for foreign service types are ignored.
void ColmanMdnsAnnouncerTest::test_no_response_for_other_service_type()
{
  colman::MdnsAnnouncer::Config config;
  config.service_name = QStringLiteral("Test Library");
  config.host_name = QStringLiteral("testhost.local");
  config.port = 18080;
  config.mdns_port = TestMdnsPort;
  colman::MdnsAnnouncer announcer{config};
  QVERIFY(announcer.start());

  QUdpSocket query_socket;
  QVERIFY(query_socket.bind(QHostAddress::LocalHost, 0));
  auto const query =
      build_query(0x4321, QStringLiteral("_other._tcp.local"));
  query_socket.writeDatagram(query, QHostAddress::LocalHost,
                             TestMdnsPort);

  QTest::qWait(300);
  QVERIFY(!query_socket.hasPendingDatagrams());

  announcer.stop();
}

QTEST_GUILESS_MAIN(ColmanMdnsAnnouncerTest)
#include "colman_mdns_announcer_test.moc"

/* EOF */
