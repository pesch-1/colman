// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class MdnsBrowser.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_mdns_browser.hpp"

#ifndef SYSTEM_QMAP
#  include <QMap>
#  define SYSTEM_QMAP
#endif

namespace
{

  QHostAddress const MdnsGroup{QStringLiteral("224.0.0.251")};
  quint16 const MdnsPort = 5353;

  QString const ColmanSvcType =
      QStringLiteral("_colman._tcp.local");
  QString const ColmanSvcSuffix =
      QStringLiteral("._colman._tcp.local");

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
    if(pos + 2 > data.size()) {
      return 0;
    }
    quint16 const v =
        (static_cast<quint8>(data[pos]) << 8) |
        static_cast<quint8>(data[pos + 1]);
    pos += 2;
    return v;
  }

  quint32 read_u32(QByteArray const& data, int& pos)
  {
    if(pos + 4 > data.size()) {
      return 0;
    }
    quint32 const v =
        (static_cast<quint32>(static_cast<quint8>(data[pos])) << 24) |
        (static_cast<quint32>(static_cast<quint8>(data[pos + 1])) << 16) |
        (static_cast<quint32>(static_cast<quint8>(data[pos + 2])) << 8) |
        static_cast<quint32>(static_cast<quint8>(data[pos + 3]));
    pos += 4;
    return v;
  }

/// Reads a DNS-encoded name (with compression pointer support).
/// Updates pos to just after the name or pointer.  Returns empty on error.
  QString read_name(QByteArray const& data, int& pos)
  {
    QStringList labels;
    int cur = pos;
    int safety = 0;
    bool jumped = false;
    while(cur < data.size() && safety++ < 128) {
      auto const len = static_cast<quint8>(data[cur]);
      if(len == 0) {
        if(!jumped) {
          pos = cur + 1;
        }
        break;
      }
      if((len & 0xC0) == 0xC0) {
        if(cur + 1 >= data.size()) {
          return {};
        }
        int const offset =
            ((len & 0x3F) << 8) | static_cast<quint8>(data[cur + 1]);
        if(!jumped) {
          pos = cur + 2;
        }
        cur = offset;
        jumped = true;
        continue;
      }
      ++cur;
      if(cur + len > data.size()) {
        return {};
      }
      labels.append(QString::fromUtf8(data.mid(cur, len)));
      cur += len;
    }
    return labels.join('.');
  }

/// Parsed representation of a single DNS resource record.
  struct RR
  {
    QString name;
    quint16 type = 0;
    quint32 ttl = 0;
    QByteArray rdata;
  };

/// Parses all resource records from one section of a DNS packet.
  std::vector<RR> parse_rrs(QByteArray const& data, int& pos, int count)
  {
    std::vector<RR> result;
    for(int i = 0; i < count; ++i) {
      if(pos >= data.size()) {
        break;
      }
      RR rr;
      rr.name = read_name(data, pos);
      if(pos + 10 > data.size()) {
        break;
      }
      rr.type = read_u16(data, pos);
      pos += 2;              // CLASS (skip)
      rr.ttl = read_u32(data, pos);
      quint16 const rdlen = read_u16(data, pos);
      if(pos + rdlen > data.size()) {
        break;
      }
      rr.rdata = data.mid(pos, rdlen);
      pos += rdlen;
      result.push_back(std::move(rr));
    }
    return result;
  }

} // namespace

/// Returns the base URL, e.g. "https://host:8443" or "http://host:8080".
QString colman::MdnsBrowser::ServiceInfo::url() const
{
  auto const scheme =
      tls ? QStringLiteral("https://") : QStringLiteral("http://");
  return scheme + host + QStringLiteral(":") + QString::number(port);
}

/**
   Constructs a browser object.
   @param parent Qt parent object.
*/
colman::MdnsBrowser::MdnsBrowser(QObject* parent)
    : QObject(parent)
{
  QObject::connect(&retry_timer_, &QTimer::timeout,
                   this, &MdnsBrowser::on_retry);
  retry_timer_.setSingleShot(true);
}

colman::MdnsBrowser::~MdnsBrowser() = default;

/**
   Clears the discovered server list, sends a PTR query for
   `_colman._tcp.local.`, and schedules retries at 1 s and 3 s per
   RFC 6762 §5.2 initial query backoff.
*/
void colman::MdnsBrowser::start_browse()
{
  retry_count_ = 0;
  if(!socket_.isValid() ||
     socket_.state() != QAbstractSocket::BoundState) {
    if(!socket_.bind(QHostAddress::AnyIPv4, MdnsPort,
                     QUdpSocket::ShareAddress |
                         QUdpSocket::ReuseAddressHint)) {
      return;
    }
    socket_.joinMulticastGroup(MdnsGroup);
    QObject::connect(&socket_, &QUdpSocket::readyRead,
                     this, &MdnsBrowser::on_datagram_ready,
                     Qt::UniqueConnection);
  }
  send_ptr_query();
  retry_timer_.start(1000);
}

void colman::MdnsBrowser::on_retry()
{
  send_ptr_query();
  ++retry_count_;
  if(retry_count_ == 1) {
    retry_timer_.start(3000);
  }
}

void colman::MdnsBrowser::send_ptr_query()
{
  QByteArray pkt;
  // DNS query header
  write_u16(pkt, 0);      // ID
  write_u16(pkt, 0);      // Flags: standard query
  write_u16(pkt, 1);      // QDCOUNT
  write_u16(pkt, 0);      // ANCOUNT
  write_u16(pkt, 0);      // NSCOUNT
  write_u16(pkt, 0);      // ARCOUNT
  // Question
  write_name(pkt, ColmanSvcType);
  write_u16(pkt, 12);     // QTYPE PTR
  write_u16(pkt, 0x0001); // QCLASS IN
  socket_.writeDatagram(pkt, MdnsGroup, MdnsPort);
}

void colman::MdnsBrowser::on_datagram_ready()
{
  while(socket_.hasPendingDatagrams()) {
    QByteArray data;
    data.resize(static_cast<int>(socket_.pendingDatagramSize()));
    socket_.readDatagram(data.data(), data.size());
    handle_response(data);
  }
}

void colman::MdnsBrowser::handle_response(QByteArray const& data)
{
  if(data.size() < 12) {
    return;
  }
  int pos = 0;
  read_u16(data, pos);                       // ID
  quint16 const flags = read_u16(data, pos);
  if(!(flags & 0x8000)) {
    return;                                  // Not a response
  }
  quint16 const qdcount = read_u16(data, pos);
  quint16 const ancount = read_u16(data, pos);
  quint16 const nscount = read_u16(data, pos);
  quint16 const arcount = read_u16(data, pos);

  // Skip question section
  for(int i = 0; i < qdcount; ++i) {
    read_name(data, pos);
    pos += 4;
    if(pos > data.size()) {
      return;
    }
  }

  // Collect all resource records
  auto records = parse_rrs(data, pos,
                           ancount + nscount + arcount);

  // Collect announced IPv4 addresses: host name (lowercase) → IP
  QMap<QString, QString> a_records;
  for(auto const& rr : records) {
    if(rr.type == 1 && rr.rdata.size() == 4) {
      quint32 const v4 =
          (static_cast<quint32>(static_cast<quint8>(rr.rdata[0])) << 24) |
          (static_cast<quint32>(static_cast<quint8>(rr.rdata[1])) << 16) |
          (static_cast<quint32>(static_cast<quint8>(rr.rdata[2])) << 8) |
          static_cast<quint32>(static_cast<quint8>(rr.rdata[3]));
      a_records.insert(rr.name.toLower(), QHostAddress(v4).toString());
    }
  }

  // Build a partial ServiceInfo for each discovered instance
  QMap<QString, ServiceInfo> infos;   // key = instance name (lowercase)

  for(auto const& rr : records) {
    // SRV: instance name ends with ._colman._tcp.local
    if(rr.type == 33 &&
       rr.name.endsWith(ColmanSvcSuffix, Qt::CaseInsensitive)) {
      if(rr.rdata.size() < 7) {
        continue;
      }
      int rpos = 0;
      read_u16(rr.rdata, rpos);              // Priority
      read_u16(rr.rdata, rpos);              // Weight
      quint16 const port = read_u16(rr.rdata, rpos);
      QString const target = read_name(rr.rdata, rpos);
      QString host = target;
      // Strip trailing ".local" or ".local." for display
      if(host.endsWith(QStringLiteral(".local."))) {
        host.chop(7);
      }
      else if(host.endsWith(QStringLiteral(".local"))) {
        host.chop(6);
      }
      // Prefer the announced IPv4 address over the hostname:
      // resolving bare LAN hostnames or .local names is unreliable
      // on hosts without an mDNS resolver
      auto const ip = a_records.value(target.toLower());
      auto& info = infos[rr.name.toLower()];
      info.host = ip.isEmpty() ? host : ip;
      info.port = port;
    }
    // TXT: instance name ends with ._colman._tcp.local
    else if(rr.type == 16 &&
            rr.name.endsWith(ColmanSvcSuffix, Qt::CaseInsensitive)) {
      int rpos = 0;
      while(rpos < rr.rdata.size()) {
        auto const len = static_cast<quint8>(rr.rdata[rpos]);
        ++rpos;
        if(rpos + len > rr.rdata.size()) {
          break;
        }
        auto const kv =
            QString::fromUtf8(rr.rdata.mid(rpos, len));
        rpos += len;
        if(kv.startsWith(QStringLiteral("name="))) {
          auto& info = infos[rr.name.toLower()];
          info.name = kv.mid(5);
        }
        else if(kv == QStringLiteral("tls=1")) {
          auto& info = infos[rr.name.toLower()];
          info.tls = true;
        }
      }
    }
  }

  // Emit signals for complete or goodbye records
  for(auto const& rr : records) {
    if(rr.type != 12) {
      continue;                              // Only PTR
    }
    if(!rr.name.endsWith(ColmanSvcType, Qt::CaseInsensitive)) {
      continue;
    }
    // rr.rdata is the encoded instance name
    int rpos = 0;
    QString const instance_name = read_name(rr.rdata, rpos);
    if(rr.ttl == 0) {
      // Goodbye packet
      Q_EMIT service_removed(instance_name);
    }
    else {
      auto it = infos.find(instance_name.toLower());
      if(it != infos.end() && !it->host.isEmpty()) {
        if(it->name.isEmpty()) {
          // Fall back to the instance part before ._colman._tcp.local
          it->name = instance_name.section(
              QStringLiteral("._colman"), 0, 0);
        }
        Q_EMIT service_found(*it);
      }
    }
  }
}

/* EOF */
