// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class MdnsAnnouncer.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_MDNS_ANNOUNCER_HPP
#  define COLMAN_MDNS_ANNOUNCER_HPP

#  ifndef SYSTEM_QOBJECT
#    include <QObject>
#    define SYSTEM_QOBJECT
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QUDPSOCKET
#    include <QUdpSocket>
#    define SYSTEM_QUDPSOCKET
#  endif
#  ifndef SYSTEM_QHOSTADDRESS
#    include <QHostAddress>
#    define SYSTEM_QHOSTADDRESS
#  endif
#  ifndef SYSTEM_QTIMER
#    include <QTimer>
#    define SYSTEM_QTIMER
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif

namespace colman
{

/// Announces a colman REST server on the LAN via mDNS (RFC 6762).
/**
   Registers a `_colman._tcp.local.` service with the provided name and
   port.  Uses QUdpSocket multicast on 224.0.0.251:5353.  Sends an initial
   announcement on start() and re-announces periodically at half the TTL
   interval.  Responds immediately to incoming PTR queries (RFC 6762 §6).
   Queries originating from a port other than 5353 are answered with a
   unicast response sent directly to the querier (legacy unicast,
   RFC 6762 §6.7) — this makes discovery work for clients that cannot
   receive multicast reliably (e.g. phones on Wi-Fi).  Sends a goodbye
   packet (TTL=0) on stop().

   No external daemon or library is required; Qt6::Network suffices.
*/
  class MdnsAnnouncer : public QObject
  {
    Q_OBJECT
  public:
  /// Configuration for a single mDNS service announcement.
    struct Config
    {
      QString service_name;  ///< human-readable display name, e.g. "School Lib"
      QString host_name;     ///< fully-qualified hostname, e.g. "host.local"
      quint16 port = 8080;     ///< REST server port
      int ttl_secs = 120;  ///< DNS TTL; re-announce fires at ttl/2 seconds
      bool tls = false;  ///< true if the server uses HTTPS/TLS
      quint16 mdns_port = 5353;  ///< mDNS UDP port; override only in tests
    };

  /// Constructs an announcer with the given configuration.
    explicit MdnsAnnouncer(Config const& config,
                           QObject* parent = nullptr);
    ~MdnsAnnouncer() override;

  /// Joins the mDNS multicast group, sends the first announcement,
  /// and starts the periodic re-announce timer.
  /** @return false if the socket could not be bound or the multicast
      group could not be joined. */
    bool start();

  /// Sends a goodbye packet (TTL=0) and stops the re-announce timer.
    void stop();

  Q_SIGNALS:
  /// Emitted when the socket cannot be bound or multicast join fails.
    void error_occurred(QString const& message);

  private Q_SLOTS:
    void on_timer();
    void on_datagram_ready();

  private:
    void send_announcement(quint32 ttl);
    void send_legacy_unicast_response(quint16 query_id,
                                      QHostAddress const& sender,
                                      quint16 sender_port);
    QByteArray build_mdns_packet(quint32 ttl, quint16 query_id,
                                 bool legacy) const;

    Config config_;
    QUdpSocket socket_;
    QTimer timer_;
    bool started_ = false;
  };

} // namespace colman

#endif /* #ifndef COLMAN_MDNS_ANNOUNCER_HPP */

/* EOF */
