// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of run_tutorial_screenshots().
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TUTORIAL_SCREENSHOTS_HPP
#  define COLMAN_TUTORIAL_SCREENSHOTS_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  class AppData;

  /// Options of the tutorial screenshot scenario.
  struct TutorialScreenshotOptions
  {
    QString out_dir_;          ///< directory receiving the JPEG files
    QString sample_data_file_; ///< JSON file with the sample collection data
    QString work_dir_;         ///< scratch directory for the collection file
    bool with_network_ = false; ///< also take screenshots needing internet
  };

  /// Drives the application through the tutorial and writes screenshots.
  /**
     Creates a sample collection from scratch with the start page flow, fills
     it with the data from the sample data file (partly through the UI, partly
     directly through the query layer) and walks through the tutorial
     chapters, saving one JPEG per tutorial image into the output directory.
     The application must be set up (start and preferences pages) and its
     main window shown; translators decide the language of the screenshots.
     @param data    central application state.
     @param options output directory, sample data and scratch directory.
     @return process exit code: 0 on success, 1 if a step failed.
  */
  int run_tutorial_screenshots(AppData& data,
                               TutorialScreenshotOptions const& options);

} // namespace colman

#endif /* #ifndef COLMAN_TUTORIAL_SCREENSHOTS_HPP */
/* EOF */
