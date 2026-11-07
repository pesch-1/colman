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

void colman::MainWindowUi::build_lending_person(QWidget* page)
{
  auto* verticalLayout_22 = new QVBoxLayout(page);
  lending_person_back_ = new QPushButton(page);
  verticalLayout_22->addWidget(lending_person_back_);
  auto* horizontalLayout_46 = new QHBoxLayout();
  auto* label_153 = make_icon_label(page, ":/icons/owls/lending.jpeg");
  horizontalLayout_46->addWidget(label_153);
  auto* verticalLayout_43 = new QVBoxLayout();
  auto* label_149 = new colman::ScalingLabel(page);
  label_149->setFont(bold_font());
  label_149->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_43->addWidget(label_149);
  horizontalLayout_46->addLayout(verticalLayout_43);
  verticalLayout_22->addLayout(horizontalLayout_46);
  auto* splitter_10 = new QSplitter(page);
  set_size_policy(splitter_10, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_10->setOrientation(Qt::Orientation::Horizontal);
  lending_person_person_list_ = new colman::PersonListWidget(splitter_10);
  set_size_policy(lending_person_person_list_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_10->addWidget(lending_person_person_list_);
  lending_person_person_ = new colman::PersonWidget(splitter_10);
  set_size_policy(lending_person_person_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_10->addWidget(lending_person_person_);
  verticalLayout_22->addWidget(splitter_10);
  auto* horizontalLayout_44 = new QHBoxLayout();
  lending_person_extend_ = new QPushButton(page);
  lending_person_extend_->setEnabled(false);
  horizontalLayout_44->addWidget(lending_person_extend_);
  auto* horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_44->addItem(horizontalSpacer_14);
  lending_person_next_ = new QPushButton(page);
  horizontalLayout_44->addWidget(lending_person_next_);
  verticalLayout_22->addLayout(horizontalLayout_44);

  lending_person_back_->setText(tr("back"));
  label_149->setText(tr("Step 1: Please select lending person!"));
  lending_person_extend_->setText(tr("extend due date..."));
  lending_person_next_->setText(tr("next"));
}

void colman::MainWindowUi::build_lending_item(QWidget* page)
{
  auto* verticalLayout_54 = new QVBoxLayout(page);
  lending_item_back_ = new QPushButton(page);
  verticalLayout_54->addWidget(lending_item_back_);
  auto* horizontalLayout_47 = new QHBoxLayout();
  auto* label_154 = make_icon_label(page, ":/icons/owls/lending.jpeg");
  horizontalLayout_47->addWidget(label_154);
  auto* verticalLayout_53 = new QVBoxLayout();
  label_472 = new colman::ScalingLabel(page);
  label_472->setFont(bold_font());
  verticalLayout_53->addWidget(label_472);
  auto* horizontalLayout_134 = new QHBoxLayout();
  auto* label_473 = new QLabel(page);
  horizontalLayout_134->addWidget(label_473);
  lending_item_barcode_ = new QLineEdit(page);
  horizontalLayout_134->addWidget(lending_item_barcode_);
  lending_item_camera_ = new QToolButton(page);
  horizontalLayout_134->addWidget(lending_item_camera_);
  verticalLayout_53->addLayout(horizontalLayout_134);
  horizontalLayout_47->addLayout(verticalLayout_53);
  verticalLayout_54->addLayout(horizontalLayout_47);
  auto* splitter_52 = new QSplitter(page);
  set_size_policy(splitter_52, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_52->setOrientation(Qt::Orientation::Horizontal);
  splitter_52->setHandleWidth(5);
  lending_item_item_view_ = new QTableView(splitter_52);
  lending_item_item_view_->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  lending_item_item_view_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  splitter_52->addWidget(lending_item_item_view_);
  lending_item_item_widget_ = new colman::ItemWidget(splitter_52);
  set_size_policy(lending_item_item_widget_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_52->addWidget(lending_item_item_widget_);
  verticalLayout_54->addWidget(splitter_52);
  auto* horizontalLayout_135 = new QHBoxLayout();
  lending_item_item_count_ = new QLabel(page);
  horizontalLayout_135->addWidget(lending_item_item_count_);
  lending_item_load_more_ = new QPushButton(page);
  lending_item_load_more_->setVisible(false);
  horizontalLayout_135->addWidget(lending_item_load_more_);
  auto* horizontalSpacer_47 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_135->addItem(horizontalSpacer_47);
  lending_item_next_ = new QPushButton(page);
  horizontalLayout_135->addWidget(lending_item_next_);
  verticalLayout_54->addLayout(horizontalLayout_135);

  lending_item_back_->setText(tr("back"));
  label_472->setText(tr("Step 2: Please select item to lend!"));
  label_473->setText(tr("barcode/name:"));
#if QT_CONFIG(tooltip)
  lending_item_camera_->setToolTip(tr("Scan barcode with camera"));
#endif // QT_CONFIG(tooltip)
  lending_item_load_more_->setText(tr("Load more..."));
  lending_item_next_->setText(tr("next"));

  label_473->setBuddy(lending_item_barcode_);
}

void colman::MainWindowUi::build_lending_details(QWidget* page)
{
  auto* verticalLayout_55 = new QVBoxLayout(page);
  lending_details_back_ = new QPushButton(page);
  verticalLayout_55->addWidget(lending_details_back_);
  auto* horizontalLayout_48 = new QHBoxLayout();
  auto* label_155 = make_icon_label(page, ":/icons/owls/lending.jpeg");
  horizontalLayout_48->addWidget(label_155);
  auto* verticalLayout_44 = new QVBoxLayout();
  label_494 = new colman::ScalingLabel(page);
  label_494->setFont(bold_font());
  verticalLayout_44->addWidget(label_494);
  auto* formLayout_16 = new QFormLayout();
  formLayout_16->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);
  auto* label_495 = new QLabel(page);
  formLayout_16->setWidget(0, QFormLayout::ItemRole::LabelRole, label_495);
  lending_details_person_name_ = new QLineEdit(page);
  lending_details_person_name_->setReadOnly(true);
  formLayout_16->setWidget(0, QFormLayout::ItemRole::FieldRole, lending_details_person_name_);
  auto* label_496 = new QLabel(page);
  formLayout_16->setWidget(1, QFormLayout::ItemRole::LabelRole, label_496);
  lending_details_item_name_ = new QLineEdit(page);
  lending_details_item_name_->setReadOnly(true);
  formLayout_16->setWidget(1, QFormLayout::ItemRole::FieldRole, lending_details_item_name_);
  verticalLayout_44->addLayout(formLayout_16);
  horizontalLayout_48->addLayout(verticalLayout_44);
  verticalLayout_55->addLayout(horizontalLayout_48);
  lending_details_available_ = new QLabel(page);
  verticalLayout_55->addWidget(lending_details_available_);
  auto* lending_details_action_layout_ = new QHBoxLayout();
  lending_details_lend_radio_ = new QRadioButton(page);
  lending_details_lend_radio_->setChecked(true);
  lending_details_action_layout_->addWidget(lending_details_lend_radio_);
  lending_details_reserve_radio_ = new QRadioButton(page);
  lending_details_action_layout_->addWidget(lending_details_reserve_radio_);
  auto* lending_details_action_spacer_ = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  lending_details_action_layout_->addItem(lending_details_action_spacer_);
  verticalLayout_55->addLayout(lending_details_action_layout_);
  auto* horizontalLayout_45 = new QHBoxLayout();
  lending_details_count_ = new QSpinBox(page);
  horizontalLayout_45->addWidget(lending_details_count_);
  label_150 = new QLabel(page);
  horizontalLayout_45->addWidget(label_150);
  lending_details_datetime_ = new QDateTimeEdit(page);
  horizontalLayout_45->addWidget(lending_details_datetime_);
  auto* horizontalSpacer_15 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_45->addItem(horizontalSpacer_15);
  verticalLayout_55->addLayout(horizontalLayout_45);
  lending_details_duedate_row_ = new QWidget(page);
  auto* horizontalLayout_136 = new QHBoxLayout(lending_details_duedate_row_);
  horizontalLayout_136->setContentsMargins(0, 0, 0, 0);
  horizontalLayout_136->setContentsMargins(0, 0, 0, 0);
  lending_details_due_date_check_ = new QCheckBox(lending_details_duedate_row_);
  horizontalLayout_136->addWidget(lending_details_due_date_check_);
  lending_details_due_date_ = new QDateEdit(lending_details_duedate_row_);
  lending_details_due_date_->setEnabled(false);
  horizontalLayout_136->addWidget(lending_details_due_date_);
  auto* horizontalSpacer_16 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_136->addItem(horizontalSpacer_16);
  verticalLayout_55->addWidget(lending_details_duedate_row_);
  lending_details_expires_row_ = new QWidget(page);
  lending_details_expires_row_->setVisible(false);
  auto* lending_details_expires_layout_ = new QHBoxLayout(lending_details_expires_row_);
  lending_details_expires_layout_->setContentsMargins(0, 0, 0, 0);
  lending_details_expires_layout_->setContentsMargins(0, 0, 0, 0);
  auto* lending_details_expires_label_ = new QLabel(lending_details_expires_row_);
  lending_details_expires_layout_->addWidget(lending_details_expires_label_);
  lending_details_expires_date_ = new QDateEdit(lending_details_expires_row_);
  lending_details_expires_date_->setCalendarPopup(true);
  lending_details_expires_layout_->addWidget(lending_details_expires_date_);
  auto* lending_details_expires_spacer_ = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  lending_details_expires_layout_->addItem(lending_details_expires_spacer_);
  verticalLayout_55->addWidget(lending_details_expires_row_);
  auto* verticalSpacer_16 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_55->addItem(verticalSpacer_16);
  label_151 = new QLabel(page);
  verticalLayout_55->addWidget(label_151);
  lending_details_remarks_ = new QPlainTextEdit(page);
  set_size_policy(lending_details_remarks_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding, 0, 2);
  verticalLayout_55->addWidget(lending_details_remarks_);
  auto* horizontalLayout_137 = new QHBoxLayout();
  auto* horizontalSpacer_48 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_137->addItem(horizontalSpacer_48);
  lending_details_next_ = new QPushButton(page);
  horizontalLayout_137->addWidget(lending_details_next_);
  verticalLayout_55->addLayout(horizontalLayout_137);

  lending_details_back_->setText(tr("back"));
  label_494->setText(tr("Step 3: Please select number of items to lend and optionally add comments!"));
  label_495->setText(tr("Person:"));
  label_496->setText(tr("Item:"));
  lending_details_available_->setText(tr("No items are available."));
  lending_details_lend_radio_->setText(tr("Lend"));
  lending_details_reserve_radio_->setText(tr("Reserve"));
  lending_details_count_->setSuffix(tr(" item(s)", "lend count"));
  lending_details_count_->setPrefix(tr("lend ", "lend count"));
  label_150->setText(tr(" on ", "date and time"));
  lending_details_datetime_->setDisplayFormat(tr("yyyy-MM-dd HH:mm:ss"));
  lending_details_due_date_check_->setText(tr("with duedate"));
  lending_details_due_date_->setDisplayFormat(tr("yyyy-MM-dd"));
  lending_details_expires_label_->setText(tr("Expires:"));
  lending_details_expires_date_->setDisplayFormat(tr("yyyy-MM-dd"));
  label_151->setText(tr("Do you have some remarks for this lending?"));
  lending_details_next_->setText(tr("next"));

  QObject::connect(lending_details_due_date_check_, &QCheckBox::toggled, lending_details_due_date_, &QDateEdit::setEnabled);
}

void colman::MainWindowUi::build_lending_confirm(QWidget* page)
{
  lending_details_available_->raise();
  label_151->raise();
  lending_details_remarks_->raise();
  auto* verticalLayout_56 = new QVBoxLayout(page);
  lending_confirm_back_ = new QPushButton(page);
  verticalLayout_56->addWidget(lending_confirm_back_);
  auto* horizontalLayout_49 = new QHBoxLayout();
  auto* label_156 = make_icon_label(page, ":/icons/owls/lending.jpeg");
  horizontalLayout_49->addWidget(label_156);
  auto* label_497 = new colman::ScalingLabel(page);
  label_497->setFont(bold_font());
  horizontalLayout_49->addWidget(label_497);
  verticalLayout_56->addLayout(horizontalLayout_49);
  auto* formLayout_17 = new QFormLayout();
  auto* label_498 = new QLabel(page);
  formLayout_17->setWidget(0, QFormLayout::ItemRole::LabelRole, label_498);
  lending_confirm_person_ = new QLineEdit(page);
  lending_confirm_person_->setReadOnly(true);
  formLayout_17->setWidget(0, QFormLayout::ItemRole::FieldRole, lending_confirm_person_);
  auto* label_499 = new QLabel(page);
  formLayout_17->setWidget(1, QFormLayout::ItemRole::LabelRole, label_499);
  lending_confirm_item_ = new QLineEdit(page);
  lending_confirm_item_->setReadOnly(true);
  formLayout_17->setWidget(1, QFormLayout::ItemRole::FieldRole, lending_confirm_item_);
  auto* label_500 = new QLabel(page);
  formLayout_17->setWidget(2, QFormLayout::ItemRole::LabelRole, label_500);
  lending_confirm_number_ = new QLineEdit(page);
  lending_confirm_number_->setReadOnly(true);
  formLayout_17->setWidget(2, QFormLayout::ItemRole::FieldRole, lending_confirm_number_);
  auto* label_501 = new QLabel(page);
  formLayout_17->setWidget(3, QFormLayout::ItemRole::LabelRole, label_501);
  lending_confirm_due_date_ = new QLineEdit(page);
  lending_confirm_due_date_->setReadOnly(true);
  formLayout_17->setWidget(3, QFormLayout::ItemRole::FieldRole, lending_confirm_due_date_);
  auto* label_502 = new QLabel(page);
  formLayout_17->setWidget(4, QFormLayout::ItemRole::LabelRole, label_502);
  lending_confirm_remarks_ = new QPlainTextEdit(page);
  set_size_policy(lending_confirm_remarks_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding, 0, 1);
  lending_confirm_remarks_->setReadOnly(true);
  formLayout_17->setWidget(4, QFormLayout::ItemRole::FieldRole, lending_confirm_remarks_);
  verticalLayout_56->addLayout(formLayout_17);
  auto* label_152 = new QLabel(page);
  verticalLayout_56->addWidget(label_152);
  auto* horizontalLayout_138 = new QHBoxLayout();
  auto* horizontalSpacer_49 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_138->addItem(horizontalSpacer_49);
  lending_confirm_finish_ = new QPushButton(page);
  horizontalLayout_138->addWidget(lending_confirm_finish_);
  verticalLayout_56->addLayout(horizontalLayout_138);

  lending_confirm_back_->setText(tr("back"));
  label_497->setText(tr("Step 4: Please check your lending data!"));
  label_498->setText(tr("person:"));
  label_499->setText(tr("item:"));
  label_500->setText(tr("number:"));
  label_501->setText(tr("due date:"));
  label_502->setText(tr("remarks:"));
  label_152->setText(tr("If the above values are correct, please click on \"finish\"!"));
  lending_confirm_finish_->setText(tr("finish"));
}

void colman::MainWindowUi::build_lending_done(QWidget* page)
{
  QFont font1;
  auto* verticalLayout_57 = new QVBoxLayout(page);
  lending_done_back_ = new QPushButton(page);
  verticalLayout_57->addWidget(lending_done_back_);
  auto* verticalSpacer_48 = new QSpacerItem(20, 300, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_57->addItem(verticalSpacer_48);
  auto* horizontalLayout_50 = new QHBoxLayout();
  auto* horizontalSpacer_17 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_50->addItem(horizontalSpacer_17);
  auto* label_157 = make_icon_label(page, ":/icons/owls/lending.jpeg");
  horizontalLayout_50->addWidget(label_157);
  auto* horizontalSpacer_18 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_50->addItem(horizontalSpacer_18);
  verticalLayout_57->addLayout(horizontalLayout_50);
  auto* horizontalLayout_51 = new QHBoxLayout();
  auto* horizontalSpacer_19 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_51->addItem(horizontalSpacer_19);
  lending_done_question_ = new QLabel(page);
  lending_done_question_->setFont(font1);
  lending_done_question_->setAlignment(Qt::AlignmentFlag::AlignCenter);
  horizontalLayout_51->addWidget(lending_done_question_);
  auto* horizontalSpacer_20 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_51->addItem(horizontalSpacer_20);
  verticalLayout_57->addLayout(horizontalLayout_51);
  auto* verticalSpacer_50 = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_57->addItem(verticalSpacer_50);
  auto* horizontalLayout_139 = new QHBoxLayout();
  auto* horizontalSpacer_50 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_139->addItem(horizontalSpacer_50);
  lending_done_yes_ = new QPushButton(page);
  horizontalLayout_139->addWidget(lending_done_yes_);
  lending_done_no_ = new QPushButton(page);
  horizontalLayout_139->addWidget(lending_done_no_);
  auto* horizontalSpacer_51 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_139->addItem(horizontalSpacer_51);
  verticalLayout_57->addLayout(horizontalLayout_139);
  auto* verticalSpacer_49 = new QSpacerItem(20, 400, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_57->addItem(verticalSpacer_49);

  lending_done_back_->setText(tr("back"));
  lending_done_question_->setText(tr("Further lendings for person?"));
  lending_done_yes_->setText(tr("yes"));
  lending_done_no_->setText(tr("no"));
}

/* EOF */
