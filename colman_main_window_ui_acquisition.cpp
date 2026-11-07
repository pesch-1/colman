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

void colman::MainWindowUi::build_acquisition_select(QWidget* page)
{
  auto* verticalLayout_70 = new QVBoxLayout(page);
  acquisition_select_back_ = new QPushButton(page);
  verticalLayout_70->addWidget(acquisition_select_back_);
  auto* horizontalLayout_60 = new QHBoxLayout();
  auto* label_177 = make_icon_label(page, ":/icons/owls/add_book.jpeg");
  horizontalLayout_60->addWidget(label_177);
  auto* verticalLayout_67 = new QVBoxLayout();
  auto* label_178 = new colman::ScalingLabel(page);
  set_size_policy(label_178, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_178->setFont(bold_font());
  label_178->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_67->addWidget(label_178);
  horizontalLayout_60->addLayout(verticalLayout_67);
  verticalLayout_70->addLayout(horizontalLayout_60);
  auto* label_11 = new QLabel(page);
  set_size_policy(label_11, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  label_11->setWordWrap(true);
  verticalLayout_70->addWidget(label_11);
  auto* horizontalLayout_27 = new QHBoxLayout();
  auto* label_10 = new QLabel(page);
  horizontalLayout_27->addWidget(label_10);
  acquisition_select_search_ = new QLineEdit(page);
  horizontalLayout_27->addWidget(acquisition_select_search_);
  acquisition_select_camera_ = new QToolButton(page);
  horizontalLayout_27->addWidget(acquisition_select_camera_);
  verticalLayout_70->addLayout(horizontalLayout_27);
  auto* horizontalLayout_acq_type = new QHBoxLayout();
  auto* label_acq_type = new QLabel(page);
  horizontalLayout_acq_type->addWidget(label_acq_type);
  acquisition_select_type_ = new QComboBox(page);
  horizontalLayout_acq_type->addWidget(acquisition_select_type_);
  auto* horizontalSpacer_acq_type = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_acq_type->addItem(horizontalSpacer_acq_type);
  acquisition_select_clear_ = new QPushButton(page);
  horizontalLayout_acq_type->addWidget(acquisition_select_clear_);
  acquisition_select_internet_search_ = new QPushButton(page);
  horizontalLayout_acq_type->addWidget(acquisition_select_internet_search_);
  verticalLayout_70->addLayout(horizontalLayout_acq_type);
  auto* splitter_9 = make_splitter(page, Qt::Orientation::Horizontal);
  splitter_9->setHandleWidth(5);
  acquisition_select_items_view_ = new QTableView(splitter_9);
  configure_item_view(acquisition_select_items_view_);
  acquisition_select_items_view_->setSortingEnabled(true);
  splitter_9->addWidget(acquisition_select_items_view_);
  acquisition_select_item_ = new colman::ItemWidget(splitter_9);
  splitter_9->addWidget(acquisition_select_item_);
  verticalLayout_70->addWidget(splitter_9);
  auto* horizontalLayout_29 = new QHBoxLayout();
  acquisition_select_item_count_ = new QLabel(page);
  horizontalLayout_29->addWidget(acquisition_select_item_count_);
  acquisition_select_load_more_ = new QPushButton(page);
  acquisition_select_load_more_->setVisible(false);
  horizontalLayout_29->addWidget(acquisition_select_load_more_);
  auto* horizontalSpacer_29 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_29->addItem(horizontalSpacer_29);
  acquisition_select_restore_ = new QPushButton(page);
  acquisition_select_restore_->setEnabled(false);
  horizontalLayout_29->addWidget(acquisition_select_restore_);
  acquisition_select_change_item_ = new QPushButton(page);
  acquisition_select_change_item_->setEnabled(false);
  horizontalLayout_29->addWidget(acquisition_select_change_item_);
  acquisition_select_new_item_ = new QPushButton(page);
  horizontalLayout_29->addWidget(acquisition_select_new_item_);
  acquisition_select_more_copies_ = new QPushButton(page);
  acquisition_select_more_copies_->setEnabled(false);
  horizontalLayout_29->addWidget(acquisition_select_more_copies_);
  verticalLayout_70->addLayout(horizontalLayout_29);

  acquisition_select_back_->setText(tr("back"));
  label_178->setText(tr("Step 1: Please select the item!"));
  label_11->setText(tr("Enter or scan an ISBN or product barcode (EAN/UPC) to search for an existing item (an internet look-up is triggered automatically for unknown barcodes). To add a completely new item, enter the data in the fields on the right and press \"As new item\"."));
  label_10->setText(tr("name/barcode:"));
#if QT_CONFIG(tooltip)
  acquisition_select_camera_->setToolTip(tr("Scan barcode with camera"));
#endif // QT_CONFIG(tooltip)
  label_acq_type->setText(tr("Type:"));
  acquisition_select_clear_->setText(tr("Clear fields"));
  acquisition_select_internet_search_->setText(tr("Internet search"));
  acquisition_select_load_more_->setText(tr("Load more..."));
  acquisition_select_restore_->setText(tr("Restore"));
  acquisition_select_change_item_->setText(tr("Change item"));
  acquisition_select_new_item_->setText(tr("As new item"));
  acquisition_select_more_copies_->setText(tr("Acquire more copies"));
}

void colman::MainWindowUi::build_acquisition_count(QWidget* page)
{
  auto* verticalLayout_74 = new QVBoxLayout(page);
  acquisition_count_back_ = new QPushButton(page);
  verticalLayout_74->addWidget(acquisition_count_back_);
  auto* horizontalLayout_62 = new QHBoxLayout();
  auto* label_181 = make_icon_label(page, ":/icons/owls/add_book.jpeg");
  horizontalLayout_62->addWidget(label_181);
  auto* verticalLayout_73 = new QVBoxLayout();
  auto* label_182 = new colman::ScalingLabel(page);
  label_182->setFont(bold_font());
  label_182->setTextFormat(Qt::TextFormat::AutoText);
  verticalLayout_73->addWidget(label_182);
  horizontalLayout_62->addLayout(verticalLayout_73);
  verticalLayout_74->addLayout(horizontalLayout_62);
  auto* verticalSpacer_18 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_74->addItem(verticalSpacer_18);
  auto* horizontalLayout_63 = new QHBoxLayout();
  auto* label_12 = new colman::ScalingLabel(page);
  set_size_policy(label_12, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
  label_12->setFont(bold_font());
  horizontalLayout_63->addWidget(label_12);
  acquisition_count_item_name_ = new colman::ScalingLabel(page);
  set_size_policy(acquisition_count_item_name_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  acquisition_count_item_name_->setFont(bold_font());
  horizontalLayout_63->addWidget(acquisition_count_item_name_);
  verticalLayout_74->addLayout(horizontalLayout_63);
  auto* horizontalLayout_64 = new QHBoxLayout();
  auto* label_13 = new QLabel(page);
  set_size_policy(label_13, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
  horizontalLayout_64->addWidget(label_13);
  acquisition_count_count_ = new QSpinBox(page);
  acquisition_count_count_->setMinimum(1);
  acquisition_count_count_->setMaximum(999999999);
  acquisition_count_count_->setValue(1);
  horizontalLayout_64->addWidget(acquisition_count_count_);
  verticalLayout_74->addLayout(horizontalLayout_64);
  auto* horizontalLayout_65 = new QHBoxLayout();
  label_14 = new QLabel(page);
  set_size_policy(label_14, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
  horizontalLayout_65->addWidget(label_14);
  acquisition_count_datetime_ = new QDateTimeEdit(page);
  horizontalLayout_65->addWidget(acquisition_count_datetime_);
  verticalLayout_74->addLayout(horizontalLayout_65);
  auto* verticalSpacer_20 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_74->addItem(verticalSpacer_20);
  auto* horizontalLayout_67 = new QHBoxLayout();
  auto* horizontalSpacer_31 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_67->addItem(horizontalSpacer_31);
  acquisition_count_finish_ = new QPushButton(page);
  acquisition_count_finish_->setEnabled(true);
  horizontalLayout_67->addWidget(acquisition_count_finish_);
  verticalLayout_74->addLayout(horizontalLayout_67);

  acquisition_count_back_->setText(tr("back"));
  label_182->setText(tr("Step 2: Edit acquisition details"));
  label_12->setText(tr("item:"));
  label_13->setText(tr("How many items have been acquired? "));
  label_14->setText(tr("date of acquisition?"));
  acquisition_count_finish_->setText(tr("finish"));
}

void colman::MainWindowUi::build_acquisition_done(QWidget* page)
{
  QFont font1;
  auto* verticalLayout_75 = new QVBoxLayout(page);
  auto* verticalSpacer_54 = new QSpacerItem(20, 285, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_75->addItem(verticalSpacer_54);
  auto* horizontalLayout_68 = new QHBoxLayout();
  auto* horizontalSpacer_32 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_68->addItem(horizontalSpacer_32);
  auto* label_183 = make_icon_label(page, ":/icons/owls/add_book.jpeg");
  horizontalLayout_68->addWidget(label_183);
  auto* horizontalSpacer_33 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_68->addItem(horizontalSpacer_33);
  verticalLayout_75->addLayout(horizontalLayout_68);
  auto* horizontalLayout_69 = new QHBoxLayout();
  auto* horizontalSpacer_34 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_69->addItem(horizontalSpacer_34);
  auto* lending_done_question_2 = new QLabel(page);
  lending_done_question_2->setFont(font1);
  lending_done_question_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
  horizontalLayout_69->addWidget(lending_done_question_2);
  auto* horizontalSpacer_35 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_69->addItem(horizontalSpacer_35);
  verticalLayout_75->addLayout(horizontalLayout_69);
  auto* horizontalLayout_144 = new QHBoxLayout();
  auto* horizontalSpacer_56 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_144->addItem(horizontalSpacer_56);
  acquisition_done_yes_ = new QPushButton(page);
  horizontalLayout_144->addWidget(acquisition_done_yes_);
  acquisition_done_no_ = new QPushButton(page);
  horizontalLayout_144->addWidget(acquisition_done_no_);
  auto* horizontalSpacer_57 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_144->addItem(horizontalSpacer_57);
  verticalLayout_75->addLayout(horizontalLayout_144);
  auto* horizontalLayout_acq5_print = new QHBoxLayout();
  auto* horizontalSpacer_acq5_pl = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_acq5_print->addItem(horizontalSpacer_acq5_pl);
  acquisition_done_print_card_ = new QPushButton(page);
  horizontalLayout_acq5_print->addWidget(acquisition_done_print_card_);
  acquisition_done_print_top_ = new QPushButton(page);
  horizontalLayout_acq5_print->addWidget(acquisition_done_print_top_);
  acquisition_done_print_bottom_ = new QPushButton(page);
  horizontalLayout_acq5_print->addWidget(acquisition_done_print_bottom_);
  auto* horizontalSpacer_acq5_pr = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_acq5_print->addItem(horizontalSpacer_acq5_pr);
  verticalLayout_75->addLayout(horizontalLayout_acq5_print);
  auto* verticalSpacer_55 = new QSpacerItem(20, 285, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_75->addItem(verticalSpacer_55);

  lending_done_question_2->setText(tr("Further acquisitions?"));
  acquisition_done_yes_->setText(tr("yes"));
  acquisition_done_no_->setText(tr("no"));
  acquisition_done_print_card_->setText(tr("Print item card"));
  acquisition_done_print_top_->setText(tr("Print upper signature"));
  acquisition_done_print_bottom_->setText(tr("Print lower signature"));
}

/* EOF */
