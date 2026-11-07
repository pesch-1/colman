// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Password support.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PASSWORDS_HPP
#  define COLMAN_PASSWORDS_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// generates a Argon2 hash for the password.
  QString hash_password(QString const& password);
  /// verifies a password, given an Argon2 hash.
  bool verify_password(QString const& password, QString const& hash);

} // namespace colman

#endif /* #ifndef COLMAN_PASSWORDS_HPP */

/* EOF */
