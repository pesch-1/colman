// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of SignatureLabelEditor.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_signature_label_editor.hpp"
#ifndef COLMAN_CARD_ELEMENT_ITEM_HPP
#  include "colman_card_element_item.hpp"
#endif
#ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP
#  include "colman_label_layout_widget.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPEN
#  include <QPen>
#  define SYSTEM_QPEN
#endif
#ifndef SYSTEM_QRESIZEEVENT
#  include <QResizeEvent>
#  define SYSTEM_QRESIZEEVENT
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
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_QSLIDER
#  include <QSlider>
#  define SYSTEM_QSLIDER
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QWHEELEVENT
#  include <QWheelEvent>
#  define SYSTEM_QWHEELEVENT
#endif
#ifndef COLMAN_SIGNATURE_LABEL_LAYOUT_HPP
#  include "colman_signature_label_layout.hpp"
#endif

namespace
{

/// Converts mm to scene units (1 scene unit = 0.1 mm).
  inline qreal mm(qreal x)
  {
    return x * 10.0;
  }

/// Converts scene units back to mm.
  inline double to_mm(qreal x)
  {
    return x / 10.0;
  }

} // anonymous namespace


// ====================================================================
// colman::SignatureLabelEditor
// ====================================================================

/**
   Constructs the editor and populates the scene with saved or default
   zone positions.
   @param queries database / REST queries.
   @param parent  parent widget.
*/
colman::SignatureLabelEditor::SignatureLabelEditor(
    IQueries& queries, QWidget* parent)
    : QWidget(parent)
{
  Q_UNUSED(queries); // layouts are now machine-local (QSettings), not in DB
  auto* vl = new QVBoxLayout(this);
  vl->setContentsMargins(0, 0, 0, 0);

  // ---- label-size selection ---------------------------------------
  auto* grp = new QGroupBox(tr("Label size"), this);
  auto* grp_vl = new QVBoxLayout(grp);
  grp_vl->setContentsMargins(4, 4, 4, 4);
  layout_w_ = new colman::LabelLayoutWidget(grp);
  layout_w_->set_purpose(colman::LabelLayoutPurpose::Signature);
  grp_vl->addWidget(layout_w_);
  vl->addWidget(grp);

  // ---- label preview / editor -------------------------------------
  scene_ = new QGraphicsScene(this);
  view_ = new QGraphicsView(scene_, this);
  view_->setRenderHint(QPainter::Antialiasing);
  view_->setDragMode(QGraphicsView::NoDrag);
  view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  view_->installEventFilter(this);
  view_->viewport()->installEventFilter(this);
  vl->addWidget(view_, 1);

  // Selecting a layout (or New) loads its geometry and text arrangement.
  connect(layout_w_, &colman::LabelLayoutWidget::layout_applied,
          [this](colman::LabelLayoutPreset const& p) {
            apply_preset_to_preview(p);
          });

  // Live geometry edits resize the preview but keep the current text zone.
  connect(layout_w_, &colman::LabelLayoutWidget::geometry_changed,
          [this](colman::LabelLayoutPreset const& p) {
            card_w_mm_ = p.label_width;
            card_h_mm_ = p.label_height;
            SignatureLabelLayout const cur = current_sig_arrangement();
            rebuild_card(card_w_mm_, card_h_mm_, &cur);
          });

  // ---- Margin slider ----------------------------------------------
  {
    auto* mhl = new QHBoxLayout;
    auto* mlbl = new QLabel(tr("Margin:"), this);
    margin_slider_ = new QSlider(Qt::Horizontal, this);
    margin_slider_->setRange(0, 100);
    margin_slider_->setValue(qRound(margin_mm_ * 10.0));
    margin_slider_->setMaximumWidth(180);
    margin_slider_->setToolTip(
        tr("Free margin around the label edge (0 – 10 mm)"));
    margin_value_label_ = new QLabel(
        QString("%1 mm").arg(margin_mm_, 0, 'f', 1), this);
    margin_value_label_->setMinimumWidth(45);
    mhl->addWidget(mlbl);
    mhl->addWidget(margin_slider_);
    mhl->addWidget(margin_value_label_);
    mhl->addStretch();
    vl->addLayout(mhl);

    connect(margin_slider_, &QSlider::valueChanged,
            [this](int v) {
              margin_mm_ = v / 10.0;
              margin_value_label_->setText(
                  QString("%1 mm").arg(margin_mm_, 0, 'f', 1));
              SignatureLabelLayout const cur = current_sig_arrangement();
              rebuild_card(card_w_mm_, card_h_mm_, &cur);
            });
  }

  // ---- New / Delete / Save / Reset buttons ------------------------
  // The layout-lifecycle buttons live next to Save (reparented out of the
  // embedded LabelLayoutWidget into this bottom row).
  auto* hl = new QHBoxLayout;
  save_btn_ = new QPushButton(tr("Save"), this);
  save_btn_->setObjectName("layout_save_");
  reset_btn_ = new QPushButton(tr("Reset"), this);
  hl->addStretch();
  hl->addWidget(layout_w_->new_button());
  hl->addWidget(layout_w_->delete_button());
  hl->addWidget(save_btn_);
  hl->addWidget(reset_btn_);
  vl->addLayout(hl);

  connect(save_btn_, &QPushButton::clicked,
          this, &colman::SignatureLabelEditor::save_layout);
  connect(reset_btn_, &QPushButton::clicked,
          this, &colman::SignatureLabelEditor::reset_layout);

  // Load the last-used signature layout, then build the initial preview
  // from whatever is currently selected.
  QString const last = colman::LabelLayoutWidget::last_selected_layout_name();
  if(!last.isEmpty()) {
    layout_w_->select_preset(last);
  }
  apply_preset_to_preview(layout_w_->current_preset());
}


/**
   Clears and rebuilds the scene for a label of @p w_mm × @p h_mm mm.
   If @p saved is non-null the text element position is taken from it;
   otherwise default placement is used.
   @param w_mm  label width in mm.
   @param h_mm  label height in mm.
   @param saved previously saved layout, or nullptr for default placement.
*/
void colman::SignatureLabelEditor::rebuild_card(
    double w_mm, double h_mm, SignatureLabelLayout const* saved)
{
  card_w_mm_ = w_mm;
  card_h_mm_ = h_mm;

  scene_->clear();
  text_item_ = nullptr;

  qreal const cw = mm(w_mm);
  qreal const ch = mm(h_mm);

  scene_->setSceneRect(0, 0, cw, ch);
  scene_->addRect(0, 0, cw, ch,
                  QPen(Qt::black, 3), QBrush(Qt::white));

  QRectF const card(0, 0, cw, ch);

  qreal const mg = mm(margin_mm_);
  qreal const bw = std::max(0.0, cw - 2.0 * mg);
  qreal const bh = std::max(0.0, ch - 2.0 * mg);
  QRectF const bounds(mg, mg, bw, bh);
  if(mg > 0.0) {
    auto* mr = scene_->addRect(mg, mg, bw, bh,
                               QPen(QColor(150, 150, 150), 1, Qt::DashLine), Qt::NoBrush);
    mr->setZValue(0.5);
  }

  QRectF text_rect;
  int text_orientation = 0;

  if(saved) {
    double const min_mm = 5.0;
    double const cx =
        std::max(0.0, std::min(saved->text_x_mm_, w_mm - min_mm));
    double const cy =
        std::max(0.0, std::min(saved->text_y_mm_, h_mm - min_mm));
    double const cw2 =
        std::max(min_mm, std::min(saved->text_width_mm_, w_mm - cx));
    double const ch2 =
        std::max(min_mm, std::min(saved->text_height_mm_, h_mm - cy));
    text_rect = QRectF(mm(cx), mm(cy), mm(cw2), mm(ch2));
    text_orientation = saved->text_orientation_;
  }
  else {
    text_rect = QRectF(mg, mg, bw, bh);
  }

  auto* tx = new colman::CardElementItem(
      colman::IdCardElementType::Text, text_rect, card);
  {
    QStringList lines;
    lines << tr("Sample Signature");
    tx->set_text_lines(lines, 2);
  }
  tx->set_text_halign(Qt::AlignHCenter);
  if(text_orientation != 0) {
    tx->set_initial_orientation(text_orientation);
  }
  scene_->addItem(tx);
  tx->set_card_bounds(bounds);
  text_item_ = tx;

  view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}


/**
   Returns the text-zone arrangement currently shown in the scene.
*/
colman::SignatureLabelLayout
colman::SignatureLabelEditor::current_sig_arrangement() const
{
  colman::SignatureLabelLayout lay;
  lay.card_width_mm_ = card_w_mm_;
  lay.card_height_mm_ = card_h_mm_;
  lay.margin_mm_ = margin_mm_;
  if(text_item_) {
    colman::CardElementItem const* item =
        static_cast<colman::CardElementItem const*>(text_item_);
    QRectF const sr = item->scene_rect();
    lay.text_x_mm_ = to_mm(sr.x());
    lay.text_y_mm_ = to_mm(sr.y());
    lay.text_width_mm_ = to_mm(sr.width());
    lay.text_height_mm_ = to_mm(sr.height());
    lay.text_orientation_ = item->orientation();
  }
  lay.label_layout_name_ = current_layout_name_;
  return lay;
}


/**
   Loads @p p's dimensions and text-zone arrangement into the preview.
   When the preset carries no arrangement the scene uses default
   (full-label) text placement.
*/
void colman::SignatureLabelEditor::apply_preset_to_preview(
    colman::LabelLayoutPreset const& p)
{
  current_layout_name_ = p.name;
  card_w_mm_ = p.label_width;
  card_h_mm_ = p.label_height;
  colman::SignatureLabelLayout const& arr = p.sig_arrangement;
  bool const has_arr = arr.card_width_mm_ > 0.0;
  margin_mm_ = has_arr ? std::max(0.0, std::min(10.0, arr.margin_mm_)) : 0.0;
  if(margin_slider_) {
    QSignalBlocker b(margin_slider_);
    margin_slider_->setValue(qRound(margin_mm_ * 10.0));
  }
  if(margin_value_label_) {
    margin_value_label_->setText(
        QString("%1 mm").arg(margin_mm_, 0, 'f', 1));
  }
  rebuild_card(card_w_mm_, card_h_mm_, has_arr ? &arr : nullptr);
}


/**
   Assembles the cohesive layout (geometry + printer + text arrangement)
   from the embedded chooser and the scene, then saves it under its name
   as a user layout.  This is the single Save for the editor page.
*/
void colman::SignatureLabelEditor::save_layout()
{
  if(!text_item_) {
    return;
  }
  colman::LabelLayoutPreset p = layout_w_->current_preset();
  QString err;
  if(!colman::LabelLayoutWidget::validate_save_name(p.name, &err)) {
    QMessageBox::warning(this, tr("Cannot save layout"), err);
    return;
  }
  p.sig_arrangement = current_sig_arrangement();
  layout_w_->set_current_sig_arrangement(p.sig_arrangement);
  colman::LabelLayoutWidget::save_user_layout(p);
  QSettings().setValue("label_layouts/last_selected", p.name);
  layout_w_->reselect_after_save(p.name);
}


/**
   Catches QEvent::Resize on the view to keep the preview fitted when
   the user resizes the window.
*/
bool colman::SignatureLabelEditor::eventFilter(QObject* obj, QEvent* e)
{
  if(obj == view_ && e->type() == QEvent::Resize && scene_) {
    QTimer::singleShot(0, this, [this] {
      if(scene_ && view_) {
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
      }
    });
  }
  else if(obj == view_->viewport() && e->type() == QEvent::Wheel) {
    auto* we = static_cast<QWheelEvent*>(e);
    if(we->modifiers() & Qt::ControlModifier) {
      qreal const factor = we->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
      view_->scale(factor, factor);
      return true;
    }
  }
  return QWidget::eventFilter(obj, e);
}


/**
   Defers fitInView to the next event-loop iteration so that both the
   layout and the scene are fully set up before the initial zoom is
   applied.
*/
void colman::SignatureLabelEditor::showEvent(QShowEvent* e)
{
  QWidget::showEvent(e);
  QTimer::singleShot(0, this, [this] {
    if(scene_ && view_) {
      view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    }
  });
}


/**
   Restores the scene to the text arrangement of the currently selected
   layout, or to default placement when it carries none.
*/
void colman::SignatureLabelEditor::reset_layout()
{
  apply_preset_to_preview(layout_w_->current_preset());
}

/* EOF */
