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
#ifndef COLMAN_CODE128_ITEM_HPP
#  include "colman_code128_item.hpp"
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

void colman::MainWindowUi::build_item_type_page(QWidget* page)
{
  auto* verticalLayout_42 = new QVBoxLayout(page);
  item_type_back_ = new QPushButton(page);
  verticalLayout_42->addWidget(item_type_back_);
  auto* label_115 = new colman::ScalingLabel(page);
  set_size_policy(label_115, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_115->setFont(bold_font());
  verticalLayout_42->addWidget(label_115);
  auto* verticalSpacer_15 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_42->addItem(verticalSpacer_15);
  auto* splitter_15 = make_splitter(page, Qt::Orientation::Horizontal);
  splitter_15->setHandleWidth(6);
  auto* layoutWidget_1 = new QWidget(splitter_15);
  auto* verticalLayout_40 = new QVBoxLayout(layoutWidget_1);
  verticalLayout_40->setContentsMargins(0, 0, 0, 0);
  auto* label_118 = new colman::ScalingLabel(layoutWidget_1);
  label_118->setFont(bold_font());
  verticalLayout_40->addWidget(label_118);
  item_type_type_view_ = new QTableView(layoutWidget_1);
  configure_item_view(item_type_type_view_);
  verticalLayout_40->addWidget(item_type_type_view_);
  auto* horizontalLayout_39 = new QHBoxLayout();
  item_type_remove_ = new QPushButton(layoutWidget_1);
  horizontalLayout_39->addWidget(item_type_remove_);
  item_type_change_ = new QPushButton(layoutWidget_1);
  horizontalLayout_39->addWidget(item_type_change_);
  verticalLayout_40->addLayout(horizontalLayout_39);
  splitter_15->addWidget(layoutWidget_1);
  auto* layoutWidget_2 = new QWidget(splitter_15);
  auto* verticalLayout_41 = new QVBoxLayout(layoutWidget_2);
  verticalLayout_41->setContentsMargins(0, 0, 0, 0);
  auto* horizontalLayout_37 = new QHBoxLayout();
  auto* label_116 = new QLabel(layoutWidget_2);
  horizontalLayout_37->addWidget(label_116);
  item_type_name_ = new QLineEdit(layoutWidget_2);
  horizontalLayout_37->addWidget(item_type_name_);
  verticalLayout_41->addLayout(horizontalLayout_37);
  auto* horizontalLayout_media_kind = new QHBoxLayout();
  auto* label_media_kind = new QLabel(layoutWidget_2);
  horizontalLayout_media_kind->addWidget(label_media_kind);
  item_type_media_kind_ = new QComboBox(layoutWidget_2);
  horizontalLayout_media_kind->addWidget(item_type_media_kind_);
  verticalLayout_41->addLayout(horizontalLayout_media_kind);
  auto* label_117 = new colman::ScalingLabel(layoutWidget_2);
  label_117->setFont(bold_font());
  verticalLayout_41->addWidget(label_117);
  item_type_fields_ = new QTableWidget(layoutWidget_2);
  if(item_type_fields_->columnCount() < 8) {
    item_type_fields_->setColumnCount(8);
  }
  QTableWidgetItem* __qtablewidgetitem = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(0, __qtablewidgetitem);
  QTableWidgetItem* __qtablewidgetitem1 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(1, __qtablewidgetitem1);
  QTableWidgetItem* __qtablewidgetitem2 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(2, __qtablewidgetitem2);
  QTableWidgetItem* __qtablewidgetitem3 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(3, __qtablewidgetitem3);
  QTableWidgetItem* __qtablewidgetitem4 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(4, __qtablewidgetitem4);
  QTableWidgetItem* __qtablewidgetitem5 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(5, __qtablewidgetitem5);
  QTableWidgetItem* __qtablewidgetitem6 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(6, __qtablewidgetitem6);
  QTableWidgetItem* __qtablewidgetitem7 = new QTableWidgetItem();
  item_type_fields_->setHorizontalHeaderItem(7, __qtablewidgetitem7);
  configure_item_view(item_type_fields_);
  item_type_fields_->setSortingEnabled(false);
  item_type_fields_->setColumnCount(8);
  verticalLayout_41->addWidget(item_type_fields_);
  auto* horizontalLayout_38 = new QHBoxLayout();
  auto* item_type_remove_all_ = new QPushButton(layoutWidget_2);
  horizontalLayout_38->addWidget(item_type_remove_all_);
  item_type_remove_field_ = new QPushButton(layoutWidget_2);
  horizontalLayout_38->addWidget(item_type_remove_field_);
  item_type_add_field_ = new QPushButton(layoutWidget_2);
  horizontalLayout_38->addWidget(item_type_add_field_);
  item_type_move_up_ = new QPushButton(layoutWidget_2);
  horizontalLayout_38->addWidget(item_type_move_up_);
  item_type_move_down_ = new QPushButton(layoutWidget_2);
  horizontalLayout_38->addWidget(item_type_move_down_);
  item_type_marc21_template_ = new QToolButton(layoutWidget_2);
  item_type_marc21_template_->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
  item_type_marc21_template_->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  horizontalLayout_38->addWidget(item_type_marc21_template_);
  auto* horizontalSpacer_13 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_38->addItem(horizontalSpacer_13);
  verticalLayout_41->addLayout(horizontalLayout_38);
  auto* horizontalLayout_40 = new QHBoxLayout();
  item_type_revert_ = new QPushButton(layoutWidget_2);
  horizontalLayout_40->addWidget(item_type_revert_);
  item_type_insert_ = new QPushButton(layoutWidget_2);
  horizontalLayout_40->addWidget(item_type_insert_);
  verticalLayout_41->addLayout(horizontalLayout_40);
  splitter_15->addWidget(layoutWidget_2);
  verticalLayout_42->addWidget(splitter_15);

  item_type_back_->setText(tr("back"));
  label_115->setText(tr("Add, change or remove item type"));
  label_118->setText(tr("known item types:"));
  item_type_remove_->setText(tr("remove type"));
  item_type_change_->setText(tr("change type"));
  label_116->setText(tr("item type name:"));
#if QT_CONFIG(tooltip)
  item_type_name_->setToolTip(tr("An item type is something like 'book', 'CD' or 'DVD'"));
#endif // QT_CONFIG(tooltip)
  label_media_kind->setText(tr("search category:"));
#if QT_CONFIG(tooltip)
  item_type_media_kind_->setToolTip(tr("Maps this type to an internet search category (book, CD, DVD, Blu-ray, board game) independently of its name"));
#endif // QT_CONFIG(tooltip)
  label_117->setText(tr("item type fields:"));
  QTableWidgetItem* ___qtablewidgetitem = item_type_fields_->horizontalHeaderItem(0);
  ___qtablewidgetitem->setText(tr("field name"));
  QTableWidgetItem* ___qtablewidgetitem1 = item_type_fields_->horizontalHeaderItem(1);
  ___qtablewidgetitem1->setText(tr("field type"));
  QTableWidgetItem* ___qtablewidgetitem2 = item_type_fields_->horizontalHeaderItem(2);
  ___qtablewidgetitem2->setText(tr("use minimum"));
  QTableWidgetItem* ___qtablewidgetitem3 = item_type_fields_->horizontalHeaderItem(3);
  ___qtablewidgetitem3->setText(tr("minimum"));
  QTableWidgetItem* ___qtablewidgetitem4 = item_type_fields_->horizontalHeaderItem(4);
  ___qtablewidgetitem4->setText(tr("use maximum"));
  QTableWidgetItem* ___qtablewidgetitem5 = item_type_fields_->horizontalHeaderItem(5);
  ___qtablewidgetitem5->setText(tr("maximum"));
  QTableWidgetItem* ___qtablewidgetitem6 = item_type_fields_->horizontalHeaderItem(6);
  ___qtablewidgetitem6->setText(tr("edit"));
  QTableWidgetItem* ___qtablewidgetitem7 = item_type_fields_->horizontalHeaderItem(7);
  ___qtablewidgetitem7->setText(tr("Repeatable"));
  item_type_remove_all_->setText(tr("remove all"));
  item_type_remove_field_->setText(tr("remove field"));
  item_type_add_field_->setText(tr("add field"));
  item_type_move_up_->setText(tr("Move up"));
  item_type_move_down_->setText(tr("Move down"));
  item_type_marc21_template_->setText(tr("MARC21 template"));
  item_type_revert_->setText(tr("revert"));
  item_type_insert_->setText(tr("insert type"));

  label_116->setBuddy(item_type_name_);
  label_media_kind->setBuddy(item_type_media_kind_);
}

void colman::MainWindowUi::build_person_page(QWidget* page)
{
  auto* verticalLayout_10 = new QVBoxLayout(page);
  back_button2_ = new QPushButton(page);
  verticalLayout_10->addWidget(back_button2_);
  auto* label_88 = new colman::ScalingLabel(page);
  set_size_policy(label_88, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_88->setFont(bold_font());
  verticalLayout_10->addWidget(label_88);
  auto* splitter_4 = make_splitter(page, Qt::Orientation::Horizontal);
  auto* layoutWidget_3 = new QWidget(splitter_4);
  auto* verticalLayout_8 = new QVBoxLayout(layoutWidget_3);
  verticalLayout_8->setContentsMargins(0, 0, 0, 0);
  person_person_list_ = new colman::PersonListWidget(layoutWidget_3);
  set_size_policy(person_person_list_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  verticalLayout_8->addWidget(person_person_list_);
  splitter_4->addWidget(layoutWidget_3);
  auto* person_right_scroll_ = new QScrollArea(splitter_4);
  person_right_scroll_->setWidgetResizable(true);
  person_right_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto* layoutWidget_5 = new QWidget();
  verticalLayout_9 = new QVBoxLayout(layoutWidget_5);
  person_person_ = new colman::PersonWidget(layoutWidget_5);
  set_size_policy(person_person_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  verticalLayout_9->addWidget(person_person_);
  person_right_scroll_->setWidget(layoutWidget_5);
  splitter_4->addWidget(person_right_scroll_);
  verticalLayout_10->addWidget(splitter_4);
  auto* horizontalLayout_2 = new QHBoxLayout();
  person_clear_ = new QPushButton(page);
  horizontalLayout_2->addWidget(person_clear_);
  remove_person_ = new QPushButton(page);
  remove_person_->setEnabled(false);
  horizontalLayout_2->addWidget(remove_person_);
  change_person_ = new QPushButton(page);
  change_person_->setEnabled(false);
  horizontalLayout_2->addWidget(change_person_);
  insert_person_ = new QPushButton(page);
  insert_person_->setEnabled(false);
  horizontalLayout_2->addWidget(insert_person_);
  verticalLayout_10->addLayout(horizontalLayout_2);

  back_button2_->setText(tr("back"));
  label_88->setText(tr("Add, change or delete persons"));
#if QT_CONFIG(tooltip)
  person_clear_->setToolTip(tr("clears all fields above"));
#endif // QT_CONFIG(tooltip)
  person_clear_->setText(tr("clear fields"));
#if QT_CONFIG(tooltip)
  remove_person_->setToolTip(tr("removes the selected person"));
#endif // QT_CONFIG(tooltip)
  remove_person_->setText(tr("remove person"));
#if QT_CONFIG(tooltip)
  change_person_->setToolTip(tr("uses your edits to change selected person"));
#endif // QT_CONFIG(tooltip)
  change_person_->setText(tr("change person"));
#if QT_CONFIG(tooltip)
  insert_person_->setToolTip(tr("adds above values as a new person"));
#endif // QT_CONFIG(tooltip)
  insert_person_->setText(tr("insert as new person"));

  QObject::connect(person_clear_, &QPushButton::clicked, person_person_, qOverload<>(&colman::PersonWidget::clear_fields));
}

void colman::MainWindowUi::build_person_barcode_page(QWidget* page)
{
  auto* verticalLayout_24 = new QVBoxLayout(page);
  person_barcode_back_ = new QPushButton(page);
  set_size_policy(person_barcode_back_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
  verticalLayout_24->addWidget(person_barcode_back_);
  auto* horizontalLayout_pbc_heading = new QHBoxLayout();
  auto* label_85 = new colman::ScalingLabel(page);
  set_size_policy(label_85, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_85->setFont(bold_font());
  horizontalLayout_pbc_heading->addWidget(label_85);
  verticalLayout_24->addLayout(horizontalLayout_pbc_heading);
  // Search row: only acts on the person tree of the selection step, so it
  // is added inside the selection page below and not shown on the print step.
  auto* horizontalLayout_pbc_search = new QHBoxLayout();
  auto* label_pbc_search = new QLabel(page);
  horizontalLayout_pbc_search->addWidget(label_pbc_search);
  person_barcode_search_ = new QLineEdit(page);
  person_barcode_search_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  horizontalLayout_pbc_search->addWidget(person_barcode_search_);
  auto* label_pbc_class = new QLabel(page);
  horizontalLayout_pbc_search->addWidget(label_pbc_class);
  person_barcode_class_ = new QComboBox(page);
  person_barcode_class_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  horizontalLayout_pbc_search->addWidget(person_barcode_class_);
  person_barcode_select_all_ = new QPushButton(page);
  horizontalLayout_pbc_search->addWidget(person_barcode_select_all_);
  person_barcode_deselect_all_ = new QPushButton(page);
  horizontalLayout_pbc_search->addWidget(person_barcode_deselect_all_);
  auto* horizontalSpacer_pbc_search = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_pbc_search->addItem(horizontalSpacer_pbc_search);
  // Optional restriction to the most recently created persons. Disabled by
  // default; the spin box stays disabled until the check box is ticked.
  person_barcode_recent_check_ = new QCheckBox(page);
  horizontalLayout_pbc_search->addWidget(person_barcode_recent_check_);
  person_barcode_recent_count_ = new QSpinBox(page);
  person_barcode_recent_count_->setRange(1, 9999);
  person_barcode_recent_count_->setValue(20);
  person_barcode_recent_count_->setEnabled(false);
  horizontalLayout_pbc_search->addWidget(person_barcode_recent_count_);

  // The "show border" option applies to the printed cards, so it stays
  // above the stack and remains visible on the print step.
  auto* horizontalLayout_pbc_top = new QHBoxLayout();
  person_barcode_show_border_ = new QCheckBox(page);
  person_barcode_show_border_->setChecked(true);
  horizontalLayout_pbc_top->addWidget(person_barcode_show_border_);
  auto* horizontalSpacer_pbc_top = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_pbc_top->addItem(horizontalSpacer_pbc_top);
  verticalLayout_24->addLayout(horizontalLayout_pbc_top);
  person_barcode_stack_ = new QStackedWidget(page);
  auto* person_barcode_sel_page_ = new QWidget();
  auto* verticalLayout_pbc_sel = new QVBoxLayout(person_barcode_sel_page_);
  auto* person_barcode_splitter_ = make_splitter(person_barcode_sel_page_, Qt::Orientation::Horizontal);
  person_barcode_tree_ = new QTreeWidget(person_barcode_splitter_);
  person_barcode_tree_->setAlternatingRowColors(true);
  person_barcode_tree_->setSortingEnabled(true);
  person_barcode_tree_->setColumnCount(3);
  person_barcode_splitter_->addWidget(person_barcode_tree_);
  person_barcode_person_ = new colman::PersonWidget(person_barcode_splitter_);
  set_size_policy(person_barcode_person_, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  person_barcode_splitter_->addWidget(person_barcode_person_);
  verticalLayout_pbc_sel->addLayout(horizontalLayout_pbc_search);
  verticalLayout_pbc_sel->addWidget(person_barcode_splitter_);
  auto* horizontalLayout_pbc_bottom = new QHBoxLayout();
  person_barcode_selection_count_ = new QLabel(person_barcode_sel_page_);
  horizontalLayout_pbc_bottom->addWidget(person_barcode_selection_count_);
  auto* horizontalSpacer_pbc_bottom = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_pbc_bottom->addItem(horizontalSpacer_pbc_bottom);
  person_card_print_ = new QPushButton(person_barcode_sel_page_);
  person_card_print_->setEnabled(false);
  horizontalLayout_pbc_bottom->addWidget(person_card_print_);
  verticalLayout_pbc_sel->addLayout(horizontalLayout_pbc_bottom);
  person_barcode_stack_->addWidget(person_barcode_sel_page_);
  person_barcode_print_page_ = new QWidget();
  person_barcode_stack_->addWidget(person_barcode_print_page_);
  verticalLayout_24->addWidget(person_barcode_stack_);

  person_barcode_back_->setText(tr("back"));
  label_85->setText(tr("Print Library Cards"));
  label_pbc_search->setText(tr("name/barcode:"));
  label_pbc_class->setText(tr("Class:"));
  label_pbc_class->setBuddy(person_barcode_class_);
  person_barcode_select_all_->setText(tr("Select all"));
  person_barcode_deselect_all_->setText(tr("Deselect all"));
  person_barcode_recent_check_->setText(tr("Only the last"));
#if QT_CONFIG(tooltip)
  person_barcode_recent_check_->setToolTip(
      tr("Show only the most recently created persons"));
  person_barcode_recent_count_->setToolTip(
      tr("Number of most recently created persons to show"));
#endif // QT_CONFIG(tooltip)
  person_barcode_recent_count_->setSuffix(tr(" created persons"));
  person_barcode_show_border_->setText(tr("Show border"));
  QTreeWidgetItem* ___qtreewidgetitem = person_barcode_tree_->headerItem();
  ___qtreewidgetitem->setText(1, tr("First name"));
  ___qtreewidgetitem->setText(0, tr("Last name"));
  ___qtreewidgetitem->setText(2, tr("Class"));
  person_barcode_selection_count_->setText(tr("No persons selected"));
  person_card_print_->setText(tr("Print library cards"));
#if QT_CONFIG(tooltip)
  person_card_print_->setToolTip(tr("Print library cards for the selected persons"));
#endif // QT_CONFIG(tooltip)

  person_barcode_stack_->setCurrentIndex(0);
}

void colman::MainWindowUi::build_item_barcode_page(QWidget* page)
{
  auto* verticalLayout_38 = new QVBoxLayout(page);
  item_barcode_back_ = new QPushButton(page);
  set_size_policy(item_barcode_back_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
  verticalLayout_38->addWidget(item_barcode_back_);
  horizontalLayout_ibc_top = new QHBoxLayout();
  item_barcode_logo_ = new QLabel(page);
  item_barcode_logo_->setMinimumSize(QSize(48, 48));
  item_barcode_logo_->setMaximumSize(QSize(48, 48));
  item_barcode_logo_->setScaledContents(true);
  horizontalLayout_ibc_top->addWidget(item_barcode_logo_);
  verticalLayout_38->addLayout(horizontalLayout_ibc_top);
  auto* horizontalLayout_ibc_search = new QHBoxLayout();
  auto* label_ibc_search = new QLabel(page);
  horizontalLayout_ibc_search->addWidget(label_ibc_search);
  item_barcode_search_ = new QLineEdit(page);
  item_barcode_search_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  horizontalLayout_ibc_search->addWidget(item_barcode_search_);
  item_barcode_select_all_ = new QPushButton(page);
  horizontalLayout_ibc_search->addWidget(item_barcode_select_all_);
  item_barcode_deselect_all_ = new QPushButton(page);
  horizontalLayout_ibc_search->addWidget(item_barcode_deselect_all_);
  auto* horizontalSpacer_ibc = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_ibc_search->addItem(horizontalSpacer_ibc);
  // Optional restriction to the most recently acquired items. Disabled by
  // default; the spin box stays disabled until the check box is ticked.
  item_barcode_recent_check_ = new QCheckBox(page);
  horizontalLayout_ibc_search->addWidget(item_barcode_recent_check_);
  item_barcode_recent_count_ = new QSpinBox(page);
  item_barcode_recent_count_->setRange(1, 9999);
  item_barcode_recent_count_->setValue(20);
  item_barcode_recent_count_->setEnabled(false);
  horizontalLayout_ibc_search->addWidget(item_barcode_recent_count_);
  // Optional restriction to items acquired within a time span. Mutually
  // exclusive with the most-recent restriction above; the date editors stay
  // disabled until the check box is ticked.
  item_barcode_daterange_check_ = new QCheckBox(page);
  horizontalLayout_ibc_search->addWidget(item_barcode_daterange_check_);
  item_barcode_date_from_ = new QDateEdit(page);
  item_barcode_date_from_->setCalendarPopup(true);
  item_barcode_date_from_->setDate(QDate::currentDate().addMonths(-1));
  item_barcode_date_from_->setEnabled(false);
  horizontalLayout_ibc_search->addWidget(item_barcode_date_from_);
  item_barcode_date_to_label_ = new QLabel(page);
  horizontalLayout_ibc_search->addWidget(item_barcode_date_to_label_);
  item_barcode_date_to_ = new QDateEdit(page);
  item_barcode_date_to_->setCalendarPopup(true);
  item_barcode_date_to_->setDate(QDate::currentDate());
  item_barcode_date_to_->setEnabled(false);
  horizontalLayout_ibc_search->addWidget(item_barcode_date_to_);
  // The search field and select-all/deselect-all buttons only act on the
  // item tree of the selection step, so they live inside the selection page
  // and are no longer shown on the second (layout/print) step.
  item_barcode_stack_ = new QStackedWidget(page);
  auto* item_barcode_sel_page_ = new QWidget();
  auto* verticalLayout_ibc_sel = new QVBoxLayout(item_barcode_sel_page_);
  auto* splitter_ibc = make_splitter(item_barcode_sel_page_, Qt::Orientation::Horizontal);
  item_barcode_tree_ = new QTreeWidget(splitter_ibc);
  item_barcode_tree_->setAlternatingRowColors(true);
  item_barcode_tree_->setSortingEnabled(true);
  splitter_ibc->addWidget(item_barcode_tree_);
  item_barcode_item_widget_ = new colman::ItemWidget(splitter_ibc);
  set_size_policy(item_barcode_item_widget_, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
  splitter_ibc->addWidget(item_barcode_item_widget_);
  verticalLayout_ibc_sel->addLayout(horizontalLayout_ibc_search);
  verticalLayout_ibc_sel->addWidget(splitter_ibc);
  auto* horizontalLayout_ibc_bottom = new QHBoxLayout();
  item_barcode_selection_count_ = new QLabel(item_barcode_sel_page_);
  horizontalLayout_ibc_bottom->addWidget(item_barcode_selection_count_);
  item_barcode_visible_count_ = new QLabel(item_barcode_sel_page_);
  horizontalLayout_ibc_bottom->addWidget(item_barcode_visible_count_);
  auto* spacerItem = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_ibc_bottom->addItem(spacerItem);
  item_card_print_ = new QPushButton(item_barcode_sel_page_);
  item_card_print_->setEnabled(false);
  horizontalLayout_ibc_bottom->addWidget(item_card_print_);
  verticalLayout_ibc_sel->addLayout(horizontalLayout_ibc_bottom);
  item_barcode_stack_->addWidget(item_barcode_sel_page_);
  item_barcode_print_page_ = new QWidget();
  item_barcode_stack_->addWidget(item_barcode_print_page_);
  verticalLayout_38->addWidget(item_barcode_stack_);

  item_barcode_back_->setText(tr("back"));
  label_ibc_search->setText(tr("barcode/ISBN/name:"));
  item_barcode_select_all_->setText(tr("Select all"));
  item_barcode_deselect_all_->setText(tr("Deselect all"));
  item_barcode_recent_check_->setText(tr("Only the last"));
#if QT_CONFIG(tooltip)
  item_barcode_recent_check_->setToolTip(
      tr("Show only the most recently acquired items"));
  item_barcode_recent_count_->setToolTip(
      tr("Number of most recently acquired items to show"));
#endif // QT_CONFIG(tooltip)
  item_barcode_recent_count_->setSuffix(tr(" acquired items"));
  item_barcode_daterange_check_->setText(tr("Acquired between"));
  item_barcode_date_to_label_->setText(tr("to"));
#if QT_CONFIG(tooltip)
  item_barcode_daterange_check_->setToolTip(
      tr("Show only items acquired within the given time span"));
  item_barcode_date_from_->setToolTip(tr("Start of the acquisition time span"));
  item_barcode_date_to_->setToolTip(tr("End of the acquisition time span"));
#endif // QT_CONFIG(tooltip)
  item_barcode_date_from_->setDisplayFormat(tr("yyyy-MM-dd"));
  item_barcode_date_to_->setDisplayFormat(tr("yyyy-MM-dd"));
  QTreeWidgetItem* ___qtreewidgetitem1 = item_barcode_tree_->headerItem();
  ___qtreewidgetitem1->setText(1, tr("Author"));
  ___qtreewidgetitem1->setText(0, tr("Name"));
  item_barcode_selection_count_->setText(tr("0 items selected"));
  item_card_print_->setText(tr("Print labels"));
#if QT_CONFIG(tooltip)
  item_card_print_->setToolTip(tr("Print labels using the saved item card layout"));
#endif // QT_CONFIG(tooltip)

  item_barcode_stack_->setCurrentIndex(0);
}

void colman::MainWindowUi::build_person_view_page(QWidget* page)
{
  auto* verticalLayout_33 = new QVBoxLayout(page);
  person_view_back_ = new QPushButton(page);
  verticalLayout_33->addWidget(person_view_back_);
  auto* label_96 = new colman::ScalingLabel(page);
  set_size_policy(label_96, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
  label_96->setFont(bold_font());
  verticalLayout_33->addWidget(label_96);
  auto* splitter_12 = make_splitter(page, Qt::Orientation::Horizontal);
  auto* layoutWidget_19 = new QWidget(splitter_12);
  auto* verticalLayout_31 = new QVBoxLayout(layoutWidget_19);
  verticalLayout_31->setContentsMargins(0, 0, 0, 0);
  person_view_with_items_ = new QCheckBox(layoutWidget_19);
  person_view_with_items_->setChecked(true);
  verticalLayout_31->addWidget(person_view_with_items_);
  person_view_only_selected_ = new QCheckBox(layoutWidget_19);
  verticalLayout_31->addWidget(person_view_only_selected_);
  person_view_generate_ = new QPushButton(layoutWidget_19);
  verticalLayout_31->addWidget(person_view_generate_);
  person_view_person_list_ = new colman::PersonListWidget(layoutWidget_19);
  set_size_policy(person_view_person_list_, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  verticalLayout_31->addWidget(person_view_person_list_);
  auto* horizontalLayout_28 = new QHBoxLayout();
  auto* person_view_deselect_all_ = new QPushButton(layoutWidget_19);
  horizontalLayout_28->addWidget(person_view_deselect_all_);
  auto* person_view_select_all_ = new QPushButton(layoutWidget_19);
  horizontalLayout_28->addWidget(person_view_select_all_);
  auto* horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_28->addItem(horizontalSpacer_10);
  verticalLayout_31->addLayout(horizontalLayout_28);
  splitter_12->addWidget(layoutWidget_19);
  auto* layoutWidget_20 = new QWidget(splitter_12);
  auto* verticalLayout_32 = new QVBoxLayout(layoutWidget_20);
  verticalLayout_32->setContentsMargins(0, 0, 0, 0);
  person_view_preview_ = new QTextEdit(layoutWidget_20);
  person_view_preview_->setReadOnly(true);
  verticalLayout_32->addWidget(person_view_preview_);
  auto* horizontalLayout_33 = new QHBoxLayout();
  person_view_print_ = new QPushButton(layoutWidget_20);
  horizontalLayout_33->addWidget(person_view_print_);
  verticalLayout_32->addLayout(horizontalLayout_33);
  splitter_12->addWidget(layoutWidget_20);
  verticalLayout_33->addWidget(splitter_12);

  person_view_back_->setText(tr("back"));
  label_96->setText(tr("Person based reports"));
  person_view_with_items_->setText(tr("with lend items"));
  person_view_only_selected_->setText(tr("only selected persons"));
  person_view_generate_->setText(tr("generate report"));
  person_view_deselect_all_->setText(tr("deselect all"));
  person_view_select_all_->setText(tr("select all"));
  person_view_print_->setText(tr("print"));

  QObject::connect(person_view_deselect_all_, &QPushButton::clicked, person_view_person_list_, qOverload<>(&colman::PersonListWidget::clear_selection));
  QObject::connect(person_view_select_all_, &QPushButton::clicked, person_view_person_list_, qOverload<>(&colman::PersonListWidget::select_all));
}

void colman::MainWindowUi::build_history_page(QWidget* page)
{
  auto* verticalLayout_7 = new QVBoxLayout(page);
  back_button_ = new QPushButton(page);
  verticalLayout_7->addWidget(back_button_);
  auto* label_87 = new colman::ScalingLabel(page);
  label_87->setFont(bold_font());
  verticalLayout_7->addWidget(label_87);
  auto* history_date_layout_ = new QHBoxLayout();
  history_from_label_ = new QLabel(page);
  history_date_layout_->addWidget(history_from_label_);
  history_from_date_ = new QDateEdit(page);
  history_from_date_->setCalendarPopup(true);
  history_date_layout_->addWidget(history_from_date_);
  history_to_label_ = new QLabel(page);
  history_date_layout_->addWidget(history_to_label_);
  history_to_date_ = new QDateEdit(page);
  history_to_date_->setCalendarPopup(true);
  history_date_layout_->addWidget(history_to_date_);
  history_apply_button_ = new QPushButton(page);
  history_date_layout_->addWidget(history_apply_button_);
  history_show_all_ = new QCheckBox(page);
  history_date_layout_->addWidget(history_show_all_);
  history_action_filter_ = new QComboBox(page);
  history_date_layout_->addWidget(history_action_filter_);
  auto* history_date_spacer_ = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  history_date_layout_->addItem(history_date_spacer_);
  verticalLayout_7->addLayout(history_date_layout_);
  combined_history_table_ = new QTableView(page);
  combined_history_table_->setAlternatingRowColors(true);
  combined_history_table_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  combined_history_table_->setSortingEnabled(true);
  combined_history_table_->horizontalHeader()->setCascadingSectionResizes(true);
  combined_history_table_->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));
  verticalLayout_7->addWidget(combined_history_table_);
  auto* history_action_layout_ = new QHBoxLayout();
  history_count_label_ = new QLabel(page);
  history_action_layout_->addWidget(history_count_label_);
  history_load_more_ = new QPushButton(page);
  history_load_more_->setVisible(false);
  history_action_layout_->addWidget(history_load_more_);
  auto* history_action_spacer_ = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  history_action_layout_->addItem(history_action_spacer_);
  history_details_button_ = new QPushButton(page);
  history_action_layout_->addWidget(history_details_button_);
  history_change_button_ = new QPushButton(page);
  history_action_layout_->addWidget(history_change_button_);
  history_delete_button_ = new QPushButton(page);
  history_action_layout_->addWidget(history_delete_button_);
  verticalLayout_7->addLayout(history_action_layout_);

  back_button_->setText(tr("back"));
  label_87->setText(tr("Collection History Overview"));
  history_from_label_->setText(tr("From:"));
  history_to_label_->setText(tr("To:"));
  history_apply_button_->setText(tr("Apply"));
  history_show_all_->setText(tr("show all"));
  history_action_filter_->addItem(tr("all actions"), QString());
  history_action_filter_->addItem(tr("acquisitions"),
                                  QStringLiteral("acquisition"));
  history_action_filter_->addItem(tr("liquidations"),
                                  QStringLiteral("liquidation"));
  history_action_filter_->addItem(tr("lendings"), QStringLiteral("lending"));
  history_action_filter_->addItem(tr("returns"), QStringLiteral("return"));
  history_action_filter_->addItem(tr("reservations"),
                                  QStringLiteral("reservation"));
  history_load_more_->setText(tr("Load more..."));
  history_details_button_->setText(tr("show details"));
  history_change_button_->setText(tr("change"));
  history_delete_button_->setText(tr("delete"));
}

void colman::MainWindowUi::build_settings_page(QWidget* page)
{
  auto* verticalLayout_29 = new QVBoxLayout(page);
  settings_back_ = new QPushButton(page);
  verticalLayout_29->addWidget(settings_back_);
  auto* verticalSpacer_5 = new QSpacerItem(20, 13, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_29->addItem(verticalSpacer_5);
  auto* label_79 = new colman::ScalingLabel(page);
  label_79->setFont(bold_font());
  verticalLayout_29->addWidget(label_79);
  auto* verticalSpacer_8 = new QSpacerItem(17, 13, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_29->addItem(verticalSpacer_8);
  auto* horizontalLayout_21 = new QHBoxLayout();
  auto* formLayout_3 = new QFormLayout();
  auto* label_75 = new QLabel(page);
  formLayout_3->setWidget(0, QFormLayout::ItemRole::LabelRole, label_75);
  settings_collection_name_ = new QLineEdit(page);
  formLayout_3->setWidget(0, QFormLayout::ItemRole::FieldRole, settings_collection_name_);
  auto* label_76 = new QLabel(page);
  formLayout_3->setWidget(1, QFormLayout::ItemRole::LabelRole, label_76);
  settings_organization_ = new QLineEdit(page);
  formLayout_3->setWidget(1, QFormLayout::ItemRole::FieldRole, settings_organization_);
  auto* label_77 = new QLabel(page);
  formLayout_3->setWidget(2, QFormLayout::ItemRole::LabelRole, label_77);
  settings_responsible_ = new QLineEdit(page);
  formLayout_3->setWidget(2, QFormLayout::ItemRole::FieldRole, settings_responsible_);
  auto* horizontalLayout_19 = new QHBoxLayout();
  settings_browse_logo_ = new QPushButton(page);
  settings_browse_logo_->setEnabled(false);
  set_size_policy(settings_browse_logo_, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
  horizontalLayout_19->addWidget(settings_browse_logo_);
  settings_logo_ = new QLabel(page);
  settings_logo_->setEnabled(false);
  set_size_policy(settings_logo_, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
  horizontalLayout_19->addWidget(settings_logo_);
  formLayout_3->setLayout(3, QFormLayout::ItemRole::FieldRole, horizontalLayout_19);
  settings_use_logo_ = new QCheckBox(page);
  formLayout_3->setWidget(3, QFormLayout::ItemRole::LabelRole, settings_use_logo_);
  horizontalLayout_21->addLayout(formLayout_3);
  settings_logo_pixmap_ = new colman::PixmapLabel(page);
  horizontalLayout_21->addWidget(settings_logo_pixmap_);
  verticalLayout_29->addLayout(horizontalLayout_21);
  auto* horizontalLayout_18 = new QHBoxLayout();
  settings_use_lending_period_ = new QCheckBox(page);
  horizontalLayout_18->addWidget(settings_use_lending_period_);
  settings_lending_period_years_ = new QSpinBox(page);
  horizontalLayout_18->addWidget(settings_lending_period_years_);
  settings_lending_period_months_ = new QSpinBox(page);
  settings_lending_period_months_->setValue(1);
  horizontalLayout_18->addWidget(settings_lending_period_months_);
  settings_lending_period_days_ = new QSpinBox(page);
  settings_lending_period_days_->setMaximum(999);
  horizontalLayout_18->addWidget(settings_lending_period_days_);
  verticalLayout_29->addLayout(horizontalLayout_18);
  auto* horizontalLayout_reservations = new QHBoxLayout();
  settings_allow_reservations_ = new QCheckBox(page);
  horizontalLayout_reservations->addWidget(settings_allow_reservations_);
  settings_reservation_period_days_ = new QSpinBox(page);
  settings_reservation_period_days_->setMinimum(1);
  settings_reservation_period_days_->setMaximum(999);
  settings_reservation_period_days_->setValue(14);
  horizontalLayout_reservations->addWidget(settings_reservation_period_days_);
  auto* horizontalSpacer_reservations = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_reservations->addItem(horizontalSpacer_reservations);
  verticalLayout_29->addLayout(horizontalLayout_reservations);
  auto* horizontalLayout_26 = new QHBoxLayout();
  auto* label_78 = new QLabel(page);
  horizontalLayout_26->addWidget(label_78);
  settings_default_type_ = new QComboBox(page);
  horizontalLayout_26->addWidget(settings_default_type_);
  auto* horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_26->addItem(horizontalSpacer_9);
  verticalLayout_29->addLayout(horizontalLayout_26);
  settings_default_is_schoolitem_ = new QCheckBox(page);
  verticalLayout_29->addWidget(settings_default_is_schoolitem_);
  auto* horizontalLayout_sig_top_cat = new QHBoxLayout();
  auto* verticalLayout_sig_top_cat_label = new QVBoxLayout();
  auto* label_sig_top_categories = new QLabel(page);
  verticalLayout_sig_top_cat_label->addWidget(label_sig_top_categories);
  settings_sig_top_categories_ = new QListWidget(page);
  verticalLayout_sig_top_cat_label->addWidget(settings_sig_top_categories_);
  horizontalLayout_sig_top_cat->addLayout(verticalLayout_sig_top_cat_label);
  auto* verticalLayout_sig_top_cat_buttons = new QVBoxLayout();
  settings_add_sig_top_cat_ = new QPushButton(page);
  verticalLayout_sig_top_cat_buttons->addWidget(settings_add_sig_top_cat_);
  settings_remove_sig_top_cat_ = new QPushButton(page);
  verticalLayout_sig_top_cat_buttons->addWidget(settings_remove_sig_top_cat_);
  auto* verticalSpacer_sig_top_cat = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_sig_top_cat_buttons->addItem(verticalSpacer_sig_top_cat);
  horizontalLayout_sig_top_cat->addLayout(verticalLayout_sig_top_cat_buttons);
  verticalLayout_29->addLayout(horizontalLayout_sig_top_cat);
  auto* horizontalLayout_sig_bottom_cat = new QHBoxLayout();
  auto* verticalLayout_sig_bottom_cat_label = new QVBoxLayout();
  auto* label_sig_bottom_categories = new QLabel(page);
  verticalLayout_sig_bottom_cat_label->addWidget(label_sig_bottom_categories);
  settings_sig_bottom_categories_ = new QListWidget(page);
  verticalLayout_sig_bottom_cat_label->addWidget(settings_sig_bottom_categories_);
  horizontalLayout_sig_bottom_cat->addLayout(verticalLayout_sig_bottom_cat_label);
  auto* verticalLayout_sig_bottom_cat_buttons = new QVBoxLayout();
  settings_add_sig_bottom_cat_ = new QPushButton(page);
  verticalLayout_sig_bottom_cat_buttons->addWidget(settings_add_sig_bottom_cat_);
  settings_remove_sig_bottom_cat_ = new QPushButton(page);
  verticalLayout_sig_bottom_cat_buttons->addWidget(settings_remove_sig_bottom_cat_);
  auto* verticalSpacer_sig_bottom_cat = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_sig_bottom_cat_buttons->addItem(verticalSpacer_sig_bottom_cat);
  horizontalLayout_sig_bottom_cat->addLayout(verticalLayout_sig_bottom_cat_buttons);
  verticalLayout_29->addLayout(horizontalLayout_sig_bottom_cat);
  auto* verticalSpacer_6 = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_29->addItem(verticalSpacer_6);
  auto* horizontalLayout_20 = new QHBoxLayout();
  settings_revert_ = new QPushButton(page);
  horizontalLayout_20->addWidget(settings_revert_);
  settings_apply_ = new QPushButton(page);
  horizontalLayout_20->addWidget(settings_apply_);
  auto* horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_20->addItem(horizontalSpacer_6);
  verticalLayout_29->addLayout(horizontalLayout_20);
  auto* verticalSpacer_7 = new QSpacerItem(17, 428, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
  verticalLayout_29->addItem(verticalSpacer_7);

  settings_back_->setText(tr("back"));
  label_79->setText(tr("Collection Settings"));
  label_75->setText(tr("collection name:"));
  label_76->setText(tr("organization:"));
  label_77->setText(tr("responsible:"));
  settings_browse_logo_->setText(tr("browse"));
  settings_use_logo_->setText(tr("logo"));
  settings_use_lending_period_->setText(tr("default lending period:"));
  settings_lending_period_years_->setSuffix(tr(" year(s)"));
  settings_lending_period_years_->setPrefix(QString());
  settings_lending_period_months_->setSuffix(tr(" month(s)"));
  settings_lending_period_days_->setSuffix(tr(" day(s)"));
  settings_allow_reservations_->setText(tr("Allow reservations"));
  settings_reservation_period_days_->setSuffix(tr(" day(s)"));
  label_78->setText(tr("Default type for new items:"));
  settings_default_is_schoolitem_->setText(tr("items are school items by default"));
  label_sig_top_categories->setText(tr("Signature top categories:"));
  settings_add_sig_top_cat_->setText(tr("add"));
  settings_remove_sig_top_cat_->setText(tr("remove"));
  label_sig_bottom_categories->setText(tr("Signature bottom categories (for selected top):"));
  settings_add_sig_bottom_cat_->setText(tr("add"));
  settings_remove_sig_bottom_cat_->setText(tr("remove"));
  settings_revert_->setText(tr("revert"));
  settings_apply_->setText(tr("apply"));

  label_75->setBuddy(settings_collection_name_);
  label_76->setBuddy(settings_organization_);
  label_77->setBuddy(settings_responsible_);

  QObject::connect(settings_use_lending_period_, &QCheckBox::toggled, settings_lending_period_years_, &QSpinBox::setEnabled);
  QObject::connect(settings_use_lending_period_, &QCheckBox::toggled, settings_lending_period_months_, &QSpinBox::setEnabled);
  QObject::connect(settings_use_lending_period_, &QCheckBox::toggled, settings_lending_period_days_, &QSpinBox::setEnabled);
  QObject::connect(settings_allow_reservations_, &QCheckBox::toggled, settings_reservation_period_days_, &QSpinBox::setEnabled);
  QObject::connect(settings_use_logo_, &QCheckBox::toggled, settings_logo_, &QLabel::setEnabled);
  QObject::connect(settings_use_logo_, &QCheckBox::toggled, settings_browse_logo_, &QPushButton::setEnabled);
  QObject::connect(settings_use_logo_, SIGNAL(toggled(bool)), settings_logo_pixmap_, SLOT(setVisible(bool)));
}

void colman::MainWindowUi::build_preferences_page(QWidget* page)
{
  auto* verticalLayout_30 = new QVBoxLayout(page);
  auto* preferences_scroll_area_ = new QScrollArea(page);
  preferences_scroll_area_->setWidgetResizable(true);
  preferences_scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  auto* preferences_scroll_contents_ = new QWidget();
  auto* verticalLayout_prefs_content = new QVBoxLayout(preferences_scroll_contents_);
  preferences_back_ = new QPushButton(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(preferences_back_);
  auto* verticalSpacer_9 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_prefs_content->addItem(verticalSpacer_9);
  auto* label_80 = new colman::ScalingLabel(preferences_scroll_contents_);
  label_80->setFont(bold_font());
  verticalLayout_prefs_content->addWidget(label_80);
  auto* verticalSpacer_10 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_prefs_content->addItem(verticalSpacer_10);
  auto* horizontalLayout_22 = new QHBoxLayout();
  auto* label_81 = new QLabel(preferences_scroll_contents_);
  horizontalLayout_22->addWidget(label_81);
  pref_thumbnail_dir_ = new QLineEdit(preferences_scroll_contents_);
  horizontalLayout_22->addWidget(pref_thumbnail_dir_);
  pref_thumbnail_dir_browse_ = new QPushButton(preferences_scroll_contents_);
  horizontalLayout_22->addWidget(pref_thumbnail_dir_browse_);
  verticalLayout_prefs_content->addLayout(horizontalLayout_22);
  auto* verticalSpacer_13 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_prefs_content->addItem(verticalSpacer_13);
  auto* label_94 = new QLabel(preferences_scroll_contents_);
  label_94->setWordWrap(true);
  verticalLayout_prefs_content->addWidget(label_94);
  pref_search_openlibrary_ = new QCheckBox(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(pref_search_openlibrary_);
  pref_search_googleapis_ = new QCheckBox(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(pref_search_googleapis_);
  auto* label_sru_servers_ = new QLabel(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(label_sru_servers_);
  pref_sru_servers_ = new QTableWidget(preferences_scroll_contents_);
  if(pref_sru_servers_->columnCount() < 7) {
    pref_sru_servers_->setColumnCount(7);
  }
  QTableWidgetItem* __qtablewidgetitem8 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(0, __qtablewidgetitem8);
  QTableWidgetItem* __qtablewidgetitem9 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(1, __qtablewidgetitem9);
  QTableWidgetItem* __qtablewidgetitem10 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(2, __qtablewidgetitem10);
  QTableWidgetItem* __qtablewidgetitem11 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(3, __qtablewidgetitem11);
  QTableWidgetItem* __qtablewidgetitem12 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(4, __qtablewidgetitem12);
  QTableWidgetItem* __qtablewidgetitem13 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(5, __qtablewidgetitem13);
  QTableWidgetItem* __qtablewidgetitem14 = new QTableWidgetItem();
  pref_sru_servers_->setHorizontalHeaderItem(6, __qtablewidgetitem14);
  pref_sru_servers_->setMinimumSize(QSize(0, 120));
  verticalLayout_prefs_content->addWidget(pref_sru_servers_);
  auto* horizontalLayout_sru_buttons = new QHBoxLayout();
  pref_sru_add_ = new QPushButton(preferences_scroll_contents_);
  horizontalLayout_sru_buttons->addWidget(pref_sru_add_);
  pref_sru_remove_ = new QPushButton(preferences_scroll_contents_);
  horizontalLayout_sru_buttons->addWidget(pref_sru_remove_);
  auto* horizontalSpacer_sru = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_sru_buttons->addItem(horizontalSpacer_sru);
  verticalLayout_prefs_content->addLayout(horizontalLayout_sru_buttons);
  pref_search_musicbrainz_ = new QCheckBox(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(pref_search_musicbrainz_);
  pref_search_bgg_ = new QCheckBox(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(pref_search_bgg_);
  auto* horizontalLayout_bgg_token = new QHBoxLayout();
  auto* label_bgg_token_ = new QLabel(preferences_scroll_contents_);
  horizontalLayout_bgg_token->addWidget(label_bgg_token_);
  pref_bgg_token_ = new QLineEdit(preferences_scroll_contents_);
  horizontalLayout_bgg_token->addWidget(pref_bgg_token_);
  verticalLayout_prefs_content->addLayout(horizontalLayout_bgg_token);
  pref_bgg_logo_ = new QLabel(preferences_scroll_contents_);
  pref_bgg_logo_->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
  verticalLayout_prefs_content->addWidget(pref_bgg_logo_);
  pref_search_opengtindb_ = new QCheckBox(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(pref_search_opengtindb_);
  auto* horizontalLayout_opengtindb_userid = new QHBoxLayout();
  auto* label_opengtindb_userid_ = new QLabel(preferences_scroll_contents_);
  horizontalLayout_opengtindb_userid->addWidget(label_opengtindb_userid_);
  pref_opengtindb_userid_ = new QLineEdit(preferences_scroll_contents_);
  horizontalLayout_opengtindb_userid->addWidget(pref_opengtindb_userid_);
  verticalLayout_prefs_content->addLayout(horizontalLayout_opengtindb_userid);
  auto* horizontalLayout_proxy_user = new QHBoxLayout();
  auto* label_proxy_user_ = new QLabel(preferences_scroll_contents_);
  horizontalLayout_proxy_user->addWidget(label_proxy_user_);
  pref_proxy_user_ = new QLineEdit(preferences_scroll_contents_);
  horizontalLayout_proxy_user->addWidget(pref_proxy_user_);
  verticalLayout_prefs_content->addLayout(horizontalLayout_proxy_user);
  auto* horizontalLayout_proxy_password = new QHBoxLayout();
  auto* label_proxy_password_ = new QLabel(preferences_scroll_contents_);
  horizontalLayout_proxy_password->addWidget(label_proxy_password_);
  pref_proxy_password_ = new QLineEdit(preferences_scroll_contents_);
  pref_proxy_password_->setEchoMode(QLineEdit::Password);
  horizontalLayout_proxy_password->addWidget(pref_proxy_password_);
  verticalLayout_prefs_content->addLayout(horizontalLayout_proxy_password);
  pref_download_thumbnails_ = new QCheckBox(preferences_scroll_contents_);
  verticalLayout_prefs_content->addWidget(pref_download_thumbnails_);
  auto* horizontalLayout_25 = new QHBoxLayout();
  auto* label_92 = new QLabel(preferences_scroll_contents_);
  horizontalLayout_25->addWidget(label_92);
  pref_timeout_ = new QSpinBox(preferences_scroll_contents_);
  pref_timeout_->setMinimum(1);
  pref_timeout_->setMaximum(99);
  pref_timeout_->setValue(5);
  horizontalLayout_25->addWidget(pref_timeout_);
  auto* horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_25->addItem(horizontalSpacer_7);
  verticalLayout_prefs_content->addLayout(horizontalLayout_25);
  auto* horizontalLayout_146 = new QHBoxLayout();
  auto* label_508 = new QLabel(preferences_scroll_contents_);
  horizontalLayout_146->addWidget(label_508);
  pref_poll_interval_ = new QSpinBox(preferences_scroll_contents_);
  pref_poll_interval_->setMinimum(5);
  pref_poll_interval_->setMaximum(300);
  pref_poll_interval_->setSingleStep(5);
  pref_poll_interval_->setValue(10);
  horizontalLayout_146->addWidget(pref_poll_interval_);
  auto* horizontalSpacer_60 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_146->addItem(horizontalSpacer_60);
  verticalLayout_prefs_content->addLayout(horizontalLayout_146);
  auto* horizontalLayout_search_threshold = new QHBoxLayout();
  auto* label_search_threshold = new QLabel(preferences_scroll_contents_);
  horizontalLayout_search_threshold->addWidget(label_search_threshold);
  pref_search_threshold_ = new QSpinBox(preferences_scroll_contents_);
  pref_search_threshold_->setMinimum(0);
  pref_search_threshold_->setMaximum(999999);
  pref_search_threshold_->setValue(1000);
  horizontalLayout_search_threshold->addWidget(pref_search_threshold_);
  auto* label_search_threshold_hint = new QLabel(preferences_scroll_contents_);
  horizontalLayout_search_threshold->addWidget(label_search_threshold_hint);
  auto* horizontalSpacer_search_threshold = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_search_threshold->addItem(horizontalSpacer_search_threshold);
  verticalLayout_prefs_content->addLayout(horizontalLayout_search_threshold);
  auto* horizontalLayout_barcode_unit = new QHBoxLayout();
  auto* label_barcode_unit = new QLabel(preferences_scroll_contents_);
  horizontalLayout_barcode_unit->addWidget(label_barcode_unit);
  pref_barcode_unit_ = new QDoubleSpinBox(preferences_scroll_contents_);
  pref_barcode_unit_->setDecimals(2);
  pref_barcode_unit_->setMinimum(0.15);
  pref_barcode_unit_->setMaximum(1.00);
  pref_barcode_unit_->setSingleStep(0.01);
  pref_barcode_unit_->setValue(colman::DefaultBarcodeUnitMm);
  horizontalLayout_barcode_unit->addWidget(pref_barcode_unit_);
  auto* label_barcode_unit_hint = new QLabel(preferences_scroll_contents_);
  horizontalLayout_barcode_unit->addWidget(label_barcode_unit_hint);
  auto* horizontalSpacer_barcode_unit = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_barcode_unit->addItem(horizontalSpacer_barcode_unit);
  verticalLayout_prefs_content->addLayout(horizontalLayout_barcode_unit);
  auto* formLayout_4 = new QFormLayout();
  auto* label_82 = new QLabel(preferences_scroll_contents_);
  formLayout_4->setWidget(0, QFormLayout::ItemRole::LabelRole, label_82);
  pref_googleapis_key_ = new QLineEdit(preferences_scroll_contents_);
  formLayout_4->setWidget(0, QFormLayout::ItemRole::FieldRole, pref_googleapis_key_);
  verticalLayout_prefs_content->addLayout(formLayout_4);
  auto* verticalSpacer_11 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
  verticalLayout_prefs_content->addItem(verticalSpacer_11);
  auto* horizontalLayout_23 = new QHBoxLayout();
  pref_revert_ = new QPushButton(preferences_scroll_contents_);
  horizontalLayout_23->addWidget(pref_revert_);
  pref_apply_ = new QPushButton(preferences_scroll_contents_);
  horizontalLayout_23->addWidget(pref_apply_);
  auto* horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  horizontalLayout_23->addItem(horizontalSpacer_8);
  verticalLayout_prefs_content->addLayout(horizontalLayout_23);
  preferences_scroll_area_->setWidget(preferences_scroll_contents_);
  verticalLayout_30->addWidget(preferences_scroll_area_);

  preferences_back_->setText(tr("back"));
  label_80->setText(tr("Application Preferences"));
#if QT_CONFIG(tooltip)
  label_81->setToolTip(tr("where to store item thumbnails"));
#endif // QT_CONFIG(tooltip)
  label_81->setText(tr("Thumbnail directory:"));
  pref_thumbnail_dir_browse_->setText(tr("browse"));
  label_94->setText(tr("Colman can try to search book and media infos and thumbnails on several web servers. However, to protect your privacy, Colman will not connect to these servers without your explicit permission, which you can give below."));
  pref_search_openlibrary_->setText(tr("search ISBN on openlibrary.org"));
  pref_search_googleapis_->setText(tr("search ISBN on www.googleapis.com"));
  label_sru_servers_->setText(tr("SRU library catalogue servers:"));
  QTableWidgetItem* ___qtablewidgetitem8 = pref_sru_servers_->horizontalHeaderItem(0);
  ___qtablewidgetitem8->setText(tr("Enable"));
  QTableWidgetItem* ___qtablewidgetitem9 = pref_sru_servers_->horizontalHeaderItem(1);
  ___qtablewidgetitem9->setText(tr("Name"));
  QTableWidgetItem* ___qtablewidgetitem10 = pref_sru_servers_->horizontalHeaderItem(2);
  ___qtablewidgetitem10->setText(tr("Endpoint"));
  QTableWidgetItem* ___qtablewidgetitem11 = pref_sru_servers_->horizontalHeaderItem(3);
  ___qtablewidgetitem11->setText(tr("ISBN field"));
  QTableWidgetItem* ___qtablewidgetitem12 = pref_sru_servers_->horizontalHeaderItem(4);
  ___qtablewidgetitem12->setText(tr("EAN field"));
  QTableWidgetItem* ___qtablewidgetitem13 = pref_sru_servers_->horizontalHeaderItem(5);
  ___qtablewidgetitem13->setText(tr("Schema"));
  QTableWidgetItem* ___qtablewidgetitem14 = pref_sru_servers_->horizontalHeaderItem(6);
  ___qtablewidgetitem14->setText(tr("Title field"));
  pref_sru_add_->setText(tr("Add server"));
  pref_sru_remove_->setText(tr("Remove selected"));
  pref_search_musicbrainz_->setText(tr("search product barcode on MusicBrainz"));
  pref_search_bgg_->setText(tr("search board games on boardgamegeek.com"));
  label_bgg_token_->setText(tr("boardgamegeek.com API token:"));
  pref_bgg_token_->setPlaceholderText(tr("Bearer token (from boardgamegeek.com)"));
  pref_search_opengtindb_->setText(tr("look up EAN on opengtindb.org (games, DVDs, Blu-rays, \342\200\246)"));
  label_opengtindb_userid_->setText(tr("opengtindb.org user ID:"));
  pref_opengtindb_userid_->setPlaceholderText(tr("user ID (from opengtindb.org)"));
  label_proxy_user_->setText(tr("proxy user name:"));
  pref_proxy_user_->setPlaceholderText(tr("user name (if proxy requires authentication)"));
  label_proxy_password_->setText(tr("proxy password:"));
  pref_proxy_password_->setPlaceholderText(tr("password (if proxy requires authentication)"));
  pref_download_thumbnails_->setText(tr("try to download thumbnails"));
  label_92->setText(tr("internet lookup timeout:"));
  pref_timeout_->setSuffix(tr(" second(s)"));
  label_508->setText(tr("poll interval (REST client):"));
  pref_poll_interval_->setSuffix(tr(" second(s)"));
  label_search_threshold->setText(tr("Search threshold:"));
  pref_search_threshold_->setSuffix(tr(" items"));
  label_search_threshold_hint->setText(tr("(0 = always show all)"));
  label_barcode_unit->setText(tr("Barcode bar width:"));
  pref_barcode_unit_->setSuffix(tr(" mm"));
#if QT_CONFIG(tooltip)
  pref_barcode_unit_->setToolTip(tr("Width of the narrowest barcode bar (X-dimension). 0.33 mm is the recommended value for reliable scanning."));
#endif // QT_CONFIG(tooltip)
  label_barcode_unit_hint->setText(tr("(narrowest bar; 0.33 mm recommended)"));
  label_82->setText(tr("Google Books API key (optional):"));
  pref_revert_->setText(tr("revert"));
  pref_apply_->setText(tr("apply"));

  label_81->setBuddy(pref_thumbnail_dir_);
  label_82->setBuddy(pref_googleapis_key_);
}

/* EOF */
