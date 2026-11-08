// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Console input helpers for the command-line tools
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CONSOLE_HPP
#  define COLMAN_CONSOLE_HPP

#  ifndef SYSTEM_OPTIONAL
#    include <optional>
#    define SYSTEM_OPTIONAL
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Prints \a prompt on stderr and reads a line from stdin without echo.
  /** If stdin is a terminal (Linux) or console (Windows), the typed
      characters are not shown.  Otherwise, e.g. with piped input, the line
      is read as UTF-8.  Trailing CR/LF is removed.
      @param prompt  text shown before the input
      @return the entered line, or std::nullopt at the end of the input */
  std::optional<QString> read_password(QString const& prompt);
} // namespace colman

#endif /* #ifndef COLMAN_CONSOLE_HPP */

/* EOF */
