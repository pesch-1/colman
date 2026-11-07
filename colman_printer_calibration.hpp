// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Printer print-position calibration data and persistence.
/**
   @file
   @author Christoph Pesch

   Lightweight, UI-free home of the PrinterCalibration value type and the
   load/save helpers that persist it in QSettings.  Keeping these separate
   from PrinterCalibrationWidget lets the low-level printing code
   (colman_label_printer, colman_card_batch_print) apply a calibration
   without depending on the calibration widget (and thus on QWidget).
*/

#ifndef COLMAN_PRINTER_CALIBRATION_HPP
#  define COLMAN_PRINTER_CALIBRATION_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QTGLOBAL
#    include <QtGlobal>
#    define SYSTEM_QTGLOBAL
#  endif

namespace colman
{

  /// Correction transform applied when rendering a label for printing.
  /**
     Characterises the difference between the nominal and actual print
     position for a specific printer and corrects it.  The transform is:
     @code
       printed_x = scale_p_x * content_x + trans_p_x   (printer error)
       content_x_corrected = scale_x * content_x + offset_x  (correction)
     @endcode
     such that @c printed(corrected(x)) == x.

     A default-constructed value (zero offset, unit scale) is the identity
     transform and leaves the output unchanged.  The calibration is applied
     by render_label_scene() for the thermal/label path and directly to the
     painter rectangle for the sheet-printer path.
  */
  struct PrinterCalibration
  {
    qreal offset_x = 0.0;  ///< horizontal translation correction in mm
    qreal offset_y = 0.0;  ///< vertical translation correction in mm
    qreal scale_x = 1.0;  ///< horizontal scale correction (1.0 = no change)
    qreal scale_y = 1.0;  ///< vertical scale correction  (1.0 = no change)
  };


  /// Returns the saved calibration for @p printer_name from QSettings.
  /**
     Returns a default-constructed PrinterCalibration (zero offset, unit
     scale) when no data has been saved for this printer yet.
     @param printer_name  printer name as returned by QPrinter::printerName()
  */
  PrinterCalibration load_printer_calibration(QString const& printer_name);


  /// Saves @p cal for @p printer_name to QSettings.
  /**
     @param printer_name  printer name as returned by QPrinter::printerName()
     @param cal           calibration data to persist
  */
  void save_printer_calibration(QString const& printer_name,
                                PrinterCalibration const& cal);

} // namespace colman

#endif /* #ifndef COLMAN_PRINTER_CALIBRATION_HPP */

/* EOF */
