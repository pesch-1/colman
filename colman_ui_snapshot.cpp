// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Definition of render_ui_snapshots().
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_UI_SNAPSHOT_HPP
#  include "colman_ui_snapshot.hpp"
#endif
#ifndef COLMAN_MAIN_WINDOW_UI_HPP
#  include "colman_main_window_ui.hpp"
#endif
#ifndef SYSTEM_QMAINWINDOW
#  include <QMainWindow>
#  define SYSTEM_QMAINWINDOW
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif

int colman::render_ui_snapshots(QString const& dir)
{
  QDir().mkpath(dir);
  colman::MainWindowUi ui;
  QMainWindow window;
  ui.setup_ui(window);
  window.resize(1200, 824);
  window.show();
  QApplication::processEvents();
  QStackedWidget* stack = ui.pages_stack_;
  if(!stack) {
    return -1;
  }
  int const count = stack->count();
  for(int i = 0; i < count; ++i) {
    stack->setCurrentIndex(i);
    QApplication::processEvents();
    QWidget* page = stack->widget(i);
    QPixmap const pixmap = page->grab();
    QString const name =
        QString("%1/page_%2.png").arg(dir).arg(i, 2, 10, QChar('0'));
    pixmap.save(name);
  }
  return count;
}

/* EOF */
