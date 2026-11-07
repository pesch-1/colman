// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
#include "colman_screenshot.hpp"
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QBOXLAYOUT
#  include <QBoxLayout>
#  define SYSTEM_QBOXLAYOUT
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QFILEDIALOG
#  include <QFileDialog>
#  define SYSTEM_QFILEDIALOG
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif

namespace
{

int constexpr MaxPreviewW = 640;
int constexpr MaxPreviewH = 480;

/// Dialog showing a screenshot preview with format, quality, and save options.
class ScreenshotDialog : public QDialog
{
public:
  explicit ScreenshotDialog(QPixmap const& pixmap, QWidget* parent)
      : QDialog(parent), pixmap_(pixmap)
  {
    setWindowTitle(QApplication::translate("ScreenshotDialog","Screenshot"));

    preview_ = new QLabel(this);
    preview_->setAlignment(Qt::AlignCenter);
    QPixmap const scaled = pixmap_.scaled(
        MaxPreviewW, MaxPreviewH,
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    preview_->setPixmap(scaled);
    preview_->setFixedSize(scaled.size());

    format_combo_ = new QComboBox(this);
    format_combo_->addItem("JPEG", QString("JPG"));
    format_combo_->addItem("PNG", QString("PNG"));

    quality_label_ = new QLabel(
        QApplication::translate("ScreenshotDialog","Quality:"), this);
    quality_spin_ = new QSpinBox(this);
    quality_spin_->setRange(1, 100);
    quality_spin_->setValue(75);
    quality_spin_->setSuffix(QString::fromUtf8(" %"));

    QSettings settings;
    QString const saved_fmt =
        settings.value("screenshot/format", "JPG").toString();
    int const saved_quality =
        settings.value("screenshot/quality", 75).toInt();
    int const combo_idx = format_combo_->findData(saved_fmt);
    if(combo_idx >= 0) {
      format_combo_->setCurrentIndex(combo_idx);
    }
    quality_spin_->setValue(qBound(1, saved_quality, 100));

    auto* buttons = new QDialogButtonBox(this);
    auto* save_btn =
        buttons->addButton(QApplication::translate("ScreenshotDialog","Save..."),
                           QDialogButtonBox::AcceptRole);
    buttons->addButton(QDialogButtonBox::Cancel);

    auto* format_row = new QHBoxLayout;
    format_row->addWidget(
        new QLabel(QApplication::translate("ScreenshotDialog","Format:"), this));
    format_row->addWidget(format_combo_);
    format_row->addSpacing(16);
    format_row->addWidget(quality_label_);
    format_row->addWidget(quality_spin_);
    format_row->addStretch();

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(preview_, 0, Qt::AlignCenter);
    layout->addLayout(format_row);
    layout->addWidget(buttons);

    update_quality_visibility();

    QObject::connect(
        format_combo_,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int) { update_quality_visibility(); });

    QObject::connect(
        save_btn, &QPushButton::clicked,
        this,
        [this]() { save_screenshot(); });

    QObject::connect(
        buttons, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
  }

private:
  void update_quality_visibility()
  {
    bool const jpeg = format_combo_->currentData().toString() == "JPG";
    quality_label_->setVisible(jpeg);
    quality_spin_->setVisible(jpeg);
  }

  void save_screenshot()
  {
    QString const fmt = format_combo_->currentData().toString();
    int const quality = quality_spin_->value();

    QSettings settings;
    QString const last_dir =
        settings.value("screenshot/last_dir",
                       QDir::homePath()).toString();

    QString filter;
    QString suffix;
    if(fmt == "JPG") {
      filter = QApplication::translate("ScreenshotDialog","JPEG Images (*.jpg)");
      suffix = "jpg";
    }
    else {
      filter = QApplication::translate("ScreenshotDialog","PNG Images (*.png)");
      suffix = "png";
    }

    QString const default_name =
        last_dir + "/" + "screenshot." + suffix;
    QString const path = QFileDialog::getSaveFileName(
        this,
        QApplication::translate("ScreenshotDialog","Save Screenshot"),
        default_name,
        filter);

    if(path.isEmpty()) {
      return;
    }

    bool const ok = pixmap_.save(
        path,
        fmt == "JPG" ? "JPEG" : "PNG",
        fmt == "JPG" ? quality : -1);

    if(!ok) {
      QMessageBox::warning(
          this,
          QApplication::translate("ScreenshotDialog","Screenshot"),
          QApplication::translate("ScreenshotDialog",
              "Could not save screenshot to \"%1\".")
              .arg(path));
      return;
    }

    settings.setValue("screenshot/format", fmt);
    settings.setValue("screenshot/quality", quality);
    settings.setValue("screenshot/last_dir",
                      QFileInfo(path).absolutePath());
    accept();
  }

  QPixmap const pixmap_;
  QLabel* preview_ = nullptr;
  QComboBox* format_combo_ = nullptr;
  QLabel* quality_label_ = nullptr;
  QSpinBox* quality_spin_ = nullptr;
};

} // namespace


namespace colman
{

void take_screenshot()
{
  QWidget* const window = QApplication::activeWindow();
  if(!window) {
    return;
  }
  QPixmap const pixmap = window->grab();
  ScreenshotDialog dlg(pixmap, window);
  dlg.exec();
}

} // namespace colman

/* EOF */
