// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Standalone interactive test for PrinterCalibrationWidget.
/**
   Opens a window containing a PrinterCalibrationWidget so that the
   print-position calibration for any installed printer can be verified
   and saved independently of the main Colman application.

   The program uses the same QSettings organisation and application name
   as Colman ("cpesch" / "colman"), so any offset that is saved here is
   immediately effective when Colman prints barcode labels.

   Usage:
   @code
   ./colman_calibrate_printer
   @endcode

   @file
   @author Christoph Pesch
*/

#include "colman_printer_calibration_widget.hpp"
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QMAINWINDOW
#  include <QMainWindow>
#  define SYSTEM_QMAINWINDOW
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSTATUSBAR
#  include <QStatusBar>
#  define SYSTEM_QSTATUSBAR
#endif
#ifndef SYSTEM_QTRANSLATOR
#  include <QTranslator>
#  define SYSTEM_QTRANSLATOR
#endif

/// Fills @p layout from QSettings, applying Colman's saved label layout.
/**
   Uses the same keys as colman_item_barcode_page.cpp so that the
   calibration sheet matches the actual label sheet used in Colman.
   Falls back to PageLayout defaults when no settings are present.
   @param layout  Layout object to populate.
*/
static void load_layout(colman::PageLayout& layout)
{
  QSettings s;
  layout.set_page_width(
      s.value("page_width", layout.get_page_width()).toDouble());
  layout.set_page_height(
      s.value("page_height", layout.get_page_height()).toDouble());
  layout.set_printer_top_margin(
      s.value("printer_top_margin", layout.get_printer_top_margin()).toDouble());
  layout.set_printer_bottom_margin(
      s.value("printer_bottom_margin", layout.get_printer_bottom_margin()).toDouble());
  layout.set_printer_left_margin(
      s.value("printer_left_margin", layout.get_printer_left_margin()).toDouble());
  layout.set_printer_right_margin(
      s.value("printer_right_margin", layout.get_printer_right_margin()).toDouble());
  layout.set_rows(
      s.value("label_rows", layout.get_rows()).toInt());
  layout.set_columns(
      s.value("label_columns", layout.get_columns()).toInt());
  layout.set_label_width(
      s.value("label_width", layout.get_label_width()).toDouble());
  layout.set_label_height(
      s.value("label_height", layout.get_label_height()).toDouble());
  layout.set_label_distance_x(
      s.value("label_distance_x", layout.get_label_distance_x()).toDouble());
  layout.set_label_distance_y(
      s.value("label_distance_y", layout.get_label_distance_y()).toDouble());
}

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  // Same identity as Colman so QSettings are shared.
  QApplication::setOrganizationName("cpesch");
  QApplication::setApplicationName("colman");

  // Load the same translations as the main Colman application.
  QTranslator translator;
  if(translator.load(QLocale::system(), "colman", "_",
                     ":/translations", ".qm")) {
    QApplication::installTranslator(&translator);
  }

  colman::PageLayout layout;
  load_layout(layout);

  QMainWindow win;
  win.setWindowTitle(QObject::tr("Printer Calibration \xe2\x80\x93 Colman"));
  win.resize(720, 820);

  auto* widget = new colman::PrinterCalibrationWidget(layout, &win);
  win.setCentralWidget(widget);

  win.statusBar()->showMessage(
      QObject::tr(
          "Select printer, print calibration sheet, measure offset and "
          "save."));

  QObject::connect(
      widget, &colman::PrinterCalibrationWidget::calibration_saved,
      [&win]() {
        win.statusBar()->showMessage(
            QObject::tr("Calibration saved."), 4000);
      });

  win.show();
  return app.exec();
}

/* EOF */
