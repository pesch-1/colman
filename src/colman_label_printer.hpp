// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Thermal label printer support.
/**
   @file
   @author Christoph Pesch

   Provides CUPS-based and TSPL-based detection, high-resolution rendering, and
   dialog-free printing for thermal label printers such as the Vevor Y486.

   @par Scene coordinate convention
   All functions that accept a @c QGraphicsScene* expect the scene to use
   <b>1 scene unit = 0.1 mm</b> (10 units per millimetre), which is the
   standard throughout colman.

   @par Typical usage
   @code
   // 1. Find the printer
      QList<colman::ThermalPrinterInfo> printers =
      colman::list_thermal_printers();
      if(printers.isEmpty()) return;   // no thermal printer found

   // 2. Render a prepared QGraphicsScene (e.g. from render_item_card…)
      QSizeF const label(85.6, 54.0);  // mm
      QString error;
      bool ok = colman::print_label_scene(
      &my_scene, label, printers[0].cups_name, printers[0].dpi, error);
      @endcode
 */

#ifndef COLMAN_LABEL_PRINTER_HPP
#  define COLMAN_LABEL_PRINTER_HPP

#  ifndef COLMAN_PRINTER_CALIBRATION_HPP
#    include "colman_printer_calibration.hpp"
#  endif
#  ifndef SYSTEM_QIMAGE
#    include <QImage>
#    define SYSTEM_QIMAGE
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QSIZEF
#    include <QSizeF>
#    define SYSTEM_QSIZEF
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

class QGraphicsScene;
class QWidget;

namespace colman
{

  /// Highest darkness value that thermal label printers actually honour.
  /**
     The print head density is a 4-bit register (TSPL @c DENSITY, range
     0–15), so 15 is the darkest usable setting.  Some CUPS drivers (e.g. the
     Vevor Y486) advertise a wider range such as 0–30, but values above 15 do
     not print darker: the firmware interprets the surplus as a signed/overflow
     value and the result becomes *lighter* again.  All darkness values are
     therefore clamped to this maximum before they reach the printer.
   */
  int constexpr MaxThermalDarkness = 15;


  /// Information about one CUPS-registered label or thermal printer.
  struct ThermalPrinterInfo
  {
    QString cups_name;        ///< system printer name, or @c "usb:vvvv:pppp" for a printer connected directly via USB
    QString display_name;     ///< human-readable model / description
    QSizeF default_size_mm;  ///< default label dimensions in mm; (0,0) if unknown
    int dpi = 203;          ///< native resolution in dots per inch
  };


  /// Returns all label or thermal printers.
  /**
     System printers (CUPS, Windows) qualify by two heuristics:
     - The printer's make-and-model string or CUPS name contains a keyword
       associated with known label/thermal printer brands (Vevor, Brother QL,
       Dymo, Zebra, TSC, Niimbot, Bixolon, HPRT, Rollo, Munbyn, …).
       - The printer reports at least one supported page size whose shorter
         dimension is between 5 mm and 110 mm (typical for label rolls).

     In addition, known TSPL models connected directly via USB (Vevor
     Y486, Munbyn RW403B) are listed as @c "usb:vvvv:pppp" — they print
     over the direct path and need no queue in the printing system.  They
     are recognized by their USB ID; nothing is sent to the printer.

   The returned list may be empty when no such printer is installed.
   */
  QList<ThermalPrinterInfo> list_thermal_printers();


  /// Returns true if the printer @p cups_name is currently available.
  /**
     For a system printer, availability means the name appears in the
     printer list and a USB printer device is connected; for a directly
     connected printer (@c "usb:…"), a device with its USB ID must be
     connected.  No attempt is made to contact the physical device.
     @param cups_name  printer name as stored in ThermalPrinterInfo::cups_name
   */
  bool thermal_printer_ready(QString const& cups_name);


  /// Paper status of a USB printer as reported by the device itself.
  enum struct PaperStatus {
    Ok,      ///< paper (label roll) present
    Empty,   ///< the printer reports "paper empty" — label roll is used up
    Unknown  ///< status could not be determined
  };


  /// Queries the paper status of the USB printer device @p device_path.
  PaperStatus query_paper_status(QString const& device_path);


  /// Returns the combined paper status of all connected USB printers.
  PaperStatus thermal_printer_paper_status();


  /// Sends the label geometry to all connected TSPL/EPL2 label printers.
  /** @return true when the geometry was sent to at least one printer */
  bool prime_thermal_printer_geometry(QSizeF const& size_mm, int dpi);


  /// Shows a warning dialog when the connected label printer is out of labels.
  /** @return true when the label roll is empty and printing should not start */
  bool warn_when_out_of_labels(QWidget* parent);


  /// Watches the printer while it prints and warns when the roll runs out.
  /**
     Call right after a successful send_images_to_thermal_printer().
     Polls the printer's TSPL status while the job is printing; when the
     printer reports an empty label roll mid-job, a warning dialog tells
     the user to insert a new roll (the printer keeps the rest of the job
     in its memory and resumes by itself).  A device that can be opened
     but stays silent is treated as busy printing — the Vevor Y486 does
     not answer status requests while the head runs.  Returns immediately
     when no device node can be opened (CUPS fallback in progress, or no
     device access) and skips printers whose firmware never answers the
     status request (Munbyn RW403B) — the roll cannot be watched there.

   @param parent      parent widget for the dialogs
   @param label_count number of labels in the job just sent
   @param size_mm     physical label size in millimetres
   */
  void warn_when_roll_runs_out_while_printing(QWidget* parent,
                                              int label_count,
                                              QSizeF const& size_mm);


  /// Renders a card scene to a raster image at the given resolution.
  /**
     The scene must use colman's standard coordinate system
     (1 scene unit = 0.1 mm).  The returned image covers @p size_mm exactly
     at @p dpi dots per inch, with DPI metadata embedded.  The alpha channel,
     if any, is composited onto white.

   The printer calibration @p cal, when not the identity transform, shifts
   and scales the rendered content within the (fixed-size) label image to
   compensate for a specific printer's print-position error.  Content moved
   past the label edge is clipped.  The default identity calibration leaves
   the output unchanged.

   When @p rotate_180 is true the scene content is rotated by 180° within
   the label; use this for printers that feed the label "upside down".
   The calibration is applied after the rotation, i.e. it stays in printer
   coordinates — recalibrate when toggling the rotation.

   @param scene      scene to render; sceneRect() should enclose all items
   @param size_mm    physical label dimensions in millimetres (width × height)
   @param dpi        output resolution in dots per inch (203 or 300 are
   typical)
   @param cal        per-printer print-position correction; identity by
   default
   @param rotate_180 rotate the rendered content by 180°
   @return rendered image in QImage::Format_RGB32; null on invalid input
   */
  QImage render_label_scene(QGraphicsScene* scene,
                            QSizeF const& size_mm,
                            int dpi,
                            PrinterCalibration const& cal = {},
                            bool rotate_180 = false);


  /// Sends @p image to a thermal printer without showing a dialog.
  /**
     Convenience wrapper around send_images_to_thermal_printer() for a
     single label; see there for the printing strategy.

   @param image      image to print; produced by render_label_scene()
   @param cups_name  CUPS printer name from ThermalPrinterInfo::cups_name
   @param size_mm    physical label size in millimetres
   @param dpi        image resolution in dots per inch
   @param error      receives a human-readable message on failure
   @param darkness   print darkness (TSPL @c DENSITY / CUPS @c Darkness),
   valid range 0–MaxThermalDarkness; a negative value keeps the printer default
   @return true on success
   */
  bool send_image_to_thermal_printer(QImage const& image,
                                     QString const& cups_name,
                                     QSizeF const& size_mm,
                                     int dpi,
                                     QString& error,
                                     int darkness = -1);


  /// Sends several images to a thermal printer as a single print job.
  /**
     Two strategies, tried in order:

   1. <b>Direct TSPL</b> — every image is thresholded to a 1-bit bitmap
      and the whole batch is written as one raw TSPL job (@c SIZE /
      @c GAP / @c DENSITY, then @c CLS + @c BITMAP + @c PRINT per label)
      to the connected USB printer (see list_thermal_printer_devices())
      that answers the TSPL status probe or whose USB ID names a known
      TSPL model that never answers it (Munbyn RW403B — such models are not probed at all,
      because the probe bytes would make the firmware discard the job).
      The job uses a per-model placement profile, so printers with
      different TSPL origin handling all print edge-exact.  This
      reproduces the rendered labels pixel-exact and avoids the halftone
      grain of the CUPS raster chain; it is the same path the Flutter
      client uses over USB.
   2. <b>CUPS fallback</b> — when no accessible TSPL device answers,
      each image becomes one page of a single CUPS job addressed to
      @p cups_name (as before).

   Bundling all labels into one job avoids the per-job initialisation and
   gap calibration that printers such as the Vevor Y486 perform at the
   start of every job.  All images share the same page size, resolution
   and darkness.

   @param images     labels to print, in order; an empty list is an error
   @param cups_name  CUPS printer name from ThermalPrinterInfo::cups_name;
   used by the CUPS fallback only
   @param size_mm    physical label size in millimetres, applied to every page
   @param dpi        image resolution in dots per inch
   @param error      receives a human-readable message on failure
   @param darkness   print darkness (TSPL @c DENSITY / CUPS @c Darkness),
   valid range 0–MaxThermalDarkness; a negative value keeps the printer default
   @return true on success
   */
  bool send_images_to_thermal_printer(QList<QImage> const& images,
                                      QString const& cups_name,
                                      QSizeF const& size_mm,
                                      int dpi,
                                      QString& error,
                                      int darkness = -1);


  /// Renders @p scene and prints one label on a thermal printer.
  /**
     Convenience wrapper around render_label_scene() and
     send_image_to_thermal_printer().  Both steps are performed; the first
     failure stops processing and populates @p error.  The saved print-position
     calibration for @p cups_name is loaded and applied automatically.

   @param scene      card scene in 0.1 mm/unit coordinates
   @param size_mm    physical label dimensions in millimetres
   @param cups_name  CUPS printer name
   @param dpi        printer resolution (203 is standard for most thermal
   label printers; use 300 for higher-resolution models)
   @param error      receives a human-readable message on failure
   @param darkness   print darkness passed to the CUPS @c Darkness option
   valid range 0–MaxThermalDarkness; a negative value keeps the printer default
   @return true on success
   */
  bool print_label_scene(QGraphicsScene* scene,
                         QSizeF const& size_mm,
                         QString const& cups_name,
                         int dpi,
                         QString& error,
                         int darkness = -1);


  /// Lists all connected USB printer-class devices.
  /** Linux: @c /dev/usb/lp* nodes of usblp; Windows: usbprint.sys device
      interfaces.  Intended for direct device access (UsbPrinterDevice),
      e.g. raw TSPL jobs and status queries. */
  QList<QString> list_thermal_printer_devices();

} // namespace colman

#endif /* #ifndef COLMAN_LABEL_PRINTER_HPP */

/* EOF */
