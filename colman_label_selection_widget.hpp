// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of LabelSelectionWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_LABEL_SELECTION_WIDGET_HPP
#  define COLMAN_LABEL_SELECTION_WIDGET_HPP

#  include "colman_label_layout_widget.hpp"

#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif

class QComboBox;
class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QPushButton;
class QResizeEvent;
class QShowEvent;

namespace colman
{

/// One label on the sheet, as returned by LabelSelectionWidget.
/**
   Position and size are given in millimetres relative to the top-left
   corner of the page.
*/
  struct SelectedLabel
  {
    int col = 0;         ///< column index (0-based)
    int row = 0;         ///< row index (0-based)
    double x_mm = 0.0;      ///< left edge on page (mm)
    double y_mm = 0.0;      ///< top edge on page (mm)
    double width_mm = 0.0;  ///< label width (mm)
    double height_mm = 0.0; ///< label height (mm)
  };

/// Widget for selecting individual labels on a label sheet.
/**
   Shows a graphical A4-page preview with one clickable rectangle per
   label.  Clicking a label toggles its selection; the number of
   selected labels is capped at @c max_count.

   A combo box at the top lists all built-in and user-defined label
   layouts.  An edit button opens a dialog containing LabelLayoutWidget
   so the user can manage layouts without leaving the context.

   The widget emits no signals — the caller retrieves the result via
   selected_labels() when needed (e.g. on dialog accept).

   @par Usage
   @code
   auto* w = new LabelSelectionWidget(10);   // up to 10 labels
   // … show w in a dialog …
   QList<SelectedLabel> sel = w->selected_labels();
   @endcode
*/
  class LabelSelectionWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit LabelSelectionWidget(int max_count = 1, QWidget* parent = nullptr);

  /// Returns the labels currently selected by the user.
    QList<SelectedLabel> selected_labels() const;

  /// Returns the layout preset currently active in the widget.
    LabelLayoutPreset const& current_layout() const { return layout_; }

  /// Returns the internal graphics scene for read-only overlay additions.
    QGraphicsScene* view_scene() const { return scene_; }

    int max_count() const { return max_count_; } ///< Maximum selectable labels.

  /// Changes the maximum.  Clears the selection if it exceeds the new limit.
    void set_max_count(int n);

  /// Pre-selects the preset whose label dimensions best match the card size.
  /**
     Searches all built-in and user-defined presets for one whose
     @c label_width and @c label_height are within 0.5 mm of @p card_w_mm
     and @p card_h_mm.  If a match is found it is selected in the combo box
     (triggering a normal layout switch); otherwise the selection is unchanged.
     @param card_w_mm  card width in mm (longer side).
     @param card_h_mm  card height in mm (shorter side).
  */
    void set_initial_preset_for_card(double card_w_mm, double card_h_mm);

  /// Pre-selects the preset with the given @p name in the combo box.
  /**
     If @p name is empty or not found in the combo box the selection is
     left unchanged.
     @param name  the preset name as stored in LabelLayoutPreset::name.
  */
    void set_initial_preset_by_name(QString const& name);

  /// Clears the position selection.
    void clear_selection();

  /// Refreshes the layout combo from QSettings and the printer list.
  /**
     Call this before showing the widget to pick up presets that were
     saved after construction.
  */
    void refresh_layouts();

  /// Restricts the combo to user-defined, named layouts only.
  /**
     When enabled the combo lists neither the factory (Avery) presets nor
     the auto-detected label printers, and the "Edit label layouts" button
     is hidden.  Use this in print dialogs where the operator may only pick
     a layout an administrator has created and named, but may not create or
     edit layouts.  Repopulates the combo immediately.
     @param on  true to show only user layouts.
  */
    void set_user_layouts_only(bool on);

  /// Restricts the user-only combo to layouts created for @p purpose.
  /**
     Layouts whose purpose equals @p purpose (or which are untagged @c Any)
     are offered; all others are hidden.  Only takes effect together with
     set_user_layouts_only(true).  Repopulates the combo immediately.
  */
    void set_layout_purpose(LabelLayoutPurpose purpose);

  /// Returns true if no user layout is available for @p purpose.
  /**
     A layout qualifies when its purpose equals @p purpose or is @c Any.
     Print pages use this to disable printing when nothing is offered.
  */
    static bool has_no_user_layouts_for(LabelLayoutPurpose purpose);

  Q_SIGNALS:
  /// Emitted when the active layout preset changes.
    void layout_changed();
  /// Emitted when the label position selection changes.
    void selection_changed();

  protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private Q_SLOTS:
    void on_layout_combo_changed(int index);
    void on_edit_layouts();
    void on_layout_applied(colman::LabelLayoutPreset const& p);
    void on_deselect_all();
    void on_select_all();

  private:
    void build_ui();
    void populate_combo();
    void apply_layout(LabelLayoutPreset const& p);
    void rebuild_scene();
    void toggle_label(int col, int row);
    void update_item_states();
    void update_status();

    int max_count_ = 1;
    bool user_layouts_only_ = false; ///< true = list only user-defined layouts
    LabelLayoutPurpose layout_purpose_ = LabelLayoutPurpose::Any; ///< combo filter
    LabelLayoutPreset layout_;
    QList<bool> selected_; ///< size = rows × columns, row-major

  /// Clickable label items; actual runtime type is LabelItem (see .cpp).
    QList<QGraphicsRectItem*> items_;

    QComboBox* layout_combo_ = nullptr;
    QPushButton* edit_btn_ = nullptr;
    QLabel* status_label_ = nullptr;
    QPushButton* deselect_btn_ = nullptr;
    QPushButton* select_all_btn_ = nullptr;
    QGraphicsView* view_ = nullptr;
    QGraphicsScene* scene_ = nullptr;
  };

} // namespace colman

#endif /* #ifndef COLMAN_LABEL_SELECTION_WIDGET_HPP */

/* EOF */
