// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Trace level and logging categories
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TRACE_HPP
#  define COLMAN_TRACE_HPP

#  ifndef SYSTEM_QLOGGINGCATEGORY
#    include <QLoggingCategory>
#    define SYSTEM_QLOGGINGCATEGORY
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

/// Logging category for internet search (ISBN/EAN/title lookup) traces.
Q_DECLARE_LOGGING_CATEGORY(colman_lookup_log)

namespace colman
{
  /// Minimum trace level at which internet search traces are printed.
  int constexpr LookupTraceLevel = 2;

  /// Enables the logging categories that belong to trace level \a level.
  void set_trace_level(int level);

  /// Parses \a text as trace level and applies it.
  /**
     @param text trace level as given on the command line.
     @return false (and nothing changed) if \a text is no number >= 0.
  */
  bool set_trace_level(QString const& text);
} // namespace colman

#endif /* #ifndef COLMAN_TRACE_HPP */

/* EOF */
