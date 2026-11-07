// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Page for scanning barcodes with a connected camera.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CAMERA_SCAN_PAGE_HPP
#  define COLMAN_CAMERA_SCAN_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif

class QLineEdit;

namespace colman
{
  /// Creates the camera scan page and adds it to the page stack.
  /**
     To be called once at program start, before any page that uses
     scan_barcode().
     @param app  Qt application object.
     @param data central application state.
  */
  void setup_camera_scan_page(QApplication& app, AppData& data);

  /// Switches to the camera scan page and starts decoding barcodes.
  /**
     When a barcode is recognised the page stops the camera, returns to the
     originating page and invokes \a on_scanned with the decoded string. The
     Back button returns to the originating page without invoking the callback.
     @param data       central application state.
     @param on_scanned callback receiving the decoded barcode string.
  */
  void scan_barcode(AppData& data,
                    std::function<void(QString const&)> on_scanned);

  /// Scans a barcode and feeds the result into \a field.
  /**
     Convenience wrapper around scan_barcode() that sets the line edit text to
     the decoded barcode, marks it modified and emits editingFinished().
     @param data  central application state.
     @param field the QLineEdit that receives the scanned barcode text.
  */
  void scan_barcode_into(AppData& data, QLineEdit* field);

} // namespace colman

#endif /* #ifndef COLMAN_CAMERA_SCAN_PAGE_HPP */

/* EOF */
