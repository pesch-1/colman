// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Shared state and helpers for the card batch-print workflow.
/**
   @file
   @author Christoph Pesch

   Both the item-barcode and person-barcode pages use the same
   multi-batch print loop (preview overlays, label-printer path, sheet-
   printer path, abort).  This file provides the shared infrastructure so
   that the per-page code only supplies a render callback and widget pointers.

   @par Scene coordinate convention
   All render callbacks receive dimensions in scene units (1 unit = 0.1 mm),
   consistent with the rest of the colman rendering pipeline.
*/

#ifndef COLMAN_CARD_BATCH_PRINT_HPP
#  define COLMAN_CARD_BATCH_PRINT_HPP

#  include "colman_label_selection_widget.hpp"

#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif
#  ifndef SYSTEM_MEMORY
#    include <memory>
#    define SYSTEM_MEMORY
#  endif
#  ifndef SYSTEM_QGRAPHICSITEM
#    include <QGraphicsItem>
#    define SYSTEM_QGRAPHICSITEM
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

#  ifndef SYSTEM_QPAINTER
#    include <QPainter>
#    define SYSTEM_QPAINTER
#  endif
#  ifndef SYSTEM_QPRINTER
#    include <QPrinter>
#    define SYSTEM_QPRINTER
#  endif

class QGraphicsScene;
class QSpinBox;
class QStackedWidget;
class QTreeWidget;
class QWidget;

namespace colman
{

/// Render callback for a single card into a pre-sized scene.
/**
   The scene is already sized (sceneRect set to (0,0,sw,sh)).
   Returns @c true on success.
   @param scene       target scene; scene units: 0.1 mm per unit.
   @param entity_id   item or person database ID.
   @param sw          scene width in units (= label_width_mm × 10).
   @param sh          scene height in units.
   @param draw_border whether to draw a thin border rectangle.
   @param arrangement element arrangement of the selected layout (an empty
                      arrangement, card_width_mm_ == 0, means "use default").
*/
  using CardRenderFn = std::function<
      bool(QGraphicsScene* scene, int entity_id,
           qreal sw, qreal sh, bool draw_border,
           IdCardLayout const& arrangement)>;


/// Shared state for a multi-page / multi-batch card print job.
  struct CardBatchState
  {
    LabelSelectionWidget* label_sel_ = nullptr; ///< label selector widget
    QSpinBox* copies_spin_ = nullptr; ///< copies-per-entity spin box (may be null)
    QList<QGraphicsItem*> preview_items_;     ///< overlay items in label_sel's scene
    QList<int> base_ids_;          ///< selected entity IDs before copy expansion
    QList<int> remaining_;         ///< entity IDs still to print
    QList<int> printed_;           ///< entity IDs printed this job
    bool print_started_ = false;
    std::unique_ptr<QPrinter> printer_;
    std::unique_ptr<QPainter> painter_;
    bool first_page_ = true;
    LabelLayoutPreset preset_;
  };


/// Removes and deletes all preview overlay items from label_sel's scene.
/**
   @param st shared print state.
*/
  void batch_remove_previews(std::shared_ptr<CardBatchState> const& st);


/// Adds card-preview pixmap overlays for the first N remaining entities.
/**
   Pairs each currently selected label position with the corresponding ID
   from @p st->remaining_, renders a preview image, and overlays it on
   the label-selector scene.
   @param st        shared print state.
   @param render_fn card render callback.
*/
  void batch_add_previews(std::shared_ptr<CardBatchState> const& st,
                          CardRenderFn const& render_fn);


/// Aborts an in-progress print job; unchecks only already-printed items.
/**
   Items in @p st->remaining_ stay checked; items in @p st->printed_ are
   unchecked and then cleared.  Any open QPainter/QPrinter is closed.
   @param st   shared print state.
   @param tree tree widget containing the checkable entity rows.
*/
  void batch_abort(std::shared_ptr<CardBatchState> const& st,
                   QTreeWidget* tree);


/// Executes one print batch using the current label selection.
/**
   Label-printer path: all remaining entities are sent in one call with no
   dialog; shows a warning on failure and stops.

   Sheet-printer path: opens QPrintDialog on the first call, then prints
   one page per call until all entities are done or the user cancels.

   @p save_layout_fn is called once (first call only) with the preset name.
   On completion (@p st->remaining_ empty): unchecks printed items in
   @p tree, navigates @p stack to index 0, and calls @p update_status_fn.

   @param st               shared print state.
   @param render_fn        card render callback.
   @param draw_border      whether to request a card border.
   @param parent_widget    parent widget for dialogs.
   @param stack            page stack; set to index 0 on completion.
   @param tree             tree widget; printed items are unchecked on done.
   @param update_status_fn called after completion to refresh the UI.
   @param save_layout_fn   called once with the layout name to persist it.
*/
  void batch_do_print(std::shared_ptr<CardBatchState> const& st,
                      CardRenderFn const& render_fn,
                      bool draw_border,
                      QWidget* parent_widget,
                      QStackedWidget* stack,
                      QTreeWidget* tree,
                      std::function<void()> const& update_status_fn,
                      std::function<void(QString const&)> const& save_layout_fn);

} // namespace colman

#endif /* #ifndef COLMAN_CARD_BATCH_PRINT_HPP */

/* EOF */
