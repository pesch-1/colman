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
#ifndef COLMAN_UI_BUILD_HPP
#  include "colman_ui_build.hpp"
#endif
#ifndef SYSTEM_QABSTRACTITEMVIEW
#  include <QAbstractItemView>
#  define SYSTEM_QABSTRACTITEMVIEW
#endif
#ifndef SYSTEM_QSIZEPOLICY
#  include <QSizePolicy>
#  define SYSTEM_QSIZEPOLICY
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif

void colman::MainWindowUi::build_liquidation_item(QWidget* page)
{
  auto* verticalLayout_78 = new QVBoxLayout(page);
  liquidation_item_back_ = new QPushButton(page);
  verticalLayout_78->addWidget(liquidation_item_back_);
  auto* horizontalLayout_66 = new QHBoxLayout();
  auto* label_184 = make_icon_label(page, ":/icons/owls/remove_book.jpeg");
  horizontalLayout_66->addWidget(label_184);
  auto* verticalLayout_76 = new QVBoxLayout();
  auto* label_185 = new colman::ScalingLabel(page);
  label_185->setFont(bold_font());
  label_185->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_76->addWidget(label_185);
  horizontalLayout_66->addLayout(verticalLayout_76);
  verticalLayout_78->addLayout(horizontalLayout_66);
  auto* splitter_16 = new QSplitter(page);
  set_size_policy(splitter_16, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_16->setOrientation(Qt::Orientation::Horizontal);
  splitter_16->setHandleWidth(5);
  auto* layoutWidget_21 = new QWidget(splitter_16);
  auto* verticalLayout_77 = new QVBoxLayout(layoutWidget_21);
  verticalLayout_77->setContentsMargins(0, 0, 0, 0);
  auto* horizontalLayout_70 = new QHBoxLayout();
  auto* label_15 = new QLabel(layoutWidget_21);
  horizontalLayout_70->addWidget(label_15);
  liquidation_item_search_ = new QLineEdit(layoutWidget_21);
  horizontalLayout_70->addWidget(liquidation_item_search_);
  liquidation_item_camera_ = new QToolButton(layoutWidget_21);
  horizontalLayout_70->addWidget(liquidation_item_camera_);
  verticalLayout_77->addLayout(horizontalLayout_70);
  liquidation_item_items_view_ = new QTableView(layoutWidget_21);
  verticalLayout_77->addWidget(liquidation_item_items_view_);
  splitter_16->addWidget(layoutWidget_21);
  liquidation_item_item_ = new colman::ItemWidget(splitter_16);
  set_size_policy(liquidation_item_item_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_16->addWidget(liquidation_item_item_);
  verticalLayout_78->addWidget(splitter_16);
  auto* horizontalLayout_71 = new QHBoxLayout();
  liquidation_item_item_count_ = new QLabel(page);
  horizontalLayout_71->addWidget(liquidation_item_item_count_);
  liquidation_item_load_more_ = new QPushButton(page);
  liquidation_item_load_more_->setVisible(false);
  horizontalLayout_71->addWidget(liquidation_item_load_more_);
  auto* horizontalSpacer_36 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_71->addItem(horizontalSpacer_36);
  liquidation_item_next_ = new QPushButton(page);
  liquidation_item_next_->setEnabled(false);
  horizontalLayout_71->addWidget(liquidation_item_next_);
  verticalLayout_78->addLayout(horizontalLayout_71);

  liquidation_item_back_->setText(tr("back"));
  label_185->setText(tr("Step 1: Select item to liquidate!"));
  label_15->setText(tr("barcode/name: "));
#if QT_CONFIG(tooltip)
  liquidation_item_camera_->setToolTip(tr("Scan barcode with camera"));
#endif // QT_CONFIG(tooltip)
  liquidation_item_load_more_->setText(tr("Load more..."));
  liquidation_item_next_->setText(tr("next"));
}

void colman::MainWindowUi::build_liquidation_count(QWidget* page)
{
  auto* verticalLayout_82 = new QVBoxLayout(page);
  liquidation_count_back_ = new QPushButton(page);
  verticalLayout_82->addWidget(liquidation_count_back_);
  auto* horizontalLayout_74 = new QHBoxLayout();
  auto* label_188 = make_icon_label(page, ":/icons/owls/remove_book.jpeg");
  horizontalLayout_74->addWidget(label_188);
  auto* verticalLayout_81 = new QVBoxLayout();
  auto* label_189 = new colman::ScalingLabel(page);
  label_189->setFont(bold_font());
  label_189->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_81->addWidget(label_189);
  horizontalLayout_74->addLayout(verticalLayout_81);
  verticalLayout_82->addLayout(horizontalLayout_74);
  auto* verticalSpacer_22 = new QSpacerItem(20, 242, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_82->addItem(verticalSpacer_22);
  auto* horizontalLayout_75 = new QHBoxLayout();
  auto* formLayout_8 = new QFormLayout();
  auto* label_16 = new colman::ScalingLabel(page);
  label_16->setFont(bold_font());
  formLayout_8->setWidget(0, QFormLayout::ItemRole::LabelRole, label_16);
  liquidation_count_item_name_ = new QLabel(page);
  set_size_policy(liquidation_count_item_name_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  formLayout_8->setWidget(0, QFormLayout::ItemRole::FieldRole, liquidation_count_item_name_);
  auto* label_21 = new colman::ScalingLabel(page);
  label_21->setFont(bold_font());
  formLayout_8->setWidget(1, QFormLayout::ItemRole::LabelRole, label_21);
  liquidation_count_available_ = new QLabel(page);
  set_size_policy(liquidation_count_available_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  formLayout_8->setWidget(1, QFormLayout::ItemRole::FieldRole, liquidation_count_available_);
  horizontalLayout_75->addLayout(formLayout_8);
  verticalLayout_82->addLayout(horizontalLayout_75);
  auto* formLayout_9 = new QFormLayout();
  auto* label_22 = new QLabel(page);
  formLayout_9->setWidget(0, QFormLayout::ItemRole::LabelRole, label_22);
  liquidation_count_count_ = new QSpinBox(page);
  liquidation_count_count_->setMinimum(1);
  liquidation_count_count_->setMaximum(999999999);
  formLayout_9->setWidget(0, QFormLayout::ItemRole::FieldRole, liquidation_count_count_);
  label_23 = new QLabel(page);
  formLayout_9->setWidget(1, QFormLayout::ItemRole::LabelRole, label_23);
  liquidation_count_datetime_ = new QDateTimeEdit(page);
  formLayout_9->setWidget(1, QFormLayout::ItemRole::FieldRole, liquidation_count_datetime_);
  verticalLayout_82->addLayout(formLayout_9);
  auto* verticalSpacer_23 = new QSpacerItem(20, 250, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_82->addItem(verticalSpacer_23);
  auto* horizontalLayout_76 = new QHBoxLayout();
  auto* horizontalSpacer_40 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_76->addItem(horizontalSpacer_40);
  liquidation_count_previous_ = new QPushButton(page);
  horizontalLayout_76->addWidget(liquidation_count_previous_);
  liquidation_count_finish_ = new QPushButton(page);
  liquidation_count_finish_->setEnabled(true);
  horizontalLayout_76->addWidget(liquidation_count_finish_);
  verticalLayout_82->addLayout(horizontalLayout_76);

  liquidation_count_back_->setText(tr("back"));
  label_189->setText(tr("Step 2: How many items do you want to liquidate?"));
  label_16->setText(tr("item: "));
  label_21->setText(tr("available:"));
  label_22->setText(tr("how many items have been liquidated? "));
  liquidation_count_count_->setSuffix(QString());
  label_23->setText(tr("date and time of liquidation?"));
  liquidation_count_previous_->setText(tr("previous"));
  liquidation_count_finish_->setText(tr("finish"));
}

void colman::MainWindowUi::build_liquidation_done(QWidget* page)
{
  QFont font1;
  auto* verticalLayout_83 = new QVBoxLayout(page);
  auto* verticalSpacer_56 = new QSpacerItem(20, 285, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_83->addItem(verticalSpacer_56);
  auto* horizontalLayout_77 = new QHBoxLayout();
  auto* horizontalSpacer_41 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_77->addItem(horizontalSpacer_41);
  auto* label_190 = make_icon_label(page, ":/icons/owls/remove_book.jpeg");
  horizontalLayout_77->addWidget(label_190);
  auto* horizontalSpacer_42 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_77->addItem(horizontalSpacer_42);
  verticalLayout_83->addLayout(horizontalLayout_77);
  auto* horizontalLayout_78 = new QHBoxLayout();
  auto* horizontalSpacer_43 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_78->addItem(horizontalSpacer_43);
  auto* lending_done_question_3 = new QLabel(page);
  lending_done_question_3->setFont(font1);
  lending_done_question_3->setAlignment(Qt::AlignmentFlag::AlignCenter);
  horizontalLayout_78->addWidget(lending_done_question_3);
  auto* horizontalSpacer_44 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_78->addItem(horizontalSpacer_44);
  verticalLayout_83->addLayout(horizontalLayout_78);
  auto* horizontalLayout_145 = new QHBoxLayout();
  auto* horizontalSpacer_58 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_145->addItem(horizontalSpacer_58);
  liquidation_done_yes_ = new QPushButton(page);
  horizontalLayout_145->addWidget(liquidation_done_yes_);
  liquidation_done_no_ = new QPushButton(page);
  horizontalLayout_145->addWidget(liquidation_done_no_);
  auto* horizontalSpacer_59 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_145->addItem(horizontalSpacer_59);
  verticalLayout_83->addLayout(horizontalLayout_145);
  auto* verticalSpacer_57 = new QSpacerItem(20, 285, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_83->addItem(verticalSpacer_57);

  lending_done_question_3->setText(tr("Further liquidations?"));
  liquidation_done_yes_->setText(tr("yes"));
  liquidation_done_no_->setText(tr("no"));
}

/* EOF */
