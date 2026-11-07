// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class MdnsBrowser.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_MDNS_BROWSER_HPP
#  define COLMAN_MDNS_BROWSER_HPP

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
#  ifndef SYSTEM_QTIMER
#    include <QTimer>
#    define SYSTEM_QTIMER
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QHOSTADDRESS
#    include <QHostAddress>
#    define SYSTEM_QHOSTADDRESS
#  endif

namespace colman
{

/// Discovers colman REST servers on the LAN via mDNS (RFC 6762).
/**
   Sends a PTR query for `_colman._tcp.local.` on the mDNS multicast
   address (224.0.0.251:5353) and listens for responses.  For each
   complete response containing SRV (host+port) and TXT (name=) records
   for a `_colman._tcp.local.` instance, service_found() is emitted.
   When a goodbye packet (TTL=0) arrives, service_removed() is emitted.

   No external daemon or library is required; Qt6::Network suffices.
*/
  class MdnsBrowser : public QObject
  {
    Q_OBJECT
  public:
  /// Information about a discovered colman REST server.
    struct ServiceInfo
    {
      QString name;    ///< display name from the TXT "name=" record
      QString host;    ///< host from the SRV record
      quint16 port = 0;  ///< port from the SRV record
      bool tls = false;  ///< true if the server advertises TXT "tls=1"
    /// Returns the base URL, e.g. "https://host:8443" or "http://host:8080".
      QString url() const;
    };

    explicit MdnsBrowser(QObject* parent = nullptr);
    ~MdnsBrowser() override;

  /// Clears the discovered server list, sends a PTR query, and listens for
  /// responses.  Resends the query at 1 s and 3 s (RFC 6762 §5.2 backoff).
    void start_browse();

  Q_SIGNALS:
  /// Emitted when a new (or updated) service is fully resolved.
    void service_found(colman::MdnsBrowser::ServiceInfo const& info);
  /// Emitted when a service sends a goodbye packet (TTL=0).
    void service_removed(QString const& name);

  private Q_SLOTS:
    void on_datagram_ready();
    void on_retry();

  private:
    void send_ptr_query();
    void handle_response(QByteArray const& data);

    QUdpSocket socket_;
    QTimer retry_timer_;
    int retry_count_ = 0;
  };

} // namespace colman

Q_DECLARE_METATYPE(colman::MdnsBrowser::ServiceInfo)

#endif /* #ifndef COLMAN_MDNS_BROWSER_HPP */

/* EOF */
