// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of SignaturePrintPage.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_signature_print_page.hpp"
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#endif
#ifndef COLMAN_LABEL_PRINTER_HPP
#  include "colman_label_printer.hpp"
#endif
#ifndef COLMAN_PRINTER_CALIBRATION_HPP
#  include "colman_printer_calibration.hpp"
#endif
#ifndef COLMAN_LABEL_SELECTION_WIDGET_HPP
#  include "colman_label_selection_widget.hpp"
#endif
#ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP
#  include "colman_label_layout_widget.hpp"
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
#ifndef SYSTEM_QGRAPHICSITEM
#  include <QGraphicsItem>
#  define SYSTEM_QGRAPHICSITEM
#endif
#ifndef SYSTEM_QGRAPHICSPIXMAPITEM
#  include <QGraphicsPixmapItem>
#  define SYSTEM_QGRAPHICSPIXMAPITEM
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QIMAGE
#  include <QImage>
#  define SYSTEM_QIMAGE
#endif
#ifndef SYSTEM_QITEMSELECTIONMODEL
#  include <QItemSelectionModel>
#  define SYSTEM_QITEMSELECTIONMODEL
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPAGELAYOUT
#  include <QPageLayout>
#  define SYSTEM_QPAGELAYOUT
#endif
#ifndef SYSTEM_QPAGESIZE
#  include <QPageSize>
#  define SYSTEM_QPAGESIZE
#endif
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPRINTDIALOG
#  include <QPrintDialog>
#  define SYSTEM_QPRINTDIALOG
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef COLMAN_SIGNATURE_LABEL_LAYOUT_HPP
#  include "colman_signature_label_layout.hpp"
#endif
#ifndef SYSTEM_QSIZEF
#  include <QSizeF>
#  define SYSTEM_QSIZEF
#endif
#ifndef SYSTEM_QSIZEPOLICY
#  include <QSizePolicy>
#  define SYSTEM_QSIZEPOLICY
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QSTANDARDITEMMODEL
#  include <QStandardItemModel>
#  define SYSTEM_QSTANDARDITEMMODEL
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif


namespace
{

/// QGraphicsItem that paints a single signature text string.
/**
   Uses pixel-size fonts relative to the item's scene-coordinate rect
   so that the text scales correctly when the scene is rendered at any
   device resolution (screen or printer).  No resize handles are drawn.
*/
  class SignatureTextItem : public QGraphicsItem
  {
  public:
    SignatureTextItem(QString const& text, QRectF const& rect)
        : text_(text), rect_(rect)
    {}

    QRectF boundingRect() const override { return rect_; }

    void paint(QPainter* p,
               QStyleOptionGraphicsItem const*,
               QWidget*) override
    {
      if(text_.isEmpty() || rect_.isEmpty()) {
        return;
      }
      p->setPen(Qt::black);
      int const px = std::max(1, (int)(rect_.height() * 0.55));
      QFont font("Arial");
      font.setPixelSize(px);
      font.setBold(true);
    // Scale font down if text is wider than the available zone.
      QFontMetrics fm(font);
      int const adv = fm.horizontalAdvance(text_);
      qreal const avail = rect_.width() - 4.0;
      if(avail > 0 && adv > avail) {
        int const px2 = std::max(1, (int)(px * avail / adv));
        font.setPixelSize(px2);
      }
      p->setFont(font);
      p->drawText(rect_, Qt::AlignCenter, text_);
    }

  private:
    QString text_;
    QRectF rect_;
  };


/// Populates @p scene with a white background and a centred text item.
/**
   The text zone position is taken from @p layout.
   @p w_mm and @p h_mm are the label dimensions in millimetres.
   Scene coordinate convention: 1 unit = 0.1 mm.

   The text-zone coordinates stored in @p layout are absolute millimetres
   authored for the label size @c layout.card_width_mm_ ×
   @c layout.card_height_mm_.  When the requested label (@p w_mm × @p h_mm)
   has a different size the zone is scaled proportionally so that selecting
   another label layout takes effect immediately, instead of reusing the
   stale coordinates of the previously designed label.
*/
  void build_signature_label_scene(
      QGraphicsScene* scene,
      QString const& text,
      double w_mm,
      double h_mm,
      colman::SignatureLabelLayout const& layout)
  {
    qreal const cw = w_mm * 10.0;
    qreal const ch = h_mm * 10.0;
    scene->setSceneRect(0, 0, cw, ch);
    scene->addRect(0, 0, cw, ch, QPen(Qt::NoPen), QBrush(Qt::white));

    if(text.isEmpty()) {
      return;
    }

    // Scale the authored text zone to the actually requested label size.
    double const sx =
        layout.card_width_mm_ > 0.0 ? w_mm / layout.card_width_mm_ : 1.0;
    double const sy =
        layout.card_height_mm_ > 0.0 ? h_mm / layout.card_height_mm_ : 1.0;

    double const tx = layout.text_x_mm_ * sx * 10.0;
    double const ty = layout.text_y_mm_ * sy * 10.0;
    double const tw_raw =
        (layout.text_width_mm_ > 0.0
             ? layout.text_width_mm_ * sx
             : w_mm) *
        10.0;
    double const th_raw =
        (layout.text_height_mm_ > 0.0
             ? layout.text_height_mm_ * sy
             : h_mm) *
        10.0;

    double const min_sz = 5.0;
    double const cx = std::max(0.0, std::min(tx, cw - min_sz));
    double const cy = std::max(0.0, std::min(ty, ch - min_sz));
    double const cw2 = std::max(min_sz, std::min(tw_raw, cw - cx));
    double const ch2 = std::max(min_sz, std::min(th_raw, ch - cy));

    auto* ti = new SignatureTextItem(text, QRectF(cx, cy, cw2, ch2));
    scene->addItem(ti);
  }


/// Full-page layout chooser and batch printer for signature labels.
/**
   Occupies a page-stack slot; the Back button and Cancel return to the
   originating page.  At the top a LabelSelectionWidget lets the operator
   choose the label layout — restricted to user-defined, named layouts —
   and, for sheet layouts, the label positions, with the signature text
   previewed in each selected label.  The default selection is the layout
   last saved in the signature label editor.

   The user clicks "Print" to send the labels to the chosen printer:
   thermal layouts print one label per copy directly; sheet layouts print
   the current page and repeat until the requested label count is reached
   or the user cancels.  The page is created per invocation and deletes
   itself when done.
*/
  class SignatureBatchPrintPage : public QWidget
  {
  public:
    SignatureBatchPrintPage(QString const& text,
                            int count,
                            QString const& default_name,
                            QWidget* parent = nullptr)
        : QWidget(parent), text_(text), remaining_(count)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);
      vbox->setContentsMargins(0, 0, 0, 0);

      QPushButton* back_btn = new QPushButton(
          QApplication::tr("Back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn);

      auto* heading = new colman::ScalingLabel(
          "<b>" + QApplication::tr("Print signatures") + "</b>", this, 2);
      vbox->addWidget(heading);

      status_label_ = new QLabel(this);
      update_status();
      vbox->addWidget(status_label_);

      label_sel_ = new colman::LabelSelectionWidget(remaining_, this);
      label_sel_->set_user_layouts_only(true);
      label_sel_->set_layout_purpose(colman::LabelLayoutPurpose::Signature);
      // Default to the layout last used / edited for signatures.
      label_sel_->set_initial_preset_by_name(default_name);
      vbox->addWidget(label_sel_, 1);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      QPushButton* cancel_btn = new QPushButton(
          QApplication::tr("Cancel"), this);
      print_btn_ = new QPushButton(
          QApplication::tr("Print"), this);
      btn_hbox->addWidget(cancel_btn);
      btn_hbox->addStretch();
      btn_hbox->addWidget(print_btn_);
      vbox->addLayout(btn_hbox);

      // Without any user-defined signature layout there is nothing to print.
      if(colman::LabelSelectionWidget::has_no_user_layouts_for(
             colman::LabelLayoutPurpose::Signature)) {
        status_label_->setText(QApplication::tr(
            "No label layouts have been defined yet. Please ask an "
            "administrator to create and name a label layout first."));
        print_btn_->setEnabled(false);
      }

      connect(label_sel_,
              &colman::LabelSelectionWidget::layout_changed,
              [this]() {
                preview_items_.clear();
                update_previews();
              });
      connect(label_sel_,
              &colman::LabelSelectionWidget::selection_changed,
              [this]() { remove_previews(); update_previews(); });
      connect(print_btn_, &QPushButton::clicked,
              [this]() { do_batch(); });
      connect(cancel_btn, &QPushButton::clicked,
              [this]() { do_cancel(); });
      connect(back_btn, &QPushButton::clicked,
              [this]() { do_cancel(); });

      // The initial preset was selected above before these connections
      // existed, so its layout_changed went unhandled. Draw the preview
      // for the current selection now (e.g. the auto-selected single label
      // of a thermal layout would otherwise show no text until a switch).
      update_previews();
    }

    ~SignatureBatchPrintPage() override
    {
      if(print_started_ && painter_ && painter_->isActive()) {
        painter_->end();
      }
    }

  /// Sets the callback invoked when the batch finishes or is cancelled.
    void set_on_done(std::function<void()> on_done)
    {
      on_done_ = std::move(on_done);
    }

  private:
  /// Invokes the done callback, returning to the originating page.
    void finish()
    {
      if(on_done_) {
        on_done_();
      }
    }

    void update_status()
    {
      status_label_->setText(
          QApplication::tr("Remaining: %1 labels").arg(remaining_));
    }

  /// Removes all signature-text preview overlays from the label scene.
    void remove_previews()
    {
      QGraphicsScene* sc = label_sel_->view_scene();
      for(QGraphicsItem* it : preview_items_) {
        sc->removeItem(it);
        delete it;
      }
      preview_items_.clear();
    }

  /// Adds a text-preview pixmap for each currently selected label.
    void update_previews()
    {
      QGraphicsScene* sc = label_sel_->view_scene();
      QList<colman::SelectedLabel> const sel = label_sel_->selected_labels();
      colman::SignatureLabelLayout const arr =
          label_sel_->current_layout().sig_arrangement;
      int const n = std::min((int)sel.size(), remaining_);
      for(int i = 0; i < n; ++i) {
        colman::SelectedLabel const& lbl = sel[i];
        qreal const x = lbl.x_mm * 10.0;
        qreal const y = lbl.y_mm * 10.0;
        qreal const sw = lbl.width_mm * 10.0;
        qreal const sh = lbl.height_mm * 10.0;

        QGraphicsScene card_scene;
        build_signature_label_scene(
            &card_scene, text_, lbl.width_mm, lbl.height_mm, arr);

        int const img_w = qMax(1, (int)(sw) * 2);
        int const img_h = qMax(1, (int)(sh) * 2);
        QImage img(img_w, img_h, QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        card_scene.render(&p, QRectF(0, 0, img_w, img_h),
                          card_scene.sceneRect());
        p.end();

        auto* pix = new QGraphicsPixmapItem(QPixmap::fromImage(img));
        pix->setPos(x, y);
        pix->setTransform(
            QTransform::fromScale(sw / img_w, sh / img_h));
        pix->setZValue(1.0);
        pix->setAcceptedMouseButtons(Qt::NoButton);
        sc->addItem(pix);
        preview_items_.append(pix);
      }
    }

  /// Prints the currently selected label positions on the sheet or thermal printer.
    void do_batch()
    {
      colman::LabelLayoutPreset const cur = label_sel_->current_layout();

      // Remember the chosen layout as the new default, regardless of
      // whether the actual print succeeds (the thermal printer may be
      // offline, but the user's layout choice should still stick).
      if(!cur.name.isEmpty()) {
        QSettings().setValue("label_layouts/last_selected", cur.name);
      }
      colman::SignatureLabelLayout const arr = cur.sig_arrangement;

      if(cur.is_label_printer) {
        if(!colman::thermal_printer_ready(cur.printer_cups_name)) {
          QMessageBox::warning(
              this,
              QApplication::tr("Printer not connected"),
              QApplication::tr(
                  "Please connect the \"%1\" printer and try again, "
                  "or select a different layout.")
                  .arg(cur.printer_cups_name));
          return;
        }
        if(colman::warn_when_out_of_labels(this)) {
          return;
        }
        QSizeF const label_mm(cur.label_width, cur.label_height);
        // All copies carry the same text, so render once and repeat the image.
        QGraphicsScene scene;
        build_signature_label_scene(
            &scene, text_, cur.label_width, cur.label_height, arr);
        QImage const img = colman::render_label_scene(
            &scene, label_mm, cur.printer_dpi,
            colman::load_printer_calibration(cur.printer_cups_name),
            cur.rotate_180);
        QList<QImage> images;
        for(int i = 0; i < remaining_; ++i) {
          images.append(img);
        }
        QString err;
        if(!colman::send_images_to_thermal_printer(
               images, cur.printer_cups_name, label_mm, cur.printer_dpi, err,
               cur.darkness)) {
          QMessageBox::warning(this, QApplication::tr("Print error"), err);
          return;
        }
        colman::warn_when_roll_runs_out_while_printing(
            this, images.size(), label_mm);
        finish();
        return;
      }

      QList<colman::SelectedLabel> const labels =
          label_sel_->selected_labels();
      if(labels.isEmpty()) {
        QMessageBox::information(
            this,
            QApplication::tr("Print signatures"),
            QApplication::tr(
                "Please select at least one label position."));
        return;
      }

      if(!print_started_) {
        printer_ = std::make_unique<QPrinter>(QPrinter::HighResolution);
        printer_->setFullPage(true);
        printer_->setPageSize(QPageSize(
            QSizeF(cur.page_width_mm, cur.page_height_mm),
            QPageSize::Millimeter));
        printer_->setPageMargins(
            QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

        QPrintDialog dlg(printer_.get(), this);
        if(dlg.exec() != QDialog::Accepted) {
          printer_.reset();
          return;
        }
        painter_ = std::make_unique<QPainter>(printer_.get());
        if(!painter_->isActive()) {
          printer_.reset();
          painter_.reset();
          return;
        }
        painter_->setRenderHint(QPainter::Antialiasing);
        print_started_ = true;
        first_page_ = true;
      }

      if(!first_page_) {
        printer_->newPage();
      }
      first_page_ = false;

      qreal const dpi = printer_->resolution();
      qreal const mm_to_px = dpi / 25.4;
      int const n = std::min((int)labels.size(), remaining_);
      for(int i = 0; i < n; ++i) {
        colman::SelectedLabel const& lbl = labels[i];
        QGraphicsScene scene;
        build_signature_label_scene(
            &scene, text_, lbl.width_mm, lbl.height_mm, arr);
        scene.render(
            painter_.get(),
            QRectF(lbl.x_mm * mm_to_px, lbl.y_mm * mm_to_px,
                   lbl.width_mm * mm_to_px, lbl.height_mm * mm_to_px));
      }
      remaining_ -= n;

      if(remaining_ <= 0) {
        painter_->end();
        printer_.reset();
        painter_.reset();
        finish();
      }
      else {
        update_status();
        label_sel_->set_max_count(remaining_);
        label_sel_->clear_selection();
        remove_previews();
      }
    }

  /// Ends any open print job and returns to the originating page.
    void do_cancel()
    {
      if(print_started_ && painter_ && painter_->isActive()) {
        painter_->end();
      }
      printer_.reset();
      painter_.reset();
      finish();
    }

    QString text_;
    int remaining_;
    std::function<void()> on_done_;
    colman::LabelSelectionWidget* label_sel_ = nullptr;
    QPushButton* print_btn_ = nullptr;
    QLabel* status_label_ = nullptr;
    QList<QGraphicsItem*> preview_items_;
    std::unique_ptr<QPrinter> printer_;
    std::unique_ptr<QPainter> painter_;
    bool print_started_ = false;
    bool first_page_ = true;
  };

/// Shows a transient batch-print page on the stack for the sheet path.
/**
   Captures the currently visible page as the return target, so the batch
   page returns there once the user finishes or cancels. The page deletes
   itself afterwards.
*/
  void show_signature_batch_print_page(
      colman::AppData& data,
      QString const& text,
      int count)
  {
    QString const default_name =
        colman::LabelLayoutWidget::last_selected_layout_name();
    QWidget* const back = data.mw_.pages_stack_->currentWidget();
    auto* page = new SignatureBatchPrintPage(text, count, default_name);
    data.mw_.pages_stack_->addWidget(page);
    page->set_on_done([&data, page, back]() {
      data.mw_.pages_stack_->setCurrentWidget(back);
      data.mw_.pages_stack_->removeWidget(page);
      page->deleteLater();
    });
    data.mw_.pages_stack_->setCurrentWidget(page);
  }

} // namespace


/**
   Dispatches the print of @p count signature labels with @p text.

   Only user-defined, named layouts are considered; the layout last saved
   in the signature label editor is used as the default.  When that layout
   is a connected thermal printer the labels are printed directly without a
   dialog (so acquisition is not interrupted).  In every other case the
   batch-print page is shown so the operator can choose the layout and, for
   sheet printers, the label positions.
   @param text  text to print on every label.
   @param count number of labels requested.
   @param data  central application state (queries and page stack).
*/
void colman::print_signature_labels(QString const& text,
                                    int count,
                                    colman::AppData& data)
{
  if(text.trimmed().isEmpty() || count <= 0) {
    return;
  }

  QString const preset_name =
      colman::LabelLayoutWidget::last_selected_layout_name();

  colman::LabelLayoutPreset preset;
  bool found = false;
  if(!preset_name.isEmpty()) {
    for(colman::LabelLayoutPreset const& p :
        colman::LabelLayoutWidget::load_user_layouts()) {
      if(p.name == preset_name) {
        preset = p;
        found = true;
        break;
      }
    }
  }

  // Thermal + printer available: print directly without a dialog.
  if(found && preset.is_label_printer && colman::thermal_printer_ready(preset.printer_cups_name))
  {
    if(colman::warn_when_out_of_labels(&data.w_)) {
      return;
    }
    QSizeF const label_mm(preset.label_width, preset.label_height);
    // All copies carry the same text, so render once and repeat the image.
    QGraphicsScene scene;
    build_signature_label_scene(
        &scene, text, preset.label_width, preset.label_height,
        preset.sig_arrangement);
    QImage const img = colman::render_label_scene(
        &scene, label_mm, preset.printer_dpi,
        colman::load_printer_calibration(preset.printer_cups_name),
        preset.rotate_180);
    QList<QImage> images;
    for(int i = 0; i < count; ++i) {
      images.append(img);
    }
    QString err;
    if(!colman::send_images_to_thermal_printer(
           images, preset.printer_cups_name, label_mm, preset.printer_dpi, err,
           preset.darkness)) {
      QMessageBox::warning(&data.w_, QApplication::tr("Print error"), err);
      return;
    }
    colman::warn_when_roll_runs_out_while_printing(
        &data.w_, images.size(), label_mm);
    return;
  }
  show_signature_batch_print_page(data, text, count);
}


/**
   Builds the page UI and wires up the Back button and item selection.
   @param data    central application state.
   @param on_back callback invoked when the Back button is clicked.
   @param parent  optional parent widget.
*/
colman::SignaturePrintPage::SignaturePrintPage(
    colman::AppData& data,
    std::function<void()> on_back,
    QWidget* parent)
    : QWidget(parent),
      data_(data),
      on_back_(std::move(on_back))
{
  QVBoxLayout* outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(0);

  // --- back button ---
  QPushButton* back_btn = new QPushButton(
      QApplication::tr("Back"), this);
  back_btn->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Fixed);
  outer->addWidget(back_btn);

  // --- heading row: page icon + title ---
  {
    QHBoxLayout* heading_hbox = new QHBoxLayout;
    heading_hbox->setContentsMargins(4, 4, 4, 4);
    QLabel* icon_lbl = new QLabel(this);
    QPixmap const pm(":/icons/owls/print_signatures.jpeg");
    if(!pm.isNull()) {
      icon_lbl->setPixmap(
          pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      icon_lbl->setFixedSize(52, 52);
      icon_lbl->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    }
    heading_hbox->addWidget(icon_lbl);
    auto* title_lbl = new colman::ScalingLabel(
        "<b>" + QApplication::tr("print signatures") + "</b>", this, 2);
    heading_hbox->addWidget(title_lbl, 1);
    outer->addLayout(heading_hbox);
  }

  // --- two-column content area ---
  QHBoxLayout* hbox = new QHBoxLayout;
  outer->addLayout(hbox, 1);

  // Left column: search field + item list
  QVBoxLayout* left_vbox = new QVBoxLayout;
  search_edit_ = new QLineEdit(this);
  search_edit_->setPlaceholderText(
      QApplication::tr("Filter items..."));
  search_edit_->setClearButtonEnabled(true);
  left_vbox->addWidget(search_edit_);

  item_model_ = new QStandardItemModel(0, 2, this);
  item_model_->setHorizontalHeaderLabels(
      {QApplication::tr("Name"), QApplication::tr("Type")});
  item_proxy_ = new QSortFilterProxyModel(this);
  item_proxy_->setSourceModel(item_model_);
  item_proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
  item_proxy_->setFilterKeyColumn(-1);
  item_table_ = new QTableView(this);
  item_table_->setModel(item_proxy_);
  item_table_->setSelectionBehavior(
      QAbstractItemView::SelectRows);
  item_table_->setSelectionMode(
      QAbstractItemView::SingleSelection);
  item_table_->setEditTriggers(
      QAbstractItemView::NoEditTriggers);
  item_table_->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  item_table_->verticalHeader()->setVisible(false);
  left_vbox->addWidget(item_table_, 1);
  item_count_label_ = new QLabel(this);
  left_vbox->addWidget(item_count_label_);
  hbox->addLayout(left_vbox, 1);

  // Right column: signature controls + label layout
  QVBoxLayout* right_vbox = new QVBoxLayout;
  hbox->addLayout(right_vbox, 1);

  // --- Upper signature group ---
  QGroupBox* top_group =
      new QGroupBox(QApplication::tr("Upper signature"), this);
  QHBoxLayout* top_hbox = new QHBoxLayout(top_group);
  top_combo_ = new QComboBox(top_group);
  top_combo_->setEditable(true);
  top_hbox->addWidget(top_combo_, 1);
  top_count_ = new QSpinBox(top_group);
  top_count_->setObjectName("signature_top_count_");
  top_count_->setRange(1, 99);
  top_count_->setValue(1);
  top_hbox->addWidget(top_count_);
  print_top_btn_ = new QPushButton(
      QApplication::tr("Print upper signature"), top_group);
  print_top_btn_->setObjectName("signature_print_top_");
  top_hbox->addWidget(print_top_btn_);
  right_vbox->addWidget(top_group);

  // --- Lower signature group ---
  QGroupBox* bottom_group =
      new QGroupBox(QApplication::tr("Lower signature"), this);
  QHBoxLayout* bottom_hbox = new QHBoxLayout(bottom_group);
  bottom_combo_ = new QComboBox(bottom_group);
  bottom_combo_->setEditable(true);
  bottom_hbox->addWidget(bottom_combo_, 1);
  bottom_count_ = new QSpinBox(bottom_group);
  bottom_count_->setRange(1, 99);
  bottom_count_->setValue(1);
  bottom_hbox->addWidget(bottom_count_);
  print_bottom_btn_ = new QPushButton(
      QApplication::tr("Print lower signature"), bottom_group);
  bottom_hbox->addWidget(print_bottom_btn_);
  right_vbox->addWidget(bottom_group);

  right_vbox->addStretch(1);

  // --- signal connections ---
  connect(back_btn, &QPushButton::clicked,
          [this]() { on_back_(); });

  connect(search_edit_, &QLineEdit::textChanged,
          [this](QString const& text) {
            item_proxy_->setFilterFixedString(text);
            item_count_label_->setText(
                QApplication::tr("%n item(s) shown", "",
                                 item_proxy_->rowCount()));
          });

  connect(item_table_->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          [this](QItemSelection const&, QItemSelection const&) {
            on_item_selected();
          });

  connect(print_top_btn_, &QPushButton::clicked,
          [this]() {
            do_print_signatures(
                top_combo_->currentText(),
                top_count_->value());
          });

  connect(print_bottom_btn_, &QPushButton::clicked,
          [this]() {
            do_print_signatures(
                bottom_combo_->currentText(),
                bottom_count_->value());
          });
}

/**
   Refreshes the item list query and repopulates the signature
   category combo boxes from the database.  The label layout itself is
   chosen on the second page that opens when a print button is clicked.
*/
void colman::SignaturePrintPage::enter()
{
  populate_item_list();

  populate_top_combo();
  populate_bottom_combo();
}

/**
   Called when the item table selection changes.  Reads the selected
   item's @c signature_top and @c signature_bottom fields from the
   database and copies them into the corresponding combo boxes.
   Does nothing when nothing is selected or when the item cannot be
   loaded.
*/
void colman::SignaturePrintPage::on_item_selected()
{
  QModelIndexList const sel =
      item_table_->selectionModel()->selectedRows();
  if(sel.isEmpty()) {
    return;
  }
  QModelIndex const src = item_proxy_->mapToSource(sel.first());
  QStandardItem const* si = item_model_->item(src.row(), 0);
  if(!si) {
    return;
  }
  int const item_id = si->data(Qt::UserRole).toInt();
  if(item_id <= 0) {
    return;
  }
  colman::Item item;
  QString type_name;
  if(!data_.queries_->get_item(item_id, item, type_name)) {
    return;
  }
  top_combo_->setEditText(item.get_signature_top());
  bottom_combo_->setEditText(item.get_signature_bottom());
}

/**
   Opens the second page on which the operator chooses the label layout
   and (for sheet printers) the label positions, then prints.

   The layout choice is restricted to user-defined, named layouts; the
   default selection is the layout last saved in the signature label
   editor.  Nothing is printed from this first page directly.
   @param text  text to print on each label.
   @param count number of labels requested.
*/
void colman::SignaturePrintPage::do_print_signatures(
    QString const& text, int count)
{
  if(text.trimmed().isEmpty() || count <= 0) {
    return;
  }

  show_signature_batch_print_page(data_, text, count);
}

/**
   Repopulates the item list from the database.  The previously selected
   item is reselected if still present.
*/
void colman::SignaturePrintPage::populate_item_list()
{
  // Remember the currently selected item_id via proxy index.
  int selected_id = -1;
  {
    QModelIndexList const sel =
        item_table_->selectionModel()->selectedRows();
    if(!sel.isEmpty()) {
      QModelIndex const src = item_proxy_->mapToSource(sel.first());
      QStandardItem const* si = item_model_->item(src.row(), 0);
      if(si) {
        selected_id = si->data(Qt::UserRole).toInt();
      }
    }
  }

  item_model_->removeRows(0, item_model_->rowCount());
  item_table_->verticalHeader()->setDefaultSectionSize(
      item_table_->fontMetrics().height() + 4);

  auto const& itts = data_.queries_->get_items_with_type_and_thumbnails();
  int restore_src_row = -1;
  int row = 0;
  for(auto const& itt : itts) {
    int const id = itt.item_id_;
    auto* name_item = new QStandardItem(itt.item_name_);
    name_item->setData(id, Qt::UserRole);
    name_item->setEditable(false);

    QString const type_locale =
        data_.queries_->get_type_locale(itt.type_name_);
    auto* type_item = new QStandardItem(
        type_locale.isEmpty() ? itt.type_name_ : type_locale);
    type_item->setEditable(false);

    item_model_->appendRow({name_item, type_item});

    if(id == selected_id) {
      restore_src_row = row;
    }
    ++row;
  }

  if(restore_src_row >= 0) {
    QModelIndex const proxy_idx =
        item_proxy_->mapFromSource(item_model_->index(restore_src_row, 0));
    if(proxy_idx.isValid()) {
      item_table_->selectRow(proxy_idx.row());
      item_table_->scrollTo(proxy_idx);
    }
  }

  item_count_label_->setText(
      QApplication::tr("%n item(s) shown", "", item_proxy_->rowCount()));
}


/**
   Repopulates @c top_combo_ with the names read from the
   @c signature_top_categories table.  The previously displayed text
   is preserved so that a free-text entry is not lost.
*/
void colman::SignaturePrintPage::populate_top_combo()
{
  QString const current = top_combo_->currentText();
  top_combo_->clear();
  for(QString const& name :
      data_.queries_->read_signature_top_categories())
  {
    top_combo_->addItem(name);
  }
  int const idx = top_combo_->findText(current);
  if(idx >= 0) {
    top_combo_->setCurrentIndex(idx);
  }
  else if(!current.isEmpty()) {
    top_combo_->setEditText(current);
  }
}

/**
   Repopulates @c bottom_combo_ with all distinct names from the
   @c signature_bottom_categories table, regardless of the associated
   top category.  The previously displayed text is preserved.
*/
void colman::SignaturePrintPage::populate_bottom_combo()
{
  QString const current = bottom_combo_->currentText();
  bottom_combo_->clear();
  auto const& cats = data_.queries_->read_signature_bottom_categories("");
  for(auto const& cat : cats) {
    bottom_combo_->addItem(cat);
  }
  int const idx = bottom_combo_->findText(current);
  if(idx >= 0) {
    bottom_combo_->setCurrentIndex(idx);
  }
  else if(!current.isEmpty()) {
    bottom_combo_->setEditText(current);
  }
}

/* EOF */
