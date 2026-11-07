// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of struct ItemWidgetUi.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item_widget_ui.hpp"
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QPLAINTEXTEDIT
#  include <QPlainTextEdit>
#  define SYSTEM_QPLAINTEXTEDIT
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif

/**
   Constructs all fixed form widgets and lays them out: the item-type
   combo box on top, followed by a scroll area whose grid holds the
   common fields.  Wires the school-item checkbox so toggling it shows
   or hides the subject, academic-year and school-type rows.
   @param parent the ItemWidget hosting the form.
*/
void colman::ItemWidgetUi::setup_ui(QWidget* parent)
{
  parent->setWindowTitle(tr("Item"));

  auto* outer_layout = new QVBoxLayout(parent);

  item_type_box_ = new QComboBox(parent);
  outer_layout->addWidget(item_type_box_);

  auto* scroll_area = new QScrollArea(parent);
  scroll_area->setWidgetResizable(true);

  item_widget_ = new QWidget;
  item_grid_ = new QGridLayout(item_widget_);

  // Row 0: name/title.
  label_21_ = new QLabel(tr("name/title:"), item_widget_);
  name_ = new QLineEdit(item_widget_);
  name_->setObjectName("item_name_");
  name_->setClearButtonEnabled(false);
  item_grid_->addWidget(label_21_, 0, 0);
  item_grid_->addWidget(name_, 0, 1);

  // Row 1: thumbnail with action buttons.
  label_22_ = new QLabel(tr("thumbnail:"), item_widget_);
  thumbnail_ = new QLabel(item_widget_);
  thumbnail_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  thumbnail_->setScaledContents(false);
  thumbnail_layout_ = new QHBoxLayout;
  thumbnail_layout_->addWidget(thumbnail_);
  auto* button_column = new QVBoxLayout;
  browse_thumbnail_ = new QPushButton(tr("browse"), item_widget_);
  browse_thumbnail_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  scan_thumbnail_ = new QPushButton(tr("scan"), item_widget_);
  scan_thumbnail_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  amazon_thumbnail_ = new QPushButton(tr("Amazon"), item_widget_);
  amazon_thumbnail_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  zoom_thumbnail_ = new QPushButton(tr("zoom"), item_widget_);
  zoom_thumbnail_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  zoom_thumbnail_->setEnabled(false);
  remove_thumbnail_ = new QPushButton(tr("remove"), item_widget_);
  remove_thumbnail_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  remove_thumbnail_->setEnabled(false);
  button_column->addWidget(browse_thumbnail_);
  button_column->addWidget(scan_thumbnail_);
  button_column->addWidget(amazon_thumbnail_);
  button_column->addWidget(zoom_thumbnail_);
  button_column->addWidget(remove_thumbnail_);
  thumbnail_layout_->addLayout(button_column);
  item_grid_->addWidget(label_22_, 1, 0);
  item_grid_->addLayout(thumbnail_layout_, 1, 1);

  // Row 2: school-item toggle.
  is_schoolitem_ = new QCheckBox(tr("is school item"), item_widget_);
  is_schoolitem_->setChecked(true);
  item_grid_->addWidget(is_schoolitem_, 2, 1);

  // Rows 3-5: school-item sub-fields.
  label_100_ = new QLabel(tr("subject:"), item_widget_);
  subject_ = new QLineEdit(item_widget_);
  item_grid_->addWidget(label_100_, 3, 0);
  item_grid_->addWidget(subject_, 3, 1);
  label_101_ = new QLabel(tr("academic year:"), item_widget_);
  academic_year_ = new QLineEdit(item_widget_);
  academic_year_->setPlaceholderText(tr("e.g. 5-7, 9"));
  QString const academic_year_hint =
      tr("Grade levels or ranges, comma-separated, e.g. \"5-7, 9\". "
         "Leave empty if no grade level applies.");
  academic_year_->setToolTip(academic_year_hint);
  label_101_->setToolTip(academic_year_hint);
  item_grid_->addWidget(label_101_, 4, 0);
  item_grid_->addWidget(academic_year_, 4, 1);
  label_102_ = new QLabel(tr("type:", "schoolitem"), item_widget_);
  school_type_ = new QLineEdit(item_widget_);
  item_grid_->addWidget(label_102_, 5, 0);
  item_grid_->addWidget(school_type_, 5, 1);

  // Row 6: place.
  label_23_ = new QLabel(tr("place:"), item_widget_);
  place_ = new QLineEdit(item_widget_);
  item_grid_->addWidget(label_23_, 6, 0);
  item_grid_->addWidget(place_, 6, 1);

  // Rows 7-8: signatures.
  label_sig_top_ = new QLabel(tr("signature top:"), item_widget_);
  signature_top_ = new QComboBox(item_widget_);
  signature_top_->setEditable(true);
  item_grid_->addWidget(label_sig_top_, 7, 0);
  item_grid_->addWidget(signature_top_, 7, 1);
  label_sig_bottom_ = new QLabel(tr("signature bottom:"), item_widget_);
  signature_bottom_ = new QComboBox(item_widget_);
  signature_bottom_->setEditable(true);
  item_grid_->addWidget(label_sig_bottom_, 8, 0);
  item_grid_->addWidget(signature_bottom_, 8, 1);

  // Row 9: description.
  label_24_ = new QLabel(tr("description:"), item_widget_);
  description_ = new QPlainTextEdit(item_widget_);
  item_grid_->addWidget(label_24_, 9, 0);
  item_grid_->addWidget(description_, 9, 1);

  // Row 10: keywords (placeholder; ItemWidget swaps in a list editor).
  // The placeholder is tracked as a member so remove_common_fields() can
  // detach it before the dynamic rebuild; otherwise it would linger at this
  // grid cell and collide (overlap) with whatever field the dynamic layout
  // later places here.
  label_keywords_ = new QLabel(tr("keywords:"), item_widget_);
  keywords_placeholder_ = new QLineEdit(item_widget_);
  item_grid_->addWidget(label_keywords_, 10, 0);
  item_grid_->addWidget(keywords_placeholder_, 10, 1);

  // Row 11: barcode.
  label_barcode_ = new QLabel(tr("Colman barcode:"), item_widget_);
  barcode_display_ = new QLineEdit(item_widget_);
  barcode_display_->setReadOnly(true);
  item_grid_->addWidget(label_barcode_, 11, 0);
  item_grid_->addWidget(barcode_display_, 11, 1);

  scroll_area->setWidget(item_widget_);
  outer_layout->addWidget(scroll_area);

  // School-item fields follow the checkbox state.
  QObject::connect(is_schoolitem_, &QCheckBox::toggled,
                   subject_, &QLineEdit::setVisible);
  QObject::connect(is_schoolitem_, &QCheckBox::toggled,
                   academic_year_, &QLineEdit::setVisible);
  QObject::connect(is_schoolitem_, &QCheckBox::toggled,
                   school_type_, &QLineEdit::setVisible);
  QObject::connect(is_schoolitem_, &QCheckBox::toggled,
                   label_100_, &QLabel::setVisible);
  QObject::connect(is_schoolitem_, &QCheckBox::toggled,
                   label_101_, &QLabel::setVisible);
  QObject::connect(is_schoolitem_, &QCheckBox::toggled,
                   label_102_, &QLabel::setVisible);
}

/* EOF */
