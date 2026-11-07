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

void colman::MainWindowUi::build_return_item(QWidget* page)
{
  auto* verticalLayout_39 = new QVBoxLayout(page);
  return_item_back_ = new QPushButton(page);
  verticalLayout_39->addWidget(return_item_back_);
  auto* horizontalLayout_54 = new QHBoxLayout();
  auto* label_160 = make_icon_label(page, ":/icons/owls/returns.jpeg");
  horizontalLayout_54->addWidget(label_160);
  auto* verticalLayout_58 = new QVBoxLayout();
  auto* label_474 = new colman::ScalingLabel(page);
  label_474->setFont(bold_font());
  verticalLayout_58->addWidget(label_474);
  auto* horizontalLayout_141 = new QHBoxLayout();
  auto* label_475 = new QLabel(page);
  horizontalLayout_141->addWidget(label_475);
  return_item_barcode_ = new QLineEdit(page);
  horizontalLayout_141->addWidget(return_item_barcode_);
  return_item_camera_ = new QToolButton(page);
  horizontalLayout_141->addWidget(return_item_camera_);
  verticalLayout_58->addLayout(horizontalLayout_141);
  horizontalLayout_54->addLayout(verticalLayout_58);
  verticalLayout_39->addLayout(horizontalLayout_54);
  auto* splitter_53 = new QSplitter(page);
  set_size_policy(splitter_53, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_53->setOrientation(Qt::Orientation::Horizontal);
  splitter_53->setHandleWidth(5);
  return_item_item_view_ = new QTableView(splitter_53);
  return_item_item_view_->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  return_item_item_view_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  splitter_53->addWidget(return_item_item_view_);
  return_item_item_widget_ = new colman::ItemWidget(splitter_53);
  set_size_policy(return_item_item_widget_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_53->addWidget(return_item_item_widget_);
  verticalLayout_39->addWidget(splitter_53);
  auto* horizontalLayout_140 = new QHBoxLayout();
  auto* horizontalSpacer_52 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_140->addItem(horizontalSpacer_52);
  return_item_next_ = new QPushButton(page);
  return_item_next_->setEnabled(false);
  horizontalLayout_140->addWidget(return_item_next_);
  verticalLayout_39->addLayout(horizontalLayout_140);

  return_item_back_->setText(tr("back"));
  label_474->setText(tr("Step 1: Please select returned item!"));
  label_475->setText(tr("barcode/name:"));
#if QT_CONFIG(tooltip)
  return_item_camera_->setToolTip(tr("Scan barcode with camera"));
#endif // QT_CONFIG(tooltip)
  return_item_next_->setText(tr("next"));
}

void colman::MainWindowUi::build_return_borrower(QWidget* page)
{
  auto* verticalLayout_26 = new QVBoxLayout(page);
  return_borrower_back_ = new QPushButton(page);
  verticalLayout_26->addWidget(return_borrower_back_);
  auto* horizontalLayout_53 = new QHBoxLayout();
  auto* label_158 = make_icon_label(page, ":/icons/owls/returns.jpeg");
  horizontalLayout_53->addWidget(label_158);
  auto* verticalLayout_51 = new QVBoxLayout();
  auto* label_159 = new colman::ScalingLabel(page);
  label_159->setFont(bold_font());
  label_159->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_51->addWidget(label_159);
  horizontalLayout_53->addLayout(verticalLayout_51);
  verticalLayout_26->addLayout(horizontalLayout_53);
  auto* splitter_11 = new QSplitter(page);
  set_size_policy(splitter_11, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_11->setOrientation(Qt::Orientation::Horizontal);
  return_borrower_person_list_ = new colman::PersonCheckListWidget(splitter_11);
  set_size_policy(return_borrower_person_list_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_11->addWidget(return_borrower_person_list_);
  return_borrower_person_ = new colman::PersonWidget(splitter_11);
  set_size_policy(return_borrower_person_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  splitter_11->addWidget(return_borrower_person_);
  verticalLayout_26->addWidget(splitter_11);
  return_borrower_message_ = new QLabel(page);
  return_borrower_message_->setWordWrap(true);
  verticalLayout_26->addWidget(return_borrower_message_);
  auto* horizontalLayout_10 = new QHBoxLayout();
  return_borrower_status_ = new QLabel(page);
  horizontalLayout_10->addWidget(return_borrower_status_);
  auto* horizontalSpacer_21 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_10->addItem(horizontalSpacer_21);
  return_borrower_next_ = new QPushButton(page);
  return_borrower_next_->setEnabled(false);
  horizontalLayout_10->addWidget(return_borrower_next_);
  verticalLayout_26->addLayout(horizontalLayout_10);

  return_borrower_back_->setText(tr("back"));
  label_159->setText(tr("Step 2: Please tick the returning person(s)!"));
  return_borrower_next_->setText(tr("next"));
}

void colman::MainWindowUi::build_return_lending(QWidget* page)
{
  auto* verticalLayout_52 = new QVBoxLayout(page);
  return_lending_back_ = new QPushButton(page);
  verticalLayout_52->addWidget(return_lending_back_);
  auto* horizontalLayout_55 = new QHBoxLayout();
  auto* label_161 = make_icon_label(page, ":/icons/owls/returns.jpeg");
  horizontalLayout_55->addWidget(label_161);
  auto* verticalLayout_59 = new QVBoxLayout();
  auto* label_162 = new colman::ScalingLabel(page);
  label_162->setFont(bold_font());
  label_162->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_59->addWidget(label_162);
  horizontalLayout_55->addLayout(verticalLayout_59);
  verticalLayout_52->addLayout(horizontalLayout_55);
  auto* formLayout_7 = new QFormLayout();
  auto* label_7 = new colman::ScalingLabel(page);
  set_size_policy(label_7, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_7->setFont(bold_font());
  formLayout_7->setWidget(0, QFormLayout::ItemRole::LabelRole, label_7);
  return_lending_person_name_ = new QLabel(page);
  formLayout_7->setWidget(0, QFormLayout::ItemRole::FieldRole, return_lending_person_name_);
  auto* label_8 = new colman::ScalingLabel(page);
  set_size_policy(label_8, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_8->setFont(bold_font());
  formLayout_7->setWidget(1, QFormLayout::ItemRole::LabelRole, label_8);
  return_lending_item_name_ = new QLabel(page);
  formLayout_7->setWidget(1, QFormLayout::ItemRole::FieldRole, return_lending_item_name_);
  verticalLayout_52->addLayout(formLayout_7);
  auto* label_148 = new colman::ScalingLabel(page);
  label_148->setFont(bold_font());
  verticalLayout_52->addWidget(label_148);
  return_lending_lendings_view_ = new QTableView(page);
  configure_item_view(return_lending_lendings_view_);
  return_lending_lendings_view_->setSortingEnabled(true);
  return_lending_lendings_view_->verticalHeader()->setVisible(false);
  verticalLayout_52->addWidget(return_lending_lendings_view_);
  auto* horizontalLayout_11 = new QHBoxLayout();
  auto* horizontalSpacer_22 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_11->addItem(horizontalSpacer_22);
  return_lending_next_ = new QPushButton(page);
  return_lending_next_->setEnabled(false);
  horizontalLayout_11->addWidget(return_lending_next_);
  verticalLayout_52->addLayout(horizontalLayout_11);

  return_lending_back_->setText(tr("back"));
  label_162->setText(tr("Step 3: Please select lending corresponding to returned item!"));
  label_7->setText(tr("Lending person: "));
  label_8->setText(tr("Item:"));
  label_148->setText(tr("Lendings:"));
  return_lending_next_->setText(tr("next"));
}

void colman::MainWindowUi::build_return_details(QWidget* page)
{
  QFont font2;
  font2.setBold(false);
  auto* verticalLayout_62 = new QVBoxLayout(page);
  return_details_back_ = new QPushButton(page);
  verticalLayout_62->addWidget(return_details_back_);
  auto* horizontalLayout_56 = new QHBoxLayout();
  auto* label_163 = make_icon_label(page, ":/icons/owls/returns.jpeg");
  horizontalLayout_56->addWidget(label_163);
  auto* verticalLayout_61 = new QVBoxLayout();
  label_164 = new colman::ScalingLabel(page);
  label_164->setFont(bold_font());
  label_164->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_61->addWidget(label_164);
  horizontalLayout_56->addLayout(verticalLayout_61);
  verticalLayout_62->addLayout(horizontalLayout_56);
  auto* formLayout = new QFormLayout();
  auto* label_5 = new colman::ScalingLabel(page);
  label_5->setFont(bold_font());
  formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_5);
  return_details_item_name_ = new QLabel(page);
  set_size_policy(return_details_item_name_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  return_details_item_name_->setFont(font2);
  formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, return_details_item_name_);
  auto* label_165 = new QLabel(page);
  formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label_165);
  return_details_lent_count_ = new QLabel(page);
  set_size_policy(return_details_lent_count_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  return_details_lent_count_->setText(QString::fromUtf8("0"));
  formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, return_details_lent_count_);
  auto* label_166 = new QLabel(page);
  formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, label_166);
  return_details_returned_count_ = new QLabel(page);
  set_size_policy(return_details_returned_count_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  return_details_returned_count_->setText(QString::fromUtf8("0"));
  formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, return_details_returned_count_);
  auto* label_167 = new QLabel(page);
  formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, label_167);
  return_details_remaining_count_ = new QLabel(page);
  set_size_policy(return_details_remaining_count_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  return_details_remaining_count_->setText(QString::fromUtf8("0"));
  formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, return_details_remaining_count_);
  auto* label_6 = new colman::ScalingLabel(page);
  label_6->setFont(bold_font());
  formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label_6);
  return_details_person_name_ = new QLabel(page);
  set_size_policy(return_details_person_name_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, return_details_person_name_);
  verticalLayout_62->addLayout(formLayout);
  return_details_counts_form_ = formLayout;
  return_details_summary_ = new QLabel(page);
  return_details_summary_->setWordWrap(true);
  return_details_summary_->setVisible(false);
  verticalLayout_62->addWidget(return_details_summary_);
  auto* verticalSpacer_17 = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_62->addItem(verticalSpacer_17);
  auto* formLayout_6 = new QFormLayout();
  formLayout_6->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
  formLayout_6->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);
  formLayout_6->setVerticalSpacing(5);
  auto* label_168 = new QLabel(page);
  formLayout_6->setWidget(0, QFormLayout::ItemRole::LabelRole, label_168);
  return_details_count_ = new QSpinBox(page);
  return_details_count_->setMinimum(1);
  return_details_count_->setMaximum(1000000);
  formLayout_6->setWidget(0, QFormLayout::ItemRole::FieldRole, return_details_count_);
  label_169 = new QLabel(page);
  formLayout_6->setWidget(1, QFormLayout::ItemRole::LabelRole, label_169);
  return_details_datetime_ = new QDateTimeEdit(page);
  return_details_datetime_->setCalendarPopup(true);
  formLayout_6->setWidget(1, QFormLayout::ItemRole::FieldRole, return_details_datetime_);
  auto* label_170 = new QLabel(page);
  set_size_policy(label_170, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  formLayout_6->setWidget(2, QFormLayout::ItemRole::LabelRole, label_170);
  return_details_remarks_ = new QPlainTextEdit(page);
  set_size_policy(return_details_remarks_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding, 0, 1);
  formLayout_6->setWidget(2, QFormLayout::ItemRole::FieldRole, return_details_remarks_);
  verticalLayout_62->addLayout(formLayout_6);
  return_details_count_form_ = formLayout_6;
  auto* horizontalLayout_16 = new QHBoxLayout();
  return_details_extend_ = new QPushButton(page);
  horizontalLayout_16->addWidget(return_details_extend_);
  auto* horizontalSpacer_23 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_16->addItem(horizontalSpacer_23);
  return_details_next_ = new QPushButton(page);
  return_details_next_->setEnabled(false);
  horizontalLayout_16->addWidget(return_details_next_);
  verticalLayout_62->addLayout(horizontalLayout_16);

  return_details_back_->setText(tr("back"));
  label_164->setText(tr("Step 4: Please select number of returned items!"));
  label_5->setText(tr("item:"));
  label_165->setText(tr("lent: "));
  label_166->setText(tr("already returned: "));
  label_167->setText(tr("still lent: "));
  label_6->setText(tr("lending person:"));
  label_168->setText(tr("count: "));
  label_169->setText(tr("date and time: "));
  return_details_datetime_->setDisplayFormat(tr("yyyy-MM-dd HH:mm:ss"));
  label_170->setText(tr("remarks: "));
  return_details_extend_->setText(tr("extend due date..."));
  return_details_next_->setText(tr("next"));
}

void colman::MainWindowUi::build_return_confirm(QWidget* page)
{
  auto* verticalLayout_64 = new QVBoxLayout(page);
  return_confirm_back_ = new QPushButton(page);
  verticalLayout_64->addWidget(return_confirm_back_);
  auto* horizontalLayout_57 = new QHBoxLayout();
  auto* label_171 = make_icon_label(page, ":/icons/owls/returns.jpeg");
  horizontalLayout_57->addWidget(label_171);
  auto* verticalLayout_63 = new QVBoxLayout();
  auto* label_172 = new colman::ScalingLabel(page);
  label_172->setFont(bold_font());
  label_172->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_63->addWidget(label_172);
  horizontalLayout_57->addLayout(verticalLayout_63);
  verticalLayout_64->addLayout(horizontalLayout_57);
  auto* formLayout_18 = new QFormLayout();
  auto* label_503 = new QLabel(page);
  formLayout_18->setWidget(0, QFormLayout::ItemRole::LabelRole, label_503);
  return_confirm_person_ = new QLineEdit(page);
  return_confirm_person_->setReadOnly(true);
  formLayout_18->setWidget(0, QFormLayout::ItemRole::FieldRole, return_confirm_person_);
  auto* label_504 = new QLabel(page);
  formLayout_18->setWidget(1, QFormLayout::ItemRole::LabelRole, label_504);
  return_confirm_item_ = new QLineEdit(page);
  return_confirm_item_->setReadOnly(true);
  formLayout_18->setWidget(1, QFormLayout::ItemRole::FieldRole, return_confirm_item_);
  auto* label_505 = new QLabel(page);
  formLayout_18->setWidget(2, QFormLayout::ItemRole::LabelRole, label_505);
  return_confirm_number_ = new QLineEdit(page);
  return_confirm_number_->setReadOnly(true);
  formLayout_18->setWidget(2, QFormLayout::ItemRole::FieldRole, return_confirm_number_);
  auto* label_506 = new QLabel(page);
  formLayout_18->setWidget(3, QFormLayout::ItemRole::LabelRole, label_506);
  return_confirm_datetime_ = new QLineEdit(page);
  return_confirm_datetime_->setReadOnly(true);
  formLayout_18->setWidget(3, QFormLayout::ItemRole::FieldRole, return_confirm_datetime_);
  auto* label_507 = new QLabel(page);
  formLayout_18->setWidget(4, QFormLayout::ItemRole::LabelRole, label_507);
  return_confirm_remarks_ = new QPlainTextEdit(page);
  set_size_policy(return_confirm_remarks_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding, 0, 1);
  return_confirm_remarks_->setReadOnly(true);
  formLayout_18->setWidget(4, QFormLayout::ItemRole::FieldRole, return_confirm_remarks_);
  return_confirm_persons_ = new QListWidget(page);
  return_confirm_persons_->setSelectionMode(
      QAbstractItemView::SelectionMode::NoSelection);
  set_size_policy(return_confirm_persons_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding, 0, 3);
  formLayout_18->addRow(tr("persons:"), return_confirm_persons_);
  formLayout_18->setRowVisible(return_confirm_persons_, false);
  verticalLayout_64->addLayout(formLayout_18, 1);
  return_confirm_form_ = formLayout_18;
  auto* label_173 = new QLabel(page);
  verticalLayout_64->addWidget(label_173);
  auto* horizontalLayout_142 = new QHBoxLayout();
  auto* horizontalSpacer_53 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_142->addItem(horizontalSpacer_53);
  return_confirm_finish_ = new QPushButton(page);
  return_confirm_finish_->setEnabled(true);
  horizontalLayout_142->addWidget(return_confirm_finish_);
  verticalLayout_64->addLayout(horizontalLayout_142);

  return_confirm_back_->setText(tr("back"));
  label_172->setText(tr("Step 5: Please check your return data!"));
  label_503->setText(tr("person:"));
  label_504->setText(tr("item:"));
  label_505->setText(tr("number:"));
  label_506->setText(tr("date and time:"));
  label_507->setText(tr("remarks:"));
  label_173->setText(tr("If the above values are correct, please click on \"finish\"!"));
  return_confirm_finish_->setText(tr("finish"));
}

void colman::MainWindowUi::build_return_done(QWidget* page)
{
  QFont font1;
  auto* verticalLayout_65 = new QVBoxLayout(page);
  return_done_back_ = new QPushButton(page);
  verticalLayout_65->addWidget(return_done_back_);
  auto* verticalSpacer_52 = new QSpacerItem(20, 271, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_65->addItem(verticalSpacer_52);
  auto* horizontalLayout_52 = new QHBoxLayout();
  auto* horizontalSpacer_24 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_52->addItem(horizontalSpacer_24);
  auto* label_174 = make_icon_label(page, ":/icons/owls/returns.jpeg");
  horizontalLayout_52->addWidget(label_174);
  auto* horizontalSpacer_25 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_52->addItem(horizontalSpacer_25);
  verticalLayout_65->addLayout(horizontalLayout_52);
  auto* horizontalLayout_58 = new QHBoxLayout();
  auto* horizontalSpacer_26 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_58->addItem(horizontalSpacer_26);
  auto* lending_done_question_1 = new QLabel(page);
  lending_done_question_1->setFont(font1);
  lending_done_question_1->setAlignment(Qt::AlignmentFlag::AlignCenter);
  horizontalLayout_58->addWidget(lending_done_question_1);
  auto* horizontalSpacer_27 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_58->addItem(horizontalSpacer_27);
  verticalLayout_65->addLayout(horizontalLayout_58);
  auto* verticalSpacer_51 = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_65->addItem(verticalSpacer_51);
  auto* horizontalLayout_143 = new QHBoxLayout();
  auto* horizontalSpacer_54 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_143->addItem(horizontalSpacer_54);
  return_done_yes_ = new QPushButton(page);
  horizontalLayout_143->addWidget(return_done_yes_);
  return_done_no_ = new QPushButton(page);
  horizontalLayout_143->addWidget(return_done_no_);
  auto* horizontalSpacer_55 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_143->addItem(horizontalSpacer_55);
  verticalLayout_65->addLayout(horizontalLayout_143);
  auto* verticalSpacer_53 = new QSpacerItem(20, 271, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_65->addItem(verticalSpacer_53);

  return_done_back_->setText(tr("back"));
  lending_done_question_1->setText(tr("Further returned items?"));
  return_done_yes_->setText(tr("yes"));
  return_done_no_->setText(tr("no"));
}

/* EOF */
