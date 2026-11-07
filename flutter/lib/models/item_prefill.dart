// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import '../api/ean_lookup.dart' show EanLookupResult;
import '../api/isbn_lookup.dart' show BookLookupResult;
import '../api/title_lookup.dart';

/// Pre-fill data for the "Create new item" dialog, populated from an internet
/// barcode lookup.
///
/// Passed to [AcquisitionInitialData] when Fall 3 of the barcode dispatch
/// flow identifies an ISBN or EAN that is not yet in the database.
class ItemPrefill {
  const ItemPrefill({
    this.name = '',
    this.barcode = '',
    this.description = '',
    this.typeSpecific = const {},
    this.typeListSpecific = const {},
    this.preferredTypeHints = const [],
    this.coverUrl,
    this.coverKey,
  });

  /// Display name of the new item.
  final String name;

  /// Barcode value to pre-fill in the item form (ISBN-13 or EAN-13).
  ///
  /// This goes into a type-specific field (e.g. `isbn` or `ean`), **not**
  /// into the internal Colman barcode which is generated server-side.
  final String barcode;

  /// Long description / blurb of the item.
  final String description;

  /// Scalar type-specific field values keyed by field name (best-effort match).
  final Map<String, String> typeSpecific;

  /// List type-specific field values keyed by field name (best-effort match).
  final Map<String, List<String>> typeListSpecific;

  /// Ordered lowercase substrings matched against [ItemType.name] when
  /// pre-selecting the type dropdown.  First match wins; falls back to the
  /// first available type when nothing matches.
  final List<String> preferredTypeHints;

  /// URL of the cover image returned by the internet lookup, or null.
  final String? coverUrl;

  /// Cache key for the thumbnail on the server (e.g. `"isbn_9783866721647"`).
  final String? coverKey;

  /// Search term for an Amazon.de search, derived from [coverKey].
  ///
  /// Strips the `"isbn_"` or `"ean_"` prefix.  Returns null when [coverKey]
  /// is null or has no recognised prefix.
  String? get amazonSearchTerm {
    final key = coverKey;
    if(key == null) return null;
    if(key.startsWith('isbn_')) return key.substring(5);
    if(key.startsWith('ean_')) return key.substring(4);
    return null;
  }
}

/// Converts a [BookLookupResult] (ISBN internet search) to an [ItemPrefill].
///
/// The normalised ISBN-13 string [barcode] is stored in the `isbn` type-
/// specific field so it lands in the item's metadata, not in the Colman
/// library barcode field.
ItemPrefill bookToItemPrefill(BookLookupResult book, String barcode) {
  final typeSpecific = <String, String>{};
  if(book.publisher.isNotEmpty) typeSpecific['publisher'] = book.publisher;
  if(book.publicationDate.isNotEmpty) {
    typeSpecific['publication_date'] = book.publicationDate;
  }
  if(book.pages.isNotEmpty) typeSpecific['pages'] = book.pages;
  final typeListSpecific = <String, List<String>>{};
  if(barcode.isNotEmpty) typeListSpecific['ISBN13'] = [barcode];
  if(book.isbn10.isNotEmpty) typeListSpecific['ISBN10'] = [book.isbn10];
  if(book.authors.isNotEmpty) typeListSpecific['authors'] = book.authors;
  if(book.ddc.isNotEmpty) typeListSpecific['ddc'] = [book.ddc];
  return ItemPrefill(
    name: book.name,
    description: book.description,
    typeSpecific: typeSpecific,
    typeListSpecific: typeListSpecific,
    preferredTypeHints: const ['buch', 'book'],
    coverUrl: book.coverUrl,
    coverKey: barcode.isNotEmpty ? 'isbn_$barcode' : null,
  );
}

/// Converts an [EanLookupResult] (EAN/UPC internet search) to an [ItemPrefill].
///
/// The normalised EAN-13 string [barcode] is stored in the `ean` type-specific
/// field.  Type hints are derived from the media classification so the dialog
/// pre-selects the most likely item type (CD, DVD, Blu-ray, board game, …);
/// a non-null [hintsOverride] replaces the derived hints (used by the title
/// search where the media kind is chosen by the user).
ItemPrefill avToItemPrefill(EanLookupResult av, String barcode,
    {List<String>? hintsOverride}) {
  final typeSpecific = <String, String>{};
  if(barcode.isNotEmpty) typeSpecific['ean'] = barcode;
  if(av.publisher.isNotEmpty) typeSpecific['publisher'] = av.publisher;
  if(av.publisherNumber.isNotEmpty) {
    typeSpecific['publisher_number'] = av.publisherNumber;
  }
  if(av.publicationDate.isNotEmpty) typeSpecific['year'] = av.publicationDate;
  if(av.director.isNotEmpty) typeSpecific['director'] = av.director;
  if(av.participants.isNotEmpty) {
    typeSpecific['participants'] = av.participants;
  }
  if(av.mediaType.isNotEmpty) typeSpecific['media_type'] = av.mediaType;
  final typeListSpecific = <String, List<String>>{};
  if(av.artists.isNotEmpty) typeListSpecific['artist'] = av.artists;
  if(av.genres.isNotEmpty) typeListSpecific['genre'] = av.genres;

  // Derive type hints from media classification.
  // Primary signals: MARC21 337 (media type) and 338 (carrier type).
  // Fallback signals: MARC21 300 (extent), 336 (content type), 508/700
  // director credit, and 511 participants.
  final hints = <String>[];
  if(av.isGame) {
    hints.addAll(['spiel', 'game', 'board']);
  } else {
    final mt = av.mediaType.toLowerCase();
    final ct = av.carrierType.toLowerCase();
    final ext = av.extent.toLowerCase();
    final cot = av.contentType.toLowerCase();
    final hasVideoSignal = mt.contains('video') ||
        ct.contains('video') ||
        ct.contains('blu') ||
        ct.contains('dvd') ||
        ext.contains('blu-ray') ||
        ext.contains('bluray') ||
        ext.contains('dvd') ||
        cot.contains('video');
    final hasAudioSignal =
        mt.contains('audio') || ct.contains('audio') || cot.contains('audio');
    final hasDirector =
        av.director.isNotEmpty || av.participants.isNotEmpty;
    if(hasVideoSignal || hasDirector) {
      // Blu-ray is indicated when carrier type or extent says so explicitly.
      final isBluray = ct.contains('blu') ||
          ext.contains('blu-ray') ||
          ext.contains('bluray');
      if(isBluray) {
        hints.addAll(['blu', 'dvd', 'video', 'film']);
      } else {
        hints.addAll(['dvd', 'video', 'film', 'blu']);
      }
    } else if(hasAudioSignal || av.artists.isNotEmpty) {
      hints.addAll(['cd', 'audio', 'musik', 'tonträger', 'tontraeger']);
    }
  }

  return ItemPrefill(
    name: av.name,
    description: av.description,
    typeSpecific: typeSpecific,
    typeListSpecific: typeListSpecific,
    preferredTypeHints: hintsOverride ?? hints,
    coverUrl: av.coverUrl,
    coverKey: barcode.isNotEmpty ? 'ean_$barcode' : null,
  );
}

/// Converts a BGG board-game detail record to an [ItemPrefill].
///
/// Maps the board-game fields to the predefined `board_game` type keys
/// (`min_players`, `max_players`, `min_age`, `playing_time`,
/// `publication_date`, lists `designer`/`artist`/`genre`).  The cover key
/// is derived from the BGG game ID [bggId].
ItemPrefill bggToItemPrefill(EanLookupResult game, int bggId) {
  final typeSpecific = <String, String>{};
  if(game.publisher.isNotEmpty) typeSpecific['publisher'] = game.publisher;
  if(game.publicationDate.isNotEmpty) {
    typeSpecific['publication_date'] = game.publicationDate;
  }
  if(game.minPlayers.isNotEmpty) {
    typeSpecific['min_players'] = game.minPlayers;
  }
  if(game.maxPlayers.isNotEmpty) {
    typeSpecific['max_players'] = game.maxPlayers;
  }
  if(game.minAge.isNotEmpty) typeSpecific['min_age'] = game.minAge;
  if(game.playingTime.isNotEmpty) {
    typeSpecific['playing_time'] = game.playingTime;
  }
  final typeListSpecific = <String, List<String>>{};
  if(game.designers.isNotEmpty) typeListSpecific['designer'] = game.designers;
  if(game.artists.isNotEmpty) typeListSpecific['artist'] = game.artists;
  if(game.genres.isNotEmpty) typeListSpecific['genre'] = game.genres;
  return ItemPrefill(
    name: game.name,
    description: game.description,
    typeSpecific: typeSpecific,
    typeListSpecific: typeListSpecific,
    preferredTypeHints: const ['spiel', 'game', 'board', 'gesellschaft'],
    coverUrl: game.coverUrl,
    coverKey: game.coverUrl != null ? 'bgg_$bggId' : null,
  );
}

/// Adds a synthetic cover key when [prefill] has a cover URL but no key.
///
/// Title-search results without an ISBN or EAN have no natural cover key;
/// without one the create-item dialog skips the thumbnail upload.
ItemPrefill _withCoverKey(ItemPrefill prefill) {
  final url = prefill.coverUrl;
  if(url == null || prefill.coverKey != null) return prefill;
  return ItemPrefill(
    name: prefill.name,
    barcode: prefill.barcode,
    description: prefill.description,
    typeSpecific: prefill.typeSpecific,
    typeListSpecific: prefill.typeListSpecific,
    preferredTypeHints: prefill.preferredTypeHints,
    coverUrl: url,
    coverKey: 'title_${url.hashCode.toRadixString(16)}',
  );
}

/// Converts a book or AV title-search result to an [ItemPrefill].
///
/// [kind] is the media kind the user chose in the title search dialog;
/// it selects exact type hints so the create-item dialog pre-selects the
/// right item type (the derived hints cannot distinguish DVD from
/// Blu-ray).  [BggTitleResult] entries must be resolved with
/// `queryBggThing` and mapped via [bggToItemPrefill] instead.
ItemPrefill titleResultToItemPrefill(TitleSearchResult result,
    MediaKind kind) {
  switch(result) {
    case BookTitleResult():
      return _withCoverKey(
          bookToItemPrefill(result.data, result.data.isbn13));
    case AvTitleResult():
      final hints = switch(kind) {
        MediaKind.cd =>
            const ['cd', 'audio', 'musik', 'tonträger', 'tontraeger'],
        MediaKind.dvd => const ['dvd', 'video', 'film'],
        MediaKind.bluray => const ['blu', 'bluray', 'video', 'film'],
        _ => null,
      };
      return _withCoverKey(
          avToItemPrefill(result.data, '', hintsOverride: hints));
    case BggTitleResult():
      throw StateError(
          'BGG results must be resolved via queryBggThing first');
  }
}
