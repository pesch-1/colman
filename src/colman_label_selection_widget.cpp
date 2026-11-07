// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of LabelSelectionWidget.
/**
   @file
   @author Christoph Pesch

   Scene coordinate system: 1 unit = 0.1 mm (matches LabelLayoutWidget).
   Page size assumed A4 portrait: 210 × 297 mm.
*/

#include "colman_label_selection_widget.hpp"
#ifndef COLMAN_LABEL_PRINTER_HPP
#  include "colman_label_printer.hpp"
#endif

#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
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
#ifndef SYSTEM_QGRAPHICSITEM
#  include <QGraphicsItem>
#  define SYSTEM_QGRAPHICSITEM
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
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
#ifndef SYSTEM_QSHOWEVENT
#  include <QShowEvent>
#  define SYSTEM_QSHOWEVENT
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif

#ifndef SYSTEM_FUNCTIONAL
#  include <functional>
#  define SYSTEM_FUNCTIONAL
#endif
#ifndef SYSTEM_CMATH
#  include <cmath>
#  define SYSTEM_CMATH
#endif
#ifndef SYSTEM_QWHEELEVENT
#  include <QWheelEvent>
#  define SYSTEM_QWHEELEVENT
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif

namespace
{

/// Converts millimetres to scene units (0.1 mm per unit).
  inline qreal mm(qreal x)
  {
    return x * 10.0;
  }

/// Clickable rectangle representing one label on the sheet.
/**
   Calls the provided callback with (col, row) on left-click.  Visual
   appearance reflects whether the label is selected, unselected, or
   blocked (max already reached).
*/
  class LabelItem : public QGraphicsRectItem
  {
  public:
    LabelItem(int col, int row, QRectF const& rect,
              std::function<void(int, int)> on_click)
        : QGraphicsRectItem(rect), col_(col), row_(row), on_click_(std::move(on_click))
    {
      setAcceptedMouseButtons(Qt::LeftButton);
      update_look(false, false);
    }

  /// Updates the visual state of the item.
  /**
   * @param selected  true when this label is selected
   * @param blocked   true when max_count is reached and this label is not selected
   */
    void update_look(bool selected, bool blocked)
    {
      selected_ = selected;
      if(selected_) {
        setBrush(QColor(70, 130, 210, 210));
        setPen(QPen(QColor(30, 80, 170), 2));
        setCursor(Qt::PointingHandCursor);
      }
      else if(blocked) {
        setBrush(QColor(210, 210, 210));
        setPen(QPen(Qt::gray, 1));
        setCursor(Qt::ForbiddenCursor);
      }
      else {
        setBrush(Qt::white);
        setPen(QPen(QColor(60, 100, 200), 2));
        setCursor(Qt::PointingHandCursor);
      }
    }

    int col() const { return col_; } ///< Column index (0-based).
    int row() const { return row_; } ///< Row index (0-based).
    bool is_selected() const { return selected_; }

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override
    {
      on_click_(col_, row_);
    }

  private:
    int col_ = 0, row_ = 0;
    bool selected_ = false;
    std::function<void(int, int)> on_click_;
  };

} // anonymous namespace

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

colman::LabelSelectionWidget::LabelSelectionWidget(
    int max_count, QWidget* parent)
    : QWidget(parent), max_count_(max_count)
{
  build_ui();
  populate_combo();
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

QList<colman::SelectedLabel>
colman::LabelSelectionWidget::selected_labels() const
{
  // For a label printer there is no sheet position — one label per job.
  if(layout_.is_label_printer) {
    SelectedLabel s;
    s.col = 0;
    s.row = 0;
    s.x_mm = 0.0;
    s.y_mm = 0.0;
    s.width_mm = layout_.label_width;
    s.height_mm = layout_.label_height;
    QList<SelectedLabel> r;
    r.append(s);
    return r;
  }

  QList<SelectedLabel> result;
  double const dx = layout_.label_width + layout_.h_gap;
  double const dy = layout_.label_height + layout_.v_gap;
  for(int i = 0; i < items_.size(); ++i) {
    if(!selected_[i]) {
      continue;
    }
    LabelItem const* item = static_cast<LabelItem const*>(items_[i]);
    SelectedLabel s;
    s.col = item->col();
    s.row = item->row();
    s.x_mm = layout_.left_margin + s.col * dx;
    s.y_mm = layout_.top_margin + s.row * dy;
    s.width_mm = layout_.label_width;
    s.height_mm = layout_.label_height;
    result.append(s);
  }
  return result;
}

void colman::LabelSelectionWidget::set_max_count(int n)
{
  max_count_ = n;
  if(selected_.count(true) > n) {
    selected_.fill(false, selected_.size());
  }
  update_item_states();
  update_status();
}

void colman::LabelSelectionWidget::clear_selection()
{
  selected_.fill(false, selected_.size());
  update_item_states();
  update_status();
  Q_EMIT selection_changed();
}

void colman::LabelSelectionWidget::refresh_layouts()
{
  populate_combo();
}

void colman::LabelSelectionWidget::set_user_layouts_only(bool on)
{
  user_layouts_only_ = on;
  if(edit_btn_) {
    edit_btn_->setVisible(!on);
  }
  populate_combo();
}

void colman::LabelSelectionWidget::set_layout_purpose(
    colman::LabelLayoutPurpose purpose)
{
  layout_purpose_ = purpose;
  populate_combo();
}

bool colman::LabelSelectionWidget::has_no_user_layouts_for(
    colman::LabelLayoutPurpose purpose)
{
  for(LabelLayoutPreset const& p : LabelLayoutWidget::load_user_layouts()) {
    if(purpose == LabelLayoutPurpose::Any
       || p.purpose == LabelLayoutPurpose::Any
       || p.purpose == purpose) {
      return false;
    }
  }
  return true;
}

void colman::LabelSelectionWidget::set_initial_preset_by_name(
    QString const& name)
{
  if(name.isEmpty()) {
    return;
  }
  int const idx = layout_combo_->findData(name);
  if(idx < 0) {
    return;
  }
  {
    QSignalBlocker b(layout_combo_);
    layout_combo_->setCurrentIndex(idx);
  }
  on_layout_combo_changed(idx);
}

void colman::LabelSelectionWidget::set_initial_preset_for_card(
    double card_w_mm, double card_h_mm)
{
  double constexpr Tol = 0.5; // mm tolerance for dimension matching
  auto matches = [&](LabelLayoutPreset const& p) -> bool {
    return std::abs(p.label_width - card_w_mm) < Tol && std::abs(p.label_height - card_h_mm) < Tol;
  };
  // Select the first matching preset that is actually offered in the combo.
  // Skipping entries that are absent (e.g. factory presets in user-only
  // mode) lets a matching user layout still be picked.
  auto try_select = [&](QList<LabelLayoutPreset> const& list) -> bool {
    for(LabelLayoutPreset const& p : list) {
      if(matches(p)) {
        int const idx = layout_combo_->findData(p.name);
        if(idx >= 0) {
          layout_combo_->setCurrentIndex(idx);
          return true;
        }
      }
    }
    return false;
  };
  if(try_select(LabelLayoutWidget::builtin_layouts())) {
    return;
  }
  try_select(LabelLayoutWidget::load_user_layouts());
}

// ---------------------------------------------------------------------------
// Private: UI construction
// ---------------------------------------------------------------------------

void colman::LabelSelectionWidget::build_ui()
{
  // --- top bar: layout selector ---
  auto* layout_label = new QLabel(QApplication::tr("Layout:"), this);
  layout_combo_ = new QComboBox(this);

  edit_btn_ = new QPushButton(this);
  edit_btn_->setIcon(QIcon(":/icons/owls/edit_label_layouts.jpeg"));
  edit_btn_->setIconSize(QSize(32, 32));
  // edit_btn_->setFixedSize(36, 36);
  edit_btn_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  edit_btn_->setToolTip(QApplication::tr("Edit label layouts"));
  edit_btn_->setText(QApplication::tr("Edit label layouts"));
  auto* top_bar = new QHBoxLayout;
  top_bar->addWidget(layout_label);
  top_bar->addWidget(layout_combo_, 1);
  top_bar->addWidget(edit_btn_);

  // --- page preview ---
  scene_ = new QGraphicsScene(this);
  view_ = new QGraphicsView(scene_, this);
  view_->setRenderHint(QPainter::Antialiasing);
  view_->setBackgroundBrush(QBrush(QColor(160, 160, 160)));
  view_->setDragMode(QGraphicsView::NoDrag);
  view_->setMinimumHeight(300);
  view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  view_->viewport()->installEventFilter(this);

  // --- bottom bar: deselect + select all + status ---
  deselect_btn_ = new QPushButton(QApplication::tr("Deselect all"), this);
  select_all_btn_ = new QPushButton(QApplication::tr("Select all"), this);
  status_label_ = new QLabel(this);

  auto* bottom_bar = new QHBoxLayout;
  bottom_bar->addWidget(deselect_btn_);
  bottom_bar->addWidget(select_all_btn_);
  bottom_bar->addStretch();
  bottom_bar->addWidget(status_label_);

  // --- main layout ---
  auto* vbox = new QVBoxLayout(this);
  vbox->addLayout(top_bar);
  vbox->addWidget(view_, 1);
  vbox->addLayout(bottom_bar);

  // --- connections ---
  connect(layout_combo_, &QComboBox::currentIndexChanged,
          this, &LabelSelectionWidget::on_layout_combo_changed);
  connect(edit_btn_, &QPushButton::clicked,
          this, &LabelSelectionWidget::on_edit_layouts);
  connect(deselect_btn_, &QPushButton::clicked,
          this, &LabelSelectionWidget::on_deselect_all);
  connect(select_all_btn_, &QPushButton::clicked,
          this, &LabelSelectionWidget::on_select_all);
}

// ---------------------------------------------------------------------------
// Private: layout management
// ---------------------------------------------------------------------------

void colman::LabelSelectionWidget::populate_combo()
{
  QSignalBlocker b(layout_combo_);
  QString const current = layout_combo_->currentData().toString();
  layout_combo_->clear();

  QList<LabelLayoutPreset> const user = LabelLayoutWidget::load_user_layouts();

  if(user_layouts_only_) {
    // Print dialogs: only administrator-created, named layouts of the matching
    // purpose (plus untagged legacy ones).  No factory (Avery) presets and no
    // raw printer entries.
    for(LabelLayoutPreset const& p : user) {
      if(layout_purpose_ == LabelLayoutPurpose::Any
         || p.purpose == LabelLayoutPurpose::Any
         || p.purpose == layout_purpose_) {
        layout_combo_->addItem(p.name, p.name);
      }
    }
  }
  else {
    for(LabelLayoutPreset const& p : LabelLayoutWidget::builtin_layouts()) {
      layout_combo_->addItem(p.name, p.name);
    }
    if(!user.isEmpty()) {
      layout_combo_->insertSeparator(layout_combo_->count());
      for(LabelLayoutPreset const& p : user) {
        layout_combo_->addItem(
            p.name + QApplication::tr(" (user)"), p.name);
      }
    }

    // Add all detected label printers so the user can always create
    // additional layouts for any detected printer.
    QList<colman::ThermalPrinterInfo> const printers =
        colman::list_thermal_printers();
    if(!printers.isEmpty()) {
      layout_combo_->insertSeparator(layout_combo_->count());
      for(colman::ThermalPrinterInfo const& pi : printers) {
        layout_combo_->addItem(
            pi.display_name + QApplication::tr(" (printer)"),
            pi.display_name);
      }
    }
  }

  int const idx = layout_combo_->findData(current);
  layout_combo_->setCurrentIndex(idx >= 0 ? idx : 0);
  // apply the (possibly restored) selection
  on_layout_combo_changed(layout_combo_->currentIndex());
}

void colman::LabelSelectionWidget::apply_layout(
    LabelLayoutPreset const& p)
{
  layout_ = p;
  rebuild_scene();
  Q_EMIT layout_changed();
}

// ---------------------------------------------------------------------------
// Private: scene
// ---------------------------------------------------------------------------

void colman::LabelSelectionWidget::rebuild_scene()
{
  scene_->clear();
  items_.clear();

  qreal const lw = mm(layout_.label_width);
  qreal const lh = mm(layout_.label_height);

  if(layout_.is_label_printer) {
    // Single auto-selected label — no sheet, no position choice.
    selected_.fill(true, 1);
    scene_->setSceneRect(0, 0, lw, lh);
    auto* item = new LabelItem(
        0, 0, QRectF(0, 0, lw, lh),
        [](int, int) {});   // clicks ignored
    scene_->addItem(item);
    item->update_look(true, false);
    items_.append(item);
    deselect_btn_->setEnabled(false);
    select_all_btn_->setEnabled(false);
    update_status();
    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    return;
  }

  deselect_btn_->setEnabled(true);
  select_all_btn_->setEnabled(true);
  int const n = layout_.rows * layout_.columns;
  selected_.fill(false, n);

  // page background + border
  qreal const pw = mm(layout_.page_width_mm);
  qreal const ph = mm(layout_.page_height_mm);
  scene_->setSceneRect(0, 0, pw, ph);
  scene_->addRect(0, 0, pw, ph, QPen(Qt::black, 3), QBrush(Qt::white));

  // label items
  qreal const dx = mm(layout_.label_width + layout_.h_gap);
  qreal const dy = mm(layout_.label_height + layout_.v_gap);
  qreal const lm = mm(layout_.left_margin);
  qreal const tm = mm(layout_.top_margin);

  for(int row = 0; row < layout_.rows; ++row) {
    for(int col = 0; col < layout_.columns; ++col) {
      qreal const x = lm + col * dx;
      qreal const y = tm + row * dy;
      auto* item = new LabelItem(
          col, row, QRectF(x, y, lw, lh),
          [this](int c, int r) { toggle_label(c, r); });
      scene_->addItem(item);
      items_.append(item);
    }
  }

  update_item_states();
  update_status();
  view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void colman::LabelSelectionWidget::toggle_label(int col, int row)
{
  int const idx = row * layout_.columns + col;
  if(idx < 0 || idx >= selected_.size()) {
    return;
  }
  if(!selected_[idx] && selected_.count(true) >= max_count_) {
    return;
  }
  selected_[idx] = !selected_[idx];
  update_item_states();
  update_status();
  Q_EMIT selection_changed();
}

void colman::LabelSelectionWidget::update_item_states()
{
  int const n_sel = selected_.count(true);
  bool const at_max = (n_sel >= max_count_);
  for(int i = 0; i < items_.size(); ++i) {
    bool const sel = selected_[i];
    static_cast<LabelItem*>(items_[i])->update_look(sel, !sel && at_max);
  }
}

void colman::LabelSelectionWidget::update_status()
{
  if(layout_.is_label_printer) {
    status_label_->setText(QApplication::tr("1 label per print job"));
    return;
  }
  int const n = selected_.count(true);
  status_label_->setText(
      QApplication::tr("%1 / %2 labels selected").arg(n).arg(max_count_));
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void colman::LabelSelectionWidget::on_layout_combo_changed(int index)
{
  QString const name = layout_combo_->itemData(index).toString();
  if(name.isEmpty()) {
    return;
  }
  for(LabelLayoutPreset const& p : LabelLayoutWidget::builtin_layouts()) {
    if(p.name == name) {
      apply_layout(p);
      return;
    }
  }
  for(LabelLayoutPreset const& p : LabelLayoutWidget::load_user_layouts()) {
    if(p.name == name) {
      apply_layout(p);
      return;
    }
  }
  // Unsaved detected thermal printer (keyed by display_name).
  for(colman::ThermalPrinterInfo const& pi : colman::list_thermal_printers()) {
    if(pi.display_name == name) {
      LabelLayoutPreset p;
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
      apply_layout(p);
      return;
    }
  }
}

void colman::LabelSelectionWidget::on_edit_layouts()
{
  QDialog dlg(this);
  dlg.setWindowTitle(QApplication::tr("Edit label layouts"));
  auto* lw = new LabelLayoutWidget(&dlg);
  auto* bb = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
  auto* vbox = new QVBoxLayout(&dlg);
  vbox->addWidget(lw);
  vbox->addWidget(bb);
  connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
  connect(lw, &LabelLayoutWidget::layout_applied,
          this, &LabelSelectionWidget::on_layout_applied);
  dlg.exec();
}

void colman::LabelSelectionWidget::on_layout_applied(
    LabelLayoutPreset const& p)
{
  populate_combo();
  int const idx = layout_combo_->findData(p.name);
  if(idx >= 0) {
    layout_combo_->setCurrentIndex(idx);
  }
  else {
    apply_layout(p);
  }
}

void colman::LabelSelectionWidget::on_deselect_all()
{
  selected_.fill(false, selected_.size());
  update_item_states();
  update_status();
  Q_EMIT selection_changed();
}

/**
   Selects all positions on the sheet, starting with the first one in
   row-major order.  Already selected positions stay selected; further
   positions are added until @c max_count is reached.
*/
void colman::LabelSelectionWidget::on_select_all()
{
  auto n_sel = selected_.count(true);
  for(int i = 0; i < selected_.size() && n_sel < max_count_; ++i) {
    if(!selected_[i]) {
      selected_[i] = true;
      ++n_sel;
    }
  }
  update_item_states();
  update_status();
  Q_EMIT selection_changed();
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

bool colman::LabelSelectionWidget::eventFilter(QObject* obj, QEvent* e)
{
  if(obj == view_->viewport() && e->type() == QEvent::Wheel) {
    auto* we = static_cast<QWheelEvent*>(e);
    if(we->modifiers() & Qt::ControlModifier) {
      qreal const factor = we->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
      view_->scale(factor, factor);
      return true;
    }
  }
  return QWidget::eventFilter(obj, e);
}


void colman::LabelSelectionWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  if(!scene_->sceneRect().isEmpty()) {
    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  }
}

void colman::LabelSelectionWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  if(!scene_->sceneRect().isEmpty()) {
    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
  }
}

/* EOF */
