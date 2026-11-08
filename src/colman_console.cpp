// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Console input helpers for the command-line tools
/**
   @file
   @author Christoph Pesch
*/

#include "colman_console.hpp"
#ifndef SYSTEM_CSTDIO
#  include <cstdio>
#  define SYSTEM_CSTDIO
#endif
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef SYSTEM_WINDOWS_H
#    include <windows.h>
#    define SYSTEM_WINDOWS_H
#  endif
#else
#  ifndef SYSTEM_TERMIOS
#    include <termios.h>
#    define SYSTEM_TERMIOS
#  endif
#  ifndef SYSTEM_UNISTD
#    include <unistd.h>
#    define SYSTEM_UNISTD
#  endif
#endif

namespace
{
  /// Size of the input buffers (characters).
  int constexpr BufferSize = 1024;

  /// Removes trailing CR and LF characters from \a line.
  QString chop_line_end(QString line)
  {
    while(line.endsWith('\n') || line.endsWith('\r')) {
      line.chop(1);
    }
    return line;
  }

  /// Reads a line from stdin as UTF-8 (with whatever echo is active).
  std::optional<QString> read_utf8_line()
  {
    char buf[BufferSize] = {};
    if(!std::fgets(buf, BufferSize, stdin)) {
      return std::nullopt;
    }
    return chop_line_end(QString::fromUtf8(buf));
  }

#ifdef _WIN32
  /// Reads a line from the console without echo, as UTF-16.
  /**
   * @param console  console input handle
   * @param mode     current console mode
   */
  std::optional<QString> read_console_line(HANDLE console, DWORD mode)
  {
    SetConsoleMode(console, mode & ~static_cast<DWORD>(ENABLE_ECHO_INPUT));
    QString line;
    bool got_input = false;
    wchar_t buf[BufferSize] = {};
    DWORD read = 0;
    while(ReadConsoleW(console, buf, BufferSize, &read, nullptr) &&
          read > 0)
    {
      got_input = true;
      line += QString::fromWCharArray(buf, static_cast<int>(read));
      if(line.endsWith('\n')) {
        break;
      }
    }
    SetConsoleMode(console, mode);
    if(!got_input) {
      return std::nullopt;
    }
    return chop_line_end(line);
  }
#else
  /// Reads a line from the terminal \a old_term belongs to, without echo.
  std::optional<QString> read_terminal_line(termios const& old_term)
  {
    termios no_echo = old_term;
    no_echo.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &no_echo);
    auto line = read_utf8_line();
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    return line;
  }
#endif

} // anonymous namespace

/**
 * Prints \a prompt on stderr and reads a password from stdin.  On a
 * terminal or console the echo is switched off while reading and a newline
 * is printed afterwards, because the one typed by the user is not shown.
 * On Windows the console is read as UTF-16, so that characters outside the
 * console code page arrive correctly.
 * @param prompt  text shown before the input
 * @return the entered line without CR/LF, or std::nullopt at the end of
 *         the input
 */
std::optional<QString> colman::read_password(QString const& prompt)
{
  std::fprintf(stderr, "%s", prompt.toLocal8Bit().constData());
  std::fflush(stderr);
#ifdef _WIN32
  HANDLE const console = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode = 0;
  if(!GetConsoleMode(console, &mode)) {
    return read_utf8_line();
  }
  auto line = read_console_line(console, mode);
#else
  termios old_term;
  if(tcgetattr(STDIN_FILENO, &old_term) != 0) {
    return read_utf8_line();
  }
  auto line = read_terminal_line(old_term);
#endif
  std::fprintf(stderr, "\n");
  return line;
}

/* EOF */
