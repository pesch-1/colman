// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of render_ui_snapshots().
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_UI_SNAPSHOT_HPP
#  define COLMAN_UI_SNAPSHOT_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{

  /// Renders every main-window stack page to a PNG for regression testing.
  /**
     Builds a MainWindowUi into an offscreen QMainWindow and saves one PNG per
     page of pages_stack_ into @p dir (created if needed).  Used as a pixel
     baseline to guard the hand-written main-window UI against layout
     regressions while it is refactored.  Requires a running QApplication.
     @param dir output directory for the page_NN.png files.
     @return number of pages written, or -1 on error.
  */
  int render_ui_snapshots(QString const& dir);

} // namespace colman

#endif /* #ifndef COLMAN_UI_SNAPSHOT_HPP */
/* EOF */
