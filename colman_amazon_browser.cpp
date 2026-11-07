// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of the Amazon browser page and ImagePickerObject.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_amazon_browser.hpp"
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QWEBENGINEVIEW
#  include <QWebEngineView>
#  define SYSTEM_QWEBENGINEVIEW
#endif
#ifndef SYSTEM_QWEBENGINEPAGE
#  include <QWebEnginePage>
#  define SYSTEM_QWEBENGINEPAGE
#endif
#ifndef SYSTEM_QWEBENGINESCRIPT
#  include <QWebEngineScript>
#  define SYSTEM_QWEBENGINESCRIPT
#endif
#ifndef SYSTEM_QWEBENGINESCRIPTCOLLECTION
#  include <QWebEngineScriptCollection>
#  define SYSTEM_QWEBENGINESCRIPTCOLLECTION
#endif
#ifndef SYSTEM_QWEBCHANNEL
#  include <QWebChannel>
#  define SYSTEM_QWEBCHANNEL
#endif
#ifndef SYSTEM_QNETWORKACCESSMANAGER
#  include <QNetworkAccessManager>
#  define SYSTEM_QNETWORKACCESSMANAGER
#endif
#ifndef SYSTEM_QNETWORKREQUEST
#  include <QNetworkRequest>
#  define SYSTEM_QNETWORKREQUEST
#endif
#ifndef SYSTEM_QNETWORKREPLY
#  include <QNetworkReply>
#  define SYSTEM_QNETWORKREPLY
#endif
#ifndef SYSTEM_QPROGRESSBAR
#  include <QProgressBar>
#  define SYSTEM_QPROGRESSBAR
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
namespace
{
/// JavaScript injected after each page load to intercept image clicks.
/** Waits for QWebChannel to connect, then registers a capture-phase click
    listener that posts the src URL of the topmost <img> ancestor (up to
    five levels) to the C++ ImagePickerObject via the "imagePicker" channel
    object. */
QString const InjectJs = R"js(
(function() {
  new QWebChannel(qt.webChannelTransport, function(channel) {
    var picker = channel.objects.imagePicker;
    document.addEventListener('click', function(e) {
      var el = e.target;
      var depth = 0;
      while(el && el.tagName !== 'IMG' && depth < 5) {
        el = el.parentElement;
        depth++;
      }
      if(el && el.tagName === 'IMG' && el.src &&
         (el.src.indexOf('https://') === 0 ||
          el.src.indexOf('http://') === 0)) {
        e.preventDefault();
        e.stopPropagation();
        picker.image_clicked(el.src);
      }
    }, true);
  });
})();
)js";

/// Amazon.de conditions of use, linked from the copyright notice.
char const* const AmazonConditionsUrl =
    "https://www.amazon.de/gp/help/customer/display.html?nodeId=201909000";
} // namespace

colman::ImagePickerObject::ImagePickerObject(QObject* parent)
    : QObject(parent)
{}

/// Emits image_url_received() with the clicked image URL.
/**
  Called from JavaScript via QWebChannel.
  @param url Absolute URL of the clicked image element.
*/
void colman::ImagePickerObject::image_clicked(QString const& url)
{
  Q_EMIT image_url_received(url);
}

namespace
{
  /// Stacked-widget page with an embedded browser for Amazon cover selection.
  /**
     Sets up a QWebEngineView with a QWebChannel that exposes an
     ImagePickerObject under the name "imagePicker".  qwebchannel.js is injected
     at document-creation time so that InjectJs can reference
     qt.webChannelTransport after every page load.  When the user clicks a
     product image it is downloaded asynchronously; on success on_selected() is
     invoked followed by on_back().
  */
  class AmazonBrowserPage : public QWidget
  {
  public:
    /// Constructs the page and begins loading the Amazon product-search page.
    /** @param search_term EAN, ISBN, or title to use as the search query. */
    explicit AmazonBrowserPage(QString const& search_term,
                               QWidget* parent = nullptr)
        : QWidget(parent)
        , view_(new QWebEngineView(this))
        , nam_(new QNetworkAccessManager(this))
    {
      auto* channel = new QWebChannel(this);
      auto* picker = new colman::ImagePickerObject(this);
      channel->registerObject("imagePicker", picker);
      view_->page()->setWebChannel(channel);

      // Inject the Qt WebChannel bridge at document creation so that
      // qt.webChannelTransport is available when InjectJs runs.
      QWebEngineScript bridge;
      bridge.setName("qwebchannel_bridge");
      bridge.setSourceUrl(QUrl("qrc:///qtwebchannel/qwebchannel.js"));
      bridge.setInjectionPoint(QWebEngineScript::DocumentCreation);
      bridge.setWorldId(QWebEngineScript::MainWorld);
      view_->page()->scripts().insert(bridge);

      auto* back_btn = new QPushButton(QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

      auto* progress_bar = new QProgressBar(this);
      progress_bar->setTextVisible(false);
      progress_bar->setMaximumHeight(4);
      progress_bar->setRange(0, 100);

      auto* hint = new QLabel(
          QApplication::tr("Click a product image to use it as thumbnail."),
          this);
      hint->setAlignment(Qt::AlignCenter);

      auto* legal_notice = new QLabel(
          QApplication::tr(
              "Cover images are protected by copyright; Amazon grants no "
              "rights to them beyond personal, non-commercial use (see the "
              "<a href=\"%1\">Amazon conditions of use</a>). Only adopt an "
              "image if you are permitted to, e.g. for a private collection "
              "or for cataloguing the stock of a non-commercial, publicly "
              "accessible library.")
              .arg(AmazonConditionsUrl),
          this);
      legal_notice->setWordWrap(true);
      legal_notice->setTextFormat(Qt::RichText);
      legal_notice->setOpenExternalLinks(true);
      legal_notice->setContentsMargins(6, 0, 6, 0);

      auto* layout = new QVBoxLayout(this);
      layout->setContentsMargins(0, 0, 0, 6);
      layout->setSpacing(4);
      layout->addWidget(back_btn);
      layout->addWidget(legal_notice);
      layout->addWidget(progress_bar);
      layout->addWidget(view_, 1);
      layout->addWidget(hint);

      connect(view_, &QWebEngineView::loadProgress,
              progress_bar, &QProgressBar::setValue);
      connect(view_, &QWebEngineView::loadFinished,
              this, [this](bool) { view_->page()->runJavaScript(InjectJs); });
      connect(picker, &colman::ImagePickerObject::image_url_received,
              this, [this](QString const& url) { on_image_url(url); });
      connect(back_btn, &QPushButton::clicked,
              this, [this]() {
                if(on_back_) {
                  on_back_();
                }
              });

      QString const encoded = QString::fromUtf8(
          QUrl::toPercentEncoding(search_term));
      view_->load(QUrl("https://www.amazon.de/s?k=" + encoded));
    }

    /// Sets the callback invoked when the Back button is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

    /// Sets the callback invoked with the downloaded cover on success.
    void set_on_selected(std::function<void(QPixmap const&)> on_selected)
    {
      on_selected_ = std::move(on_selected);
    }

  private:
    /// Downloads the clicked image and on success reports it, then returns.
    /**
       Ignores concurrent download attempts and shows a warning if the
       download or image decoding fails.
       @param url Absolute URL of the image to download.
    */
    void on_image_url(QString const& url)
    {
      if(downloading_) {
        return;
      }
      downloading_ = true;
      setCursor(Qt::WaitCursor);

      QString https_url = url;
      if(https_url.startsWith("http://")) {
        https_url.replace(0, 7, "https://");
      }

      auto* reply = nam_->get(QNetworkRequest(QUrl(https_url)));
      connect(reply, &QNetworkReply::finished, this,
      [this, reply]()
      {
        reply->deleteLater();
        unsetCursor();
        downloading_ = false;

        if(reply->error() != QNetworkReply::NoError) {
          QMessageBox::warning(
              this,
              QApplication::tr("Collection Manager"),
              QApplication::tr("Could not download the image."));
          return;
        }

        QByteArray const data = reply->readAll();
        QPixmap pm;
        if(!pm.loadFromData(data) || pm.isNull()) {
          QMessageBox::warning(
              this,
              QApplication::tr("Collection Manager"),
              QApplication::tr("The selected image could not be loaded."));
          return;
        }

        if(on_selected_) {
          on_selected_(pm);
        }
        if(on_back_) {
          on_back_();
        }
      });
    }

    QWebEngineView*        view_        = nullptr;
    QNetworkAccessManager* nam_         = nullptr;
    std::function<void()>  on_back_     = {};
    std::function<void(QPixmap const&)> on_selected_ = {};
    bool                   downloading_ = false;
  };

} // namespace

void colman::open_amazon_browser_page(
    colman::AppData& data, QString const& search_term,
    std::function<void(QPixmap const&)> on_selected)
{
  QWidget* const back = data.mw_.pages_stack_->currentWidget();
  auto* page = new AmazonBrowserPage(search_term);
  data.mw_.pages_stack_->addWidget(page);
  auto go_back = [&data, page, back]() {
    data.mw_.pages_stack_->setCurrentWidget(back);
    data.mw_.pages_stack_->removeWidget(page);
    page->deleteLater();
  };
  page->set_on_selected(std::move(on_selected));
  page->set_on_back(go_back);
  data.mw_.pages_stack_->setCurrentWidget(page);
}

/* EOF */
