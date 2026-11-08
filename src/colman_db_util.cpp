// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Database utility functions
/**
   @file
   @author Christoph Pesch
*/

#include "colman_db_util.hpp"
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_CSTRING
#  include <cstring>
#  define SYSTEM_CSTRING
#endif

namespace
{
  /// Characters that must not appear in a file name on Windows; the path
  /// separators are not allowed in a file name on any system.
  char constexpr ForbiddenFileNameChars[] = "<>:\"/\\|?*";

  /// Replaces the characters of \a name that are not allowed in file names.
  /**
   * Control characters and ForbiddenFileNameChars become '_', so that the
   * result is a valid file name on Linux and Windows alike.
   */
  QString portable_file_name(QString const& name)
  {
    QString result = name;
    for(QChar& c : result) {
      bool const forbidden =
          c.unicode() < 0x20 ||
          (c.unicode() < 0x80 &&
           std::strchr(ForbiddenFileNameChars, c.toLatin1()) != nullptr);
      if(forbidden) {
        c = QChar('_');
      }
    }
    return result;
  }

} // anonymous namespace

/**
 * Returns the system language reliably.
 * QLocale::system().language() may return QLocale::C on some Qt 6
 * builds even when LANG is set (it only honours LC_ALL).  When that
 * happens we fall back to constructing a QLocale from the LANG
 * environment variable.
 * @return the detected system language, or QLocale::C if unknown
 */
QLocale::Language colman::system_language()
{
  QLocale::Language lang = QLocale::system().language();
  if(lang != QLocale::C && lang != QLocale::AnyLanguage) {
    return lang;
  }
  // fallback: parse LANG / LC_ALL
  QString const env = qEnvironmentVariable("LANG");
  if(!env.isEmpty()) {
    QLocale fallback(env);
    auto const flang = fallback.language();
    if(flang != QLocale::C && flang != QLocale::AnyLanguage) {
      return flang;
    }
  }
  return lang;
}


/**
 * Converts an arbitrary display string into a name that is safe to use
 * as a SQLite column name.  ASCII letters and digits are kept as-is;
 * whitespace and punctuation become underscores; any other character
 * becomes '@'.
 * @param str the display string to sanitise
 * @return a string suitable for use as a column identifier
 */
QString colman::string_to_column_name(QString const& str)
{
  QString retval;
  int const len = str.size();
  retval.reserve(len);
  for(int i = 0; i < len; ++i) {
    if(str[i].unicode() < 128 && str[i].isLetterOrNumber()) {
      retval.push_back(str[i]);
    }
    else if(str[i].isSpace() || str[i].isPunct()) {
      retval.push_back('_');
    }
    else {
      retval.push_back('@');
    }
  }
  return retval;
}


/// Converts a thumbnail key to a portable relative filename (.png).
/**
 * Keys may contain item titles; characters that are not allowed in file
 * names on Windows (e.g. ':' in "Title : Subtitle") or on any system
 * ('/') are replaced by '_', so that thumbnails can be stored and copied
 * between Linux and Windows.
 * @param key the thumbnail key (e.g. an ISBN or barcode-derived string)
 * @return the portable filename with ".png" appended
 */
QString colman::key_to_thumbnail_file(QString const& key)
{
  return portable_file_name(key) + ".png";
}


/// Returns the filename a thumbnail key had before names were portable.
/**
 * Before colman 2.0.2 the key was used unchanged.  Thumbnails stored
 * under such names keep them (the thumbnails table records the name), and
 * use_local_thumbnail() still finds files with these names.
 * @param key the thumbnail key
 * @return the key with ".png" appended
 */
QString colman::legacy_key_to_thumbnail_file(QString const& key)
{
  return key + ".png";
}


/// Converts a relative thumbnail filename to its absolute filesystem path.
/**
 * Uses \a thumbnail_dir as the root, splits the filename on the first '_'
 * character to form a prefix subdirectory, and further distributes files into
 * a two-digit checksum subdirectory to keep directory sizes manageable.
 * Missing directories are created.
 * @param fname          the relative filename (e.g. "1234_5678.png")
 * @param thumbnail_dir  root directory for thumbnail files on disk
 * @return the absolute path where the thumbnail is stored
 */
QString colman::filename_to_thumbnail_path(QString const& fname,
                                           QString const& thumbnail_dir)
{
  QDir dir(thumbnail_dir);
  int const pend = fname.indexOf('_');
  if(pend > 0) {
    QString const prefix = fname.left(pend);
    dir.mkpath(prefix);
    dir.cd(prefix);
  }
  quint16 const check = qChecksum(QByteArrayView(fname.toUtf8())) % 100;
  QString const sub = QString("%1").arg(check, 2, 10, QChar('0'));
  dir.mkpath(sub);
  dir.cd(sub);
  return dir.filePath(fname);
}


/// Returns the absolute filesystem path for a thumbnail key.
/**
 * Combines key_to_thumbnail_file() and filename_to_thumbnail_path() to
 * produce the full path at which the thumbnail for \a key is stored.
 * @param key            the thumbnail key
 * @param thumbnail_dir  root directory for thumbnail files on disk
 * @return the absolute path of the thumbnail file
 */
QString colman::key_to_thumbnail_path(QString const& key,
                                      QString const& thumbnail_dir)
{
  QString const basename = key_to_thumbnail_file(key);
  return filename_to_thumbnail_path(basename, thumbnail_dir);
}


/**
 * Returns true if \a fname does not begin with the SQLite file-format magic.
 * An sqlite3mc-encrypted database replaces the first page (including the
 * 16-byte "SQLite format 3\000" header) with ciphertext, so the bytes at the
 * start of the file no longer match the magic string.
 * @param fname  absolute path to test
 */
bool colman::is_file_encrypted(QString const& fname)
{
  QFile f(fname);
  if(!f.open(QIODevice::ReadOnly)) {
    return false;
  }
  QByteArray const header = f.read(16);
  return header != QByteArray("SQLite format 3\000", 16);
}

/* EOF */
