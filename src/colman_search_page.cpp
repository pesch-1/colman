// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Search page: full-text item search with a per-field filter panel.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_search_page.hpp"
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef COLMAN_ITEM_BARCODE_PAGE_HPP
#  include "colman_item_barcode_page.hpp"
#endif
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef COLMAN_TYPE_HPP
#  include "colman_type.hpp"
#endif
#ifndef COLMAN_UI_BUILD_HPP
#  include "colman_ui_build.hpp"
#endif
#ifndef SYSTEM_QABSTRACTTABLEMODEL
#  include <QAbstractTableModel>
#  define SYSTEM_QABSTRACTTABLEMODEL
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QGROUPBOX
#  include <QGroupBox>
#  define SYSTEM_QGROUPBOX
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
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
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QGUIAPPLICATION
#  include <QGuiApplication>
#  define SYSTEM_QGUIAPPLICATION
#endif
#ifndef SYSTEM_QSCREEN
#  include <QScreen>
#  define SYSTEM_QSCREEN
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QSPLITTER
#  include <QSplitter>
#  define SYSTEM_QSPLITTER
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif

#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QBUFFER
#  include <QBuffer>
#  define SYSTEM_QBUFFER
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QFILEDIALOG
#  include <QFileDialog>
#  define SYSTEM_QFILEDIALOG
#endif
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QPRINTPREVIEWDIALOG
#  include <QPrintPreviewDialog>
#  define SYSTEM_QPRINTPREVIEWDIALOG
#endif
#ifndef SYSTEM_QRADIOBUTTON
#  include <QRadioButton>
#  define SYSTEM_QRADIOBUTTON
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QTEXTDOCUMENT
#  include <QTextDocument>
#  define SYSTEM_QTEXTDOCUMENT
#endif
#ifndef SYSTEM_QTEXTSTREAM
#  include <QTextStream>
#  define SYSTEM_QTEXTSTREAM
#endif
#ifndef COLMAN_MARC21_HPP
#  include "colman_marc21.hpp"
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef COLMAN_LENDING_PAGE_HPP
#  include "colman_lending_page.hpp"
#endif
#ifndef COLMAN_ITEM_WIDGET_HPP
#  include "colman_item_widget.hpp"
#endif

namespace
{

/// Highest selectable grade level in the search filter (German grades 1-13).
  int constexpr MaxGradeLevel = 13;

/// Table model for the search-page item list.
/**
   Five columns: 0=id (hidden), 1=Name, 2=Type, 3=Available, 4=Lent.
*/
  class SearchItemListModel : public QAbstractTableModel
  {
  public:
    using Row = colman::IQueries::SearchItemRow;

    explicit SearchItemListModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent)
    {}

    void set_rows(std::vector<Row> rows)
    {
      beginResetModel();
      rows_ = std::move(rows);
      endResetModel();
    }

    int rowCount(QModelIndex const& = {}) const override
    {
      return static_cast<int>(rows_.size());
    }

    int columnCount(QModelIndex const& = {}) const override
    {
      return 6;
    }

    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override
    {
      if(!idx.isValid() || idx.row() >= static_cast<int>(rows_.size())) {
        return {};
      }
      if(role != Qt::DisplayRole && role != Qt::UserRole) {
        return {};
      }
      Row const& r = rows_[idx.row()];
      if(role == Qt::UserRole) {
        return r.id;
      }
      switch(idx.column()) {
      case 0:
        return r.id;
      case 1:
        return r.name;
      case 2:
        return r.type_locale;
      case 3:
        return r.available;
      case 4:
        return r.count - r.available;
      case 5:
        if(r.total_reserved > 0) {
          return QApplication::tr("%n reserved", "", r.total_reserved);
        }
        return QString();
      }
      return {};
    }

    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override
    {
      if(role != Qt::DisplayRole || o != Qt::Horizontal) {
        return {};
      }
      switch(section) {
      case 0:
        return QStringLiteral("id");
      case 1:
        return QApplication::tr("Name/Title");
      case 2:
        return QApplication::tr("Type");
      case 3:
        return QApplication::tr("Available");
      case 4:
        return QApplication::tr("Lent");
      case 5:
        return QApplication::tr("Reserved");
      }
      return {};
    }

    void append_rows(std::vector<Row> rows)
    {
      if(rows.empty()) {
        return;
      }
      auto const first = static_cast<int>(rows_.size());
      auto const last = first + static_cast<int>(rows.size()) - 1;
      beginInsertRows({}, first, last);
      for(auto& r : rows) {
        rows_.push_back(std::move(r));
      }
      endInsertRows();
    }

    Row const& row_at(int i) const { return rows_[i]; }
    std::vector<Row> const& all_rows() const { return rows_; }

  private:
    std::vector<Row> rows_;
  };


/// Full-page view showing all information for a single item.
/**
   Replaces the former ItemDetailDialog. Occupies a full page-stack slot;
   a back button at the top returns to the search page.
*/
  class ItemDetailPage : public QWidget
  {
  public:
  /// Constructs the page (content is loaded later via load()).
    explicit ItemDetailPage(colman::AppData& data, QWidget* parent = nullptr)
        : QWidget(parent), data_(data)
    {
      auto* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

      auto* top_row = new QHBoxLayout;
      top_row->setContentsMargins(0, 0, 0, 0);
      top_row->setSpacing(0);
      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      top_row->addWidget(back_btn_, 1);
      print_btn_ = new QPushButton(QApplication::tr("print label"), this);
      print_btn_->setEnabled(false);
      top_row->addWidget(print_btn_);
      outer->addLayout(top_row);

      scroll_area_ = new QScrollArea(this);
      scroll_area_->setWidgetResizable(true);
      outer->addWidget(scroll_area_, 1);

      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
      connect(print_btn_, &QPushButton::clicked,
              [this]() {
                if(current_item_id_ < 0) {
                  return;
                }
                colman::enter_item_barcode_page_for_item(
                    data_, current_item_id_, [](colman::AppData& d) {
                      d.mw_.pages_stack_->setCurrentWidget(
                          d.search_item_detail_page_);
                    });
              });
    }

  /// Sets the callback invoked when the back button is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  /// Reloads the currently shown item (no-op when none is shown).
    void reload()
    {
      if(current_item_id_ >= 0) {
        load(current_item_id_);
      }
    }

  /// Loads and displays the item with \a item_id.
    void load(int item_id)
    {
      current_item_id_ = item_id;
      auto* content = new QWidget;
      auto* vbox = new QVBoxLayout(content);
      vbox->setSpacing(8);

      colman::Item item;
      QString type_name;
      if(!data_.queries_->get_item(item_id, item, type_name)) {
        print_btn_->setEnabled(false);
        vbox->addWidget(
            new QLabel(QApplication::tr("Item not found."), content));
        scroll_area_->setWidget(content);
        return;
      }
      print_btn_->setEnabled(true);

    // --- Thumbnail + name heading ---
      {
        auto* top = new QHBoxLayout;
        if(item.get_thumbnail_id() >= 0) {
          QByteArray raw;
          if(data_.queries_->get_thumbnail(item.get_thumbnail_id(), raw)) {
            QPixmap pm;
            pm.loadFromData(raw);
            if(!pm.isNull()) {
              auto* thumb_col = new QVBoxLayout;
              auto* thumb = new QLabel(content);
              thumb->setPixmap(
                  pm.scaled(120, 120, Qt::KeepAspectRatio,
                            Qt::SmoothTransformation));
              thumb->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
              thumb_col->addWidget(thumb);
              auto* zoom_btn =
                  new QPushButton(QApplication::tr("zoom"), content);
              QPixmap const full_pm = pm;
              connect(zoom_btn, &QPushButton::clicked,
                      [full_pm, content]() {
                        auto* dlg = new QDialog(content);
                        dlg->setWindowTitle(
                            QApplication::tr("Cover image"));
                        dlg->setAttribute(Qt::WA_DeleteOnClose);
                        auto const screen =
                            QGuiApplication::primaryScreen()
                                ->availableGeometry()
                                .size();
                        auto const max_size =
                            QSize(screen.width() * 4 / 5,
                                  screen.height() * 4 / 5);
                        auto const scaled = full_pm.scaled(
                            max_size, Qt::KeepAspectRatio,
                            Qt::SmoothTransformation);
                        auto* lbl = new QLabel(dlg);
                        lbl->setPixmap(scaled);
                        lbl->setAlignment(Qt::AlignCenter);
                        auto* layout = new QVBoxLayout(dlg);
                        layout->addWidget(lbl);
                        dlg->adjustSize();
                        dlg->exec();
                      });
              thumb_col->addWidget(zoom_btn);
              top->addLayout(thumb_col);
            }
          }
        }
        auto* name_lbl =
            new colman::ScalingLabel(item.get_name(), content, 3, true);
        top->addWidget(name_lbl, 1);
        vbox->addLayout(top);
      }

    // --- Item properties ---
      auto* props_box =
          new QGroupBox(QApplication::tr("Item Information"), content);
      auto* props_vbox = new QVBoxLayout(props_box);
      auto* tbl = new QTableWidget(0, 2, props_box);
      tbl->setWordWrap(true);
      tbl->verticalHeader()->setSectionResizeMode(
          QHeaderView::ResizeToContents);
      tbl->verticalHeader()->setVisible(false);
      tbl->horizontalHeader()->setVisible(false);
      tbl->horizontalHeader()->setSectionResizeMode(
          0, QHeaderView::ResizeToContents);
      tbl->horizontalHeader()->setSectionResizeMode(
          1, QHeaderView::Stretch);
      tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
      tbl->setSelectionMode(QAbstractItemView::NoSelection);
      tbl->setShowGrid(false);
      tbl->setFrameShape(QFrame::NoFrame);
      tbl->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      tbl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      tbl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
      tbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      props_vbox->addWidget(tbl);

      auto add_row = [&](QString const& label, QString const& value) {
        if(!value.isEmpty()) {
          int const row = tbl->rowCount();
          tbl->insertRow(row);
          auto* lbl_item = new QTableWidgetItem(label + ":");
          lbl_item->setFlags(Qt::ItemIsEnabled);
          tbl->setItem(row, 0, lbl_item);
          auto* val_item = new QTableWidgetItem(value);
          val_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
          tbl->setItem(row, 1, val_item);
        }
      };

      {
        QString const loc = data_.queries_->get_type_locale(type_name);
        add_row(QApplication::tr("Type"), loc.isEmpty() ? type_name : loc);
      }
      add_row(QApplication::tr("Colman barcode"), item.get_barcode());
      add_row(QApplication::tr("Place"), item.get_place());
      add_row(QApplication::tr("Description"), item.get_description());
      if(item.get_is_school_item()) {
        add_row(QApplication::tr("Subject"), item.get_subject());
        add_row(QApplication::tr("Grade level"),
                colman::academic_years_to_string(item.get_academic_year()));
        add_row(QApplication::tr("Subtype"), item.get_type());
      }
    // Type-specific key-value pairs — use localised column names.
      {
        QLocale::Language const lang = QLocale::system().language();
        QStringList const order =
            data_.queries_->get_combined_field_order(item.get_type_id());
        colman::StringMap const& values = item.get_values();
        colman::StringListMap const& list_values = item.get_list_values();
        for(auto const& field : order) {
          if(field.startsWith("__")) {
            continue;
          }
          QString label =
              data_.queries_->get_type_field_label(
                  item.get_type_id(), field, lang);
          if(label.isEmpty()) {
            label = field;
          }
          auto const it = values.find(field);
          if(it != values.end() && !it->second.isEmpty()) {
            add_row(label, it->second);
            continue;
          }
          auto const lit = list_values.find(field);
          if(lit != list_values.end() && !lit->second.isEmpty()) {
            add_row(label, lit->second.join(", "));
          }
        }
      }
      vbox->addWidget(props_box);

    // --- Active lendings ---
      auto const lendings = data_.queries_->query_item_lendings(item_id);
      int active_count = 0;
      for(auto const& li : lendings) {
        std::size_t lent = 0, ret = 0;
        data_.queries_->lending_balance(li.lending_id_, lent, ret);
        if(lent > ret) {
          ++active_count;
        }
      }

      if(active_count > 0) {
        auto* lend_box =
            new QGroupBox(QApplication::tr("Active Lendings"), content);
        auto* lend_vbox = new QVBoxLayout(lend_box);
        auto* lend_tbl = new QTableWidget(active_count, 3, lend_box);
        lend_tbl->setHorizontalHeaderLabels({QApplication::tr("Borrower"),
                                             QApplication::tr("Lent"),
                                             QApplication::tr("Returned")});
        lend_tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        lend_tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
        lend_tbl->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
        lend_tbl->verticalHeader()->setVisible(false);
        int row = 0;
        for(auto const& li : lendings) {
          std::size_t lent = 0, ret = 0;
          if(!data_.queries_->lending_balance(li.lending_id_, lent, ret)) {
            continue;
          }
          if(lent <= ret) {
            continue;
          }
          QString title = li.person_title_;
          if(!title.isEmpty()) {
            title += " ";
          }
          QString const person_name =
              title + li.person_last_name_ + ", " + li.person_first_name_;
          lend_tbl->setItem(row, 0, new QTableWidgetItem(person_name));
          lend_tbl->setItem(
              row, 1, new QTableWidgetItem(QString::number(lent)));
          lend_tbl->setItem(
              row, 2, new QTableWidgetItem(QString::number(ret)));
          ++row;
        }
        lend_vbox->addWidget(lend_tbl);
        vbox->addWidget(lend_box);
      }

    // --- Active reservations ---
      {
        auto const reservations =
            data_.queries_->get_reservations_for_item(item_id);
        if(!reservations.empty()) {
          int const uid = data_.current_data_collector_;
          bool const can_see_who =
              uid >= 0 &&
              data_.queries_->has_permission(uid, "edit_reservations");
          auto* res_box =
              new QGroupBox(QApplication::tr("Active Reservations"), content);
          auto* res_vbox = new QVBoxLayout(res_box);
          int const cols = can_see_who ? 3 : 2;
          auto* res_tbl =
              new QTableWidget(
                  static_cast<int>(reservations.size()), cols, res_box);
          QStringList res_headers;
          if(can_see_who) {
            res_headers << QApplication::tr("Person");
          }
          res_headers << QApplication::tr("Count")
                      << QApplication::tr("Expires");
          res_tbl->setHorizontalHeaderLabels(res_headers);
          res_tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
          res_tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
          res_tbl->horizontalHeader()->setSectionResizeMode(
              QHeaderView::ResizeToContents);
          res_tbl->verticalHeader()->setVisible(false);
          int res_row = 0;
          for(auto const& r : reservations) {
            int col = 0;
            if(can_see_who) {
              colman::Person p;
              QString name;
              if(data_.queries_->get_person(r.person_id, p)) {
                name = p.get_lastname() + ", " + p.get_firstname();
              }
              res_tbl->setItem(res_row, col, new QTableWidgetItem(name));
              ++col;
            }
            res_tbl->setItem(
                res_row, col,
                new QTableWidgetItem(QString::number(r.count)));
            ++col;
            res_tbl->setItem(
                res_row, col,
                new QTableWidgetItem(r.expires_at.toString(Qt::ISODate)));
            ++res_row;
          }
          res_vbox->addWidget(res_tbl);
          vbox->addWidget(res_box);
        }
      }

      vbox->addStretch(1);
      scroll_area_->setWidget(content);
    }

  private:
    colman::AppData& data_;
    std::function<void()> on_back_;
    QPushButton* back_btn_ = nullptr;
    QPushButton* print_btn_ = nullptr;
    QScrollArea* scroll_area_ = nullptr;
    int current_item_id_ = -1;  ///< item currently shown, -1 if none
  };


/// Full-page editor for a single item's data.
/**
   Replaces the former ItemEditDialog. Occupies a full page-stack slot;
   the back button at the top and the cancel button return to the search
   page without saving.
*/
  class ItemEditPage : public QWidget
  {
  public:
  /// Constructs the page (content is loaded later via load()).
    explicit ItemEditPage(colman::AppData& data, QWidget* parent = nullptr)
        : QWidget(parent), data_(data)
    {
      auto* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer->addWidget(back_btn_);

      auto* heading = new colman::ScalingLabel(
          "<b>" + QApplication::tr("Edit item") + "</b>", this, 2);
      outer->addWidget(heading);

      scroll_area_ = new QScrollArea(this);
      scroll_area_->setWidgetResizable(true);
      outer->addWidget(scroll_area_, 1);

      auto* btns = new QDialogButtonBox(
          QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
      QPushButton* const complete_btn = btns->addButton(
          QApplication::tr("Fill in from the internet"),
          QDialogButtonBox::ActionRole);
      complete_btn->setToolTip(QApplication::tr(
          "Searches the internet for this item (by ISBN, EAN or title) and "
          "fills in only empty fields and a missing cover image; existing "
          "entries are kept."));
      outer->addWidget(btns);

      auto go_back = [this]() {
        if(on_back_) {
          on_back_();
        }
      };
      connect(back_btn_, &QPushButton::clicked, go_back);
      connect(btns, &QDialogButtonBox::rejected, go_back);
      connect(btns, &QDialogButtonBox::accepted,
              [this]() { save(); });
      connect(complete_btn, &QPushButton::clicked, [this]() {
        if(item_widget_) {
          item_widget_->internet_lookup(QString());
        }
      });
    }

  /// Sets the callback invoked when the back button is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  /// Sets the callback invoked after the item was saved successfully.
    void set_on_saved(std::function<void()> on_saved)
    {
      on_saved_ = std::move(on_saved);
    }

  /// Loads the item with \a item_id into a fresh editor widget.
    void load(int item_id)
    {
      item_id_ = item_id;
      item_widget_ = new colman::ItemWidget(nullptr);
      item_widget_->setup(data_);
      item_widget_->set_completers();
      item_widget_->set_read_only(false);
      // Internet lookups complete the item but never overwrite its data.
      item_widget_->set_lookup_completes_only(true);
      colman::Item item;
      QString type_name;
      if(data_.queries_->get_item(item_id, item, type_name)) {
        item_widget_->reset_item_type(item.get_type_id());
        item_widget_->item_to_fields(item, type_name);
      }
      scroll_area_->setWidget(item_widget_); // deletes the previous editor
    }

  private:
  /// Saves the edited item; on success notifies and navigates back.
    void save()
    {
      colman::Item item;
      bool ok = item_widget_->fields_to_item(item);
      if(ok) {
        item.set_data_collector_id(data_.current_data_collector_);
        ok = data_.queries_->change_item(item_id_, item);
      }
      if(!ok) {
        QMessageBox::warning(
            this,
            QApplication::tr("Collection Manager"),
            QApplication::tr("Could not save item changes."));
        return;
      }
      if(on_saved_) {
        on_saved_();
      }
      if(on_back_) {
        on_back_();
      }
    }

    colman::AppData& data_;
    int item_id_ = -1;
    std::function<void()> on_back_;
    std::function<void()> on_saved_;
    QPushButton* back_btn_ = nullptr;
    QScrollArea* scroll_area_ = nullptr;
    colman::ItemWidget* item_widget_ = nullptr;
  };


/// One column entry for print/CSV export field selection.
  struct FieldEntry
  {
    QString key;        ///< sentinel, MARC21 tag, or DB column name
    QString csv_header; ///< MARC21 tag or descriptive key for CSV column header
    QString display;    ///< human-readable label shown in the selection dialog
  };

/// Returns a CSV-safe quoted version of \a s.
  QString csv_quote(QString const& s)
  {
    QString out = s;
    out.replace('"', QStringLiteral("\"\""));
    if(out.contains(',') || out.contains('"') || out.contains('\n')) {
      out = '"' + out + '"';
    }
    return out;
  }

/// Extracts the value for field \a fe from \a item.
/**
   Falls back to list_values joined with " | " when no scalar value exists.
   Sentinel fields (__name__ etc.) are handled directly.  Type-specific
   fields are resolved against \a type_metas by MARC21 tag or column name.
*/
  QString get_field_value(
      colman::Item const& item,
      FieldEntry const& fe,
      std::vector<colman::IQueries::TypeFieldMeta> const& type_metas)
  {
    QString const& k = fe.key;
    if(k == "__name__") {
      return item.get_name();
    }
    if(k == "__description__") {
      return item.get_description();
    }
    if(k == "__place__") {
      return item.get_place();
    }
    if(k == "__keywords__") {
      return colman::keywords_to_string(item.get_keywords());
    }
    if(k == "__sig_top__") {
      return item.get_signature_top();
    }
    if(k == "__sig_bottom__") {
      return item.get_signature_bottom();
    }
  // __type__, __count__, __available__ are resolved by the caller.
    for(auto const& meta : type_metas) {
      bool const match =
          (!meta.marc21_tag.isEmpty() && meta.marc21_tag == k) || meta.column == k;
      if(!match) {
        continue;
      }
      auto const& vals = item.get_values();
      auto const it = vals.find(meta.column);
      if(it != vals.end() && !it->second.isEmpty()) {
        return it->second;
      }
      auto const& lvals = item.get_list_values();
      auto const lit = lvals.find(meta.column);
      if(lit != lvals.end() && !lit->second.isEmpty()) {
        return lit->second.join(QStringLiteral(" | "));
      }
      return {};
    }
    return {};
  }

/// Builds the ordered union of field entries for all types in \a rows.
  std::vector<FieldEntry> build_field_entries(
      colman::AppData& data,
      std::vector<SearchItemListModel::Row> const& rows)
  {
    using namespace colman;
    std::vector<FieldEntry> result;
    QLocale::Language const lang = QLocale::system().language();

    auto marc21_display = [](QString const& tag) -> QString {
      Marc21Field const* f = find_marc21_field(tag);
      if(!f) {
        return tag;
      }
      return marc21_translated_display_name(*f) + " (" + tag + ")";
    };

  // Fixed common / sentinel fields.
    result.push_back({"__name__", "245$a", marc21_display("245$a")});
    result.push_back({"__type__", "type",
                      QApplication::tr("Type")});
    result.push_back({"__count__", "stock",
                      QApplication::tr("Stock count")});
    result.push_back({"__available__", "available",
                      QApplication::tr("Available count")});
    result.push_back({"__description__", "520$a", marc21_display("520$a")});
    result.push_back({"__keywords__", "650$a", marc21_display("650$a")});
    result.push_back({"__place__", "place",
                      QApplication::tr("Place")});
    result.push_back({"__sig_top__", "sig_top",
                      QApplication::tr("Signature top")});
    result.push_back({"__sig_bottom__", "sig_bottom",
                      QApplication::tr("Signature bottom")});
    result.push_back({"__reserved__", "reserved",
                      QApplication::tr("Reserved")});

  // Collect unique type_ids from the result rows.
    std::vector<int> type_ids;
    for(auto const& r : rows) {
      if(r.type_id <= 0) {
        continue;
      }
      bool found = false;
      for(int t : type_ids) {
        if(t == r.type_id) {
          found = true;
          break;
        }
      }
      if(!found) {
        type_ids.push_back(r.type_id);
      }
    }

  // Track keys already added to avoid duplicates.
    std::vector<QString> seen_keys;
    for(auto const& fe : result) {
      seen_keys.push_back(fe.key);
    }

    auto already_seen = [&seen_keys](QString const& k) {
      for(auto const& s : seen_keys) {
        if(s == k) {
          return true;
        }
      }
      return false;
    };

    for(int tid : type_ids) {
      auto const metas = data.queries_->get_type_field_metas(tid);
      for(auto const& meta : metas) {
        QString const key =
            meta.marc21_tag.isEmpty() ? meta.column : meta.marc21_tag;
        if(already_seen(key)) {
          continue;
        }
        seen_keys.push_back(key);
        QString display =
            data.queries_->get_type_field_label(tid, meta.column, lang);
        if(display.isEmpty()) {
          display = data.queries_->get_type_field_label(
              tid, meta.column, QLocale::AnyLanguage);
        }
        if(display.isEmpty() && !meta.marc21_tag.isEmpty()) {
          display = marc21_display(meta.marc21_tag);
        }
        if(display.isEmpty()) {
          display = meta.column;
          if(!display.isEmpty()) {
            display[0] = display[0].toUpper();
          }
          display.replace('_', ' ');
        }
        result.push_back({key, meta.marc21_tag.isEmpty() ? meta.column : meta.marc21_tag,
                          display});
      }
    }
    return result;
  }

/// Returns the display name of \a fe with any MARC21 tag suffix stripped.
/** E.g. "Author (100$a)" → "Author". Used as short label in print output. */
  QString short_label(FieldEntry const& fe)
  {
    QString s = fe.display;
    int const i = s.lastIndexOf(QStringLiteral(" ("));
    if(i > 0 && s.endsWith(')')) {
      s.truncate(i);
    }
    return s;
  }

/// Full-page selector for fields and scope before printing or CSV export.
/**
   Replaces the former PrintExportDialog. Occupies a full page-stack slot;
   the back button at the top and the Cancel button return to the search
   page without printing or exporting. The selection widgets are rebuilt on
   every enter() because the available fields depend on the current results.
*/
  class PrintExportPage : public QWidget
  {
  public:
  /// Constructs the page (content is built later via enter()).
    explicit PrintExportPage(QWidget* parent = nullptr)
        : QWidget(parent)
    {
      auto* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer->addWidget(back_btn_);

      heading_ = new colman::ScalingLabel(QString{}, this, 2);
      outer->addWidget(heading_);

      scroll_area_ = new QScrollArea(this);
      scroll_area_->setWidgetResizable(true);
      outer->addWidget(scroll_area_, 1);

      btns_ = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
      accept_btn_ =
          btns_->addButton(QString{}, QDialogButtonBox::AcceptRole);
      outer->addWidget(btns_);

      auto go_back = [this]() {
        if(on_back_) {
          on_back_();
        }
      };
      connect(back_btn_, &QPushButton::clicked, go_back);
      connect(btns_, &QDialogButtonBox::rejected, go_back);
      connect(btns_, &QDialogButtonBox::accepted,
              [this]() { on_accept(); });
    }

  /// Sets the callback invoked when the back/cancel button is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  /// Sets the callback invoked when the user confirms print/export.
    void set_on_accept(std::function<void()> on_accept)
    {
      on_accept_ = std::move(on_accept);
    }

  /// Builds the field/scope selection for a fresh print or export run.
  /**
     @param fields         ordered list of available export fields
     @param total_count    number of items in the full result set
     @param selected_count number of rows currently selected in the table
     @param print_mode     true for print (shows thumbnail option),
                           false for CSV export
  */
    void enter(
        std::vector<FieldEntry> const& fields,
        int total_count,
        int selected_count,
        bool print_mode)
    {
      fields_ = fields;
      print_mode_ = print_mode;

      heading_->setText(
          "<b>" +
          (print_mode_ ? QApplication::tr("Print Items")
                       : QApplication::tr("Export CSV")) +
          "</b>");
      accept_btn_->setText(print_mode_
                               ? QApplication::tr("Print...")
                               : QApplication::tr("Export CSV..."));

      auto* content = new QWidget;
      QVBoxLayout* vbox = new QVBoxLayout(content);

    // Scope radio buttons.
      QHBoxLayout* radio_hbox = new QHBoxLayout;
      all_radio_ = new QRadioButton(
          QApplication::tr("All items (%1)").arg(total_count), content);
      sel_radio_ = new QRadioButton(
          QApplication::tr("Selected items (%1)").arg(selected_count),
          content);
      all_radio_->setChecked(true);
      if(selected_count == 0) {
        sel_radio_->setEnabled(false);
      }
      radio_hbox->addWidget(all_radio_);
      radio_hbox->addWidget(sel_radio_);
      radio_hbox->addStretch(1);
      vbox->addLayout(radio_hbox);

    // Sort order (print only).
      sort_combo_ = nullptr;
      if(print_mode_) {
        QHBoxLayout* sort_hbox = new QHBoxLayout;
        sort_hbox->addWidget(
            new QLabel(QApplication::tr("Sort by:"), content));
        sort_combo_ = new QComboBox(content);
        for(auto const& fe : fields_) {
          sort_combo_->addItem(fe.display);
        }
        sort_hbox->addWidget(sort_combo_, 1);
        vbox->addLayout(sort_hbox);
      }

    // Field selection list.
    // In print mode pre-check only the four default fields; CSV checks all.
      vbox->addWidget(new QLabel(QApplication::tr("Fields:"), content));
      field_list_ = new QListWidget(content);
      for(auto const& fe : fields_) {
        auto* it = new QListWidgetItem(fe.display, field_list_);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        bool const pre_checked = !print_mode_ || fe.key == "__name__" || fe.key == "__type__" || fe.key == "__count__" || fe.key == "100$a";
        it->setCheckState(pre_checked ? Qt::Checked : Qt::Unchecked);
      }
      vbox->addWidget(field_list_, 1);

    // Thumbnail and numbering options (print only).
      thumbnails_cb_ = nullptr;
      number_entries_cb_ = nullptr;
      if(print_mode_) {
        thumbnails_cb_ = new QCheckBox(
            QApplication::tr("Include thumbnails"), content);
        thumbnails_cb_->setChecked(false);
        vbox->addWidget(thumbnails_cb_);
        number_entries_cb_ = new QCheckBox(
            QApplication::tr("Number entries"), content);
        number_entries_cb_->setChecked(false);
        vbox->addWidget(number_entries_cb_);
      }

      load_settings();
      scroll_area_->setWidget(content); // deletes the previous content
    }

  /// Saves the settings, runs the export callback, and returns to search.
    void on_accept()
    {
      save_settings();
      if(on_accept_) {
        on_accept_();
      }
      if(on_back_) {
        on_back_();
      }
    }

  /// Returns true when "All items" radio is selected.
    bool use_all_items() const { return all_radio_->isChecked(); }

  /// Returns all fields whose checkbox is checked.
    std::vector<FieldEntry> chosen_fields() const
    {
      std::vector<FieldEntry> out;
      for(int i = 0; i < field_list_->count(); ++i) {
        if(field_list_->item(i)->checkState() == Qt::Checked) {
          out.push_back(fields_[i]);
        }
      }
      return out;
    }

  /// Returns true when thumbnails should be included (print mode only).
    bool with_thumbnails() const
    {
      return print_mode_ && thumbnails_cb_ && thumbnails_cb_->isChecked();
    }

  /// Returns true when entries should be numbered (print mode only).
    bool number_entries() const
    {
      return print_mode_ && number_entries_cb_ && number_entries_cb_->isChecked();
    }

  /// Returns the field entry selected for sorting (print mode only).
    FieldEntry sort_field() const
    {
      if(sort_combo_ && sort_combo_->currentIndex() >= 0 && sort_combo_->currentIndex() < static_cast<int>(fields_.size()))
      {
        return fields_[sort_combo_->currentIndex()];
      }
      return fields_.empty() ? FieldEntry{} : fields_.front();
    }

  /// Persists the current dialog state to QSettings.
    void save_settings() const
    {
      QSettings settings;
      QString const prefix =
          print_mode_ ? QStringLiteral("search/print")
                      : QStringLiteral("search/csv");
      QStringList checked;
      for(int i = 0; i < field_list_->count(); ++i) {
        if(field_list_->item(i)->checkState() == Qt::Checked) {
          checked.append(fields_[i].key);
        }
      }
      settings.setValue(prefix + QStringLiteral("_fields"), checked);
      if(print_mode_) {
        settings.setValue(
            QStringLiteral("search/print_sort_key"), sort_field().key);
        settings.setValue(
            QStringLiteral("search/print_thumbnails"), with_thumbnails());
        settings.setValue(
            QStringLiteral("search/print_number_entries"), number_entries());
      }
    }

  private:
  /// Restores the previously saved dialog state from QSettings.
    void load_settings()
    {
      QSettings settings;
      QString const prefix =
          print_mode_ ? QStringLiteral("search/print")
                      : QStringLiteral("search/csv");
      QStringList const saved =
          settings.value(prefix + QStringLiteral("_fields")).toStringList();
      if(!saved.isEmpty()) {
        for(int i = 0; i < field_list_->count(); ++i) {
          bool const on = saved.contains(fields_[i].key);
          field_list_->item(i)->setCheckState(
              on ? Qt::Checked : Qt::Unchecked);
        }
      }
      if(print_mode_) {
        QString const sort_key =
            settings.value(QStringLiteral("search/print_sort_key")).toString();
        if(!sort_key.isEmpty() && sort_combo_) {
          for(int i = 0; i < static_cast<int>(fields_.size()); ++i) {
            if(fields_[i].key == sort_key) {
              sort_combo_->setCurrentIndex(i);
              break;
            }
          }
        }
        if(thumbnails_cb_) {
          thumbnails_cb_->setChecked(
              settings.value(
                          QStringLiteral("search/print_thumbnails"), false)
                  .toBool());
        }
        if(number_entries_cb_) {
          number_entries_cb_->setChecked(
              settings.value(
                          QStringLiteral("search/print_number_entries"),
                          false)
                  .toBool());
        }
      }
    }

    std::vector<FieldEntry> fields_;
    bool print_mode_ = false;
    std::function<void()> on_back_;
    std::function<void()> on_accept_;
    QPushButton* back_btn_ = nullptr;
    colman::ScalingLabel* heading_ = nullptr;
    QScrollArea* scroll_area_ = nullptr;
    QDialogButtonBox* btns_ = nullptr;
    QPushButton* accept_btn_ = nullptr;
    QRadioButton* all_radio_ = nullptr;
    QRadioButton* sel_radio_ = nullptr;
    QComboBox* sort_combo_ = nullptr;
    QListWidget* field_list_ = nullptr;
    QCheckBox* thumbnails_cb_ = nullptr;
    QCheckBox* number_entries_cb_ = nullptr;
  };

/// One type-specific group box with its search fields.
  struct TypeSection
  {
    int type_id_ = 0;
    QGroupBox* box_ = nullptr;
    struct Field
    {
      QString col_;    ///< column name in DB
      QLineEdit* edit_ = nullptr;
    };
    std::vector<Field> fields_;
  };


/// Search page: left column with filter fields, right column with item list.
  class SearchPage : public QWidget
  {
  public:
  /// Constructs the search page.
    explicit SearchPage(colman::AppData& data,
                        ItemDetailPage* detail_page,
                        ItemEditPage* edit_page,
                        PrintExportPage* print_export_page,
                        std::function<void()> on_back,
                        QWidget* parent = nullptr)
        : QWidget(parent), data_(data), detail_page_(detail_page),
          edit_page_(edit_page), print_export_page_(print_export_page),
          on_back_(std::move(on_back))
    {
      edit_page_->set_on_saved([this]() { do_search(); });

      QVBoxLayout* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

    // Back button — full width.
      QPushButton* back_btn = new QPushButton(
          QApplication::tr("back"), this);
      back_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer->addWidget(back_btn);

      QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
      outer->addWidget(splitter, 1);

    // ---- Left panel: search controls ----
      left_scroll_ = new QScrollArea(splitter);
      left_scroll_->setWidgetResizable(true);
      left_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      left_content_ = new QWidget;
      left_vbox_ = new QVBoxLayout(left_content_);
      left_vbox_->setSpacing(6);

    // Heading + icon.
      QHBoxLayout* heading_hbox = new QHBoxLayout;
      QLabel* icon_lbl = new QLabel(left_content_);
      QPixmap const pm(":/icons/owls/search.jpeg");
      if(!pm.isNull()) {
        icon_lbl->setPixmap(
            pm.scaledToHeight(48, Qt::SmoothTransformation));
        icon_lbl->setFixedSize(52, 52);
        icon_lbl->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      }
      heading_hbox->addWidget(icon_lbl);
      auto* heading_lbl = new colman::ScalingLabel(
          "<b>" + QApplication::tr("Item Search") + "</b>",
          left_content_, 2);
      heading_hbox->addWidget(heading_lbl, 1);
      left_vbox_->addLayout(heading_hbox);

    // Type filter combo.
      QHBoxLayout* type_hbox = new QHBoxLayout;
      QLabel* type_lbl = new QLabel(
          QApplication::tr("Type:"), left_content_);
      type_hbox->addWidget(type_lbl);
      type_combo_ = new QComboBox(left_content_);
      type_hbox->addWidget(type_combo_, 1);
      left_vbox_->addLayout(type_hbox);

    // Separator.
      {
        QFrame* sep = new QFrame(left_content_);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        left_vbox_->addWidget(sep);
      }

    // General search field.
      QLabel* gen_lbl = new QLabel(
          QApplication::tr("Search (all fields):"), left_content_);
      left_vbox_->addWidget(gen_lbl);
      general_edit_ = new QLineEdit(left_content_);
      general_edit_->setObjectName("search_general_");
      general_edit_->setPlaceholderText(
          QApplication::tr("Search words..."));
      left_vbox_->addWidget(general_edit_);

    // Common specific fields.
      QFormLayout* common_form = new QFormLayout;
      common_form->setRowWrapPolicy(QFormLayout::WrapAllRows);

      auto make_edit = [&](QString const& ph) -> QLineEdit* {
        QLineEdit* e = new QLineEdit(left_content_);
        e->setPlaceholderText(ph);
        return e;
      };

      name_edit_ = make_edit(
          QApplication::tr("Name/Title contains..."));
      common_form->addRow(QApplication::tr("Name/Title:"), name_edit_);

      subject_edit_ = make_edit(
          QApplication::tr("Subject contains..."));
      common_form->addRow(QApplication::tr("Subject:"), subject_edit_);

      description_edit_ = make_edit(
          QApplication::tr("Description contains..."));
      common_form->addRow(QApplication::tr("Description:"), description_edit_);

      keywords_edit_ = make_edit(
          QApplication::tr("Keywords contains..."));
      common_form->addRow(QApplication::tr("Keywords:"), keywords_edit_);

      left_vbox_->addLayout(common_form);

    // Grade-level (academic year) span filter for school items.  The default
    // "any/any" applies no filter; once a span is chosen, only items whose
    // grade range overlaps it are shown (items without a grade level are
    // excluded).
      {
        QHBoxLayout* grade_hbox = new QHBoxLayout;
        grade_hbox->addWidget(
            new QLabel(QApplication::tr("Grade level:"), left_content_));
        grade_from_combo_ = new QComboBox(left_content_);
        grade_to_combo_ = new QComboBox(left_content_);
        grade_from_combo_->addItem(QApplication::tr("any"), 0);
        grade_to_combo_->addItem(QApplication::tr("any"), 0);
        for(int g = 1; g <= MaxGradeLevel; ++g) {
          grade_from_combo_->addItem(QString::number(g), g);
          grade_to_combo_->addItem(QString::number(g), g);
        }
        grade_hbox->addWidget(grade_from_combo_, 1);
        grade_hbox->addWidget(
            new QLabel(QStringLiteral("–"), left_content_));
        grade_hbox->addWidget(grade_to_combo_, 1);
        left_vbox_->addLayout(grade_hbox);
      }

    // Separator before type-specific sections.
      {
        QFrame* sep = new QFrame(left_content_);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        left_vbox_->addWidget(sep);
      }

    // Placeholder for dynamic type-specific sections.
      type_sections_widget_ = new QWidget(left_content_);
      type_sections_layout_ = new QVBoxLayout(type_sections_widget_);
      type_sections_layout_->setContentsMargins(0, 0, 0, 0);
      type_sections_layout_->setSpacing(4);
      left_vbox_->addWidget(type_sections_widget_);

    // Clear button.
      clear_btn_ = new QPushButton(
          QApplication::tr("Clear"), left_content_);
      left_vbox_->addWidget(clear_btn_, 0, Qt::AlignLeft);
      left_vbox_->addStretch(1);

      left_scroll_->setWidget(left_content_);
      splitter->addWidget(left_scroll_);

    // ---- Right panel: item table ----
      QWidget* right_panel = new QWidget(splitter);
      QVBoxLayout* right_vbox = new QVBoxLayout(right_panel);
      right_vbox->setContentsMargins(0, 0, 0, 0);

      list_view_ = new QTableView(right_panel);
      list_view_->setObjectName("search_results_");
      proxy_ = new QSortFilterProxyModel(list_view_);
      proxy_->setSourceModel(&model_);
      list_view_->setModel(proxy_);
      list_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
      list_view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
      list_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
      list_view_->setSortingEnabled(true);
      proxy_->setSortCaseSensitivity(Qt::CaseInsensitive);
      proxy_->sort(1, Qt::AscendingOrder);
      list_view_->horizontalHeader()->setSectionResizeMode(
          QHeaderView::ResizeToContents);
      list_view_->horizontalHeader()->setStretchLastSection(true);
      list_view_->verticalHeader()->setVisible(false);
      list_view_->setColumnHidden(0, true);
      right_vbox->addWidget(list_view_, 1);

      empty_label_ = new QLabel(
          QApplication::tr("Enter search text to show items."),
          right_panel);
      empty_label_->setAlignment(Qt::AlignCenter);
      empty_label_->setWordWrap(true);
      empty_label_->hide();
      right_vbox->addWidget(empty_label_, 1);

      count_label_ = new QLabel(right_panel);
      right_vbox->addWidget(count_label_);

      load_more_btn_ = new QPushButton(
          QApplication::tr("Load more..."), right_panel);
      load_more_btn_->hide();
      right_vbox->addWidget(load_more_btn_);

    // Print and CSV export buttons below the result table.  A FlowLayout
    // lets the buttons wrap onto further rows when the panel is narrow, so
    // the button row no longer forces a wide minimum on the right panel
    // (which would otherwise stop the splitter from moving far enough right).
      QWidget* action_widget = new QWidget(right_panel);
      auto* action_flow = new colman::FlowLayout(action_widget, 0, 6, 6);
      print_btn_ = new QPushButton(
          QApplication::tr("Print..."), right_panel);
      csv_btn_ = new QPushButton(
          QApplication::tr("Export CSV..."), right_panel);
      reserve_btn_ = new QPushButton(
          QApplication::tr("Reserve"), right_panel);
      cancel_res_btn_ = new QPushButton(
          QApplication::tr("Cancel reservation"), right_panel);
      lend_btn_ = new QPushButton(
          QApplication::tr("Lend"), right_panel);
      edit_btn_ = new QPushButton(
          QApplication::tr("Edit item"), right_panel);
      delete_btn_ = new QPushButton(
          QApplication::tr("Delete item"), right_panel);
      print_btn_->setEnabled(false);
      csv_btn_->setEnabled(false);
      reserve_btn_->setEnabled(false);
      cancel_res_btn_->setEnabled(false);
      lend_btn_->setEnabled(false);
      edit_btn_->setEnabled(false);
      edit_btn_->hide();
      delete_btn_->setEnabled(false);
      delete_btn_->hide();
      action_flow->addWidget(print_btn_);
      action_flow->addWidget(csv_btn_);
      action_flow->addWidget(reserve_btn_);
      action_flow->addWidget(lend_btn_);
      action_flow->addWidget(cancel_res_btn_);
      action_flow->addWidget(edit_btn_);
      action_flow->addWidget(delete_btn_);
      right_vbox->addWidget(action_widget);

      splitter->addWidget(right_panel);
      splitter->setStretchFactor(0, 1);
      splitter->setStretchFactor(1, 2);

    // --- Connections ---
      connect(back_btn, &QPushButton::clicked,
              [this]() { on_back_(); });
      connect(clear_btn_, &QPushButton::clicked,
              [this]() { clear_fields(); });

      search_timer_ = new QTimer(this);
      search_timer_->setSingleShot(true);
      search_timer_->setInterval(300);
      connect(search_timer_, &QTimer::timeout,
              [this]() { do_search(); });

      auto on_text_changed = [this]() {
        search_timer_->start();
      };
      connect(general_edit_, &QLineEdit::textChanged, on_text_changed);
      connect(name_edit_, &QLineEdit::textChanged, on_text_changed);
      connect(subject_edit_, &QLineEdit::textChanged, on_text_changed);
      connect(description_edit_, &QLineEdit::textChanged, on_text_changed);
      connect(keywords_edit_, &QLineEdit::textChanged, on_text_changed);

      connect(type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
              [this](int) { on_type_changed(); });

      connect(grade_from_combo_,
              QOverload<int>::of(&QComboBox::currentIndexChanged),
              [this](int) { search_timer_->start(); });
      connect(grade_to_combo_,
              QOverload<int>::of(&QComboBox::currentIndexChanged),
              [this](int) { search_timer_->start(); });

      connect(print_btn_, &QPushButton::clicked,
              [this]() { on_print_clicked(); });
      connect(csv_btn_, &QPushButton::clicked,
              [this]() { on_export_csv_clicked(); });
      connect(&model_, &QAbstractItemModel::modelReset,
              [this]() {
                bool const have = model_.rowCount() > 0;
                print_btn_->setEnabled(have);
                csv_btn_->setEnabled(have);
                lend_btn_->setEnabled(false);
                update_reservation_buttons();
                update_edit_button();
                update_delete_button();
              });

      connect(list_view_->selectionModel(),
              &QItemSelectionModel::selectionChanged,
              [this](QItemSelection const&, QItemSelection const&) {
                bool const have_one = single_selected_item_id() >= 0;
                lend_btn_->setEnabled(have_one);
                update_reservation_buttons();
                update_edit_button();
                update_delete_button();
              });

      connect(reserve_btn_, &QPushButton::clicked,
              [this]() { on_reserve_clicked(); });
      connect(lend_btn_, &QPushButton::clicked,
              [this]() { on_lend_clicked(); });
      connect(cancel_res_btn_, &QPushButton::clicked,
              [this]() { on_cancel_reservation_clicked(); });

      connect(edit_btn_, &QPushButton::clicked,
              [this]() {
                int const item_id = single_selected_item_id();
                if(item_id < 0) {
                  return;
                }
                edit_page_->load(item_id);
                data_.mw_.pages_stack_->setCurrentWidget(edit_page_);
              });

      connect(delete_btn_, &QPushButton::clicked,
              [this]() { on_delete_clicked(); });

      connect(load_more_btn_, &QPushButton::clicked,
              [this]() { load_more(); });

      connect(list_view_, &QTableView::doubleClicked,
              [this](QModelIndex const& proxy_idx) {
                QModelIndex const src_idx =
                    proxy_->mapToSource(proxy_idx);
                if(!src_idx.isValid()) {
                  return;
                }
                int const item_id =
                    model_.row_at(src_idx.row()).id;
                detail_page_->load(item_id);
                data_.mw_.pages_stack_->setCurrentWidget(detail_page_);
              });
    }

  /// Rebuilds type sections and reloads the item list.
    void enter()
    {
      rebuild_type_sections();
      cached_item_count_ = data_.queries_->count_items();
      update_edit_button();
      do_search();
    }

  private:
  /// Clears and rebuilds the per-type group boxes from the database.
    void rebuild_type_sections()
    {
    // Remove all existing type section widgets.
      while(QLayoutItem* item = type_sections_layout_->takeAt(0)) {
        delete item->widget();
        delete item;
      }
      type_sections_.clear();

    // Disconnect and repopulate the type combo.
      type_combo_->blockSignals(true);
      type_combo_->clear();
      type_combo_->addItem(QApplication::tr("All types"), QVariant(-1));

    // query_types(-1) already sets name_ to the localised display name.
      std::vector<colman::Type> const type_stubs =
          data_.queries_->query_types(-1);
      QLocale::Language const lang = QLocale::system().language();

      for(auto const& stub : type_stubs) {
        QString const display = stub.get_name();
        type_combo_->addItem(display, QVariant(stub.get_id()));

      // Load the full type to get the KeyVector.
        colman::Type const full = data_.queries_->get_type(stub.get_id());

        TypeSection sec;
        sec.type_id_ = stub.get_id();

        sec.box_ = new QGroupBox(display, type_sections_widget_);
        QFormLayout* form = new QFormLayout(sec.box_);
        form->setRowWrapPolicy(QFormLayout::WrapAllRows);

        for(auto const& ki : full.get_keys()) {
          QString const& col = ki.first;
          QString label = data_.queries_->get_type_field_label(
              stub.get_id(), col, lang);
          if(label.isEmpty()) {
            label = col;
          }
          QLineEdit* edit = new QLineEdit(sec.box_);
          edit->setPlaceholderText(label + "...");
          form->addRow(label + ":", edit);
          connect(edit, &QLineEdit::textChanged,
                  [this]() { search_timer_->start(); });
          sec.fields_.push_back({col, edit});
        }

        type_sections_layout_->addWidget(sec.box_);
        type_sections_.push_back(std::move(sec));
      }

      type_combo_->blockSignals(false);
      update_type_section_visibility();
    }

  /// Shows or hides type group boxes based on the combo selection.
    void update_type_section_visibility()
    {
      int const sel = type_combo_->currentData().toInt();
      for(auto const& sec : type_sections_) {
        if(sec.box_) {
          sec.box_->setVisible(sel < 0 || sec.type_id_ == sel);
        }
      }
    }

  /// Called when the type combo changes.
    void on_type_changed()
    {
      update_type_section_visibility();
      search_timer_->start();
    }

  /// Returns true when all search fields are empty and type is "All".
    bool all_fields_empty() const
    {
      if(!general_edit_->text().isEmpty()) {
        return false;
      }
      if(!name_edit_->text().isEmpty()) {
        return false;
      }
      if(!subject_edit_->text().isEmpty()) {
        return false;
      }
      if(!description_edit_->text().isEmpty()) {
        return false;
      }
      if(!keywords_edit_->text().isEmpty()) {
        return false;
      }
      if(type_combo_->currentData().toInt() > 0) {
        return false;
      }
      if(grade_from_combo_->currentData().toInt() > 0 ||
         grade_to_combo_->currentData().toInt() > 0) {
        return false;
      }
      for(auto const& sec : type_sections_) {
        for(auto const& field : sec.fields_) {
          if(!field.edit_->text().isEmpty()) {
            return false;
          }
        }
      }
      return true;
    }

  /// Builds the type_fields vector from current filter inputs.
    std::vector<colman::IQueries::TypeFieldFilter>
    collect_type_fields(int sel_type) const
    {
      std::vector<colman::IQueries::TypeFieldFilter> type_fields;
      for(auto const& sec : type_sections_) {
        if(sel_type > 0 && sec.type_id_ != sel_type) {
          continue;
        }
        for(auto const& field : sec.fields_) {
          if(!field.edit_->text().isEmpty()) {
            type_fields.push_back({field.col_, field.edit_->text()});
          }
        }
      }
      return type_fields;
    }

  /// Reads the selected grade-level span; 0 means no bound on that side.
  /** Swaps the bounds when the user picked a higher "from" than "to". */
    void grade_span(int& grade_min, int& grade_max) const
    {
      grade_min = grade_from_combo_->currentData().toInt();
      grade_max = grade_to_combo_->currentData().toInt();
      if(grade_min > 0 && grade_max > 0 && grade_min > grade_max) {
        int const tmp = grade_min;
        grade_min = grade_max;
        grade_max = tmp;
      }
    }

  /// Updates count_label_ to reflect the current number of displayed items.
    void update_count_label()
    {
      int const n = model_.rowCount();
      count_label_->setText(
          QApplication::tr("%n item(s) shown", "", n));
    }

  /// Executes the search and refreshes the model.
    void do_search()
    {
      search_offset_ = 0;
      int const threshold =
          QSettings().value("search_threshold", 1000).toInt();

      if(threshold > 0 && all_fields_empty() && cached_item_count_ > threshold) {
        model_.set_rows({});
        list_view_->hide();
        load_more_btn_->hide();
        count_label_->hide();
        empty_label_->show();
        return;
      }
      empty_label_->hide();
      list_view_->show();
      count_label_->show();

      int const sel_type = type_combo_->currentData().toInt();
      auto const type_fields = collect_type_fields(sel_type);
      int const lim = (threshold > 0) ? threshold : 0;
      int grade_min = 0, grade_max = 0;
      grade_span(grade_min, grade_max);

      auto rows = data_.queries_->query_items_for_search(
          general_edit_->text(),
          name_edit_->text(),
          subject_edit_->text(),
          description_edit_->text(),
          keywords_edit_->text(),
          sel_type,
          type_fields,
          grade_min,
          grade_max,
          lim,
          0);

      has_more_ = (lim > 0 && static_cast<int>(rows.size()) > lim);
      if(has_more_) {
        rows.pop_back();
      }
      model_.set_rows(std::move(rows));
      load_more_btn_->setVisible(has_more_);
      update_count_label();
    }

  /// Loads the next page of results and appends them to the model.
    void load_more()
    {
      int const threshold =
          QSettings().value("search_threshold", 1000).toInt();
      int const lim = (threshold > 0) ? threshold : 0;
      if(lim <= 0) {
        return;
      }
      search_offset_ += lim;

      int const sel_type = type_combo_->currentData().toInt();
      auto const type_fields = collect_type_fields(sel_type);
      int grade_min = 0, grade_max = 0;
      grade_span(grade_min, grade_max);

      auto rows = data_.queries_->query_items_for_search(
          general_edit_->text(),
          name_edit_->text(),
          subject_edit_->text(),
          description_edit_->text(),
          keywords_edit_->text(),
          sel_type,
          type_fields,
          grade_min,
          grade_max,
          lim,
          search_offset_);

      has_more_ = static_cast<int>(rows.size()) > lim;
      if(has_more_) {
        rows.pop_back();
      }
      model_.append_rows(std::move(rows));
      load_more_btn_->setVisible(has_more_);
      update_count_label();
    }

  /// Returns the item IDs of all currently selected rows.
    QList<int> collect_selected_ids() const
    {
      QList<int> result;
      for(auto const& idx :
          list_view_->selectionModel()->selectedRows())
      {
        QModelIndex const src = proxy_->mapToSource(idx);
        if(src.isValid()) {
          result.append(model_.row_at(src.row()).id);
        }
      }
      return result;
    }

  /// Returns the rows to export (all or selected, based on \a use_all).
    std::vector<SearchItemListModel::Row>
    collect_chosen_rows(bool use_all, QList<int> const& sel_ids) const
    {
      std::vector<SearchItemListModel::Row> result;
      if(use_all) {
        result = model_.all_rows();
      }
      else {
        for(auto const& r : model_.all_rows()) {
          if(sel_ids.contains(r.id)) {
            result.push_back(r);
          }
        }
      }
      return result;
    }

  /// Shows the print field/scope selector for the current search results.
    void on_print_clicked()
    {
      std::vector<FieldEntry> const fields =
          build_field_entries(data_, model_.all_rows());
      QList<int> const sel_ids = collect_selected_ids();
      print_export_page_->set_on_accept(
          [this, sel_ids]() { run_print(sel_ids); });
      print_export_page_->enter(
          fields, model_.rowCount(), sel_ids.size(), true);
      data_.mw_.pages_stack_->setCurrentWidget(print_export_page_);
    }

  /// Opens the print preview after the user confirmed the selection.
    void run_print(QList<int> const& sel_ids)
    {
      std::vector<FieldEntry> const chosen =
          print_export_page_->chosen_fields();
      if(chosen.empty()) {
        return;
      }
      bool const with_thumb = print_export_page_->with_thumbnails();
      bool const numbered = print_export_page_->number_entries();
      std::vector<SearchItemListModel::Row> const rows =
          collect_chosen_rows(print_export_page_->use_all_items(), sel_ids);

    // Partition chosen fields: title (line 1), author/100$a (line 2),
    // everything else (line 3 as "Label: Value" pairs).
      FieldEntry const* title_fe = nullptr;
      FieldEntry const* author_fe = nullptr;
      std::vector<FieldEntry const*> rest_fes;
      for(auto const& fe : chosen) {
        if(fe.key == "__name__") {
          title_fe = &fe;
        }
        else if(fe.key == "100$a") {
          author_fe = &fe;
        }
        else {
          rest_fes.push_back(&fe);
        }
      }

    // Pre-load all items so we can sort before generating HTML.
      FieldEntry const sort_fe = print_export_page_->sort_field();
      struct PrintRow
      {
        SearchItemListModel::Row row;
        colman::Item item;
        std::vector<colman::IQueries::TypeFieldMeta> type_metas;
        QString sort_val;
      };
      std::vector<PrintRow> print_rows;
      print_rows.reserve(rows.size());
      for(auto const& row : rows) {
        PrintRow pr;
        pr.row = row;
        QString type_name;
        if(!data_.queries_->get_item(row.id, pr.item, type_name)) {
          continue;
        }
        pr.type_metas =
            data_.queries_->get_type_field_metas(pr.item.get_type_id());
        if(sort_fe.key == "__type__") {
          pr.sort_val = row.type_locale;
        }
        else if(sort_fe.key == "__count__") {
          pr.sort_val = QString::number(row.count).rightJustified(10, '0');
        }
        else if(sort_fe.key == "__available__") {
          pr.sort_val =
              QString::number(row.available).rightJustified(10, '0');
        }
        else {
          pr.sort_val = get_field_value(pr.item, sort_fe, pr.type_metas);
        }
        print_rows.push_back(std::move(pr));
      }
      std::stable_sort(print_rows.begin(), print_rows.end(),
                       [](PrintRow const& a, PrintRow const& b) {
                         return QString::localeAwareCompare(a.sort_val, b.sort_val) < 0;
                       });

    // Build HTML: 2-col table (thumbnail | text) or 1-col (text only).
    // cellpadding='2' keeps items compact; border-top separates entries.
      QString html;
      html += "<html><head><meta charset='utf-8'/></head><body>";
      html += "<h2>" +
              QApplication::tr("Item list").toHtmlEscaped() + "</h2>";
      html += "<table border='0' cellspacing='0' cellpadding='2'"
              " style='width:100%'>";

      bool first_row = true;
      int row_num = 1;
      for(auto const& pr : print_rows) {
      // Gather cell content.
        QString title_val;
        if(title_fe) {
          title_val = pr.item.get_name().toHtmlEscaped();
        }
        QString author_val;
        if(author_fe) {
          author_val =
              get_field_value(pr.item, *author_fe, pr.type_metas)
                  .toHtmlEscaped();
        }
        QStringList rest_parts;
        for(auto const* rfe : rest_fes) {
          QString val;
          if(rfe->key == "__type__") {
            val = pr.row.type_locale;
          }
          else if(rfe->key == "__count__") {
            val = QString::number(pr.row.count);
          }
          else if(rfe->key == "__available__") {
            val = QString::number(pr.row.available);
          }
          else if(rfe->key == "__reserved__") {
            if(pr.row.total_reserved > 0) {
              val = QString::number(pr.row.total_reserved);
            }
          }
          else {
            val = get_field_value(pr.item, *rfe, pr.type_metas);
          }
          if(!val.isEmpty()) {
            rest_parts.append(
                short_label(*rfe).toHtmlEscaped() + ": " + val.toHtmlEscaped());
          }
        }

      // Compose text cell.
        QString cell;
        if(!title_val.isEmpty()) {
          cell += "<b>" + title_val + "</b>";
        }
        if(!author_val.isEmpty()) {
          if(!cell.isEmpty()) {
            cell += "<br/>";
          }
          cell += author_val;
        }
        if(!rest_parts.isEmpty()) {
          if(!cell.isEmpty()) {
            cell += "<br/>";
          }
          cell += rest_parts.join(QStringLiteral(", "));
        }

      // Separator: border-top on the cells themselves (no extra empty row).
        QString const sep = first_row
                                ? QString{}
                                : QStringLiteral(";border-top:1px solid #aaa");
        first_row = false;

        html += "<tr>";
        if(numbered) {
          html += "<td style='width:2em;vertical-align:top" + sep + "'>" + QString::number(row_num++) + ".</td>";
        }
        if(with_thumb) {
          html += "<td style='width:90px;vertical-align:top" + sep + "'>";
          if(pr.item.get_thumbnail_id() >= 0) {
            QByteArray raw;
            if(data_.queries_->get_thumbnail(
                   pr.item.get_thumbnail_id(), raw))
            {
              QPixmap pm;
              pm.loadFromData(raw);
              if(!pm.isNull()) {
                pm = pm.scaledToWidth(80, Qt::SmoothTransformation);
                QByteArray png;
                QBuffer buf(&png);
                pm.save(&buf, "PNG");
                html += "<img src='data:image/png;base64,";
                html += QString::fromLatin1(png.toBase64());
                html += "'/>";
              }
            }
          }
          html += "</td>";
        }
        html += "<td style='vertical-align:top" + sep + "'>" + cell + "</td>";
        html += "</tr>";
      }
      html += "</table></body></html>";

      QTextDocument doc;
      doc.setHtml(html);
      QPrinter printer(QPrinter::HighResolution);
      printer.setPageMargins(
          QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
      QPrintPreviewDialog preview(&printer, this);
      preview.connect(
          &preview, &QPrintPreviewDialog::paintRequested,
          [&doc](QPrinter* p) {
            doc.setPageSize(p->pageRect(QPrinter::Point).size());
            doc.print(p);
          });
      preview.exec();
    }

  /// Shows the CSV field/scope selector for the current search results.
    void on_export_csv_clicked()
    {
      std::vector<FieldEntry> const fields =
          build_field_entries(data_, model_.all_rows());
      QList<int> const sel_ids = collect_selected_ids();
      print_export_page_->set_on_accept(
          [this, sel_ids]() { run_export_csv(sel_ids); });
      print_export_page_->enter(
          fields, model_.rowCount(), sel_ids.size(), false);
      data_.mw_.pages_stack_->setCurrentWidget(print_export_page_);
    }

  /// Writes the selected fields to a CSV file after user confirmation.
    void run_export_csv(QList<int> const& sel_ids)
    {
      std::vector<FieldEntry> const chosen =
          print_export_page_->chosen_fields();
      if(chosen.empty()) {
        return;
      }

      QString const path = QFileDialog::getSaveFileName(
          this, QApplication::tr("Save CSV file"), {},
          QStringLiteral("CSV (*.csv)"));
      if(path.isEmpty()) {
        return;
      }

      QFile file(path);
      if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            this,
            QApplication::tr("Export failed"),
            QApplication::tr("Could not write CSV file."));
        return;
      }
      QTextStream out(&file);

    // Header row.
      QStringList header;
      for(auto const& fe : chosen) {
        header.append(csv_quote(fe.csv_header));
      }
      out << header.join(',') << '\n';

    // Data rows.
      std::vector<SearchItemListModel::Row> const rows =
          collect_chosen_rows(print_export_page_->use_all_items(), sel_ids);
      for(auto const& row : rows) {
        colman::Item item;
        QString type_name;
        if(!data_.queries_->get_item(row.id, item, type_name)) {
          continue;
        }
        auto const type_metas =
            data_.queries_->get_type_field_metas(item.get_type_id());
        QStringList cells;
        for(auto const& fe : chosen) {
          QString val;
          if(fe.key == "__type__") {
            val = row.type_locale;
          }
          else if(fe.key == "__count__") {
            val = QString::number(row.count);
          }
          else if(fe.key == "__available__") {
            val = QString::number(row.available);
          }
          else if(fe.key == "__reserved__") {
            if(row.total_reserved > 0) {
              val = QString::number(row.total_reserved);
            }
          }
          else {
            val = get_field_value(item, fe, type_metas);
          }
          cells.append(csv_quote(val));
        }
        out << cells.join(',') << '\n';
      }
    }

  /// Returns the item_id of the single selected row, or -1 if none/multi.
    int single_selected_item_id() const
    {
      QModelIndexList const sel =
          list_view_->selectionModel()->selectedRows();
      if(sel.size() != 1) {
        return -1;
      }
      QModelIndex const src = proxy_->mapToSource(sel.front());
      if(!src.isValid()) {
        return -1;
      }
      return model_.row_at(src.row()).id;
    }

  /// Returns the stock count of the single selected row, or -1 if none/multi.
    int single_selected_item_count() const
    {
      QModelIndexList const sel =
          list_view_->selectionModel()->selectedRows();
      if(sel.size() != 1) {
        return -1;
      }
      QModelIndex const src = proxy_->mapToSource(sel.front());
      if(!src.isValid()) {
        return -1;
      }
      return model_.row_at(src.row()).count;
    }

  /// Updates the visible/enabled state of the edit-item button.
    void update_edit_button()
    {
      int const uid = data_.current_data_collector_;
      bool const can_edit =
          uid >= 0 && data_.queries_->has_permission(uid, "collect_acquisitions");
      edit_btn_->setVisible(can_edit);
      edit_btn_->setEnabled(can_edit && single_selected_item_id() >= 0);
    }

  /// Updates the visible/enabled state of the delete-item button.
  /**
   * The button is only shown to users with the collect_acquisitions
   * permission (same gate as the REST delete route), and only enabled when
   * a single item with zero copies is selected, since only such items may
   * be removed.
   */
    void update_delete_button()
    {
      int const uid = data_.current_data_collector_;
      bool const can_delete =
          uid >= 0 && data_.queries_->has_permission(uid, "collect_acquisitions");
      delete_btn_->setVisible(can_delete);
      delete_btn_->setEnabled(can_delete && single_selected_item_count() == 0);
    }

  /// Updates the enabled/visible state of the reservation action buttons.
    void update_reservation_buttons()
    {
      int const uid = data_.current_data_collector_;
      if(uid < 0) {
        reserve_btn_->setVisible(false);
        cancel_res_btn_->setVisible(false);
        return;
      }
      colman::Collection const col = data_.queries_->read_collection();
      bool const can_edit =
          data_.queries_->has_permission(uid, "edit_reservations");

      int const item_id = single_selected_item_id();
      bool item_allows_reservations = col.get_allow_reservations();
      if(item_id >= 0) {
        colman::Item item;
        QString tmp;
        data_.queries_->get_item(item_id, item, tmp);
        if(item.get_allow_reservations() != -1) {
          item_allows_reservations = (item.get_allow_reservations() != 0);
        }
      }
      bool const can_reserve =
          item_allows_reservations && data_.queries_->has_permission(uid, "reserve");

    // The reserve button requires reservations to be allowed; the cancel
    // button stays available for edit_reservations users to manage
    // existing reservations.
      reserve_btn_->setVisible(can_reserve);
      cancel_res_btn_->setVisible(can_reserve || can_edit);

      if(item_id < 0 || (!can_reserve && !can_edit)) {
        reserve_btn_->setEnabled(false);
        cancel_res_btn_->setEnabled(false);
        return;
      }

    // Enable reserve if there are copies left that nobody has reserved.
      auto const reservations =
          data_.queries_->get_reservations_for_item(item_id);
      int reserved_total = 0;
      for(auto const& r : reservations) {
        reserved_total += r.count;
      }
      int const avail = static_cast<int>(data_.queries_->available(item_id));
      int const free = avail - reserved_total;
      reserve_btn_->setEnabled(can_reserve && free > 0);

    // Enable cancel if this user (or anyone, for edit_reservations) has a reservation.
      bool has_own_res = false;
      colman::IQueries::Reservation own_res;
      if(can_reserve) {
        has_own_res = data_.queries_->get_reservation(item_id, uid, own_res);
      }
      cancel_res_btn_->setEnabled(
          has_own_res || (can_edit && !reservations.empty()));
    }

  /// Handles the Reserve button click: opens lending wizard with reserve pre-selected.
    void on_reserve_clicked()
    {
      int const item_id = single_selected_item_id();
      if(item_id < 0) {
        return;
      }
      colman::enter_reserve_page_for_item(data_, item_id);
    }

  /// Handles the Lend button click: opens lending wizard with item pre-selected.
    void on_lend_clicked()
    {
      int const item_id = single_selected_item_id();
      if(item_id < 0) {
        return;
      }
      colman::ActionInfo ai;
      ai.item_id_ = item_id;
      colman::enter_lend_page_for_action(data_, ai);
    }

  /// Handles the Delete item button click.
  /**
   * Permanently removes the selected item together with all its past
   * acquisitions, liquidations, lendings and returns.  Only allowed for
   * items with zero copies; asks for confirmation first.
   */
    void on_delete_clicked()
    {
      QModelIndexList const sel =
          list_view_->selectionModel()->selectedRows();
      if(sel.size() != 1) {
        return;
      }
      QModelIndex const src = proxy_->mapToSource(sel.front());
      if(!src.isValid()) {
        return;
      }
      auto const& row = model_.row_at(src.row());
      if(row.count != 0) {
      // Should not happen: button is disabled unless count == 0.
        return;
      }

      QMessageBox::StandardButton const answer = QMessageBox::warning(
          this,
          QApplication::tr("Delete item"),
          QApplication::tr(
              "Permanently delete \"%1\" and all its past acquisitions, "
              "liquidations, lendings and returns?\n\n"
              "This cannot be undone.")
              .arg(row.name),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No);
      if(answer != QMessageBox::Yes) {
        return;
      }

      if(data_.queries_->remove_item(row.id)) {
        do_search();
      }
      else {
        QMessageBox::critical(
            this,
            QApplication::tr("Collection Manager"),
            QApplication::tr("Could not delete the item."));
      }
    }

  /// Handles the Cancel reservation button click.
    void on_cancel_reservation_clicked()
    {
      int const item_id = single_selected_item_id();
      if(item_id < 0) {
        return;
      }
      int const uid = data_.current_data_collector_;
      bool const can_edit =
          data_.queries_->has_permission(uid, "edit_reservations");

    // Find the reservation to cancel.
      int res_id = -1;
      if(can_edit) {
      // Let user pick from all active reservations for this item.
        auto const ress = data_.queries_->get_reservations_for_item(item_id);
        if(ress.empty()) {
          return;
        }
        if(ress.size() == 1) {
          res_id = ress.front().id;
        }
        else {
        // Build a selection dialog with one entry per reservation.
          QDialog picker(this);
          picker.setWindowTitle(QApplication::tr("Cancel reservation"));
          auto* vbox = new QVBoxLayout(&picker);
          vbox->addWidget(new QLabel(
              QApplication::tr("Select reservation to cancel:"), &picker));
          auto* list = new QListWidget(&picker);
          for(auto const& r : ress) {
            colman::Person p;
            if(data_.queries_->get_person(r.person_id, p)) {
              list->addItem(
                  p.get_lastname() + ", " + p.get_firstname() + " (" + QString::number(r.count) + "x)");
            }
            else {
              list->addItem(QStringLiteral("ID %1").arg(r.person_id));
            }
            list->item(list->count() - 1)->setData(Qt::UserRole, r.id);
          }
          list->setCurrentRow(0);
          auto* btns = new QDialogButtonBox(
              QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &picker);
          connect(btns, &QDialogButtonBox::accepted,
                  &picker, &QDialog::accept);
          connect(btns, &QDialogButtonBox::rejected,
                  &picker, &QDialog::reject);
          vbox->addWidget(list);
          vbox->addWidget(btns);
          if(picker.exec() != QDialog::Accepted) {
            return;
          }
          QListWidgetItem const* sel = list->currentItem();
          if(!sel) {
            return;
          }
          res_id = sel->data(Qt::UserRole).toInt();
        }
      }
      else {
        colman::IQueries::Reservation r;
        if(!data_.queries_->get_reservation(item_id, uid, r)) {
          return;
        }
        res_id = r.id;
      }
      if(res_id < 0) {
        return;
      }
      if(data_.queries_->remove_reservation(res_id)) {
        do_search();
      }
      else {
        QMessageBox::warning(
            this,
            QApplication::tr("Collection Manager"),
            QApplication::tr("Could not cancel reservation."));
      }
    }

  /// Clears all search fields (triggers a new search via textChanged).
    void clear_fields()
    {
      general_edit_->clear();
      name_edit_->clear();
      subject_edit_->clear();
      description_edit_->clear();
      keywords_edit_->clear();
      grade_from_combo_->setCurrentIndex(0);
      grade_to_combo_->setCurrentIndex(0);
      for(auto const& sec : type_sections_) {
        for(auto const& field : sec.fields_) {
          field.edit_->clear();
        }
      }
    }

    colman::AppData& data_;
    ItemDetailPage* detail_page_ = nullptr;
    ItemEditPage* edit_page_ = nullptr;
    PrintExportPage* print_export_page_ = nullptr;
    std::function<void()> on_back_;
    SearchItemListModel model_;
    QSortFilterProxyModel* proxy_ = nullptr;
    QTableView* list_view_ = nullptr;
    QPushButton* print_btn_ = nullptr;
    QPushButton* csv_btn_ = nullptr;
    QPushButton* reserve_btn_ = nullptr;
    QPushButton* lend_btn_ = nullptr;
    QPushButton* cancel_res_btn_ = nullptr;
    QPushButton* edit_btn_ = nullptr;
    QPushButton* delete_btn_ = nullptr;
    QScrollArea* left_scroll_ = nullptr;
    QWidget* left_content_ = nullptr;
    QVBoxLayout* left_vbox_ = nullptr;
    QComboBox* type_combo_ = nullptr;
    QLineEdit* general_edit_ = nullptr;
    QLineEdit* name_edit_ = nullptr;
    QLineEdit* subject_edit_ = nullptr;
    QLineEdit* description_edit_ = nullptr;
    QLineEdit* keywords_edit_ = nullptr;
    QComboBox* grade_from_combo_ = nullptr;
    QComboBox* grade_to_combo_ = nullptr;
    QPushButton* clear_btn_ = nullptr;
    QWidget* type_sections_widget_ = nullptr;
    QVBoxLayout* type_sections_layout_ = nullptr;
    std::vector<TypeSection> type_sections_;
    QTimer* search_timer_ = nullptr;
    QLabel* empty_label_ = nullptr;
    QLabel* count_label_ = nullptr;
    QPushButton* load_more_btn_ = nullptr;
    int cached_item_count_ = 0;
    int search_offset_ = 0;
    bool has_more_ = false;
  };

} // namespace


/// Initializes the search page and wires the search button.
/**
   Creates a SearchPage widget, adds it to the page stack, and connects
   the search_button_ on the central page to navigate to it.
   @param app  Qt application object.
   @param data central application state.
*/
void colman::setup_search_page(QApplication& app, colman::AppData& data)
{
  auto* detail_page = new ItemDetailPage(data);
  data.mw_.pages_stack_->addWidget(detail_page);
  // Expose the detail page so the REST client's change poller can refresh it
  // when the shown item's availability changes on the server.
  data.search_item_detail_page_ = detail_page;
  data.refresh_search_item_detail_ = [detail_page]() { detail_page->reload(); };

  auto* edit_page = new ItemEditPage(data);
  data.mw_.pages_stack_->addWidget(edit_page);

  auto* print_export_page = new PrintExportPage;
  data.mw_.pages_stack_->addWidget(print_export_page);

  auto* search_page = new SearchPage(
      data,
      detail_page,
      edit_page,
      print_export_page,
      [&data]() {
        data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
      });
  data.mw_.pages_stack_->addWidget(search_page);

  detail_page->set_on_back([&data, search_page]() {
    data.mw_.pages_stack_->setCurrentWidget(search_page);
  });

  edit_page->set_on_back([&data, search_page]() {
    data.mw_.pages_stack_->setCurrentWidget(search_page);
  });

  print_export_page->set_on_back([&data, search_page]() {
    data.mw_.pages_stack_->setCurrentWidget(search_page);
  });

  app.connect(
      data.search_button_, &QToolButton::clicked,
      [&data, search_page]() {
        search_page->enter();
        data.mw_.pages_stack_->setCurrentWidget(search_page);
      });
}

/* EOF */
