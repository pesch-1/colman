// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// setup of item barcode page
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item_barcode_page.hpp"
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef COLMAN_CODE128_ITEM_HPP
#  include "colman_code128_item.hpp"
#endif
#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  include "colman_signal_helpers.hpp"
#endif
#ifndef COLMAN_LABEL_SELECTION_WIDGET_HPP
#  include "colman_label_selection_widget.hpp"
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
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QGRAPHICSTEXTITEM
#  include <QGraphicsTextItem>
#  define SYSTEM_QGRAPHICSTEXTITEM
#endif
#ifndef SYSTEM_QGRAPHICSPIXMAPITEM
#  include <QGraphicsPixmapItem>
#  define SYSTEM_QGRAPHICSPIXMAPITEM
#endif
#ifndef SYSTEM_QTRANSFORM
#  include <QTransform>
#  define SYSTEM_QTRANSFORM
#endif
#ifndef SYSTEM_QTREEWIDGET
#  include <QTreeWidget>
#  define SYSTEM_QTREEWIDGET
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QSIGNALBLOCKER
#  include <QSignalBlocker>
#  define SYSTEM_QSIGNALBLOCKER
#endif
#ifndef SYSTEM_QDATEEDIT
#  include <QDateEdit>
#  define SYSTEM_QDATEEDIT
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_UTILITY
#  include <utility>
#  define SYSTEM_UTILITY
#endif
#ifndef SYSTEM_QFONTMETRICS
#  include <QFontMetrics>
#  define SYSTEM_QFONTMETRICS
#endif

namespace
{

// ---- card rendering helpers -----------------------------------------------

/// Looks up a common author/artist key in the item's type-specific values.
/** Checks both repeatable list values and plain string values. */
  QString item_author(colman::Item const& item)
  {
    colman::StringListMap const& lvals = item.get_list_values();
    colman::StringMap const& vals = item.get_values();
    for(char const* key : {"authors", "author", "Autoren", "Autor", "Author",
                           "Verfasser", "artist", "director"}) {
      QString const k(key);
      auto lit = lvals.find(k);
      if(lit != lvals.end() && !lit->second.isEmpty()) {
        return lit->second.join(", ");
      }
      auto it = vals.find(k);
      if(it != vals.end() && !it->second.isEmpty()) {
        return it->second;
      }
    }
    return {};
  }


/// Returns the ISBN13, or ISBN10 as fallback, from an item's values.
  QString item_isbn(colman::Item const& item)
  {
    colman::StringMap const& vals = item.get_values();
    auto it13 = vals.find(QString("ISBN13"));
    if(it13 != vals.end() && !it13->second.isEmpty()) {
      return it13->second;
    }
    auto it10 = vals.find(QString("ISBN10"));
    if(it10 != vals.end() && !it10->second.isEmpty()) {
      return it10->second;
    }
    return {};
  }


/// Truncates @p text to fit within @p max_w (scene units) at @p fm.
/// Cuts at the last space before overflow and appends "...".
  QString truncate_to_width(QString const& text,
                            QFontMetricsF const& fm,
                            qreal max_w)
  {
    if(fm.horizontalAdvance(text) <= max_w) {
      return text;
    }
    QString t = text;
    while(!t.isEmpty()) {
      int const sp = t.lastIndexOf(' ');
      if(sp > 0) {
        t = t.left(sp);
        if(fm.horizontalAdvance(t + "...") <= max_w) {
          return t + "...";
        }
      }
      else {
        break;
      }
    }
    t = text;
    while(!t.isEmpty() && fm.horizontalAdvance(t + "...") > max_w) {
      t.chop(1);
    }
    return t + "...";
  }


/// Splits @p title into two lines so that both fit within @p max_w.
/// Finds the last space where the first part still fits, maximising the
/// first line and minimising truncation of the second (last) line.
/// Returns two strings, or one truncated string if no fitting split exists.
  QStringList split_title_text(QString const& title,
                               QFontMetricsF const& fm,
                               qreal max_w)
  {
  // Last space where the first part fits: l1 is as long as possible so
  // that it renders at the same font size as l2 (no scaling for either).
    int best_sp = -1;
    for(int i = 0; i < title.size(); ++i) {
      if(title[i] == ' ' && fm.horizontalAdvance(title.left(i)) <= max_w) {
        best_sp = i;
      }
    }
    if(best_sp < 0) {
      return {truncate_to_width(title, fm, max_w)};
    }
    QString const l1 = title.left(best_sp);
    QString l2 = title.mid(best_sp + 1);
    if(fm.horizontalAdvance(l2) > max_w) {
      l2 = truncate_to_width(l2, fm, max_w);
    }
    return {l1, l2};
  }


/**
   Renders a single item card into @p scene using the saved item card layout.
*/
  bool render_item_card_with_layout(
      colman::AppData& data,
      QGraphicsScene* scene,
      int item_id, qreal x0, qreal y0,
      qreal card_w, qreal card_h, bool draw_border,
      colman::Collection const& col,
      colman::IdCardLayout const& arrangement)
  {
    colman::Item item;
    QString type_str;
    if(item_id < 0 || !data.queries_->get_item(item_id, item, type_str)) {
      return false;
    }
    // Use the selected layout's arrangement; fall back to a default
    // placement when it carries none (e.g. a geometry-only layout).
    colman::IdCardLayout const layout =
        arrangement.card_width_mm_ > 0.0
            ? arrangement
            : colman::default_card_arrangement(
                  card_w / 10.0, card_h / 10.0,
                  !col.get_logo_raw().isEmpty(), 0.60);
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
      auto* r = new QGraphicsRectItem(0, 0, card_w, card_h);
      r->setPos(x0, y0);
      r->setPen(QPen(Qt::gray, 0));
      scene->addItem(r);
    }

  // -- Barcode zone --
    {
      QRectF const bc = elem_rect(layout.barcode_);
      int const orient = layout.barcode_.orientation_;
      bool const rotated = (orient % 180) != 0;
      qreal const nat_w = rotated ? bc.height() : bc.width();
      qreal const nat_h = rotated ? bc.width() : bc.height();

    // Invisible container carries the rotation so child items need not.
      auto* cont = new QGraphicsRectItem(0, 0, nat_w, nat_h);
      cont->setPen(Qt::NoPen);
      cont->setBrush(Qt::NoBrush);
      cont->setTransformOriginPoint(nat_w / 2, nat_h / 2);
      cont->setRotation(orient);
      cont->setPos(bc.center() - QPointF(nat_w / 2, nat_h / 2));
      scene->addItem(cont);

      QString barcode_text =
          data.queries_->compute_item_barcode(item.get_type_id(), item_id);
      if(barcode_text.isEmpty()) {
        barcode_text = colman::compute_item_barcode(
            col, item.get_type_id(), item_id);
      }
      auto* bi = new colman::Code128Item(cont);
      bi->set_text(barcode_text);
      bi->adapt_size(nat_w - 4, nat_h - 4, colman::barcode_unit_scene_units());
      QRectF const br = bi->boundingRect();
      bi->setPos((nat_w - br.width()) / 2 - br.x(),
                 (nat_h - br.height()) / 2 - br.y());
    }

  // -- Text zone --
    {
      QRectF const tz = elem_rect(layout.text_);
      int const orient_t = layout.text_.orientation_;
      bool const rotated_t = (orient_t % 180) != 0;
      qreal const nat_tw = rotated_t ? tz.height() : tz.width();
      qreal const nat_th = rotated_t ? tz.width() : tz.height();

    // Invisible container carries the rotation.
      auto* cont = new QGraphicsRectItem(0, 0, nat_tw, nat_th);
      cont->setPen(Qt::NoPen);
      cont->setBrush(Qt::NoBrush);
      cont->setTransformOriginPoint(nat_tw / 2, nat_th / 2);
      cont->setRotation(orient_t);
      cont->setPos(tz.center() - QPointF(nat_tw / 2, nat_th / 2));
      scene->addItem(cont);

      QStringList text_lines;
      QList<bool> italic_list;

    // Combine top and bottom spine signatures into a single line.
      QString sig = item.get_signature_top().trimmed();
      {
        QString const bottom = item.get_signature_bottom().trimmed();
        if(!bottom.isEmpty()) {
          if(!sig.isEmpty()) {
            sig += ' ';
          }
          sig += bottom;
        }
      }

      bool const is_book = (type_str == "book");
      if(is_book) {
        colman::StringMap const& vals = item.get_values();
        auto title_it = vals.find(QString("title"));
        QString const title =
            (title_it != vals.end() && !title_it->second.isEmpty())
                ? title_it->second
                : item.get_name();
        QString const isbn = item_isbn(item);
        text_lines << col.get_organization()
                   << title
                   << item_author(item);
        italic_list << false << true << false;
        if(!isbn.isEmpty()) {
          text_lines << ("ISBN: " + isbn);
          italic_list << false;
        }
      }
      else {
        text_lines << col.get_organization()
                   << item.get_name()
                   << item_author(item);
        italic_list << false << true << false;
      }

    // Append the signature line below the author for any item that has one.
      if(!sig.isEmpty()) {
        text_lines << sig;
        italic_list << false;
      }
      int n = text_lines.size();
      qreal lh = nat_th / n;
      int px = std::max(1, (int)(lh * 0.55));

    // 1 scene unit = 0.1 mm; 1 pt = 0.3528 mm → 1 pt ≈ 3.528 scene units
      qreal constexpr MinPtInScene = 8.0 * 3.528;
      int const title_idx = 1;
      int author_idx = 2;

    // If the title line would scale below 8 pt, split at a space.
      {
        QFont ft("Arial");
        ft.setPixelSize(px);
        ft.setItalic(true);
        QFontMetricsF const fm(ft);
        qreal const tw = fm.horizontalAdvance(text_lines[title_idx]);
        if(tw > nat_tw && px * (nat_tw / tw) < MinPtInScene) {
          QStringList const parts =
              split_title_text(text_lines[title_idx], fm, nat_tw);
          text_lines[title_idx] = parts[0];
          if(parts.size() == 2) {
            text_lines.insert(title_idx + 1, parts[1]);
            italic_list.insert(title_idx + 1, true);
            ++author_idx;
            n = text_lines.size();
            lh = nat_th / n;
            px = std::max(1, (int)(lh * 0.55));
          }
        }
      }

    // If the author line would scale below 8 pt, truncate at a space.
      if(author_idx < n) {
        QFont fa("Arial");
        fa.setPixelSize(px);
        QFontMetricsF const fm(fa);
        qreal const tw = fm.horizontalAdvance(text_lines[author_idx]);
        if(tw > nat_tw && px * (nat_tw / tw) < MinPtInScene) {
          text_lines[author_idx] =
              truncate_to_width(text_lines[author_idx], fm, nat_tw);
        }
      }

      for(int i = 0; i < n; ++i) {
        QFont f("Arial");
        f.setPixelSize(px);
        f.setItalic(italic_list[i]);
        auto* ti = new QGraphicsTextItem(text_lines[i], cont);
        ti->setFont(f);
        ti->setDefaultTextColor(Qt::black);
        qreal const tw = ti->boundingRect().width();
        qreal const th = ti->boundingRect().height();
        qreal x_pos = 0.0;
        if(tw > nat_tw) {
          ti->setScale(nat_tw / tw);
        }
        else {
          x_pos = (nat_tw - tw) / 2;
        }
        ti->setPos(x_pos, (i + 0.5) * lh - th / 2);
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
          auto* pi = scene->addPixmap(logo);
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


// ---- item tree helpers ----------------------------------------------------

/// Displays item fields in the detail widget.
/**
 * @param data    central application state.
 * @param item_id database id; negative clears the widget.
 */
  void show_item(colman::AppData& data, int item_id)
  {
    colman::Item item;
    QString type_str;
    if(item_id >= 0) {
      data.queries_->get_item(item_id, item, type_str);
    }
    data.mw_.item_barcode_item_widget_->item_to_fields(item, type_str);
  }


/// Populates the tree with all items from the database.
/**
 * @param data        central application state.
 * @param checked_ids item IDs to pre-check.
 */
  void populate_item_tree(colman::AppData& data,
                          QSet<int> const& checked_ids = QSet<int>())
  {
    QTreeWidget* tree = data.mw_.item_barcode_tree_;
    colman::AutoBlockSignals b(tree);
    tree->setSortingEnabled(false);
    tree->clear();

  // Optionally restrict the listing either to the most recently acquired
  // items or to items acquired within a time span. The two restrictions are
  // mutually exclusive (enforced by the check-box wiring).
    bool const restrict_recent = data.mw_.item_barcode_recent_check_->isChecked();
    bool const restrict_daterange =
        data.mw_.item_barcode_daterange_check_->isChecked();
    bool const restrict = restrict_recent || restrict_daterange;
    QSet<int> allowed_ids;
    if(restrict_recent) {
      int const n = data.mw_.item_barcode_recent_count_->value();
      auto const recent = data.queries_->get_recently_acquired_item_ids(n);
      for(int const id : recent) {
        allowed_ids.insert(id);
      }
    }
    else if(restrict_daterange) {
      QDateTime const from(data.mw_.item_barcode_date_from_->date(),
                           QTime(0, 0, 0));
      QDateTime const to(data.mw_.item_barcode_date_to_->date(),
                         QTime(23, 59, 59));
      auto const in_range = data.queries_->get_items_acquired_between(from, to);
      for(int const id : in_range) {
        allowed_ids.insert(id);
      }
    }

    auto const& items = data.queries_->get_items_ordered_by_name();
    for(auto const& item_id_name : items) {
      int const id = item_id_name.first;
      if(restrict && !allowed_ids.contains(id)) {
        continue;
      }
      QString const name = item_id_name.second;
      colman::Item item;
      QString type_str;
      data.queries_->get_item(id, item, type_str);
      QString const author = item_author(item);

      auto* ti = new QTreeWidgetItem;
      ti->setFlags(ti->flags() | Qt::ItemIsUserCheckable);
      ti->setCheckState(0,
                        checked_ids.contains(id) ? Qt::Checked : Qt::Unchecked);
      ti->setText(0, name);
      ti->setText(1, author);
      ti->setData(0, Qt::UserRole, id);
      ti->setData(0, Qt::UserRole + 1, item_isbn(item));
      tree->addTopLevelItem(ti);
    }
    tree->setSortingEnabled(true);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  }


/// Shows/hides tree items based on search text; unchecks hidden items.
/**
 * @param data   central application state.
 * @param search search text (case-insensitive, matched against name and author).
 */
  void filter_item_tree(colman::AppData& data, QString const& search)
  {
    QString const s = search.trimmed().toLower();
    QTreeWidget* tree = data.mw_.item_barcode_tree_;
    colman::AutoBlockSignals b(tree);
    int visible = 0;
    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
      QTreeWidgetItem* ti = tree->topLevelItem(i);
      bool const match = s.isEmpty() || ti->text(0).toLower().contains(s) ||
                         ti->text(1).toLower().contains(s) ||
                         ti->data(0, Qt::UserRole + 1)
                             .toString()
                             .toLower()
                             .contains(s);
      ti->setHidden(!match);
      if(!match) {
        ti->setCheckState(0, Qt::Unchecked);
      }
      else {
        ++visible;
      }
    }
    int const total = tree->topLevelItemCount();
    if(s.isEmpty()) {
      data.mw_.item_barcode_visible_count_->setText({});
    }
    else {
      data.mw_.item_barcode_visible_count_->setText(
          QApplication::tr("%n of %1 item(s) shown", "", visible)
              .arg(total));
    }
  }


/// Returns IDs of all checked visible tree items, in display order.
  QList<int> checked_item_ids(colman::AppData& data)
  {
    QList<int> result;
    QTreeWidget* tree = data.mw_.item_barcode_tree_;
    for(int i = 0; i < tree->topLevelItemCount(); ++i) {
      QTreeWidgetItem* ti = tree->topLevelItem(i);
      if(!ti->isHidden() && ti->checkState(0) == Qt::Checked) {
        result.append(ti->data(0, Qt::UserRole).toInt());
      }
    }
    return result;
  }


/// Updates the selection count label and enables/disables the print buttons.
  void update_ibc_status(colman::AppData& data)
  {
    int const n = checked_item_ids(data).size();
    data.mw_.item_card_print_->setEnabled(n > 0);
    if(n == 0) {
      data.mw_.item_barcode_selection_count_->setText(
          QApplication::tr("No items selected"));
    }
    else {
      data.mw_.item_barcode_selection_count_->setText(
          QApplication::tr("%1 items selected").arg(n));
    }
  }


// ---- print page helpers ---------------------------------------------------

/// Returns @p ids with each id repeated @p copies times (grouped per id).
  QList<int> expand_with_copies(QList<int> const& ids, int copies)
  {
    QList<int> out;
    out.reserve(ids.size() * copies);
    for(int id : ids) {
      for(int c = 0; c < copies; ++c) {
        out.append(id);
      }
    }
    return out;
  }


/// Switches to the print page and prepares the label widget for @p ids.
  void start_label_print(colman::AppData& data,
                         std::shared_ptr<colman::CardBatchState> const& st,
                         QList<int> const& ids)
  {
    if(colman::LabelSelectionWidget::has_no_user_layouts_for(
           colman::LabelLayoutPurpose::ItemLabel)) {
      QMessageBox::warning(data.mw_.item_barcode_page_,
                           QApplication::tr("No item card layout"),
                           QApplication::tr(
                               "No item card layout has been saved yet. "
                               "Please open the item card layout editor first."));
      return;
    }

    st->base_ids_ = ids;
    st->remaining_ = ids;   // one copy per item by default
    st->printed_.clear();
    st->print_started_ = false;
    st->printer_.reset();
    st->painter_.reset();
    st->first_page_ = true;

    // Reset the copies field to its default without triggering a rebuild;
    // remaining_ is already set to the one-copy list above.
    if(st->copies_spin_) {
      QSignalBlocker b(st->copies_spin_);
      st->copies_spin_->setValue(1);
    }

    if(st->label_sel_) {
      st->label_sel_->refresh_layouts();   // pick up any newly-saved presets
      st->label_sel_->set_max_count(ids.size());
      st->label_sel_->clear_selection();
      QString const name =
          colman::LabelLayoutWidget::last_selected_layout_name();
      if(!name.isEmpty()) {
        st->label_sel_->set_initial_preset_by_name(name);
      }
    }

    data.mw_.item_barcode_stack_->setCurrentIndex(1);
  }


/// Navigates away from the item barcode page.
/**
 * @param data central application state.
 */
  void leave_item_barcode_page(colman::AppData& data)
  {
    data.ibc_item_model_.set_rows({});
    data.invoke_ibc_back();
  }


/// Switches to the item barcode page and populates the item tree.
/**
 * @param data central application state.
 */
  void enter_item_barcode_page(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.item_barcode_page_);
    data.mw_.item_barcode_stack_->setCurrentIndex(0);
    populate_item_tree(data);
    filter_item_tree(data, data.mw_.item_barcode_search_->text());
    data.mw_.item_barcode_item_widget_->reset_item_type();
    data.mw_.item_barcode_search_->setFocus();
    update_ibc_status(data);
  }

} // namespace


/**
 * @param data central application state.
 */
void colman::refresh_item_barcode_page(colman::AppData& data)
{
  // Preserve existing check states across refresh
  QSet<int> checked;
  QTreeWidget* tree = data.mw_.item_barcode_tree_;
  for(int i = 0; i < tree->topLevelItemCount(); ++i) {
    QTreeWidgetItem* ti = tree->topLevelItem(i);
    if(ti->checkState(0) == Qt::Checked) {
      checked.insert(ti->data(0, Qt::UserRole).toInt());
    }
  }
  populate_item_tree(data, checked);
  filter_item_tree(data, data.mw_.item_barcode_search_->text());
  update_ibc_status(data);
}


/**
 * One-time setup of the item barcode page.
 * @param app  Qt application object.
 * @param data central application state.
 */
void colman::setup_item_barcode_page(QApplication& app, colman::AppData& data)
{
  // Logo + heading
  {
    QPixmap logo(":/icons/owls/print_barcode.jpeg");
    if(!logo.isNull()) {
      data.mw_.item_barcode_logo_->setPixmap(
          logo.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    auto* title_lbl = new colman::ScalingLabel(
        "<b>" + QApplication::tr("print labels") + "</b>",
        data.mw_.item_barcode_logo_->parentWidget(), 2);
    data.mw_.horizontalLayout_ibc_top->insertWidget(1, title_lbl);
  }

  // Item detail widget
  data.mw_.item_barcode_item_widget_->setup(data);
  data.mw_.item_barcode_item_widget_->set_read_only(true);

  // ---- Build the print page (stack index 1) entirely in code ----
  auto st = std::make_shared<colman::CardBatchState>();
  {
    auto* page = data.mw_.item_barcode_print_page_;
    auto* vbox = new QVBoxLayout(page);

    auto* label_sel = new colman::LabelSelectionWidget(1, page);
    // Operators may only pick administrator-defined, named layouts here;
    // the factory presets and the layout-edit button are hidden, and only
    // layouts created for item-label printing are offered.
    label_sel->set_user_layouts_only(true);
    label_sel->set_layout_purpose(colman::LabelLayoutPurpose::ItemLabel);
    st->label_sel_ = label_sel;
    vbox->addWidget(label_sel, 1);

    // Bottom: Cancel + copies + border checkbox + Print buttons
    auto* btn_box = new QHBoxLayout;
    auto* cancel_btn = new QPushButton(QApplication::tr("Cancel"), page);
    auto* copies_label = new QLabel(QApplication::tr("Copies:"), page);
    auto* copies_spin = new QSpinBox(page);
    copies_spin->setMinimum(1);
    copies_spin->setMaximum(99);
    copies_spin->setValue(1);
    copies_spin->setToolTip(
        QApplication::tr("Number of identical labels per selected item"));
    st->copies_spin_ = copies_spin;
    auto* show_border_check = new QCheckBox(
        QApplication::tr("Show border"), page);
    auto* print_btn = new QPushButton(QApplication::tr("Print"), page);
    btn_box->addWidget(cancel_btn);
    btn_box->addStretch();
    btn_box->addWidget(copies_label);
    btn_box->addWidget(copies_spin);
    btn_box->addWidget(show_border_check);
    btn_box->addWidget(print_btn);
    vbox->addLayout(btn_box);

    colman::CardRenderFn render_fn =
        [&data](QGraphicsScene* sc, int id, qreal sw, qreal sh,
                bool border, colman::IdCardLayout const& arrangement) -> bool {
      colman::Collection const col = data.queries_->read_collection();
      return render_item_card_with_layout(
          data, sc, id, 0, 0, sw, sh, border, col, arrangement);
    };

    // layout_changed fires after LabelSelectionWidget rebuilds its internal
    // scene, which deletes all items in the scene including our overlays.
    // The pointers in preview_items_ are already dangling, so just clear the
    // list and repopulate; do NOT call batch_remove_previews here.
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

    // Copies spin: re-expand the print list so each selected item is printed
    // the chosen number of times.  Ignored while a multi-page sheet print is
    // in progress so the running job is not disturbed.
    QObject::connect(copies_spin,
                     QOverload<int>::of(&QSpinBox::valueChanged),
                     [st, copies_spin]() {
                       if(st->print_started_) {
                         return;
                       }
                       st->remaining_ =
                           expand_with_copies(st->base_ids_, copies_spin->value());
                       st->printed_.clear();
                       st->label_sel_->set_max_count(st->remaining_.size());
                       // Emits selection_changed → previews are refreshed.
                       st->label_sel_->clear_selection();
                     });

    // Cancel button
    QObject::connect(cancel_btn, &QPushButton::clicked,
                     [&data, st]() {
                       colman::batch_abort(st, data.mw_.item_barcode_tree_);
                       update_ibc_status(data);
                       data.mw_.item_barcode_stack_->setCurrentIndex(0);
                     });

    // Print button
    QObject::connect(print_btn, &QPushButton::clicked,
                     [&data, st, render_fn, show_border_check]() {
                       colman::batch_do_print(
                           st, render_fn,
                           show_border_check->isChecked(),
                           data.mw_.item_barcode_page_,
                           data.mw_.item_barcode_stack_,
                           data.mw_.item_barcode_tree_,
                           [&data]() { update_ibc_status(data); },
                           [](QString const& name) {
                             colman::LabelLayoutWidget::
                                 set_last_selected_layout_name(name);
                           });
                     });
  }

  // ---- Navigation + toolbar connections ----

  app.connect(data.print_barcode_button_, &QToolButton::clicked,
              [&data]() {
                enter_item_barcode_page(data);
              });

  app.connect(data.mw_.item_barcode_back_, &QPushButton::clicked,
              [&data, st]() {
                if(data.mw_.item_barcode_stack_->currentIndex() == 1) {
                  colman::batch_abort(st, data.mw_.item_barcode_tree_);
                  update_ibc_status(data);
                  data.mw_.item_barcode_stack_->setCurrentIndex(0);
                }
                else {
                  leave_item_barcode_page(data);
                }
              });

  app.connect(data.mw_.item_barcode_search_, &QLineEdit::textChanged,
              [&data](QString const& text) {
                filter_item_tree(data, text);
                update_ibc_status(data);
              });

  // "Only last N acquired items": toggle enables the spin box and rebuilds
  // the tree; changing N rebuilds only while the restriction is active.
  // Enabling it switches off the mutually exclusive time-span restriction.
  app.connect(data.mw_.item_barcode_recent_check_, &QCheckBox::toggled,
              [&data](bool on) {
                data.mw_.item_barcode_recent_count_->setEnabled(on);
                if(on && data.mw_.item_barcode_daterange_check_->isChecked()) {
                  colman::AutoBlockSignals b(
                      data.mw_.item_barcode_daterange_check_);
                  data.mw_.item_barcode_daterange_check_->setChecked(false);
                  data.mw_.item_barcode_date_from_->setEnabled(false);
                  data.mw_.item_barcode_date_to_->setEnabled(false);
                }
                refresh_item_barcode_page(data);
              });

  app.connect(data.mw_.item_barcode_recent_count_,
              QOverload<int>::of(&QSpinBox::valueChanged),
              [&data](int) {
                if(data.mw_.item_barcode_recent_check_->isChecked()) {
                  refresh_item_barcode_page(data);
                }
              });

  // "Acquired between": toggle enables the date editors and rebuilds the
  // tree; enabling it switches off the mutually exclusive most-recent
  // restriction. Changing a date rebuilds only while the restriction is
  // active.
  app.connect(data.mw_.item_barcode_daterange_check_, &QCheckBox::toggled,
              [&data](bool on) {
                data.mw_.item_barcode_date_from_->setEnabled(on);
                data.mw_.item_barcode_date_to_->setEnabled(on);
                if(on && data.mw_.item_barcode_recent_check_->isChecked()) {
                  colman::AutoBlockSignals b(
                      data.mw_.item_barcode_recent_check_);
                  data.mw_.item_barcode_recent_check_->setChecked(false);
                  data.mw_.item_barcode_recent_count_->setEnabled(false);
                }
                refresh_item_barcode_page(data);
              });

  auto const on_date_changed = [&data](QDate const&) {
    if(data.mw_.item_barcode_daterange_check_->isChecked()) {
      refresh_item_barcode_page(data);
    }
  };
  app.connect(data.mw_.item_barcode_date_from_, &QDateEdit::dateChanged,
              on_date_changed);
  app.connect(data.mw_.item_barcode_date_to_, &QDateEdit::dateChanged,
              on_date_changed);

  app.connect(data.mw_.item_barcode_select_all_, &QPushButton::clicked,
              [&data]() {
                QTreeWidget* tree = data.mw_.item_barcode_tree_;
                colman::AutoBlockSignals b(tree);
                for(int i = 0; i < tree->topLevelItemCount(); ++i) {
                  QTreeWidgetItem* ti = tree->topLevelItem(i);
                  if(!ti->isHidden()) {
                    ti->setCheckState(0, Qt::Checked);
                  }
                }
                update_ibc_status(data);
              });

  app.connect(data.mw_.item_barcode_deselect_all_, &QPushButton::clicked,
              [&data]() {
                QTreeWidget* tree = data.mw_.item_barcode_tree_;
                colman::AutoBlockSignals b(tree);
                for(int i = 0; i < tree->topLevelItemCount(); ++i) {
                  QTreeWidgetItem* ti = tree->topLevelItem(i);
                  if(!ti->isHidden()) {
                    ti->setCheckState(0, Qt::Unchecked);
                  }
                }
                update_ibc_status(data);
              });

  app.connect(data.mw_.item_barcode_tree_,
              &QTreeWidget::currentItemChanged,
              [&data](QTreeWidgetItem* cur, QTreeWidgetItem*) {
                show_item(data,
                          cur ? cur->data(0, Qt::UserRole).toInt() : -1);
              });

  app.connect(data.mw_.item_barcode_tree_,
              &QTreeWidget::itemChanged,
              [&data](QTreeWidgetItem*, int) {
                update_ibc_status(data);
              });

  app.connect(data.mw_.item_card_print_, &QPushButton::clicked,
              [&data, st]() {
                QList<int> ids = checked_item_ids(data);
                if(!ids.isEmpty()) {
                  start_label_print(data, st, ids);
                }
              });
}


/**
   Navigates to the item barcode page with @p item_id pre-checked.
   Calls the file-local enter_item_barcode_page() setup, then iterates
   the tree to find and check the item row so the user can print immediately.
   @param data    central application state.
   @param item_id ID of the item to pre-check; ignored when < 1.
   @param back    invoked when the user leaves the page via its back
                  button; navigates back to the calling page.
*/
void colman::enter_item_barcode_page_for_item(colman::AppData& data,
                                              int item_id,
                                              colman::AppData::AppFunction back)
{
  data.set_ibc_back_function(std::move(back));

  // Clear any tree restriction (recent/date-range filters, search text) so
  // that the requested item is guaranteed to be listed and visible;
  // checked-but-hidden rows would be ignored when printing.
  if(item_id >= 1) {
    if(data.mw_.item_barcode_recent_check_->isChecked()) {
      colman::AutoBlockSignals b(data.mw_.item_barcode_recent_check_);
      data.mw_.item_barcode_recent_check_->setChecked(false);
      data.mw_.item_barcode_recent_count_->setEnabled(false);
    }
    if(data.mw_.item_barcode_daterange_check_->isChecked()) {
      colman::AutoBlockSignals b(data.mw_.item_barcode_daterange_check_);
      data.mw_.item_barcode_daterange_check_->setChecked(false);
      data.mw_.item_barcode_date_from_->setEnabled(false);
      data.mw_.item_barcode_date_to_->setEnabled(false);
    }
    data.mw_.item_barcode_search_->clear();
  }

  enter_item_barcode_page(data);
  if(item_id < 1) {
    return;
  }

  QTreeWidget* tree = data.mw_.item_barcode_tree_;
  for(int i = 0; i < tree->topLevelItemCount(); ++i) {
    QTreeWidgetItem* ti = tree->topLevelItem(i);
    if(ti->data(0, Qt::UserRole).toInt() == item_id) {
      ti->setCheckState(0, Qt::Checked);
      tree->scrollToItem(ti);
      break;
    }
  }
  update_ibc_status(data);
}


/* EOF */
