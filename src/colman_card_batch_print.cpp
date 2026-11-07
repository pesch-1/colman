// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of shared card batch-print helpers.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_card_batch_print.hpp"
#ifndef COLMAN_CODE128_ITEM_HPP
#  include "colman_code128_item.hpp"
#endif
#ifndef COLMAN_LABEL_PRINTER_HPP
#  include "colman_label_printer.hpp"
#endif
#ifndef COLMAN_PRINTER_CALIBRATION_HPP
#  include "colman_printer_calibration.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QGRAPHICSPIXMAPITEM
#  include <QGraphicsPixmapItem>
#  define SYSTEM_QGRAPHICSPIXMAPITEM
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QIMAGE
#  include <QImage>
#  define SYSTEM_QIMAGE
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPAGELAYOUT
#  include <QPageLayout>
#  define SYSTEM_QPAGELAYOUT
#endif
#ifndef SYSTEM_QPAGESIZE
#  include <QPageSize>
#  define SYSTEM_QPAGESIZE
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPRINTDIALOG
#  include <QPrintDialog>
#  define SYSTEM_QPRINTDIALOG
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QTRANSFORM
#  include <QTransform>
#  define SYSTEM_QTRANSFORM
#endif
#ifndef SYSTEM_QTREEWIDGET
#  include <QTreeWidget>
#  define SYSTEM_QTREEWIDGET
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{

/// Converts millimetres to scene units (1 scene unit = 0.1 mm).
  inline qreal mm_to_scene(qreal x)
  {
    return x * 10.0;
  }

/// Applies a barcode bar width reduction to every Code128Item in @p scene.
/**
   The reduction compensates for ink/heat spread on (mostly thermal) printers
   by painting black bars narrower; it is taken from the active layout preset
   so each printer can carry its own value (0 = no compensation).
*/
  void apply_bar_width_reduction(QGraphicsScene& scene, qreal mm)
  {
    if(mm <= 0.0) {
      return;
    }
    for(QGraphicsItem* it : scene.items()) {
      if(auto* bc = dynamic_cast<colman::Code128Item*>(it)) {
        bc->set_bar_width_reduction(mm);
      }
    }
  }

/// Unchecks all tree items whose ID is in @p done; signals blocked during edit.
  void uncheck_done_items(QTreeWidget* tree, QSet<int> const& done)
  {
    if(!tree) {
      return;
    }
    colman::AutoBlockSignals b(tree);
    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
      QTreeWidgetItem* ti = tree->topLevelItem(i);
      if(done.contains(ti->data(0, Qt::UserRole).toInt())) {
        ti->setCheckState(0, Qt::Unchecked);
      }
    }
  }

} // namespace


/**
   Removes every item in @p st->preview_items_ from the scene, deletes it,
   and clears the list.  Must not be called after the scene itself has been
   destroyed (e.g. inside a layout_changed handler — see batch_add_previews).
*/
void colman::batch_remove_previews(std::shared_ptr<CardBatchState> const& st)
{
  if(!st->label_sel_) {
    return;
  }
  QGraphicsScene* sc = st->label_sel_->view_scene();
  for(QGraphicsItem* it : st->preview_items_) {
    sc->removeItem(it);
    delete it;
  }
  st->preview_items_.clear();
}


/**
   Renders each remaining entity into a temporary scene at 2× scene
   resolution, and adds it as a QGraphicsPixmapItem overlay at the
   corresponding label position on the label-selector scene.
*/
void colman::batch_add_previews(std::shared_ptr<CardBatchState> const& st,
                                CardRenderFn const& render_fn)
{
  if(!st->label_sel_ || st->remaining_.isEmpty()) {
    return;
  }
  QGraphicsScene* sc = st->label_sel_->view_scene();
  QList<SelectedLabel> const sel = st->label_sel_->selected_labels();
  // The selected layout (and thus its element arrangement) may have changed
  // since the last print batch; keep st->preset_ in sync for the preview.
  st->preset_ = st->label_sel_->current_layout();
  IdCardLayout const& arrangement = st->preset_.card_arrangement;

  for(int i = 0; i < sel.size() && i < st->remaining_.size(); ++i) {
    SelectedLabel const& lbl = sel[i];
    qreal const x = mm_to_scene(lbl.x_mm);
    qreal const y = mm_to_scene(lbl.y_mm);
    qreal const sw = mm_to_scene(lbl.width_mm);
    qreal const sh = mm_to_scene(lbl.height_mm);

    QGraphicsScene card_scene;
    card_scene.setSceneRect(0, 0, sw, sh);
    render_fn(&card_scene, st->remaining_[i], sw, sh, true, arrangement);

    int const img_w = qMax(1, static_cast<int>(sw) * 2);
    int const img_h = qMax(1, static_cast<int>(sh) * 2);
    QImage img(img_w, img_h, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter painter_(&img);
    painter_.setRenderHint(QPainter::Antialiasing);
    card_scene.render(&painter_, QRectF(0, 0, img_w, img_h),
                      card_scene.sceneRect());
    painter_.end();

    auto* pix = new QGraphicsPixmapItem(QPixmap::fromImage(img));
    pix->setPos(x, y);
    pix->setTransform(QTransform::fromScale(sw / img_w, sh / img_h));
    pix->setZValue(1.0);
    pix->setAcceptedMouseButtons(Qt::NoButton);
    sc->addItem(pix);
    st->preview_items_.append(pix);
  }
}


/**
   Ends any open print job and unchecks already-printed items in @p tree.
   Items in @p st->remaining_ remain checked so the user can retry.
*/
void colman::batch_abort(std::shared_ptr<CardBatchState> const& st,
                         QTreeWidget* tree)
{
  if(st->print_started_) {
    if(st->painter_) {
      st->painter_->end();
    }
    st->printer_.reset();
    st->painter_.reset();
    st->print_started_ = false;
    st->first_page_ = true;
  }
  if(!st->printed_.isEmpty()) {
    QSet<int> done;
    for(int id : st->printed_) {
      done.insert(id);
    }
    st->printed_.clear();
    uncheck_done_items(tree, done);
  }
  st->remaining_.clear();
}


/**
   Executes one print batch.

   Label-printer path: all remaining entities are printed in a single
   call without a dialog.  On error a warning is shown and the loop
   stops; only entities actually sent to the printer are moved to printed_.

   Sheet-printer path: opens QPrintDialog on the first call, then prints
   one page per call until all entities are processed or the user aborts.

   On completion (remaining_ empty): printed items are unchecked in
   @p tree, @p stack is set to index 0, and @p update_status_fn is called.
*/
void colman::batch_do_print(std::shared_ptr<CardBatchState> const& st,
                            CardRenderFn const& render_fn,
                            bool draw_border,
                            QWidget* parent_widget,
                            QStackedWidget* stack,
                            QTreeWidget* tree,
                            std::function<void()> const& update_status_fn,
                            std::function<void(QString const&)> const& save_layout_fn)
{
  if(!st->label_sel_) {
    return;
  }
  QList<SelectedLabel> const labels = st->label_sel_->selected_labels();
  if(labels.isEmpty()) {
    return;
  }

  LabelLayoutPreset const preset = st->label_sel_->current_layout();
  if(!st->print_started_) {
    st->preset_ = preset;
    save_layout_fn(preset.name);
  }

  // --- Label-printer path: one label per entity, no dialog ----------------
  if(preset.is_label_printer) {
    // CUPS silently queues jobs for an offline printer, so check readiness
    // first and tell the user to connect it instead of printing nothing.
    if(!colman::thermal_printer_ready(preset.printer_cups_name)) {
      QMessageBox::warning(
          parent_widget,
          QApplication::tr("Printer not connected"),
          QApplication::tr(
              "Please connect the \"%1\" printer and try again, "
              "or select a different layout.")
              .arg(preset.printer_cups_name));
      return;
    }
    // Same idea for an empty label roll: the printer would accept the job
    // and stall, so warn the user up front instead.
    if(colman::warn_when_out_of_labels(parent_widget)) {
      return;
    }
    QSizeF const label_mm(preset.label_width, preset.label_height);
    qreal const sw = mm_to_scene(preset.label_width);
    qreal const sh = mm_to_scene(preset.label_height);
    // The print-position calibration is the same for every label in the batch.
    colman::PrinterCalibration const cal =
        colman::load_printer_calibration(preset.printer_cups_name);
    // Render every card to an image first, then emit them as a single CUPS
    // job; one job per label would make the printer recalibrate between
    // labels and pause for several seconds each time.
    QList<QImage> images;
    QList<int> ids;
    for(int id : st->remaining_) {
      QGraphicsScene card_scene;
      card_scene.setSceneRect(0, 0, sw, sh);
      render_fn(&card_scene, id, sw, sh, draw_border, preset.card_arrangement);
      apply_bar_width_reduction(card_scene, preset.bar_width_reduction_mm);
      images.append(colman::render_label_scene(
          &card_scene, label_mm, preset.printer_dpi, cal, preset.rotate_180));
      ids.append(id);
    }
    QString err;
    if(colman::send_images_to_thermal_printer(
           images, preset.printer_cups_name, label_mm, preset.printer_dpi, err,
           preset.darkness)) {
      st->printed_.append(ids);
      colman::warn_when_roll_runs_out_while_printing(
          parent_widget, images.size(), label_mm);
    }
    else {
      QMessageBox::warning(parent_widget,
                           QApplication::tr("Print error"), err);
    }
    // Treat the label-printer batch as one atomic unit; remaining_ is
    // cleared here so the "done" block below runs and resets the UI.
    st->remaining_.clear();
  }
  else {
    // --- Sheet-printer path: QPrintDialog on first call ------------------
    if(!st->print_started_) {
      st->printer_ = std::make_unique<QPrinter>(QPrinter::HighResolution);
      st->printer_->setFullPage(true);
      st->printer_->setPageSize(QPageSize(
          QSizeF(preset.page_width_mm, preset.page_height_mm),
          QPageSize::Millimeter));
      st->printer_->setPageMargins(
          QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

      QPrintDialog dlg(st->printer_.get(), parent_widget);
      if(dlg.exec() != QDialog::Accepted) {
        st->printer_.reset();
        return;
      }
      st->painter_ = std::make_unique<QPainter>(st->printer_.get());
      if(!st->painter_->isActive()) {
        st->printer_.reset();
        st->painter_.reset();
        return;
      }
      st->painter_->setRenderHint(QPainter::Antialiasing);
      st->print_started_ = true;
      st->first_page_ = true;
    }

    if(!st->first_page_) {
      st->printer_->newPage();
    }
    st->first_page_ = false;

    qreal const dpi = st->printer_->resolution();
    qreal const mm_to_px = dpi / 25.4;
    // Correct the chosen sheet printer's print-position error: the offset
    // shifts every label by the same physical amount on the page, the scale
    // adjusts each card's size.
    colman::PrinterCalibration const cal =
        colman::load_printer_calibration(st->printer_->printerName());

    int const n = std::min(labels.size(), st->remaining_.size());
    for(int i = 0; i < n; ++i) {
      SelectedLabel const& lbl = labels[i];
      qreal const sw = mm_to_scene(lbl.width_mm);
      qreal const sh = mm_to_scene(lbl.height_mm);
      QGraphicsScene card_scene;
      card_scene.setSceneRect(0, 0, sw, sh);
      render_fn(&card_scene, st->remaining_[i], sw, sh, draw_border,
                preset.card_arrangement);
      apply_bar_width_reduction(card_scene, preset.bar_width_reduction_mm);
      card_scene.render(
          st->painter_.get(),
          QRectF((lbl.x_mm + cal.offset_x) * mm_to_px,
                 (lbl.y_mm + cal.offset_y) * mm_to_px,
                 lbl.width_mm * cal.scale_x * mm_to_px,
                 lbl.height_mm * cal.scale_y * mm_to_px));
      st->printed_.append(st->remaining_[i]);
    }
    st->remaining_ = st->remaining_.mid(n);
  }

  if(st->remaining_.isEmpty()) {
    if(st->painter_) {
      st->painter_->end();
    }
    st->printer_.reset();
    st->painter_.reset();
    st->print_started_ = false;

    QSet<int> done;
    for(int id : st->printed_) {
      done.insert(id);
    }
    st->printed_.clear();

    uncheck_done_items(tree, done);
    if(stack) {
      stack->setCurrentIndex(0);
    }
    update_status_fn();
  }
  else {
    st->label_sel_->set_max_count(st->remaining_.size());
    st->label_sel_->clear_selection();
  }
}

/* EOF */
