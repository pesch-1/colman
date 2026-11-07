// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of the camera barcode scan page.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CAMERA_SCAN_PAGE_HPP
#  include "colman_camera_scan_page.hpp"
#  define COLMAN_CAMERA_SCAN_PAGE_HPP
#endif
#ifndef SYSTEM_QCAMERA
#  include <QCamera>
#  define SYSTEM_QCAMERA
#endif
#ifndef SYSTEM_QCAMERADEVICE
#  include <QCameraDevice>
#  define SYSTEM_QCAMERADEVICE
#endif
#ifndef SYSTEM_QCAMERAFORMAT
#  include <QCameraFormat>
#  define SYSTEM_QCAMERAFORMAT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QMEDIACAPTURESESSION
#  include <QMediaCaptureSession>
#  define SYSTEM_QMEDIACAPTURESESSION
#endif
#ifndef SYSTEM_QMEDIADEVICES
#  include <QMediaDevices>
#  define SYSTEM_QMEDIADEVICES
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPAINTEREVENT
#  include <QPaintEvent>
#  define SYSTEM_QPAINTEREVENT
#endif
#ifndef SYSTEM_QPAINTERPATH
#  include <QPainterPath>
#  define SYSTEM_QPAINTERPATH
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSTACKEDLAYOUT
#  include <QStackedLayout>
#  define SYSTEM_QSTACKEDLAYOUT
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QVIDEOFRAME
#  include <QVideoFrame>
#  define SYSTEM_QVIDEOFRAME
#endif
#ifndef SYSTEM_QVIDEOSINK
#  include <QVideoSink>
#  define SYSTEM_QVIDEOSINK
#endif
#ifndef SYSTEM_ZXING_READBARCODE
#  include <ZXing/ReadBarcode.h>
#  define SYSTEM_ZXING_READBARCODE
#endif

namespace
{
  /// Smallest allowed scan-window scale factor.
  double constexpr MinScanScale = 0.6;
  /// Largest allowed scan-window scale factor.
  double constexpr MaxScanScale = 3.0;
  /// Amount the scale factor changes per click of the +/- buttons.
  double constexpr ScanScaleStep = 0.2;
  /// QSettings key under which the scan-window scale factor is persisted.
  char const* const ScanScaleKey = "camera_scan/window_scale";

  /// Computes the scan-window rectangle (in widget pixels) for a given size.
  /**
     Single source of truth shared by the overlay (which dims everything
     outside this rectangle) and the decoder (which only reads inside it), so
     that what the user frames is exactly what gets decoded. The base size is
     multiplied by @a scale so the user can enlarge the window for low
     resolution cameras, but the result never exceeds the widget bounds.
     @param size  the size of the preview/overlay widget
     @param scale the user-chosen scan-window scale factor
     @return the centred scan-window rectangle
  */
  QRect scan_window_rect(QSize const& size, double scale)
  {
    int const base_w = qMin(360, size.width() * 4 / 5);
    int const base_h = qMin(160, size.height() * 2 / 3);
    int const hole_w = qMin(int(base_w * scale), size.width());
    int const hole_h = qMin(int(base_h * scale), size.height());
    return QRect{
        (size.width() - hole_w) / 2,
        (size.height() - hole_h) / 2,
        hole_w,
        hole_h};
  }

  /// Picks a video format whose resolution is close to a target for scanning.
  /**
     A higher resolution preserves the fine bars of a barcode, but very large
     frames are expensive to convert and decode on every frame, so an
     intermediate resolution (~1280x720) is the better trade-off. Among formats
     equally close to the target the one with the higher frame rate wins.
     @param formats the video formats reported by the camera device
     @return the chosen format, or a null format if @a formats is empty
  */
  QCameraFormat select_camera_format(QList<QCameraFormat> const& formats)
  {
    int constexpr TargetArea = 1280 * 720;
    QCameraFormat best;
    int best_diff = 0;
    for(auto const& fmt : formats) {
      QSize const res = fmt.resolution();
      if(res.isEmpty()) {
        continue;
      }
      int const diff = qAbs(res.width() * res.height() - TargetArea);
      if(best.isNull() || diff < best_diff
         || (diff == best_diff && fmt.maxFrameRate() > best.maxFrameRate())) {
        best = fmt;
        best_diff = diff;
      }
    }
    return best;
  }

  /// Transparent overlay that draws a scan-window frame and corner brackets.
  class ScanOverlay : public QWidget
  {
  public:
    explicit ScanOverlay(QWidget* parent = nullptr) : QWidget(parent)
    {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      setAttribute(Qt::WA_NoSystemBackground);
    }

    /// Returns the current scan-window scale factor.
    double scale() const
    {
      return scale_;
    }

    /// Sets the scan-window scale factor (clamped) and repaints the overlay.
    void set_scale(double scale)
    {
      scale_ = qBound(MinScanScale, scale, MaxScanScale);
      update();
    }

  protected:
    void paintEvent(QPaintEvent*) override
    {
      QPainter p(this);
      QRect const win = rect();
      QRect const hole = scan_window_rect(win.size(), scale_);

      QPainterPath path;
      path.addRect(win);
      path.addRect(hole);
      p.fillPath(path, QColor(0, 0, 0, 140));

      int const arm = 20;
      p.setPen(QPen(Qt::white, 3));
      p.drawLine(hole.left(),         hole.top() + arm, hole.left(),  hole.top());
      p.drawLine(hole.left(),         hole.top(),  hole.left() + arm, hole.top());
      p.drawLine(hole.right() - arm,  hole.top(),  hole.right(),      hole.top());
      p.drawLine(hole.right(),         hole.top(),  hole.right(),      hole.top() + arm);
      p.drawLine(hole.left(),          hole.bottom() - arm, hole.left(),  hole.bottom());
      p.drawLine(hole.left(),          hole.bottom(), hole.left() + arm, hole.bottom());
      p.drawLine(hole.right() - arm,   hole.bottom(), hole.right(),      hole.bottom());
      p.drawLine(hole.right(),          hole.bottom() - arm, hole.right(), hole.bottom());
    }

  private:
    double scale_ = 1.0; ///< User-chosen scan-window scale factor.
  };

  /// Stacked-widget page showing a live camera preview that decodes barcodes.
  /**
     The camera is created lazily on the first start() and reused afterwards.
     start() begins scanning; the camera is stopped when a barcode is found or
     the Back button is pressed.
  */
  class CameraScanPage : public QWidget
  {
  public:
    /// Constructs the page; the camera is created on the first start().
    explicit CameraScanPage(QWidget* parent = nullptr) : QWidget(parent)
    {
      auto* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer->addWidget(back_btn_);

      auto* container = new QWidget(this);
      auto* stack = new QStackedLayout(container);
      stack->setStackingMode(QStackedLayout::StackAll);

      preview_ = new QLabel(container);
      preview_->setAlignment(Qt::AlignCenter);
      preview_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      preview_->setStyleSheet("background: black; color: white;");
      stack->addWidget(preview_);

      overlay_ = new ScanOverlay(container);
      overlay_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      // Restore the scan-window size the user last chose, so a camera with low
      // resolution keeps the larger window across sessions.
      overlay_->set_scale(
          QSettings().value(ScanScaleKey, 1.0).toDouble());
      stack->addWidget(overlay_);
      // In StackAll mode every widget stays visible, but the current one is
      // raised to the top; make that the overlay so its dimming and brackets
      // are drawn over the preview rather than hidden behind it.
      stack->setCurrentWidget(overlay_);

      outer->addWidget(container, 1);

      hint_ = new QLabel(QApplication::tr("Hold barcode in front of camera"),
                         this);
      hint_->setAlignment(Qt::AlignCenter);
      outer->addWidget(hint_);

      // Buttons to enlarge or shrink the scan window. A smaller window is
      // faster and easier to aim; a larger one helps low-resolution cameras
      // where the barcode bars are coarse and need more pixels to resolve.
      auto* zoom_row = new QHBoxLayout;
      zoom_row->addStretch(1);
      zoom_row->addWidget(
          new QLabel(QApplication::tr("Scan area:"), this));
      auto* zoom_out_btn = new QPushButton(QStringLiteral("−"), this);
      auto* zoom_in_btn = new QPushButton(QStringLiteral("+"), this);
      zoom_out_btn->setToolTip(QApplication::tr("Shrink scan area"));
      zoom_in_btn->setToolTip(QApplication::tr("Enlarge scan area"));
      zoom_out_btn->setFixedWidth(48);
      zoom_in_btn->setFixedWidth(48);
      zoom_row->addWidget(zoom_out_btn);
      zoom_row->addWidget(zoom_in_btn);
      zoom_row->addStretch(1);
      outer->addLayout(zoom_row);

      connect(zoom_out_btn, &QPushButton::clicked,
              [this]() { change_scan_scale(-ScanScaleStep); });
      connect(zoom_in_btn, &QPushButton::clicked,
              [this]() { change_scan_scale(ScanScaleStep); });

      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                stop();
                if(on_back_) {
                  on_back_();
                }
              });
    }

    /// Sets the callback invoked when the Back button is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

    /// Starts scanning; \a on_scanned is invoked once a barcode is recognised.
    void start(std::function<void(QString const&)> on_scanned)
    {
      on_scanned_ = std::move(on_scanned);
      detected_ = false;

      if(QMediaDevices::videoInputs().isEmpty()) {
        preview_->setText(QApplication::tr("No camera available."));
        hint_->hide();
        return;
      }
      hint_->show();

      if(!camera_) {
        camera_ = new QCamera(QMediaDevices::defaultVideoInput(), this);
        sink_ = new QVideoSink(this);
        session_ = new QMediaCaptureSession(this);
        session_->setCamera(camera_);
        session_->setVideoSink(sink_);

        // Prefer continuous autofocus so barcodes stay sharp; ignore silently
        // if the camera is fixed-focus and does not support it.
        if(camera_->isFocusModeSupported(QCamera::FocusModeAuto)) {
          camera_->setFocusMode(QCamera::FocusModeAuto);
        }

        // Pick a barcode-friendly resolution when the device offers a choice.
        QCameraFormat const format =
            select_camera_format(camera_->cameraDevice().videoFormats());
        if(!format.isNull()) {
          camera_->setCameraFormat(format);
        }

        connect(sink_, &QVideoSink::videoFrameChanged,
                this,
                [this](QVideoFrame const& frame) { on_frame(frame); },
                Qt::QueuedConnection);
      }
      camera_->start();
    }

    /// Stops the camera if it is running.
    void stop()
    {
      if(camera_) {
        camera_->stop();
      }
    }

  private:
    /// Changes the scan-window scale by @a delta and persists the new value.
    /**
       @param delta amount added to the current scale factor (may be negative)
    */
    void change_scan_scale(double delta)
    {
      overlay_->set_scale(overlay_->scale() + delta);
      QSettings().setValue(ScanScaleKey, overlay_->scale());
    }

    /// Renders one frame and tries to decode a barcode from it.
    void on_frame(QVideoFrame const& frame)
    {
      auto img = frame.toImage();
      if(img.isNull()) {
        return;
      }

      preview_->setPixmap(
          QPixmap::fromImage(img).scaled(
              preview_->size(),
              Qt::KeepAspectRatio,
              Qt::SmoothTransformation));

      if(detected_) {
        return;
      }

      // Map the visible scan window back onto the camera image so that only
      // the framed region is decoded. The preview shows the image scaled with
      // KeepAspectRatio and centred, so account for the letterbox offsets.
      QSize const view_size = preview_->size();
      QSize const shown = img.size().scaled(view_size, Qt::KeepAspectRatio);
      if(shown.isEmpty()) {
        return;
      }
      int const off_x = (view_size.width() - shown.width()) / 2;
      int const off_y = (view_size.height() - shown.height()) / 2;
      QRect const win = scan_window_rect(view_size, overlay_->scale());
      double const inv = double(img.width()) / double(shown.width());
      QRect crop{
          int((win.x() - off_x) * inv),
          int((win.y() - off_y) * inv),
          int(win.width() * inv),
          int(win.height() * inv)};
      crop = crop.intersected(img.rect());
      if(crop.isEmpty()) {
        return;
      }

      auto gray = img.copy(crop).convertToFormat(QImage::Format_Grayscale8);
      ZXing::ImageView const view{
          gray.constBits(), gray.width(), gray.height(),
          ZXing::ImageFormat::Lum, int(gray.bytesPerLine())};
      ZXing::ReaderOptions opts;
      opts.setTryHarder(true);
      opts.setFormats(ZXing::BarcodeFormat::Code128
                    | ZXing::BarcodeFormat::EAN8
                    | ZXing::BarcodeFormat::EAN13
                    | ZXing::BarcodeFormat::QRCode);
      auto result = ZXing::ReadBarcode(view, opts);
      if(!result.isValid()) {
        return;
      }

      detected_ = true;
      QString const barcode = QString::fromStdString(result.text());
      stop();
      // Return to the originating page first, then run the callback, so that
      // any navigation the callback performs (e.g. dispatching a scanned
      // barcode to a lending page) is not overridden by the back navigation.
      if(on_back_) {
        on_back_();
      }
      if(on_scanned_) {
        on_scanned_(barcode);
      }
    }

    QPushButton*          back_btn_  = nullptr;
    QLabel*               preview_   = nullptr;
    QLabel*               hint_      = nullptr;
    ScanOverlay*          overlay_   = nullptr;
    QCamera*              camera_    = nullptr;
    QMediaCaptureSession* session_   = nullptr;
    QVideoSink*           sink_      = nullptr;
    std::function<void()> on_back_   = {};
    std::function<void(QString const&)> on_scanned_ = {};
    bool                  detected_  = false;
  };

} // namespace

void colman::setup_camera_scan_page(QApplication&, colman::AppData& data)
{
  auto* page = new CameraScanPage(&data.w_);
  data.mw_.pages_stack_->addWidget(page);
  data.camera_scan_page_ = page;
}

void colman::scan_barcode(colman::AppData& data,
                          std::function<void(QString const&)> on_scanned)
{
  auto* page = static_cast<CameraScanPage*>(data.camera_scan_page_);
  if(!page) {
    return;
  }
  QWidget* origin = data.mw_.pages_stack_->currentWidget();
  page->set_on_back([&data, origin]() {
    data.mw_.pages_stack_->setCurrentWidget(origin);
  });
  page->start(std::move(on_scanned));
  data.mw_.pages_stack_->setCurrentWidget(page);
}

void colman::scan_barcode_into(colman::AppData& data, QLineEdit* field)
{
  scan_barcode(data, [field](QString const& barcode) {
    field->setText(barcode);
    field->setModified(true);
    Q_EMIT field->editingFinished();
  });
}

/* EOF */
