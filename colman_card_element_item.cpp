// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of CardElementItem.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_card_element_item.hpp"
#ifndef COLMAN_CODE128_ITEM_HPP
#  include "colman_code128_item.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSSCENECONTEXTMENUEVENT
#  include <QGraphicsSceneContextMenuEvent>
#  define SYSTEM_QGRAPHICSSCENECONTEXTMENUEVENT
#endif
#ifndef SYSTEM_QGRAPHICSSCENEMOUSEEVENT
#  include <QGraphicsSceneMouseEvent>
#  define SYSTEM_QGRAPHICSSCENEMOUSEEVENT
#endif
#ifndef SYSTEM_QMENU
#  include <QMenu>
#  define SYSTEM_QMENU
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPEN
#  include <QPen>
#  define SYSTEM_QPEN
#endif
#ifndef SYSTEM_QTRANSFORM
#  include <QTransform>
#  define SYSTEM_QTRANSFORM
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef SYSTEM_QFONTMETRICS
#  include <QFontMetrics>
#  define SYSTEM_QFONTMETRICS
#endif
#ifndef SYSTEM_QSVGRENDERER
#  include <QSvgRenderer>
#  define SYSTEM_QSVGRENDERER
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{

  /// Half the handle square side length in scene units.
  qreal constexpr HandleHalf = 10.0;

  /// Full handle square side length in scene units.
  qreal constexpr HandleSize = HandleHalf * 2.0;

  /// Minimum item width/height in scene units (= 5 mm).
  qreal constexpr MinSize = 50.0;

  /// Renders a Code128 barcode for @p text into a pixmap of the given
  /// pixel dimensions.  The barcode is centred vertically.
  QPixmap render_barcode_pixmap(QString const& text, int w, int h)
  {
    auto* bc = new colman::Code128Item;
    bc->set_text(text);
    int const bc_h = (int)(h * 0.8);
    bc->adapt_size(w - 4, bc_h, w);
    QGraphicsScene tmp;
    tmp.addItem(bc);
    tmp.setSceneRect(bc->boundingRect());
    QPixmap px(w, h);
    px.fill(Qt::white);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    int const y_off = (h - bc_h) / 2;
    tmp.render(&p, QRectF(0, y_off, w, bc_h), tmp.sceneRect());
    return px;
  }

  /// Returns whether orientation is 90° or 270° (dims swapped).
  bool is_odd_rotation(int orientation)
  {
    return ((orientation / 90) % 2) != 0;
  }

} // anonymous namespace


// ====================================================================
// colman::CardElementItem
// ====================================================================

/**
   Constructs the item at the given scene rect, constrained to
   @p card_bounds.
*/
colman::CardElementItem::CardElementItem(
    IdCardElementType type,
    QRectF const& scene_rect,
    QRectF const& card_bounds)
    : type_(type),
      rect_(QRectF(QPointF(0, 0), scene_rect.size())),
      card_bounds_(card_bounds)
{
  setPos(scene_rect.topLeft());
}


/**
   Assigns the library logo pixmap (Logo zones only).
*/
void colman::CardElementItem::set_logo_pixmap(QPixmap const& px)
{
  logo_raw_ = QByteArray();
  is_svg_logo_ = false;
  logo_orig_ = px;
  if(!logo_orig_.isNull() && logo_orig_.height() > 0) {
    logo_ratio_ = (double)logo_orig_.width() / logo_orig_.height();
  }
  regenerate_logo_pixmap();
}


/**
   Assigns raw SVG logo bytes; renders vector at paint time (Logo zones).
*/
void colman::CardElementItem::set_logo_raw(QByteArray const& raw)
{
  logo_raw_ = raw;
  logo_orig_ = QPixmap();
  logo_cache_ = QPixmap();
  QByteArray const t = raw.trimmed();
  is_svg_logo_ = (t.startsWith("<?xml") || t.startsWith("<svg"));
  if(is_svg_logo_) {
    QSvgRenderer renderer(raw);
    QSize const sz = renderer.defaultSize();
    if(sz.width() > 0 && sz.height() > 0) {
      logo_ratio_ = (double)sz.width() / sz.height();
    }
  }
  update();
}


/**
   Sets the barcode text and regenerates the pixmap (Barcode zones only).
*/
void colman::CardElementItem::set_barcode_text(QString const& text)
{
  barcode_text_ = text;
  regenerate_barcode_pixmap();
}


/**
   Sets the text lines shown in the Text zone.
   @param lines     lines to display (one per row).
   @param emph_line index of the line rendered larger and bold (-1 for none).
*/
void colman::CardElementItem::set_text_lines(
    QStringList const& lines, int emph_line)
{
  text_lines_ = lines;
  emph_line_ = emph_line;
  update();
}


/// Sets the horizontal text alignment within the Text zone.
void colman::CardElementItem::set_text_halign(Qt::AlignmentFlag align)
{
  text_halign_ = align;
  update();
}


/// Controls how the emphasized line is rendered (italic or bold+larger).
void colman::CardElementItem::set_text_emph_italic(bool italic)
{
  text_emph_italic_ = italic;
  update();
}


/**
   Returns the initial orientation without changing the rect dimensions.
*/
void colman::CardElementItem::set_initial_orientation(int deg)
{
  orientation_ = deg % 360;
  if(((deg / 90) % 2) != 0 && logo_ratio_ > 0) {
    logo_ratio_ = 1.0 / logo_ratio_;
  }
  regenerate_logo_pixmap();
}


/**
   Rotates the zone by 90°, swapping dimensions.
   For Logo zones the aspect ratio is updated; for Barcode zones the
   pixmap is regenerated in the new natural orientation.
*/
void colman::CardElementItem::rotate_90()
{
  QSizeF const new_size(rect_.height(), rect_.width());
  // Reject immediately if the rotated size does not fit in the card at all.
  if(new_size.width() > card_bounds_.width() ||
     new_size.height() > card_bounds_.height()) {
    return;
  }
  // Clamp position so the rotated rect stays within card bounds.
  QPointF p = pos();
  p.setX(std::max(card_bounds_.left(),
                  std::min(p.x(), card_bounds_.right() - new_size.width())));
  p.setY(std::max(card_bounds_.top(),
                  std::min(p.y(), card_bounds_.bottom() - new_size.height())));
  if(would_overlap(QRectF(p, new_size))) {
    return;
  }
  orientation_ = (orientation_ + 90) % 360;
  if(type_ == IdCardElementType::Logo && logo_ratio_ > 0) {
    logo_ratio_ = 1.0 / logo_ratio_;
  }
  prepareGeometryChange();
  rect_ = QRectF(0, 0, new_size.width(), new_size.height());
  setPos(p);
  if(type_ == IdCardElementType::Logo) {
    regenerate_logo_pixmap();
  }
  else if(type_ == IdCardElementType::Barcode) {
    regenerate_barcode_pixmap();
  }
  else {
    update();
  }
}


/**
   Rotates the zone by 180° (dimensions stay the same).
*/
void colman::CardElementItem::rotate_180()
{
  orientation_ = (orientation_ + 180) % 360;
  if(type_ == IdCardElementType::Logo) {
    regenerate_logo_pixmap();
  }
  else {
    update();
  }
}


QRectF colman::CardElementItem::scene_rect() const
{
  return QRectF(pos(), rect_.size());
}


QRectF colman::CardElementItem::boundingRect() const
{
  return rect_.adjusted(-1, -1, 1, 1);
}


void colman::CardElementItem::paint(
    QPainter* painter,
    QStyleOptionGraphicsItem const*,
    QWidget*)
{
  painter->setBrush(Qt::transparent);
  painter->setPen(QPen(QColor(120, 120, 120), 2));
  painter->drawRect(rect_);

  painter->save();
  painter->setClipRect(rect_);
  switch(type_) {
  case IdCardElementType::Barcode:
    draw_barcode(painter);
    break;
  case IdCardElementType::Text:
    draw_text(painter);
    break;
  case IdCardElementType::Logo:
    draw_logo(painter);
    break;
  }
  painter->restore();

  draw_handles(painter);
}


void colman::CardElementItem::set_card_bounds(QRectF const& bounds)
{
  card_bounds_ = bounds;
  qreal const x = std::max(bounds.left(),
                           std::min(pos().x(), bounds.right() - rect_.width()));
  qreal const y = std::max(bounds.top(),
                           std::min(pos().y(), bounds.bottom() - rect_.height()));
  setPos(x, y);
}


void colman::CardElementItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
  if(e->button() != Qt::LeftButton) {
    e->ignore();
    return;
  }
  Handle h = handle_at(e->pos());
  // Logo zone only uses corner handles
  if(type_ == IdCardElementType::Logo && (h == Handle::Top || h == Handle::Right || h == Handle::Bottom || h == Handle::Left))
  {
    h = Handle::NoHandle;
  }
  drag_pos_ = e->scenePos();
  item_pos_ = pos();
  drag_rect_ = rect_;
  if(h != Handle::NoHandle) {
    active_handle_ = h;
  }
  else if(rect_.contains(e->pos())) {
    moving_ = true;
  }
  e->accept();
}


void colman::CardElementItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
  QPointF const delta = e->scenePos() - drag_pos_;
  if(active_handle_ != Handle::NoHandle) {
    apply_resize(delta);
  }
  else if(moving_) {
    QPointF new_pos = item_pos_ + delta;
    new_pos.setX(
        std::max(card_bounds_.left(),
                 std::min(new_pos.x(),
                          card_bounds_.right() - rect_.width())));
    new_pos.setY(
        std::max(card_bounds_.top(),
                 std::min(new_pos.y(),
                          card_bounds_.bottom() - rect_.height())));
    if(!would_overlap(QRectF(new_pos, rect_.size()))) {
      setPos(new_pos);
    }
  }
  e->accept();
}


void colman::CardElementItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
  bool const was_resizing = (active_handle_ != Handle::NoHandle);
  if(was_resizing && type_ == IdCardElementType::Barcode && rect_.size() != drag_rect_.size())
  {
    regenerate_barcode_pixmap();
  }
  active_handle_ = Handle::NoHandle;
  moving_ = false;
  e->accept();
}


void colman::CardElementItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent* e)
{
  QMenu menu;

  QAction* act90 = menu.addAction(QApplication::tr("Rotate 90\xC2\xB0"));
  QAction* act180 = menu.addAction(QApplication::tr("Rotate 180\xC2\xB0"));
  menu.addSeparator();
  QAction* actLeft = menu.addAction(QApplication::tr("Align left"));
  QAction* actRight = menu.addAction(QApplication::tr("Align right"));
  QAction* actHCenter = menu.addAction(
      QApplication::tr("Center horizontally"));
  menu.addSeparator();
  QAction* actTop = menu.addAction(QApplication::tr("Align top"));
  QAction* actBottom = menu.addAction(QApplication::tr("Align bottom"));
  QAction* actVCenter = menu.addAction(
      QApplication::tr("Center vertically"));

  QAction* chosen = menu.exec(e->screenPos());
  if(chosen == act90) {
    rotate_90();
  }
  if(chosen == act180) {
    rotate_180();
  }
  if(chosen == actLeft) {
    align_left();
  }
  if(chosen == actRight) {
    align_right();
  }
  if(chosen == actHCenter) {
    align_hcenter();
  }
  if(chosen == actTop) {
    align_top();
  }
  if(chosen == actBottom) {
    align_bottom();
  }
  if(chosen == actVCenter) {
    align_vcenter();
  }
  e->accept();
}


void colman::CardElementItem::mouseDoubleClickEvent(
    QGraphicsSceneMouseEvent* e)
{
  if(e->button() != Qt::LeftButton) {
    e->ignore();
    return;
  }
  maximize_in_card();
  e->accept();
}


// ---- private: hit testing ------------------------------------------

colman::CardElementItem::Handle
colman::CardElementItem::handle_at(QPointF const& p) const
{
  qreal const w = rect_.width();
  qreal const h = rect_.height();
  auto hr = [](qreal tx, qreal ty) -> QRectF {
    return QRectF(tx - HandleHalf, ty - HandleHalf,
                  HandleSize, HandleSize);
  };
  if(hr(HandleHalf, HandleHalf).contains(p)) {
    return Handle::TopLeft;
  }
  if(hr(w / 2, HandleHalf).contains(p)) {
    return Handle::Top;
  }
  if(hr(w - HandleHalf, HandleHalf).contains(p)) {
    return Handle::TopRight;
  }
  if(hr(w - HandleHalf, h / 2).contains(p)) {
    return Handle::Right;
  }
  if(hr(w - HandleHalf, h - HandleHalf).contains(p)) {
    return Handle::BottomRight;
  }
  if(hr(w / 2, h - HandleHalf).contains(p)) {
    return Handle::Bottom;
  }
  if(hr(HandleHalf, h - HandleHalf).contains(p)) {
    return Handle::BottomLeft;
  }
  if(hr(HandleHalf, h / 2).contains(p)) {
    return Handle::Left;
  }
  return Handle::NoHandle;
}


// ---- private: resize logic -----------------------------------------

void colman::CardElementItem::apply_resize(QPointF const& delta)
{
  QRectF r = drag_rect_;
  QPointF p = item_pos_;
  switch(active_handle_) {
  case Handle::TopLeft:
    p += delta;
    r.setWidth(r.width() - delta.x());
    r.setHeight(r.height() - delta.y());
    break;
  case Handle::Top:
    p.ry() += delta.y();
    r.setHeight(r.height() - delta.y());
    break;
  case Handle::TopRight:
    p.ry() += delta.y();
    r.setWidth(r.width() + delta.x());
    r.setHeight(r.height() - delta.y());
    break;
  case Handle::Right:
    r.setWidth(r.width() + delta.x());
    break;
  case Handle::BottomRight:
    r.setWidth(r.width() + delta.x());
    r.setHeight(r.height() + delta.y());
    break;
  case Handle::Bottom:
    r.setHeight(r.height() + delta.y());
    break;
  case Handle::BottomLeft:
    p.rx() += delta.x();
    r.setWidth(r.width() - delta.x());
    r.setHeight(r.height() + delta.y());
    break;
  case Handle::Left:
    p.rx() += delta.x();
    r.setWidth(r.width() - delta.x());
    break;
  default:
    return;
  }

  bool const moves_left = (active_handle_ == Handle::TopLeft || active_handle_ == Handle::Left || active_handle_ == Handle::BottomLeft);
  bool const moves_top = (active_handle_ == Handle::TopLeft || active_handle_ == Handle::Top || active_handle_ == Handle::TopRight);
  bool const moves_right = (active_handle_ == Handle::TopRight || active_handle_ == Handle::Right || active_handle_ == Handle::BottomRight);
  bool const moves_bottom = (active_handle_ == Handle::BottomLeft || active_handle_ == Handle::Bottom || active_handle_ == Handle::BottomRight);
  if(moves_left && p.x() < card_bounds_.left()) {
    r.setWidth(r.width() - (card_bounds_.left() - p.x()));
    p.setX(card_bounds_.left());
  }
  if(moves_top && p.y() < card_bounds_.top()) {
    r.setHeight(r.height() - (card_bounds_.top() - p.y()));
    p.setY(card_bounds_.top());
  }
  if(moves_right && p.x() + r.width() > card_bounds_.right()) {
    r.setWidth(card_bounds_.right() - p.x());
  }
  if(moves_bottom && p.y() + r.height() > card_bounds_.bottom()) {
    r.setHeight(card_bounds_.bottom() - p.y());
  }

  if(r.width() < MinSize || r.height() < MinSize) {
    return;
  }

  if(type_ == IdCardElementType::Logo && logo_ratio_ > 0) {
    enforce_logo_ratio(active_handle_, r, p);
    if(r.width() < MinSize || r.height() < MinSize) {
      return;
    }
  }

  if(would_overlap(QRectF(p, r.size()))) {
    bool const corner = (active_handle_ == Handle::TopLeft ||
                         active_handle_ == Handle::TopRight ||
                         active_handle_ == Handle::BottomLeft ||
                         active_handle_ == Handle::BottomRight);
    if(type_ != IdCardElementType::Logo && corner) {
      QPointF const h_p(p.x(), item_pos_.y());
      QRectF const h_r(0, 0, r.width(), drag_rect_.height());
      if(h_r.width() >= MinSize && !would_overlap(QRectF(h_p, h_r.size()))) {
        prepareGeometryChange();
        rect_ = h_r;
        setPos(h_p);
        if(type_ == IdCardElementType::Barcode) {
          regenerate_barcode_pixmap();
        }
        update();
      }
    }
    return;
  }

  prepareGeometryChange();
  rect_ = QRectF(0, 0, r.width(), r.height());
  setPos(p);
}


void colman::CardElementItem::enforce_logo_ratio(
    Handle h, QRectF& r, QPointF& pos_out) const
{
  qreal const new_h = r.width() / logo_ratio_;
  switch(h) {
  case Handle::TopLeft:
    pos_out.setY(item_pos_.y() + drag_rect_.height() - new_h);
    break;
  case Handle::TopRight:
    pos_out.setY(item_pos_.y() + drag_rect_.height() - new_h);
    break;
  case Handle::BottomRight:
    break;
  case Handle::BottomLeft:
    break;
  default:
    break;
  }
  r.setHeight(new_h);
}


// ---- private: painting ---------------------------------------------

void colman::CardElementItem::draw_handles(QPainter* p) const
{
  p->setBrush(Qt::white);
  p->setPen(QPen(Qt::black, 1));
  qreal const w = rect_.width();
  qreal const h = rect_.height();
  bool const logo = (type_ == IdCardElementType::Logo);
  struct
  {
    qreal x, y;
    bool edge;
  } pts[] = {
      {HandleHalf, HandleHalf, false},  // Handle::TopLeft
      {w / 2, HandleHalf, true},  // Top
      {w - HandleHalf, HandleHalf, false},  // Handle::TopRight
      {w - HandleHalf, h / 2, true},  // Right
      {w - HandleHalf, h - HandleHalf, false},  // Handle::BottomRight
      {w / 2, h - HandleHalf, true},  // Bottom
      {HandleHalf, h - HandleHalf, false},  // Handle::BottomLeft
      {HandleHalf, h / 2, true}   // Left
  };
  for(auto const& pt : pts) {
    if(logo && pt.edge) {
      continue;
    }
    p->drawRect(QRectF(pt.x - HandleHalf, pt.y - HandleHalf,
                       HandleSize, HandleSize));
  }
}


void colman::CardElementItem::draw_barcode(QPainter* p) const
{
  if(barcode_px_.isNull()) {
    return;
  }
  if(orientation_ == 0) {
    p->drawPixmap(rect_.toRect(), barcode_px_);
    return;
  }
  qreal const cx = rect_.width() / 2.0;
  qreal const cy = rect_.height() / 2.0;
  p->translate(cx, cy);
  p->rotate(orientation_);
  QRectF draw_rect;
  if(is_odd_rotation(orientation_)) {
    draw_rect = QRectF(-cy, -cx, rect_.height(), rect_.width());
  }
  else {
    draw_rect = QRectF(-cx, -cy, rect_.width(), rect_.height());
  }
  p->drawPixmap(draw_rect.toRect(), barcode_px_);
}


void colman::CardElementItem::draw_text(QPainter* p) const
{
  p->setPen(Qt::black);
  int const n = text_lines_.size();
  if(n == 0) {
    return;
  }

  // Set up rotation transform and determine the drawing rect.
  QRectF draw_rect;
  if(orientation_ != 0) {
    qreal const cx = rect_.width() / 2.0;
    qreal const cy = rect_.height() / 2.0;
    p->translate(cx, cy);
    p->rotate(orientation_);
    if(is_odd_rotation(orientation_)) {
      draw_rect = QRectF(-cy, -cx, rect_.height(), rect_.width());
    }
    else {
      draw_rect = QRectF(-cx, -cy, rect_.width(), rect_.height());
    }
  }
  else {
    draw_rect = rect_;
  }

  qreal const line_h = draw_rect.height() / n;
  int const px_small = std::max(1, (int)(line_h * 0.55));
  int const px_large = std::max(1, (int)(line_h * 0.75));
  qreal const avail_w = draw_rect.width() - 4.0;

  for(int i = 0; i < n; ++i) {
    bool const emph = (i == emph_line_);
    QFont font("Arial");
    if(text_emph_italic_) {
      font.setPixelSize(px_small);
      font.setItalic(emph);
    }
    else {
      font.setPixelSize(emph ? px_large : px_small);
      font.setBold(emph);
    }

    // Scale font down if text exceeds available width.
    QString const& text = text_lines_.at(i);
    if(!text.isEmpty() && avail_w > 0) {
      int const advance = QFontMetrics(font).horizontalAdvance(text);
      if(advance > (int)avail_w) {
        int const new_px = std::max(1, (int)(font.pixelSize() * avail_w / advance));
        font.setPixelSize(new_px);
      }
    }

    p->setFont(font);
    QRectF const lr(draw_rect.x(), draw_rect.y() + i * line_h,
                    draw_rect.width(), line_h);
    p->drawText(lr, Qt::AlignVCenter | text_halign_, text);
  }
}


void colman::CardElementItem::draw_logo(QPainter* p) const
{
  if(is_svg_logo_ && !logo_raw_.isEmpty()) {
    QSvgRenderer renderer(logo_raw_);
    p->save();
    if(orientation_ != 0) {
      qreal const rw = rect_.width();
      qreal const rh = rect_.height();
      p->translate(rw / 2.0, rh / 2.0);
      p->rotate(orientation_);
      if((orientation_ % 180) != 0) {
        renderer.render(p, QRectF(-rh / 2.0, -rw / 2.0, rh, rw));
      }
      else {
        renderer.render(p, QRectF(-rw / 2.0, -rh / 2.0, rw, rh));
      }
    }
    else {
      renderer.render(p, rect_);
    }
    p->restore();
  }
  else if(!logo_cache_.isNull()) {
    p->drawPixmap(rect_.toRect(), logo_cache_);
  }
}


// ---- private: pixmap regeneration ----------------------------------

void colman::CardElementItem::regenerate_barcode_pixmap()
{
  if(barcode_text_.isEmpty()) {
    return;
  }
  // Generate the barcode in its natural (landscape) orientation.
  // When rotated 90°/270°, rect_ has swapped dimensions; use the
  // original width/height to keep bars readable.
  int pw, ph;
  if(is_odd_rotation(orientation_)) {
    pw = std::max(100, (int)(rect_.height() * 2));
    ph = std::max(50, (int)(rect_.width() * 2));
  }
  else {
    pw = std::max(100, (int)(rect_.width() * 2));
    ph = std::max(50, (int)(rect_.height() * 2));
  }
  barcode_px_ = render_barcode_pixmap(barcode_text_, pw, ph);
  update();
}


void colman::CardElementItem::regenerate_logo_pixmap()
{
  if(logo_orig_.isNull()) {
    logo_cache_ = QPixmap();
    update();
    return;
  }
  int const pw = std::max(1, (int)(rect_.width() * 2));
  int const ph = std::max(1, (int)(rect_.height() * 2));
  if(orientation_ == 0) {
    logo_cache_ = logo_orig_.scaled(pw, ph,
                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation);
  }
  else {
    QTransform t;
    t.rotate(orientation_);
    QPixmap const rotated = logo_orig_.transformed(t, Qt::SmoothTransformation);
    logo_cache_ = rotated.scaled(pw, ph,
                                 Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
  }
  update();
}


// ---- private: free space / maximize / align ------------------------

QRectF colman::CardElementItem::free_space_rect() const
{
  qreal left = card_bounds_.left();
  qreal right = card_bounds_.right();
  qreal top = card_bounds_.top();
  qreal bottom = card_bounds_.bottom();

  qreal const my_left = pos().x();
  qreal const my_right = pos().x() + rect_.width();
  qreal const my_top = pos().y();
  qreal const my_bottom = pos().y() + rect_.height();
  qreal const my_cx = (my_left + my_right) / 2.0;
  qreal const my_cy = (my_top + my_bottom) / 2.0;

  if(scene()) {
    for(QGraphicsItem* it : scene()->items()) {
      CardElementItem* other = dynamic_cast<CardElementItem*>(it);
      if(!other || other == this) {
        continue;
      }
      QRectF const sr = other->scene_rect();
      if(sr.right() <= my_cx && sr.bottom() > my_top && sr.top() < my_bottom) {
        left = std::max(left, sr.right());
      }
      if(sr.left() >= my_cx && sr.bottom() > my_top && sr.top() < my_bottom) {
        right = std::min(right, sr.left());
      }
      if(sr.bottom() <= my_cy && sr.right() > my_left && sr.left() < my_right) {
        top = std::max(top, sr.bottom());
      }
      if(sr.top() >= my_cy && sr.right() > my_left && sr.left() < my_right) {
        bottom = std::min(bottom, sr.top());
      }
    }
  }
  return QRectF(left, top, right - left, bottom - top);
}


void colman::CardElementItem::align_left()
{
  setPos(free_space_rect().left(), pos().y());
}

void colman::CardElementItem::align_right()
{
  QRectF const fs = free_space_rect();
  setPos(fs.right() - rect_.width(), pos().y());
}

void colman::CardElementItem::align_hcenter()
{
  QRectF const fs = free_space_rect();
  setPos(fs.left() + (fs.width() - rect_.width()) / 2.0, pos().y());
}

void colman::CardElementItem::align_top()
{
  setPos(pos().x(), free_space_rect().top());
}

void colman::CardElementItem::align_bottom()
{
  QRectF const fs = free_space_rect();
  setPos(pos().x(), fs.bottom() - rect_.height());
}

void colman::CardElementItem::align_vcenter()
{
  QRectF const fs = free_space_rect();
  setPos(pos().x(), fs.top() + (fs.height() - rect_.height()) / 2.0);
}


void colman::CardElementItem::maximize_in_card()
{
  QRectF const fs = free_space_rect();
  qreal const left = fs.left();
  qreal const top = fs.top();
  qreal new_w = fs.width();
  qreal new_h = fs.height();
  if(new_w < MinSize || new_h < MinSize) {
    return;
  }

  if(type_ == IdCardElementType::Logo && logo_ratio_ > 0) {
    if(new_w / new_h > logo_ratio_) {
      new_w = new_h * logo_ratio_;
    }
    else {
      new_h = new_w / logo_ratio_;
    }
  }

  prepareGeometryChange();
  rect_ = QRectF(0, 0, new_w, new_h);
  setPos(left, top);
  if(type_ == IdCardElementType::Barcode) {
    regenerate_barcode_pixmap();
  }
  else if(type_ == IdCardElementType::Logo) {
    regenerate_logo_pixmap();
    return;
  }
  update();
}


// ---- private: overlap detection ------------------------------------

bool colman::CardElementItem::would_overlap(
    QRectF const& proposed_rect) const
{
  if(!scene()) {
    return false;
  }
  for(QGraphicsItem* it : scene()->items()) {
    CardElementItem* other = dynamic_cast<CardElementItem*>(it);
    if(!other || other == this) {
      continue;
    }
    QRectF const their_rect(other->pos(), other->rect_.size());
    QRectF const inter = proposed_rect.intersected(their_rect);
    if(inter.width() > 1.0 && inter.height() > 1.0) {
      return true;
    }
  }
  return false;
}


/* EOF */
