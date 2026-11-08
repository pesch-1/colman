// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Interactive test for colman_label_printer.hpp
/**
   Lets the user select a printer, enter label dimensions (mm), enter a
   text line and a barcode string, preview the rendered image and send
   the label to the printer.

   Scene coordinate convention: 1 unit = 0.1 mm (matches the rest of
   colman).

   Usage:
   @code
   ./colman_test_label_printer
   @endcode

   @file
   @author Christoph Pesch
*/

#include "colman_label_printer.hpp"
#include "colman_code128_item.hpp"

#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QDOUBLESPINBOX
#  include <QDoubleSpinBox>
#  define SYSTEM_QDOUBLESPINBOX
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QFRAME
#  include <QFrame>
#  define SYSTEM_QFRAME
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSTEXTITEM
#  include <QGraphicsTextItem>
#  define SYSTEM_QGRAPHICSTEXTITEM
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPRINTERINFO
#  include <QPrinterInfo>
#  define SYSTEM_QPRINTERINFO
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QSTRINGLIST
#  include <QStringList>
#  define SYSTEM_QSTRINGLIST
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QWIDGET
#  include <QWidget>
#  define SYSTEM_QWIDGET
#endif

namespace
{

/// Builds a QGraphicsScene with a text line and a Code128 barcode.
/**
   The scene uses 1 unit = 0.1 mm.  The scene rect is set to exactly
   @p size_mm * 10 so that render_label_scene() maps it 1:1 to the
   physical label.

   Layout (top to bottom):
   - 5 unit margin
   - text line  (~30 % of label height)
   - 5 unit gap
   - barcode    (remaining height minus bottom margin)
   - 5 unit margin

   Ownership of the returned object is transferred to the caller.

   @param size_mm      physical label size in millimetres
   @param text         text to render above the barcode
   @param barcode_text string to encode in the Code128 barcode;
                       if empty no barcode is added
   @return new QGraphicsScene*; never null
*/
  QGraphicsScene* build_label_scene(QSizeF const& size_mm,
                                    QString const& text,
                                    QString const& barcode_text,
                                    qreal bwr_mm)
  {
    double const w = size_mm.width() * 10.0;  // scene units (0.1 mm each)
    double const h = size_mm.height() * 10.0;

    double const margin = 5.0;
    double const gap = 5.0;
    double const text_h = h * 0.30;
    double const barcode_y = margin + text_h + gap;
    double const barcode_h = h - barcode_y - margin;
    double const content_w = w - 2.0 * margin;

    auto* scene = new QGraphicsScene;
    scene->setSceneRect(0.0, 0.0, w, h);

  // --- text ---
    if(!text.isEmpty()) {
      auto* ti = scene->addText(text);
      QFont font = ti->font();
    // pixelSize in scene coordinates → ~70 % of text_h scene units tall
      font.setPixelSize(qMax(1, static_cast<int>(text_h * 0.70)));
      ti->setFont(font);
      ti->setDefaultTextColor(Qt::black);
      ti->setPos(margin, margin);
    // Scale down horizontally if text is too wide
      double const tw = ti->boundingRect().width();
      if(tw > content_w && tw > 0.0) {
        double const sx = content_w / tw;
        ti->setTransform(QTransform::fromScale(sx, 1.0));
      }
    }

  // --- barcode ---
    if(!barcode_text.isEmpty() && barcode_h > 0.0) {
      auto* bc = new colman::Code128Item;
      bc->set_text(barcode_text);
      bc->adapt_size(content_w, barcode_h, colman::barcode_unit_scene_units());
      bc->set_bar_width_reduction(bwr_mm);
      bc->setPos(margin, barcode_y);
      scene->addItem(bc);
    }

    return scene;
  }

} // anonymous namespace


int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  QApplication::setOrganizationName("cpesch");
  QApplication::setApplicationName("colman");

  // --- main window ---
  QWidget window;
  window.setWindowTitle(QObject::tr("Label Printer Test \xe2\x80\x93 Colman"));

  QVBoxLayout* vbox = new QVBoxLayout(&window);

  // --- form ---
  QFormLayout* form = new QFormLayout;

  // Printer combo: list all printers; mark thermal printers with [thermal]
  QWidget* printer_widget = new QWidget;
  QHBoxLayout* printer_row = new QHBoxLayout(printer_widget);
  printer_row->setContentsMargins(0, 0, 0, 0);

  QComboBox* printer_combo = new QComboBox;
  QPushButton* refresh_btn = new QPushButton(QObject::tr("Refresh"));
  refresh_btn->setToolTip(QObject::tr("Scan for newly connected printers"));
  printer_row->addWidget(printer_combo, 1);
  printer_row->addWidget(refresh_btn);

  // Shows the connected USB printer devices and their paper status;
  // updated together with the printer combo.
  QLabel* device_status_label = new QLabel;

  // Populate (or re-populate) the printer combo, keeping the previous
  // selection when it is still available after a refresh.
  auto populate_printers = [printer_combo, device_status_label]() {
    QString const previous = printer_combo->currentData().toString();
    printer_combo->clear();

    // Report every USB printer device node with its paper status.
    QStringList device_lines;
    for(QString const& dev : colman::list_thermal_printer_devices()) {
      QString status_text;
      switch(colman::query_paper_status(dev)) {
      case colman::PaperStatus::Ok:
        status_text = QObject::tr("paper present");
        break;
      case colman::PaperStatus::Empty:
        status_text = QObject::tr("out of paper");
        break;
      case colman::PaperStatus::Unknown:
        // On Linux the usual cause is a missing read permission on the
        // device node; name it so that the user does not have to debug it.
        status_text = !dev.startsWith("/dev/") || QFileInfo(dev).isReadable()
                          ? QObject::tr("status unknown")
                          : QObject::tr("no read permission — add your "
                                        "user to the \"lp\" group");
        break;
      }
      device_lines.append(QString("%1: %2").arg(dev, status_text));
    }
    device_status_label->setText(
        device_lines.isEmpty()
            ? QObject::tr("(no USB printer device found)")
            : device_lines.join("\n"));

    QList<colman::ThermalPrinterInfo> const thermal =
        colman::list_thermal_printers();
    QSet<QString> thermal_names;
    for(colman::ThermalPrinterInfo const& tp : thermal) {
      thermal_names.insert(tp.cups_name);
    }

    for(QPrinterInfo const& pi : QPrinterInfo::availablePrinters()) {
      QString label = pi.printerName();
      if(thermal_names.contains(pi.printerName())) {
        label += QObject::tr(" [thermal]");
      }
      printer_combo->addItem(label, pi.printerName());
    }
    if(printer_combo->count() == 0) {
      printer_combo->addItem(QObject::tr("(no printers found)"), QString());
      return;
    }
    // Restore previous selection if still present
    int const idx = printer_combo->findData(previous);
    if(idx >= 0) {
      printer_combo->setCurrentIndex(idx);
    }
  };

  populate_printers();

  QObject::connect(refresh_btn, &QPushButton::clicked, populate_printers);

  form->addRow(QObject::tr("Printer:"), printer_widget);
  form->addRow(QObject::tr("USB devices:"), device_status_label);

  // Label size
  QWidget* size_widget = new QWidget;
  QHBoxLayout* size_row = new QHBoxLayout(size_widget);
  size_row->setContentsMargins(0, 0, 0, 0);

  QDoubleSpinBox* width_spin = new QDoubleSpinBox;
  width_spin->setRange(5.0, 300.0);
  width_spin->setDecimals(1);
  width_spin->setValue(62.0);
  width_spin->setSuffix(" mm");

  QDoubleSpinBox* height_spin = new QDoubleSpinBox;
  height_spin->setRange(5.0, 300.0);
  height_spin->setDecimals(1);
  height_spin->setValue(29.0);
  height_spin->setSuffix(" mm");

  size_row->addWidget(new QLabel(QObject::tr("Width:")));
  size_row->addWidget(width_spin);
  size_row->addWidget(new QLabel(QObject::tr("Height:")));
  size_row->addWidget(height_spin);
  size_row->addStretch();
  form->addRow(QObject::tr("Label size:"), size_widget);

  // DPI
  QSpinBox* dpi_spin = new QSpinBox;
  dpi_spin->setRange(72, 600);
  dpi_spin->setValue(203);
  form->addRow(QObject::tr("DPI:"), dpi_spin);

  // Bar width reduction (thermal ink-spread compensation)
  QDoubleSpinBox* bwr_spin = new QDoubleSpinBox;
  bwr_spin->setRange(0.0, 0.5);
  bwr_spin->setDecimals(2);
  bwr_spin->setSingleStep(0.05);
  bwr_spin->setValue(0.0);
  bwr_spin->setSuffix(" mm");
  bwr_spin->setToolTip(QObject::tr(
      "Shrinks each black barcode bar to compensate for ink or heat spread."));
  form->addRow(QObject::tr("Bar width reduction:"), bwr_spin);

  // Print darkness (CUPS Darkness option; -1 keeps the printer default)
  QSpinBox* darkness_spin = new QSpinBox;
  darkness_spin->setRange(-1, 30);
  darkness_spin->setSpecialValueText(QObject::tr("Printer default"));
  darkness_spin->setToolTip(QObject::tr(
      "Thermal print darkness; higher prints darker. "
      "\"Printer default\" leaves the printer's own setting unchanged."));
  form->addRow(QObject::tr("Print darkness:"), darkness_spin);

  // Text
  QLineEdit* text_edit = new QLineEdit("Hello, Label Printer!");
  form->addRow(QObject::tr("Text:"), text_edit);

  // Barcode
  QLineEdit* barcode_edit = new QLineEdit("123456789");
  form->addRow(QObject::tr("Barcode:"), barcode_edit);

  vbox->addLayout(form);

  // --- preview area ---
  QLabel* preview_label = new QLabel(
      QObject::tr("Press \"Preview\" to render the label."));
  preview_label->setMinimumSize(400, 150);
  preview_label->setFrameShape(QFrame::Box);
  preview_label->setAlignment(Qt::AlignCenter);
  preview_label->setScaledContents(false);
  vbox->addWidget(preview_label, 1);

  // --- status ---
  QLabel* status_label = new QLabel;
  vbox->addWidget(status_label);

  // --- buttons ---
  QHBoxLayout* btn_row = new QHBoxLayout;
  QPushButton* preview_btn = new QPushButton(QObject::tr("Preview"));
  QPushButton* print_btn = new QPushButton(QObject::tr("Print"));
  btn_row->addStretch();
  btn_row->addWidget(preview_btn);
  btn_row->addWidget(print_btn);
  vbox->addLayout(btn_row);

  // --- preview action ---
  QObject::connect(preview_btn, &QPushButton::clicked, [&]() {
    QSizeF const size_mm(width_spin->value(), height_spin->value());
    int const dpi = dpi_spin->value();

    QGraphicsScene* scene = build_label_scene(
        size_mm, text_edit->text(), barcode_edit->text(), bwr_spin->value());

    QImage const img = colman::render_label_scene(scene, size_mm, dpi);
    delete scene;

    if(img.isNull()) {
      status_label->setText(QObject::tr("Rendering failed."));
      return;
    }

    // Scale image to fit the preview label while keeping aspect ratio
    QPixmap const pm = QPixmap::fromImage(img).scaled(
        preview_label->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    preview_label->setPixmap(pm);
    status_label->setText(
        QObject::tr("Rendered: %1 x %2 px at %3 dpi")
            .arg(img.width())
            .arg(img.height())
            .arg(dpi));
  });

  // --- print action ---
  QObject::connect(print_btn, &QPushButton::clicked, [&]() {
    QString const cups_name =
        printer_combo->currentData().toString();
    if(cups_name.isEmpty()) {
      QMessageBox::warning(
          &window,
          QObject::tr("No Printer"),
          QObject::tr("Please select a printer."));
      return;
    }

    QSizeF const size_mm(width_spin->value(), height_spin->value());
    int const dpi = dpi_spin->value();

    QGraphicsScene* scene = build_label_scene(
        size_mm, text_edit->text(), barcode_edit->text(), bwr_spin->value());

    QString error;
    bool const ok = colman::print_label_scene(
        scene, size_mm, cups_name, dpi, error, darkness_spin->value());
    delete scene;

    if(ok) {
      status_label->setText(QObject::tr("Label sent to \"%1\".").arg(cups_name));
    }
    else {
      QMessageBox::critical(
          &window,
          QObject::tr("Print Error"),
          error);
      status_label->setText(QObject::tr("Print failed: %1").arg(error));
    }
  });

  window.resize(640, 540);
  window.show();
  return app.exec();
}

/* EOF */
