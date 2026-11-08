// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of thermal label printer support.
/**
   @file
   @author Christoph Pesch

   Scene coordinate system: 1 unit = 0.1 mm (matches LabelLayoutWidget).
   All public functions work in millimetres; pixel conversion is local.

   @par Linux / CUPS notes
   Thermal label printers (Vevor Y486, Brother QL, Dymo LabelWriter, Zebra,
   TSC, …) are accessed through CUPS exactly like any other printer.  The
   key differences from standard office printing are:
   - Zero page margins (full-bleed on the label).
   - A custom page size that matches the physical label roll exactly.
   - No multi-copy or collation — each label is an independent print job.

   QPrinter::setFullPage(true) and QPageSize(mm, QPageSize::Millimeter) are
   used to achieve zero margins and custom sizes.  If the CUPS driver for a
   particular printer does not accept arbitrary page sizes, the user can
   instead call render_label_scene() + the operating system's @c lp command:
   @code
   lp -d VevorY486 -o PageSize=Custom.85.6x54mm -o print-scaling=none img.png
   @endcode
*/

#include "colman_label_printer.hpp"

#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QPAGELAYOUT
#  include <QPageLayout>
#  define SYSTEM_QPAGELAYOUT
#endif
#ifndef SYSTEM_QPAGESIZE
#  include <QPageSize>
#  define SYSTEM_QPAGESIZE
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_CMATH
#  include <cmath>
#  define SYSTEM_CMATH
#endif
#ifndef SYSTEM_QPRINTERINFO
#  include <QPrinterInfo>
#  define SYSTEM_QPRINTERINFO
#endif
#ifndef SYSTEM_QPRINTENGINE
#  include <QPrintEngine>
#  define SYSTEM_QPRINTENGINE
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QELAPSEDTIMER
#  include <QElapsedTimer>
#  define SYSTEM_QELAPSEDTIMER
#endif
#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif
#ifndef SYSTEM_QPROGRESSDIALOG
#  include <QProgressDialog>
#  define SYSTEM_QPROGRESSDIALOG
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef COLMAN_USB_PRINTER_HPP
#  include "colman_usb_printer.hpp"
#endif


namespace
{

/// Brand/category keywords that identify label or thermal printers.
/** All strings must be lower-case; comparison is case-insensitive. */
  char const* const ThermalKeywords[] = {
      "vevor", "brother ql", "ql-", "dymo", "zebra",
      "tsc", "niimbot", "bixolon", "hprt", "rollo",
      "munbyn", "citizen cl", "label", "thermal", "pos-",
      "receipt", "203dpi", "300dpi"};

/// Returns true if @p text contains any ThermalKeywords entry.
  bool matches_thermal_keyword(QString const& text)
  {
    QString const lower = text.toLower();
    for(char const* kw : ThermalKeywords) {
      if(lower.contains(QLatin1String(kw))) {
        return true;
      }
    }
    return false;
  }

/// Returns true when @p pi reports at least one label-sized page size.
/**
   A size qualifies when its shorter dimension is between 5 mm and 65 mm.
   This covers common thermal label rolls (58 mm, 62 mm) but excludes
   photo paper (4×6 = 102 mm), envelopes, and other non-label media
   that inkjet and laser printers may report.
*/
  bool has_label_sized_page(QPrinterInfo const& pi)
  {
    for(QPageSize const& ps : pi.supportedPageSizes()) {
      QSizeF const s = ps.size(QPageSize::Millimeter);
      double const min_dim = qMin(s.width(), s.height());
      if(min_dim > 5.0 && min_dim < 65.0) {
        return true;
      }
    }
    return false;
  }

/// "Paper empty" bit of the USB printer-class port status byte.
/**
   The byte returned by the @c LPGETSTATUS ioctl follows the IEEE 1284
   convention: bit 5 = paper empty, bit 4 = selected, bit 3 = not-error.
*/
  int constexpr PaperEmptyStatusBit = 0x20;

/// TSPL immediate status request @c "ESC !?" (three bytes, no terminator).
/**
   The firmware answers with a single status byte right away; the request
   is handled out-of-band and is neither spooled nor printed.
*/
  char constexpr TsplStatusRequest[] = {0x1b, '!', '?'};

/// "Out of paper" bit of the TSPL status byte answered for @c "ESC !?".
/**
   Verified on real hardware: the printer answers 0x00 with a label roll
   inserted and 0x07 (head opened | paper jam | out of paper) with the
   roll removed.
*/
  int constexpr TsplOutOfPaperBit = 0x04;

/// "Printing" bit of the TSPL status byte answered for @c "ESC !?".
/** Not every firmware reports this bit; the post-print watch treats it
    as an optional early-exit signal only. */
  int constexpr TsplPrintingStatusBit = 0x20;

/// Milliseconds between two status polls while watching a print job.
  int constexpr TsplMonitorIntervalMs = 500;

/// Consecutive idle answers after which the watch decides that nothing
/// is being printed on the watched device and gives up quietly.
  int constexpr TsplMonitorIdlePolls = 3;

/// Fixed setup allowance of the watch deadline, in milliseconds.
  int constexpr TsplMonitorSetupMs = 3000;

/// Watch-deadline allowance per label, in milliseconds.
/**
   Well above the measured per-label cycle of the Vevor Y486 (~400 ms
   for a 15 mm label including positioning).  The deadline is only a
   safety net: normally the watch ends as soon as the printer answers
   idle after the job.
*/
  int constexpr TsplMonitorMsPerLabel = 1000;

/// Milliseconds to wait for the printer's answer to a TSPL status request.
  int constexpr TsplStatusTimeoutMs = 500;

/// Assumed gap between two labels on the roll, in millimetres.
/**
   Sent to the printer together with the label height so that it can
   position the next label after each print.  2 mm is the common gap of
   standard direct-thermal rolls; the gap sensor still fine-tunes on the
   real label edge, so small deviations are harmless.
*/
  double constexpr LabelGapMm = 2.0;

/// Writes the TSPL status request to @p device and reads the answer byte.
/**
   The firmware does not answer at all while the print head is running
   (verified on the Vevor Y486) and delivers late answers only after
   the head stops; such uncollected answers stay queued in the device.
   To keep request and answer paired, everything still queued is
   discarded before the request is written, and the freshest byte wins
   when more than one arrives.

   @param device open USB printer device
   @return the status byte (>= 0), or -1 when the printer did not answer
           within TsplStatusTimeoutMs (e.g. a non-TSPL printer, or a
           printer that is busy printing)
*/
  int tspl_query_status(colman::UsbPrinterDevice& device)
  {
    device.discard_input();
    if(!device.write_request(
           QByteArray(TsplStatusRequest, sizeof(TsplStatusRequest))))
    {
      return -1;
    }
    return device.read_last_byte(TsplStatusTimeoutMs);
  }

/// Performs the TSPL status query on an idle printer, with one retry.
/**
   Used before a print, when an idle TSPL printer is expected to answer.
   On Windows an answer can get lost after an earlier read request had to
   be cancelled (see colman::UsbPrinterDevice::discard_input()), so a
   missing answer is requested once more before the printer counts as
   silent.

   @param device open USB printer device
   @return the status byte (>= 0), or -1 when the printer did not answer
*/
  int tspl_query_idle_status(colman::UsbPrinterDevice& device)
  {
    int const answer = tspl_query_status(device);
    if(answer >= 0) {
      return answer;
    }
    return tspl_query_status(device);
  }

/// Opens @p device_path and performs one TSPL status query.
/**
   The two failure modes are kept apart because they mean different
   things to the post-print watch: a device that cannot be opened is
   gone or inaccessible (CUPS detached usblp, missing permission, a
   Windows print queue holding the port),
   while an open device that stays silent is — on the Vevor Y486 —
   busy printing.

   @return the status byte (>= 0), -1 when the device did not answer,
           or -2 when it could not be opened
*/
  int tspl_poll_device(QString const& device_path)
  {
    colman::UsbPrinterDevice device(device_path);
    if(!device.is_open()) {
      return -2;
    }
    return tspl_query_status(device);
  }

/// TSPL @c REFERENCE x coordinate (dots) of the Vevor Y486.
/**
   Aligns the print origin with the loaded label roll on the Vevor
   Y486.  The value matches the Flutter client's USB printing code, so
   desktop and mobile prints land on the same spot.
*/
  int constexpr Y486ReferenceXDots = -64;

/// Horizontal bitmap position (mm) of the Vevor Y486.
/**
   Together with Y486ReferenceXDots this reproduces the calibrated
   label placement of the Flutter client on the Vevor Y486.
*/
  double constexpr Y486BitmapXOffsetMm = 11.0;

/// Placement and capability profile of one TSPL printer model.
/**
   TSPL origin handling differs between firmwares: the Vevor Y486 needs
   the print origin shifted via @c REFERENCE and the bitmap indented to
   land on the label, while the Munbyn RW403B prints edge-exact from
   the standard origin (both verified on hardware).  The profile also
   records whether the firmware answers the TSPL status request at all:
   the RW403B accepts jobs but never sends a byte back, so all
   status-based features are skipped for it.

   The Y486 additionally takes its print darkness only from the EPL2
   density register: its bilingual firmware accepts TSPL @c DENSITY
   but the value has virtually no effect on the print (verified on
   hardware — thin strokes react clearly to the EPL2 @c D register,
   the path its CUPS driver uses, and barely to @c DENSITY).
   epl2_density_ requests the EPL2 prefix; it must stay off for other
   models — the RW403B discards jobs preceded by foreign commands.
*/
  struct TsplDeviceProfile
  {
    int reference_x_dots_ = 0;         ///< TSPL @c REFERENCE x (dots)
    double bitmap_x_offset_mm_ = 0.0;  ///< horizontal @c BITMAP position (mm)
    bool answers_status_ = true;       ///< firmware answers @c "ESC !?"
    bool epl2_density_ = false;        ///< darkness needs the EPL2 @c D register
  };

/// Returns the profile of the printer behind @p device_path.
/**
   Known models are identified by their USB vendor/product ID; unknown
   models get the default profile (standard TSPL origin, status request
   assumed to work).
*/
  TsplDeviceProfile tspl_device_profile(QString const& device_path)
  {
    QString const usb_id = colman::usb_printer_id(device_path);
    if(usb_id == "5958:0486") {  // Vevor Y486
      return {Y486ReferenceXDots, Y486BitmapXOffsetMm, true, true};
    }
    if(usb_id == "0d28:ccdd") {  // Munbyn RW403B
      return {0, 0.0, false};
    }
    return {};
  }

/// Milliseconds to wait for the device to accept more raw job data.
  int constexpr TsplWriteTimeoutMs = 5000;

/// Formats a millimetre value for TSPL: integer when whole, else 1 decimal.
  QByteArray tspl_mm(double v)
  {
    double const rounded = std::round(v);
    if(std::abs(v - rounded) < 1e-9) {
      return QByteArray::number(static_cast<int>(rounded));
    }
    return QByteArray::number(v, 'f', 1);
  }

/// Packs @p image into 1-bit rows for the TSPL @c BITMAP command.
/**
   Rows are packed MSB-first with bit value 0 = black (printed) — the
   polarity the TSPL @c BITMAP command expects.  Every pixel is
   thresholded at 50 % grey: the same black-or-white decision the CUPS
   raster chain makes per pixel, but without the halftone dithering
   that makes text edges grainy.
*/
  QByteArray pack_mono_bitmap(QImage const& image)
  {
    int const w = image.width();
    int const h = image.height();
    int const width_bytes = (w + 7) / 8;
    QByteArray packed(width_bytes * h, static_cast<char>(0xff));
    char* bits = packed.data();
    for(int y = 0; y < h; ++y) {
      for(int x = 0; x < w; ++x) {
        if(qGray(image.pixel(x, y)) < 128) {
          bits[y * width_bytes + (x >> 3)] &= static_cast<char>(~(0x80 >> (x & 7)));
        }
      }
    }
    return packed;
  }

/// Builds one TSPL job that prints each of @p images on its own label.
/**
   The printer is set up once (SIZE / GAP / DIRECTION / REFERENCE /
   DENSITY), then every label is a @c CLS … @c BITMAP … @c PRINT 1
   block.  The geometry travels with the job, so no separate priming of
   the stored form length is needed.

   The bitmap is drawn in mode 1 (OR): onto the CLS-cleared buffer this
   produces exactly the overwrite result, but unlike mode 0 it works on
   every tested firmware — the Munbyn RW403B silently discards whole
   jobs whose @c BITMAP uses mode 0 (verified on hardware).

   When the profile requests it, the job is prefixed with the EPL2
   density register command @c "\\nN\\nD<darkness>\\n": the Vevor
   Y486's bilingual firmware accepts TSPL @c DENSITY but the value has
   virtually no effect — only the EPL2 register (the path its CUPS
   driver uses) changes the print darkness (verified on hardware).  A
   negative @p darkness keeps the printer default: no prefix is sent.

   @param images   pre-rendered labels, all the same size
   @param size_mm  physical label size in millimetres (exact, without
                   dot-rounding — used for the SIZE command)
   @param dpi      printer resolution in dots per inch
   @param darkness TSPL @c DENSITY value (0–MaxThermalDarkness); a
                   negative value keeps the printer default
   @param profile  placement profile of the addressed printer
   @return the complete job byte stream
*/
  QByteArray build_tspl_job(QList<QImage> const& images,
                            QSizeF const& size_mm,
                            int dpi,
                            int darkness,
                            TsplDeviceProfile const& profile)
  {
    double const mm_to_dots = static_cast<double>(dpi) / 25.4;
    int const x_dots =
        static_cast<int>(std::round(profile.bitmap_x_offset_mm_ * mm_to_dots));
    QByteArray job;
    if(profile.epl2_density_ && darkness >= 0) {
      job += "\nN\nD" +
             QByteArray::number(qMin(darkness, colman::MaxThermalDarkness)) +
             "\n";
    }
    job += "SIZE " + tspl_mm(size_mm.width()) + " mm," + tspl_mm(size_mm.height()) + " mm\r\n";
    job += "GAP " + tspl_mm(LabelGapMm) + " mm,0 mm\r\n";
    job += "DIRECTION 0\r\n";
    job += "REFERENCE " + QByteArray::number(profile.reference_x_dots_) + ",0\r\n";
    if(darkness >= 0) {
      job += "DENSITY " + QByteArray::number(qMin(darkness, colman::MaxThermalDarkness)) + "\r\n";
    }
    for(QImage const& image : images) {
      int const width_bytes = (image.width() + 7) / 8;
      job += "CLS\r\n";
      job += "BITMAP " + QByteArray::number(x_dots) + ",0," + QByteArray::number(width_bytes) + "," + QByteArray::number(image.height()) + ",1,";
      job += pack_mono_bitmap(image);
      job += "\r\nPRINT 1\r\n";
    }
    return job;
  }

/// Sends @p images as one raw TSPL job to the first eligible USB device.
/**
   A device is eligible when it answers the TSPL status probe, or when
   its USB ID identifies a known TSPL model whose firmware never
   answers (the Munbyn RW403B).  Such models must not be probed at
   all: the probe bytes enter the printer's command stream, and a
   firmware that does not handle @c "ESC !?" out-of-band silently
   discards the whole job that follows them (verified on the RW403B).
   Unknown devices that stay silent are left to the CUPS fallback —
   sending raw TSPL to an unidentified printer could emit garbage on
   models that speak another language (Brother QL, Dymo, ZPL-only
   Zebra, …).

   The job is built with the placement profile of the chosen device, so
   models with different TSPL origin handling all print edge-exact.
   A CUPS printer name cannot be mapped to a device node reliably, so —
   like the paper-status and priming helpers — this addresses the
   connected TSPL printer, which is exact for the usual
   single-label-printer setup.

   UsbPrinterDevice::write_all() waits until the printer has accepted
   every byte before the device is closed — see there for why closing
   earlier would truncate the job.

   @param images   pre-rendered labels, all the same size
   @param size_mm  physical label size in millimetres
   @param dpi      printer resolution in dots per inch
   @param darkness TSPL @c DENSITY value; negative keeps the default
   @return true when the whole job was accepted by a printer
*/
  bool send_tspl_job_to_device(QList<QImage> const& images,
                               QSizeF const& size_mm,
                               int dpi,
                               int darkness)
  {
    for(QString const& dev : colman::list_thermal_printer_devices()) {
      colman::UsbPrinterDevice device(dev);
      if(!device.is_open()) {
        continue;
      }
      TsplDeviceProfile const profile = tspl_device_profile(dev);
      // A profile that says "never answers" marks a known TSPL model:
      // eligible without probing (the probe would kill the job, see
      // above).  Everything else must answer the probe.
      bool const eligible =
          !profile.answers_status_ || tspl_query_idle_status(device) >= 0;
      if(!eligible) {
        continue;
      }
      QByteArray const job =
          build_tspl_job(images, size_mm, dpi, darkness, profile);
      if(device.write_all(job, TsplWriteTimeoutMs)) {
        return true;
      }
    }
    return false;
  }

} // anonymous namespace


// ---------------------------------------------------------------------------
// list_thermal_printers
// ---------------------------------------------------------------------------

QList<colman::ThermalPrinterInfo> colman::list_thermal_printers()
{
  QList<ThermalPrinterInfo> result;
  for(QPrinterInfo const& pi : QPrinterInfo::availablePrinters()) {
    if(pi.printerName().isEmpty()) {
      continue;
    }

    bool const thermal =
        matches_thermal_keyword(pi.makeAndModel()) ||
        matches_thermal_keyword(pi.printerName()) ||
        has_label_sized_page(pi);
    if(!thermal) {
      continue;
    }

    ThermalPrinterInfo info;
    info.cups_name = pi.printerName();
    info.display_name = pi.makeAndModel().isEmpty()
                            ? pi.printerName()
                            : pi.makeAndModel();
    info.dpi = 203; // conservative default (industry-standard thermal DPI)

    QPageSize const ps = pi.defaultPageSize();
    QSizeF const s = ps.size(QPageSize::Millimeter);
    if(s.width() > 0.0 && s.height() > 0.0) {
      info.default_size_mm = s;
    }
    result.append(info);
  }
  return result;
}


// ---------------------------------------------------------------------------
// thermal_printer_ready
// ---------------------------------------------------------------------------

bool colman::thermal_printer_ready(QString const& cups_name)
{
  if(cups_name.isEmpty()) {
    return false;
  }
  bool in_cups = false;
  for(QPrinterInfo const& pi : QPrinterInfo::availablePrinters()) {
    if(pi.printerName() == cups_name) {
      in_cups = true;
      break;
    }
  }
  if(!in_cups) {
    return false;
  }
  // CUPS keeps printers registered even when they are physically
  // disconnected and silently queues jobs for offline devices.
  // QPrinterInfo::state() was unreliable for this: it always reported
  // Ready even for powered-off devices. Checking whether a USB device
  // node exists is a more dependable signal that the printer is present.
  return !list_thermal_printer_devices().isEmpty();
}


// ---------------------------------------------------------------------------
// render_label_scene
// ---------------------------------------------------------------------------

QImage colman::render_label_scene(QGraphicsScene* scene,
                                  QSizeF const& size_mm,
                                  int dpi,
                                  PrinterCalibration const& cal,
                                  bool rotate_180)
{
  if(!scene || size_mm.width() <= 0.0 || size_mm.height() <= 0.0 || dpi <= 0) {
    return {};
  }

  // Convert physical dimensions to pixel counts at the requested DPI.
  double const mm_to_px = static_cast<double>(dpi) / 25.4;
  int const w = qMax(1, static_cast<int>(std::round(size_mm.width() * mm_to_px)));
  int const h = qMax(1, static_cast<int>(std::round(size_mm.height() * mm_to_px)));

  // Embed resolution metadata so that CUPS and image viewers honour it.
  int const dots_per_meter =
      static_cast<int>(std::round(static_cast<double>(dpi) / 0.0254));

  QImage img(w, h, QImage::Format_RGB32);
  img.setDotsPerMeterX(dots_per_meter);
  img.setDotsPerMeterY(dots_per_meter);
  img.fill(Qt::white);

  QPainter p(&img);
  // No edge antialiasing: the CUPS chain rasterises the label to 1-bit,
  // and grey edge pixels come out as scattered halftone dots — the print
  // looks grainy.  Hard black/white edges reproduce exactly.  Smooth
  // pixmap scaling stays on for photos/logos embedded in the scene,
  // whose grey tones are meant to be halftoned.
  p.setRenderHint(QPainter::Antialiasing, false);
  p.setRenderHint(QPainter::TextAntialiasing, false);
  p.setRenderHint(QPainter::SmoothPixmapTransform);
  // The calibration shifts (offset in mm → pixels) and scales the content
  // to compensate for the printer's print-position error; content pushed
  // past the image edge is clipped.
  p.translate(cal.offset_x * mm_to_px, cal.offset_y * mm_to_px);
  p.scale(cal.scale_x, cal.scale_y);
  // Optional half-turn for printers that feed the label "upside down".
  // Applied before the calibration in the transform chain so that the
  // calibration stays in printer coordinates.
  if(rotate_180) {
    p.translate(w / 2.0, h / 2.0);
    p.rotate(180.0);
    p.translate(-w / 2.0, -h / 2.0);
  }
  // Map the scene's own sceneRect onto the (full) image rectangle so that
  // the rendered result is correctly scaled regardless of scene origin.
  scene->render(&p, QRectF(0.0, 0.0, w, h), scene->sceneRect());
  p.end();

  return img;
}


// ---------------------------------------------------------------------------
// send_image_to_thermal_printer
// ---------------------------------------------------------------------------

bool colman::send_image_to_thermal_printer(QImage const& image,
                                           QString const& cups_name,
                                           QSizeF const& size_mm,
                                           int dpi,
                                           QString& error,
                                           int darkness)
{
  return send_images_to_thermal_printer(
      QList<QImage>{image}, cups_name, size_mm, dpi, error, darkness);
}


// ---------------------------------------------------------------------------
// send_images_to_thermal_printer
// ---------------------------------------------------------------------------

bool colman::send_images_to_thermal_printer(QList<QImage> const& images,
                                            QString const& cups_name,
                                            QSizeF const& size_mm,
                                            int dpi,
                                            QString& error,
                                            int darkness)
{
  if(images.isEmpty()) {
    error = QApplication::tr("No labels to print.");
    return false;
  }
  for(QImage const& image : images) {
    if(image.isNull()) {
      error = QApplication::tr("Label image is empty.");
      return false;
    }
  }
  if(cups_name.isEmpty()) {
    error = QApplication::tr("No printer specified.");
    return false;
  }

  // Preferred path: threshold the labels to 1-bit and send them to the
  // printer as one raw TSPL job.  This bypasses the CUPS raster chain
  // (PDF → Ghostscript → EPL2 driver), which rescales the page image by a
  // fractional pixel and halftones the resulting grey edge pixels — the
  // cause of visibly grainy text.  The direct job reproduces the rendered
  // label pixel-exact; the Flutter client prints the same way over USB.
  if(send_tspl_job_to_device(images, size_mm, dpi, darkness)) {
    return true;
  }

  // CUPS fallback for printers without an accessible TSPL device node
  // (missing /dev/usb/lp* permission, network printers, non-TSPL models).
  // The printer's stored form length is volatile (lost on power-cycle) and
  // the CUPS raster driver does not transmit it, so push the label geometry
  // to the device first — otherwise printers such as the Vevor Y486 feed
  // several blank labels after each printed one.
  prime_thermal_printer_geometry(size_mm, dpi);

  // Build a QPrinter that targets the label dimensions exactly.
  // QPrinter::HighResolution is required for correct scaling when the
  // printer DPI differs from the screen DPI.
  QPrinter printer(QPrinter::HighResolution);
  printer.setPrinterName(cups_name);
  printer.setFullPage(true);  // paint rect covers the entire label, no margins
  printer.setPageSize(QPageSize(size_mm, QPageSize::Millimeter));
  printer.setPageMargins(
      QMarginsF(0.0, 0.0, 0.0, 0.0), QPageLayout::Millimeter);
  printer.setResolution(dpi);
  printer.setCopyCount(1);

  // Pass the print darkness through to CUPS as the driver's "Darkness"
  // option.  PPK_CupsOptions (0xfe00) is a stable but private Qt key that
  // takes a flat list of key/value pairs.  A negative darkness means "leave
  // the printer's own default untouched".  The driver may advertise a wider
  // range (the Vevor Y486 reports 0–30), but only 0–MaxThermalDarkness print
  // progressively darker; higher values overflow and become lighter, so the
  // value is clamped here regardless of where it originated.
  if(darkness >= 0) {
    int const clamped = qMin(darkness, MaxThermalDarkness);
    printer.printEngine()->setProperty(
        QPrintEngine::PrintEnginePropertyKey(0xfe00),
        QStringList() << QStringLiteral("Darkness")
                      << QString::number(clamped));
  }

  QPainter painter;
  if(!painter.begin(&printer)) {
    error = QApplication::tr("Could not open printer \"%1\".").arg(cups_name);
    return false;
  }

  // Emit every label as a page of the same job; newPage() between pages keeps
  // the printer from re-initialising and recalibrating between labels.
  for(int i = 0; i < images.size(); ++i) {
    if(i > 0) {
      printer.newPage();
    }
    // Draw the image 1:1 in device pixels.  The device size may differ
    // from the rendered image by one pixel (Qt rounds the page size to
    // whole points, e.g. 15 mm → 43 pt → 121 px while the image has
    // 120 px); scaling the image to the device rect would resample every
    // row, and the 1-bit halftone step downstream turns the resulting
    // grey values into visible grain across the whole label.
    painter.drawImage(QPointF(0.0, 0.0), images[i]);
  }
  painter.end();

  return true;
}


// ---------------------------------------------------------------------------
// print_label_scene
// ---------------------------------------------------------------------------

bool colman::print_label_scene(QGraphicsScene* scene,
                               QSizeF const& size_mm,
                               QString const& cups_name,
                               int dpi,
                               QString& error,
                               int darkness)
{
  QImage const img = render_label_scene(
      scene, size_mm, dpi, load_printer_calibration(cups_name));
  if(img.isNull()) {
    error = QApplication::tr(
        "Failed to render label (scene is null or label dimensions are zero).");
    return false;
  }
  return send_image_to_thermal_printer(
      img, cups_name, size_mm, dpi, error, darkness);
}

// ---------------------------------------------------------------------------
// list_thermal_printer_devices
// ---------------------------------------------------------------------------


/**
   Generic detection: every connected USB printer-class device is
   returned, independent of the printer's vendor and model — on Linux the
   @c /dev/usb/lp* nodes of the usblp kernel driver, on Windows the
   device interfaces of usbprint.sys (see list_usb_printer_devices()).

   On Linux the nodes disappear temporarily while a CUPS job is being
   printed, because the CUPS usb backend detaches the usblp kernel driver
   for the duration of the job.

   @return device paths for UsbPrinterDevice
*/
QList<QString> colman::list_thermal_printer_devices()
{
  return list_usb_printer_devices();
}


// ---------------------------------------------------------------------------
// query_paper_status
// ---------------------------------------------------------------------------

/**
   Two-stage query:

   1. The USB printer-class port status byte is read (@c LPGETSTATUS
      ioctl of usblp on Linux, @c IOCTL_USBPRINT_GET_LPT_STATUS of
      usbprint.sys on Windows).  This is a passive
      class-level request that does not interfere with the printer's data
      stream, so it is safe on any USB printer regardless of its command
      language.  Unfortunately many label printer firmwares do not answer
      the underlying @c GET_PORT_STATUS request (the Vevor Y486 and the
      Munbyn RW403B fail with @c EIO), hence stage 2.
   2. The TSPL immediate status request @c "ESC !?" is written to the
      device and the one-byte answer is read back (with a timeout, since
      a non-TSPL printer never answers).  This is an active query: the
      three request bytes enter the printer's data stream.  TSPL and
      ZPL-compatible firmwares handle them out-of-band without printing
      anything; an exotic non-TSPL device on @c /dev/usb/lp* might
      however interpret them as data.  The query is therefore only
      triggered from the label-printing paths, right before a print —
      and never for models whose firmware is known not to answer it
      (Munbyn RW403B): there the request bytes would enter the command
      stream and make the firmware discard the whole following job.

   PaperStatus::Unknown is returned when the device cannot be opened
   (missing permission, busy because a CUPS job is being printed, or held
   by a Windows print queue) or
   when both stages fail.  Note that a firmware that supports neither
   query can never report an empty roll — printing then proceeds as
   before this check existed.

   @param device_path device path from list_thermal_printer_devices()
   @return paper status as reported by the printer
*/
colman::PaperStatus colman::query_paper_status(QString const& device_path)
{
  UsbPrinterDevice device(device_path);
  if(!device.is_open()) {
    return PaperStatus::Unknown;
  }

  // Stage 1: passive USB printer-class port status byte.
  int const port_status = device.port_status();
  if(port_status >= 0) {
    return (port_status & PaperEmptyStatusBit) ? PaperStatus::Empty
                                               : PaperStatus::Ok;
  }

  // Stage 2: active TSPL status request.  Never sent to models whose
  // firmware does not answer it (Munbyn RW403B): the request bytes
  // would enter the printer's command stream and make it discard the
  // whole job that follows.
  if(!tspl_device_profile(device_path).answers_status_) {
    return PaperStatus::Unknown;
  }
  int const answer = tspl_query_idle_status(device);
  if(answer < 0) {
    return PaperStatus::Unknown;
  }
  return (answer & TsplOutOfPaperBit) ? PaperStatus::Empty : PaperStatus::Ok;
}


// ---------------------------------------------------------------------------
// thermal_printer_paper_status
// ---------------------------------------------------------------------------

/**
   Combines query_paper_status() over all device nodes returned by
   list_thermal_printer_devices().  A CUPS printer name cannot be mapped
   to a device node reliably, so the result is intentionally
   conservative: PaperStatus::Empty is only reported when at least one
   printer reports "paper empty" and none reports "paper present" — with
   a single connected label printer (the usual setup) this is exact, and
   with several printers it never blocks printing on the wrong device.

   @return combined paper status of all connected USB printers
*/
colman::PaperStatus colman::thermal_printer_paper_status()
{
  bool any_ok = false;
  bool any_empty = false;
  for(QString const& dev : list_thermal_printer_devices()) {
    switch(query_paper_status(dev)) {
    case PaperStatus::Ok:
      any_ok = true;
      break;
    case PaperStatus::Empty:
      any_empty = true;
      break;
    case PaperStatus::Unknown:
      break;
    }
  }
  if(any_ok) {
    return PaperStatus::Ok;
  }
  if(any_empty) {
    return PaperStatus::Empty;
  }
  return PaperStatus::Unknown;
}


// ---------------------------------------------------------------------------
// prime_thermal_printer_geometry
// ---------------------------------------------------------------------------

/**
   Thermal label printers position each label using a stored form length
   (label height plus gap).  That setting is volatile on printers such as
   the Vevor Y486: after a power cycle it reverts to a default that
   rarely matches the loaded roll, and the CUPS raster driver never
   transmits the form length itself — the printer then feeds several
   blank labels after every printed one.  This function therefore pushes
   the label geometry directly to the device right before a print.

   Only printers that answer the TSPL status request are configured (the
   same probe as query_paper_status()); other devices are left
   untouched.  The EPL2 form-length command @c Q<height>,<gap> is used
   because the CUPS jobs are emitted in EPL2 by the rastertolabel
   driver; TSPL-only firmwares ignore the unknown line.

   @param size_mm physical label size in millimetres
   @param dpi     printer resolution in dots per inch
   @return true when the geometry was sent to at least one printer
*/
bool colman::prime_thermal_printer_geometry(QSizeF const& size_mm, int dpi)
{
  if(size_mm.height() <= 0.0 || dpi <= 0) {
    return false;
  }
  int const height_dots =
      static_cast<int>(std::round(size_mm.height() * dpi / 25.4));
  int const gap_dots =
      static_cast<int>(std::round(LabelGapMm * dpi / 25.4));
  QByteArray const cmd = "Q" + QByteArray::number(height_dots) + "," + QByteArray::number(gap_dots) + "\r\n";

  bool primed = false;
  for(QString const& dev : list_thermal_printer_devices()) {
    if(!tspl_device_profile(dev).answers_status_) {
      // The firmware never answers the probe (Munbyn RW403B); probing
      // would only pollute the command stream ahead of the CUPS job,
      // and its CUPS driver transmits the geometry itself.
      continue;
    }
    UsbPrinterDevice device(dev);
    if(!device.is_open()) {
      continue;
    }
    if(tspl_query_idle_status(device) >= 0 &&
       device.write_all(cmd, TsplWriteTimeoutMs))
    {
      primed = true;
    }
  }
  return primed;
}


// ---------------------------------------------------------------------------
// warn_when_out_of_labels
// ---------------------------------------------------------------------------

/**
   Convenience pre-print check for the label-printer paths: when
   thermal_printer_paper_status() reports an empty label roll, a warning
   dialog is shown and true is returned so that the caller can abort.
   PaperStatus::Unknown does not warn — printing proceeds when the
   status cannot be determined.

   @param parent parent widget for the warning dialog
   @return true when the label roll is empty and printing should not start
*/
bool colman::warn_when_out_of_labels(QWidget* parent)
{
  if(thermal_printer_paper_status() != PaperStatus::Empty) {
    return false;
  }
  QMessageBox::warning(
      parent,
      QApplication::tr("Out of labels"),
      QApplication::tr(
          "The label printer reports that it is out of labels. "
          "Please insert a new label roll and try again."));
  return true;
}


// ---------------------------------------------------------------------------
// warn_when_roll_runs_out_while_printing
// ---------------------------------------------------------------------------

/**
   Post-print companion to warn_when_out_of_labels(): once a raw TSPL
   job has been handed to the printer, the roll can still run out in
   the middle of the batch.  The printer then pauses with the rest of
   the job in its memory and resumes by itself when a new roll is
   inserted — but without this watch the user would not be told why
   printing stopped.

   Verified behaviour of the Vevor Y486 that shapes this logic:
   - While the head is printing, the firmware does not answer the TSPL
     status request at all.  Silence from an open device is therefore
     the "busy printing" signal, not a failure.
   - When the roll runs out mid-job the firmware starts answering
     again, with the out-of-paper bit set (0x24, then 0x06/0x07).
   - After the job the firmware answers 0x00, and it prints the rest
     of an interrupted job by itself once a new roll is inserted.

   The watch therefore runs until the first idle answer after any
   activity (silence or the "printing" bit), shows the warning dialog
   as soon as an answer carries the out-of-paper bit, and quits early
   when consecutive idle answers show that nothing is printing (the
   job went through the CUPS fallback and has not started yet).  The
   deadline is a pure safety net.  When no device node can even be
   opened, there is nothing to watch (CUPS detached usblp, or no
   permission) and the function returns immediately.  Devices whose
   firmware never answers the status request (Munbyn RW403B) are
   skipped for the same reason.

   While watching, a modal progress dialog blocks re-entry into the
   print actions; its Cancel button only stops the watch, never the
   printer.

   @param parent      parent widget for the dialogs
   @param label_count number of labels in the job just sent
   @param size_mm     physical label size in millimetres
*/
void colman::warn_when_roll_runs_out_while_printing(QWidget* parent,
                                                    int label_count,
                                                    QSizeF const& size_mm)
{
  if(label_count <= 0 || size_mm.height() <= 0.0) {
    return;
  }

  // Pick the device to watch: prefer one that answers right away
  // (idle, or a printer that answers while busy); otherwise take the
  // first one that can be opened — the Y486 stays silent exactly
  // while it prints.
  QString device;
  int status = -1;
  for(QString const& dev : list_thermal_printer_devices()) {
    if(!tspl_device_profile(dev).answers_status_) {
      // The firmware never answers the status request (Munbyn RW403B):
      // this device cannot be watched, and its silence must not be
      // mistaken for "busy printing".
      continue;
    }
    int const answer = tspl_poll_device(dev);
    if(answer >= 0) {
      device = dev;
      status = answer;
      break;
    }
    if(answer == -1 && device.isEmpty()) {
      device = dev;
    }
  }
  if(device.isEmpty()) {
    return;
  }

  int const deadline_ms =
      TsplMonitorSetupMs + label_count * TsplMonitorMsPerLabel;

  bool seen_activity =
      status < 0 || (status & TsplPrintingStatusBit) != 0;
  bool out_of_paper = status >= 0 && (status & TsplOutOfPaperBit) != 0;
  if(!out_of_paper) {
    QProgressDialog progress(QApplication::tr("Printing labels…"),
                             QApplication::tr("Cancel"), 0, 0, parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();

    QElapsedTimer elapsed;
    elapsed.start();
    int idle_polls = 0;
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(TsplMonitorIntervalMs);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
      int const answer = tspl_poll_device(device);
      if(answer >= 0) {
        if(answer & TsplOutOfPaperBit) {
          out_of_paper = true;
          loop.quit();
          return;
        }
        if(answer & TsplPrintingStatusBit) {
          seen_activity = true;
          idle_polls = 0;
        }
        else {
          if(seen_activity) {
            // First idle answer after activity: the job is done.
            loop.quit();
            return;
          }
          ++idle_polls;
          if(idle_polls >= TsplMonitorIdlePolls) {
            // Only idle answers so far: nothing is printing here.
            loop.quit();
            return;
          }
        }
      }
      else {
        // Silence (or a temporarily missing node) while the job is
        // running: the head is printing.
        seen_activity = true;
        idle_polls = 0;
      }
      if(elapsed.elapsed() >= deadline_ms) {
        loop.quit();
      }
    });
    QObject::connect(
        &progress, &QProgressDialog::canceled, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();
  }

  if(out_of_paper) {
    QMessageBox::warning(
        parent,
        QApplication::tr("Out of labels"),
        QApplication::tr(
            "The label roll ran out while printing. Insert a new label "
            "roll; the printer will then continue printing the "
            "remaining labels by itself."));
  }
}

/* EOF */
