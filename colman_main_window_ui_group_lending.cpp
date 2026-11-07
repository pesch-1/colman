// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Group lending pages of struct MainWindowUi.
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

namespace
{
  char constexpr GroupLendingIcon[] = ":/icons/owls/lending_group.jpeg";

  /// Adds the icon + bold heading row to \a layout.
  /**
     @param page   the page owning the widgets
     @param layout the page's main layout
     @param text   heading text
     @return the heading label
  */
  colman::ScalingLabel* add_heading(QWidget* page, QVBoxLayout* layout,
                                    QString const& text)
  {
    auto* row = new QHBoxLayout();
    row->addWidget(colman::make_icon_label(page, GroupLendingIcon));
    auto* heading = new colman::ScalingLabel(page);
    heading->setFont(colman::bold_font());
    heading->setText(text);
    row->addWidget(heading);
    layout->addLayout(row);
    return heading;
  }


  /// Adds a right-aligned button row with \a button to \a layout.
  void add_button_row(QVBoxLayout* layout, QWidget* left, QPushButton* button)
  {
    auto* row = new QHBoxLayout();
    if(left != nullptr) {
      row->addWidget(left);
    }
    row->addItem(new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding,
                                 QSizePolicy::Policy::Minimum));
    row->addWidget(button);
    layout->addLayout(row);
  }

} // namespace


void colman::MainWindowUi::build_group_lending_item(QWidget* page)
{
  auto* layout = new QVBoxLayout(page);
  group_lending_item_back_ = new QPushButton(tr("back"), page);
  layout->addWidget(group_lending_item_back_);
  add_heading(page, layout,
              tr("Group lending, step 1: Please select the item to hand "
                 "out!"));
  auto* search_row = new QHBoxLayout();
  auto* search_label = new QLabel(tr("barcode/name:"), page);
  search_row->addWidget(search_label);
  group_lending_item_barcode_ = new QLineEdit(page);
  search_label->setBuddy(group_lending_item_barcode_);
  search_row->addWidget(group_lending_item_barcode_);
  group_lending_item_camera_ = new QToolButton(page);
#if QT_CONFIG(tooltip)
  group_lending_item_camera_->setToolTip(tr("Scan barcode with camera"));
#endif // QT_CONFIG(tooltip)
  search_row->addWidget(group_lending_item_camera_);
  layout->addLayout(search_row);
  auto* splitter = new QSplitter(page);
  set_size_policy(splitter, QSizePolicy::Policy::Expanding,
                  QSizePolicy::Policy::Expanding);
  splitter->setOrientation(Qt::Orientation::Horizontal);
  splitter->setHandleWidth(5);
  group_lending_item_view_ = new QTableView(splitter);
  group_lending_item_view_->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  group_lending_item_view_->setSelectionBehavior(
      QAbstractItemView::SelectionBehavior::SelectRows);
  splitter->addWidget(group_lending_item_view_);
  group_lending_item_widget_ = new colman::ItemWidget(splitter);
  set_size_policy(group_lending_item_widget_, QSizePolicy::Policy::Expanding,
                  QSizePolicy::Policy::Expanding);
  splitter->addWidget(group_lending_item_widget_);
  layout->addWidget(splitter);
  auto* bottom = new QHBoxLayout();
  group_lending_item_count_ = new QLabel(page);
  bottom->addWidget(group_lending_item_count_);
  group_lending_item_load_more_ = new QPushButton(tr("Load more..."), page);
  group_lending_item_load_more_->setVisible(false);
  bottom->addWidget(group_lending_item_load_more_);
  bottom->addItem(new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding,
                                  QSizePolicy::Policy::Minimum));
  group_lending_item_next_ = new QPushButton(tr("next"), page);
  bottom->addWidget(group_lending_item_next_);
  layout->addLayout(bottom);
}


void colman::MainWindowUi::build_group_lending_persons(QWidget* page)
{
  auto* layout = new QVBoxLayout(page);
  group_lending_persons_back_ = new QPushButton(tr("back"), page);
  layout->addWidget(group_lending_persons_back_);
  add_heading(page, layout,
              tr("Group lending, step 2: Please tick the borrowers! "
                 "Each one gets one copy."));
  group_lending_persons_item_name_ = new QLabel(page);
  group_lending_persons_item_name_->setFont(bold_font());
  layout->addWidget(group_lending_persons_item_name_);
  group_lending_persons_list_ = new colman::PersonCheckListWidget(page);
  set_size_policy(group_lending_persons_list_,
                  QSizePolicy::Policy::Expanding,
                  QSizePolicy::Policy::Expanding);
  layout->addWidget(group_lending_persons_list_);
  group_lending_persons_message_ = new QLabel(page);
  group_lending_persons_message_->setWordWrap(true);
  layout->addWidget(group_lending_persons_message_);
  group_lending_persons_status_ = new QLabel(page);
  group_lending_persons_next_ = new QPushButton(tr("next"), page);
  add_button_row(layout, group_lending_persons_status_,
                 group_lending_persons_next_);
}


void colman::MainWindowUi::build_group_lending_details(QWidget* page)
{
  auto* layout = new QVBoxLayout(page);
  group_lending_details_back_ = new QPushButton(tr("back"), page);
  layout->addWidget(group_lending_details_back_);
  add_heading(page, layout,
              tr("Group lending, step 3: Please set the due date and "
                 "optionally add comments!"));
  auto* form = new QFormLayout();
  form->setFieldGrowthPolicy(
      QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);
  group_lending_details_item_name_ = new QLineEdit(page);
  group_lending_details_item_name_->setReadOnly(true);
  form->addRow(tr("Item:"), group_lending_details_item_name_);
  group_lending_details_persons_ = new QLineEdit(page);
  group_lending_details_persons_->setReadOnly(true);
  form->addRow(tr("Borrowers:"), group_lending_details_persons_);
  layout->addLayout(form);
  auto* due_row = new QHBoxLayout();
  group_lending_details_due_date_check_ =
      new QCheckBox(tr("with duedate"), page);
  due_row->addWidget(group_lending_details_due_date_check_);
  group_lending_details_due_date_ = new QDateEdit(page);
  group_lending_details_due_date_->setDisplayFormat(tr("yyyy-MM-dd"));
  group_lending_details_due_date_->setCalendarPopup(true);
  group_lending_details_due_date_->setEnabled(false);
  due_row->addWidget(group_lending_details_due_date_);
  due_row->addItem(new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding,
                                   QSizePolicy::Policy::Minimum));
  layout->addLayout(due_row);
  layout->addWidget(
      new QLabel(tr("Do you have some remarks for these lendings?"), page));
  group_lending_details_remarks_ = new QPlainTextEdit(page);
  set_size_policy(group_lending_details_remarks_,
                  QSizePolicy::Policy::Expanding,
                  QSizePolicy::Policy::Expanding, 0, 2);
  layout->addWidget(group_lending_details_remarks_);
  group_lending_details_next_ = new QPushButton(tr("next"), page);
  add_button_row(layout, nullptr, group_lending_details_next_);

  QObject::connect(group_lending_details_due_date_check_,
                   &QCheckBox::toggled, group_lending_details_due_date_,
                   &QDateEdit::setEnabled);
}


void colman::MainWindowUi::build_group_lending_confirm(QWidget* page)
{
  auto* layout = new QVBoxLayout(page);
  group_lending_confirm_back_ = new QPushButton(tr("back"), page);
  layout->addWidget(group_lending_confirm_back_);
  add_heading(page, layout,
              tr("Group lending, step 4: Please check your lending data!"));
  auto* form = new QFormLayout();
  group_lending_confirm_item_ = new QLineEdit(page);
  group_lending_confirm_item_->setReadOnly(true);
  form->addRow(tr("item:"), group_lending_confirm_item_);
  group_lending_confirm_number_ = new QLineEdit(page);
  group_lending_confirm_number_->setReadOnly(true);
  form->addRow(tr("number:"), group_lending_confirm_number_);
  group_lending_confirm_due_date_ = new QLineEdit(page);
  group_lending_confirm_due_date_->setReadOnly(true);
  form->addRow(tr("due date:"), group_lending_confirm_due_date_);
  group_lending_confirm_remarks_ = new QPlainTextEdit(page);
  group_lending_confirm_remarks_->setReadOnly(true);
  group_lending_confirm_remarks_->setMaximumHeight(80);
  form->addRow(tr("remarks:"), group_lending_confirm_remarks_);
  group_lending_confirm_persons_ = new QListWidget(page);
  group_lending_confirm_persons_->setSelectionMode(
      QAbstractItemView::SelectionMode::NoSelection);
  form->addRow(tr("borrowers:"), group_lending_confirm_persons_);
  layout->addLayout(form, 1);
  layout->addWidget(new QLabel(
      tr("If the above values are correct, please click on \"finish\"!"),
      page));
  group_lending_confirm_finish_ = new QPushButton(tr("finish"), page);
  add_button_row(layout, nullptr, group_lending_confirm_finish_);
}


void colman::MainWindowUi::build_group_lending_done(QWidget* page)
{
  auto* layout = new QVBoxLayout(page);
  group_lending_done_back_ = new QPushButton(tr("back"), page);
  layout->addWidget(group_lending_done_back_);
  layout->addItem(new QSpacerItem(20, 300, QSizePolicy::Policy::Minimum,
                                  QSizePolicy::Policy::Expanding));
  auto* icon_row = new QHBoxLayout();
  icon_row->addStretch();
  icon_row->addWidget(make_icon_label(page, GroupLendingIcon));
  icon_row->addStretch();
  layout->addLayout(icon_row);
  group_lending_done_message_ = new QLabel(page);
  group_lending_done_message_->setAlignment(Qt::AlignmentFlag::AlignCenter);
  group_lending_done_message_->setWordWrap(true);
  layout->addWidget(group_lending_done_message_);
  layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum,
                                  QSizePolicy::Policy::Fixed));
  auto* buttons = new QHBoxLayout();
  buttons->addStretch();
  group_lending_done_print_ =
      new QPushButton(tr("Print hand-out list..."), page);
  buttons->addWidget(group_lending_done_print_);
  group_lending_done_again_ =
      new QPushButton(tr("Further group lending"), page);
  buttons->addWidget(group_lending_done_again_);
  group_lending_done_close_ = new QPushButton(tr("done"), page);
  buttons->addWidget(group_lending_done_close_);
  buttons->addStretch();
  layout->addLayout(buttons);
  layout->addItem(new QSpacerItem(20, 400, QSizePolicy::Policy::Minimum,
                                  QSizePolicy::Policy::Expanding));
}

/* EOF */
