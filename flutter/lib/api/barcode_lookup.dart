// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import '../models/internet_services_settings.dart';
import 'ean_lookup.dart' show EanLookupResult, isProductEan, isUpcA, queryEan;
import 'isbn_lookup.dart'
    show BookLookupResult, isIsbn10, isIsbn13, isbn10ToIsbn13, queryIsbn;

// ---------------------------------------------------------------------------
// BarcodeSearchResult — sealed class hierarchy
// ---------------------------------------------------------------------------

/// The result of a barcode lookup, discriminated by media type.
///
/// Use Dart pattern matching to handle each case exhaustively:
/// ```dart
/// switch(result) {
///   case BookSearchResult(:final data): ...
///   case AvSearchResult(:final data): ...
///   case NoSearchResult(): ...
/// }
/// ```
sealed class BarcodeSearchResult {
  const BarcodeSearchResult();
}

/// Barcode was identified as an ISBN; [data] holds the book metadata.
final class BookSearchResult extends BarcodeSearchResult {
  BookSearchResult(this.data);

  /// Book metadata returned by the ISBN lookup.
  final BookLookupResult data;
}

/// Barcode was identified as a product EAN or UPC-A; [data] holds the
/// AV media or board-game metadata.
final class AvSearchResult extends BarcodeSearchResult {
  AvSearchResult(this.data);

  /// AV media or board-game metadata returned by the EAN lookup.
  final EanLookupResult data;
}

/// Barcode could not be identified as either an ISBN or a product EAN/UPC.
///
/// This is returned for syntactically invalid or unrecognised barcodes.
/// It is NOT returned when a valid barcode simply yields no search results.
final class NoSearchResult extends BarcodeSearchResult {
  const NoSearchResult();
}

// ---------------------------------------------------------------------------
// Barcode normalisation
// ---------------------------------------------------------------------------

/// Strips non-digit characters (except 'X'/'x') from [barcode] and
/// uppercases any 'x' to 'X' for ISBN-10 check-digit handling.
String normalizeBarcode(String barcode) =>
    barcode.toUpperCase().replaceAll(RegExp(r'[^0-9X]'), '');

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/// Performs a barcode lookup against enabled internet services.
///
/// The barcode type is determined automatically:
/// - ISBN-13 (prefix 978/979, valid check digit) → [queryIsbn]
/// - ISBN-10 (valid check digit) → converted to ISBN-13, then [queryIsbn]
/// - EAN-13 (non-ISBN prefix, valid check digit) → [queryEan]
/// - UPC-A (12 digits, valid check digit) → padded to EAN-13, then [queryEan]
/// - Anything else → [NoSearchResult]
///
/// [barcode] may contain hyphens or spaces (e.g. "978-0-596-00712-3");
/// they are stripped before classification. Returns [NoSearchResult] for
/// an empty or unclassifiable input. Returns [BookSearchResult] or
/// [AvSearchResult] even when the underlying query finds no data.
Future<BarcodeSearchResult> queryBarcode(
  String barcode,
  InternetServicesSettings settings,
) async {
  final clean = normalizeBarcode(barcode);

  // isIsbn13() validates only the checksum (shared with EAN-13), so a prefix
  // check is required here to distinguish ISBNs from product EANs.
  final looksLikeIsbn = isIsbn10(clean) ||
      (isIsbn13(clean) &&
          (clean.startsWith('978') || clean.startsWith('979')));

  if(looksLikeIsbn) {
    final isbn13 = isIsbn10(clean) ? isbn10ToIsbn13(clean) : clean;
    final result = await queryIsbn(isbn13, settings);
    return BookSearchResult(result);
  }

  if(isProductEan(clean)) {
    final result = await queryEan(clean, settings);
    return AvSearchResult(result);
  }

  if(isUpcA(clean)) {
    final result = await queryEan('0$clean', settings);
    return AvSearchResult(result);
  }

  return const NoSearchResult();
}
