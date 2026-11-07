// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// start page setup function
/**
   @file
   @author Christoph Pesch
*/

#include "colman_start_page.hpp"
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP
#  include "colman_label_layout_widget.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_PERSON_PAGE_HPP
#  include "colman_person_page.hpp"
#endif
#ifndef COLMAN_PERSON_VIEW_PAGE_HPP
#  include "colman_person_view_page.hpp"
#endif
#ifndef COLMAN_PERSON_BARCODE_PAGE_HPP
#  include "colman_person_barcode_page.hpp"
#endif
#ifndef COLMAN_ITEM_BARCODE_PAGE_HPP
#  include "colman_item_barcode_page.hpp"
#endif
#ifndef COLMAN_ITEM_TYPE_PAGE_HPP
#  include "colman_item_type_page.hpp"
#endif
#ifndef COLMAN_HISTORY_PAGE_HPP
#  include "colman_history_page.hpp"
#endif
#ifndef COLMAN_ACQUISITION_PAGE_HPP
#  include "colman_acquisition_page.hpp"
#endif
#ifndef COLMAN_LENDING_PAGE_HPP
#  include "colman_lending_page.hpp"
#endif
#ifndef COLMAN_GROUP_LENDING_PAGE_HPP
#  include "colman_group_lending_page.hpp"
#endif
#ifndef COLMAN_LIQUIDATION_PAGE_HPP
#  include "colman_liquidation_page.hpp"
#endif
#ifndef COLMAN_RETURN_PAGE_HPP
#  include "colman_return_page.hpp"
#endif
#ifndef COLMAN_CREATE_COLLECTION_PAGE_HPP
#  include "colman_create_collection_page.hpp"
#endif
#ifndef COLMAN_SETTINGS_PAGE_HPP
#  include "colman_settings_page.hpp"
#endif
#ifndef COLMAN_CENTRAL_PAGE_HPP
#  include "colman_central_page.hpp"
#endif
#ifndef COLMAN_CAMERA_SCAN_PAGE_HPP
#  include "colman_camera_scan_page.hpp"
#endif
#ifndef COLMAN_SEARCH_PAGE_HPP
#  include "colman_search_page.hpp"
#endif
#ifndef COLMAN_CLASS_MANAGEMENT_PAGE_HPP
#  include "colman_class_management_page.hpp"
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#endif
#ifndef COLMAN_SCALING_LABEL_HPP
#  include "colman_scaling_label.hpp"
#endif
#ifndef COLMAN_TILE_BUTTON_HPP
#  include "colman_tile_button.hpp"
#endif
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QDIALOGBUTTONBOX
#  include <QDialogButtonBox>
#  define SYSTEM_QDIALOGBUTTONBOX
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
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
#ifndef SYSTEM_QSIZE
#  include <QSize>
#  define SYSTEM_QSIZE
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif
#ifndef SYSTEM_QSPACERITEM
#  include <QSpacerItem>
#  define SYSTEM_QSPACERITEM
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QFILEDIALOG
#  include <QFileDialog>
#  define SYSTEM_QFILEDIALOG
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_FUNCTIONAL
#  include <functional>
#  define SYSTEM_FUNCTIONAL
#endif
#ifndef SYSTEM_QSTATUSBAR
#  include <QStatusBar>
#  define SYSTEM_QSTATUSBAR
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QTEXTDOCUMENT
#  include <QTextDocument>
#  define SYSTEM_QTEXTDOCUMENT
#endif
#ifndef SYSTEM_QPRINTER
#  include <QPrinter>
#  define SYSTEM_QPRINTER
#endif
#ifndef SYSTEM_QPAGELAYOUT
#  include <QPageLayout>
#  define SYSTEM_QPAGELAYOUT
#endif
#ifndef SYSTEM_QPRINTPREVIEWDIALOG
#  include <QPrintPreviewDialog>
#  define SYSTEM_QPRINTPREVIEWDIALOG
#endif
#ifndef SYSTEM_QSCREEN
#  include <QScreen>
#  define SYSTEM_QSCREEN
#endif
#ifndef SYSTEM_QSTYLE
#  include <QStyle>
#  define SYSTEM_QSTYLE
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif
#ifndef SYSTEM_QLOCKFILE
#  include <QLockFile>
#  define SYSTEM_QLOCKFILE
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif

void colman::update_login_status(colman::AppData& data)
{
  if(data.current_data_collector_ < 0) {
    data.w_.statusBar()->clearMessage();
    return;
  }
  QString name;
  if(data.current_data_collector_ == 0) {
    name = QApplication::tr("Administrator");
  }
  else {
    colman::Person person;
    if(data.queries_->get_person(data.current_data_collector_, person)) {
      name =
          (person.get_firstname() + " " + person.get_lastname()).trimmed();
    }
  }
  data.w_.statusBar()->showMessage(QApplication::tr("Login: ") + name);
}


namespace
{

/// Full page showing lendings that are due today (or this weekend on Fridays).
/**
   Replaces the former DueLendingsDialog. Displays a waiting-owl icon, the
   heading "Due Returns", and a table with columns: due date, borrower,
   count, item name. The back button at the top returns to the central page.
*/
  class DueLendingsPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param rows   due lending rows returned by query_due_lendings()
     @param parent optional parent widget
  */
    explicit DueLendingsPage(
        std::vector<colman::DueLendingListModel::Row> rows,
        QWidget* parent = nullptr)
        : QWidget(parent), rows_(std::move(rows))
    {
      QVBoxLayout* outer = new QVBoxLayout(this);
      outer->setContentsMargins(0, 0, 0, 0);
      outer->setSpacing(0);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      outer->addWidget(back_btn_);

      QWidget* content = new QWidget(this);
      QVBoxLayout* vbox = new QVBoxLayout(content);
      outer->addWidget(content, 1);

      QHBoxLayout* hdr_hbox = new QHBoxLayout;
      QLabel* owl_lbl = new QLabel(content);
      QPixmap const pm(":/icons/owls/reserve2.jpeg");
      if(!pm.isNull()) {
        owl_lbl->setPixmap(pm.scaledToHeight(64, Qt::SmoothTransformation));
        owl_lbl->setFixedHeight(64);
      }
      hdr_hbox->addWidget(owl_lbl);
      QLabel* title_lbl = new colman::ScalingLabel(
          "<b>" + QApplication::tr("Due Returns") + "</b>", content, 4);
      hdr_hbox->addWidget(title_lbl, 1);
      vbox->addLayout(hdr_hbox);

      QStringList const headers = {
          QApplication::tr("due date"),
          QApplication::tr("borrower"),
          QApplication::tr("count"),
          QApplication::tr("name")};
      QTableWidget* table = new QTableWidget(
          static_cast<int>(rows_.size()), 4, content);
      table->setHorizontalHeaderLabels(headers);
      table->setSelectionBehavior(QAbstractItemView::SelectRows);
      table->setSelectionMode(QAbstractItemView::SingleSelection);
      table->setEditTriggers(QAbstractItemView::NoEditTriggers);
      table->horizontalHeader()->setSectionResizeMode(
          QHeaderView::ResizeToContents);
      table->horizontalHeader()->setStretchLastSection(true);
      table->verticalHeader()->setVisible(false);

      for(int row = 0; row < static_cast<int>(rows_.size()); ++row) {
        colman::DueLendingListModel::Row const& r = rows_[row];
        QString const due_str = r.duedate.isValid()
                                    ? QLocale().toString(r.duedate, QLocale::ShortFormat)
                                    : QString{};
        auto* it0 = new QTableWidgetItem(due_str);
        it0->setFlags(it0->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 0, it0);
        auto* it1 = new QTableWidgetItem(r.person_with_class());
        it1->setFlags(it1->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 1, it1);
        auto* it2 = new QTableWidgetItem(
            QString::number(r.outstanding_count));
        it2->setFlags(it2->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 2, it2);
        auto* it3 = new QTableWidgetItem(r.item_name);
        it3->setFlags(it3->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 3, it3);
      }
      vbox->addWidget(table, 1);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      QPushButton* print_btn = new QPushButton(
          QIcon(":/icons/owls/printer.jpeg"), QString{}, content);
      print_btn->setIconSize(QSize(48, 48));
      print_btn->setMinimumHeight(60);
      print_btn->setToolTip(QApplication::tr("Print"));
      btn_hbox->addWidget(print_btn);
      number_cb_ = new QCheckBox(
          QApplication::tr("Number entries"), content);
      number_cb_->setChecked(
          QSettings().value(
                         QStringLiteral("due_returns/number_entries"),
                         false)
              .toBool());
      btn_hbox->addWidget(number_cb_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);

      connect(print_btn, &QPushButton::clicked,
              this, &DueLendingsPage::do_print);
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_back_) {
                  on_back_();
                }
              });
    }

  /// Sets the callback invoked when the back button is pressed.
    void set_on_back(std::function<void()> on_back)
    {
      on_back_ = std::move(on_back);
    }

  private:
    std::vector<colman::DueLendingListModel::Row> rows_; ///< rows to display
    std::function<void()> on_back_;
    QCheckBox* number_cb_ = nullptr;
    QPushButton* back_btn_ = nullptr;

    void do_print()
    {
      bool const numbered = number_cb_->isChecked();
      QSettings().setValue(
          QStringLiteral("due_returns/number_entries"), numbered);

      QString html;
      html += "<html><head><meta charset='utf-8'/></head><body>";
      html += "<h2>" + QApplication::tr("Due Returns") + "</h2>";
      html +=
          "<table border='1' cellspacing='0' cellpadding='4'"
          " style='border-collapse:collapse;width:100%'>"
          "<tr>";
      if(numbered) {
        html += "<th>#</th>";
      }
      html +=
          "<th>" + QApplication::tr("due date") + "</th>"
                                                  "<th>" +
          QApplication::tr("borrower") + "</th>"
                                         "<th>" +
          QApplication::tr("count") + "</th>"
                                      "<th>" +
          QApplication::tr("name") + "</th>"
                                     "</tr>";
      int row_num = 1;
      for(colman::DueLendingListModel::Row const& r : rows_) {
        QString const due = r.duedate.isValid()
                                ? QLocale().toString(r.duedate, QLocale::ShortFormat)
                                : QString{};
        html += "<tr>";
        if(numbered) {
          html += "<td>" + QString::number(row_num++) + "</td>";
        }
        html +=
            "<td>" + due.toHtmlEscaped() + "</td>"
                                           "<td>" +
            r.person_with_class().toHtmlEscaped() + "</td>"
                                            "<td>" +
            QString::number(r.outstanding_count) + "</td>"
                                                   "<td>" +
            r.item_name.toHtmlEscaped() + "</td>"
                                          "</tr>";
      }
      html += "</table></body></html>";

      QTextDocument doc;
      doc.setHtml(html);
      QPrinter printer(QPrinter::HighResolution);
      printer.setPageMargins(
          QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
      QPrintPreviewDialog dialog(&printer, this);
      dialog.connect(
          &dialog, &QPrintPreviewDialog::paintRequested,
          [&doc](QPrinter* p) {
            doc.setPageSize(p->pageRect(QPrinter::Point).size());
            doc.print(p);
          });
      dialog.exec();
    }
  };


/// Shows the due-lendings page, replacing any leftover previous instance.
/**
   The page is created on demand because its row data is fixed at
   construction time; the back button returns to the central page.
   @param data central application state.
   @param rows due lending rows returned by query_due_lendings()
*/
  void show_due_lendings_page(
      colman::AppData& data,
      std::vector<colman::DueLendingListModel::Row> rows)
  {
  // Remove a leftover page from a previously opened collection.
    int const stack_size = data.mw_.pages_stack_->count();
    for(int i = 0; i < stack_size; ++i) {
      QWidget* w = data.mw_.pages_stack_->widget(i);
      if(w && w->objectName() == "due_lendings_page") {
        data.mw_.pages_stack_->removeWidget(w);
        delete w;
        break;
      }
    }

    auto* page = new DueLendingsPage(std::move(rows));
    page->setObjectName("due_lendings_page");
    data.mw_.pages_stack_->addWidget(page);
    page->set_on_back([&data]() {
      data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    });
    data.mw_.pages_stack_->setCurrentWidget(page);
  }


/// Returns a widget with [A−] [A+] buttons that change the application font.
  QWidget* make_font_buttons(QWidget* parent)
  {
    QWidget* w = new QWidget(parent);
    QHBoxLayout* hb = new QHBoxLayout(w);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->addStretch(1);
    auto* minus_btn = new QPushButton(QString::fromUtf8("A\xe2\x88\x92"), w);
    auto* plus_btn = new QPushButton("A+", w);
    minus_btn->setToolTip(
        QApplication::tr("Decrease font size (Ctrl+\xe2\x88\x92)"));
    plus_btn->setToolTip(
        QApplication::tr("Increase font size (Ctrl++)"));
    minus_btn->setFixedSize(36, 28);
    plus_btn->setFixedSize(36, 28);
    hb->addWidget(minus_btn);
    hb->addWidget(plus_btn);
    hb->addStretch(1);
    auto change = [](int delta) {
      int pt = qBound(8, QApplication::font().pointSize() + delta, 24);
      QFont f = QApplication::font();
      f.setPointSize(pt);
      QApplication::setFont(f);
      QSettings().setValue("font_size", pt);
    };
    QObject::connect(minus_btn, &QPushButton::clicked,
                     [change]() { change(-1); });
    QObject::connect(plus_btn, &QPushButton::clicked,
                     [change]() { change(+1); });
    return w;
  }


/// Minimal event filter that calls fn() on QEvent::ApplicationFontChange.
  class FontChangeFilter : public QObject
  {
  public:
    explicit FontChangeFilter(std::function<void()> fn, QObject* parent)
        : QObject(parent), fn_(std::move(fn)) {}
    bool eventFilter(QObject*, QEvent* e) override
    {
      if(e->type() == QEvent::ApplicationFontChange) {
        fn_();
      }
      return false;
    }

  private:
    std::function<void()> fn_;
  };


/// Sets all buttons to the same size (the widest and highest sizeHint).
  void equalize_button_sizes(std::vector<QToolButton*> const& btns)
  {
    int max_w = 0;
    int max_h = 0;
    for(auto* btn : btns) {
      btn->ensurePolished();
      auto const hint = btn->sizeHint();
      max_w = std::max(max_w, hint.width());
      max_h = std::max(max_h, hint.height());
    }
    for(auto* btn : btns) {
      btn->setFixedSize(max_w, max_h);
    }
  }


/// Builds the start-page UI programmatically inside start_page_.
/**
   Creates the two big buttons (open / new collection), the
   "recently opened:" label, and the recently-opened table widget.
   The three pointers are stored in @p data.
   @param data central application state.
*/
  void build_start_page_ui(colman::AppData& data)
  {
    QWidget* page = data.mw_.start_page_;
    QVBoxLayout* vbox = new QVBoxLayout(page);

  // --- logo ---
    vbox->addStretch(1);
    QLabel* logo = new QLabel(page);
    QPixmap const pm(":/icons/owls/colman_logo.jpeg");
    if(!pm.isNull()) {
      logo->setPixmap(pm.scaledToHeight(300, Qt::SmoothTransformation));
      logo->setAlignment(Qt::AlignHCenter);
    }
    vbox->addWidget(logo);

  // --- title + version ---
    colman::ScalingLabel* title = new colman::ScalingLabel(page, 6);
    title->setText("<b>ColMan</b> " COLMAN_VERSION);
    title->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(title);

  // --- subtitle ---
    QLabel* subtitle = new QLabel(
        QApplication::tr("the collection manager"), page);
    subtitle->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(subtitle);
    vbox->addStretch(1);

  // --- button row ---
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addItem(new QSpacerItem(40, 20,
                                  QSizePolicy::Expanding,
                                  QSizePolicy::Minimum));

    data.open_button_ = new colman::TileButton(page);
    data.open_button_->setObjectName("data.open_button_");
    data.open_button_->setToolTip(
        QApplication::tr("click here to open an existing collection!"));
    data.open_button_->setLayoutDirection(Qt::LeftToRight);
    data.open_button_->setText(
        QApplication::tr("open existing\ncollection"));
    data.open_button_->setIcon(QIcon(":/icons/owls/open_collection.jpeg"));
    data.open_button_->setIconSize(QSize(128, 128));
    data.open_button_->setShortcut(QString::fromUtf8("Ctrl+X, Ctrl+F"));
    data.open_button_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    data.open_button_->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(data.open_button_, 0, Qt::AlignTop);

    hbox->addItem(new QSpacerItem(80, 20,
                                  QSizePolicy::Fixed,
                                  QSizePolicy::Minimum));

    data.new_button_ = new colman::TileButton(page);
    data.new_button_->setObjectName("data.new_button_");
    data.new_button_->setToolTip(
        QApplication::tr("click here to create a new collection!"));
    data.new_button_->setText(
        QApplication::tr("create new\ncollection"));
    data.new_button_->setIcon(QIcon(":/icons/owls/new_collection.jpeg"));
    data.new_button_->setIconSize(QSize(128, 128));
    data.new_button_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    data.new_button_->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(data.new_button_, 0, Qt::AlignTop);

    hbox->addItem(new QSpacerItem(80, 20,
                                  QSizePolicy::Fixed,
                                  QSizePolicy::Minimum));

    data.exit_button_ = new colman::TileButton(page);
    data.exit_button_->setObjectName("data.exit_button_");
    data.exit_button_->setToolTip(
        QApplication::tr("click here to quit the program!"));
    data.exit_button_->setText(
        QApplication::tr("quit program"));
    data.exit_button_->setIcon(QIcon(":/icons/owls/exit2.jpeg"));
    data.exit_button_->setIconSize(QSize(128, 128));
    data.exit_button_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    data.exit_button_->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(data.exit_button_, 0, Qt::AlignTop);

    {
      std::vector<QToolButton*> eq_btns{
          data.open_button_, data.new_button_, data.exit_button_};
      equalize_button_sizes(eq_btns);
      auto* filt = new FontChangeFilter(
          [eq_btns]() { equalize_button_sizes(eq_btns); }, page);
      page->installEventFilter(filt);
    }

    hbox->addItem(new QSpacerItem(40, 20,
                                  QSizePolicy::Expanding,
                                  QSizePolicy::Minimum));
    vbox->addLayout(hbox);
    vbox->addSpacing(4);
    vbox->addWidget(make_font_buttons(page));

  // --- spacer between buttons and table ---
    vbox->addItem(new QSpacerItem(20, 20,
                                  QSizePolicy::Minimum,
                                  QSizePolicy::Fixed));

  // --- "recently opened:" label ---
    QLabel* label = new QLabel(
        QApplication::tr("recently opened:"), page);
    vbox->addWidget(label);

  // --- recently-opened table ---
    data.recently_opened_ = new QTableWidget(0, 3, page);
    data.recently_opened_->setObjectName("data.recently_opened_");
    data.recently_opened_->setAlternatingRowColors(true);
    data.recently_opened_->setSelectionMode(
        QAbstractItemView::SingleSelection);
    data.recently_opened_->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    data.recently_opened_->setSortingEnabled(true);
    data.recently_opened_->setWordWrap(true);
    data.recently_opened_->verticalHeader()->setVisible(false);
    QStringList headers;
    headers << QApplication::tr("collection")
            << QApplication::tr("filename")
            << QApplication::tr("last time opened");
    data.recently_opened_->setHorizontalHeaderLabels(headers);
    vbox->addWidget(data.recently_opened_);
  }


/// Page embedded in the stacked widget for Administrator login.
/**
   Displays the admin owl image and a password field.  Calls on_success_()
   when the correct administrator password is entered, on_back_() on Back.
   On a wrong password the input is locked for three seconds before the user
   may try again (same pattern as UserLoginPage).
*/
  class AdminLoginPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param queries    query interface; must outlive the page
     @param on_success called when the correct password is entered
     @param on_back    called when the Back button is pressed
     @param parent     optional parent widget
  */
    explicit AdminLoginPage(colman::IQueries& queries,
                            std::function<void()> on_success,
                            std::function<void()> on_back,
                            QWidget* parent = nullptr)
        : QWidget(parent),
          queries_(queries),
          on_success_(on_success),
          on_back_(on_back)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("Back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

    // icon + login label tight together
      QVBoxLayout* icon_vbox = new QVBoxLayout;
      icon_vbox->setSpacing(0);
      icon_vbox->setContentsMargins(0, 0, 0, 0);
      QLabel* img = new QLabel(this);
      QPixmap const pm(":/icons/owls/admin.jpeg");
      if(!pm.isNull()) {
        img->setPixmap(pm.scaledToHeight(120, Qt::SmoothTransformation));
        img->setAlignment(Qt::AlignHCenter);
        img->setFixedHeight(120);
      }
      icon_vbox->addWidget(img);
      QLabel* login_title = new QLabel("<b>Login</b>", this);
      login_title->setAlignment(Qt::AlignHCenter);
      icon_vbox->addWidget(login_title);
      vbox->addLayout(icon_vbox);

      QFormLayout* form = new QFormLayout;
      pw_edit_ = new QLineEdit(this);
      pw_edit_->setObjectName("login_password_");
      pw_edit_->setEchoMode(QLineEdit::Password);
      pw_edit_->setMinimumWidth(
          pw_edit_->fontMetrics().horizontalAdvance(
              QString(20, QChar(pw_edit_->style()->styleHint(
                              QStyle::SH_LineEdit_PasswordCharacter)))) +
          8);
      form->addRow(QApplication::tr("Password:"), pw_edit_);
      vbox->addLayout(form);

      error_label_ = new QLabel(QApplication::tr("Wrong password!"), this);
      error_label_->setStyleSheet("color: red;");
      error_label_->setAlignment(Qt::AlignHCenter);
      error_label_->setVisible(false);
      vbox->addWidget(error_label_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("Login"), this);
      ok_btn_->setObjectName("login_ok_");
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(pw_edit_, &QLineEdit::textChanged,
              [this]() {
                if(pw_edit_->isEnabled()) {
                  ok_btn_->setEnabled(!pw_edit_->text().isEmpty());
                }
              });
      connect(pw_edit_, &QLineEdit::returnPressed,
              [this]() { if(ok_btn_->isEnabled()){ try_login(); } });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() { try_login(); });
      connect(back_btn_, &QPushButton::clicked,
              [this]() { on_back_(); });
    }

  /// Prepares the page for a new login attempt.
  /**
     Resets the password field, error label, and button states, and focuses
     the password field.
  */
    void enter()
    {
      pw_edit_->setEnabled(true);
      pw_edit_->clear();
      ok_btn_->setEnabled(false);
      error_label_->setVisible(false);
      pw_edit_->setFocus();
    }

  private:
  /// Verifies the entered password; calls on_success_() or starts lockout.
    void try_login()
    {
      QString hash;
      bool const ok =
          queries_.get_password(0, hash) &&
          colman::verify_password(pw_edit_->text(), hash);
      if(ok) {
        on_success_();
      }
      else {
        error_label_->setVisible(true);
        pw_edit_->setEnabled(false);
        ok_btn_->setEnabled(false);
        QTimer::singleShot(3000, pw_edit_,
                           [this]() {
                             pw_edit_->setEnabled(true);
                             pw_edit_->clear();
                             pw_edit_->setFocus();
                             ok_btn_->setEnabled(false);
                             error_label_->setVisible(false);
                           });
      }
    }

    colman::IQueries& queries_;
    std::function<void()> on_success_;
    std::function<void()> on_back_;
    QLineEdit* pw_edit_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };


/// Page embedded in the stacked widget for regular-user login.
/**
   Shows a username field and a password field.  Calls on_success_() when
   check_local_credentials() returns a valid person id, on_back_() on Back.
   A wrong credential pair locks the input for three seconds (same pattern
   as AdminLoginPage).
*/
  class UserLoginPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param data       central application state
     @param on_success called when credentials are correct
     @param on_back    called when the Back button is pressed
     @param parent     optional parent widget
  */
    explicit UserLoginPage(colman::AppData& data,
                           std::function<void()> on_success,
                           std::function<void()> on_back,
                           QWidget* parent = nullptr)
        : QWidget(parent),
          data_(data),
          on_success_(on_success),
          on_back_(on_back)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("Back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

    // icon + login label tight together
      QVBoxLayout* icon_vbox = new QVBoxLayout;
      icon_vbox->setSpacing(0);
      icon_vbox->setContentsMargins(0, 0, 0, 0);
      QLabel* img = new QLabel(this);
      QPixmap const pm(":/icons/owls/login.jpeg");
      if(!pm.isNull()) {
        img->setPixmap(pm.scaledToHeight(120, Qt::SmoothTransformation));
        img->setAlignment(Qt::AlignHCenter);
        img->setFixedHeight(120);
      }
      icon_vbox->addWidget(img);
      QLabel* login_title = new QLabel("<b>Login</b>", this);
      login_title->setAlignment(Qt::AlignHCenter);
      icon_vbox->addWidget(login_title);
      vbox->addLayout(icon_vbox);

      QFormLayout* form = new QFormLayout;
      username_edit_ = new QLineEdit(this);
      username_edit_->setObjectName("login_username_");
      username_edit_->setMinimumWidth(
          username_edit_->fontMetrics().horizontalAdvance(
              QString(20, QChar('m'))) +
          8);
      form->addRow(QApplication::tr("Username or barcode:"), username_edit_);
      pw_edit_ = new QLineEdit(this);
      pw_edit_->setObjectName("login_password_");
      pw_edit_->setEchoMode(QLineEdit::Password);
      pw_edit_->setMinimumWidth(
          pw_edit_->fontMetrics().horizontalAdvance(
              QString(20, QChar(pw_edit_->style()->styleHint(
                              QStyle::SH_LineEdit_PasswordCharacter)))) +
          8);
      form->addRow(QApplication::tr("Password:"), pw_edit_);
      vbox->addLayout(form);

      error_label_ = new QLabel(
          QApplication::tr("Wrong username or password."), this);
      error_label_->setStyleSheet("color: red;");
      error_label_->setAlignment(Qt::AlignHCenter);
      error_label_->setVisible(false);
      vbox->addWidget(error_label_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("Login"), this);
      ok_btn_->setObjectName("login_ok_");
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(username_edit_, &QLineEdit::textChanged,
              [this]() { update_ok_btn(); });
      connect(pw_edit_, &QLineEdit::textChanged,
              [this]() { update_ok_btn(); });
      connect(pw_edit_, &QLineEdit::returnPressed,
              [this]() { if(ok_btn_->isEnabled()){ try_login(); } });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() { try_login(); });
      connect(back_btn_, &QPushButton::clicked,
              [this]() { on_back_(); });
    }

  /// Prepares the page for a new login attempt with \a prefill in the
  /// username/barcode field.
  /**
     Resets the password field, error label, and button states.
     Focuses the password field when \a prefill is non-empty, otherwise the
     username/barcode field.
  */
    void enter(QString const& prefill)
    {
      pw_edit_->clear();
      ok_btn_->setEnabled(false);
      error_label_->setVisible(false);
      username_edit_->setText(prefill);
      if(!prefill.isEmpty()) {
        pw_edit_->setFocus();
      }
      else {
        username_edit_->setFocus();
      }
    }

  /// Prepares the page for a new login attempt.
  /**
     Pre-fills the username from QSettings (last successful login).
     Resets the password field, error label, and button states.
  */
    void enter()
    {
      QSettings settings;
      enter(settings.value("last_user_name/" + data_.filename_).toString());
    }

  private:
  /// Enables the Login button when both fields are non-empty.
    void update_ok_btn()
    {
      ok_btn_->setEnabled(
          !username_edit_->text().isEmpty() &&
          !pw_edit_->text().isEmpty());
    }

  /// Verifies credentials; calls on_success_() or starts lockout.
  /**
     Accepts either a username or a person barcode in the first field.
     For a barcode the password is verified directly against the stored hash.
     For a username the existing check_local_credentials() path is used.
  */
    void try_login()
    {
      auto const user = username_edit_->text().trimmed();
      int pid = -1;
      bool ok = false;
      int barcode_pid = -1;
      if(data_.queries_->parse_person_barcode(user, barcode_pid)) {
        QString hash;
        ok = data_.queries_->get_password(barcode_pid, hash) &&
             colman::verify_password(pw_edit_->text(), hash);
        if(ok) {
          pid = barcode_pid;
        }
      }
      else {
        pid = data_.queries_->check_local_credentials(user, pw_edit_->text());
        if(pid > 0) {
          ok = true;
          QSettings settings;
          settings.setValue("last_user_name/" + data_.filename_, user);
        }
      }
      if(ok) {
        data_.current_data_collector_ = pid;
        on_success_();
      }
      else {
        error_label_->setVisible(true);
        username_edit_->setEnabled(false);
        pw_edit_->setEnabled(false);
        ok_btn_->setEnabled(false);
        QTimer::singleShot(3000, pw_edit_,
                           [this]() {
                             username_edit_->setEnabled(true);
                             pw_edit_->setEnabled(true);
                             pw_edit_->clear();
                             pw_edit_->setFocus();
                             ok_btn_->setEnabled(false);
                             error_label_->setVisible(false);
                           });
      }
    }

    colman::AppData& data_;
    std::function<void()> on_success_;
    std::function<void()> on_back_;
    QLineEdit* username_edit_ = nullptr;
    QLineEdit* pw_edit_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };


  struct RecentEntry
  {
    QString name_;
    QString fname_;
    QString datestr_;
  };

  /// QTableWidgetItem that sorts by an ISO date string stored in Qt::UserRole.
  class DateTableWidgetItem : public QTableWidgetItem
  {
  public:
    explicit DateTableWidgetItem(QDate const& date)
        : QTableWidgetItem(QLocale().toString(date, QLocale::ShortFormat))
    {
      setData(Qt::UserRole, date.toString("yyyyMMdd"));
      setFlags(flags() & ~Qt::ItemIsEditable);
    }
    bool operator<(QTableWidgetItem const& other) const override
    {
      return data(Qt::UserRole).toString() < other.data(Qt::UserRole).toString();
    }
  };

  std::vector<RecentEntry> load_recent_entries()
  {
    QSettings settings;
    QStringList const recently = settings.value("recently_opened").toStringList();
    std::vector<RecentEntry> result;
    for(auto it = recently.begin(); it != recently.end(); ++it) {
      RecentEntry e;
      e.name_ = *it++;
      if(it == recently.end()) {
        break;
      }
      e.fname_ = *it++;
      if(it == recently.end()) {
        break;
      }
      e.datestr_ = *it;
      result.push_back(e);
    }
    return result;
  }

  void save_recent_entries(std::vector<RecentEntry> const& entries)
  {
    QStringList list;
    for(auto const& e : entries) {
      list.push_back(e.name_);
      list.push_back(e.fname_);
      list.push_back(e.datestr_);
    }
    QSettings settings;
    settings.setValue("recently_opened", list);
  }

  /**
     Post-open setup: reads collection metadata, updates the recently-opened
     list, triggers all page setups on first call, switches to the central page,
     and updates the window title.
     @param app   Qt application object.
     @param data  central application state.
     @param fname absolute path of the just-opened database file.
  */
  void setup(QApplication& app, colman::AppData& data, QString const& fname)
  {
    colman::Collection collection = data.queries_->read_collection();
    QString const abs_fname = QFileInfo(fname).absoluteFilePath();
    if(!abs_fname.isEmpty()) {
      std::vector<RecentEntry> entries = load_recent_entries();
      auto it = std::find_if(
          entries.begin(), entries.end(),
          [&abs_fname](RecentEntry const& e) { return e.fname_ == abs_fname; });
      if(it != entries.end()) {
        it->name_ = collection.get_name();
        it->datestr_ = QDate::currentDate().toString("yyyyMMdd");
        RecentEntry e = std::move(*it);
        entries.erase(it);
        entries.insert(entries.begin(), std::move(e));
      }
      else {
        RecentEntry e;
        e.name_ = collection.get_name();
        e.fname_ = abs_fname;
        e.datestr_ = QDate::currentDate().toString("yyyyMMdd");
        entries.insert(entries.begin(), std::move(e));
        int constexpr MaxEntries = 10;
        if((int)entries.size() > MaxEntries) {
          entries.resize(MaxEntries);
        }
      }
      save_recent_entries(entries);
    }
    if(!app.property("not_first_start").toBool()) {
      colman::setup_history_page(app, data);
      colman::setup_person_page(app, data);
      colman::setup_person_view_page(app, data);
      colman::setup_person_barcode_page(app, data);
      colman::setup_item_barcode_page(app, data);
      colman::setup_item_type_page(app, data);
      colman::setup_camera_scan_page(app, data);
      colman::setup_acquisition_pages(app, data);
      colman::setup_liquidation_pages(app, data);
      colman::setup_lending_pages(app, data);
      colman::setup_group_lending_pages(app, data);
      colman::setup_return_pages(app, data);
      colman::setup_settings_page(app, data);
      colman::setup_search_page(app, data);
      colman::setup_class_management_pages(app, data);
      app.setProperty("not_first_start", true);
    }
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
    data.w_.setWindowTitle("colman - " + fname);
    data.filename_ = fname;

    // Show due-returns page for admin and view_persons users.
    bool const can_view =
        (data.current_data_collector_ == 0) ||
        data.queries_->has_permission(
            data.current_data_collector_, "view_persons");
    if(can_view) {
      QDate const today = QDate::currentDate();
      QDate to_date = today;
      if(today.dayOfWeek() == Qt::Friday) {
        to_date = today.addDays(2);
      }
      auto rows =
          data.queries_->query_due_lendings(today, to_date);
      if(!rows.empty()) {
        show_due_lendings_page(data, std::move(rows));
      }
    }
  }


  /// Page that asks for the database password when opening an encrypted
  /// collection.
  /**
     Shows an informational label and a password field.  The OK button is
     disabled until the field is non-empty.  Holds the collection's lock file
     while the user enters the password; the lock is moved into AppData on a
     successful unlock.  Calls on_success_() once the database is unlocked, or
     on_cancel_() when the user presses Back or fails three times.
  */
  class DbPasswordPage : public QWidget
  {
  public:
  /// Constructs the page.
  /**
     @param data  central application state
     @param fname path of the encrypted collection to open
     @param lock  the acquired lock file, held until the unlock succeeds
     @param parent optional parent widget
  */
    explicit DbPasswordPage(colman::AppData& data,
                            QString const& fname,
                            std::unique_ptr<QLockFile> lock,
                            QWidget* parent = nullptr)
        : QWidget(parent),
          data_(data),
          fname_(fname),
          lock_(std::move(lock))
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("Back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

      QLabel* info = new QLabel(
          QApplication::tr("Enter the database password:"), this);
      info->setWordWrap(true);
      info->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(info);

      QFormLayout* form = new QFormLayout;
      pw_edit_ = new QLineEdit(this);
      pw_edit_->setObjectName("db_password_");
      pw_edit_->setEchoMode(QLineEdit::Password);
      pw_edit_->setMinimumWidth(
          pw_edit_->fontMetrics().horizontalAdvance(
              QString(20, QChar(pw_edit_->style()->styleHint(
                              QStyle::SH_LineEdit_PasswordCharacter)))) +
          8);
      form->addRow(QApplication::tr("Password:"), pw_edit_);
      vbox->addLayout(form);

      error_label_ = new QLabel(
          QApplication::tr("Wrong password or database error."), this);
      error_label_->setStyleSheet("color: red;");
      error_label_->setAlignment(Qt::AlignHCenter);
      error_label_->setVisible(false);
      vbox->addWidget(error_label_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("OK"), this);
      ok_btn_->setObjectName("db_password_ok_");
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(pw_edit_, &QLineEdit::textChanged,
              [this](QString const& t) {
                ok_btn_->setEnabled(!t.isEmpty());
              });
      connect(pw_edit_, &QLineEdit::returnPressed,
              [this]() {
                if(!pw_edit_->text().isEmpty()) {
                  try_unlock();
                }
              });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() { try_unlock(); });
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_cancel_) {
                  on_cancel_();
                }
              });
    }

  /// Sets the callback invoked once the database is successfully unlocked.
    void set_on_success(std::function<void()> on_success)
    {
      on_success_ = std::move(on_success);
    }

  /// Sets the callback invoked on Back or after three failed attempts.
    void set_on_cancel(std::function<void()> on_cancel)
    {
      on_cancel_ = std::move(on_cancel);
    }

  /// Focuses the password field for a fresh attempt.
    void enter()
    {
      pw_edit_->setFocus();
    }

  private:
  /// Tries to unlock the database; reports success or shows the error.
    void try_unlock()
    {
      if(data_.queries_->load_database(fname_, pw_edit_->text())) {
        data_.collection_lock_ = std::move(lock_);
        if(on_success_) {
          on_success_();
        }
        return;
      }
      error_label_->setVisible(true);
      pw_edit_->clear();
      pw_edit_->setFocus();
      if(++attempts_ >= MaxTries && on_cancel_) {
        on_cancel_();
      }
    }

    static int constexpr MaxTries = 3;
    colman::AppData& data_;
    QString fname_;
    std::unique_ptr<QLockFile> lock_;
    std::function<void()> on_success_;
    std::function<void()> on_cancel_;
    QLineEdit* pw_edit_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
    int attempts_ = 0;
  };


  /**
     Repopulates the recently-opened table from QSettings.
     Removes entries for files that no longer exist and updates the
     stored list accordingly.
  */
  void setup_recently(colman::AppData& data)
  {
    data.recently_opened_->setSortingEnabled(false);
    data.recently_opened_->setRowCount(0);
    int row = 0;
    std::vector<RecentEntry> entries = load_recent_entries();
    std::vector<RecentEntry> valid;
    for(auto const& e : entries) {
      if(!e.fname_.isEmpty() && QFileInfo(e.fname_).isAbsolute() && QFileInfo::exists(e.fname_)) {
        data.recently_opened_->setRowCount(row + 1);
        QDate const date = QDate::fromString(e.datestr_, "yyyyMMdd");
        auto* name_item = new QTableWidgetItem(e.name_);
        name_item->setFlags(name_item->flags() & ~Qt::ItemIsEditable);
        data.recently_opened_->setItem(row, 0, name_item);
        auto* fname_item = new QTableWidgetItem(e.fname_);
        fname_item->setFlags(fname_item->flags() & ~Qt::ItemIsEditable);
        data.recently_opened_->setItem(row, 1, fname_item);
        data.recently_opened_->setItem(row, 2, new DateTableWidgetItem(date));
        valid.push_back(e);
        ++row;
      }
    }
    if(valid.size() < entries.size()) {
      save_recent_entries(valid);
    }
    data.recently_opened_->setSortingEnabled(true);
    data.recently_opened_->sortByColumn(2, Qt::DescendingOrder);
  }


  /**
     Closes the current collection: switches to the start page, closes the db,
     resets the window title, and discards lazily created editor pages.
     @param data central application state.
  */
  void close_collection(colman::AppData& data)
  {
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.start_page_);
    data.w_.setWindowTitle("colman");
    data.filename_.clear();
    data.current_data_collector_ = -1;
    colman::update_login_status(data);
    data.collection_lock_.reset();
    data.clear_lazy_editors();
    setup_recently(data);
  }


  /// Completes opening after the database has been loaded and locked.
  /**
     Stores the filename and either shows the login selection page (so a user
     can authenticate against the open collection) or, if there is none, sets
     the app up directly.  The collection lock must already be stored in
     data.collection_lock_.
     @param app   Qt application object.
     @param data  central application state.
     @param fname absolute path of the opened database file.
  */
  /// Copies the legacy per-collection element arrangements into the
  /// matching named QSettings layouts (one-time, idempotent).
  /**
     Before layouts bundled their element arrangement, each collection held
     a single ID/item/signature arrangement plus the name of the chosen
     sheet layout.  This moves that arrangement into the user layout of that
     name when the layout does not already carry one, so existing setups are
     preserved.  Filled layouts are skipped, so re-running is harmless.
  */
  void migrate_card_layouts(colman::IQueries& queries)
  {
    colman::Collection const col = queries.read_collection();
    QList<colman::LabelLayoutPreset> users =
        colman::LabelLayoutWidget::load_user_layouts();
    auto find = [&](QString const& name) -> int {
      for(int i = 0; i < users.size(); ++i) {
        if(users[i].name == name) {
          return i;
        }
      }
      return -1;
    };

    colman::IdCardLayout const& id_arr = col.get_id_card_layout();
    QString const id_name = queries.read_id_label_layout_name();
    if(id_arr.card_width_mm_ > 0.0 && !id_name.isEmpty()) {
      int const i = find(id_name);
      if(i >= 0 && users[i].card_arrangement.card_width_mm_ <= 0.0) {
        users[i].card_arrangement = id_arr;
        colman::LabelLayoutWidget::save_user_layout(users[i]);
      }
    }

    colman::IdCardLayout const& it_arr = col.get_item_card_layout();
    QString const it_name = queries.read_item_card_label_layout_name();
    if(it_arr.card_width_mm_ > 0.0 && !it_name.isEmpty()) {
      int const i = find(it_name);
      if(i >= 0 && users[i].card_arrangement.card_width_mm_ <= 0.0) {
        users[i].card_arrangement = it_arr;
        colman::LabelLayoutWidget::save_user_layout(users[i]);
      }
    }

    colman::SignatureLabelLayout const& sig_arr = col.get_sig_label_layout();
    QString const sig_name = sig_arr.label_layout_name_;
    if(sig_arr.card_width_mm_ > 0.0 && !sig_name.isEmpty()) {
      int const i = find(sig_name);
      if(i >= 0 && users[i].sig_arrangement.card_width_mm_ <= 0.0) {
        users[i].sig_arrangement = sig_arr;
        colman::LabelLayoutWidget::save_user_layout(users[i]);
      }
    }

    // The last-used sheet name used to live in the database; seed the local
    // QSettings value from it once so the print dialogs keep their default.
    if(colman::LabelLayoutWidget::last_selected_layout_name().isEmpty()) {
      QString const last = !id_name.isEmpty() ? id_name : it_name;
      if(!last.isEmpty()) {
        colman::LabelLayoutWidget::set_last_selected_layout_name(last);
      }
    }
  }

  void finish_open(QApplication& app, colman::AppData& data,
                   QString const& fname)
  {
    // Store the filename early so the login page's admin handler can use it.
    data.filename_ = fname;
    // Bring forward any pre-existing element arrangement into its layout.
    migrate_card_layouts(*data.queries_);
    QWidget* login_page =
        data.mw_.pages_stack_->findChild<QWidget*>(
            "login_selection_page_", Qt::FindDirectChildrenOnly);
    if((login_page)) {
      QToolButton* lb =
          login_page->findChild<QToolButton*>(
              "login_btn_", Qt::FindDirectChildrenOnly);
      if((lb)) {
        lb->setVisible(data.queries_->has_non_admin_persons());
      }
      data.mw_.pages_stack_->setCurrentWidget(login_page);
      auto* be =
          login_page->findChild<QLineEdit*>("login_barcode_edit_");
      if((be)) {
        be->clear();
        be->setFocus();
      }
    }
    else {
      setup(app, data, fname);
    }
  }

  /**
     Closes any open collection and opens the database at @p fname.

     For an encrypted collection the password is requested on a stacked page,
     so the result is reported asynchronously: @p on_done is invoked with true
     on success and false on failure or cancellation.  For an unencrypted
     collection @p on_done is invoked synchronously before returning.
     @param app     Qt application object.
     @param data    central application state.
     @param fname   absolute path of the database file to open.
     @param on_done callback receiving the open result.
  */
  void open_collection(QApplication& app, colman::AppData& data,
                       QString const& fname,
                       std::function<void(bool)> on_done)
  {
    close_collection(data);

    auto lock = std::make_unique<QLockFile>(fname + ".lock");
    lock->setStaleLockTime(0);
    if(!lock->tryLock()) {
      QLockFile::LockError const err = lock->error();
      if(err == QLockFile::LockFailedError) {
        QMessageBox::warning(
            &data.w_,
            QApplication::tr("Collection Manager"),
            QApplication::tr(
                "The collection is already open in another "
                "instance of colman. "
                "Please close it there first."));
        on_done(false);
        return;
      }
      if(err == QLockFile::PermissionError) {
        QMessageBox::warning(
            &data.w_,
            QApplication::tr("Collection Manager"),
            QApplication::tr(
                "Could not create a lock file for this "
                "collection (read-only or network location). "
                "Opening anyway, but concurrent access is "
                "not protected."));
      }
      // UnknownError: fall through
    }

    if(colman::is_file_encrypted(fname)) {
      if(!data.queries_->supports_encryption()) {
        QMessageBox::warning(
            &data.w_,
            QApplication::tr("Collection Manager"),
            QApplication::tr(
                "This collection is encrypted, but the SQLite "
                "cipher plugin is not installed. Please install "
                "the SQLITE_MULTIPLE_CIPHERS Qt SQL driver."));
        on_done(false);
        return;
      }
      QWidget* const back = data.mw_.pages_stack_->currentWidget();
      auto* page = new DbPasswordPage(data, fname, std::move(lock));
      data.mw_.pages_stack_->addWidget(page);
      page->set_on_success([&app, &data, fname, page, on_done]() {
        // The lock has already been moved into data.collection_lock_.
        finish_open(app, data, fname);
        data.mw_.pages_stack_->removeWidget(page);
        page->deleteLater();
        if(on_done) {
          on_done(true);
        }
      });
      page->set_on_cancel([&data, back, page, on_done]() {
        data.mw_.pages_stack_->setCurrentWidget(back);
        data.mw_.pages_stack_->removeWidget(page);
        page->deleteLater();
        if(on_done) {
          on_done(false);
        }
      });
      data.mw_.pages_stack_->setCurrentWidget(page);
      page->enter();
      return;
    }

    if(!data.queries_->load_database(fname)) {
      on_done(false);
      return;
    }
    data.collection_lock_ = std::move(lock);
    finish_open(app, data, fname);
    on_done(true);
  }


  /**
     Shows a file-open dialog and opens the selected collection.
     Restores/saves the dialog state via QSettings.
     @param app  Qt application object.
     @param data central application state.
  */
  void open(QApplication& app, colman::AppData& data)
  {
    QSettings settings;
    QByteArray state = settings.value("open/state").toByteArray();
    QFileDialog diag(nullptr, QApplication::tr("Open Collection"), "");
    QStringList filters;
    filters << QApplication::tr("collection files (*.col *.db)")
            << QApplication::tr("any files (*)");
    diag.setNameFilters(filters);
    diag.setFileMode(QFileDialog::ExistingFile);
    (void)diag.restoreState(state);
    if(diag.exec()) {
      QStringList file_names = diag.selectedFiles();
      QByteArray const state = diag.saveState();
      settings.setValue("open/state", state);
      if(!file_names.empty()) {
        QString const fname = file_names.front();
        open_collection(app, data, fname, [&data, fname](bool ok) {
          if(!ok) {
            QMessageBox::warning(
                data.mw_.start_page_,
                QApplication::tr("Collection Manager"),
                QApplication::tr("Could not read collection file."));
          }
          else {
            data.w_.setWindowTitle("colman - " + fname);
          }
        });
      }
    }
  }


  /**
     Closes the current collection and starts the new-collection step flow.
     On completion the newly created database is locked and opened.
     @param app  Qt application object.
     @param data central application state.
  */
  void new_collection(QApplication& app, colman::AppData& data)
  {
    close_collection(data);
    colman::enter_create_collection_flow(
        data,
        [&app, &data](QString const& new_fname) {
          auto new_lock = std::make_unique<QLockFile>(new_fname + ".lock");
          new_lock->setStaleLockTime(0);
          if(!new_lock->tryLock() &&
             new_lock->error() == QLockFile::LockFailedError) {
            QMessageBox::warning(
                &data.w_,
                QApplication::tr("Collection Manager"),
                QApplication::tr(
                    "The collection is already open in another "
                    "instance of colman. "
                    "Please close it there first."));
            return;
          }
          data.collection_lock_ = std::move(new_lock);
          data.current_data_collector_ = 0;
          setup(app, data, new_fname);
          colman::update_login_status(data);
        });
  }


  /**
     Removes @p file from the recently-opened list in QSettings and
     refreshes the recently-opened table widget.
     @param data central application state.
     @param file absolute file path to remove.
  */
  void remove_from_recently(colman::AppData& data, QString const& file)
  {
    std::vector<RecentEntry> entries = load_recent_entries();
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [&file](RecentEntry const& e) { return e.fname_ == file; }),
        entries.end());
    save_recent_entries(entries);
    setup_recently(data);
  }

} // namespace


/**
   Initializes the start page; to be called once at program start.
   Sets up the recently-opened table, reads the mode setting, connects all
   start-page buttons and menu actions (open, new, close, mode),
   and opens the file passed on the command line (if any).
   @param app  Qt application object.
   @param data central application state.
   @param collection_file collection file to open (may be empty).
*/
void colman::setup_start_page(QApplication& app, colman::AppData& data,
                              QString const& collection_file)
{
  build_start_page_ui(data);
  data.recently_opened_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  setup_recently(data);

  // --- login selection page (shown after loading an existing collection) ---
  QWidget* login_page = new QWidget;
  login_page->setObjectName("login_selection_page_");
  QVBoxLayout* lp_vbox = new QVBoxLayout(login_page);
  lp_vbox->addStretch(1);
  QHBoxLayout* lp_hbox = new QHBoxLayout;
  lp_hbox->addStretch(1);

  QToolButton* admin_btn = new colman::TileButton(login_page);
  admin_btn->setObjectName("admin_login_btn_");
  admin_btn->setText(QApplication::tr("Administrator Login"));
  admin_btn->setIcon(QIcon(":/icons/owls/admin.jpeg"));
  admin_btn->setIconSize(QSize(128, 128));
  admin_btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  admin_btn->setFocusPolicy(Qt::NoFocus);
  lp_hbox->addWidget(admin_btn, 0, Qt::AlignTop);

  lp_hbox->addStretch(1);

  QToolButton* login_btn = new colman::TileButton(login_page);
  login_btn->setObjectName("login_btn_");
  login_btn->setText(QApplication::tr("User Login"));
  login_btn->setIcon(QIcon(":/icons/owls/login.jpeg"));
  login_btn->setIconSize(QSize(128, 128));
  login_btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  login_btn->setFocusPolicy(Qt::NoFocus);
  lp_hbox->addWidget(login_btn, 0, Qt::AlignTop);

  lp_hbox->addStretch(1);

  QToolButton* close_btn = new colman::TileButton(login_page);
  close_btn->setObjectName("close_collection_btn_");
  close_btn->setText(QApplication::tr("close collection"));
  close_btn->setIcon(QIcon(":/icons/owls/logout.jpeg"));
  close_btn->setIconSize(QSize(128, 128));
  close_btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  close_btn->setFocusPolicy(Qt::NoFocus);
  lp_hbox->addWidget(close_btn, 0, Qt::AlignTop);

  {
    std::vector<QToolButton*> eq_btns{admin_btn, login_btn, close_btn};
    equalize_button_sizes(eq_btns);
    auto* filt = new FontChangeFilter(
        [eq_btns]() { equalize_button_sizes(eq_btns); }, login_page);
    login_page->installEventFilter(filt);
  }

  lp_hbox->addStretch(1);
  lp_vbox->addLayout(lp_hbox);
  lp_vbox->addSpacing(16);

  QHBoxLayout* scan_hbox = new QHBoxLayout;
  scan_hbox->addStretch(1);
  QLabel* scan_label =
      new QLabel(QApplication::tr("or scan ID card:"), login_page);
  scan_hbox->addWidget(scan_label);
  QLineEdit* barcode_edit = new QLineEdit(login_page);
  barcode_edit->setObjectName("login_barcode_edit_");
  barcode_edit->setPlaceholderText(QApplication::tr("Scan ID card..."));
  barcode_edit->setMinimumWidth(
      barcode_edit->fontMetrics().horizontalAdvance(QString(20, QChar('m'))) +
      8);
  scan_hbox->addWidget(barcode_edit);
  scan_hbox->addStretch(1);
  lp_vbox->addLayout(scan_hbox);
  lp_vbox->addSpacing(8);

  lp_vbox->addWidget(make_font_buttons(login_page));
  lp_vbox->addStretch(1);
  data.mw_.pages_stack_->addWidget(login_page);

  // --- admin login page ---
  AdminLoginPage* admin_login_page =
      new AdminLoginPage(
          *data.queries_,
          [&app, &data]() {
            data.current_data_collector_ = 0;
            setup(app, data, data.filename_);
            colman::update_login_status(data);
          },
          [&data, login_page]() {
            data.mw_.pages_stack_->setCurrentWidget(login_page);
          });
  data.mw_.pages_stack_->addWidget(admin_login_page);

  app.connect(admin_btn, &QToolButton::clicked,
              [&data, admin_login_page]() {
                admin_login_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(admin_login_page);
              });
  app.connect(close_btn, &QToolButton::clicked,
              [&data]() {
                close_collection(data);
              });

  // --- user login page ---
  UserLoginPage* user_login_page =
      new UserLoginPage(
          data,
          [&app, &data]() {
            setup(app, data, data.filename_);
            colman::update_login_status(data);
          },
          [&data, login_page, barcode_edit]() {
            data.mw_.pages_stack_->setCurrentWidget(login_page);
            barcode_edit->clear();
            barcode_edit->setFocus();
          });
  data.mw_.pages_stack_->addWidget(user_login_page);

  app.connect(login_btn, &QToolButton::clicked,
              [&data, user_login_page]() {
                user_login_page->enter();
                data.mw_.pages_stack_->setCurrentWidget(user_login_page);
              });

  app.connect(barcode_edit, &QLineEdit::returnPressed,
              [&data, barcode_edit, user_login_page, admin_login_page]() {
                auto const bc = barcode_edit->text().trimmed();
                barcode_edit->clear();
                if(bc.isEmpty()) {
                  return;
                }
                int pid = -1;
                if(!data.queries_->parse_person_barcode(bc, pid)) {
                  return;
                }
                if(pid == 0) {
                  admin_login_page->enter();
                  data.mw_.pages_stack_->setCurrentWidget(admin_login_page);
                }
                else {
                  user_login_page->enter(bc);
                  data.mw_.pages_stack_->setCurrentWidget(user_login_page);
                }
              });
  // ---

  // --- dynamic central page ---
  colman::setup_central_page(app, data);
  // ---

  QSettings settings;
  app.connect(data.open_button_, &QPushButton::clicked,
              [&app, &data]() {
                open(app, data);
              });
  app.connect(data.mw_.action_open_, &QAction::triggered,
              [&app, &data]() {
                open(app, data);
              });
  app.connect(data.mw_.action_close_, &QAction::triggered,
              [&data]() {
                close_collection(data);
              });
  app.connect(data.mw_.action_new_, &QAction::triggered,
              [&app, &data]() {
                new_collection(app, data);
              });
  app.connect(data.recently_opened_, &QTableWidget::cellDoubleClicked,
              [&app, &data](int row, int) {
                QTableWidgetItem* item = data.recently_opened_->item(row, 1);
                if((item)) {
                  QString const name = item->text();
                  open_collection(app, data, name,
                                  [&data, name](bool ok) {
                                    if(!ok && !QFileInfo::exists(name)) {
                                      remove_from_recently(data, name);
                                    }
                                  });
                }
              });
  app.connect(data.new_button_, &QPushButton::clicked,
              [&app, &data]() {
                new_collection(app, data);
              });
  app.connect(data.exit_button_, &QPushButton::clicked,
              [&app, &data]() {
                if(colman::question_yes_no(
                       &data.w_,
                       QApplication::tr("Collection Manager"),
                       QApplication::tr("Really quit the program?")) == QMessageBox::Yes) {
                  app.quit();
                }
              });
  if(!collection_file.isEmpty()) {
    QString const fname = collection_file;
    open_collection(app, data, fname, [&data, fname](bool ok) {
      if(ok) {
        data.w_.setWindowTitle("colman - " + fname);
      }
    });
  }
}


/**
   Logs out the current user: resets current_data_collector_ to -1,
   clears the status bar, and switches to the login selection page so
   a different user can log in to the same open collection.
   @param data central application state.
*/
void colman::logout(colman::AppData& data)
{
  data.current_data_collector_ = -1;
  colman::update_login_status(data);
  QWidget* page =
      data.mw_.pages_stack_->findChild<QWidget*>(
          "login_rest_page_", Qt::FindDirectChildrenOnly);
  if(!page) {
    page = data.mw_.pages_stack_->findChild<QWidget*>(
        "login_selection_page_", Qt::FindDirectChildrenOnly);
  }
  if(page) {
    data.mw_.pages_stack_->setCurrentWidget(page);
  }
}

/* EOF */
