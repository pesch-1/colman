// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Database utility functions
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_DB_UTIL_HPP
#  define COLMAN_DB_UTIL_HPP

#  ifndef SYSTEM_QLOCALE
#    include <QLocale>
#    define SYSTEM_QLOCALE
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  // -----------------------------------------------------------------------
  // Encryption helper
  // -----------------------------------------------------------------------

  /// Returns true if \a fname does not start with the SQLite magic header.
  /** An encrypted sqlite3mc file replaces the plaintext header with
      ciphertext, so the first 16 bytes no longer match "SQLite format 3".
      @param fname  absolute path of the .col file to test */
  bool is_file_encrypted(QString const& fname);

  // -----------------------------------------------------------------------
  // Locale helper
  // -----------------------------------------------------------------------

  /// Returns the system language reliably.
  /**
   * QLocale::system().language() may return QLocale::C on some Qt 6
   * builds even when LANG is set.  When that happens this function
   * falls back to constructing a QLocale from the LANG environment
   * variable.
   * @return the detected system language, or QLocale::C if unknown
   */
  QLocale::Language system_language();

  // -----------------------------------------------------------------------
  // Thumbnails
  // -----------------------------------------------------------------------

  /// Converts a thumbnail key to a relative filename with a .png extension.
  QString key_to_thumbnail_file(QString const& key);

  /// Converts a relative thumbnail filename to its absolute filesystem path.
  QString filename_to_thumbnail_path(QString const& fname,
                                     QString const& thumbnail_dir);

  /// Returns the local filesystem path for a thumbnail identified by \a key.
  QString key_to_thumbnail_path(QString const& key,
                                QString const& thumbnail_dir);

  // -----------------------------------------------------------------------
  // Translations
  // -----------------------------------------------------------------------

  /// Converts a display string to a valid SQLite column name.
  QString string_to_column_name(QString const& str);

} // namespace colman

#endif /* #ifndef COLMAN_DB_UTIL_HPP */

/* EOF */
