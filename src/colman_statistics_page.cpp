// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of the statistics page.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_statistics_page.hpp"

#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef COLMAN_TRY_CATCH_HPP
#  include "colman_try_catch.hpp"
#endif

#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QDATEEDIT
#  include <QDateEdit>
#  define SYSTEM_QDATEEDIT
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_FUNCTIONAL
#  include <functional>
#  define SYSTEM_FUNCTIONAL
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif

namespace
{
  /// One table row: item type name and its figures.
  struct StatisticsRow
  {
    QString name_;           ///< item type name, or "Total"
    std::vector<int> values_; ///< one figure per numeric column
  };

  /// Creates a read-only table with the given column headers.
  /**
     @param headers column headers; the first is the item type column
     @param name    object name of the table
     @param parent  parent widget
     @return the new table
  */
  QTableWidget* make_table(QStringList const& headers, QString const& name,
                           QWidget* parent)
  {
    auto* table = new QTableWidget(0, headers.size(), parent);
    table->setObjectName(name);
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    // Columns keep their natural width so the figures stay next to their
    // item type; the remaining width stays empty.
    table->horizontalHeader()->setStretchLastSection(false);
    return table;
  }

  /// Fills \a table with \a rows and, for several rows, a total row.
  /**
     The figures are right-aligned; the total row is printed bold.
     @param table table to fill
     @param rows  rows to show
  */
  void fill_table(QTableWidget* table, std::vector<StatisticsRow> rows)
  {
    if(rows.size() > 1) {
      StatisticsRow total;
      total.name_ = QApplication::tr("Total");
      total.values_.assign(rows.front().values_.size(), 0);
      for(auto const& row : rows) {
        for(std::size_t i = 0; i < row.values_.size(); ++i) {
          total.values_[i] += row.values_[i];
        }
      }
      rows.push_back(std::move(total));
    }
    table->setRowCount(static_cast<int>(rows.size()));
    for(std::size_t r = 0; r < rows.size(); ++r) {
      bool const is_total = rows.size() > 1 && r + 1 == rows.size();
      QFont font = table->font();
      font.setBold(is_total);
      auto* name_item = new QTableWidgetItem(rows[r].name_);
      name_item->setFont(font);
      table->setItem(static_cast<int>(r), 0, name_item);
      for(std::size_t c = 0; c < rows[r].values_.size(); ++c) {
        auto* item = new QTableWidgetItem(
            QLocale().toString(rows[r].values_[c]));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        item->setFont(font);
        table->setItem(static_cast<int>(r), static_cast<int>(c) + 1, item);
      }
    }
  }

  /// Page with stock, circulation and acquisition figures per item type.
  /**
     The upper table shows the current stock (items and copies) and how
     many copies are lent, reserved and overdue.  The lower table shows how
     many items and copies were acquired and liquidated in a time span the
     user chooses (default: the last year).  With several item types, both
     tables end with a total row.
  */
  class StatisticsPage : public QWidget
  {
  public:
    /// Constructs the page.
    /**
       @param data    central application state
       @param on_back called when the Back button is pressed
       @param parent  optional parent widget
    */
    StatisticsPage(colman::AppData& data, std::function<void()> on_back,
                   QWidget* parent = nullptr)
        : QWidget(parent), data_(data)
    {
      auto* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);

      auto* back_btn = new QPushButton(QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      auto* hdr_hbox = new QHBoxLayout;
      hdr_hbox->setContentsMargins(4, 4, 4, 4);
      auto* owl_lbl = new QLabel(this);
      QPixmap const pm(":/icons/owls/statistics.jpeg");
      if(!pm.isNull()) {
        owl_lbl->setPixmap(pm.scaledToHeight(64, Qt::SmoothTransformation));
        owl_lbl->setFixedHeight(64);
      }
      hdr_hbox->addWidget(owl_lbl);
      auto* title_lbl = new colman::ScalingLabel(
          "<b>" + QApplication::tr("Statistics") + "</b>", this, 4);
      hdr_hbox->addWidget(title_lbl, 1);
      vbox->addLayout(hdr_hbox);

      auto* stock_box =
          new QGroupBox(QApplication::tr("Current stock"), this);
      auto* stock_vbox = new QVBoxLayout(stock_box);
      stock_table_ = make_table(
          {QApplication::tr("Category"), QApplication::tr("Items"),
           QApplication::tr("Copies"), QApplication::tr("Lent"),
           QApplication::tr("Reserved"), QApplication::tr("Overdue")},
          "statistics_stock_table_", stock_box);
      stock_vbox->addWidget(stock_table_);
      vbox->addWidget(stock_box, 1);

      auto* period_box = new QGroupBox(
          QApplication::tr("Acquisitions and liquidations"), this);
      auto* period_vbox = new QVBoxLayout(period_box);
      auto* range_hbox = new QHBoxLayout;
      range_hbox->addWidget(
          new QLabel(QApplication::tr("from"), period_box));
      from_edit_ = new QDateEdit(period_box);
      from_edit_->setObjectName("statistics_from_");
      from_edit_->setCalendarPopup(true);
      range_hbox->addWidget(from_edit_);
      range_hbox->addWidget(new QLabel(QApplication::tr("to"), period_box));
      to_edit_ = new QDateEdit(period_box);
      to_edit_->setObjectName("statistics_to_");
      to_edit_->setCalendarPopup(true);
      range_hbox->addWidget(to_edit_);
      range_hbox->addStretch(1);
      period_vbox->addLayout(range_hbox);
      period_table_ = make_table(
          {QApplication::tr("Category"), QApplication::tr("Acquired items"),
           QApplication::tr("Acquired copies"),
           QApplication::tr("Liquidated items"),
           QApplication::tr("Liquidated copies")},
          "statistics_period_table_", period_box);
      period_vbox->addWidget(period_table_);
      vbox->addWidget(period_box, 1);

      connect(back_btn, &QPushButton::clicked, [on_back]() { on_back(); });
      connect(from_edit_, &QDateEdit::dateChanged, [this](QDate const& d) {
        COLMAN_TRY {
          to_edit_->setMinimumDate(d);
          refresh();
        }
        COLMAN_CATCH;
      });
      connect(to_edit_, &QDateEdit::dateChanged, [this](QDate const&) {
        COLMAN_TRY {
          refresh();
        }
        COLMAN_CATCH;
      });
    }

    /// Resets the time span to the last year and reloads all figures.
    void enter()
    {
      QDate const today = QDate::currentDate();
      QSignalBlocker const block_from(from_edit_);
      QSignalBlocker const block_to(to_edit_);
      from_edit_->setDate(today.addYears(-1));
      to_edit_->setMinimumDate(from_edit_->date());
      to_edit_->setDate(today);
      refresh();
    }

  private:
    /// Reloads the figures of both tables.
    void refresh()
    {
      auto const stats = data_.queries_->query_statistics(
          from_edit_->date(), to_edit_->date());
      std::vector<StatisticsRow> stock_rows;
      std::vector<StatisticsRow> period_rows;
      for(auto const& s : stats) {
        if(s.copies_ != 0 || s.lent_ != 0 || s.reserved_ != 0) {
          stock_rows.push_back(
              {s.type_name_,
               {s.items_, s.copies_, s.lent_, s.reserved_, s.overdue_}});
        }
        if(s.acquired_copies_ != 0 || s.liquidated_copies_ != 0) {
          period_rows.push_back(
              {s.type_name_,
               {s.acquired_items_, s.acquired_copies_, s.liquidated_items_,
                s.liquidated_copies_}});
        }
      }
      fill_table(stock_table_, std::move(stock_rows));
      fill_table(period_table_, std::move(period_rows));
    }

    colman::AppData& data_;                ///< central application state
    QTableWidget* stock_table_ = nullptr;  ///< current stock per item type
    QTableWidget* period_table_ = nullptr; ///< movements in the time span
    QDateEdit* from_edit_ = nullptr;       ///< first day of the time span
    QDateEdit* to_edit_ = nullptr;         ///< last day of the time span
  };

} // namespace

/**
   Creates the statistics page, adds it to the page stack and connects it
   to the statistics tile of the central page.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_statistics_page(QApplication& app, colman::AppData& data)
{
  auto* page = new StatisticsPage(data, [&data]() {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
  });
  data.mw_.pages_stack_->addWidget(page);
  app.connect(data.statistics_button_, &QToolButton::clicked,
              [&data, page]() {
                COLMAN_TRY {
                  page->enter();
                  data.mw_.pages_stack_->setCurrentWidget(page);
                }
                COLMAN_CATCH;
              });
}

/* EOF */
