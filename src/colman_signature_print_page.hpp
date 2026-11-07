// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of SignaturePrintPage.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SIGNATURE_PRINT_PAGE_HPP
#  define COLMAN_SIGNATURE_PRINT_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QSpinBox;
class QStandardItemModel;
class QTableView;

namespace colman
{

/// Prints @p count signature labels with the given @p text.
/**
   Reads the last saved preset name from the collection settings and
   dispatches to the thermal or sheet-printer path:
   - Thermal: sends one label per copy directly; warns if not connected.
   - Sheet: opens the batch-print page for interactive position selection.
   @param text  text to print on each label.
   @param count number of labels to print.
   @param data  central application state (queries and page stack).
*/
  void print_signature_labels(QString const& text,
                              int count,
                              AppData& data);

/// Page for printing signature labels.
/**
   Shows an item list on the left.  On the right the user can choose
   upper / lower signature text and a label count, then trigger printing.
   Clicking a print button opens a second page on which the actual label
   layout is chosen (restricted to user-defined, named layouts); the
   default selection is the layout last saved in the signature label
   editor.
*/
  class SignaturePrintPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param data    central application state.
     @param on_back called when the Back button is pressed.
     @param parent  optional parent widget.
  */
    explicit SignaturePrintPage(colman::AppData& data,
                                std::function<void()> on_back,
                                QWidget* parent = nullptr);

  /// Refreshes item list and combo boxes from the database.
  /**
     Call this every time the page becomes visible.
  */
    void enter();

  private:
  /// Repopulates the item list from the database.
    void populate_item_list();
  /// Repopulates the upper-signature combo from the database.
    void populate_top_combo();
  /// Repopulates the lower-signature combo from the database.
    void populate_bottom_combo();
  /// Fills the combo boxes from the currently selected item.
    void on_item_selected();
  /// Sends @p count copies of @p text to the printer.
    void do_print_signatures(QString const& text, int count);

    colman::AppData& data_;
    std::function<void()> on_back_;
    QStandardItemModel* item_model_ = nullptr;
    QSortFilterProxyModel* item_proxy_ = nullptr;
    QTableView* item_table_ = nullptr;
    QLineEdit* search_edit_ = nullptr;
    QLabel* item_count_label_ = nullptr;
    QComboBox* top_combo_ = nullptr;
    QSpinBox* top_count_ = nullptr;
    QPushButton* print_top_btn_ = nullptr;
    QComboBox* bottom_combo_ = nullptr;
    QSpinBox* bottom_count_ = nullptr;
    QPushButton* print_bottom_btn_ = nullptr;
  };

} // namespace colman

#endif /* #ifndef COLMAN_SIGNATURE_PRINT_PAGE_HPP */

/* EOF */
