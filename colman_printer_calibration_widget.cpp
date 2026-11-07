// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PrinterCalibrationWidget.
/**
   @file
   @author Christoph Pesch

   Scene coordinate system: 1 unit = 0.1 mm (same as colman_item_barcode_page).
   Conversion to printer dots: factor = 0.0039370079 = 1 / (10 * 25.4).

   Three reference crosses at nominal positions (all in mm from paper corner):
   - A at (RefX,       RefY)       = (50, 50)
   - B at (RefX+SpanX, RefY)       = (150, 50)   — 100 mm right of A
   - C at (RefX,       RefY+SpanY) = (50, 150)   — 100 mm below A

   The user measures the actual print positions A(a_x, a_y), B(b_x), C(c_y).
   From these the printer error transform is computed:
   @code
     scale_p_x = (b_x - a_x) / SpanX
     scale_p_y = (c_y - a_y) / SpanY
     trans_p_x = a_x - scale_p_x * RefX
     trans_p_y = a_y - scale_p_y * RefY
   @endcode
   The correction transform (scale_x, offset_x) that compensates the error:
   @code
     scale_x  = 1.0 / scale_p_x
     offset_x = -trans_p_x * scale_x
     scale_y  = 1.0 / scale_p_y
     offset_y = -trans_p_y * scale_y
   @endcode
*/

#include "colman_printer_calibration_widget.hpp"
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
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSTEXTITEM
#  include <QGraphicsTextItem>
#  define SYSTEM_QGRAPHICSTEXTITEM
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
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
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPRINTDIALOG
#  include <QPrintDialog>
#  define SYSTEM_QPRINTDIALOG
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QPRINTERINFO
#  include <QPrinterInfo>
#  define SYSTEM_QPRINTERINFO
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QRESIZEEVENT
#  include <QResizeEvent>
#  define SYSTEM_QRESIZEEVENT
#endif
#ifndef SYSTEM_QSHOWEVENT
#  include <QShowEvent>
#  define SYSTEM_QSHOWEVENT
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QPAGESIZE
#  include <QPageSize>
#  define SYSTEM_QPAGESIZE
#endif

namespace
{

/// Returns QPageSize::Letter for Letter-paper locales, A4 otherwise.
/**
   Letter is used in the US, Canada, and Mexico; all other territories
   default to A4.
*/
  QPageSize locale_page_size()
  {
    switch(QLocale::system().territory()) {
    case QLocale::UnitedStates:
    case QLocale::Canada:
    case QLocale::Mexico:
      return QPageSize(QPageSize::Letter);
    default:
      return QPageSize(QPageSize::A4);
    }
  }

/// Nominal X position of cross A from the paper left edge (mm).
  qreal constexpr RefX = 50.0;
/// Nominal Y position of cross A from the paper top edge (mm).
  qreal constexpr RefY = 50.0;
/// Horizontal distance from cross A to cross B (mm).
  qreal constexpr SpanX = 100.0;
/// Vertical distance from cross A to cross C (mm).
  qreal constexpr SpanY = 100.0;

/// Converts millimetres to scene units (0.1 mm per unit).
  inline qreal mm(qreal x)
  {
    return x * 10.0;
  }

/// Draws a crosshair centred at (@p x, @p y) with arms of length @p arm.
  void draw_crosshair(QGraphicsScene* scene,
                      qreal x, qreal y,
                      qreal arm,
                      QPen const& pen)
  {
    scene->addLine(x - arm, y, x + arm, y, pen);
    scene->addLine(x, y - arm, x, y + arm, pen);
  }

/// Draws a single calibration cross with circle and label.
/**
   @param scene   Target scene.
   @param cx      X centre in scene units.
   @param cy      Y centre in scene units.
   @param label   Letter label shown next to the cross (e.g. "A").
   @param color   Cross colour.
*/
  void draw_cal_cross(QGraphicsScene* scene,
                      qreal cx, qreal cy,
                      QString const& label,
                      QColor const& color)
  {
    QPen const pen(color, 3);
    draw_crosshair(scene, cx, cy, mm(5), pen);
    scene->addEllipse(cx - mm(3), cy - mm(3), mm(6), mm(6),
                      QPen(color, 2), Qt::NoBrush);
    QFont font;
    font.setPixelSize(static_cast<int>(mm(4)));
    font.setBold(true);
    QGraphicsTextItem* t = scene->addText(label, font);
    t->setDefaultTextColor(color);
    t->setPos(cx + mm(6), cy - mm(5));
  }

/// Draws ruler tick marks along the top and left page edges.
/**
   Minor ticks every 5 mm, major ticks with numeric labels every 10 mm.
   @param scene  Target scene.
   @param pw     Page width in scene units.
   @param ph     Page height in scene units.
*/
  void draw_rulers(QGraphicsScene* scene, qreal pw, qreal ph)
  {
    QPen const minor_pen(QColor(160, 160, 160), 1);
    QPen const major_pen(QColor(60, 60, 60), 1);
    QFont label_font;
    label_font.setPixelSize(static_cast<int>(mm(2.5)));

    qreal const minor_tick = mm(2);
    qreal const major_tick = mm(4);

    for(qreal x = 0; x <= pw; x += mm(5)) {
      bool const major = (qRound(x / mm(5)) % 2 == 0);
      qreal const tick = major ? major_tick : minor_tick;
      scene->addLine(x, 0, x, tick, major ? major_pen : minor_pen);
      if(major && x > 0) {
        QGraphicsTextItem* t = scene->addText(
            QString::number(qRound(x / 10)), label_font);
        t->setPos(x + mm(0.5), mm(0.5));
      }
    }
    for(qreal y = 0; y <= ph; y += mm(5)) {
      bool const major = (qRound(y / mm(5)) % 2 == 0);
      qreal const tick = major ? major_tick : minor_tick;
      scene->addLine(0, y, tick, y, major ? major_pen : minor_pen);
      if(major && y > 0) {
        QGraphicsTextItem* t = scene->addText(
            QString::number(qRound(y / 10)), label_font);
        t->setPos(mm(0.5), y + mm(0.5));
      }
    }
  }

/// Draws the label grid in light gray for context.
  void draw_label_grid(QGraphicsScene* scene,
                       colman::PageLayout const& layout)
  {
    qreal const lm = mm(layout.get_printer_left_margin());
    qreal const tm = mm(layout.get_printer_top_margin());
    qreal const lw = mm(layout.get_label_width());
    qreal const lh = mm(layout.get_label_height());
    qreal const step_x = mm(layout.get_label_width() + layout.get_label_distance_x());
    qreal const step_y = mm(layout.get_label_height() + layout.get_label_distance_y());
    QPen const pen(QColor(180, 180, 200), 1, Qt::DashLine);
    for(int row = 0; row < layout.get_rows(); ++row) {
      for(int col = 0; col < layout.get_columns(); ++col) {
        scene->addRect(lm + col * step_x, tm + row * step_y,
                       lw, lh, pen, Qt::NoBrush);
      }
    }
  }

/// Populates @p scene with the calibration preview.
/**
   Draws the full page with:
   - 5-mm grid (light gray)
   - Label grid (dashed, for context)
   - Ruler tick marks along top and left edges
   - Three calibration crosses A (red), B (blue), C (green) at their
     nominal positions

   @param scene   Target scene (will be cleared).
   @param layout  Label sheet geometry (used for the background grid).
*/
  void build_scene(QGraphicsScene* scene,
                   colman::PageLayout const& layout)
  {
    scene->clear();

    qreal const pw = mm(layout.get_page_width());
    qreal const ph = mm(layout.get_page_height());
    scene->setSceneRect(0, 0, pw, ph);

  // background and border
    scene->addRect(0, 0, pw, ph,
                   QPen(Qt::black, 3),
                   QBrush(Qt::white));

  // 5-mm grid
    QPen const grid_pen(QColor(235, 235, 235), 1);
    for(qreal x = 0; x <= pw; x += mm(5)) {
      scene->addLine(x, 0, x, ph, grid_pen);
    }
    for(qreal y = 0; y <= ph; y += mm(5)) {
      scene->addLine(0, y, pw, y, grid_pen);
    }

    draw_label_grid(scene, layout);
    draw_rulers(scene, pw, ph);

  // Three calibration crosses at nominal positions
    draw_cal_cross(scene, mm(RefX), mm(RefY),
                   "A", QColor(Qt::red));
    draw_cal_cross(scene, mm(RefX + SpanX), mm(RefY),
                   "B", QColor(0, 80, 200));
    draw_cal_cross(scene, mm(RefX), mm(RefY + SpanY),
                   "C", QColor(0, 140, 0));
  }

/// Renders the calibration sheet to @p printer.
/**
   Prints on plain paper.  The three reference crosses appear at their
   nominal positions so the user can measure their actual positions with
   a ruler.

   @param printer  Configured, accepted printer.
   @param layout   Label sheet geometry (drawn as gray grid for context).
*/
  void do_print_calibration_sheet(QPrinter* printer,
                                  colman::PageLayout const& layout)
  {
    QGraphicsScene scene;
    build_scene(&scene, layout);

  // instruction text below cross C
    QFont instr_font;
    instr_font.setPixelSize(static_cast<int>(mm(3)));
    qreal const text_y = mm(RefY + SpanY) + mm(14);
    QGraphicsTextItem* instr = scene.addText(
        QApplication::tr(
            "Use a ruler to measure the distances of the cross centres\n"
            "from the paper edge and enter the values in the widget:\n"
            "  A: X and Y distance from edge  "
            "(expected: X=%1 mm, Y=%2 mm)\n"
            "  B: X distance from edge        "
            "(expected: X=%3 mm)\n"
            "  C: Y distance from edge        "
            "(expected: Y=%4 mm)\n"
            "Then click \"Compute offset\".")
            .arg(RefX)
            .arg(RefY)
            .arg(RefX + SpanX)
            .arg(RefY + SpanY),
        instr_font);
    instr->setPos(mm(5), text_y);
    instr->setTextWidth(scene.width() - mm(10));

    qreal const res = printer->resolution();
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);
    qreal const w = scene.width();
    qreal const h = scene.height();
  // 0.0039370079 = 1 / (10 * 25.4): converts 0.1-mm scene units to inches
    scene.render(&painter,
                 QRectF(0, 0,
                        0.0039370079 * w * res,
                        0.0039370079 * h * res));
  }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

colman::PrinterCalibrationWidget::PrinterCalibrationWidget(
    colman::PageLayout const& layout,
    QWidget* parent)
    : QWidget(parent), layout_(layout)
{
  // --- printer combo -------------------------------------------------------
  printer_combo_ = new QComboBox(this);
  for(QPrinterInfo const& info : QPrinterInfo::availablePrinters()) {
    QString const display = info.description().isEmpty()
                                ? info.printerName()
                                : info.description();
    printer_combo_->addItem(display, info.printerName());
  }
  if(printer_combo_->count() == 0) {
    printer_combo_->addItem(
        QApplication::tr("(no printer available)"), QString());
  }

  // --- step 1: print -------------------------------------------------------
  auto* print_btn = new QPushButton(
      QApplication::tr("Print calibration sheet"), this);
  print_btn->setToolTip(QApplication::tr(
                            "Prints a sheet on plain paper with three crosses\n"
                            "A (%1|%2) mm, B (%3|%4) mm, C (%5|%6) mm.")
                            .arg(RefX)
                            .arg(RefY)
                            .arg(RefX + SpanX)
                            .arg(RefY)
                            .arg(RefX)
                            .arg(RefY + SpanY));

  // --- step 2: measure and compute -----------------------------------------
  auto* ref_label = new QLabel(
      QApplication::tr(
          "Measure the distances of the cross centres from the paper edge:"),
      this);

  meas_a_x_spin_ = new QDoubleSpinBox(this);
  meas_a_x_spin_->setRange(0.0, 250.0);
  meas_a_x_spin_->setDecimals(1);
  meas_a_x_spin_->setSingleStep(0.5);
  meas_a_x_spin_->setSuffix(" mm");
  meas_a_x_spin_->setValue(RefX);
  meas_a_x_spin_->setToolTip(
      QApplication::tr("Expected: %1 mm").arg(RefX));

  meas_a_y_spin_ = new QDoubleSpinBox(this);
  meas_a_y_spin_->setRange(0.0, 350.0);
  meas_a_y_spin_->setDecimals(1);
  meas_a_y_spin_->setSingleStep(0.5);
  meas_a_y_spin_->setSuffix(" mm");
  meas_a_y_spin_->setValue(RefY);
  meas_a_y_spin_->setToolTip(
      QApplication::tr("Expected: %1 mm").arg(RefY));

  meas_b_x_spin_ = new QDoubleSpinBox(this);
  meas_b_x_spin_->setRange(0.0, 250.0);
  meas_b_x_spin_->setDecimals(1);
  meas_b_x_spin_->setSingleStep(0.5);
  meas_b_x_spin_->setSuffix(" mm");
  meas_b_x_spin_->setValue(RefX + SpanX);
  meas_b_x_spin_->setToolTip(
      QApplication::tr("Expected: %1 mm").arg(RefX + SpanX));

  meas_c_y_spin_ = new QDoubleSpinBox(this);
  meas_c_y_spin_->setRange(0.0, 350.0);
  meas_c_y_spin_->setDecimals(1);
  meas_c_y_spin_->setSingleStep(0.5);
  meas_c_y_spin_->setSuffix(" mm");
  meas_c_y_spin_->setValue(RefY + SpanY);
  meas_c_y_spin_->setToolTip(
      QApplication::tr("Expected: %1 mm").arg(RefY + SpanY));

  auto* compute_btn = new QPushButton(
      QApplication::tr("Compute offset"), this);
  compute_btn->setToolTip(QApplication::tr(
      "Computes offset and scale correction from the three measured "
      "positions."));

  // --- step 3: review and save ---------------------------------------------
  offset_x_spin_ = new QDoubleSpinBox(this);
  offset_x_spin_->setRange(-30.0, 30.0);
  offset_x_spin_->setDecimals(2);
  offset_x_spin_->setSingleStep(0.1);
  offset_x_spin_->setSuffix(" mm");
  offset_x_spin_->setToolTip(QApplication::tr(
      "Positive value shifts the print to the right."));

  offset_y_spin_ = new QDoubleSpinBox(this);
  offset_y_spin_->setRange(-30.0, 30.0);
  offset_y_spin_->setDecimals(2);
  offset_y_spin_->setSingleStep(0.1);
  offset_y_spin_->setSuffix(" mm");
  offset_y_spin_->setToolTip(QApplication::tr(
      "Positive value shifts the print downward."));

  scale_x_spin_ = new QDoubleSpinBox(this);
  scale_x_spin_->setRange(0.8, 1.2);
  scale_x_spin_->setDecimals(4);
  scale_x_spin_->setSingleStep(0.001);
  scale_x_spin_->setValue(1.0);
  scale_x_spin_->setToolTip(QApplication::tr(
      "1.0 = no scaling. Values > 1 stretch, < 1 compress."));

  scale_y_spin_ = new QDoubleSpinBox(this);
  scale_y_spin_->setRange(0.8, 1.2);
  scale_y_spin_->setDecimals(4);
  scale_y_spin_->setSingleStep(0.001);
  scale_y_spin_->setValue(1.0);
  scale_y_spin_->setToolTip(QApplication::tr(
      "1.0 = no scaling. Values > 1 stretch, < 1 compress."));

  auto* save_btn = new QPushButton(
      QApplication::tr("Save"), this);
  auto* reset_btn = new QPushButton(
      QApplication::tr("Reset"), this);

  // --- preview scene + view ------------------------------------------------
  scene_ = new QGraphicsScene(this);
  preview_view_ = new QGraphicsView(scene_, this);
  preview_view_->setRenderHint(QPainter::Antialiasing);
  preview_view_->setBackgroundBrush(QBrush(QColor(160, 160, 160)));
  preview_view_->setMinimumHeight(200);
  preview_view_->setDragMode(QGraphicsView::NoDrag);

  // --- repeatability note --------------------------------------------------
  auto* repeat_hint = new QLabel(
      QApplication::tr(
          "Laser printers hold calibration well across sheets. "
          "Inkjet printers vary by up to 1&nbsp;mm per sheet."),
      this);
  repeat_hint->setWordWrap(true);
  repeat_hint->setTextFormat(Qt::RichText);

  auto* repeat_info_btn = new QPushButton(this);
  repeat_info_btn->setIcon(
      QPixmap(":/icons/owls/hint.jpeg").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  repeat_info_btn->setIconSize(QSize(32, 32));
  repeat_info_btn->setFlat(true);
  repeat_info_btn->setToolTip(
      QApplication::tr("More information about printer accuracy"));

  // --- layout --------------------------------------------------------------
  auto* form = new QFormLayout;
  form->addRow(QApplication::tr("Printer:"), printer_combo_);

  // Step 1
  auto* step1_box = new QGroupBox(
      QApplication::tr("Step 1: Print calibration sheet"), this);
  auto* margin_hint = new QLabel(
      QApplication::tr(
          "Load plain paper and confirm the print dialog with the "
          "<b>default settings</b> — page margins do not need to be "
          "changed."),
      step1_box);
  margin_hint->setWordWrap(true);
  auto* s1 = new QVBoxLayout(step1_box);
  s1->addWidget(margin_hint);
  s1->addWidget(print_btn);

  // Step 2
  auto* step2_box = new QGroupBox(
      QApplication::tr("Step 2: Measure and compute offset"), this);
  auto* meas_grid = new QGridLayout;
  meas_grid->addWidget(
      new QLabel(QApplication::tr("Cross A \xe2\x80\x94 X distance:"),
                 step2_box),
      0, 0);
  meas_grid->addWidget(meas_a_x_spin_, 0, 1);
  meas_grid->addWidget(
      new QLabel(QApplication::tr("Cross A \xe2\x80\x94 Y distance:"),
                 step2_box),
      0, 2);
  meas_grid->addWidget(meas_a_y_spin_, 0, 3);
  meas_grid->addWidget(
      new QLabel(QApplication::tr("Cross B \xe2\x80\x94 X distance:"),
                 step2_box),
      1, 0);
  meas_grid->addWidget(meas_b_x_spin_, 1, 1);
  meas_grid->addWidget(
      new QLabel(QApplication::tr("Cross C \xe2\x80\x94 Y distance:"),
                 step2_box),
      1, 2);
  meas_grid->addWidget(meas_c_y_spin_, 1, 3);
  meas_grid->setColumnStretch(1, 1);
  meas_grid->setColumnStretch(3, 1);
  auto* s2 = new QVBoxLayout(step2_box);
  s2->addWidget(ref_label);
  s2->addLayout(meas_grid);
  s2->addWidget(compute_btn);

  // Step 3
  auto* step3_box = new QGroupBox(
      QApplication::tr("Step 3: Review and save calibration"), this);
  auto* accuracy_hint = new QLabel(
      QApplication::tr(
          "<b>Typical ranges:</b> "
          "Laser: offset &lt; 2&nbsp;mm, scale 0.998&ndash;1.002. "
          "Inkjet: offset &lt; 3&nbsp;mm, scale 0.995&ndash;1.005. "
          "Ruler reading uncertainty: &plusmn;0.5&nbsp;mm (often dominates)."),
      step3_box);
  accuracy_hint->setWordWrap(true);

  auto* spin_grid = new QGridLayout;
  spin_grid->addWidget(
      new QLabel(QApplication::tr("X offset:"), step3_box), 0, 0);
  spin_grid->addWidget(offset_x_spin_, 0, 1);
  spin_grid->addWidget(
      new QLabel(QApplication::tr("Y offset:"), step3_box), 0, 2);
  spin_grid->addWidget(offset_y_spin_, 0, 3);
  spin_grid->addWidget(
      new QLabel(QApplication::tr("X scale:"), step3_box), 1, 0);
  spin_grid->addWidget(scale_x_spin_, 1, 1);
  spin_grid->addWidget(
      new QLabel(QApplication::tr("Y scale:"), step3_box), 1, 2);
  spin_grid->addWidget(scale_y_spin_, 1, 3);
  spin_grid->setColumnStretch(1, 1);
  spin_grid->setColumnStretch(3, 1);

  auto* btn_row = new QHBoxLayout;
  btn_row->addWidget(save_btn);
  btn_row->addWidget(reset_btn);

  auto* s3 = new QVBoxLayout(step3_box);
  s3->addWidget(accuracy_hint);
  s3->addLayout(spin_grid);
  s3->addLayout(btn_row);

  auto* repeat_row = new QHBoxLayout;
  repeat_row->addWidget(repeat_hint, 1);
  repeat_row->addWidget(repeat_info_btn);

  // Left column: controls
  auto* controls = new QVBoxLayout;
  controls->addLayout(form);
  controls->addLayout(repeat_row);
  controls->addWidget(step1_box);
  controls->addWidget(step2_box);
  controls->addWidget(step3_box);
  controls->addStretch(1);

  // Right column: preview
  auto* preview_col = new QVBoxLayout;
  preview_col->addWidget(
      new QLabel(QApplication::tr("Preview:"), this));
  preview_col->addWidget(preview_view_, 1);

  auto* hbox = new QHBoxLayout(this);
  setMinimumWidth(800);
  hbox->addLayout(controls);
  hbox->addLayout(preview_col, 1);

  // --- connections ---------------------------------------------------------
  connect(repeat_info_btn, &QPushButton::clicked,
          this, [this]() {
            QMessageBox box(this);
            box.setWindowTitle(QApplication::tr("Printer accuracy"));
            box.setIconPixmap(
                QPixmap(":/icons/owls/hint.jpeg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            box.setText(QApplication::tr(
                "<b>Laser printers</b> are highly consistent: sheet-to-sheet "
                "variation is typically &lt;&nbsp;0.2&nbsp;mm. One calibration "
                "is sufficient for a long time."));
            box.setInformativeText(QApplication::tr(
                "<b>Inkjet printers</b> vary by 0.3&ndash;1&nbsp;mm per sheet "
                "due to roller slip, depending on paper type and weight. "
                "The first sheet after a long pause may be further off. "
                "The horizontal axis (X) is usually more stable than the "
                "vertical (Y, paper feed direction).<br><br>"
                "Calibration removes the <i>systematic</i> offset. "
                "The remaining <i>random</i> variation cannot be corrected. "
                "Labels printed on an inkjet should therefore have at least "
                "1&ndash;2&nbsp;mm clearance inside the label edge."));
            box.exec();
          });

  connect(printer_combo_,
          &QComboBox::currentIndexChanged,
          this,
          &PrinterCalibrationWidget::on_printer_changed);
  connect(print_btn, &QPushButton::clicked,
          this, &PrinterCalibrationWidget::on_print_calibration_sheet);
  connect(compute_btn, &QPushButton::clicked,
          this, &PrinterCalibrationWidget::on_compute_offset);
  connect(save_btn, &QPushButton::clicked,
          this, &PrinterCalibrationWidget::on_save);
  connect(reset_btn, &QPushButton::clicked,
          this, &PrinterCalibrationWidget::on_reset);

  load_from_settings();
  update_preview();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/// Blocks signals on @p spin, sets @p value, unblocks.
static void set_spin_silent(QDoubleSpinBox* spin, qreal value)
{
  spin->blockSignals(true);
  spin->setValue(value);
  spin->blockSignals(false);
}

void colman::PrinterCalibrationWidget::load_from_settings()
{
  QString const name = printer_combo_->currentData().toString();
  if(name.isEmpty()) {
    return;
  }
  PrinterCalibration const cal = colman::load_printer_calibration(name);
  set_spin_silent(offset_x_spin_, cal.offset_x);
  set_spin_silent(offset_y_spin_, cal.offset_y);
  set_spin_silent(scale_x_spin_, cal.scale_x);
  set_spin_silent(scale_y_spin_, cal.scale_y);

  // Pre-fill measurement spinboxes with the values that would reproduce
  // the current calibration (inverse of the correction transform):
  //   scale_p = 1/scale_c
  //   trans_p = -offset_c * scale_p
  //   a_x = scale_p * RefX + trans_p
  qreal const sp_x = (cal.scale_x > 0.0) ? 1.0 / cal.scale_x : 1.0;
  qreal const sp_y = (cal.scale_y > 0.0) ? 1.0 / cal.scale_y : 1.0;
  qreal const tp_x = -cal.offset_x * sp_x;
  qreal const tp_y = -cal.offset_y * sp_y;
  meas_a_x_spin_->setValue(sp_x * RefX + tp_x);
  meas_a_y_spin_->setValue(sp_y * RefY + tp_y);
  meas_b_x_spin_->setValue(sp_x * (RefX + SpanX) + tp_x);
  meas_c_y_spin_->setValue(sp_y * (RefY + SpanY) + tp_y);
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void colman::PrinterCalibrationWidget::on_printer_changed(int /*index*/)
{
  load_from_settings();
  update_preview();
}

void colman::PrinterCalibrationWidget::update_preview()
{
  build_scene(scene_, layout_);
  preview_view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void colman::PrinterCalibrationWidget::on_compute_offset()
{
  qreal const a_x = meas_a_x_spin_->value();
  qreal const a_y = meas_a_y_spin_->value();
  qreal const b_x = meas_b_x_spin_->value();
  qreal const c_y = meas_c_y_spin_->value();

  // Printer error transform derived from the three measured cross positions.
  qreal const scale_p_x = (b_x - a_x) / SpanX;
  qreal const scale_p_y = (c_y - a_y) / SpanY;

  // Guard against degenerate input (measured span <= 0).
  if(scale_p_x <= 0.0 || scale_p_y <= 0.0) {
    return;
  }

  qreal const trans_p_x = a_x - scale_p_x * RefX;
  qreal const trans_p_y = a_y - scale_p_y * RefY;

  // Correction transform: printed(corrected(x)) == x
  qreal const scale_c_x = 1.0 / scale_p_x;
  qreal const scale_c_y = 1.0 / scale_p_y;
  qreal const offset_c_x = -trans_p_x * scale_c_x;
  qreal const offset_c_y = -trans_p_y * scale_c_y;

  set_spin_silent(offset_x_spin_, offset_c_x);
  set_spin_silent(offset_y_spin_, offset_c_y);
  set_spin_silent(scale_x_spin_, scale_c_x);
  set_spin_silent(scale_y_spin_, scale_c_y);
}

void colman::PrinterCalibrationWidget::on_print_calibration_sheet()
{
  QPrinter printer(QPrinter::HighResolution);
  printer.setFullPage(true);
  printer.setPageSize(locale_page_size());
  QString const name = printer_combo_->currentData().toString();
  if(!name.isEmpty()) {
    printer.setPrinterName(name);
  }
  QPrintDialog dialog(&printer, this);
  if(dialog.exec() == QDialog::Accepted) {
    do_print_calibration_sheet(&printer, layout_);
  }
}

void colman::PrinterCalibrationWidget::on_save()
{
  QString const name = printer_combo_->currentData().toString();
  if(name.isEmpty()) {
    return;
  }
  PrinterCalibration cal;
  cal.offset_x = offset_x_spin_->value();
  cal.offset_y = offset_y_spin_->value();
  cal.scale_x = scale_x_spin_->value();
  cal.scale_y = scale_y_spin_->value();
  colman::save_printer_calibration(name, cal);
  Q_EMIT calibration_saved();
}

void colman::PrinterCalibrationWidget::on_reset()
{
  meas_a_x_spin_->setValue(RefX);
  meas_a_y_spin_->setValue(RefY);
  meas_b_x_spin_->setValue(RefX + SpanX);
  meas_c_y_spin_->setValue(RefY + SpanY);
  set_spin_silent(offset_x_spin_, 0.0);
  set_spin_silent(offset_y_spin_, 0.0);
  set_spin_silent(scale_x_spin_, 1.0);
  set_spin_silent(scale_y_spin_, 1.0);
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

void colman::PrinterCalibrationWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  if(!scene_->sceneRect().isEmpty()) {
    preview_view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  }
}

void colman::PrinterCalibrationWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  if(!scene_->sceneRect().isEmpty()) {
    preview_view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  }
}

/* EOF */
