// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of the class management pages.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_class_management_page.hpp"

#ifndef COLMAN_CLASS_NAMES_HPP
#  include "colman_class_names.hpp"
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef COLMAN_TRY_CATCH_HPP
#  include "colman_try_catch.hpp"
#endif

#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QLISTWIDGET
#  include <QListWidget>
#  define SYSTEM_QLISTWIDGET
#endif
#ifndef SYSTEM_QTABLEWIDGET
#  include <QTableWidget>
#  define SYSTEM_QTABLEWIDGET
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QCHECKBOX
#  include <QCheckBox>
#  define SYSTEM_QCHECKBOX
#endif
#ifndef SYSTEM_QINPUTDIALOG
#  include <QInputDialog>
#  define SYSTEM_QINPUTDIALOG
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  define SYSTEM_QREGULAREXPRESSION
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QSTRINGLIST
#  include <QStringList>
#  define SYSTEM_QSTRINGLIST
#endif
#ifndef SYSTEM_QHEADERVIEW
#  include <QHeaderView>
#  define SYSTEM_QHEADERVIEW
#endif

namespace
{

/// Page 3: review affected persons and execute school-year changes.
  class NewSchoolYearReviewPage : public QWidget
  {
    Q_OBJECT
  public:
    explicit NewSchoolYearReviewPage(colman::AppData& data,
                                     QWidget* parent = nullptr);

  /// Sets the page-2 widget for back-navigation.
    void set_school_year_page(QWidget* p) { school_year_page_ = p; }

  /// Populates the review table.
  /**
     @param old_to_new  maps old class name → new class name (empty = delete)
     @param extra_classes  brand-new class names to add to the master list
  */
    void enter(QMap<QString, QString> const& old_to_new,
               QStringList const& extra_classes);

  private Q_SLOTS:
    void on_execute_();
    void on_back_();

  private:
    colman::AppData& data_;
    QWidget* school_year_page_ = nullptr;
    QTableWidget* table_ = nullptr;
    QCheckBox* delete_empty_cb_ = nullptr;
    QMap<QString, QString> old_to_new_;
    QStringList extra_classes_;
    QList<colman::IQueries::AffectedPerson> persons_;
  };

/// Page 2: plan the school-year class transitions.
  class NewSchoolYearPage : public QWidget
  {
    Q_OBJECT
  public:
    explicit NewSchoolYearPage(colman::AppData& data,
                               NewSchoolYearReviewPage* review_page,
                               QWidget* parent = nullptr);

  /// Populates the transition table.
    void enter(QStringList const& classes, int grade_min, int grade_max);

  private Q_SLOTS:
    void on_apply_();
    void on_back_();
    void on_add_new_class_();

  private:
    colman::AppData& data_;
    NewSchoolYearReviewPage* review_page_ = nullptr;
    QTableWidget* table_ = nullptr;
    int grade_min_ = 1;
    int grade_max_ = 13;
  };

/// Page 1: manage the current class list and grade range.
  class ClassManagementMainPage : public QWidget
  {
    Q_OBJECT
  public:
    explicit ClassManagementMainPage(colman::AppData& data,
                                     NewSchoolYearPage* school_year_page,
                                     QWidget* parent = nullptr);

  /// Reloads the grade range and class list from the database.
    void enter();

  private Q_SLOTS:
    void on_save_grade_range_();
    void on_add_();
    void on_remove_();
    void on_rename_();
    void on_new_school_year_();
    void on_back_();
    void on_selection_changed_();

  private:
    void reload_classes_();

    colman::AppData& data_;
    NewSchoolYearPage* school_year_page_ = nullptr;
    QSpinBox* min_spin_ = nullptr;
    QSpinBox* max_spin_ = nullptr;
    QListWidget* class_list_ = nullptr;
    QPushButton* remove_btn_ = nullptr;
    QPushButton* rename_btn_ = nullptr;
  };

// ---------------------------------------------------------------------------
// NewSchoolYearReviewPage
// ---------------------------------------------------------------------------

  NewSchoolYearReviewPage::NewSchoolYearReviewPage(
      colman::AppData& data, QWidget* parent)
      : QWidget(parent), data_(data)
  {
    auto* vlay = new QVBoxLayout(this);

    auto* back_btn = new QPushButton(tr("back"), this);
    back_btn->setObjectName("class_back_");
    vlay->addWidget(back_btn);

    vlay->addWidget(new QLabel(
        QString("<b>%1</b>").arg(tr("Review and Execute")), this));

    delete_empty_cb_ = new QCheckBox(
        tr("Delete persons without a new class"), this);
    vlay->addWidget(delete_empty_cb_);

    table_ = new QTableWidget(0, 3, this);
    table_->setHorizontalHeaderLabels(
        {tr("Person"), tr("Current class"), tr("New class")});
    table_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vlay->addWidget(table_, 1);

    auto* execute_btn = new QPushButton(tr("Execute"), this);
    vlay->addWidget(execute_btn);

    connect(back_btn, &QPushButton::clicked,
            this, &NewSchoolYearReviewPage::on_back_);
    connect(execute_btn, &QPushButton::clicked,
            this, &NewSchoolYearReviewPage::on_execute_);
  }

  void NewSchoolYearReviewPage::enter(
      QMap<QString, QString> const& old_to_new,
      QStringList const& extra_classes)
  {
    old_to_new_ = old_to_new;
    extra_classes_ = extra_classes;
    auto raw = data_.queries_->get_affected_persons(old_to_new);
    persons_ = QList<colman::IQueries::AffectedPerson>(
        raw.begin(), raw.end());
    table_->setRowCount(0);

    QString current_class;
    for(int i = 0; i < persons_.size(); ++i) {
      colman::IQueries::AffectedPerson const& ap = persons_[i];
      if(ap.old_class_ != current_class) {
        current_class = ap.old_class_;
        int const hr = table_->rowCount();
        table_->insertRow(hr);
        auto* h0 = new QTableWidgetItem(
            QString("--- %1 ---").arg(current_class));
        h0->setFlags(Qt::ItemIsEnabled);
        QFont f = h0->font();
        f.setBold(true);
        h0->setFont(f);
        QColor const mid = QApplication::palette().color(QPalette::Mid);
        h0->setBackground(mid);
        table_->setItem(hr, 0, h0);
        for(int c = 1; c < 3; ++c) {
          auto* hc = new QTableWidgetItem();
          hc->setFlags(Qt::ItemIsEnabled);
          hc->setBackground(mid);
          table_->setItem(hr, c, hc);
        }
      }
      int const row = table_->rowCount();
      table_->insertRow(row);
      auto* name_item = new QTableWidgetItem(ap.display_name_);
      name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      name_item->setData(Qt::UserRole, ap.person_id_);
      table_->setItem(row, 0, name_item);
      auto* old_item = new QTableWidgetItem(ap.old_class_);
      old_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      table_->setItem(row, 1, old_item);
      auto* new_item = new QTableWidgetItem(ap.new_class_);
      new_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
      table_->setItem(row, 2, new_item);
    }
  }

  void NewSchoolYearReviewPage::on_execute_()
  {
    COLMAN_TRY
    {
      {
        QMap<int, QString> pid_to_new;
        QList<int> to_delete;
        bool const del_empty = delete_empty_cb_->isChecked();

        for(int row = 0; row < table_->rowCount(); ++row) {
          QTableWidgetItem const* name_item = table_->item(row, 0);
          if(!name_item) {
            continue;
          }
          if(!name_item->flags().testFlag(Qt::ItemIsSelectable)) {
            continue; // header row
          }
          int const pid = name_item->data(Qt::UserRole).toInt();
          QTableWidgetItem const* new_item = table_->item(row, 2);
          QString const new_cls = new_item ? new_item->text() : QString();
          pid_to_new[pid] = new_cls;
          if(new_cls.isEmpty() && del_empty) {
            to_delete.append(pid);
          }
        }

        if(!to_delete.isEmpty()) {
          QStringList blocked;
          for(int pid : to_delete) {
            auto ol = data_.queries_->open_lendings_for_person(pid);
            if(!ol.empty()) {
              for(int row = 0; row < table_->rowCount(); ++row) {
                QTableWidgetItem const* ni = table_->item(row, 0);
                if(ni && ni->data(Qt::UserRole).toInt() == pid) {
                  blocked.append(ni->text());
                  break;
                }
              }
            }
          }
          if(!blocked.isEmpty()) {
            QMessageBox::warning(
                this,
                tr("Warning: open lendings"),
                tr("The following persons have open lendings and"
                   " cannot be deleted:\n") +
                    blocked.join("\n"));
            return;
          }
        }

        QSet<QString> new_names_set;
        for(QString const& v : old_to_new_.values()) {
          if(!v.isEmpty()) {
            new_names_set.insert(v);
          }
        }
        for(QString const& v : pid_to_new.values()) {
          if(!v.isEmpty()) {
            new_names_set.insert(v);
          }
        }
        for(QString const& ec : extra_classes_) {
          if(!ec.isEmpty()) {
            new_names_set.insert(ec);
          }
        }
        QStringList new_class_list = new_names_set.values();
        new_class_list.sort();

        if(!data_.queries_->apply_school_year_changes(
               pid_to_new, to_delete, new_class_list)) {
          QMessageBox::critical(this, tr("Error"),
                                tr("Could not apply school year changes."));
          return;
        }
        colman::enter_class_management_page(data_);
      }
    }
    COLMAN_CATCH;
  }

  void NewSchoolYearReviewPage::on_back_()
  {
    if(school_year_page_) {
      data_.mw_.pages_stack_->setCurrentWidget(school_year_page_);
    }
  }

// ---------------------------------------------------------------------------
// NewSchoolYearPage
// ---------------------------------------------------------------------------

  NewSchoolYearPage::NewSchoolYearPage(
      colman::AppData& data,
      NewSchoolYearReviewPage* review_page,
      QWidget* parent)
      : QWidget(parent), data_(data), review_page_(review_page)
  {
    auto* vlay = new QVBoxLayout(this);

    auto* back_btn = new QPushButton(tr("back"), this);
    back_btn->setObjectName("class_back_");
    vlay->addWidget(back_btn);

    vlay->addWidget(new QLabel(
        QString("<b>%1</b>").arg(tr("New School Year Planning")), this));

    table_ = new QTableWidget(0, 2, this);
    table_->setHorizontalHeaderLabels(
        {tr("Current name"), tr("New name")});
    table_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vlay->addWidget(table_, 1);

    {
      auto* hlay = new QHBoxLayout();
      auto* add_class_btn = new QPushButton(tr("Add new class"), this);
      hlay->addWidget(add_class_btn);
      hlay->addStretch();
      auto* apply_btn = new QPushButton(tr("Apply"), this);
      apply_btn->setObjectName("new_school_year_apply_");
      hlay->addWidget(apply_btn);
      vlay->addLayout(hlay);
      connect(add_class_btn, &QPushButton::clicked,
              this, &NewSchoolYearPage::on_add_new_class_);
      connect(back_btn, &QPushButton::clicked,
              this, &NewSchoolYearPage::on_back_);
      connect(apply_btn, &QPushButton::clicked,
              this, &NewSchoolYearPage::on_apply_);
    }
  }

  void NewSchoolYearPage::enter(
      QStringList const& classes, int grade_min, int grade_max)
  {
    grade_min_ = grade_min;
    grade_max_ = grade_max;
    table_->setRowCount(0);
    for(QString const& cls : classes) {
      int const row = table_->rowCount();
      table_->insertRow(row);
      auto* old_item = new QTableWidgetItem(cls);
      old_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      table_->setItem(row, 0, old_item);
      QString const suggested =
          colman::compute_new_name(cls, grade_min, grade_max);
      auto* new_item = new QTableWidgetItem(suggested);
      new_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
      table_->setItem(row, 1, new_item);
    }
  // Extra row for adding a brand-new class.
    {
      int const row = table_->rowCount();
      table_->insertRow(row);
      auto* placeholder = new QTableWidgetItem(tr("(new class)"));
      placeholder->setFlags(Qt::ItemIsEnabled);
      table_->setItem(row, 0, placeholder);
      auto* new_item = new QTableWidgetItem();
      new_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
      table_->setItem(row, 1, new_item);
    }
  }

  void NewSchoolYearPage::on_apply_()
  {
    COLMAN_TRY
    {
      {
        QMap<QString, QString> old_to_new;
        QStringList extra_classes;
        for(int row = 0; row < table_->rowCount(); ++row) {
          QTableWidgetItem const* old_item = table_->item(row, 0);
          QTableWidgetItem const* new_item = table_->item(row, 1);
          if(!old_item || !new_item) {
            continue;
          }
          if(old_item->flags().testFlag(Qt::ItemIsSelectable)) {
            old_to_new[old_item->text()] = new_item->text().trimmed();
          }
          else {
            QString const extra = new_item->text().trimmed();
            if(!extra.isEmpty()) {
              extra_classes.append(extra);
            }
          }
        }
        review_page_->enter(old_to_new, extra_classes);
        data_.mw_.pages_stack_->setCurrentWidget(review_page_);
      }
    }
    COLMAN_CATCH;
  }

  void NewSchoolYearPage::on_add_new_class_()
  {
    int const row = table_->rowCount();
    table_->insertRow(row);
    auto* placeholder = new QTableWidgetItem(tr("(new class)"));
    placeholder->setFlags(Qt::ItemIsEnabled);
    table_->setItem(row, 0, placeholder);
    auto* new_item = new QTableWidgetItem();
    new_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    table_->setItem(row, 1, new_item);
  }

  void NewSchoolYearPage::on_back_()
  {
    colman::enter_class_management_page(data_);
  }

// ---------------------------------------------------------------------------
// ClassManagementMainPage
// ---------------------------------------------------------------------------

  ClassManagementMainPage::ClassManagementMainPage(
      colman::AppData& data,
      NewSchoolYearPage* school_year_page,
      QWidget* parent)
      : QWidget(parent), data_(data), school_year_page_(school_year_page)
  {
    auto* vlay = new QVBoxLayout(this);

    auto* back_btn = new QPushButton(tr("back"), this);
    back_btn->setObjectName("class_back_");
    vlay->addWidget(back_btn);

    vlay->addWidget(new QLabel(
        QString("<b>%1</b>").arg(tr("Class Management")), this));

    {
      auto* hlay = new QHBoxLayout();
      hlay->addWidget(new QLabel(tr("Grade range:"), this));
      min_spin_ = new QSpinBox(this);
      min_spin_->setRange(1, 99);
      min_spin_->setValue(1);
      hlay->addWidget(min_spin_);
      hlay->addWidget(new QLabel(tr("to"), this));
      max_spin_ = new QSpinBox(this);
      max_spin_->setObjectName("grade_max_");
      max_spin_->setRange(1, 99);
      max_spin_->setValue(13);
      hlay->addWidget(max_spin_);
      auto* save_btn = new QPushButton(tr("Save"), this);
      save_btn->setObjectName("grade_save_");
      hlay->addWidget(save_btn);
      hlay->addStretch();
      vlay->addLayout(hlay);
      connect(save_btn, &QPushButton::clicked,
              this, &ClassManagementMainPage::on_save_grade_range_);
    }

    class_list_ = new QListWidget(this);
    vlay->addWidget(class_list_, 1);

    {
      auto* hlay = new QHBoxLayout();
      auto* add_btn = new QPushButton(tr("Add"), this);
      remove_btn_ = new QPushButton(tr("Remove"), this);
      rename_btn_ = new QPushButton(tr("Rename"), this);
      auto* school_year_btn = new QPushButton(tr("New school year"), this);
      school_year_btn->setObjectName("new_school_year_btn_");
      hlay->addWidget(add_btn);
      hlay->addWidget(remove_btn_);
      hlay->addWidget(rename_btn_);
      hlay->addStretch();
      hlay->addWidget(school_year_btn);
      vlay->addLayout(hlay);
      connect(add_btn, &QPushButton::clicked,
              this, &ClassManagementMainPage::on_add_);
      connect(remove_btn_, &QPushButton::clicked,
              this, &ClassManagementMainPage::on_remove_);
      connect(rename_btn_, &QPushButton::clicked,
              this, &ClassManagementMainPage::on_rename_);
      connect(school_year_btn, &QPushButton::clicked,
              this, &ClassManagementMainPage::on_new_school_year_);
      connect(back_btn, &QPushButton::clicked,
              this, &ClassManagementMainPage::on_back_);
      connect(class_list_, &QListWidget::itemSelectionChanged,
              this, &ClassManagementMainPage::on_selection_changed_);
    }

    remove_btn_->setEnabled(false);
    rename_btn_->setEnabled(false);
  }

  void ClassManagementMainPage::reload_classes_()
  {
    class_list_->clear();
    class_list_->addItems(data_.queries_->load_class_names());
    on_selection_changed_();
  }

  void ClassManagementMainPage::enter()
  {
    colman::Collection const col = data_.queries_->read_collection();
    min_spin_->setValue(col.get_grade_range_min());
    max_spin_->setValue(col.get_grade_range_max());
    reload_classes_();
  }

  void ClassManagementMainPage::on_save_grade_range_()
  {
    COLMAN_TRY
    {
      {
        colman::Collection col = data_.queries_->read_collection();
        col.set_grade_range_min(min_spin_->value());
        col.set_grade_range_max(max_spin_->value());
        data_.queries_->change_collection(col);
      }
    }
    COLMAN_CATCH;
  }

  void ClassManagementMainPage::on_add_()
  {
    COLMAN_TRY
    {
      {
        bool ok = false;
        QString const name = QInputDialog::getText(
            this, tr("Add class"),
            tr("New class name:"),
            QLineEdit::Normal, QString(), &ok);
        if(!ok || name.trimmed().isEmpty()) {
          return;
        }
        data_.queries_->add_class_name(name.trimmed());
        reload_classes_();
      }
    }
    COLMAN_CATCH;
  }

  void ClassManagementMainPage::on_remove_()
  {
    COLMAN_TRY
    {
      {
        QListWidgetItem const* item = class_list_->currentItem();
        if(!item) {
          return;
        }
        QString const name = item->text();
        if(colman::question_yes_no(
               this, tr("Remove class"),
               tr("Really remove class \"%1\"?").arg(name)) != QMessageBox::Yes) {
          return;
        }
        data_.queries_->remove_class_name(name);
        reload_classes_();
      }
    }
    COLMAN_CATCH;
  }

  void ClassManagementMainPage::on_rename_()
  {
    COLMAN_TRY
    {
      {
        QListWidgetItem const* item = class_list_->currentItem();
        if(!item) {
          return;
        }
        QString const old_name = item->text();
        bool ok = false;
        QString const new_name = QInputDialog::getText(
            this, tr("Rename class"),
            tr("New class name:"),
            QLineEdit::Normal, old_name, &ok);
        if(!ok || new_name.trimmed().isEmpty() || new_name.trimmed() == old_name) {
          return;
        }
        data_.queries_->rename_class_name(old_name, new_name.trimmed());
        reload_classes_();
      }
    }
    COLMAN_CATCH;
  }

  void ClassManagementMainPage::on_new_school_year_()
  {
    COLMAN_TRY
    {
      {
        QStringList const classes = data_.queries_->load_class_names();
        colman::Collection const col = data_.queries_->read_collection();
        school_year_page_->enter(
            classes,
            col.get_grade_range_min(),
            col.get_grade_range_max());
        data_.mw_.pages_stack_->setCurrentWidget(school_year_page_);
      }
    }
    COLMAN_CATCH;
  }

  void ClassManagementMainPage::on_back_()
  {
    data_.mw_.pages_stack_->setCurrentWidget(data_.mw_.central_page_);
  }

  void ClassManagementMainPage::on_selection_changed_()
  {
    bool const has_sel = (class_list_->currentItem() != nullptr);
    remove_btn_->setEnabled(has_sel);
    rename_btn_->setEnabled(has_sel);
  }

} // namespace

// Must be included to compile the Q_OBJECT classes in the anonymous namespace.
#include "colman_class_management_page.moc"

// ---------------------------------------------------------------------------
// Public free functions
// ---------------------------------------------------------------------------

void colman::setup_class_management_pages(
    QApplication& app, AppData& data)
{
  auto* review = new NewSchoolYearReviewPage(data);
  auto* school_year = new NewSchoolYearPage(data, review);
  auto* main_page = new ClassManagementMainPage(data, school_year);

  review->set_school_year_page(school_year);

  data.class_management_page_ = main_page;
  data.mw_.pages_stack_->addWidget(review);
  data.mw_.pages_stack_->addWidget(school_year);
  data.mw_.pages_stack_->addWidget(main_page);

  app.connect(data.class_management_button_, &QToolButton::clicked,
              [&data]() {
                colman::enter_class_management_page(data);
              });
}

void colman::enter_class_management_page(AppData& data)
{
  if(!data.class_management_page_) {
    return;
  }
  static_cast<ClassManagementMainPage*>(
      data.class_management_page_)
      ->enter();
  data.mw_.pages_stack_->setCurrentWidget(data.class_management_page_);
}

/* EOF */
