// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// person barcode setup
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person_barcode_page.hpp"
#ifndef COLMAN_LABEL_SELECTION_WIDGET_HPP
#  include "colman_label_selection_widget.hpp"
#endif
#ifndef COLMAN_CODE128_ITEM_HPP
#  include "colman_code128_item.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_CARD_RENDERING_HPP
#  include "colman_card_rendering.hpp"
#endif
#ifndef COLMAN_CARD_BATCH_PRINT_HPP
#  include "colman_card_batch_print.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QGRAPHICSSCENE
#  include <QGraphicsScene>
#  define SYSTEM_QGRAPHICSSCENE
#endif
#ifndef SYSTEM_QGRAPHICSRECTITEM
#  include <QGraphicsRectItem>
#  define SYSTEM_QGRAPHICSRECTITEM
#endif
#ifndef SYSTEM_QGRAPHICSPIXMAPITEM
#  include <QGraphicsPixmapItem>
#  define SYSTEM_QGRAPHICSPIXMAPITEM
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef COLMAN_CLASS_NAMES_HPP
#  include "colman_class_names.hpp"
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QTRANSFORM
#  include <QTransform>
#  define SYSTEM_QTRANSFORM
#endif
#ifndef SYSTEM_QTREEWIDGET
#  include <QTreeWidget>
#  define SYSTEM_QTREEWIDGET
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSVGRENDERER
#  include <QSvgRenderer>
#  define SYSTEM_QSVGRENDERER
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{

// ---- card rendering helpers -----------------------------------------------

/// Positions and scales a text item within the given width budget.
/// If the text is too wide it is scaled down to fit inside @p max_width.
/**
   @param item       text item to configure (may be null, call is a no-op).
   @param text       string to display.
   @param font_size  pixel font size.
   @param bold       whether to use a bold font.
   @param x0         horizontal reference point (meaning depends on @p alignment).
   @param max_width  maximum allowed width in scene units.
   @param above      item to position below (null means @p ydist from top).
   @param ydist      vertical gap below @p above (or from top when @p above is null).
   @param alignment  Qt alignment flags; AlignHCenter centres around @p x0.
*/
  void set_text(QGraphicsTextItem* item,
                QString const& text, qreal font_size, bool bold,
                qreal x0, qreal max_width,
                QGraphicsItem const* above, qreal ydist,
                int alignment = Qt::AlignHCenter)
  {
    if((item) && max_width > 0) {
      QFont font("Arial");
      font.setPixelSize(font_size);
      font.setBold(bold);
      item->setPlainText(text);
      item->setFont(font);
      qreal const y = (above
                           ? above->pos().y() + above->boundingRect().height() + ydist
                           : ydist);
      qreal w = item->boundingRect().width();
      if(w + 20 > max_width) {
        qreal const scale = (max_width - 20) / w;
        item->setScale(scale);
        w *= scale;
      }
      qreal x = x0;
      if(alignment & Qt::AlignHCenter) {
        x = x0 - w / 2;
      }
      else if(alignment & Qt::AlignRight) {
        x = x0 - w;
      }
      item->setPos(x, y);
    }
  }


/**
   Creates a movable, selectable, editable text item and adds it to @p scene.
   @param scene scene to add the item to.
   @return pointer to the newly created text item.
*/
  QGraphicsTextItem* create_text(QGraphicsScene& scene)
  {
    QGraphicsTextItem* item = new QGraphicsTextItem;
    scene.addItem(item);
    item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsMovable);
    item->setTextInteractionFlags(Qt::TextEditorInteraction);
    return item;
  }


/// Builds the person display name from title, last name and first name.
/**
   @param person the person to read from.
   @return formatted name string.
*/
  QString build_person_name(colman::Person const& person)
  {
    QString name = person.get_title();
    if(!name.isEmpty()) {
      name += ' ';
    }
    if(!person.get_lastname().isEmpty()) {
      name += person.get_lastname();
      if(!person.get_firstname().isEmpty()) {
        name += ", ";
      }
    }
    name += person.get_firstname();
    return name;
  }


/// Renders a person ID card using the saved IdCardLayout.
/**
   Positions barcode, text, and logo zones according to @p layout,
   scaling from the saved card dimensions to @p card_w × @p card_h.
   @param data        central application state.
   @param scene       graphics scene to render into.
   @param pid         person ID.
   @param x0          left edge of card in scene units.
   @param y0          top edge of card in scene units.
   @param card_w      card width in scene units (1 unit = 0.1 mm).
   @param card_h      card height in scene units.
   @param draw_border draws a thin border when true.
   @param col         collection data (logo, name, layout).
   @return true on success.
*/
  bool render_card_with_layout(
      colman::AppData& data, QGraphicsScene* scene,
      int pid, qreal x0, qreal y0,
      qreal card_w, qreal card_h, bool draw_border,
      colman::Collection const& col,
      colman::IdCardLayout const& arrangement)
  {
    colman::Person person;
    if(pid < 0 || !data.queries_->get_person(pid, person)) {
      return false;
    }
    // Use the selected layout's arrangement; fall back to a default
    // placement when it carries none (e.g. a geometry-only layout).
    colman::IdCardLayout const layout =
        arrangement.card_width_mm_ > 0.0
            ? arrangement
            : colman::default_card_arrangement(
                  card_w / 10.0, card_h / 10.0,
                  !col.get_logo_raw().isEmpty(), 0.55);
    if(layout.card_width_mm_ <= 0.0 || layout.card_height_mm_ <= 0.0) {
      return false;
    }
    qreal const sx = card_w / layout.card_width_mm_;
    qreal const sy = card_h / layout.card_height_mm_;

    auto elem_rect = [&](colman::IdCardElement const& e) -> QRectF {
      return QRectF(x0 + e.x_mm_ * sx, y0 + e.y_mm_ * sy,
                    e.width_mm_ * sx, e.height_mm_ * sy);
    };

    if(draw_border) {
      QGraphicsRectItem* r = new QGraphicsRectItem(0, 0, card_w, card_h);
      r->setPos(x0, y0);
      r->setPen(QPen(Qt::gray, 0));
      scene->addItem(r);
    }

  // -- Barcode zone --
    {
      QRectF const bc = elem_rect(layout.barcode_);
      colman::Code128Item* item = new colman::Code128Item;
      scene->addItem(item);
      item->set_text(data.queries_->compute_person_barcode(pid));
      item->adapt_size(bc.width() - 4, bc.height() - 4,
                       colman::barcode_unit_scene_units());
      QRectF const br = item->boundingRect();
      item->setPos(bc.x() + (bc.width() - br.width()) / 2,
                   bc.y() + (bc.height() - br.height()) / 2);
    }

  // -- Text zone --
    {
      QRectF const tz = elem_rect(layout.text_);
      qreal const lh = tz.height() / 4.0;
      int const px_small = std::max(1, (int)(lh * 0.55));
      int const px_large = std::max(1, (int)(lh * 0.75));
      QString const date_str =
          QApplication::tr("issued on: ") + QDate::currentDate().toString(QApplication::tr("yyyy-MM-dd"));
      QString const lines[4] = {col.get_name(), col.get_organization(),
                                date_str, build_person_name(person)};
      bool const bold_arr[4] = {false, false, false, true};
      int const px_arr[4] = {px_small, px_small, px_small, px_large};
      for(int i = 0; i < 4; ++i) {
        QFont f("Arial");
        f.setPixelSize(px_arr[i]);
        f.setBold(bold_arr[i]);
        QGraphicsTextItem* ti = new QGraphicsTextItem(lines[i]);
        ti->setFont(f);
        ti->setDefaultTextColor(Qt::black);
        scene->addItem(ti);
        qreal tw = ti->boundingRect().width();
        qreal const th = ti->boundingRect().height();
        if(tw > tz.width()) {
          qreal const s = tz.width() / tw;
          ti->setScale(s);
          tw = tz.width();
        }
        ti->setPos(tz.x() + (tz.width() - tw) / 2,
                   tz.y() + (i + 0.5) * lh - th / 2);
      }
    }

  // -- Logo zone --
    if(layout.has_logo_) {
      QRectF const lz = elem_rect(layout.logo_);
      if(col.has_svg_logo()) {
        scene->addItem(new colman::SvgDirectItem(col.get_logo_raw(), lz,
                                                 layout.logo_.orientation_));
      }
      else {
        QPixmap logo;
        logo.loadFromData(col.get_logo_raw());
        if(!logo.isNull()) {
          if(layout.logo_.orientation_ != 0) {
            QTransform t;
            t.rotate(layout.logo_.orientation_);
            logo = logo.transformed(t, Qt::SmoothTransformation);
          }
          QGraphicsPixmapItem* pi = scene->addPixmap(logo);
          if(logo.width() > 0 && logo.height() > 0) {
            pi->setTransform(QTransform::fromScale(
                lz.width() / logo.width(),
                lz.height() / logo.height()));
          }
          pi->setPos(lz.topLeft());
        }
      }
    }
    return true;
  }


/**
   Renders a single person ID card into @p scene at position (@p x0, @p y0).
   The card is laid out within a rectangle of @p card_w × @p card_h scene
   units.  Draws the collection logo, name, organization, person's full name,
   date, and a Code128 barcode.
   @param data   central application state.
   @param scene  graphics scene to render into.
   @param pid    person ID; must be >= 0 and must exist in the database.
   @param x0     left edge of the card in scene units.
   @param y0     top edge of the card in scene units.
   @param card_w      card width in scene units (1 unit = 0.1 mm).
   @param card_h      card height in scene units (1 unit = 0.1 mm).
   @param draw_border if true, draws a thin border rectangle around the card.
   @return true on success; false if the person could not be loaded.
*/
  bool set_barcode_data(colman::AppData& data, QGraphicsScene* scene,
                        int pid, qreal x0, qreal y0,
                        qreal card_w, qreal card_h, bool draw_border,
                        colman::IdCardLayout const& arrangement)
  {
    if(arrangement.card_width_mm_ > 0.0) {
      colman::Collection const collection = data.queries_->read_collection();
      return render_card_with_layout(data, scene, pid, x0, y0, card_w, card_h,
                                     draw_border, collection, arrangement);
    }

    // Legacy fallback: no element arrangement configured for the layout.
    colman::Collection const collection = data.queries_->read_collection();
    colman::Person person;
    if(pid < 0 || !data.queries_->get_person(pid, person)) {
      return false;
    }
    QString name(build_person_name(person));
    QString date(QApplication::tr("issued on: "));
    date += QDate::currentDate().toString(QApplication::tr("yyyy-MM-dd"));

    QGraphicsTextItem* org_item = create_text(*scene);
    QGraphicsTextItem* col_item = create_text(*scene);
    QGraphicsTextItem* name_item = create_text(*scene);
    QGraphicsTextItem* date_item = create_text(*scene);
    colman::Code128Item* barcode = new colman::Code128Item;
    scene->addItem(barcode);
    barcode->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    if(draw_border) {
      QGraphicsRectItem* rect = new QGraphicsRectItem(1, 1, card_w - 1, card_h - 1);
      scene->addItem(rect);
      rect->setPos(x0, y0);
      rect->setPen(QPen(Qt::gray, 0));
    }

    qreal max_width = card_w;
    qreal center = x0 + card_w / 2;
    qreal iw = 0;
    if(collection.has_svg_logo()) {
      QSvgRenderer tmp(collection.get_logo_raw());
      QSize const ns = tmp.defaultSize();
      if(ns.width() > 0 && ns.height() > 0) {
        qreal const nat_h = (qreal)ns.height();
        qreal const nat_w = (qreal)ns.width();
        qreal const scale = std::min(card_h / nat_h, card_w / nat_w / 4.0);
        iw = nat_w * scale;
        center += iw / 2;
        max_width -= iw;
        QRectF const svg_rect(x0,
                              y0 + (card_h - scale * nat_h) / 2.0,
                              iw, scale * nat_h);
        scene->addItem(
            new colman::SvgDirectItem(collection.get_logo_raw(), svg_rect));
      }
    }
    else {
      QPixmap logo;
      logo.loadFromData(collection.get_logo_raw());
      if(!logo.isNull()) {
        auto* pm_item = new QGraphicsPixmapItem;
        pm_item->setPixmap(logo);
        scene->addItem(pm_item);
        QRectF const br = pm_item->boundingRect();
        qreal const ih = br.height();
        iw = br.width();
        qreal const scale = std::min(card_h / ih, card_w / iw / 4.0);
        iw *= scale;
        center += iw / 2;
        max_width -= iw;
        pm_item->setScale(scale);
        pm_item->setPos(x0, y0 + (card_h - scale * ih) / 2.0);
      }
    }
    set_text(col_item, collection.get_name(), 40, false, center, max_width,
             nullptr, y0 + 30);
    set_text(org_item, collection.get_organization(), 40, false, center,
             max_width, col_item, 10);
    set_text(name_item, name, 50, true, center, max_width, org_item, 25);
    set_text(date_item, date, 30, false, center, max_width, name_item, 25);
    barcode->set_text(data.queries_->compute_person_barcode(pid));
    barcode->adapt_size(max_width - 4, 150, colman::barcode_unit_scene_units());
    barcode->setPos(x0 + iw + 2,
                    date_item->pos().y() + date_item->boundingRect().height() + 30);
    return true;
  }


// ---- person tree helpers --------------------------------------------------

/// Populates the tree with all persons from the database.
/**
   @param data        central application state.
   @param checked_ids person IDs to pre-check.
*/
  void populate_person_tree(colman::AppData& data,
                            QSet<int> const& checked_ids = QSet<int>())
  {
    QTreeWidget* tree = data.mw_.person_barcode_tree_;
    colman::AutoBlockSignals b(tree);
    tree->setSortingEnabled(false);
    tree->clear();

  // Optionally restrict the listing to the most recently created persons.
    bool const restrict_recent =
        data.mw_.person_barcode_recent_check_->isChecked();
    QSet<int> recent_ids;
    if(restrict_recent) {
      int const n = data.mw_.person_barcode_recent_count_->value();
      auto const recent = data.queries_->get_recently_created_person_ids(n);
      for(int const id : recent) {
        recent_ids.insert(id);
      }
    }

    auto const& persons = data.queries_->query_persons(0, "");
    for(auto const& person : persons) {
      int const id = person.get_id();
      if(restrict_recent && !recent_ids.contains(id)) {
        continue;
      }
      auto const& lastname = person.get_lastname();
      auto const& firstname = person.get_firstname();

      auto* ti = new QTreeWidgetItem;
      ti->setFlags(ti->flags() | Qt::ItemIsUserCheckable);
      ti->setCheckState(0,
                        checked_ids.contains(id) ? Qt::Checked : Qt::Unchecked);
      ti->setText(0, lastname);
      ti->setText(1, firstname);
      ti->setText(2, person.get_school_class());
      ti->setData(0, Qt::UserRole, id);
      tree->addTopLevelItem(ti);
    }
    tree->setSortingEnabled(true);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  }


/// Fills the class filter combo box, keeping the current choice if possible.
/**
   Offers "all classes" (invalid data), "no class" (empty string) and every
   class of the class list or of a person.
   @param data central application state.
*/
  void populate_class_combo(colman::AppData& data)
  {
    QComboBox* combo = data.mw_.person_barcode_class_;
    auto const current = combo->currentData();
    QSignalBlocker blk(combo);
    combo->clear();
    combo->addItem(QApplication::tr("all classes"), QVariant());
    combo->addItem(QApplication::tr("no class"), QString(""));
    // Also offer classes used by persons but missing from the class list.
    auto const names = colman::merge_class_names(
        data.queries_->load_class_names(),
        data.queries_->query_persons(0, ""));
    for(auto const& name : names) {
      combo->addItem(name, name);
    }
    int index = 0;
    if(current.isValid()) {
      index = std::max(0, combo->findData(current));
    }
    combo->setCurrentIndex(index);
  }


/// Shows/hides tree items based on search text and class; unchecks hidden items.
/**
   @param data   central application state.
   @param search search text (case-insensitive, matched against last/first name).
*/
  void filter_person_tree(colman::AppData& data, QString const& search)
  {
    QString const s = search.trimmed().toLower();
    auto const cls = data.mw_.person_barcode_class_->currentData();
    QTreeWidget* tree = data.mw_.person_barcode_tree_;
    colman::AutoBlockSignals b(tree);
    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
      QTreeWidgetItem* ti = tree->topLevelItem(i);
      bool const name_match = s.isEmpty() || ti->text(0).toLower().contains(s) || ti->text(1).toLower().contains(s);
      bool const class_match = !cls.isValid() || ti->text(2) == cls.toString();
      bool const match = name_match && class_match;
      ti->setHidden(!match);
      if(!match) {
        ti->setCheckState(0, Qt::Unchecked);
      }
    }
  }


/// Returns IDs of all checked visible tree items, in display order.
  QList<int> checked_person_ids(colman::AppData& data)
  {
    QList<int> result;
    QTreeWidget* tree = data.mw_.person_barcode_tree_;
    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
      QTreeWidgetItem* ti = tree->topLevelItem(i);
      if(!ti->isHidden() && ti->checkState(0) == Qt::Checked) {
        result.append(ti->data(0, Qt::UserRole).toInt());
      }
    }
    return result;
  }


/// Updates the selection count label and enables/disables the print button.
  void update_pbc_status(colman::AppData& data)
  {
    int const n = checked_person_ids(data).size();
    data.mw_.person_card_print_->setEnabled(n > 0);
    if(n == 0) {
      data.mw_.person_barcode_selection_count_->setText(
          QApplication::tr("No persons selected"));
    }
    else {
      data.mw_.person_barcode_selection_count_->setText(
          QApplication::tr("%1 persons selected").arg(n));
    }
  }


/// Displays person fields in the detail widget.
/**
   @param data      central application state.
   @param person_id database id; negative clears the widget.
*/
  void show_person(colman::AppData& data, int person_id)
  {
    colman::Person person;
    if(person_id >= 0) {
      data.queries_->get_person(person_id, person);
    }
    data.mw_.person_barcode_person_->person_to_fields(person);
  }


// ---- print page helpers ---------------------------------------------------

/// Switches to the print page and prepares the label widget for @p ids.
  void start_person_label_print(colman::AppData& data,
                                std::shared_ptr<colman::CardBatchState> const& st,
                                QList<int> const& ids)
  {
    st->remaining_ = ids;
    st->printed_.clear();
    st->print_started_ = false;
    st->printer_.reset();
    st->painter_.reset();
    st->first_page_ = true;

    if(st->label_sel_) {
      st->label_sel_->refresh_layouts();
      st->label_sel_->set_max_count(ids.size());
      st->label_sel_->clear_selection();
      QString const name =
          colman::LabelLayoutWidget::last_selected_layout_name();
      if(!name.isEmpty()) {
        st->label_sel_->set_initial_preset_by_name(name);
      }
    }

    data.mw_.person_barcode_stack_->setCurrentIndex(1);
  }


/// Leaves the person barcode page.
/**
   @param data central application state.
*/
  void leave_person_barcode_page(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
  }


/// Switches to the person barcode page and resets all views.
/**
   @param data central application state.
*/
  void enter_person_barcode_page(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.person_barcode_page_);
    data.mw_.person_barcode_stack_->setCurrentIndex(0);
    data.mw_.person_barcode_person_->set_class_names(
        data.queries_->load_class_names());
    populate_class_combo(data);
    populate_person_tree(data);
    filter_person_tree(data, data.mw_.person_barcode_search_->text());
    colman::Person empty;
    data.mw_.person_barcode_person_->person_to_fields(empty);
    data.mw_.person_barcode_search_->setFocus();
    update_pbc_status(data);
  }

} // namespace


/**
   Refreshes the person barcode page.
   @param data central application state.
*/
void colman::refresh_person_barcode_page(colman::AppData& data)
{
  QSet<int> checked;
  QTreeWidget* tree = data.mw_.person_barcode_tree_;
  for(int i = 0; i < tree->topLevelItemCount(); ++i) {
    QTreeWidgetItem* ti = tree->topLevelItem(i);
    if(ti->checkState(0) == Qt::Checked) {
      checked.insert(ti->data(0, Qt::UserRole).toInt());
    }
  }
  populate_person_tree(data, checked);
  filter_person_tree(data, data.mw_.person_barcode_search_->text());
  update_pbc_status(data);
}


/**
   Initializes the person barcode page; to be called once at program start.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_person_barcode_page(QApplication& app, colman::AppData& data)
{
  // Add page icon to the title row (vbox[1] is the heading HBox)
  {
    auto* vbox = qobject_cast<QVBoxLayout*>(
        data.mw_.person_barcode_page_->layout());
    auto* hbox = qobject_cast<QHBoxLayout*>(vbox->itemAt(1)->layout());
    QLabel* icon_lbl = new QLabel(data.mw_.person_barcode_page_);
    QPixmap const pm(":/icons/owls/print_id.jpeg");
    if(!pm.isNull()) {
      icon_lbl->setPixmap(
          pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      icon_lbl->setFixedSize(52, 52);
      icon_lbl->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    }
    hbox->insertWidget(0, icon_lbl);
  }

  data.mw_.person_barcode_person_->set_read_only(true);

  auto st = std::make_shared<colman::CardBatchState>();

  // ---- Build the print page (stack index 1) entirely in code ----
  {
    auto* page = data.mw_.person_barcode_print_page_;
    auto* vbox = new QVBoxLayout(page);

    auto* label_sel = new colman::LabelSelectionWidget(1, page);
    // Operators may only pick administrator-defined, named layouts here;
    // the factory presets and the layout-edit button are hidden, and only
    // layouts created for ID-card printing are offered.
    label_sel->set_user_layouts_only(true);
    label_sel->set_layout_purpose(colman::LabelLayoutPurpose::IdCard);
    st->label_sel_ = label_sel;
    vbox->addWidget(label_sel, 1);

    auto* btn_box = new QHBoxLayout;
    auto* cancel_btn = new QPushButton(QApplication::tr("Cancel"), page);
    auto* print_btn = new QPushButton(QApplication::tr("Print"), page);
    btn_box->addWidget(cancel_btn);
    btn_box->addStretch();
    btn_box->addWidget(print_btn);
    vbox->addLayout(btn_box);

    colman::CardRenderFn render_fn =
        [&data](QGraphicsScene* sc, int pid, qreal sw, qreal sh,
                bool border, colman::IdCardLayout const& arrangement) -> bool {
      return set_barcode_data(data, sc, pid, 0, 0, sw, sh, border, arrangement);
    };

    // layout_changed fires after LabelSelectionWidget rebuilds its internal
    // scene, which deletes all items including our overlays. Just clear the
    // pointer list and repopulate; do NOT call batch_remove_previews here.
    QObject::connect(label_sel,
                     &colman::LabelSelectionWidget::layout_changed,
                     [st, render_fn]() {
                       QString const name =
                           st->label_sel_->current_layout().name;
                       if(!name.isEmpty()) {
                         colman::LabelLayoutWidget::
                             set_last_selected_layout_name(name);
                       }
                       st->preview_items_.clear();
                       colman::batch_add_previews(st, render_fn);
                     });
    QObject::connect(label_sel,
                     &colman::LabelSelectionWidget::selection_changed,
                     [st, render_fn]() {
                       colman::batch_remove_previews(st);
                       colman::batch_add_previews(st, render_fn);
                     });

    QObject::connect(cancel_btn, &QPushButton::clicked,
                     [&data, st]() {
                       colman::batch_abort(st, data.mw_.person_barcode_tree_);
                       update_pbc_status(data);
                       data.mw_.person_barcode_stack_->setCurrentIndex(0);
                     });

    QObject::connect(print_btn, &QPushButton::clicked,
                     [&data, st, render_fn]() {
                       colman::batch_do_print(
                           st, render_fn,
                           data.mw_.person_barcode_show_border_->isChecked(),
                           data.mw_.person_barcode_page_,
                           data.mw_.person_barcode_stack_,
                           data.mw_.person_barcode_tree_,
                           [&data]() { update_pbc_status(data); },
                           [](QString const& name) {
                             colman::LabelLayoutWidget::
                                 set_last_selected_layout_name(name);
                           });
                     });
  }

  // ---- Navigation + toolbar connections ----

  app.connect(data.print_id_button_, &QToolButton::clicked,
              [&data]() {
                enter_person_barcode_page(data);
              });

  app.connect(data.mw_.person_barcode_back_, &QPushButton::clicked,
              [&data, st]() {
                if(data.mw_.person_barcode_stack_->currentIndex() == 1) {
                  colman::batch_abort(st, data.mw_.person_barcode_tree_);
                  update_pbc_status(data);
                  data.mw_.person_barcode_stack_->setCurrentIndex(0);
                }
                else {
                  leave_person_barcode_page(data);
                }
              });

  app.connect(data.mw_.person_barcode_search_, &QLineEdit::textChanged,
              [&data](QString const& text) {
                filter_person_tree(data, text);
                update_pbc_status(data);
              });

  app.connect(data.mw_.person_barcode_class_,
              QOverload<int>::of(&QComboBox::currentIndexChanged),
              [&data](int) {
                filter_person_tree(data, data.mw_.person_barcode_search_->text());
                update_pbc_status(data);
              });

  // "Only the last N created persons": toggle enables the spin box and
  // rebuilds the tree; changing N rebuilds only while the restriction is active.
  app.connect(data.mw_.person_barcode_recent_check_, &QCheckBox::toggled,
              [&data](bool on) {
                data.mw_.person_barcode_recent_count_->setEnabled(on);
                refresh_person_barcode_page(data);
              });

  app.connect(data.mw_.person_barcode_recent_count_,
              QOverload<int>::of(&QSpinBox::valueChanged),
              [&data](int) {
                if(data.mw_.person_barcode_recent_check_->isChecked()) {
                  refresh_person_barcode_page(data);
                }
              });

  app.connect(data.mw_.person_barcode_select_all_, &QPushButton::clicked,
              [&data]() {
                QTreeWidget* tree = data.mw_.person_barcode_tree_;
                colman::AutoBlockSignals b(tree);
                for(int i = 0; i < tree->topLevelItemCount(); ++i) {
                  QTreeWidgetItem* ti = tree->topLevelItem(i);
                  if(!ti->isHidden()) {
                    ti->setCheckState(0, Qt::Checked);
                  }
                }
                update_pbc_status(data);
              });

  app.connect(data.mw_.person_barcode_deselect_all_, &QPushButton::clicked,
              [&data]() {
                QTreeWidget* tree = data.mw_.person_barcode_tree_;
                colman::AutoBlockSignals b(tree);
                for(int i = 0; i < tree->topLevelItemCount(); ++i) {
                  QTreeWidgetItem* ti = tree->topLevelItem(i);
                  if(!ti->isHidden()) {
                    ti->setCheckState(0, Qt::Unchecked);
                  }
                }
                update_pbc_status(data);
              });

  app.connect(data.mw_.person_barcode_tree_,
              &QTreeWidget::currentItemChanged,
              [&data](QTreeWidgetItem* cur, QTreeWidgetItem*) {
                show_person(data,
                            cur ? cur->data(0, Qt::UserRole).toInt() : -1);
              });

  app.connect(data.mw_.person_barcode_tree_,
              &QTreeWidget::itemChanged,
              [&data](QTreeWidgetItem*, int) {
                update_pbc_status(data);
              });

  app.connect(data.mw_.person_card_print_, &QPushButton::clicked,
              [&data, st]() {
                QList<int> ids = checked_person_ids(data);
                if(!ids.isEmpty()) {
                  start_person_label_print(data, st, ids);
                }
              });
}

/* EOF */
