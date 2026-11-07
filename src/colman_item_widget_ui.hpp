// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of struct ItemWidgetUi.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ITEM_WIDGET_UI_HPP
#  define COLMAN_ITEM_WIDGET_UI_HPP

#  ifndef SYSTEM_QCOREAPPLICATION
#    include <QCoreApplication>
#    define SYSTEM_QCOREAPPLICATION
#  endif

class QCheckBox;
class QComboBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QWidget;

namespace colman
{

  /// Hand-written form layout for ItemWidget (replaces the old .ui file).
  /**
     Builds the fixed common fields of the item editor — name, thumbnail
     with its action buttons, the school-item block, place, signatures,
     description, keywords and barcode — inside a scroll area, with the
     item-type selector combo box above it.  Type-specific attribute rows
     are added dynamically by ItemWidget into @c item_grid_.

     The members mirror the widgets that the rest of ItemWidget addresses
     by name; @c setup_ui() constructs them and wires the school-item
     visibility connections.
  */
  struct ItemWidgetUi
  {
    QComboBox* item_type_box_ = nullptr;     ///< item type selector
    QWidget* item_widget_ = nullptr;         ///< scroll-area content widget
    QGridLayout* item_grid_ = nullptr;       ///< grid holding all field rows

    QLabel* label_21_ = nullptr;             ///< "name/title:" label
    QLineEdit* name_ = nullptr;              ///< item name field
    QLabel* label_22_ = nullptr;             ///< "thumbnail:" label
    QLabel* thumbnail_ = nullptr;            ///< thumbnail preview
    QHBoxLayout* thumbnail_layout_ = nullptr; ///< thumbnail + buttons row
    QPushButton* browse_thumbnail_ = nullptr; ///< browse for image
    QPushButton* scan_thumbnail_ = nullptr;   ///< scan an image
    QPushButton* amazon_thumbnail_ = nullptr; ///< fetch cover online
    QPushButton* zoom_thumbnail_ = nullptr;   ///< enlarge the thumbnail
    QPushButton* remove_thumbnail_ = nullptr; ///< remove the thumbnail
    QCheckBox* is_schoolitem_ = nullptr;     ///< school-item toggle
    QLabel* label_100_ = nullptr;            ///< "subject:" label
    QLineEdit* subject_ = nullptr;           ///< school subject field
    QLabel* label_101_ = nullptr;            ///< "academic year:" label
    QLineEdit* academic_year_ = nullptr;     ///< academic year field
    QLabel* label_102_ = nullptr;            ///< school "type:" label
    QLineEdit* school_type_ = nullptr;       ///< school type field
    QLabel* label_23_ = nullptr;             ///< "place:" label
    QLineEdit* place_ = nullptr;             ///< place field
    QLabel* label_sig_top_ = nullptr;        ///< "signature top:" label
    QComboBox* signature_top_ = nullptr;     ///< top signature combo
    QLabel* label_sig_bottom_ = nullptr;     ///< "signature bottom:" label
    QComboBox* signature_bottom_ = nullptr;  ///< bottom signature combo
    QLabel* label_24_ = nullptr;             ///< "description:" label
    QPlainTextEdit* description_ = nullptr;  ///< description editor
    QLabel* label_keywords_ = nullptr;       ///< "keywords:" label
    QLineEdit* keywords_placeholder_ = nullptr; ///< static keywords cell filler
    QLabel* label_barcode_ = nullptr;        ///< "Colman barcode:" label
    QLineEdit* barcode_display_ = nullptr;   ///< read-only barcode display

    /// Builds the widgets and layouts into @p parent.
    /**
       @param parent the ItemWidget that owns and hosts the form.
    */
    void setup_ui(QWidget* parent);

  /// Translates in the historic "ItemWidget" context.
  /**
     Declared last because the macro ends with a @c private: access
     specifier; keeping the data members and setup_ui() above it leaves
     them public.  Using the "ItemWidget" context preserves the
     translations generated from the former colman_item_widget.ui file.
  */
    Q_DECLARE_TR_FUNCTIONS(ItemWidget)
  };

} // namespace colman

#endif /* #ifndef COLMAN_ITEM_WIDGET_UI_HPP */

/* EOF */
