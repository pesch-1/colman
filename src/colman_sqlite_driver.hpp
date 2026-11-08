// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Built-in Qt SQL driver for SQLite with encryption support
/**
   colman registers its own Qt SQL driver instead of using Qt's QSQLITE
   plugin.  It is built on SQLite3MultipleCiphers, which is linked
   statically, and uses only the public QtSql API.  Encryption is therefore
   always available, without plugins or private Qt headers.

   Encrypted files use the ChaCha20-Poly1305 cipher.  Keys are passed to
   SQLite as UTF-8.

   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SQLITE_DRIVER_HPP
#  define COLMAN_SQLITE_DRIVER_HPP

#  ifndef SYSTEM_QSQLDATABASE
#    include <QSqlDatabase>
#    define SYSTEM_QSQLDATABASE
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Name under which the built-in SQLite driver is registered.
  char constexpr SqliteDriverName[] = "COLMAN_SQLITE";

  /// Registers the built-in SQLite driver with QSqlDatabase.
  /** Does nothing if the driver is already registered.  Must be called
      before QSqlDatabase::addDatabase() is called with SqliteDriverName. */
  void register_sqlite_driver();

  /// Unlocks an encrypted database opened with the built-in driver.
  /** @param db   open connection using the built-in driver
      @param key  passphrase of the database
      @return false if \a db does not use the built-in driver or the key is
              wrong */
  bool set_database_key(QSqlDatabase const& db, QString const& key);

  /// Encrypts, re-encrypts or decrypts the database file of \a db.
  /** @param db       open (and, if encrypted, unlocked) connection using the
                      built-in driver
      @param new_key  new passphrase; empty to remove the encryption
      @return true on success */
  bool change_database_key(QSqlDatabase const& db, QString const& new_key);
} // namespace colman

#endif /* #ifndef COLMAN_SQLITE_DRIVER_HPP */

/* EOF */
