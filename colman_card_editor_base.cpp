// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class CardEditorBase.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_card_editor_base.hpp"
#ifndef COLMAN_CARD_ELEMENT_ITEM_HPP
#  include "colman_card_element_item.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
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
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPEN
#  include <QPen>
#  define SYSTEM_QPEN
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QRESIZEEVENT
#  include <QResizeEvent>
#  define SYSTEM_QRESIZEEVENT
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSLIDER
#  include <QSlider>
#  define SYSTEM_QSLIDER
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QWHEELEVENT
#  include <QWheelEvent>
#  define SYSTEM_QWHEELEVENT
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
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

} // namespace


// ====================================================================
// colman::CardEditorBase
// ====================================================================

/**
   Constructs the base, storing @p queries.  Derived constructors must
   call setup_editor() to complete the initialisation.
   @param queries  database queries.
   @param parent   parent widget.
*/
colman::CardEditorBase::CardEditorBase(IQueries& queries, QWidget* parent)
    : QWidget(parent),
      queries_(queries)
{
}


/**
   Builds the complete UI, connects all signals, and performs the
   initial rebuild_card() call.

   Must be called exactly once from the derived class constructor, after
   the derived object is fully constructed so that virtual dispatch works.

   @param group_label    label shown on the card-size group box.
   @param margin_tooltip tooltip text for the margin slider.
   @param settings_key   QSettings key prefix (e.g. "id_editor").
   @param purpose        purpose tag for layouts created in this editor.
*/
void colman::CardEditorBase::setup_editor(
    QString const& group_label,
    QString const& margin_tooltip,
    QString const& settings_key,
    colman::LabelLayoutPurpose purpose)
{
  settings_key_ = settings_key;

  {
    QSettings s;
    double const v = s.value(settings_key_ + "/margin_mm", 0.0).toDouble();
    margin_mm_ = std::max(0.0, std::min(10.0, v));
  }

  auto* vl = new QVBoxLayout(this);
  vl->setContentsMargins(0, 0, 0, 0);

  // ---- card-size selection ----------------------------------------
  auto* grp = new QGroupBox(group_label, this);
  auto* grp_vl = new QVBoxLayout(grp);
  grp_vl->setContentsMargins(4, 4, 4, 4);
  layout_w_ = new colman::LabelLayoutWidget(grp);
  layout_w_->set_purpose(purpose);
  grp_vl->addWidget(layout_w_);
  vl->addWidget(grp);

  // ---- card preview / editor --------------------------------------
  scene_ = new QGraphicsScene(this);
  view_ = new QGraphicsView(scene_, this);
  view_->setRenderHint(QPainter::Antialiasing);
  view_->setDragMode(QGraphicsView::NoDrag);
  view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  view_->installEventFilter(this);
  view_->viewport()->installEventFilter(this);
  vl->addWidget(view_, 1);

  // Selecting a layout (or New) loads its geometry and element arrangement.
  connect(layout_w_, &colman::LabelLayoutWidget::layout_applied,
          [this](colman::LabelLayoutPreset const& p) {
            apply_preset_to_preview(p);
          });

  // Live geometry edits resize the preview but keep the current arrangement.
  connect(layout_w_, &colman::LabelLayoutWidget::geometry_changed,
          [this](colman::LabelLayoutPreset const& p) {
            auto const dims = card_dims(p);
            card_w_mm_ = dims.first;
            card_h_mm_ = dims.second;
            IdCardLayout const cur =
                clamp_layout(current_layout(), dims.first, dims.second);
            rebuild_card(dims.first, dims.second, &cur);
          });

  // ---- Margin slider ----------------------------------------------
  {
    auto* mhl = new QHBoxLayout;
    auto* mlbl = new QLabel(tr("Margin:"), this);
    margin_slider_ = new QSlider(Qt::Horizontal, this);
    margin_slider_->setRange(0, 100);       // 0.0 – 10.0 mm in 0.1 mm steps
    margin_slider_->setValue(qRound(margin_mm_ * 10.0));
    margin_slider_->setMaximumWidth(180);
    margin_slider_->setToolTip(margin_tooltip);
    auto* mval = new QLabel(
        QString("%1 mm").arg(margin_mm_, 0, 'f', 1), this);
    mval->setMinimumWidth(45);
    mhl->addWidget(mlbl);
    mhl->addWidget(margin_slider_);
    mhl->addWidget(mval);
    mhl->addStretch();
    vl->addLayout(mhl);

    connect(margin_slider_, &QSlider::valueChanged,
            [this, mval](int v) {
              margin_mm_ = v / 10.0;
              mval->setText(
                  QString("%1 mm").arg(margin_mm_, 0, 'f', 1));
              QSettings().setValue(
                  settings_key_ + "/margin_mm", margin_mm_);
              IdCardLayout const lay = current_layout();
              rebuild_card(card_w_mm_, card_h_mm_, &lay);
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

  colman::Collection const init_col = queries_.read_collection();
  bool const init_has_logo = !init_col.get_logo_raw().isEmpty();

  logo_check_ = new QCheckBox(tr("Show logo"), this);
  logo_check_->setChecked(init_has_logo);
  logo_check_->setEnabled(init_has_logo);
  hl->insertWidget(0, logo_check_);

  connect(save_btn_, &QPushButton::clicked,
          this, &colman::CardEditorBase::save_layout);
  connect(reset_btn_, &QPushButton::clicked,
          this, &colman::CardEditorBase::reset_layout);
  connect(logo_check_, &QCheckBox::toggled,
          [this](bool) { rebuild_card(card_w_mm_, card_h_mm_); });

  // Load the last-used layout into the combo, then build the initial
  // preview from whatever is currently selected (covers the case where no
  // layout is remembered or the remembered one no longer exists).
  QString const last = colman::LabelLayoutWidget::last_selected_layout_name();
  if(!last.isEmpty()) {
    layout_w_->select_preset(last);
  }
  apply_preset_to_preview(layout_w_->current_preset());
}


/**
   Returns the layout as currently arranged in the editor.
*/
colman::IdCardLayout colman::CardEditorBase::current_layout() const
{
  IdCardLayout layout;
  layout.card_width_mm_ = card_w_mm_;
  layout.card_height_mm_ = card_h_mm_;

  auto extract = [](QGraphicsItem const* gi, IdCardElement& elem) {
    if(!gi) {
      return;
    }
    colman::CardElementItem const* item =
        static_cast<colman::CardElementItem const*>(gi);
    elem.type_ = item->element_type();
    elem.orientation_ = item->orientation();
    QRectF const sr = item->scene_rect();
    elem.x_mm_ = to_mm(sr.x());
    elem.y_mm_ = to_mm(sr.y());
    elem.width_mm_ = to_mm(sr.width());
    elem.height_mm_ = to_mm(sr.height());
  };

  extract(barcode_item_, layout.barcode_);
  extract(text_item_, layout.text_);
  if(logo_item_) {
    extract(logo_item_, layout.logo_);
    layout.has_logo_ = true;
  }
  return layout;
}


/**
   Clears and rebuilds the scene for a card of @p w_mm × @p h_mm mm.
   If @p layout is non-null its element positions are used; otherwise
   default positions are computed so the zones do not overlap.
   @param w_mm   card width in mm.
   @param h_mm   card height in mm.
   @param layout optional saved layout to restore (may be nullptr).
*/
void colman::CardEditorBase::rebuild_card(double w_mm, double h_mm,
                                          IdCardLayout const* layout)
{
  card_w_mm_ = w_mm;
  card_h_mm_ = h_mm;

  scene_->clear();
  barcode_item_ = nullptr;
  text_item_ = nullptr;
  logo_item_ = nullptr;

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

  // Fetch content data
  colman::Collection const col = queries_.read_collection();
  QString barcode_text = preview_barcode(col);
  QPixmap logo;
  logo.loadFromData(col.get_logo_raw());
  bool const col_has_logo = !logo.isNull();
  bool const show_logo = col_has_logo && (!logo_check_ || logo_check_->isChecked());

  auto elem_rect = [](colman::IdCardElement const& e) -> QRectF {
    return QRectF(mm(e.x_mm_), mm(e.y_mm_),
                  mm(e.width_mm_), mm(e.height_mm_));
  };

  QRectF barcode_rect;
  QRectF text_rect;
  QRectF logo_rect;
  int barcode_orientation = 0;
  int text_orientation = 0;
  int logo_orientation = 0;
  bool logo_from_layout = false;

  if(layout) {
    barcode_rect = elem_rect(layout->barcode_);
    barcode_orientation = layout->barcode_.orientation_;
    text_rect = elem_rect(layout->text_);
    text_orientation = layout->text_.orientation_;
    if(layout->has_logo_ && show_logo) {
      logo_rect = elem_rect(layout->logo_);
      logo_orientation = layout->logo_.orientation_;
      logo_from_layout = true;
    }
  }

  if(!layout || (layout->has_logo_ && !show_logo)) {
    qreal const top_h = bh * default_text_height_fraction();
    qreal const bot_y = mg + top_h;
    qreal const bot_h = bh - top_h;
    barcode_rect = QRectF(mg, bot_y, bw, bot_h);
    qreal logo_x_end = 0;
    if(show_logo && logo.height() > 0) {
      double const ratio = (double)logo.width() / logo.height();
      qreal logo_h = top_h;
      qreal logo_w = logo_h * ratio;
      if(logo_w > bw * 0.35) {
        logo_w = bw * 0.35;
        logo_h = logo_w / ratio;
      }
      logo_rect = QRectF(mg, mg, logo_w, logo_h);
      logo_x_end = logo_w;
      logo_from_layout = false;
    }
    text_rect = QRectF(mg + logo_x_end, mg, bw - logo_x_end, top_h);
  }

  // ---- create items ----------------------------------------------
  auto* bc = new colman::CardElementItem(
      colman::IdCardElementType::Barcode, barcode_rect, card);
  bc->set_barcode_text(barcode_text);
  if(barcode_orientation != 0) {
    bc->set_initial_orientation(barcode_orientation);
  }
  scene_->addItem(bc);
  bc->set_card_bounds(bounds);
  barcode_item_ = bc;

  auto* tx = new colman::CardElementItem(
      colman::IdCardElementType::Text, text_rect, card);
  {
    QStringList const lines = preview_text_lines(col);
    tx->set_text_lines(lines, text_emph_line());
  }
  configure_text_item(tx);
  if(text_orientation != 0) {
    tx->set_initial_orientation(text_orientation);
  }
  scene_->addItem(tx);
  tx->set_card_bounds(bounds);
  text_item_ = tx;

  if(show_logo) {
    auto* lg = new colman::CardElementItem(
        colman::IdCardElementType::Logo, logo_rect, card);
    if(col.has_svg_logo()) {
      lg->set_logo_raw(col.get_logo_raw());
    }
    else {
      lg->set_logo_pixmap(logo);
    }
    if(logo_from_layout && logo_orientation != 0) {
      lg->set_initial_orientation(logo_orientation);
    }
    scene_->addItem(lg);
    lg->set_card_bounds(bounds);
    logo_item_ = lg;
  }

  view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}


/**
   Returns @p layout with every element clamped to fit within
   @p w_mm × @p h_mm.
   @param layout the layout to clamp (passed by value).
   @param w_mm   card width in mm.
   @param h_mm   card height in mm.
*/
colman::IdCardLayout colman::CardEditorBase::clamp_layout(
    IdCardLayout layout, double w_mm, double h_mm)
{
  double const min_mm = 5.0;
  auto clamp = [&](colman::IdCardElement& e) {
    e.x_mm_ = std::max(0.0, std::min(e.x_mm_, w_mm - min_mm));
    e.y_mm_ = std::max(0.0, std::min(e.y_mm_, h_mm - min_mm));
    e.width_mm_ = std::max(min_mm,
                           std::min(e.width_mm_, w_mm - e.x_mm_));
    e.height_mm_ = std::max(min_mm,
                            std::min(e.height_mm_, h_mm - e.y_mm_));
  };
  clamp(layout.barcode_);
  clamp(layout.text_);
  if(layout.has_logo_) {
    clamp(layout.logo_);
  }
  layout.card_width_mm_ = w_mm;
  layout.card_height_mm_ = h_mm;
  return layout;
}


/**
   Loads @p p's card dimensions and element arrangement into the preview.
   When the preset carries no arrangement (a fresh or geometry-only layout)
   the scene is rebuilt with default element placement.
*/
void colman::CardEditorBase::apply_preset_to_preview(
    colman::LabelLayoutPreset const& p)
{
  auto const dims = card_dims(p);
  card_w_mm_ = dims.first;
  card_h_mm_ = dims.second;
  IdCardLayout const& arr = p.card_arrangement;
  bool const has_arr = arr.card_width_mm_ > 0.0;
  if(logo_check_ && has_arr
     && !queries_.read_collection().get_logo_raw().isEmpty()) {
    QSignalBlocker b(logo_check_);
    logo_check_->setChecked(arr.has_logo_);
  }
  if(has_arr) {
    IdCardLayout const clamped = clamp_layout(arr, dims.first, dims.second);
    rebuild_card(dims.first, dims.second, &clamped);
  }
  else {
    rebuild_card(dims.first, dims.second);
  }
}


/**
   Assembles the cohesive layout (geometry + printer + element arrangement)
   from the embedded chooser and the scene, then saves it under its name as
   a user layout.  This is the single Save for the whole editor page.
*/
void colman::CardEditorBase::save_layout()
{
  colman::LabelLayoutPreset p = layout_w_->current_preset();
  QString err;
  if(!colman::LabelLayoutWidget::validate_save_name(p.name, &err)) {
    QMessageBox::warning(this, tr("Cannot save layout"), err);
    return;
  }
  p.card_arrangement = clamp_layout(current_layout(), card_w_mm_, card_h_mm_);
  layout_w_->set_current_card_arrangement(p.card_arrangement);
  colman::LabelLayoutWidget::save_user_layout(p);
  QSettings().setValue("label_layouts/last_selected", p.name);
  layout_w_->reselect_after_save(p.name);
}


/**
   Restores the scene to the element arrangement of the currently selected
   layout, or to default placement when it carries none.
*/
void colman::CardEditorBase::reset_layout()
{
  apply_preset_to_preview(layout_w_->current_preset());
}


/**
   Catches QEvent::Resize on the view to keep the preview fitted when
   the user resizes the window, and Ctrl+Wheel to zoom.
*/
bool colman::CardEditorBase::eventFilter(QObject* obj, QEvent* e)
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
void colman::CardEditorBase::showEvent(QShowEvent* e)
{
  QWidget::showEvent(e);
  QTimer::singleShot(0, this, [this] {
    if(scene_ && view_) {
      view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    }
  });
}


/// Default implementation: no-op.
void colman::CardEditorBase::configure_text_item(CardElementItem*) const
{
}

/* EOF */
