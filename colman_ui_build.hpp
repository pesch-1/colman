// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Small helpers for building the hand-written main-window UI.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_UI_BUILD_HPP
#  define COLMAN_UI_BUILD_HPP

#  ifndef SYSTEM_QSIZEPOLICY
#    include <QSizePolicy>
#    define SYSTEM_QSIZEPOLICY
#  endif
#  ifndef SYSTEM_QFONT
#    include <QFont>
#    define SYSTEM_QFONT
#  endif
#  ifndef SYSTEM_QLAYOUT
#    include <QLayout>
#    define SYSTEM_QLAYOUT
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QSTYLE
#    include <QStyle>
#    define SYSTEM_QSTYLE
#  endif

class QWidget;
class QAbstractItemView;
class QSplitter;
class QLabel;

namespace colman
{

  /// Applies a size policy to @p w (stretch and height-for-width as Designer).
  /**
     Reproduces the size-policy boilerplate that Qt Designer emits: build a
     QSizePolicy with the given policies and stretch factors, copy the widget's
     height-for-width flag, then apply it.
     @param w        widget to configure.
     @param h        horizontal size policy.
     @param v        vertical size policy.
     @param hstretch horizontal stretch factor (default 0).
     @param vstretch vertical stretch factor (default 0).
  */
  void set_size_policy(QWidget* w, QSizePolicy::Policy h, QSizePolicy::Policy v,
                       int hstretch = 0, int vstretch = 0);

  /// Applies the standard single-row selection configuration to @p view.
  /**
     Sets alternating row colours, single-selection mode and row-selection
     behaviour — the configuration shared by the item/type/history views.
     @param view view to configure.
  */
  void configure_item_view(QAbstractItemView* view);

  /// Creates a QSplitter with the given orientation as a child of @p parent.
  /**
     @param parent      parent widget.
     @param orientation splitter orientation.
     @return the new splitter.
  */
  QSplitter* make_splitter(QWidget* parent, Qt::Orientation orientation);

  /// Creates a fixed-size QLabel showing a scaled pixmap from a resource.
  /**
     Reproduces the owl/logo icon labels: a fixed size policy, a square maximum
     size, the pixmap and scaled contents.
     @param parent   parent widget.
     @param resource Qt resource path of the image.
     @param max_size square maximum size in pixels (default 128).
     @return the new label.
  */
  QLabel* make_icon_label(QWidget* parent, char const* resource,
                          int max_size = 128);

  /// Returns a bold copy of the default application font (for headings).
  QFont bold_font();

  /// A layout that arranges its items left to right and wraps to new rows.
  /**
     Adapted from the Qt "Flow Layout" example.  Unlike QHBoxLayout, the
     minimum width of a FlowLayout is the width of its widest single item
     rather than the sum of all items, so a row of buttons no longer forces
     a wide minimum on its containing panel — it simply wraps onto further
     rows when space is tight.
  */
  class FlowLayout : public QLayout
  {
  public:
    /// Constructs the layout as the layout of @p parent.
    /**
       @param parent    widget that owns the layout.
       @param margin    contents margin in pixels (-1 = style default).
       @param h_spacing horizontal spacing in pixels (-1 = style default).
       @param v_spacing vertical spacing in pixels (-1 = style default).
    */
    explicit FlowLayout(QWidget* parent, int margin = -1, int h_spacing = -1,
                        int v_spacing = -1);
    /// Constructs a free-standing layout (to be added to another layout).
    /**
       @param margin    contents margin in pixels (-1 = style default).
       @param h_spacing horizontal spacing in pixels (-1 = style default).
       @param v_spacing vertical spacing in pixels (-1 = style default).
    */
    explicit FlowLayout(int margin = -1, int h_spacing = -1,
                        int v_spacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem* item) override;
    /// Returns the horizontal spacing between items.
    int horizontal_spacing() const;
    /// Returns the vertical spacing between rows.
    int vertical_spacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(QRect const& rect) override;
    QSize sizeHint() const override;
    QLayoutItem* takeAt(int index) override;

  private:
    /// Lays the items out within @p rect; returns the total height needed.
    /**
       @param rect      area to lay out in.
       @param test_only when true, only compute the height (no geometry set).
       @return the height needed for the laid-out items.
    */
    int do_layout_(QRect const& rect, bool test_only) const;
    /// Resolves a spacing value from the parent style for metric @p pm.
    int smart_spacing_(QStyle::PixelMetric pm) const;

    QList<QLayoutItem*> item_list_;
    int h_space_ = -1;
    int v_space_ = -1;
  };

} // namespace colman

#endif /* #ifndef COLMAN_UI_BUILD_HPP */
/* EOF */
