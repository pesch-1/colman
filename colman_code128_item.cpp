// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Code128Item.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_code128_item.hpp"
#ifndef COLMAN_ASSERT_HPP
#  include "colman_assert.hpp"
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif

namespace
{
  int const Code128_rl_lut[107][6] = {
      {2, 1, 2, 2, 2, 2},
      {2, 2, 2, 1, 2, 2},
      {2, 2, 2, 2, 2, 1},
      {1, 2, 1, 2, 2, 3},
      {1, 2, 1, 3, 2, 2},
      {1, 3, 1, 2, 2, 2},
      {1, 2, 2, 2, 1, 3},
      {1, 2, 2, 3, 1, 2},
      {1, 3, 2, 2, 1, 2},
      {2, 2, 1, 2, 1, 3},
      {2, 2, 1, 3, 1, 2},
      {2, 3, 1, 2, 1, 2},
      {1, 1, 2, 2, 3, 2},
      {1, 2, 2, 1, 3, 2},
      {1, 2, 2, 2, 3, 1},
      {1, 1, 3, 2, 2, 2},
      {1, 2, 3, 1, 2, 2},
      {1, 2, 3, 2, 2, 1},
      {2, 2, 3, 2, 1, 1},
      {2, 2, 1, 1, 3, 2},
      {2, 2, 1, 2, 3, 1},
      {2, 1, 3, 2, 1, 2},
      {2, 2, 3, 1, 1, 2},
      {3, 1, 2, 1, 3, 1},
      {3, 1, 1, 2, 2, 2},
      {3, 2, 1, 1, 2, 2},
      {3, 2, 1, 2, 2, 1},
      {3, 1, 2, 2, 1, 2},
      {3, 2, 2, 1, 1, 2},
      {3, 2, 2, 2, 1, 1},
      {2, 1, 2, 1, 2, 3},
      {2, 1, 2, 3, 2, 1},
      {2, 3, 2, 1, 2, 1},
      {1, 1, 1, 3, 2, 3},
      {1, 3, 1, 1, 2, 3},
      {1, 3, 1, 3, 2, 1},
      {1, 1, 2, 3, 1, 3},
      {1, 3, 2, 1, 1, 3},
      {1, 3, 2, 3, 1, 1},
      {2, 1, 1, 3, 1, 3},
      {2, 3, 1, 1, 1, 3},
      {2, 3, 1, 3, 1, 1},
      {1, 1, 2, 1, 3, 3},
      {1, 1, 2, 3, 3, 1},
      {1, 3, 2, 1, 3, 1},
      {1, 1, 3, 1, 2, 3},
      {1, 1, 3, 3, 2, 1},
      {1, 3, 3, 1, 2, 1},
      {3, 1, 3, 1, 2, 1},
      {2, 1, 1, 3, 3, 1},
      {2, 3, 1, 1, 3, 1},
      {2, 1, 3, 1, 1, 3},
      {2, 1, 3, 3, 1, 1},
      {2, 1, 3, 1, 3, 1},
      {3, 1, 1, 1, 2, 3},
      {3, 1, 1, 3, 2, 1},
      {3, 3, 1, 1, 2, 1},
      {3, 1, 2, 1, 1, 3},
      {3, 1, 2, 3, 1, 1},
      {3, 3, 2, 1, 1, 1},
      {3, 1, 4, 1, 1, 1},
      {2, 2, 1, 4, 1, 1},
      {4, 3, 1, 1, 1, 1},
      {1, 1, 1, 2, 2, 4},
      {1, 1, 1, 4, 2, 2},
      {1, 2, 1, 1, 2, 4},
      {1, 2, 1, 4, 2, 1},
      {1, 4, 1, 1, 2, 2},
      {1, 4, 1, 2, 2, 1},
      {1, 1, 2, 2, 1, 4},
      {1, 1, 2, 4, 1, 2},
      {1, 2, 2, 1, 1, 4},
      {1, 2, 2, 4, 1, 1},
      {1, 4, 2, 1, 1, 2},
      {1, 4, 2, 2, 1, 1},
      {2, 4, 1, 2, 1, 1},
      {2, 2, 1, 1, 1, 4},
      {4, 1, 3, 1, 1, 1},
      {2, 4, 1, 1, 1, 2},
      {1, 3, 4, 1, 1, 1},
      {1, 1, 1, 2, 4, 2},
      {1, 2, 1, 1, 4, 2},
      {1, 2, 1, 2, 4, 1},
      {1, 1, 4, 2, 1, 2},
      {1, 2, 4, 1, 1, 2},
      {1, 2, 4, 2, 1, 1},
      {4, 1, 1, 2, 1, 2},
      {4, 2, 1, 1, 1, 2},
      {4, 2, 1, 2, 1, 1},
      {2, 1, 2, 1, 4, 1},
      {2, 1, 4, 1, 2, 1},
      {4, 1, 2, 1, 2, 1},
      {1, 1, 1, 1, 4, 3},
      {1, 1, 1, 3, 4, 1},
      {1, 3, 1, 1, 4, 1},
      {1, 1, 4, 1, 1, 3},
      {1, 1, 4, 3, 1, 1},
      {4, 1, 1, 1, 1, 3},
      {4, 1, 1, 3, 1, 1},
      {1, 1, 3, 1, 4, 1},
      {1, 1, 4, 1, 3, 1},
      {3, 1, 1, 1, 4, 1},
      {4, 1, 1, 1, 3, 1},
      {2, 1, 1, 4, 1, 2},
      {2, 1, 1, 2, 1, 4},
      {2, 1, 1, 2, 3, 2},
      {2, 3, 3, 1, 1, 1} // 2 is missing
  };

  //int const Code128_stop_rl[7]={2,3,3,1,1,1,2};
  //  99: Code C (or in Code C: 99)
  // 100: Code B (or in Code B: FNC 4)
  // 101: Code A (or in Code A: FNC 4)
  // 102: FNC 1
  // 103: Start Code A
  // 104: Start Code B
  // 105: Start Code C
  // Code C: 00 is 0, ... , 99 is 99

  /// computes Code128 B number for character \a c, returns 95 for invalid chars
  int char_to_code_b(int c)
  {
    if(c >= ' ' && c <= '~') {
      return c - ' ';
    }
    return 95;
  }


  /// computes checksum of a Code128 encoding
  /**
     ignores last two numbers (the checksum and the stop code)
  */
  int checksum_code128(std::vector<int> const& code)
  {
    size_t const len = code.size();
    if(len > 2 && len <= std::numeric_limits<int>::max()) {
      int const len2 = (int)len - 2;
      int sum = code[0];
      for(int i = 1; i < len2; ++i) {
        sum += code[i] * i;
      }
      return sum % 103;
    }
    return 0;
  }


  /**
     Encodes an ASCII text as Code128 B
  */
  std::vector<int> ascii_to_code128b(QByteArray const& text)
  {
    int const len = text.length();
    std::vector<int> retval(len + 3);
    for(int i = 0; i < len; ++i) {
      retval[i + 1] = char_to_code_b(text[i]);
    }
    retval[0] = 104; // start code b
    retval[len + 1] = checksum_code128(retval);
    retval[len + 2] = 106; // stop
    return retval;
  }


  /**
     Encodes an ASCII text as Code128 C
  */
  std::vector<int> ascii_to_code128c(QByteArray const& text)
  {
    int const len = text.length();
    int const size = 3 + (len + 1) / 2 + (len & 1);
    std::vector<int> retval(size);
    int j = 0;
    for(int i = 0; i + 1 < len; i += 2, ++j) {
      retval[j + 1] = 10 * (text[i] - '0') + text[i + 1] - '0';
    }
    if((len & 1)) {
      retval[j + 1] = 100; // code b
      retval[j + 2] = char_to_code_b(text[len - 1]);
    }
    retval[0] = 105; // start code c
    retval[size - 2] = checksum_code128(retval);
    retval[size - 1] = 106; // stop
    return retval;
  }

} // namespace


colman::Code128Item::Code128Item(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
}


/**
   Adapts the barcode dimensions to the given display area.
   @param width      available width in scene units.
   @param height     available height in scene units.
   @param width_unit preferred X-dimension in scene units.
*/
void colman::Code128Item::adapt_size(qreal width, qreal height, qreal width_unit)
{
  width_ = width;
  height_ = height;
  unit_w_ = width_unit;
  unit_h_ = 0.7 * height;
  font_h_ = 0.2 * height;
  rect_ = QRectF(0, 0, width, height);
  if(rl_sum_ > 0 && rl_sum_ * unit_w_ > width) {
    unit_w_ = width / rl_sum_;
  }
}


/**
   Reads the user-configured X-dimension and converts it to scene units.
   @return the narrowest-bar width in scene units (1 unit = 0.1 mm)
*/
qreal colman::barcode_unit_scene_units()
{
  QSettings settings;
  qreal const mm =
      settings.value("barcode_unit_mm", colman::DefaultBarcodeUnitMm)
          .toDouble();
  return mm * 10.0;
}


void colman::Code128Item::encode()
{
  QByteArray const utf8 = text_.toUtf8();
  // Code128 C requires every character to be a decimal digit; fall back to
  // Code B when the text contains anything else (e.g. a letter prefix).
  bool all_digits = code_c_;
  if(code_c_) {
    for(int i = 0; i < utf8.size(); ++i) {
      if(utf8[i] < '0' || utf8[i] > '9') {
        all_digits = false;
        break;
      }
    }
  }
  std::vector<int> const code128 = (all_digits
                                        ? ascii_to_code128c(utf8)
                                        : ascii_to_code128b(utf8));
  run_lengths_.clear();
  run_lengths_.resize(code128.size() * 6 + 1);
  rl_sum_ = 0;
  size_t pos = 0;
  for(int code : code128) {
    COLMAN_ASSERT(code < 107)
    (code)(pos / 6);
    for(int i = 0; i < 6; ++i, ++pos) {
      COLMAN_ASSERT(pos < run_lengths_.size())
      (pos)(run_lengths_.size());
      rl_sum_ += (run_lengths_[pos] = Code128_rl_lut[code][i]);
    }
  }
  COLMAN_ASSERT(pos + 1 == run_lengths_.size())
  (pos)(run_lengths_.size());
  rl_sum_ += (run_lengths_[pos] = 2); // adds the missing 2 in stop code
}


/**
   sets text to be encoded into barcode
*/
void colman::Code128Item::set_text(QString const& text)
{
  if(text != text_) {
    text_ = text;
    encode();
  }
}


/**
   @return text to be encoded into barcode
*/
QString colman::Code128Item::text() const
{
  return text_;
}


/**
   @param b true to use Code128 C (digit pairs), false for Code128 B (ASCII).
*/
void colman::Code128Item::set_use_code_c(bool b)
{
  if(b != code_c_) {
    code_c_ = b;
    encode();
  }
}


/**
   @return whether Code128 C or B is used
*/
bool colman::Code128Item::use_code_c() const
{
  return code_c_;
}


/**
   @param mm bar width reduction in millimetres compensating ink/heat spread
*/
void colman::Code128Item::set_bar_width_reduction(qreal mm)
{
  bar_reduction_mm_ = std::max(qreal(0), mm);
  update();
}


/**
   @return bar width reduction in millimetres
*/
qreal colman::Code128Item::bar_width_reduction() const
{
  return bar_reduction_mm_;
}


/**
   @return rectangle used to display barcode
*/
QRectF colman::Code128Item::boundingRect() const
{
  return rect_;
}


/**
   Implements paint.
*/
void colman::Code128Item::
    paint(QPainter* painter,
          QStyleOptionGraphicsItem const* /*option*/,
          QWidget* /*widget*/)
{
  if(!painter || run_lengths_.empty()) {
    return;
  }
  painter->save();
  painter->setClipRect(rect_, Qt::IntersectClip);
  painter->setBackground(QBrush(Qt::white));
  painter->fillRect(rect_, QBrush(Qt::white));
  painter->translate(rect_.x(), rect_.y());
  QPen p;
  p.setColor(Qt::black);
  p.setWidth(1);
  painter->setPen(p);

  // draw bars:
  // Bar width reduction: shrink every black bar (split evenly on both sides)
  // so the white gaps stay wide enough once a thermal printer's heated dots
  // bleed and widen the black bars again.  One scene unit is 0.1 mm, so the
  // millimetre value is multiplied by 10.  Capped at half the narrowest bar
  // (one module) so a single-module bar never collapses.
  qreal const reduction =
      std::min(bar_reduction_mm_ * 10.0, 0.5 * unit_w_);
  bool black = true;
  qreal xoff = 0.5 * (width_ - unit_w_ * rl_sum_);
  for(int rl : run_lengths_) {
    if(black) {
      painter->fillRect(QRectF(xoff + 0.5 * reduction, 0,
                               unit_w_ * (qreal)rl - reduction, unit_h_),
                        QBrush(Qt::black));
    }
    xoff += unit_w_ * (qreal)rl;
    black = !black;
  }
  // draw text below barcode:
  QFont font_small("Arial");
  font_small.setPixelSize(font_h_);
  painter->setFont(font_small);
  painter->drawText(0, unit_h_ + 0.05 * height_, width_, font_h_,
                    Qt::AlignCenter, text_);
  painter->restore();
}

/* EOF */
