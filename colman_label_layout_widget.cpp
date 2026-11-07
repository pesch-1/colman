// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of LabelLayoutWidget.
/**
   @file
   @author Christoph Pesch

   Scene coordinate system: 1 unit = 0.1 mm.

   Built-in preset dimensions are taken from the gLabels open-source label
   editor database (https://github.com/jimevins/glabels-qt), which has been
   cross-checked against manufacturer specifications.  Values marked
   "(approx)" are estimates derived from nominal label dimensions and have
   not been independently measured.
*/

#include "colman_label_layout_widget.hpp"
#ifndef COLMAN_LABEL_PRINTER_HPP
#  include "colman_label_printer.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
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
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
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
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QRESIZEEVENT
#  include <QResizeEvent>
#  define SYSTEM_QRESIZEEVENT
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QSHOWEVENT
#  include <QShowEvent>
#  define SYSTEM_QSHOWEVENT
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif

namespace
{

/// Converts millimetres to scene units (0.1 mm per unit).
  inline qreal mm(qreal x)
  {
    return x * 10.0;
  }

/// Draws the preview for a sheet-label or label-printer preset into @p scene.
  void build_preview(QGraphicsScene* scene,
                     colman::LabelLayoutPreset const& p)
  {
    scene->clear();

    QPen const label_pen(QColor(60, 100, 200), 2);
    QBrush const label_brush(QColor(220, 230, 255));
    qreal const lw = mm(p.label_width);
    qreal const lh = mm(p.label_height);

    if(p.is_label_printer) {
    // Show only a single label (no page background).
      scene->setSceneRect(0, 0, lw, lh);
      scene->addRect(0, 0, lw, lh, label_pen, label_brush);
      return;
    }

    qreal const pw = mm(210.0);
    qreal const ph = mm(297.0);
    scene->setSceneRect(0, 0, pw, ph);

  // page background + border
    scene->addRect(0, 0, pw, ph,
                   QPen(Qt::black, 3),
                   QBrush(Qt::white));

  // 10-mm grid (light)
    QPen const grid_pen(QColor(235, 235, 235), 1);
    for(qreal x = 0; x <= pw; x += mm(10)) {
      scene->addLine(x, 0, x, ph, grid_pen);
    }
    for(qreal y = 0; y <= ph; y += mm(10)) {
      scene->addLine(0, y, pw, y, grid_pen);
    }

  // label grid
    qreal const lm = mm(p.left_margin);
    qreal const tm = mm(p.top_margin);
    qreal const dx = mm(p.label_width + p.h_gap);
    qreal const dy = mm(p.label_height + p.v_gap);
    for(int row = 0; row < p.rows; ++row) {
      for(int col = 0; col < p.columns; ++col) {
        scene->addRect(lm + col * dx,
                       tm + row * dy,
                       lw, lh,
                       label_pen, label_brush);
      }
    }
  }

/// Returns a configured QDoubleSpinBox for millimetre values.
  QDoubleSpinBox* make_mm_spin(qreal min, qreal max, qreal step,
                               QWidget* parent)
  {
    auto* s = new QDoubleSpinBox(parent);
    s->setRange(min, max);
    s->setDecimals(2);
    s->setSingleStep(step);
    s->setSuffix(" mm");
    return s;
  }

/// Writes the element arrangement of @p p into the current QSettings group.
/**
   Only the arrangement matching @p p.purpose is written (card zones for
   IdCard/ItemLabel, the text zone for Signature).  Keys use an @c arr_
   prefix so they never collide with the geometry keys.
*/
  void write_arrangement(QSettings& s, colman::LabelLayoutPreset const& p)
  {
    auto write_elem = [&](QString const& pre, colman::IdCardElement const& e) {
      s.setValue("arr_" + pre + "_type", static_cast<int>(e.type_));
      s.setValue("arr_" + pre + "_orient", e.orientation_);
      s.setValue("arr_" + pre + "_x", e.x_mm_);
      s.setValue("arr_" + pre + "_y", e.y_mm_);
      s.setValue("arr_" + pre + "_w", e.width_mm_);
      s.setValue("arr_" + pre + "_h", e.height_mm_);
    };
    if(p.purpose == colman::LabelLayoutPurpose::Signature) {
      colman::SignatureLabelLayout const& a = p.sig_arrangement;
      s.setValue("arr_sig_w", a.card_width_mm_);
      s.setValue("arr_sig_h", a.card_height_mm_);
      s.setValue("arr_sig_margin", a.margin_mm_);
      s.setValue("arr_sig_tx", a.text_x_mm_);
      s.setValue("arr_sig_ty", a.text_y_mm_);
      s.setValue("arr_sig_tw", a.text_width_mm_);
      s.setValue("arr_sig_th", a.text_height_mm_);
      s.setValue("arr_sig_orient", a.text_orientation_);
    }
    else {
      colman::IdCardLayout const& a = p.card_arrangement;
      s.setValue("arr_card_w", a.card_width_mm_);
      s.setValue("arr_card_h", a.card_height_mm_);
      s.setValue("arr_has_logo", a.has_logo_);
      write_elem("bc", a.barcode_);
      write_elem("tx", a.text_);
      write_elem("lg", a.logo_);
    }
  }

/// Reads the element arrangement for @p p (its @c purpose must be set).
/**
   Absent keys default to zero, leaving the arrangement's card size at 0,
   which the editors and the render path treat as "not configured".
*/
  void read_arrangement(QSettings& s, colman::LabelLayoutPreset& p)
  {
    if(p.purpose == colman::LabelLayoutPurpose::Signature) {
      colman::SignatureLabelLayout a;
      a.card_width_mm_ = s.value("arr_sig_w", 0.0).toDouble();
      a.card_height_mm_ = s.value("arr_sig_h", 0.0).toDouble();
      a.margin_mm_ = s.value("arr_sig_margin", 0.0).toDouble();
      a.text_x_mm_ = s.value("arr_sig_tx", 0.0).toDouble();
      a.text_y_mm_ = s.value("arr_sig_ty", 0.0).toDouble();
      a.text_width_mm_ = s.value("arr_sig_tw", 0.0).toDouble();
      a.text_height_mm_ = s.value("arr_sig_th", 0.0).toDouble();
      a.text_orientation_ = s.value("arr_sig_orient", 0).toInt();
      a.label_layout_name_ = p.name;
      p.sig_arrangement = a;
    }
    else {
      auto read_elem = [&](QString const& pre,
                           colman::IdCardElementType type) {
        colman::IdCardElement e;
        e.type_ = type;
        e.orientation_ = s.value("arr_" + pre + "_orient", 0).toInt();
        e.x_mm_ = s.value("arr_" + pre + "_x", 0.0).toDouble();
        e.y_mm_ = s.value("arr_" + pre + "_y", 0.0).toDouble();
        e.width_mm_ = s.value("arr_" + pre + "_w", 0.0).toDouble();
        e.height_mm_ = s.value("arr_" + pre + "_h", 0.0).toDouble();
        return e;
      };
      colman::IdCardLayout a;
      a.card_width_mm_ = s.value("arr_card_w", 0.0).toDouble();
      a.card_height_mm_ = s.value("arr_card_h", 0.0).toDouble();
      a.has_logo_ = s.value("arr_has_logo", false).toBool();
      a.barcode_ = read_elem("bc", colman::IdCardElementType::Barcode);
      a.text_ = read_elem("tx", colman::IdCardElementType::Text);
      a.logo_ = read_elem("lg", colman::IdCardElementType::Logo);
      p.card_arrangement = a;
    }
  }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

QString colman::LabelLayoutWidget::settings_group(QString const& name)
{
  return "label_layouts/" + name;
}

QString colman::to_settings_key(colman::LabelLayoutPurpose purpose)
{
  switch(purpose) {
  case colman::LabelLayoutPurpose::IdCard:
    return "id_card";
  case colman::LabelLayoutPurpose::ItemLabel:
    return "item_label";
  case colman::LabelLayoutPurpose::Signature:
    return "signature";
  case colman::LabelLayoutPurpose::Any:
    break;
  }
  return QString();
}

colman::LabelLayoutPurpose
colman::purpose_from_settings_key(QString const& key)
{
  if(key == "id_card") {
    return colman::LabelLayoutPurpose::IdCard;
  }
  if(key == "item_label") {
    return colman::LabelLayoutPurpose::ItemLabel;
  }
  if(key == "signature") {
    return colman::LabelLayoutPurpose::Signature;
  }
  return colman::LabelLayoutPurpose::Any;
}

void colman::LabelLayoutWidget::set_purpose(colman::LabelLayoutPurpose purpose)
{
  purpose_ = purpose;
  populate_combo();
}

QString colman::LabelLayoutWidget::last_selected_layout_name()
{
  QSettings settings;
  return settings.value("label_layouts/last_selected").toString();
}

void colman::LabelLayoutWidget::set_last_selected_layout_name(
    QString const& name)
{
  QSettings settings;
  settings.setValue("label_layouts/last_selected", name);
}

// ---------------------------------------------------------------------------
// Factory presets
// ---------------------------------------------------------------------------

QList<colman::LabelLayoutPreset>
colman::LabelLayoutWidget::builtin_layouts()
{
  // Sources: gLabels label database (verified) and manufacturer data sheets.
  // Values marked (approx) are derived from nominal dimensions.
  QList<LabelLayoutPreset> list;

  auto add = [&](QString const& name,
                 int cols, int rows,
                 double lw, double lh,
                 double left, double top,
                 double hg, double vg) {
    LabelLayoutPreset p;
    p.name = name;
    p.columns = cols;
    p.rows = rows;
    p.label_width = lw;
    p.label_height = lh;
    p.left_margin = left;
    p.top_margin = top;
    p.h_gap = hg;
    p.v_gap = vg;
    p.builtin = true;
    list.append(p);
  };

  // Sources: doc/label_layouts.csv (measured/manufacturer data)
  // Avery Zweckform
  add("Avery 3424 (105×48 mm, 12/A4)", 2, 6, 105.0, 48.0, 0.0, 4.5, 0.0, 0.0);
  add("Avery 3474 (70×37 mm, 24/A4)", 3, 8, 70.0, 37.0, 0.0, 0.0, 0.0, 0.0);
  add("Avery 3475 (70×36 mm, 24/A4)", 3, 8, 70.0, 36.0, 0.0, 4.5, 0.0, 0.0);
  add("Avery 3478 (210×297 mm, 1/A4)", 1, 1, 210.0, 297.0, 0.0, 0.0, 0.0, 0.0);
  add("Avery 3483 (105×148 mm, 4/A4)", 2, 2, 105.0, 148.0, 0.0, 0.5, 0.0, 0.0);
  add("Avery 3651 (52.5×29.7 mm, 40/A4)", 4, 10, 52.5, 29.7, 0.0, 0.0, 0.0, 0.0);
  add("Avery 3657 (48.5×25.4 mm, 40/A4)", 4, 10, 48.5, 25.4, 7.75, 21.5, 0.0, 0.0);
  add("Avery 3666 (38×21.2 mm, 65/A4)", 5, 13, 38.0, 21.2, 10.0, 10.7, 0.0, 0.0);
  add("Avery L7160 (63.5×38.1 mm, 21/A4)", 3, 7, 63.5, 38.1, 7.2, 15.1, 2.5, 0.0);
  add("Avery L7161 (63.5×46.6 mm, 18/A4)", 3, 6, 63.5, 46.6, 7.2, 8.5, 2.5, 0.0);
  add("Avery L7162 (99.1×34 mm, 16/A4)", 2, 8, 99.1, 34.0, 4.9, 12.5, 2.8, 0.0);
  add("Avery L7163 (99.1×38.1 mm, 14/A4)", 2, 7, 99.1, 38.1, 4.9, 15.1, 2.8, 0.0);
  add("Avery L7651 (38.1×21.2 mm, 65/A4)", 5, 13, 38.1, 21.2, 9.9, 11.0, 0.0, 0.0);
  add("Avery L7654 (38.1×21.2 mm, 65/A4)", 5, 13, 38.1, 21.2, 10.0, 10.7, 0.0, 0.0);
  add("Avery L7658 (25.4×10 mm, 189/A4)", 7, 27, 25.4, 10.0, 14.2, 13.5, 2.5, 0.0);
  // Herma
  add("Herma 4250 (99.1×139 mm, 4/A4)", 2, 2, 99.1, 139.0, 4.9, 9.5, 2.8, 0.0);
  add("Herma 4280 (97×67.7 mm, 8/A4)", 2, 4, 97.0, 67.7, 8.0, 13.1, 0.0, 0.0);
  add("Herma 4453 (70×36 mm, 24/A4)", 3, 8, 70.0, 36.0, 0.0, 4.5, 0.0, 0.0);
  add("Herma 4615 (70×37 mm, 24/A4)", 3, 8, 70.0, 37.0, 0.0, 0.0, 0.0, 0.0);
  // Sigel
  add("Sigel LA111 (70×36 mm, 24/A4)", 3, 8, 70.0, 36.0, 0.0, 4.5, 0.0, 0.0);
  add("Sigel LA130 (97×42.3 mm, 14/A4)", 2, 7, 97.0, 42.3, 8.0, 0.4, 0.0, 0.0);
  add("Sigel LA151 (35.6×16.9 mm, 80/A4)", 5, 16, 35.6, 16.9, 16.0, 13.3, 0.0, 0.0);

  return list;
}

// ---------------------------------------------------------------------------
// User preset persistence
// ---------------------------------------------------------------------------

QList<colman::LabelLayoutPreset>
colman::LabelLayoutWidget::load_user_layouts()
{
  QSettings settings;
  QList<LabelLayoutPreset> list;
  settings.beginGroup("label_layouts");
  for(QString const& name : settings.childGroups()) {
    settings.beginGroup(name);
    LabelLayoutPreset p;
    p.name = name;
    p.columns = settings.value("columns", p.columns).toInt();
    p.rows = settings.value("rows", p.rows).toInt();
    p.label_width = settings.value("label_width", p.label_width).toDouble();
    p.label_height = settings.value("label_height", p.label_height).toDouble();
    p.left_margin = settings.value("left_margin", p.left_margin).toDouble();
    p.top_margin = settings.value("top_margin", p.top_margin).toDouble();
    p.h_gap = settings.value("h_gap", p.h_gap).toDouble();
    p.v_gap = settings.value("v_gap", p.v_gap).toDouble();
    p.page_width_mm = settings.value("page_width_mm", p.page_width_mm).toDouble();
    p.page_height_mm = settings.value("page_height_mm", p.page_height_mm).toDouble();
    p.is_label_printer = settings.value("is_label_printer", false).toBool();
    p.printer_cups_name = settings.value("printer_cups_name", QString()).toString();
    p.printer_dpi = settings.value("printer_dpi", 203).toInt();
    p.bar_width_reduction_mm =
        settings.value("bar_width_reduction_mm", 0.0).toDouble();
    p.darkness = settings.value("darkness", -1).toInt();
    p.rotate_180 = settings.value("rotate_180", false).toBool();
    p.purpose = purpose_from_settings_key(
        settings.value("purpose", QString()).toString());
    read_arrangement(settings, p);
    p.builtin = false;
    list.append(p);
    settings.endGroup();
  }
  settings.endGroup();
  return list;
}

void colman::LabelLayoutWidget::save_user_layout(
    LabelLayoutPreset const& p)
{
  QSettings settings;
  settings.beginGroup(settings_group(p.name));
  settings.setValue("columns", p.columns);
  settings.setValue("rows", p.rows);
  settings.setValue("label_width", p.label_width);
  settings.setValue("label_height", p.label_height);
  settings.setValue("left_margin", p.left_margin);
  settings.setValue("top_margin", p.top_margin);
  settings.setValue("h_gap", p.h_gap);
  settings.setValue("v_gap", p.v_gap);
  settings.setValue("page_width_mm", p.page_width_mm);
  settings.setValue("page_height_mm", p.page_height_mm);
  settings.setValue("is_label_printer", p.is_label_printer);
  settings.setValue("printer_cups_name", p.printer_cups_name);
  settings.setValue("printer_dpi", p.printer_dpi);
  settings.setValue("bar_width_reduction_mm", p.bar_width_reduction_mm);
  settings.setValue("darkness", p.darkness);
  settings.setValue("rotate_180", p.rotate_180);
  settings.setValue("purpose", to_settings_key(p.purpose));
  write_arrangement(settings, p);
  settings.endGroup();
}

void colman::LabelLayoutWidget::delete_user_layout(
    QString const& name)
{
  QSettings settings;
  settings.beginGroup("label_layouts");
  settings.remove(name);
  settings.endGroup();
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

colman::LabelLayoutWidget::LabelLayoutWidget(QWidget* parent)
    : QWidget(parent)
{
  // --- combo ---------------------------------------------------------------
  layout_combo_ = new QComboBox(this);
  layout_combo_->setObjectName("layout_combo_");

  // --- name ----------------------------------------------------------------
  name_edit_ = new QLineEdit(this);
  name_edit_->setObjectName("layout_name_");
  name_edit_->setPlaceholderText(
      QApplication::tr("Name for new layout"));

  // --- geometry spinboxes --------------------------------------------------
  columns_spin_ = new QSpinBox(this);
  columns_spin_->setRange(1, 20);

  rows_spin_ = new QSpinBox(this);
  rows_spin_->setRange(1, 40);

  label_width_spin_ = make_mm_spin(1.0, 300.0, 0.5, this);
  label_height_spin_ = make_mm_spin(1.0, 300.0, 0.5, this);
  left_margin_spin_ = make_mm_spin(0.0, 100.0, 0.1, this);
  top_margin_spin_ = make_mm_spin(0.0, 100.0, 0.1, this);
  h_gap_spin_ = make_mm_spin(0.0, 30.0, 0.1, this);
  v_gap_spin_ = make_mm_spin(0.0, 30.0, 0.1, this);
  bar_width_reduction_spin_ = make_mm_spin(0.0, 0.5, 0.05, this);
  bar_width_reduction_spin_->setObjectName("layout_bar_width_reduction_");
  bar_width_reduction_spin_->setToolTip(QApplication::tr(
      "Shrinks each black barcode bar by this amount to compensate for ink or "
      "heat spread. Use a small value (e.g. 0.1–0.2 mm) for thermal "
      "printers; leave at 0 for laser or inkjet printers."));

  darkness_spin_ = new QSpinBox(this);
  darkness_spin_->setRange(-1, colman::MaxThermalDarkness);
  darkness_spin_->setSpecialValueText(QApplication::tr("Printer default"));
  darkness_spin_->setToolTip(QApplication::tr(
      "Thermal print darkness (0-15) sent to the printer. Higher values print "
      "darker; use a lower value for coloured thermal paper and a higher one "
      "for white paper. \"Printer default\" leaves the printer's own setting "
      "unchanged. Only used for thermal label printers."));

  rotate_check_ = new QCheckBox(
      QApplication::tr("Rotate output by 180°"), this);
  rotate_check_->setToolTip(QApplication::tr(
      "Prints the label content rotated by half a turn. Use this when the "
      "label printer feeds the labels so that the print comes out upside "
      "down. Only used for thermal label printers."));

  count_label_ = new QLabel(this);

  // --- buttons -------------------------------------------------------------
  // The layout is saved together with its element arrangement by the
  // surrounding editor's single Save button, so this widget only manages
  // the layout lifecycle (start a new one / delete one).
  new_btn_ = new QPushButton(QApplication::tr("New"), this);
  delete_btn_ = new QPushButton(QApplication::tr("Delete"), this);

  new_btn_->setToolTip(QApplication::tr(
      "Start a new user-defined layout."));
  delete_btn_->setToolTip(QApplication::tr(
      "Delete the selected user layout (built-in layouts cannot be deleted)."));

  // --- preview -------------------------------------------------------------
  scene_ = new QGraphicsScene(this);
  preview_view_ = new QGraphicsView(scene_, this);
  preview_view_->setRenderHint(QPainter::Antialiasing);
  preview_view_->setBackgroundBrush(QBrush(QColor(160, 160, 160)));
  preview_view_->setDragMode(QGraphicsView::NoDrag);
  preview_view_->setMinimumWidth(160);

  // --- form (left column) --------------------------------------------------
  auto* form = new QFormLayout;
  form->addRow(QApplication::tr("Layout:"), layout_combo_);
  form->addRow(QApplication::tr("Name:"), name_edit_);

  printer_label_ = new QLabel(QApplication::tr("Printer:"), this);
  printer_label_->setVisible(false);
  printer_combo_ = new QComboBox(this);
  printer_combo_->setVisible(false);
  printer_combo_->setToolTip(QApplication::tr(
      "The label printer this layout prints to. Change it to bind the layout "
      "to a different detected label printer."));

  auto* geo_box = new QGroupBox(
      QApplication::tr("Geometry"), this);
  auto* geo = new QGridLayout(geo_box);
  columns_label_ = new QLabel(QApplication::tr("Columns:"), geo_box);
  geo->addWidget(columns_label_, 0, 0);
  geo->addWidget(columns_spin_, 0, 1);
  rows_label_ = new QLabel(QApplication::tr("Rows:"), geo_box);
  geo->addWidget(rows_label_, 0, 2);
  geo->addWidget(rows_spin_, 0, 3);
  geo->addWidget(
      new QLabel(QApplication::tr("Label width:"), geo_box), 1, 0);
  geo->addWidget(label_width_spin_, 1, 1);
  geo->addWidget(
      new QLabel(QApplication::tr("Label height:"), geo_box), 1, 2);
  geo->addWidget(label_height_spin_, 1, 3);
  left_margin_label_ = new QLabel(QApplication::tr("Left margin:"), geo_box);
  geo->addWidget(left_margin_label_, 2, 0);
  geo->addWidget(left_margin_spin_, 2, 1);
  top_margin_label_ = new QLabel(QApplication::tr("Top margin:"), geo_box);
  geo->addWidget(top_margin_label_, 2, 2);
  geo->addWidget(top_margin_spin_, 2, 3);
  h_gap_label_ = new QLabel(QApplication::tr("H gap:"), geo_box);
  geo->addWidget(h_gap_label_, 3, 0);
  geo->addWidget(h_gap_spin_, 3, 1);
  v_gap_label_ = new QLabel(QApplication::tr("V gap:"), geo_box);
  geo->addWidget(v_gap_label_, 3, 2);
  geo->addWidget(v_gap_spin_, 3, 3);
  // Barcode bar width reduction stays visible in both sheet and printer mode.
  geo->addWidget(
      new QLabel(QApplication::tr("Bar width reduction:"), geo_box), 4, 0);
  geo->addWidget(bar_width_reduction_spin_, 4, 1);
  // Print darkness only applies to thermal label printers.
  darkness_label_ = new QLabel(QApplication::tr("Print darkness:"), geo_box);
  geo->addWidget(darkness_label_, 4, 2);
  geo->addWidget(darkness_spin_, 4, 3);
  // 180° rotation only applies to thermal label printers.
  geo->addWidget(rotate_check_, 5, 0, 1, 4);
  geo->setColumnStretch(1, 1);
  geo->setColumnStretch(3, 1);

  auto* btn_row = new QHBoxLayout;
  btn_row->addWidget(new_btn_);
  btn_row->addWidget(delete_btn_);
  btn_row->addStretch(1);

  auto* printer_row = new QHBoxLayout;
  printer_row->addWidget(printer_label_);
  printer_row->addWidget(printer_combo_, 1);

  auto* left_col = new QVBoxLayout;
  left_col->addLayout(form);
  left_col->addWidget(geo_box);
  left_col->addWidget(count_label_);
  left_col->addLayout(printer_row);
  left_col->addLayout(btn_row);
  left_col->addStretch(1);

  // --- preview (right column) ----------------------------------------------
  auto* preview_col = new QVBoxLayout;
  preview_col->addWidget(
      new QLabel(QApplication::tr("Preview:"), this));
  preview_col->addWidget(preview_view_, 1);

  auto* hbox = new QHBoxLayout(this);
  setMinimumWidth(700);
  hbox->addLayout(left_col);
  hbox->addLayout(preview_col, 1);

  // --- connections ---------------------------------------------------------
  connect(layout_combo_, &QComboBox::currentIndexChanged,
          this, &LabelLayoutWidget::on_layout_changed);
  connect(printer_combo_, &QComboBox::currentIndexChanged,
          this, &LabelLayoutWidget::on_printer_changed);
  connect(new_btn_, &QPushButton::clicked, this, &LabelLayoutWidget::on_new);
  connect(delete_btn_, &QPushButton::clicked, this, &LabelLayoutWidget::on_delete);

  // Live preview on any user field change; also notify the surrounding
  // editor so it can resize its card preview to the new dimensions.
  // (load_into_fields blocks these signals, so this only fires on edits.)
  auto upd = [this](auto) {
    update_preview();
    Q_EMIT geometry_changed(current_preset());
  };
  connect(columns_spin_, &QSpinBox::valueChanged, this, upd);
  connect(rows_spin_, &QSpinBox::valueChanged, this, upd);
  connect(label_width_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(label_height_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(left_margin_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(top_margin_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(h_gap_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(v_gap_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(bar_width_reduction_spin_, &QDoubleSpinBox::valueChanged, this, upd);
  connect(darkness_spin_, &QSpinBox::valueChanged, this, upd);
  connect(rotate_check_, &QCheckBox::toggled, this, upd);

  populate_combo();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void colman::LabelLayoutWidget::populate_combo()
{
  QSignalBlocker blocker(layout_combo_);
  QString const current = layout_combo_->currentData().toString();
  layout_combo_->clear();

  for(LabelLayoutPreset const& p : builtin_layouts()) {
    layout_combo_->addItem(p.name, p.name);
  }

  // Only user layouts of this editor's purpose (plus untagged legacy ones)
  // are listed; built-in templates above stay available regardless.
  QList<LabelLayoutPreset> filtered;
  for(LabelLayoutPreset const& p : load_user_layouts()) {
    if(purpose_ == LabelLayoutPurpose::Any || p.purpose == LabelLayoutPurpose::Any || p.purpose == purpose_) {
      filtered.append(p);
    }
  }
  if(!filtered.isEmpty()) {
    layout_combo_->insertSeparator(layout_combo_->count());
    for(LabelLayoutPreset const& p : filtered) {
      layout_combo_->addItem(
          p.name + QApplication::tr(" (user)"), p.name);
    }
  }

  // Add all detected label printers as a separate section so the user
  // can always create additional layouts for any detected printer.
  QList<colman::ThermalPrinterInfo> const printers =
      colman::list_thermal_printers();
  if(!printers.isEmpty()) {
    layout_combo_->insertSeparator(layout_combo_->count());
    for(colman::ThermalPrinterInfo const& pi : printers) {
      // Use display_name as the combo key (unsaved template entry).
      layout_combo_->addItem(
          pi.display_name + QApplication::tr(" (printer)"),
          pi.display_name);
    }
  }

  // restore previous selection; fall back to last-applied layout, then first
  int idx = layout_combo_->findData(current);
  if(idx < 0) {
    idx = layout_combo_->findData(last_selected_layout_name());
  }
  layout_combo_->setCurrentIndex(idx >= 0 ? idx : 0);
  on_layout_changed(layout_combo_->currentIndex());
}

void colman::LabelLayoutWidget::load_into_fields(
    LabelLayoutPreset const& p)
{
  QSignalBlocker b1(columns_spin_), b2(rows_spin_),
      b3(label_width_spin_), b4(label_height_spin_),
      b5(left_margin_spin_), b6(top_margin_spin_),
      b7(h_gap_spin_), b8(v_gap_spin_), b9(bar_width_reduction_spin_),
      b10(darkness_spin_), b11(rotate_check_);

  name_edit_->setText(p.name);
  columns_spin_->setValue(p.columns);
  rows_spin_->setValue(p.rows);
  label_width_spin_->setValue(p.label_width);
  label_height_spin_->setValue(p.label_height);
  left_margin_spin_->setValue(p.left_margin);
  top_margin_spin_->setValue(p.top_margin);
  h_gap_spin_->setValue(p.h_gap);
  v_gap_spin_->setValue(p.v_gap);
  bar_width_reduction_spin_->setValue(p.bar_width_reduction_mm);
  darkness_spin_->setValue(p.darkness);
  rotate_check_->setChecked(p.rotate_180);
  current_cups_name_ = p.printer_cups_name;
  current_printer_dpi_ = p.printer_dpi;

  // The name is always editable: built-in presets and detected printers
  // serve as starting templates that the user saves under their own name
  // (saving over a built-in name is rejected by validate_save_name()).
  name_edit_->setReadOnly(false);
  delete_btn_->setEnabled(!p.builtin);
  set_printer_mode(p.is_label_printer);
}

void colman::LabelLayoutWidget::select_preset(QString const& name)
{
  int const idx = layout_combo_->findData(name);
  if(idx >= 0) {
    layout_combo_->setCurrentIndex(idx);
  }
}

bool colman::LabelLayoutWidget::is_current_builtin() const
{
  QString const name = layout_combo_->currentData().toString();
  for(LabelLayoutPreset const& p : builtin_layouts()) {
    if(p.name == name) {
      return true;
    }
  }
  return false;
}

colman::LabelLayoutPreset
colman::LabelLayoutWidget::current_preset() const
{
  LabelLayoutPreset p;
  p.name = name_edit_->text().trimmed();
  p.columns = columns_spin_->value();
  p.rows = rows_spin_->value();
  p.label_width = label_width_spin_->value();
  p.label_height = label_height_spin_->value();
  p.left_margin = left_margin_spin_->value();
  p.top_margin = top_margin_spin_->value();
  p.h_gap = h_gap_spin_->value();
  p.v_gap = v_gap_spin_->value();
  p.builtin = is_current_builtin();
  p.is_label_printer = current_is_printer_;
  p.printer_cups_name = current_cups_name_;
  p.printer_dpi = current_printer_dpi_;
  p.bar_width_reduction_mm = bar_width_reduction_spin_->value();
  p.darkness = darkness_spin_->value();
  p.rotate_180 = rotate_check_->isChecked();
  p.purpose = purpose_;
  p.card_arrangement = loaded_card_arrangement_;
  p.sig_arrangement = loaded_sig_arrangement_;
  return p;
}

void colman::LabelLayoutWidget::set_current_card_arrangement(
    IdCardLayout const& arrangement)
{
  loaded_card_arrangement_ = arrangement;
}

void colman::LabelLayoutWidget::set_current_sig_arrangement(
    SignatureLabelLayout const& arrangement)
{
  loaded_sig_arrangement_ = arrangement;
}

void colman::LabelLayoutWidget::reselect_after_save(QString const& name)
{
  populate_combo();
  int const idx = layout_combo_->findData(name);
  if(idx >= 0) {
    layout_combo_->setCurrentIndex(idx);
  }
}

bool colman::LabelLayoutWidget::validate_save_name(
    QString const& name, QString* err)
{
  if(name.trimmed().isEmpty()) {
    if(err) {
      *err = QApplication::tr("Please enter a name for the layout.");
    }
    return false;
  }
  for(LabelLayoutPreset const& b : builtin_layouts()) {
    if(b.name == name) {
      if(err) {
        *err = QApplication::tr(
            "Built-in layouts cannot be overwritten.\n"
            "Please choose a different name.");
      }
      return false;
    }
  }
  return true;
}

void colman::LabelLayoutWidget::set_printer_mode(bool printer_mode)
{
  current_is_printer_ = printer_mode;
  bool const show_sheet = !printer_mode;
  columns_label_->setVisible(show_sheet);
  columns_spin_->setVisible(show_sheet);
  rows_label_->setVisible(show_sheet);
  rows_spin_->setVisible(show_sheet);
  left_margin_label_->setVisible(show_sheet);
  left_margin_spin_->setVisible(show_sheet);
  top_margin_label_->setVisible(show_sheet);
  top_margin_spin_->setVisible(show_sheet);
  h_gap_label_->setVisible(show_sheet);
  h_gap_spin_->setVisible(show_sheet);
  v_gap_label_->setVisible(show_sheet);
  v_gap_spin_->setVisible(show_sheet);
  darkness_label_->setVisible(printer_mode);
  darkness_spin_->setVisible(printer_mode);
  rotate_check_->setVisible(printer_mode);
  printer_label_->setVisible(printer_mode);
  printer_combo_->setVisible(printer_mode);
  if(printer_mode) {
    populate_printer_combo();
  }
}

void colman::LabelLayoutWidget::populate_printer_combo()
{
  QSignalBlocker blocker(printer_combo_);
  printer_combo_->clear();

  bool current_listed = false;
  for(colman::ThermalPrinterInfo const& pi : colman::list_thermal_printers()) {
    printer_combo_->addItem(
        QApplication::tr("%1 (%2 dpi)").arg(pi.display_name).arg(pi.dpi),
        pi.cups_name);
    if(pi.cups_name == current_cups_name_) {
      current_listed = true;
    }
  }

  // Keep the bound printer selectable even when it is currently offline or no
  // longer detected, so editing the layout does not silently drop it.
  if(!current_cups_name_.isEmpty() && !current_listed) {
    printer_combo_->addItem(
        QApplication::tr("%1 (not detected)").arg(current_cups_name_),
        current_cups_name_);
  }

  int const idx = printer_combo_->findData(current_cups_name_);
  if(idx >= 0) {
    printer_combo_->setCurrentIndex(idx);
  }
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void colman::LabelLayoutWidget::on_layout_changed(int /*index*/)
{
  QString const name = layout_combo_->currentData().toString();
  if(name.isEmpty()) {
    return;
  }

  LabelLayoutPreset p;
  bool found = false;
  // search builtins first, then user layouts
  for(LabelLayoutPreset const& b : builtin_layouts()) {
    if(b.name == name) {
      p = b;
      found = true;
      break;
    }
  }
  if(!found) {
    for(LabelLayoutPreset const& u : load_user_layouts()) {
      if(u.name == name) {
        p = u;
        found = true;
        break;
      }
    }
  }
  if(!found) {
    // detected thermal printers (unsaved — keyed by display_name)
    for(colman::ThermalPrinterInfo const& pi : colman::list_thermal_printers()) {
      if(pi.display_name == name) {
        p.name = pi.display_name;
        p.is_label_printer = true;
        p.printer_cups_name = pi.cups_name;
        p.printer_dpi = pi.dpi;
        p.label_width = pi.default_size_mm.width() > 0.0
                            ? pi.default_size_mm.width()
                            : 62.0;
        p.label_height = pi.default_size_mm.height() > 0.0
                             ? pi.default_size_mm.height()
                             : 25.0;
        p.builtin = false;
        found = true;
        break;
      }
    }
  }
  if(!found) {
    return;
  }

  load_into_fields(p);
  loaded_card_arrangement_ = p.card_arrangement;
  loaded_sig_arrangement_ = p.sig_arrangement;
  update_preview();
  // Remember this as the default layout for the next print run.
  set_last_selected_layout_name(name);
  Q_EMIT layout_applied(current_preset());
}

void colman::LabelLayoutWidget::on_printer_changed(int /*index*/)
{
  QString const cups_name = printer_combo_->currentData().toString();
  if(cups_name.isEmpty()) {
    return;
  }
  current_cups_name_ = cups_name;
  // Adopt the resolution of the newly chosen printer; keep the layout's own
  // dimensions untouched.  A printer that is no longer detected keeps the
  // previously stored DPI.
  for(colman::ThermalPrinterInfo const& pi : colman::list_thermal_printers()) {
    if(pi.cups_name == cups_name) {
      current_printer_dpi_ = pi.dpi;
      break;
    }
  }
  update_preview();
}

void colman::LabelLayoutWidget::update_preview()
{
  LabelLayoutPreset const p = current_preset();
  build_preview(scene_, p);
  preview_view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  if(p.is_label_printer) {
    count_label_->setText(
        QApplication::tr("1 label per print job"));
  }
  else {
    count_label_->setText(
        QApplication::tr("%1 labels per sheet (%2 × %3)")
            .arg(p.columns * p.rows)
            .arg(p.columns)
            .arg(p.rows));
  }
}

void colman::LabelLayoutWidget::on_new()
{
  // Deselect combo so it shows nothing — prevents confusion about which
  // entry the new layout would overwrite.
  QSignalBlocker blocker(layout_combo_);
  layout_combo_->setCurrentIndex(-1);

  LabelLayoutPreset p;         // defaults
  p.builtin = false;
  p.name = QString();
  load_into_fields(p);
  loaded_card_arrangement_ = IdCardLayout{};
  loaded_sig_arrangement_ = SignatureLabelLayout{};
  name_edit_->setReadOnly(false);
  name_edit_->setFocus();
  delete_btn_->setEnabled(false);
  update_preview();
  Q_EMIT layout_applied(current_preset());
}

void colman::LabelLayoutWidget::on_delete()
{
  if(is_current_builtin()) {
    return;
  }
  QString const name = layout_combo_->currentData().toString();
  if(name.isEmpty()) {
    return;
  }
  if(colman::question_yes_no(
         this,
         QApplication::tr("Delete layout"),
         QApplication::tr("Delete user layout \"%1\"?").arg(name)) != QMessageBox::Yes) {
    return;
  }
  delete_user_layout(name);
  populate_combo();
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

void colman::LabelLayoutWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  if(!scene_->sceneRect().isEmpty()) {
    preview_view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  }
}

void colman::LabelLayoutWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  if(!scene_->sceneRect().isEmpty()) {
    preview_view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  }
}

/* EOF */
