// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Standalone interactive test for LabelSelectionWidget.
/**
   Opens a window containing a LabelSelectionWidget so that label
   selection can be tested without the main Colman application.

   The first command-line argument (optional, default: 10) sets the
   maximum number of labels that may be selected.

   Click <b>Confirm</b> to print all selected labels to stdout and exit.
   The output lists each label on one line:

   @code
   col=0 row=0  x=7.75 y=21.50  w=48.50 h=25.40 mm
   col=1 row=0  x=56.25 y=21.50  w=48.50 h=25.40 mm
   @endcode

   The program uses the same QSettings organisation and application name
   as Colman ("cpesch" / "colman"), so any user-defined layout saved
   here is immediately available in Colman's label-printing dialog.

   Usage:
   @code
   ./colman_label_selection_app [max_count]
   @endcode

   @file
   @author Christoph Pesch
*/

#include "colman_label_selection_widget.hpp"
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
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
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
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

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QObject::tr("Interactive label-selection test for Colman."));
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption count_opt(
      "count",
      QObject::tr("Maximum number of selectable labels (default: 10)."),
      "N", "10");
  parser.addOption(count_opt);
  parser.process(app);

  int const max_count = parser.value(count_opt).toInt();

  QMainWindow win;
  win.setWindowTitle(
      QObject::tr("Label Selection \xe2\x80\x93 Colman"));

  // --- central area: selection widget + confirm button ---
  auto* container = new QWidget(&win);
  auto* vbox = new QVBoxLayout(container);

  auto* sel = new colman::LabelSelectionWidget(max_count, container);
  vbox->addWidget(sel, 1);

  auto* bb = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, container);
  bb->button(QDialogButtonBox::Ok)->setText(QObject::tr("Confirm"));
  vbox->addWidget(bb);

  win.setCentralWidget(container);
  win.statusBar()->showMessage(
      QObject::tr(
          "Select up to %1 labels, then click Confirm.")
          .arg(max_count));

  // --- confirm: print selection to stdout and quit ---
  QObject::connect(
      bb->button(QDialogButtonBox::Ok), &QPushButton::clicked,
      [&app, sel]() {
        QList<colman::SelectedLabel> const labels = sel->selected_labels();
        std::printf("%d label(s) selected:\n",
                    static_cast<int>(labels.size()));
        for(colman::SelectedLabel const& s : labels) {
          std::printf(
              "  col=%d row=%d  x=%.2f y=%.2f  w=%.2f h=%.2f mm\n",
              s.col, s.row, s.x_mm, s.y_mm, s.width_mm, s.height_mm);
        }
        app.quit();
      });

  QObject::connect(
      bb->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
      &app, &QApplication::quit);

  win.resize(600, 750);
  win.show();
  return app.exec();
}

/* EOF */
