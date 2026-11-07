// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// QLabel subclass that scales its pixmap to fit available space
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PIXMAP_LABEL_HPP
#  define COLMAN_PIXMAP_LABEL_HPP

#  ifndef SYSTEM_QLABEL
#    include <QLabel>
#    define SYSTEM_QLABEL
#  endif
#  ifndef SYSTEM_QPIXMAP
#    include <QPixmap>
#    define SYSTEM_QPIXMAP
#  endif
#  ifndef SYSTEM_QRESIZEEVENT
#    include <QResizeEvent>
#    define SYSTEM_QRESIZEEVENT
#  endif
#  ifndef SYSTEM_QSIZE
#    include <QSize>
#    define SYSTEM_QSIZE
#  endif
#  ifndef SYSTEM_QSIZEPOLICY
#    include <QSizePolicy>
#    define SYSTEM_QSIZEPOLICY
#  endif

namespace colman
{

/// QLabel that scales its pixmap to fill the available space.
/**
   Stores the original full-resolution pixmap and re-scales it on every
   resize event so the image is never clipped.  The aspect ratio is always
   preserved.  Use set_original_pixmap() instead of setPixmap().

   The original (unscaled) pixmap is available via original_pixmap() and is
   also returned by QLabel::pixmap() so that callers can test for null-ness.
*/
  class PixmapLabel : public QLabel
  {
  public:
    explicit PixmapLabel(QWidget* parent = nullptr) : QLabel(parent)
    {
      setAlignment(Qt::AlignCenter);
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    /// Stores @p pm and immediately shows it scaled to the current size.
    void set_original_pixmap(QPixmap const& pm)
    {
      original_pixmap_ = pm;
      update_display();
    }

    /// Returns the original unscaled pixmap (null if none was set).
    QPixmap const& original_pixmap() const { return original_pixmap_; }

    bool hasHeightForWidth() const override
    {
      return !original_pixmap_.isNull();
    }

    int heightForWidth(int w) const override
    {
      if(original_pixmap_.isNull() || original_pixmap_.width() == 0) {
        return QLabel::heightForWidth(w);
      }
      return (w * original_pixmap_.height()) / original_pixmap_.width();
    }

    QSize sizeHint() const override
    {
      if(!original_pixmap_.isNull()) {
        return original_pixmap_.size().boundedTo(QSize(400, 300));
      }
      return QLabel::sizeHint();
    }

  protected:
    void resizeEvent(QResizeEvent* e) override
    {
      QLabel::resizeEvent(e);
      update_display();
    }

  private:
    void update_display()
    {
      if(original_pixmap_.isNull()) {
        QLabel::setPixmap(QPixmap());
        return;
      }
      QLabel::setPixmap(original_pixmap_.scaled(size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
    }

    QPixmap original_pixmap_;
  };

} // namespace colman

#endif /* #ifndef COLMAN_PIXMAP_LABEL_HPP */

/* EOF */
