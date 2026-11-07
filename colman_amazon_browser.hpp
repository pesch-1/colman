// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// In-app browser page for selecting a cover image from Amazon.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_AMAZON_BROWSER_HPP
#  define COLMAN_AMAZON_BROWSER_HPP

#  ifndef SYSTEM_QOBJECT
#    include <QObject>
#    define SYSTEM_QOBJECT
#  endif
#  ifndef SYSTEM_QPIXMAP
#    include <QPixmap>
#    define SYSTEM_QPIXMAP
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif

namespace colman
{
  class AppData;

  /// QObject exposed to the embedded browser via QWebChannel.
  /**
     JavaScript calls image_clicked() when the user taps an \<img\> element.
     The signal image_url_received() is forwarded to the Amazon browser page.
  */
  class ImagePickerObject : public QObject
  {
    Q_OBJECT

  public:
    explicit ImagePickerObject(QObject* parent = nullptr);

    /// Called from JavaScript when the user clicks an image.
    /** @param url Absolute https URL of the clicked image element. */
    Q_INVOKABLE void image_clicked(QString const& url);

  Q_SIGNALS:
    /// Forwarded from image_clicked(); carries the image URL.
    void image_url_received(QString const& url);
  };

  /// Opens an embedded Amazon browser as a transient page on the page stack.
  /**
     Shows an Amazon.de product-search page for @p search_term as a page in the
     main window's stacked widget.  After each page load a JavaScript snippet is
     injected that intercepts clicks on \<img\> elements.  When the user clicks a
     product image it is downloaded asynchronously and, on success, @p on_selected
     is invoked with the downloaded cover before returning to the originating
     page.  The Back button returns without invoking the callback.  The page is
     removed from the stack and deleted once it is left.

     @param data        central application state (provides the page stack).
     @param search_term EAN, ISBN, or title to pass as the Amazon search query.
     @param on_selected callback receiving the downloaded cover image.
  */
  void open_amazon_browser_page(AppData& data, QString const& search_term,
                                std::function<void(QPixmap const&)> on_selected);

} // namespace colman

#endif /* #ifndef COLMAN_AMAZON_BROWSER_HPP */

/* EOF */
