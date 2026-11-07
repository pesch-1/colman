// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Trace level and logging categories
/**
   @file
   @author Christoph Pesch
*/

#include "colman_trace.hpp"

// Debug messages are disabled by default; only info and above pass.
Q_LOGGING_CATEGORY(colman_lookup_log, "colman.lookup", QtInfoMsg)


/**
   Sets the Qt logging filter rules according to the trace level.
   Debug traces of the internet search are enabled only if \a level is at
   least LookupTraceLevel.  The rules replace all previously set rules, so
   the suppression of spurious KDE warnings is always included.
   @param level trace level given on the command line (0 = silent).
*/
void colman::set_trace_level(int level)
{
  // KDE's KDirModel emits this spuriously when inotify fires for files
  // that are no longer in its model (e.g. build artefacts, editor temps).
  QString rules = QStringLiteral("kf.kio.widgets.kdirmodel.warning=false");
  if(level >= LookupTraceLevel) {
    rules += QStringLiteral("\ncolman.lookup.debug=true");
  }
  QLoggingCategory::setFilterRules(rules);
}


/**
   Converts \a text to an integer and calls set_trace_level(int) with it.
   @param text trace level as given on the command line.
   @return true if \a text is a non-negative integer, false otherwise.
*/
bool colman::set_trace_level(QString const& text)
{
  auto ok = false;
  auto const level = text.toInt(&ok);
  if(!ok || level < 0) {
    return false;
  }
  set_trace_level(level);
  return true;
}


/* EOF */
