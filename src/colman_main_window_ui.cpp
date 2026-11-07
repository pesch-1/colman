// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Definition of struct MainWindowUi.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_MAIN_WINDOW_UI_HPP
#  include "colman_main_window_ui.hpp"
#endif
#ifndef SYSTEM_QMAINWINDOW
#  include <QMainWindow>
#  define SYSTEM_QMAINWINDOW
#endif
#ifndef SYSTEM_QWIDGET
#  include <QWidget>
#  define SYSTEM_QWIDGET
#endif
#ifndef SYSTEM_QACTION
#  include <QAction>
#  define SYSTEM_QACTION
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QSTATUSBAR
#  include <QStatusBar>
#  define SYSTEM_QSTATUSBAR
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QRADIOBUTTON
#  include <QRadioButton>
#  define SYSTEM_QRADIOBUTTON
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QPLAINTEXTEDIT
#  include <QPlainTextEdit>
#  define SYSTEM_QPLAINTEXTEDIT
#endif
#ifndef SYSTEM_QTEXTEDIT
#  include <QTextEdit>
#  define SYSTEM_QTEXTEDIT
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QDATEEDIT
#  include <QDateEdit>
#  define SYSTEM_QDATEEDIT
#endif
#ifndef SYSTEM_QDATETIMEEDIT
#  include <QDateTimeEdit>
#  define SYSTEM_QDATETIMEEDIT
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QTREEWIDGET
#  include <QTreeWidget>
#  define SYSTEM_QTREEWIDGET
#endif
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QSPLITTER
#  include <QSplitter>
#  define SYSTEM_QSPLITTER
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QABSTRACTITEMVIEW
#  include <QAbstractItemView>
#  define SYSTEM_QABSTRACTITEMVIEW
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QSIZEPOLICY
#  include <QSizePolicy>
#  define SYSTEM_QSIZEPOLICY
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef COLMAN_ITEM_WIDGET_HPP
#  include "colman_item_widget.hpp"
#endif
#ifndef COLMAN_PERSON_WIDGET_HPP
#  include "colman_person_widget.hpp"
#endif
#ifndef COLMAN_PERSON_LIST_WIDGET_HPP
#  include "colman_person_list_widget.hpp"
#endif
#ifndef COLMAN_PIXMAP_LABEL_HPP
#  include "colman_pixmap_label.hpp"
#endif
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif

void colman::MainWindowUi::setup_ui(QMainWindow& window)
{
  window.resize(880, 844);
  window.setWindowTitle(tr("Collection Manager"));
  window.setAutoFillBackground(true);
  window.setUnifiedTitleAndToolBarOnMac(false);

  // Actions.  Only Open/New/Close are used by the code base; the former
  // colman.ui also declared save_as/exit/preferences/settings/undo/redo and a
  // Collection_Settings action, but those were never referenced and are
  // dropped together with their dead action_exit_ -> close() connection.
  action_open_ = new QAction(&window);
  action_open_->setText(tr("Open"));
  action_new_ = new QAction(&window);
  action_new_->setText(tr("New"));
  action_close_ = new QAction(&window);
  action_close_->setText(tr("Close"));

  centralwidget = new QWidget(&window);
  auto* central_layout = new QVBoxLayout(centralwidget);
  pages_stack_ = new QStackedWidget(centralwidget);
  central_layout->addWidget(pages_stack_);
  window.setCentralWidget(centralwidget);

  // The page stack is flat: every all page is a direct
  // child, added here in the original colman.ui order.  Page contents are
  // built by the build_*() helpers; pages still empty here are filled in
  // follow-up commits.
  start_page_ = new QWidget;
  central_page_ = new QWidget;
  item_type_page_ = new QWidget;
  person_page_ = new QWidget;
  person_barcode_page_ = new QWidget;
  item_barcode_page_ = new QWidget;
  person_view_page_ = new QWidget;
  history_page_ = new QWidget;
  settings_page_ = new QWidget;
  preferences_page_ = new QWidget;
  lending_person_ = new QWidget;
  lending_item_ = new QWidget;
  lending_details_ = new QWidget;
  lending_confirm_ = new QWidget;
  lending_done_ = new QWidget;
  return_item_ = new QWidget;
  return_borrower_ = new QWidget;
  return_lending_ = new QWidget;
  return_details_ = new QWidget;
  return_confirm_ = new QWidget;
  return_done_ = new QWidget;
  acquisition_select_ = new QWidget;
  acquisition_count_ = new QWidget;
  acquisition_done_ = new QWidget;
  liquidation_item_ = new QWidget;
  liquidation_count_ = new QWidget;
  liquidation_done_ = new QWidget;
  group_lending_item_ = new QWidget;
  group_lending_persons_ = new QWidget;
  group_lending_details_ = new QWidget;
  group_lending_confirm_ = new QWidget;
  group_lending_done_ = new QWidget;

  pages_stack_->addWidget(start_page_);
  pages_stack_->addWidget(central_page_);
  pages_stack_->addWidget(item_type_page_);
  pages_stack_->addWidget(person_page_);
  pages_stack_->addWidget(person_barcode_page_);
  pages_stack_->addWidget(item_barcode_page_);
  pages_stack_->addWidget(person_view_page_);
  pages_stack_->addWidget(history_page_);
  pages_stack_->addWidget(settings_page_);
  pages_stack_->addWidget(preferences_page_);
  pages_stack_->addWidget(lending_person_);
  pages_stack_->addWidget(lending_item_);
  pages_stack_->addWidget(lending_details_);
  pages_stack_->addWidget(lending_confirm_);
  pages_stack_->addWidget(lending_done_);
  pages_stack_->addWidget(return_item_);
  pages_stack_->addWidget(return_borrower_);
  pages_stack_->addWidget(return_lending_);
  pages_stack_->addWidget(return_details_);
  pages_stack_->addWidget(return_confirm_);
  pages_stack_->addWidget(return_done_);
  pages_stack_->addWidget(acquisition_select_);
  pages_stack_->addWidget(acquisition_count_);
  pages_stack_->addWidget(acquisition_done_);
  pages_stack_->addWidget(liquidation_item_);
  pages_stack_->addWidget(liquidation_count_);
  pages_stack_->addWidget(liquidation_done_);
  pages_stack_->addWidget(group_lending_item_);
  pages_stack_->addWidget(group_lending_persons_);
  pages_stack_->addWidget(group_lending_details_);
  pages_stack_->addWidget(group_lending_confirm_);
  pages_stack_->addWidget(group_lending_done_);

  // Populate each page.
  build_item_type_page(item_type_page_);
  build_person_page(person_page_);
  build_person_barcode_page(person_barcode_page_);
  build_item_barcode_page(item_barcode_page_);
  build_person_view_page(person_view_page_);
  build_history_page(history_page_);
  build_settings_page(settings_page_);
  build_preferences_page(preferences_page_);
  build_lending_person(lending_person_);
  build_lending_item(lending_item_);
  build_lending_details(lending_details_);
  build_lending_confirm(lending_confirm_);
  build_lending_done(lending_done_);
  build_return_item(return_item_);
  build_return_borrower(return_borrower_);
  build_return_lending(return_lending_);
  build_return_details(return_details_);
  build_return_confirm(return_confirm_);
  build_return_done(return_done_);
  build_acquisition_select(acquisition_select_);
  build_acquisition_count(acquisition_count_);
  build_acquisition_done(acquisition_done_);
  build_liquidation_item(liquidation_item_);
  build_liquidation_count(liquidation_count_);
  build_liquidation_done(liquidation_done_);
  build_group_lending_item(group_lending_item_);
  build_group_lending_persons(group_lending_persons_);
  build_group_lending_details(group_lending_details_);
  build_group_lending_confirm(group_lending_confirm_);
  build_group_lending_done(group_lending_done_);

  auto* statusbar = new QStatusBar(&window);
  window.setStatusBar(statusbar);

  pages_stack_->setCurrentIndex(0);
}

/* EOF */
