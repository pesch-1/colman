// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';
import 'dart:io';

import 'package:multicast_dns/multicast_dns.dart';

import 'multicast_lock.dart';

/// Information about a discovered colman REST server.
class MdnsServiceInfo {
  const MdnsServiceInfo({
    required this.name,
    required this.host,
    required this.port,
    required this.tls,
  });

  final String name;
  final String host;
  final int port;
  final bool tls;

  String get url => '${tls ? "https" : "http"}://$host:$port';
}

/// Discovers colman REST servers on the LAN via mDNS.
///
/// Sends a PTR query for `_colman._tcp` and resolves each instance's
/// SRV (host, port) and TXT (name=, tls=1) records.
///
/// Holds the Android multicast lock while browsing and retransmits the
/// PTR query at 0 s, 1 s and 3 s (RFC 6762 §5.2 initial query backoff,
/// matching the Qt client's MdnsBrowser), since a single multicast UDP
/// query is easily lost.
///
/// On **mobile** the socket binds to an ephemeral port instead of 5353.
/// Queries sent from a non-5353 port are answered with a unicast response
/// addressed directly to this socket (legacy unicast, RFC 6762 §6.7), which
/// the colman server and standard responders (Avahi, Bonjour) implement.
/// This avoids the two failure modes of receiving multicast on phones:
/// Wi-Fi access points unreliably delivering multicast frames, and other
/// processes on the device contending for port 5353.
///
/// On **desktop** that trick backfires: the legacy-unicast reply arrives on
/// an ephemeral port that host firewalls (and connection tracking) do not
/// associate with the outbound multicast query, so it is silently dropped —
/// while the standard mDNS port 5353 is whitelisted. We therefore bind 5353
/// and receive the normal multicast responses, mirroring the Qt client which
/// works reliably on LANs. Avahi/mDNSResponder already hold 5353, so the bind
/// relies on the `reuseAddress`/`reusePort` flags the factory is given.
Stream<MdnsServiceInfo> browseMdnsServices() async* {
  final bindToMdnsPort =
      Platform.isLinux || Platform.isMacOS || Platform.isWindows;
  final client = MDnsClient(
    rawDatagramSocketFactory: (
      dynamic host,
      int port, {
      bool reuseAddress = true,
      bool reusePort = true,
      int ttl = 255,
    }) =>
        RawDatagramSocket.bind(
      host,
      bindToMdnsPort ? port : 0,
      reuseAddress: reuseAddress,
      reusePort: reusePort,
      ttl: ttl,
    ),
  );
  await MulticastLock.acquire();
  try {
    await client.start();
    const attemptTimeouts = [
      Duration(seconds: 1),
      Duration(seconds: 2),
      Duration(seconds: 2),
    ];
    final seen = <String>{};
    for (final timeout in attemptTimeouts) {
      await for (final ptr in client.lookup<PtrResourceRecord>(
        ResourceRecordQuery.serverPointer('_colman._tcp'),
        timeout: timeout,
      )) {
        if (!seen.add(ptr.domainName.toLowerCase())) continue;
        final info = await _resolveInstance(client, ptr.domainName);
        if (info != null) yield info;
      }
    }
  } finally {
    client.stop();
    await MulticastLock.release();
  }
}

/// Resolves one service instance's SRV, A, and TXT records.
///
/// SRV, A, and TXT usually arrive as additional records of the PTR
/// response and are answered from the client's cache; the short timeout
/// only matters when an instance answers the PTR query but nothing else.
Future<MdnsServiceInfo?> _resolveInstance(
  MDnsClient client,
  String domainName,
) async {
  const timeout = Duration(seconds: 2);

  String target = '';
  String host = '';
  int port = 0;
  await for (final srv in client.lookup<SrvResourceRecord>(
    ResourceRecordQuery.service(domainName),
    timeout: timeout,
  )) {
    target = srv.target;
    host = target;
    if (host.endsWith('.local.')) {
      host = host.substring(0, host.length - 7);
    } else if (host.endsWith('.local')) {
      host = host.substring(0, host.length - 6);
    }
    port = srv.port;
    break;
  }
  if (host.isEmpty || port == 0) return null;

  // Prefer the announced IPv4 address over the hostname: phones often
  // cannot resolve bare LAN hostnames, and .local names need an mDNS
  // resolver the platform may not provide.
  await for (final a in client.lookup<IPAddressResourceRecord>(
    ResourceRecordQuery.addressIPv4(target),
    timeout: timeout,
  )) {
    host = a.address.address;
    break;
  }

  String name = '';
  bool tls = false;
  await for (final txt in client.lookup<TxtResourceRecord>(
    ResourceRecordQuery.text(domainName),
    timeout: timeout,
  )) {
    for (final entry in txt.text.split('\n')) {
      if (entry.startsWith('name=')) {
        name = entry.substring(5);
      } else if (entry == 'tls=1') {
        tls = true;
      }
    }
    break;
  }
  if (name.isEmpty) {
    final idx = domainName.indexOf('._colman');
    name = idx > 0 ? domainName.substring(0, idx) : domainName;
  }
  return MdnsServiceInfo(name: name, host: host, port: port, tls: tls);
}
