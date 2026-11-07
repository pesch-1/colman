// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_test/flutter_test.dart';

import 'package:colman_client/api/ean_lookup.dart';
import 'package:colman_client/api/isbn_lookup.dart';
import 'package:colman_client/api/title_lookup.dart';
import 'package:colman_client/models/item_prefill.dart';

void main() {
  group('bggToItemPrefill', () {
    test('maps board-game fields to the board_game type keys', () {
      final game = EanLookupResult()
        ..name = 'Die Siedler von Catan'
        ..publisher = 'KOSMOS'
        ..publicationDate = '1995'
        ..minPlayers = '3'
        ..maxPlayers = '4'
        ..minAge = '10'
        ..playingTime = '75'
        ..designers = ['Klaus Teuber']
        ..artists = ['Volkan Baga']
        ..genres = ['Negotiation']
        ..description = 'Ein Klassiker.'
        ..coverUrl = 'https://cf.geekdo-images.com/thumb.jpg';
      final prefill = bggToItemPrefill(game, 13);
      expect(prefill.name, 'Die Siedler von Catan');
      expect(prefill.description, 'Ein Klassiker.');
      expect(prefill.typeSpecific['publisher'], 'KOSMOS');
      expect(prefill.typeSpecific['publication_date'], '1995');
      expect(prefill.typeSpecific['min_players'], '3');
      expect(prefill.typeSpecific['max_players'], '4');
      expect(prefill.typeSpecific['min_age'], '10');
      expect(prefill.typeSpecific['playing_time'], '75');
      expect(prefill.typeSpecific.containsKey('year'), isFalse);
      expect(prefill.typeListSpecific['designer'], ['Klaus Teuber']);
      expect(prefill.typeListSpecific['artist'], ['Volkan Baga']);
      expect(prefill.typeListSpecific['genre'], ['Negotiation']);
      expect(prefill.preferredTypeHints.first, 'spiel');
      expect(prefill.coverKey, 'bgg_13');
    });

    test('omits the cover key when there is no cover', () {
      final prefill = bggToItemPrefill(EanLookupResult()..name = 'X', 7);
      expect(prefill.coverUrl, isNull);
      expect(prefill.coverKey, isNull);
    });
  });

  group('titleResultToItemPrefill', () {
    test('book result with ISBN keeps the isbn cover key', () {
      final data = BookLookupResult()
        ..name = 'Buch Eins'
        ..isbn13 = '9780596007126'
        ..isbn10 = '0596007124'
        ..coverUrl = 'http://img/1.jpg';
      final prefill = titleResultToItemPrefill(
        BookTitleResult(
            name: 'Buch Eins', year: '2020', source: 'Google Books',
            coverUrl: 'http://img/1.jpg', data: data),
        MediaKind.book,
      );
      expect(prefill.preferredTypeHints.first, 'buch');
      expect(prefill.typeListSpecific['ISBN13'], ['9780596007126']);
      expect(prefill.coverKey, 'isbn_9780596007126');
    });

    test('book result without ISBN gets a synthetic cover key', () {
      final data = BookLookupResult()
        ..name = 'Der Hobbit'
        ..coverUrl = 'https://covers.openlibrary.org/b/id/12345-M.jpg';
      final prefill = titleResultToItemPrefill(
        BookTitleResult(
            name: 'Der Hobbit', year: '1937', source: 'Open Library',
            coverUrl: data.coverUrl, data: data),
        MediaKind.book,
      );
      expect(prefill.coverKey, isNotNull);
      expect(prefill.coverKey, startsWith('title_'));
    });

    test('kind-exact hints distinguish DVD from Blu-ray', () {
      AvTitleResult av(String name) => AvTitleResult(
          name: name, year: '', source: 'DNB',
          data: EanLookupResult()..name = name);
      final cd = titleResultToItemPrefill(av('Album'), MediaKind.cd);
      expect(cd.preferredTypeHints.first, 'cd');
      final dvd = titleResultToItemPrefill(av('Film'), MediaKind.dvd);
      expect(dvd.preferredTypeHints.first, 'dvd');
      expect(dvd.preferredTypeHints, isNot(contains('blu')));
      final bluray = titleResultToItemPrefill(av('Film'), MediaKind.bluray);
      expect(bluray.preferredTypeHints.first, 'blu');
    });

    test('AV result with cover gets a synthetic cover key', () {
      final data = EanLookupResult()
        ..name = 'Album Eins'
        ..coverUrl = 'https://coverartarchive.org/release/mbid-1/front-250';
      final prefill = titleResultToItemPrefill(
        AvTitleResult(
            name: 'Album Eins', year: '2001', source: 'MusicBrainz',
            coverUrl: data.coverUrl, data: data),
        MediaKind.cd,
      );
      expect(prefill.coverKey, startsWith('title_'));
    });

    test('BGG results must be resolved before mapping', () {
      expect(
        () => titleResultToItemPrefill(
            const BggTitleResult(
                name: 'Catan', year: '1995', source: 'BoardGameGeek',
                bggId: 13),
            MediaKind.boardGame),
        throwsStateError,
      );
    });
  });

  group('avToItemPrefill hintsOverride', () {
    test('override replaces the derived hints', () {
      final av = EanLookupResult()
        ..name = 'Film'
        ..carrierType = 'DVD-Video';
      final derived = avToItemPrefill(av, '1234567890123');
      expect(derived.preferredTypeHints.first, 'dvd');
      final overridden = avToItemPrefill(av, '1234567890123',
          hintsOverride: const ['blu', 'bluray']);
      expect(overridden.preferredTypeHints, ['blu', 'bluray']);
    });
  });
}
