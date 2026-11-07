// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Standalone interactive test for LabelLayoutWidget.
/**
   Opens a window containing a LabelLayoutWidget so that label-sheet
   layouts can be browsed, created, and saved independently of the
   main Colman application.

   The program uses the same QSettings organisation and application name
   as Colman ("cpesch" / "colman"), so any layout saved here is
   immediately available in Colman's label-printing dialog.

   Usage:
   @code
   ./colman_label_layout_app
   @endcode

   @file
   @author Christoph Pesch
*/

#include "colman_label_layout_widget.hpp"
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QMAINWINDOW
#  include <QMainWindow>
#  define SYSTEM_QMAINWINDOW
#endif
#ifndef SYSTEM_QSTATUSBAR
#  include <QStatusBar>
#  define SYSTEM_QSTATUSBAR
#endif
#ifndef SYSTEM_QTRANSLATOR
#  include <QTranslator>
#  define SYSTEM_QTRANSLATOR
#endif
#ifndef SYSTEM_STDIO
#  include <cstdio>
#  define SYSTEM_STDIO
#endif

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

  QMainWindow win;
  win.setWindowTitle(
      QObject::tr("Label Layout \xe2\x80\x93 Colman"));

  auto* widget = new colman::LabelLayoutWidget(&win);
  win.setCentralWidget(widget);

  win.statusBar()->showMessage(
      QObject::tr(
          "Select a built-in layout or define your own, then click Apply."));

  QObject::connect(
      widget, &colman::LabelLayoutWidget::layout_applied,
      [&app](colman::LabelLayoutPreset const& p) {
        std::printf("name:         %s\n",
                    p.name.toUtf8().constData());
        std::printf("columns:      %d\n", p.columns);
        std::printf("rows:         %d\n", p.rows);
        std::printf("label_width:  %.2f mm\n", p.label_width);
        std::printf("label_height: %.2f mm\n", p.label_height);
        std::printf("left_margin:  %.2f mm\n", p.left_margin);
        std::printf("top_margin:   %.2f mm\n", p.top_margin);
        std::printf("h_gap:        %.2f mm\n", p.h_gap);
        std::printf("v_gap:        %.2f mm\n", p.v_gap);
        app.quit();
      });

  win.show();
  return app.exec();
}

/* EOF */
