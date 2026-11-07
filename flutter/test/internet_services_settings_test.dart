// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/models/internet_services_settings.dart';

void main() {
  group('SruServer.fromJson title_field migration', () {
    test('absent title_field is migrated for known default endpoints', () {
      final servers = sruServersFromJson(
          '[{"name":"DNB",'
          '"endpoint":"https://services.dnb.de/sru/dnb",'
          '"isbn_field":"num","ean_field":"",'
          '"record_schema":"MARC21-xml","enabled":true},'
          '{"name":"Custom",'
          '"endpoint":"https://example.org/sru",'
          '"isbn_field":"isbn","ean_field":"",'
          '"record_schema":"marcxml","enabled":true}]');
      expect(servers, hasLength(2));
      // Known endpoint: title field taken from the default server list.
      expect(servers[0].titleField, 'tit');
      // Unknown endpoint: no default available, stays empty.
      expect(servers[1].titleField, '');
    });

    test('present title_field is used verbatim, even when empty', () {
      final servers = sruServersFromJson(
          '[{"name":"DNB",'
          '"endpoint":"https://services.dnb.de/sru/dnb",'
          '"isbn_field":"num","ean_field":"","title_field":"",'
          '"record_schema":"MARC21-xml","enabled":true}]');
      expect(servers, hasLength(1));
      expect(servers[0].titleField, '');
    });

    test('round trip keeps a custom title_field', () {
      const server = SruServer(
        name: 'Test Server',
        endpoint: 'https://example.org/sru',
        isbnField: 'isbn',
        eanField: '',
        titleField: 'custom.title',
        recordSchema: 'marcxml',
        enabled: true,
      );
      final loaded = sruServersFromJson(sruServersToJson([server]));
      expect(loaded, hasLength(1));
      expect(loaded[0], server);
      expect(loaded[0].titleField, 'custom.title');
    });
  });

  group('default SRU servers', () {
    test('all defaults carry a title field', () {
      final defaults = InternetServicesSettings.defaults().sruServers;
      for (final srv in defaults) {
        expect(srv.titleField, isNotEmpty,
            reason: '${srv.name} should have a title field');
      }
    });

    test('OeNB default uses the working Alma endpoint', () {
      final defaults = InternetServicesSettings.defaults().sruServers;
      final oenb = defaults
          .firstWhere((s) => s.name.contains('ÖNB'));
      expect(oenb.endpoint,
          'https://obv-at-oenb.alma.exlibrisgroup.com/view/sru/43ACC_ONB');
      expect(oenb.isbnField, 'alma.isbn');
      expect(oenb.titleField, 'alma.title');
    });

    test('only BnF is a UNIMARC server', () {
      final defaults = InternetServicesSettings.defaults().sruServers;
      final unimarc =
          defaults.where((s) => s.isUnimarc).map((s) => s.name).toList();
      expect(unimarc, hasLength(1));
      expect(unimarc.first, contains('BnF'));
    });
  });

  group('SruServer equality', () {
    test('differs when title field differs', () {
      const a = SruServer(
        name: 'S', endpoint: 'https://e', isbnField: 'i',
        eanField: '', titleField: 'tit',
        recordSchema: 'marcxml', enabled: true,
      );
      final b = a.copyWith(titleField: 'other');
      expect(a == b, isFalse);
      expect(a == a.copyWith(), isTrue);
    });
  });
}
