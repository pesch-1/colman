// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Definition of run_tutorial_screenshots().
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TUTORIAL_SCREENSHOTS_HPP
#  include "colman_tutorial_screenshots.hpp"
#endif
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_ACQUISITION_HPP
#  include "colman_acquisition.hpp"
#endif
#ifndef COLMAN_JSON_HPP
#  include "colman_json.hpp"
#endif
#ifndef COLMAN_LENDING_HPP
#  include "colman_lending.hpp"
#endif
#ifndef COLMAN_MEDIA_KIND_HPP
#  include "colman_media_kind.hpp"
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#endif
#ifndef COLMAN_PERSON_HPP
#  include "colman_person.hpp"
#endif
#ifndef SYSTEM_QABSTRACTBUTTON
#  include <QAbstractButton>
#  define SYSTEM_QABSTRACTBUTTON
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
#ifndef SYSTEM_QDIALOG
#  include <QDialog>
#  define SYSTEM_QDIALOG
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QELAPSEDTIMER
#  include <QElapsedTimer>
#  define SYSTEM_QELAPSEDTIMER
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QFILEDIALOG
#  include <QFileDialog>
#  define SYSTEM_QFILEDIALOG
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QJSONARRAY
#  include <QJsonArray>
#  define SYSTEM_QJSONARRAY
#endif
#ifndef SYSTEM_QJSONDOCUMENT
#  include <QJsonDocument>
#  define SYSTEM_QJSONDOCUMENT
#endif
#ifndef SYSTEM_QJSONOBJECT
#  include <QJsonObject>
#  define SYSTEM_QJSONOBJECT
#endif
#ifndef SYSTEM_QKEYEVENT
#  include <QKeyEvent>
#  define SYSTEM_QKEYEVENT
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
#ifndef SYSTEM_QPLAINTEXTEDIT
#  include <QPlainTextEdit>
#  define SYSTEM_QPLAINTEXTEDIT
#endif
#ifndef SYSTEM_QPOINTER
#  include <QPointer>
#  define SYSTEM_QPOINTER
#endif
#ifndef SYSTEM_QSTACKEDWIDGET
#  include <QStackedWidget>
#  define SYSTEM_QSTACKEDWIDGET
#endif
#ifndef SYSTEM_QABSTRACTITEMVIEW
#  include <QAbstractItemView>
#  define SYSTEM_QABSTRACTITEMVIEW
#endif
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QSCROLLAREA
#  include <QScrollArea>
#  define SYSTEM_QSCROLLAREA
#endif
#ifndef SYSTEM_QSPINBOX
#  include <QSpinBox>
#  define SYSTEM_QSPINBOX
#endif
#ifndef SYSTEM_QTABWIDGET
#  include <QTabWidget>
#  define SYSTEM_QTABWIDGET
#endif
#ifndef SYSTEM_QTABLEVIEW
#  include <QTableView>
#  define SYSTEM_QTABLEVIEW
#endif
#ifndef SYSTEM_QTOOLBUTTON
#  include <QToolButton>
#  define SYSTEM_QTOOLBUTTON
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QDOUBLESPINBOX
#  include <QDoubleSpinBox>
#  define SYSTEM_QDOUBLESPINBOX
#endif
#ifndef SYSTEM_QGRAPHICSRECTITEM
#  include <QGraphicsRectItem>
#  define SYSTEM_QGRAPHICSRECTITEM
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QMOUSEEVENT
#  include <QMouseEvent>
#  define SYSTEM_QMOUSEEVENT
#endif
#ifndef SYSTEM_MAP
#  include <map>
#  define SYSTEM_MAP
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
#ifndef SYSTEM_FUNCTIONAL
#  include <functional>
#  define SYSTEM_FUNCTIONAL
#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef SYSTEM_STDEXCEPT
#  include <stdexcept>
#  define SYSTEM_STDEXCEPT
#endif

namespace
{
  /// JPEG quality of the written screenshots.
  int constexpr JpegQuality = 85;
  /// Interval of the modal dialog watcher in milliseconds.
  int constexpr ModalPollMs = 50;
  /// Default time to let the UI settle after an action in milliseconds.
  int constexpr SettleMs = 150;
  /// Maximum time to wait for an expected dialog in milliseconds.
  int constexpr DialogTimeoutMs = 10000;

  /// Label template (prefix) for the sample ID card layout.
  char constexpr IdCardTemplate[] = "Avery L7163";
  /// Label template (prefix) for the sample item card layout.
  char constexpr ItemCardTemplate[] = "Avery L7160";
  /// Label template (prefix) for the sample signature layout.
  char constexpr SignatureTemplate[] = "Avery 3666";

  /// Thrown when a scenario step cannot be performed.
  class ScenarioError : public std::runtime_error
  {
  public:
    explicit ScenarioError(QString const& what)
        : std::runtime_error(what.toStdString())
    {
    }
  };

  /// Maps the media kind names used in the sample data file to MediaKind.
  colman::MediaKind media_kind_from_name(QString const& name)
  {
    if(name == "cd") {
      return colman::MediaKind::Cd;
    }
    if(name == "dvd") {
      return colman::MediaKind::Dvd;
    }
    if(name == "bluray") {
      return colman::MediaKind::BluRay;
    }
    if(name == "boardgame") {
      return colman::MediaKind::BoardGame;
    }
    return colman::MediaKind::Book;
  }

  /// Drives the main window through the tutorial and takes screenshots.
  /**
     The scenario runs sequentially on the GUI thread.  Waiting is done with
     nested event loops (like QTest does).  Dialogs opened with exec() block
     the code that opened them, so a permanent watcher timer hands every new
     modal dialog either to the handler armed with expect_dialog() or, when
     none is armed, logs and rejects it so the scenario cannot hang.
  */
  class TutorialScenario
  {
  public:
    TutorialScenario(colman::AppData& data,
                     colman::TutorialScreenshotOptions const& options)
        : data_(data), options_(options)
    {
    }

    /// Runs all scenes; returns the process exit code.
    int run()
    {
      QTimer watcher;
      watcher.setInterval(ModalPollMs);
      QObject::connect(&watcher, &QTimer::timeout,
                       [this]() { check_modal(); });
      watcher.start();
      int rc = 0;
      try {
        load_sample_data();
        QDir().mkpath(options_.out_dir_);
        scene_new_collection();
        insert_sample_data();
        scene_persons();
        scene_classes();
        scene_layouts();
        scene_acquisition();
        scene_item_labels();
        scene_signatures();
        scene_id_cards();
        scene_lending();
        scene_return();
        scene_liquidation();
        scene_search();
        scene_open_lendings();
        scene_history();
        scene_group_lending();
        scene_group_return();
        scene_settings();
        scene_admin_pages();
        scene_logout();
        scene_reopen_and_login();
      }
      catch(std::exception const& e) {
        std::cerr << "tutorial screenshots: step \""
                  << qPrintable(step_) << "\" failed: " << e.what()
                  << '\n';
        save_pixmap(data_.w_.grab(), "failed_step");
        rc = 1;
      }
      watcher.stop();
      std::cout << "tutorial screenshots: " << written_.size()
                << " written";
      if(!skipped_.isEmpty()) {
        std::cout << ", skipped: " << qPrintable(skipped_.join(", "));
      }
      std::cout << '\n';
      return rc;
    }

  private:
    // -----------------------------------------------------------------
    // Driver helpers
    // -----------------------------------------------------------------

    /// Names the current step for progress output and error messages.
    void step(QString const& name)
    {
      step_ = name;
      std::cout << "  " << qPrintable(name) << '\n';
    }

    /// Processes events for @p ms milliseconds (layouts, timers, models).
    void settle(int ms = SettleMs)
    {
      QElapsedTimer t;
      t.start();
      do {
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QApplication::processEvents(QEventLoop::AllEvents, 20);
      } while(t.elapsed() < ms);
    }

    /// Processes events until @p done returns true or @p timeout_ms passes.
    void wait_until(std::function<bool()> const& done, QString const& what,
                    int timeout_ms = DialogTimeoutMs)
    {
      QElapsedTimer t;
      t.start();
      while(!done()) {
        if(t.elapsed() > timeout_ms) {
          throw ScenarioError("timeout waiting for " + what);
        }
        settle(20);
      }
    }

    /// Returns the page currently shown in the main window stack.
    QWidget* current_page() const
    {
      return data_.mw_.pages_stack_->currentWidget();
    }

    /// Returns the child of @p root named @p name; throws if there is none.
    template <typename T>
    T* find(QString const& name, QObject const* root = nullptr) const
    {
      if(!root) {
        root = current_page();
      }
      T* const w = root->findChild<T*>(name);
      if(!w) {
        throw ScenarioError("widget \"" + name + "\" not found");
      }
      return w;
    }

    /// Clicks @p button and lets the UI settle.
    void click(QAbstractButton* button)
    {
      if(!button->isEnabled()) {
        throw ScenarioError("button \"" + button->text() +
                            "\" is disabled");
      }
      button->click();
      settle();
    }

    /// Types @p text into @p w with key events, like a user would.
    void type(QWidget* w, QString const& text)
    {
      if(auto* combo = qobject_cast<QComboBox*>(w)) {
        w = combo->lineEdit();
      }
      w->setFocus();
      for(QChar const c : text) {
        QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, QString(c));
        QApplication::sendEvent(w, &press);
        QKeyEvent release(QEvent::KeyRelease, 0, Qt::NoModifier, QString(c));
        QApplication::sendEvent(w, &release);
      }
      settle(50);
    }

    /// Sends the key @p k (press and release) to @p w.
    void key(QWidget* w, Qt::Key k)
    {
      w->setFocus();
      QKeyEvent press(QEvent::KeyPress, k, Qt::NoModifier);
      QApplication::sendEvent(w, &press);
      QKeyEvent release(QEvent::KeyRelease, k, Qt::NoModifier);
      QApplication::sendEvent(w, &release);
      settle();
    }

    /// Clears the line edit @p edit and types @p text into it.
    void retype(QLineEdit* edit, QString const& text)
    {
      // Focus first: leaving the previous field may trigger handlers that
      // fill this one (e.g. the username suggestion on the person page).
      edit->setFocus();
      settle(50);
      edit->clear();
      type(edit, text);
    }

    /// Saves @p pixmap as <out_dir>/<name>.jpg.
    void save_pixmap(QPixmap const& pixmap, QString const& name)
    {
      QString const path = options_.out_dir_ + "/" + name + ".jpg";
      if(!pixmap.save(path, "JPEG", JpegQuality)) {
        throw ScenarioError("could not write " + path);
      }
    }

    /// Grabs @p w (default: the main window) into the screenshot @p name.
    void shot(QString const& name, QWidget* w = nullptr)
    {
      settle(300);
      save_pixmap((w ? w : &data_.w_)->grab(), name);
      written_ << name;
    }

    /// Records that the screenshot @p name was not produced and why.
    void skip(QString const& name, QString const& reason)
    {
      std::cout << "    skipped " << qPrintable(name) << ": "
                << qPrintable(reason) << '\n';
      skipped_ << name;
    }

    /// Hands the next new modal dialog to @p handler.
    /**
       The handler runs inside the dialog's event loop and must close the
       dialog (accept, reject or a button click).
    */
    void expect_dialog(std::function<void(QWidget*)> handler)
    {
      dialog_handler_ = std::move(handler);
      dialog_handled_ = false;
      known_modal_ = QApplication::activeModalWidget();
    }

    /// Waits until the dialog armed with expect_dialog() was handled.
    void wait_dialog_handled(QString const& what)
    {
      wait_until([this]() { return dialog_handled_; }, "dialog " + what);
      if(!dialog_error_.isEmpty()) {
        QString const err = dialog_error_;
        dialog_error_.clear();
        throw ScenarioError(err);
      }
      settle();
    }

    /// Clicks @p button, which opens a dialog handled by @p handler.
    void click_dialog(QAbstractButton* button,
                      std::function<void(QWidget*)> handler)
    {
      expect_dialog(std::move(handler));
      click(button);
      wait_dialog_handled(button->text());
    }

    /// Watcher timer slot: dispatches new modal dialogs.
    void check_modal()
    {
      QWidget* const modal = QApplication::activeModalWidget();
      if(!modal || modal == known_modal_ || modal == handled_modal_ ||
         in_handler_)
      {
        return;
      }
      if(dialog_handler_) {
        auto handler = std::move(dialog_handler_);
        dialog_handler_ = nullptr;
        in_handler_ = true;
        try {
          settle(200);
          handler(modal);
        }
        catch(std::exception const& e) {
          dialog_error_ = QString::fromStdString(e.what());
          if(auto* dlg = qobject_cast<QDialog*>(modal)) {
            dlg->reject();
          }
          else {
            modal->close();
          }
        }
        in_handler_ = false;
        known_modal_ = nullptr;
        // The dialog stays the active modal widget until its event loop
        // returns; do not treat it as a new dialog meanwhile.
        handled_modal_ = modal;
        dialog_handled_ = true;
        return;
      }
      // Unexpected dialog: report it and get rid of it.
      QString text = modal->windowTitle();
      if(auto* mb = qobject_cast<QMessageBox*>(modal)) {
        text += ": " + mb->text();
      }
      std::cerr << "tutorial screenshots: unexpected dialog in step \""
                << qPrintable(step_) << "\": " << qPrintable(text) << '\n';
      save_pixmap(modal->grab(), "unexpected_dialog");
      if(auto* dlg = qobject_cast<QDialog*>(modal)) {
        dlg->reject();
      }
      else {
        modal->close();
      }
    }

    /// Answers the message box @p w with the button of role @p button.
    static void answer(QWidget* w, QMessageBox::StandardButton button)
    {
      auto* mb = qobject_cast<QMessageBox*>(w);
      if(!mb) {
        throw ScenarioError("expected a message box, got " +
                            QString(w->metaObject()->className()));
      }
      QAbstractButton* const b = mb->button(button);
      if(!b) {
        throw ScenarioError("message box has no such button: " + mb->text());
      }
      b->click();
    }

    // -----------------------------------------------------------------
    // Sample data
    // -----------------------------------------------------------------

    /// Reads the sample data file.
    void load_sample_data()
    {
      step("load sample data");
      QFile f(options_.sample_data_file_);
      if(!f.open(QIODevice::ReadOnly)) {
        throw ScenarioError("cannot read " + options_.sample_data_file_);
      }
      QJsonParseError err;
      QJsonDocument const doc = QJsonDocument::fromJson(f.readAll(), &err);
      if(doc.isNull()) {
        throw ScenarioError("invalid JSON in " +
                            options_.sample_data_file_ + ": " +
                            err.errorString());
      }
      sample_ = doc.object();
      collection_ = sample_.value("collection").toObject();
      collection_file_ = options_.work_dir_ + "/" +
                         collection_.value("file_name").toString();
    }

    /// Returns the string @p key of the "collection" sample object.
    QString coll(QString const& key) const
    {
      return collection_.value(key).toString();
    }

    /// Adds the sample person @p obj with credentials; returns its ID.
    int add_person(QJsonObject const& obj)
    {
      colman::Person person;
      colman::from_json(obj, person);
      int const id = data_.queries_->add_new_person(person);
      if(id < 0) {
        throw ScenarioError("could not add person " +
                            person.get_lastname());
      }
      QString const username = obj.value("username").toString();
      if(!username.isEmpty()) {
        data_.queries_->set_username(id, username);
      }
      QString const password = obj.value("password").toString();
      if(!password.isEmpty()) {
        data_.queries_->set_password(id,
                                     colman::hash_password(password));
      }
      for(auto const& perm : obj.value("permissions").toArray()) {
        data_.queries_->set_permission(id, perm.toString(), true);
      }
      return id;
    }

    /// Adds the sample item @p obj and acquires its copies; returns its ID.
    int add_item(QJsonObject obj)
    {
      auto const kind =
          media_kind_from_name(obj.value("media_kind").toString());
      obj.insert("type_id", data_.queries_->type_id_for_media_kind(kind));
      colman::Item item;
      if(!colman::from_json(obj, item)) {
        throw ScenarioError("invalid sample item " +
                            obj.value("name").toString());
      }
      int const id = data_.queries_->add_new_item(item);
      if(id < 0) {
        throw ScenarioError("could not add item " + item.get_name());
      }
      colman::Acquisition acq;
      acq.set_item_id(id);
      acq.set_count(obj.value("count").toInt(1));
      acq.set_date_time(QDateTime::currentDateTime().addDays(
          -obj.value("acquired_days_ago").toInt(30)));
      acq.set_data_collector_id(0);
      if(data_.queries_->add_new_acquisition(acq) < 0) {
        throw ScenarioError("could not acquire item " + item.get_name());
      }
      return id;
    }

    /// Inserts classes, persons, items and lendings of the sample data.
    void insert_sample_data()
    {
      step("insert sample data");
      for(auto const& c : sample_.value("classes").toArray()) {
        data_.queries_->add_class_name(c.toString());
      }
      for(auto const& p : sample_.value("persons").toArray()) {
        QJsonObject const obj = p.toObject();
        person_ids_.insert(obj.value("username").toString(),
                           add_person(obj));
      }
      for(auto const& i : sample_.value("items").toArray()) {
        QJsonObject const obj = i.toObject();
        item_ids_.insert(obj.value("name").toString(), add_item(obj));
      }
      for(auto const& l : sample_.value("lendings").toArray()) {
        QJsonObject const obj = l.toObject();
        QString const user = obj.value("person").toString();
        QString const name = obj.value("item").toString();
        if(!person_ids_.contains(user) || !item_ids_.contains(name)) {
          throw ScenarioError("lending refers to unknown person or item: " +
                              user + ", " + name);
        }
        QDateTime const when = QDateTime::currentDateTime().addDays(
            -obj.value("days_ago").toInt());
        colman::Lending lending;
        lending.set_person_id(person_ids_.value(user));
        lending.set_item_id(item_ids_.value(name));
        lending.set_count(static_cast<unsigned>(obj.value("count").toInt(1)));
        lending.set_date_time(when);
        lending.set_duedate(QDate::currentDate().addDays(
            obj.value("due_in_days").toInt(14)));
        lending.set_data_collector_id(0);
        if(data_.queries_->add_new_lending(lending) < 0) {
          throw ScenarioError("could not add lending of " + name);
        }
      }
    }

    // -----------------------------------------------------------------
    // Scenes
    // -----------------------------------------------------------------

    /// Clicks the Next/Finish button of the current collection flow step.
    void next_collection_step()
    {
      click(find<QPushButton>("create_collection_next_"));
    }

    /// Start page flow "create new collection" (002-008).
    void scene_new_collection()
    {
      step("new collection: name");
      QFile::remove(collection_file_);
      click(data_.new_button_);
      type(find<QLineEdit>("create_collection_name_"), coll("name"));
      type(find<QLineEdit>("create_collection_organization_"),
           coll("organization"));
      type(find<QLineEdit>("create_collection_responsible_"),
           coll("responsible"));
      shot("002_new_collection_name");
      next_collection_step();

      step("new collection: logo");
      shot("003_new_collection_logo");
      next_collection_step();

      step("new collection: lending period");
      click(find<QCheckBox>("create_collection_use_period_"));
      shot("004_new_collection_lending_period");
      next_collection_step();

      step("new collection: school items");
      shot("005_new_collection_school_items");
      next_collection_step();

      QString const enc_pw = coll("encryption_password");
      if(current_page()->findChild<QCheckBox*>(
             "create_collection_encrypt_"))
      {
        step("new collection: encryption");
        click(find<QCheckBox>("create_collection_encrypt_"));
        type(find<QLineEdit>("encryption_password_"), enc_pw);
        type(find<QLineEdit>("encryption_confirm_"), enc_pw);
        shot("007_new_collection_encryption");
        next_collection_step();
        encrypted_ = true;
      }
      else {
        skip("007_new_collection_encryption",
             "encryption not available");
      }

      step("new collection: admin password");
      type(find<QLineEdit>("create_collection_admin_password_"),
           coll("admin_password"));
      type(find<QLineEdit>("create_collection_admin_confirm_"),
           coll("admin_password"));
      shot("008_new_collection_admin_password");
      next_collection_step();

      step("new collection: file");
      type(find<QLineEdit>("create_collection_file_"), collection_file_);
      shot("006_new_collection_file");
      next_collection_step();
      wait_until(
          [this]() { return current_page() == data_.mw_.central_page_; },
          "central page after creating the collection");

      step("central page");
      check_central_grid();
      shot("009_central_page");
    }

    /// Persons page: add a person with initial password (013-016).
    void scene_persons()
    {
      step("persons: add person");
      click(data_.new_person_button_);
      QJsonObject const p = sample_.value("ui_person").toObject();
      QWidget* const page = data_.mw_.person_page_;
      type(find<QLineEdit>("person_lastname_", page),
           p.value("lastname").toString());
      type(find<QLineEdit>("person_firstname_", page),
           p.value("firstname").toString());
      type(find<QComboBox>("person_school_class_", page),
           p.value("school_class").toString());
      type(find<QLineEdit>("person_emails_", page),
           p.value("emails").toString());
      retype(find<QLineEdit>("person_username_edit_", page),
           p.value("username").toString());
      shot("013_add_person");
      click(data_.mw_.insert_person_);

      step("persons: initial password");
      QString const pw = p.value("password").toString();
      type(find<QLineEdit>("initial_password_new_"), pw);
      type(find<QLineEdit>("initial_password_confirm_"), pw);
      shot("014_initial_password");
      click(find<QPushButton>("initial_password_ok_"));
      wait_until([this]() { return current_page() == data_.mw_.person_page_; },
                 "person page after setting the initial password");
      person_ids_.insert(p.value("username").toString(),
                         data_.mw_.person_person_list_
                             ->get_selected_person_id());

      step("persons: permissions");
      scroll_to(find<QWidget>("person_permissions_widget_", page));
      shot("016_permissions");
      click(data_.mw_.back_button2_);
    }

    /// Returns to the central page without going through back buttons.
    void back_to_central()
    {
      data_.mw_.pages_stack_->setCurrentWidget(data_.mw_.central_page_);
      settle();
    }

    /// Selects the first row of @p view containing a cell equal to @p text.
    void select_row(QAbstractItemView* view, QString const& text)
    {
      QAbstractItemModel const* const model = view->model();
      for(int r = 0; r < model->rowCount(); ++r) {
        for(int c = 0; c < model->columnCount(); ++c) {
          QModelIndex const idx = model->index(r, c);
          if(idx.data().toString() == text) {
            view->setCurrentIndex(idx);
            view->selectionModel()->select(
                idx, QItemSelectionModel::ClearAndSelect |
                         QItemSelectionModel::Rows);
            view->scrollTo(idx);
            settle();
            return;
          }
        }
      }
      throw ScenarioError("no row \"" + text + "\" in table");
    }

    /// Adds the rows containing @p text to the selection of @p view.
    void add_row_to_selection(QAbstractItemView* view, QString const& text)
    {
      QAbstractItemModel const* const model = view->model();
      for(int r = 0; r < model->rowCount(); ++r) {
        for(int c = 0; c < model->columnCount(); ++c) {
          QModelIndex const idx = model->index(r, c);
          if(idx.data().toString() == text) {
            view->selectionModel()->select(
                idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            settle();
            return;
          }
        }
      }
      throw ScenarioError("no row \"" + text + "\" in table");
    }

    /// Returns the editor of the type-specific field @p column in @p item.
    /**
       Type-specific rows are identified by the "db_column" property of their
       label in column 0 of the item grid; the editor sits in column 1.
    */
    QWidget* item_field(QWidget* item, QString const& column) const
    {
      for(QLabel* label : item->findChildren<QLabel*>()) {
        if(label->property("db_column").toString() != column) {
          continue;
        }
        auto* grid =
            qobject_cast<QGridLayout*>(label->parentWidget()->layout());
        int const i = grid ? grid->indexOf(label) : -1;
        if(i < 0) {
          break;
        }
        int row = 0;
        int col = 0;
        int rs = 0;
        int cs = 0;
        grid->getItemPosition(i, &row, &col, &rs, &cs);
        if(QLayoutItem* const it = grid->itemAtPosition(row, 1)) {
          if(QWidget* const w = it->widget()) {
            if(auto* edit = qobject_cast<QLineEdit*>(w)) {
              return edit;
            }
            if(auto* edit = w->findChild<QLineEdit*>()) {
              return edit;
            }
            return w;
          }
        }
      }
      throw ScenarioError("item field \"" + column + "\" not found");
    }

    /// Selects the entry of @p combo starting with @p prefix.
    void choose(QComboBox* combo, QString const& prefix)
    {
      for(int i = 0; i < combo->count(); ++i) {
        if(combo->itemText(i).startsWith(prefix)) {
          combo->setCurrentIndex(i);
          settle();
          return;
        }
      }
      throw ScenarioError("no entry \"" + prefix + "\" in combo box");
    }

    /// Clicks the first @p n label positions of the sheet preview on the page.
    /**
       Label positions are the only rectangle items of the preview that
       accept just the left mouse button (see LabelSelectionWidget).
    */
    void click_label_positions(int n)
    {
      QGraphicsView* view = nullptr;
      for(auto* v : current_page()->findChildren<QGraphicsView*>()) {
        if(v->isVisible() && v->scene()) {
          view = v;
          break;
        }
      }
      if(!view) {
        throw ScenarioError("no label preview on the page");
      }
      std::vector<QGraphicsRectItem*> labels;
      for(QGraphicsItem* it : view->scene()->items()) {
        auto* r = qgraphicsitem_cast<QGraphicsRectItem*>(it);
        if(r && r->acceptedMouseButtons() == Qt::LeftButton) {
          labels.push_back(r);
        }
      }
      std::sort(labels.begin(), labels.end(),
                [](QGraphicsRectItem const* a, QGraphicsRectItem const* b) {
                  QPointF const pa = a->sceneBoundingRect().topLeft();
                  QPointF const pb = b->sceneBoundingRect().topLeft();
                  return pa.y() < pb.y() - 0.5 ||
                         (qAbs(pa.y() - pb.y()) <= 0.5 && pa.x() < pb.x());
                });
      int const count = qMin(n, static_cast<int>(labels.size()));
      for(int i = 0; i < count; ++i) {
        QPoint const pos =
            view->mapFromScene(labels[i]->sceneBoundingRect().center());
        QPointF const global = view->viewport()->mapToGlobal(QPointF(pos));
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(pos), global,
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(view->viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(pos), global,
                            Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(view->viewport(), &release);
        settle(50);
      }
    }

    /// Checks that the tiles of the central page form aligned columns.
    /**
       All visible tiles must have the same width and height, the n-th
       visible tile of every row must start at the same x position, and no
       tile may extend beyond the window.  Long translations would otherwise break the
       columns.
    */
    void check_central_grid()
    {
      std::map<QWidget const*, std::vector<QToolButton const*>> rows;
      for(QToolButton const* tile :
          data_.mw_.central_page_->findChildren<QToolButton*>())
      {
        if(tile->isVisible()) {
          rows[tile->parentWidget()].push_back(tile);
        }
      }
      if(rows.empty()) {
        throw ScenarioError("central page has no visible tiles");
      }
      int const width = rows.begin()->second.front()->width();
      int const height = rows.begin()->second.front()->height();
      std::vector<int> column_x;
      for(auto& row : rows) {
        auto& tiles = row.second;
        std::sort(tiles.begin(), tiles.end(),
                  [](QToolButton const* a, QToolButton const* b) {
                    return a->x() < b->x();
                  });
        for(std::size_t i = 0; i < tiles.size(); ++i) {
          QPoint const pos = tiles[i]->mapTo(&data_.w_, QPoint(0, 0));
          if(tiles[i]->width() != width) {
            throw ScenarioError("central page tile \"" + tiles[i]->text() +
                                "\" has a different width");
          }
          if(tiles[i]->height() != height) {
            throw ScenarioError("central page tile \"" + tiles[i]->text() +
                                "\" has a different height");
          }
          if(pos.x() + width > data_.w_.width()) {
            throw ScenarioError("central page tile \"" + tiles[i]->text() +
                                "\" exceeds the window");
          }
          if(i < column_x.size()) {
            if(column_x[i] != pos.x()) {
              throw ScenarioError("central page tile \"" +
                                  tiles[i]->text() + "\" is not aligned");
            }
          }
          else {
            column_x.push_back(pos.x());
          }
        }
      }
    }

    /// Scrolls the scroll area containing @p w so that @p w is visible.
    void scroll_to(QWidget* w)
    {
      for(QWidget* p = w->parentWidget(); p; p = p->parentWidget()) {
        if(auto* area = qobject_cast<QScrollArea*>(p)) {
          area->ensureWidgetVisible(w);
          settle();
          return;
        }
      }
    }

    /// Returns the sample string @p key of the object @p group.
    QString sample(QString const& group, QString const& key) const
    {
      return sample_.value(group).toObject().value(key).toString();
    }

    /// Class management and new school year planning (170-172).
    void scene_classes()
    {
      step("classes");
      click(data_.class_management_button_);
      auto* max_spin = find<QSpinBox>("grade_max_");
      max_spin->setValue(sample_.value("grade_max").toInt(13));
      click(find<QPushButton>("grade_save_"));
      shot("170_classes");

      step("new school year");
      click(find<QPushButton>("new_school_year_btn_"));
      shot("171_new_school_year_01");
      click(find<QPushButton>("new_school_year_apply_"));
      shot("172_new_school_year_02");
      back_to_central();
    }

    /// Creates a named layout from a template in the current editor.
    void save_layout(QString const& template_prefix, QString const& name)
    {
      choose(find<QComboBox>("layout_combo_"), template_prefix);
      retype(find<QLineEdit>("layout_name_"), name);
      click(find<QPushButton>("layout_save_"));
    }

    /// Layout editors for ID cards, item cards and signatures (021-023).
    void scene_layouts()
    {
      step("ID card layout");
      click(data_.edit_id_button_);
      save_layout(IdCardTemplate, sample("layouts", "id_card"));
      shot("021_id_card_layout");
      back_to_central();

      step("item card layout");
      click(data_.edit_item_card_button_);
      save_layout(ItemCardTemplate, sample("layouts", "item_card"));
      shot("022_book_card_layout");
      back_to_central();

      step("signature layout");
      click(data_.edit_signature_layout_button_);
      save_layout(SignatureTemplate, sample("layouts", "signature"));
      shot("023_signature_layout");
      back_to_central();

      step("bar width reduction");
      click(data_.edit_item_card_button_);
      auto* bar = find<QDoubleSpinBox>("layout_bar_width_reduction_");
      bar->setValue(0.15);
      bar->setFocus();
      shot("024_bar_width_reduction");
      back_to_central();
    }

    /// Acquisition of a new item entered by hand (030-036).
    void scene_acquisition()
    {
      QJsonObject const it = sample_.value("ui_item").toObject();
      auto& mw = data_.mw_;

      step("acquisition: select");
      click(data_.acquisition_button_);
      shot("030_acquisition_01");

      step("acquisition: search");
      // Return starts the search; with exactly one hit the item gets
      // selected (no internet look-up is involved).
      type(mw.acquisition_select_search_, it.value("search").toString());
      key(mw.acquisition_select_search_, Qt::Key_Return);
      settle(500);
      if(!mw.acquisition_select_more_copies_->isVisible()) {
        throw ScenarioError("search did not select exactly one item");
      }
      shot("031_acquisition_02");
      mw.acquisition_select_search_->clear();
      mw.acquisition_select_search_->setModified(false);
      click(mw.acquisition_select_clear_);
      // Reload the full list for the new item (the search only runs when
      // the field counts as modified).
      mw.acquisition_select_search_->setModified(true);
      key(mw.acquisition_select_search_, Qt::Key_Return);
      settle(500);

      step("acquisition: item data");
      QWidget* const item = mw.acquisition_select_item_;
      type(find<QLineEdit>("item_name_", item),
           it.value("name").toString());
      // Return adds the author to the list of the repeatable field.
      QWidget* const authors = item_field(item, "authors");
      type(authors, it.value("authors").toString());
      key(authors, Qt::Key_Return);
      type(item_field(item, "publisher"), it.value("publisher").toString());
      type(item_field(item, "publication_date"),
           it.value("publication_date").toString());
      scroll_to(find<QLineEdit>("item_name_", item));
      shot("032_acquisition_03");
      scroll_to(item_field(item, "publication_date"));
      shot("033_acquisition_04");

      step("acquisition: count");
      click(mw.acquisition_select_new_item_);
      mw.acquisition_count_count_->setValue(it.value("count").toInt(1));
      shot("034_acquisition_05");

      step("acquisition: done");
      click(mw.acquisition_count_finish_);
      shot("036_acquisition_07");
      click(mw.acquisition_done_no_);
      back_to_central();
    }

    /// Item label printing (080, 081).
    void scene_item_labels()
    {
      auto& mw = data_.mw_;
      step("item labels: select");
      click(data_.print_barcode_button_);
      mw.item_barcode_recent_count_->setValue(5);
      if(!mw.item_barcode_recent_check_->isChecked()) {
        click(mw.item_barcode_recent_check_);
      }
      click(mw.item_barcode_select_all_);
      shot("080_item_labels_print_01");

      step("item labels: print page");
      click(mw.item_card_print_);
      click_label_positions(5);
      shot("081_item_labels_print_02");
      back_to_central();
      click(data_.print_barcode_button_);
      click(mw.item_barcode_recent_check_);
      back_to_central();
    }

    /// Signature label printing (090, 091).
    void scene_signatures()
    {
      step("signatures");
      click(data_.print_signatures_button_);
      shot("090_signature_print_01");

      step("signatures: positions");
      find<QSpinBox>("signature_top_count_")->setValue(3);
      click(find<QPushButton>("signature_print_top_"));
      click_label_positions(3);
      shot("091_signature_print_02");
      back_to_central();
    }

    /// ID card printing (070-072).
    void scene_id_cards()
    {
      auto& mw = data_.mw_;
      step("ID cards: select");
      click(data_.print_id_button_);
      choose(mw.person_barcode_class_, sample_.value("id_card_class")
                                           .toString());
      click(mw.person_barcode_select_all_);
      shot("070_id_cards_print_01");

      step("ID cards: print page");
      click(mw.person_card_print_);
      click_label_positions(3);
      shot("071_id_cards_print_02");
      back_to_central();

      step("ID cards: recent persons");
      click(data_.print_id_button_);
      mw.person_barcode_class_->setCurrentIndex(0);
      mw.person_barcode_recent_count_->setValue(3);
      if(!mw.person_barcode_recent_check_->isChecked()) {
        click(mw.person_barcode_recent_check_);
      }
      shot("072_id_cards_print_recent");
      click(mw.person_barcode_recent_check_);
      back_to_central();
    }

    /// Lending flow and due date extension (050-055).
    void scene_lending()
    {
      auto& mw = data_.mw_;
      QJsonObject const l = sample_.value("ui_lending").toObject();
      QString const user = l.value("person").toString();

      step("lending: person");
      click(data_.lend_button_);
      mw.lending_person_person_list_->select_person(person_ids_.value(user));
      settle();
      shot("050_lending_01");

      step("lending: item");
      click(mw.lending_person_next_);
      select_row(mw.lending_item_item_view_, l.value("item").toString());
      shot("051_lending_02");

      step("lending: details");
      click(mw.lending_item_next_);
      shot("052_lending_03");

      step("lending: confirm");
      click(mw.lending_details_next_);
      shot("053_lending_04");

      step("lending: done");
      click(mw.lending_confirm_finish_);
      shot("054_lending_05");

      step("lending: extend due date");
      click(mw.lending_done_no_);
      click(data_.lend_button_);
      mw.lending_person_person_list_->select_person(person_ids_.value(user));
      settle();
      click_dialog(mw.lending_person_extend_, [this](QWidget* w) {
        shot("055_lending_extend", w);
        w->close();
      });
      back_to_central();
    }

    /// Group lending flow (056-059).
    void scene_group_lending()
    {
      auto& mw = data_.mw_;
      QJsonObject const g = sample_.value("ui_group_lending").toObject();

      step("group lending: item");
      click(data_.group_lend_button_);
      select_row(mw.group_lending_item_view_, g.value("item").toString());
      shot("056_group_lending_01");

      step("group lending: borrowers");
      click(mw.group_lending_item_next_);
      if(!mw.group_lending_persons_list_->set_class_filter(
             g.value("class").toString()))
      {
        throw ScenarioError("group lending class not offered");
      }
      mw.group_lending_persons_list_->select_all_visible();
      settle();
      shot("057_group_lending_02");

      step("group lending: confirm");
      click(mw.group_lending_persons_next_);
      click(mw.group_lending_details_next_);
      shot("058_group_lending_03");

      step("group lending: done");
      click(mw.group_lending_confirm_finish_);
      shot("059_group_lending_04");
      click(mw.group_lending_done_close_);
      back_to_central();
    }

    /// Group return of the group lending's class set (065-066).
    void scene_group_return()
    {
      auto& mw = data_.mw_;
      QJsonObject const g = sample_.value("ui_group_lending").toObject();

      step("group return: borrowers");
      click(data_.return_button_);
      select_row(mw.return_item_item_view_, g.value("item").toString());
      click(mw.return_item_next_);
      if(current_page() != mw.return_borrower_) {
        throw ScenarioError("group return item has only one borrower");
      }
      if(!mw.return_borrower_person_list_->set_class_filter(
             g.value("class").toString()))
      {
        throw ScenarioError("group return class not offered");
      }
      mw.return_borrower_person_list_->select_all_visible();
      settle();
      shot("065_return_06");

      step("group return: confirm");
      click(mw.return_borrower_next_);
      click(mw.return_details_next_);
      shot("066_return_07");
      click(mw.return_confirm_finish_);
      click(mw.return_done_no_);
      back_to_central();
    }

    /// Return flow (060-064).
    void scene_return()
    {
      auto& mw = data_.mw_;
      QJsonObject const r = sample_.value("ui_return").toObject();

      step("return: item");
      click(data_.return_button_);
      select_row(mw.return_item_item_view_, r.value("item").toString());
      shot("060_return_01");

      step("return: borrower");
      click(mw.return_item_next_);
      // The flow skips the borrower and lending steps when there is only
      // one choice; the sample data provides several for the return item.
      if(current_page() != mw.return_borrower_) {
        throw ScenarioError("return item has only one borrower");
      }
      mw.return_borrower_person_list_->check_person(
          person_ids_.value(r.value("person").toString()));
      settle();
      shot("061_return_02");

      step("return: lending");
      click(mw.return_borrower_next_);
      if(current_page() != mw.return_lending_) {
        throw ScenarioError("return borrower has only one lending");
      }
      mw.return_lending_lendings_view_->selectRow(0);
      settle();
      shot("062_return_03");

      step("return: details");
      click(mw.return_lending_next_);
      shot("063_return_04");

      step("return: confirm");
      click(mw.return_details_next_);
      shot("064_return_05");
      click(mw.return_confirm_finish_);
      click(mw.return_done_no_);
      back_to_central();
    }

    /// Liquidation flow (040-042).
    void scene_liquidation()
    {
      auto& mw = data_.mw_;
      step("liquidation: item");
      click(data_.liquidation_button_);
      select_row(mw.liquidation_item_items_view_,
                 sample_.value("ui_liquidation").toString());
      shot("040_liquidation_01");

      step("liquidation: count");
      click(mw.liquidation_item_next_);
      shot("041_liquidation_02");

      step("liquidation: done");
      click(mw.liquidation_count_finish_);
      shot("042_liquidation_03");
      click(mw.liquidation_done_no_);
      back_to_central();
    }

    /// Search page (110).
    void scene_search()
    {
      step("search");
      click(data_.search_button_);
      type(find<QLineEdit>("search_general_"),
           sample_.value("search").toString());
      settle(800);
      auto* results = find<QTableView>("search_results_");
      if(results->model()->rowCount() > 0) {
        results->setCurrentIndex(results->model()->index(0, 0));
      }
      shot("110_search");

      step("search: detail");
      Q_EMIT results->doubleClicked(results->currentIndex());
      settle(500);
      shot("111_search_detail");
      back_to_central();
    }

    /// Open lendings with print preview and extension dialog (120-122).
    void scene_open_lendings()
    {
      step("open lendings");
      click(data_.open_lendings_button_);
      shot("120_open_lendings");

      step("open lendings: preview");
      click_dialog(find<QPushButton>("open_lendings_print_"),
                   [this](QWidget* w) {
                     settle(500);
                     shot("121_open_lendings_preview", w);
                     w->close();
                   });

      step("open lendings: extend");
      auto* table = find<QTableView>("open_lendings_table_");
      table->selectRow(0);
      for(int r = 1; r < qMin(3, table->model()->rowCount()); ++r) {
        table->selectionModel()->select(
            table->model()->index(r, 0),
            QItemSelectionModel::Select | QItemSelectionModel::Rows);
      }
      settle();
      click_dialog(find<QPushButton>("open_lendings_extend_"),
                   [this](QWidget* w) {
                     shot("122_open_lendings_extend", w);
                     w->close();
                   });
      back_to_central();
    }

    /// History with delete and change dialogs (100-102).
    void scene_history()
    {
      auto& mw = data_.mw_;
      step("history");
      click(data_.history_button_);
      mw.combined_history_table_->selectRow(0);
      settle();
      shot("100_history");

      step("history: delete");
      click_dialog(mw.history_delete_button_, [this](QWidget* w) {
        shot("101_history_delete", w);
        w->close();
      });

      step("history: change");
      click_dialog(mw.history_change_button_, [this](QWidget* w) {
        shot("102_history_change", w);
        w->close();
      });
      back_to_central();
    }

    /// Settings tabs (130-133).
    void scene_settings()
    {
      step("settings");
      click(data_.settings_button_);
      auto* tabs = data_.mw_.settings_page_->findChild<QTabWidget*>();
      if(!tabs) {
        tabs = current_page()->findChild<QTabWidget*>();
      }
      if(!tabs) {
        throw ScenarioError("settings tab widget not found");
      }
      for(int i = 0; i < qMin(4, tabs->count()); ++i) {
        tabs->setCurrentIndex(i);
        shot(QString("13%1_settings_0%2").arg(i).arg(i + 1));
      }
      tabs->setCurrentIndex(0);
      back_to_central();
    }

    /// Item types, passwords and printer calibration (140, 150, 020).
    void scene_admin_pages()
    {
      step("item types");
      click(data_.item_type_button_);
      shot("140_item_types");
      back_to_central();

      step("passwords");
      click(find<QToolButton>("change_passwords_btn_"));
      shot("150_passwords");
      back_to_central();

      step("printer calibration");
      click(data_.calibrate_printer_button_);
      shot("020_printer_calibration");
      back_to_central();
    }

    /// Logout confirmation (160).
    void scene_logout()
    {
      step("logout");
      click_dialog(find<QToolButton>("logout_btn_"), [this](QWidget* w) {
        shot("160_logout", w);
        answer(w, QMessageBox::No);
      });
    }

    /// Logout, reopen the collection and log in as a user (001, 010-018).
    void scene_reopen_and_login()
    {
      step("close collection");
      data_.mw_.action_close_->trigger();
      settle();
      wait_until(
          [this]() { return current_page() == data_.mw_.start_page_; },
          "start page after closing the collection");

      step("start page");
      shot("001_start");

      step("open collection");
      click_dialog(data_.open_button_, [this](QWidget* w) {
        auto* dlg = qobject_cast<QFileDialog*>(w);
        if(!dlg) {
          throw ScenarioError("expected a file dialog");
        }
        dlg->setSidebarUrls({QUrl::fromLocalFile(options_.work_dir_)});
        dlg->setDirectory(options_.work_dir_);
        settle(500); // the file system model loads asynchronously
        // Qt's widget-based file dialog names its file name edit
        // "fileNameEdit"; typing there is what a user would do.
        retype(find<QLineEdit>("fileNameEdit", dlg),
               QFileInfo(collection_file_).fileName());
        shot("010_open_collection", dlg);
        static_cast<QDialog*>(dlg)->accept();
      });

      if(encrypted_) {
        step("database password");
        type(find<QLineEdit>("db_password_"), coll("encryption_password"));
        shot("011_database_password");
        click(find<QPushButton>("db_password_ok_"));
      }
      else {
        skip("011_database_password", "collection is not encrypted");
      }

      step("login selection");
      wait_until(
          [this]() {
            return current_page()->objectName() == "login_selection_page_";
          },
          "login selection page");
      shot("012_login_selection");

      step("user login");
      click(find<QAbstractButton>("login_btn_"));
      QJsonObject const p = sample_.value("ui_person").toObject();
      retype(find<QLineEdit>("login_username_"),
             p.value("username").toString());
      type(find<QLineEdit>("login_password_"),
           p.value("password").toString());
      shot("017_login_user");
      click(find<QPushButton>("login_ok_"));
      wait_until(
          [this]() { return current_page() == data_.mw_.central_page_; },
          "central page after user login");

      step("central page (user)");
      check_central_grid();
      shot("018_central_page_restricted");
    }

    colman::AppData& data_;
    colman::TutorialScreenshotOptions const& options_;
    QJsonObject sample_;
    QJsonObject collection_;
    QString collection_file_;
    bool encrypted_ = false;
    QMap<QString, int> person_ids_;
    QMap<QString, int> item_ids_;

    QString step_;
    QStringList written_;
    QStringList skipped_;

    std::function<void(QWidget*)> dialog_handler_;
    QWidget* known_modal_ = nullptr;
    QPointer<QWidget> handled_modal_;
    bool dialog_handled_ = false;
    bool in_handler_ = false;
    QString dialog_error_;
  };

} // namespace


/**
   Runs the TutorialScenario on @p data.
   @param data    central application state.
   @param options output directory, sample data and scratch directory.
   @return process exit code.
*/
int colman::run_tutorial_screenshots(
    AppData& data, TutorialScreenshotOptions const& options)
{
  TutorialScenario scenario(data, options);
  return scenario.run();
}

/* EOF */
