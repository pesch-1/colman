// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// QLabel subclass that tracks the application font size
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SCALING_LABEL_HPP
#  define COLMAN_SCALING_LABEL_HPP

#  ifndef SYSTEM_QLABEL
#    include <QLabel>
#    define SYSTEM_QLABEL
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif
#  ifndef SYSTEM_QEVENT
#    include <QEvent>
#    define SYSTEM_QEVENT
#  endif
#  ifndef SYSTEM_QFONT
#    include <QFont>
#    define SYSTEM_QFONT
#  endif

namespace colman
{

/// QLabel whose font tracks the application font plus a fixed point-size offset.
/**
   The label reacts to QEvent::ApplicationFontChange and immediately
   re-applies the computed font.  No deprecated fontChanged signal is used.

   @param pt_delta  added to QApplication::font().pointSize() (may be 0).
   @param bold      if true, font weight is set to bold.
*/
  class ScalingLabel : public QLabel
  {
  public:
  /// Constructs a label with initial text @p text.
    explicit ScalingLabel(QString const& text,
                          QWidget* parent,
                          int pt_delta = 0,
                          bool bold = false)
        : QLabel(text, parent), pt_delta_(pt_delta), bold_(bold)
    {
      apply_font();
    }

  /// Constructs an empty label; set text later via setText().
    explicit ScalingLabel(QWidget* parent,
                          int pt_delta = 0,
                          bool bold = false)
        : QLabel(parent), pt_delta_(pt_delta), bold_(bold)
    {
      apply_font();
    }

  protected:
    void changeEvent(QEvent* e) override
    {
      if(e->type() == QEvent::ApplicationFontChange) {
        apply_font();
      }
      QLabel::changeEvent(e);
    }

  private:
    void apply_font()
    {
      QFont f = QApplication::font();
      f.setPointSize(f.pointSize() + pt_delta_);
    // bold_ is authoritative when set in ctor; otherwise preserve the bold
    // state that may have been set externally (e.g. from a .ui font block).
      if(bold_ || QLabel::font().bold()) {
        f.setBold(true);
      }
      setFont(f);
    }

    int pt_delta_ = 0;
    bool bold_ = false;
  };

} // namespace colman

#endif /* #ifndef COLMAN_SCALING_LABEL_HPP */

/* EOF */
