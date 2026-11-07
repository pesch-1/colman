// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class MdnsAnnouncer.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_mdns_announcer.hpp"

#ifndef SYSTEM_QHOSTADDRESS
#  include <QHostAddress>
#  define SYSTEM_QHOSTADDRESS
#endif
#ifndef SYSTEM_QNETWORKINTERFACE
#  include <QNetworkInterface>
#  define SYSTEM_QNETWORKINTERFACE
#endif
#ifndef SYSTEM_QABSTRACTSOCKET
#  include <QAbstractSocket>
#  define SYSTEM_QABSTRACTSOCKET
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{

  QHostAddress const MdnsGroup{QStringLiteral("224.0.0.251")};

  void write_u16(QByteArray& out, quint16 v)
  {
    out.append(static_cast<char>((v >> 8) & 0xFF));
    out.append(static_cast<char>(v & 0xFF));
  }

  void write_u32(QByteArray& out, quint32 v)
  {
    out.append(static_cast<char>((v >> 24) & 0xFF));
    out.append(static_cast<char>((v >> 16) & 0xFF));
    out.append(static_cast<char>((v >> 8) & 0xFF));
    out.append(static_cast<char>(v & 0xFF));
  }

/// Appends a DNS-encoded domain name (labels separated by dots, no compression).
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
    if(pos + 2 > data.size()) {
      return 0;
    }
    auto const v = static_cast<quint16>(
        (static_cast<quint8>(data[pos]) << 8) |
        static_cast<quint8>(data[pos + 1]));
    pos += 2;
    return v;
  }

/// Reads a DNS-encoded name (labels only, no compression pointer resolution).
  QString read_dns_name(QByteArray const& data, int& pos)
  {
    QStringList labels;
    while(pos < data.size()) {
      auto const len = static_cast<quint8>(data[pos]);
      if(len == 0) {
        ++pos;
        break;
      }
      if((len & 0xC0) == 0xC0) { // compression pointer — skip
        pos += 2;
        break;
      }
      ++pos;
      if(pos + len > data.size()) {
        break;
      }
      labels.append(QString::fromUtf8(data.mid(pos, len)));
      pos += len;
    }
    return labels.join('.');
  }

/// Returns the first non-loopback IPv4 address of this host, or a null address.
  QHostAddress local_ipv4()
  {
    for(auto const& addr : QNetworkInterface::allAddresses()) {
      if(addr.protocol() == QAbstractSocket::IPv4Protocol &&
         !addr.isLoopback()) {
        return addr;
      }
    }
    return {};
  }

} // namespace

/**
   Constructs an announcer with the given configuration.
   @param config service name, hostname, port, and TTL settings.
   @param parent Qt parent object.
*/
colman::MdnsAnnouncer::MdnsAnnouncer(Config const& config,
                                     QObject* parent)
    : QObject(parent), config_(config)
{
  QObject::connect(&timer_, &QTimer::timeout,
                   this, &MdnsAnnouncer::on_timer);
}

colman::MdnsAnnouncer::~MdnsAnnouncer()
{
  if(started_) {
    stop();
  }
}

/**
   Joins the mDNS multicast group, sends the first announcement, and starts
   the periodic re-announce timer (fires at half the TTL interval).
   @return false if the socket cannot be bound or the multicast join fails.
*/
bool colman::MdnsAnnouncer::start()
{
  if(started_) {
    return true;
  }
  if(!socket_.bind(QHostAddress::AnyIPv4, config_.mdns_port,
                   QUdpSocket::ShareAddress |
                       QUdpSocket::ReuseAddressHint)) {
    Q_EMIT error_occurred(
        QStringLiteral("mDNS: cannot bind port ") +
        QString::number(config_.mdns_port) + QStringLiteral(": ") +
        socket_.errorString());
    return false;
  }
  if(!socket_.joinMulticastGroup(MdnsGroup)) {
    Q_EMIT error_occurred(
        QStringLiteral("mDNS: cannot join multicast group: ") +
        socket_.errorString());
    return false;
  }
  QObject::connect(&socket_, &QUdpSocket::readyRead,
                   this, &MdnsAnnouncer::on_datagram_ready);
  started_ = true;
  send_announcement(static_cast<quint32>(config_.ttl_secs));
  timer_.start((config_.ttl_secs / 2) * 1000);
  return true;
}

/// Sends a goodbye packet (TTL=0) and stops the re-announce timer.
void colman::MdnsAnnouncer::stop()
{
  if(!started_) {
    return;
  }
  timer_.stop();
  send_announcement(0);
  socket_.leaveMulticastGroup(MdnsGroup);
  socket_.close();
  started_ = false;
}

void colman::MdnsAnnouncer::on_timer()
{
  send_announcement(static_cast<quint32>(config_.ttl_secs));
}

/**
   Reads incoming mDNS datagrams and responds to PTR queries for
   `_colman._tcp.local` with a full announcement (RFC 6762 §6).
   Queries sent from a port other than 5353 receive a unicast response
   addressed directly to the querier (legacy unicast, RFC 6762 §6.7).
*/
void colman::MdnsAnnouncer::on_datagram_ready()
{
  while(socket_.hasPendingDatagrams()) {
    QByteArray data;
    data.resize(static_cast<int>(socket_.pendingDatagramSize()));
    QHostAddress sender;
    quint16 sender_port = 0;
    socket_.readDatagram(data.data(), data.size(), &sender,
                         &sender_port);
    if(data.size() < 12) {
      continue;
    }
    int pos = 0;
    auto const query_id = read_u16(data, pos);
    auto const flags = read_u16(data, pos);
    if(flags & 0x8000) {
      continue;                             // response packet — ignore
    }
    auto const qdcount = read_u16(data, pos);
    read_u16(data, pos);                    // ancount
    read_u16(data, pos);                    // nscount
    read_u16(data, pos);                    // arcount
    for(int i = 0; i < static_cast<int>(qdcount); ++i) {
      auto const name = read_dns_name(data, pos);
      auto const qtype = read_u16(data, pos);
      read_u16(data, pos);                  // qclass
      if(qtype == 12 &&
         name == QStringLiteral("_colman._tcp.local")) {
        if(sender_port == config_.mdns_port) {
          send_announcement(static_cast<quint32>(config_.ttl_secs));
        }
        else {
          send_legacy_unicast_response(query_id, sender, sender_port);
        }
        break;
      }
    }
  }
}

void colman::MdnsAnnouncer::send_announcement(quint32 ttl)
{
  auto const pkt = build_mdns_packet(ttl, 0, false);
  socket_.writeDatagram(pkt, MdnsGroup, config_.mdns_port);
}

/**
   Sends a unicast response directly to a legacy querier
   (RFC 6762 §6.7): the query ID is echoed, the question is repeated,
   the cache-flush bit is not set, and the TTL does not exceed 10 s.
   @param query_id ID field of the query being answered.
   @param sender source address of the query.
   @param sender_port source port of the query (not 5353).
*/
void colman::MdnsAnnouncer::send_legacy_unicast_response(
    quint16 query_id, QHostAddress const& sender, quint16 sender_port)
{
  auto const ttl =
      static_cast<quint32>(std::min(config_.ttl_secs, 10));
  auto const pkt = build_mdns_packet(ttl, query_id, true);
  socket_.writeDatagram(pkt, sender, sender_port);
}

/**
   Builds a DNS response packet containing PTR, SRV, TXT, and A records
   for the configured service.  If no non-loopback IPv4 address is found,
   the A record is omitted.
   @param ttl record TTL in seconds.
   @param query_id ID to place in the header (0 for multicast responses).
   @param legacy if true, builds a legacy unicast response
          (RFC 6762 §6.7): the question is repeated in the question
          section and the cache-flush bit is not set.
*/
QByteArray colman::MdnsAnnouncer::build_mdns_packet(
    quint32 ttl, quint16 query_id, bool legacy) const
{
  QString const svc_type =
      QStringLiteral("_colman._tcp.local");
  QString const instance =
      config_.service_name +
      QStringLiteral("._colman._tcp.local");

  QHostAddress const ip = local_ipv4();
  bool const has_ip = !ip.isNull();
  quint16 const rr_class = legacy ? 0x0001 : 0x8001;

  QByteArray pkt;
  // DNS header
  write_u16(pkt, query_id);                               // ID
  write_u16(pkt, 0x8400);                                 // QR=1 AA=1
  write_u16(pkt, static_cast<quint16>(legacy ? 1 : 0));  // QDCOUNT
  write_u16(pkt, static_cast<quint16>(has_ip ? 4 : 3));  // ANCOUNT
  write_u16(pkt, 0);                                      // NSCOUNT
  write_u16(pkt, 0);                                      // ARCOUNT

  // Question (legacy unicast responses repeat the question)
  if(legacy) {
    write_name(pkt, svc_type);
    write_u16(pkt, 12);     // QTYPE PTR
    write_u16(pkt, 0x0001); // QCLASS IN
  }

  // PTR: _colman._tcp.local. → <instance>._colman._tcp.local.
  write_name(pkt, svc_type);
  write_u16(pkt, 12);     // TYPE PTR
  write_u16(pkt, 0x0001); // CLASS IN
  write_u32(pkt, ttl);
  {
    QByteArray rdata;
    write_name(rdata, instance);
    write_u16(pkt, static_cast<quint16>(rdata.size()));
    pkt.append(rdata);
  }

  // SRV: <instance>. → 0 0 port host.local.
  write_name(pkt, instance);
  write_u16(pkt, 33);     // TYPE SRV
  write_u16(pkt, rr_class); // CLASS IN (+ cache-flush unless legacy)
  write_u32(pkt, ttl);
  {
    QByteArray rdata;
    write_u16(rdata, 0);              // Priority
    write_u16(rdata, 0);              // Weight
    write_u16(rdata, config_.port);   // Port
    write_name(rdata, config_.host_name);
    write_u16(pkt, static_cast<quint16>(rdata.size()));
    pkt.append(rdata);
  }

  // TXT: <instance>. → "name=<service_name>" [, "tls=1"]
  write_name(pkt, instance);
  write_u16(pkt, 16);     // TYPE TXT
  write_u16(pkt, rr_class); // CLASS IN (+ cache-flush unless legacy)
  write_u32(pkt, ttl);
  {
    QByteArray rdata;
    auto const txt =
        (QStringLiteral("name=") + config_.service_name).toUtf8();
    rdata.append(static_cast<char>(txt.size()));
    rdata.append(txt);
    if(config_.tls) {
      QByteArray const tls_txt = QByteArrayLiteral("tls=1");
      rdata.append(static_cast<char>(tls_txt.size()));
      rdata.append(tls_txt);
    }
    write_u16(pkt, static_cast<quint16>(rdata.size()));
    pkt.append(rdata);
  }

  // A: host.local. → IPv4
  if(has_ip) {
    write_name(pkt, config_.host_name);
    write_u16(pkt, 1);      // TYPE A
    write_u16(pkt, rr_class); // CLASS IN (+ cache-flush unless legacy)
    write_u32(pkt, ttl);
    write_u16(pkt, 4);      // RDLENGTH
    quint32 const v4 = ip.toIPv4Address();
    pkt.append(static_cast<char>((v4 >> 24) & 0xFF));
    pkt.append(static_cast<char>((v4 >> 16) & 0xFF));
    pkt.append(static_cast<char>((v4 >> 8) & 0xFF));
    pkt.append(static_cast<char>(v4 & 0xFF));
  }

  return pkt;
}

/* EOF */
