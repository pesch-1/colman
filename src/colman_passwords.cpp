// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of password hashing with Argon2id.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_passwords.hpp"
#ifndef SYSTEM_ARGON2_H
#  include <argon2.h>
#  define SYSTEM_ARGON2_H
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
#endif
#ifndef SYSTEM_QRANDOMGENERATOR
#  include <QRandomGenerator>
#  define SYSTEM_QRANDOMGENERATOR
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{
  uint32_t constexpr TCost = 3;       ///< Argon2id time cost (iterations)
  uint32_t constexpr MCost = 65536;   ///< Argon2id memory cost in KiB (64 MiB)
  uint32_t constexpr Parallelism = 1; ///< Argon2id parallelism degree
  uint32_t constexpr HashLen = 32;    ///< output hash length in bytes
  uint32_t constexpr SaltLen = 16;    ///< random salt length in bytes

} // namespace


/// Hashes \a password with Argon2id and returns the encoded hash string.
/**
 * Generates a cryptographically random 16-byte salt via
 * QRandomGenerator::system(), then calls argon2id_hash_encoded() to
 * produce a self-contained encoded string (algorithm parameters + salt +
 * hash) suitable for storage.
 * @param password plain-text password encoded as UTF-8
 * @return Argon2id encoded hash string, or empty string on failure
 */
QString colman::hash_password(QString const& password)
{
  // Generate cryptographically random salt byte by byte.
  QByteArray salt(static_cast<int>(SaltLen), '\0');
  for(int i = 0; i < static_cast<int>(SaltLen); ++i) {
    salt[i] = static_cast<char>(QRandomGenerator::system()->generate() & 0xFFu);
  }

  size_t const encoded_len =
      argon2_encodedlen(TCost, MCost, Parallelism, SaltLen, HashLen, Argon2_id);
  QByteArray encoded(static_cast<int>(encoded_len), '\0');

  QByteArray pwd_utf8 = password.toUtf8();
  int const rc = argon2id_hash_encoded(
      TCost, MCost, Parallelism,
      pwd_utf8.constData(), static_cast<size_t>(pwd_utf8.size()),
      salt.constData(), SaltLen,
      HashLen,
      encoded.data(), encoded_len);

  // Zero sensitive buffers before they go out of scope.
  std::fill(pwd_utf8.begin(), pwd_utf8.end(), '\0');
  std::fill(salt.begin(), salt.end(), '\0');

  if(rc != ARGON2_OK) {
    qWarning() << "hash_password: argon2id_hash_encoded failed:"
               << argon2_error_message(rc);
    return {};
  }
  // The encoded string is ASCII; Latin-1 round-trips it losslessly.
  return QString::fromLatin1(encoded.constData());
}


/// Verifies \a password against the stored Argon2id \a hash.
/**
 * @param password plain-text password encoded as UTF-8
 * @param hash     Argon2id encoded hash returned by hash_password()
 * @return true if the password matches, false otherwise
 */
bool colman::verify_password(QString const& password,
                             QString const& hash)
{
  QByteArray pwd_utf8 = password.toUtf8();
  QByteArray const hash_latin1 = hash.toLatin1();
  int const rc = argon2id_verify(
      hash_latin1.constData(),
      pwd_utf8.constData(),
      static_cast<size_t>(pwd_utf8.size()));
  // Zero sensitive buffer before it goes out of scope.
  std::fill(pwd_utf8.begin(), pwd_utf8.end(), '\0');
  return rc == ARGON2_OK;
}

/* EOF */
