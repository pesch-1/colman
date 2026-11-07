// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Sanitizes a user-visible name into an internal SQLite column / table name.
///
/// Exact port of `colman::string_to_column_name` (colman_db_util.cpp): ASCII
/// letters and digits are kept, whitespace and punctuation become `_`, and any
/// other character becomes `@`. Used to derive the `tablename` of a new type
/// and the `column` name of newly added fields so the Flutter client produces
/// the same identifiers as the desktop client.
String stringToColumnName(String str) {
  final buffer = StringBuffer();
  for (final rune in str.runes) {
    if (rune < 128 && _isAsciiLetterOrDigit(rune)) {
      buffer.writeCharCode(rune);
    } else if (_isSpace(rune) || _isPunct(rune)) {
      buffer.write('_');
    } else {
      buffer.write('@');
    }
  }
  return buffer.toString();
}

bool _isAsciiLetterOrDigit(int c) {
  return (c >= 0x30 && c <= 0x39) || // 0-9
      (c >= 0x41 && c <= 0x5A) || // A-Z
      (c >= 0x61 && c <= 0x7A); // a-z
}

bool _isSpace(int c) {
  // Matches QChar::isSpace for the common ASCII whitespace characters.
  return c == 0x20 || (c >= 0x09 && c <= 0x0D);
}

bool _isPunct(int c) {
  // ASCII punctuation ranges (matches QChar::isPunct for ASCII).
  return (c >= 0x21 && c <= 0x2F) ||
      (c >= 0x3A && c <= 0x40) ||
      (c >= 0x5B && c <= 0x60) ||
      (c >= 0x7B && c <= 0x7E);
}
