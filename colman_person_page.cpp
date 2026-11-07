// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// person page setup and interaction logic
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person_page.hpp"
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef COLMAN_PERMISSIONS_WIDGET_HPP
#  include "colman_permissions_widget.hpp"
#endif
#ifndef COLMAN_PERMISSION_DEPS_HPP
#  include "colman_permission_deps.hpp"
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
#endif
#ifndef SYSTEM_QSORTFILTERPROXYMODEL
#  include <QSortFilterProxyModel>
#  define SYSTEM_QSORTFILTERPROXYMODEL
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QPUSHBUTTON
#  include <QPushButton>
#  define SYSTEM_QPUSHBUTTON
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QSIZE
#  include <QSize>
#  define SYSTEM_QSIZE
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#endif
#ifndef COLMAN_CLASS_NAMES_HPP
#  include "colman_class_names.hpp"
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif
#ifndef SYSTEM_QFORMLAYOUT
#  include <QFormLayout>
#  define SYSTEM_QFORMLAYOUT
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QHBOXLAYOUT
#  include <QHBoxLayout>
#  define SYSTEM_QHBOXLAYOUT
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif
#ifndef SYSTEM_QSTYLE
#  include <QStyle>
#  define SYSTEM_QSTYLE
#endif

namespace
{

  /// Page for setting the initial password of a newly created person.
  /**
     Shown immediately after a new person is inserted; the username has already
     been assigned on the person page.  Leaving the password empty (or pressing
     Back) keeps the account without a password, which prevents login until one
     is set.  Calls on_done_() once applied or on Back.
  */
  class SetInitialPasswordPage : public QWidget
  {
  public:
  /// Constructs the page for the person @p person_id in @p data.
    explicit SetInitialPasswordPage(colman::AppData& data,
                                    int person_id,
                                    QWidget* parent = nullptr)
        : QWidget(parent), data_(data), person_id_(person_id)
    {
      QVBoxLayout* vbox = new QVBoxLayout(this);

      back_btn_ = new QPushButton(QApplication::tr("back"), this);
      back_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      vbox->addWidget(back_btn_);

      vbox->addStretch(1);

      QLabel* title = new QLabel(
          "<b>" + QApplication::tr("Set Initial Password") + "</b>", this);
      title->setAlignment(Qt::AlignHCenter);
      vbox->addWidget(title);

      QFormLayout* form = new QFormLayout;
      new_pw_ = new QLineEdit(this);
      new_pw_->setObjectName("initial_password_new_");
      new_pw_->setEchoMode(QLineEdit::Password);
      confirm_pw_ = new QLineEdit(this);
      confirm_pw_->setObjectName("initial_password_confirm_");
      confirm_pw_->setEchoMode(QLineEdit::Password);
      {
        QChar const mask(new_pw_->style()->styleHint(
            QStyle::SH_LineEdit_PasswordCharacter));
        int const pw_w =
            new_pw_->fontMetrics().horizontalAdvance(QString(20, mask)) + 8;
        new_pw_->setMinimumWidth(pw_w);
        confirm_pw_->setMinimumWidth(pw_w);
      }
      form->addRow(QApplication::tr("New password:"), new_pw_);
      form->addRow(QApplication::tr("Confirm:"), confirm_pw_);
      vbox->addLayout(form);

      error_label_ = new QLabel(this);
      error_label_->setStyleSheet("color: red;");
      error_label_->setAlignment(Qt::AlignHCenter);
      error_label_->setVisible(false);
      vbox->addWidget(error_label_);

      QHBoxLayout* btn_hbox = new QHBoxLayout;
      ok_btn_ = new QPushButton(QApplication::tr("OK"), this);
      ok_btn_->setObjectName("initial_password_ok_");
      ok_btn_->setEnabled(false);
      btn_hbox->addStretch(1);
      btn_hbox->addWidget(ok_btn_);
      btn_hbox->addStretch(1);
      vbox->addLayout(btn_hbox);
      vbox->addStretch(1);

      connect(new_pw_, &QLineEdit::textChanged,
              [this]() { check_ok_btn(); });
      connect(confirm_pw_, &QLineEdit::textChanged,
              [this]() { check_ok_btn(); });
      connect(ok_btn_, &QPushButton::clicked,
              [this]() { apply(); });
      connect(back_btn_, &QPushButton::clicked,
              [this]() {
                if(on_done_) {
                  on_done_();
                }
              });

      // An empty password is a valid choice (it leaves the account without a
      // login), so OK must be enabled from the start.
      check_ok_btn();
    }

  /// Sets the callback invoked after the credentials are applied or on Back.
    void set_on_done(std::function<void()> on_done)
    {
      on_done_ = std::move(on_done);
    }

  private:
    void check_ok_btn()
    {
      bool const no_pw_conflict =
          new_pw_->text().isEmpty() ||
          new_pw_->text() == confirm_pw_->text();
      ok_btn_->setEnabled(no_pw_conflict);
      if(!new_pw_->text().isEmpty() &&
         !confirm_pw_->text().isEmpty() &&
         new_pw_->text() != confirm_pw_->text())
      {
        error_label_->setText(QApplication::tr("Passwords do not match."));
        error_label_->setVisible(true);
      }
      else {
        error_label_->setVisible(false);
      }
    }

    void apply()
    {
      if(!new_pw_->text().isEmpty() &&
         new_pw_->text() == confirm_pw_->text())
      {
        data_.queries_->set_password(
            person_id_, colman::hash_password(new_pw_->text()));
      }
      if(on_done_) {
        on_done_();
      }
    }

    colman::AppData& data_;
    int person_id_ = 0;
    std::function<void()> on_done_;
    QLineEdit* new_pw_ = nullptr;
    QLineEdit* confirm_pw_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* ok_btn_ = nullptr;
    QPushButton* back_btn_ = nullptr;
  };

  /// Shows a transient SetInitialPasswordPage on the page stack.
  /**
     Captures the currently visible page as the return target.  After the
     credentials are applied (or Back is pressed) the page returns and
     @p on_done runs; the page deletes itself afterwards.
     @param data      central application state
     @param person_id the newly created person
     @param on_done   continuation run after the page is left
  */
  void run_set_initial_password_page(colman::AppData& data, int person_id,
                                     std::function<void()> on_done)
  {
    QWidget* const back = data.mw_.pages_stack_->currentWidget();
    auto* page = new SetInitialPasswordPage(data, person_id);
    data.mw_.pages_stack_->addWidget(page);
    page->set_on_done([&data, page, back, on_done]() {
      data.mw_.pages_stack_->setCurrentWidget(back);
      data.mw_.pages_stack_->removeWidget(page);
      page->deleteLater();
      if(on_done) {
        on_done();
      }
    });
    data.mw_.pages_stack_->setCurrentWidget(page);
  }

  /// Returns the set of permissions the current user may not grant.
  /**
   * A column is non-grantable when the logged-in user is not allowed to grant
   * it (see colman::may_grant_permission).  edit_persons holders may grant
   * everything they hold (empty set); delegated creators get only the
   * permissions of their profile plus the baseline.  The complete permission
   * list is read from the database so that every column is covered.
   *
   * @param queries   database queries
   * @param person_id the currently logged-in person
   * @return column names of permissions the user may not grant
   */
  QSet<QString> non_grantable_permissions_for(colman::IQueries& queries,
                                              int person_id)
  {
    QSet<QString> non_grantable;
    for(auto const& column : queries.get_permissions()) {
      if(!colman::may_grant_permission(queries, person_id, column)) {
        non_grantable.insert(column);
      }
    }
    return non_grantable;
  }

  /// Returns true if the logged-in user may create/edit the given person.
  /**
   * Wraps colman::may_manage_target; a non-positive \a person_id means no
   * target is selected, which is treated as manageable (creation of new
   * persons is gated separately via the new-person button visibility).
   * @param data      application state (queries and logged-in person)
   * @param person_id the selected target person, or <= 0 when none
   */
  bool may_manage_selected(colman::AppData& data, int person_id)
  {
    if(person_id <= 0) {
      return true;
    }
    return colman::may_manage_target(
        *data.queries_, data.current_data_collector_, person_id);
  }


  /// Builds a username suggestion from a first and last name.
  /**
     Concatenates the first letter of @p firstname with @p lastname,
     transliterates the common German umlauts, lowercases everything and drops
     any character that is not an ASCII letter or digit (spaces, punctuation and
     remaining accents).
     @param firstname the person's first name
     @param lastname  the person's last name
     @return the suggested username; may be empty if no usable characters remain
  */
  QString suggest_username(QString const& firstname, QString const& lastname)
  {
    QString raw = firstname.left(1) + lastname;
    raw = raw.toLower();
    raw.replace(QString::fromUtf8("ä"), "ae");
    raw.replace(QString::fromUtf8("ö"), "oe");
    raw.replace(QString::fromUtf8("ü"), "ue");
    raw.replace(QString::fromUtf8("ß"), "ss");
    // Decompose remaining accented letters (e.g. é -> e + accent) so the
    // accents can be dropped together with all other non-alphanumerics.
    QString const decomposed = raw.normalized(QString::NormalizationForm_KD);
    QString result;
    for(QChar const c : decomposed) {
      if((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
        result.append(c);
      }
    }
    return result;
  }


  /// enable "insert person" button, if name and a username are given
  /**
     Enables or disables the insert_person_ button.  A new lender needs both a
     valid name (last and first name filled in) and a non-empty username, since
     the username is assigned at creation time and is the sole login identifier.
     @param mw  the main window UI holding the button and person widget
  */
  void check_insert_person(colman::MainWindowUi& mw)
  {
    auto* const uname_edit =
        mw.person_page_->findChild<QLineEdit*>("person_username_edit_");
    bool const username_given =
        uname_edit != nullptr && !uname_edit->text().trimmed().isEmpty();
    mw.insert_person_->setEnabled(
        mw.person_person_->name_given() && username_given);
  }


  /// returns true, iff person has changed by user
  /**
     Compares the data currently entered in the PersonWidget against the
     person record stored in the database for the currently selected person.
     @param data  application state holding the database connection and UI
     @return true if the widget fields differ from the stored person record,
             false if no person is selected or data is unchanged
  */
  bool person_changed(colman::AppData& data)
  {
    int const person_id = data.mw_.person_person_list_->get_selected_person_id();
    if(person_id > 0) {
      colman::Person person;
      if(!data.queries_->get_person(person_id, person)) {
        return false;
      }
      return data.mw_.person_person_->modified(person);
    }
    return false;
  }


  /// enable "change person" button, if a person is selected
  /**
     Enables the change_person_ button only when a valid person is selected,
     the person widget contains a name, and the widget data differs from the
     stored record.  Disables the button otherwise.
     @param data  application state holding the database connection and UI
  */
  void check_change_person(colman::AppData& data)
  {
    int const pid = data.mw_.person_person_list_->get_selected_person_id();
    if(pid > 0 && data.mw_.person_person_->name_given() &&
       may_manage_selected(data, pid))
    {
      data.mw_.change_person_->setEnabled(person_changed(data));
    }
    else {
      data.mw_.change_person_->setEnabled(false);
    }
  }


  /**
     Enables the remove_person_ button if a person is currently selected in
     the list and the logged-in user may manage that person, disables it
     otherwise.
     @param data  application state holding the database connection and UI
  */
  void check_remove_person(colman::AppData& data)
  {
    int const pid = data.mw_.person_person_list_->get_selected_person_id();
    data.mw_.remove_person_->setEnabled(
        pid > 0 && may_manage_selected(data, pid));
  }


  /**
     Clears all input fields in the PersonWidget and updates the
     insert_person_ button state accordingly.
     @param mw  the main window UI
  */
  void clear_person(colman::MainWindowUi& mw)
  {
    mw.person_person_->clear_fields();
    check_insert_person(mw);
  }


  /**
     Populates the PersonWidget fields with the data from @p person.
     @param mw      the main window UI
     @param person  the person whose data should be displayed
  */
  void show_person(colman::MainWindowUi& mw, colman::Person const& person)
  {
    mw.person_person_->person_to_fields(person);
  }


  /**
     Loads the person identified by @p id from the database and displays it
     in the PersonWidget.  Also updates the enabled state of the remove,
     insert and change buttons to reflect that a person is now shown.
     @param data  application state holding the database connection and UI
     @param id    database id of the person to display; pass a negative value
                  to clear the widget and disable action buttons
  */
  void show_person(colman::AppData& data, int id)
  {
    colman::Person person;
    if(id >= 0) {
      (void)data.queries_->get_person(id, person);
    }
    show_person(data.mw_, person);
    data.mw_.remove_person_->setEnabled(id > 0 && may_manage_selected(data, id));
    data.mw_.insert_person_->setEnabled(false);
    data.mw_.change_person_->setEnabled(false);
  }


  /**
     Replaces the person list query model with a fresh query that returns all
     persons from the database.
     @param data  application state holding the database connection and model
  */
  void reset_person_query(colman::AppData& data)
  {
    data.mw_.person_person_list_->reset_person_query();
  }


  /**
     Shows the person identified by @p id, selects it in the person list
     widget, and updates all action buttons.  The change button is always
     disabled after selection because no edit has been made yet.
     @param data  application state holding the database connection and UI
     @param id    database id of the person to select and display
  */
  void select_person(colman::AppData& data, int id)
  {
    show_person(data, id);
    data.mw_.person_person_list_->select_person(id);
    check_insert_person(data.mw_);
    check_remove_person(data);
    data.mw_.change_person_->setEnabled(false);
  }


  /**
     Re-executes the person list query and updates the view to reflect the
     current database contents while preserving any active search filter.
     @param data  application state holding the database connection and model
  */
  void refresh_person_query(colman::AppData& data)
  {
    data.mw_.person_person_list_->refresh_person_query();
  }


  /// Removes the person with the given id from the database.
  /**
     writes a backup if necessary.
     @param data Application state containing the database and undo stack.
     @param id Database id of the person to remove.
     @return true if the removal succeeded, false otherwise.
   */
  bool remove_person(colman::AppData& data, int id)
  {
    colman::Person person;
    if(data.queries_->get_person(id, person)) {
      return data.queries_->remove_person(id);
    }
    return false;
  }


  /**
     Removes the currently selected person from the database.  Shows a
     warning dialog if the removal fails (e.g. because open lendings exist).
     Refreshes the person list and resets the action buttons on success.
     @param data  application state holding the database connection and UI
     @return true if the person was removed successfully, false otherwise
  */
  bool remove_person(colman::AppData& data)
  {
    int const id = data.mw_.person_person_list_->get_selected_person_id();
    if(id > 0) {
      if(::remove_person(data, id)) {
        refresh_person_query(data);
        clear_person(data.mw_);
        data.mw_.remove_person_->setEnabled(false);
        data.mw_.insert_person_->setEnabled(false);
        data.mw_.change_person_->setEnabled(false);
        return true;
      }
      else {
        QMessageBox::warning(data.mw_.person_page_,
                             QApplication::tr("Collection Manager"),
                             QApplication::tr("Could not remove person. "
                                              "Please return all "
                                              "lendings first and delete "
                                              "all closed lendings."));
      }
    }
    return false;
  }


  /// Offers the class list plus all classes used by persons to the filter.
  /**
     The person widget's class field keeps offering only the class list;
     the list filter must also offer classes that persons carry although
     they are missing from the class list.
     @param data  application state holding the database connection and UI
  */
  void update_class_names(colman::AppData& data)
  {
    auto const defined = data.queries_->load_class_names();
    data.mw_.person_person_->set_class_names(defined);
    data.mw_.person_person_list_->set_class_names(colman::merge_class_names(
        defined, data.queries_->query_persons(-1, "")));
  }


  /// Asks what to do if @p person's school class is not in the class list.
  /**
     Users allowed to manage classes may add the class to the class list,
     save the person without adding it, or cancel.  All other users can
     only save anyway or cancel.
     @param data    application state holding the database connection and UI
     @param person  the person about to be saved
     @return false if saving was cancelled
  */
  bool confirm_school_class(colman::AppData& data,
                            colman::Person const& person)
  {
    auto const& cls = person.get_school_class();
    if(cls.isEmpty() || data.queries_->load_class_names().contains(cls)) {
      return true;
    }
    int const uid = data.current_data_collector_;
    bool const may_add =
        0 == uid ||
        (uid > 0 && data.queries_->has_permission(uid, "manage_classes"));
    if(may_add) {
      auto const answer = QMessageBox::question(
          data.mw_.person_page_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("The class \"%1\" is not in the class list. "
                           "Add it to the class list?\n\n"
                           "\"No\" saves the person without adding the "
                           "class.")
              .arg(cls),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
          QMessageBox::Yes);
      if(QMessageBox::Yes == answer) {
        data.queries_->add_class_name(cls);
      }
      return QMessageBox::Cancel != answer;
    }
    return QMessageBox::Yes == colman::question_yes_no(
        data.mw_.person_page_,
        QApplication::tr("Collection Manager"),
        QApplication::tr("The class \"%1\" is not in the class list. "
                         "Save the person anyway?")
            .arg(cls));
  }


  /// Inserts a new person and returns its database id.
  /**
     Creates an undoable insert operation, writes a backup, and executes it.
     @param data Application state containing the database and undo stack.
     @param person Data of the person to insert.
     @return Database id of the newly inserted person, or -1 on failure.
   */
  int add_new_person(colman::AppData& data, colman::Person const& person)
  {
    colman::Person p = person;
    p.set_data_collector_id(data.current_data_collector_);
    return data.queries_->add_new_person(p);
  }


  /**
     Reads the person data from the PersonWidget and inserts a new person
     record into the database.  If a person with the same name already exists
     the user is asked to confirm the duplicate insertion.  On success the
     list is refreshed and the new person is selected.
     @param data  application state holding the database connection and UI
     @return the database id of the newly inserted person, or -1 if the
             operation was cancelled or failed
  */
  int insert_person(colman::AppData& data)
  {
    auto* const uname_edit =
        data.mw_.person_page_->findChild<QLineEdit*>("person_username_edit_");
    QString const username =
        uname_edit != nullptr ? uname_edit->text().trimmed() : QString();
    if(data.mw_.person_person_->get_lastname()
           .compare("Administrator", Qt::CaseInsensitive) == 0)
    {
      QMessageBox::warning(data.mw_.person_page_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("\"Administrator\" is a reserved "
                                            "name and cannot be used."));
      return -1;
    }
    if(data.queries_->person_known(data.mw_.person_person_->get_lastname(),
                                   data.mw_.person_person_->get_firstname()))
    {
      switch(colman::question_yes_no(
          data.mw_.person_page_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("There is already a person with this name."
                           " Really insert a second person with the "
                           "same name?")))
      {
      case QMessageBox::No:
        return -1;
      default:
        break;
      }
    }
    if(data.queries_->username_known(username))
    {
      QMessageBox::warning(data.mw_.person_page_,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("Username already taken."));
      return -1;
    }
    colman::Person person;
    data.mw_.person_person_->fields_to_person(person);
    if(data.queries_->email_known(person.get_emails()))
    {
      switch(colman::question_yes_no(
          data.mw_.person_page_,
          QApplication::tr("Collection Manager"),
          QApplication::tr("There is already a person with this email address."
                           " Really insert a second person with the "
                           "same email address?")))
      {
      case QMessageBox::No:
        return -1;
      default:
        break;
      }
    }
    if(!confirm_school_class(data, person)) {
      return -1;
    }
    int const retval = ::add_new_person(data, person);
    if(retval > 0 && !username.isEmpty()) {
      data.queries_->set_username(retval, username);
    }
    update_class_names(data);
    refresh_person_query(data);
    clear_person(data.mw_);
    select_person(data, retval);
    return retval;
  }


  /// Updates the data of an existing person.
  /**
     Creates an undoable change operation, writes a backup, and executes it.
     @param data Application state containing the database and undo stack.
     @param id Database id of the person to update.
     @param person New person data to store.
     @return true if the update succeeded, false otherwise.
   */
  bool change_person(colman::AppData& data, int id, colman::Person const& person)
  {
    colman::Person p = person;
    p.set_data_collector_id(data.current_data_collector_);
    return data.queries_->change_person(id, p);
  }


  /**
     Reads the person data from the PersonWidget and writes the changes to
     the database record identified by the currently selected person id.
     Refreshes the model and updates button states on success.
     @param data  application state holding the database connection and UI
     @return true if the person was changed successfully, false otherwise
  */
  bool change_person(colman::AppData& data)
  {
    int const id = data.mw_.person_person_list_->get_selected_person_id();
    if(id > 0) {
      colman::Person person;
      data.mw_.person_person_->fields_to_person(person);
      if(!confirm_school_class(data, person)) {
        return false;
      }
      if(::change_person(data, id, person)) {
        update_class_names(data);
        data.mw_.person_person_list_->refresh_person_query();
        check_insert_person(data.mw_);
        check_remove_person(data);
        data.mw_.change_person_->setEnabled(false);
        return true;
      }
    }
    return false;
  }


  /**
     Clears the person model and navigates back to the central start page.
     @param data  application state holding the UI stack and person model
  */
  void leave_person_page(colman::AppData& data)
  {
    data.person_model_.set_persons({});
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.central_page_);
  }


  /**
     Initialises the person page for a new visit: disables all action
     buttons, loads the full person list, clears the input widget, switches
     the stacked widget to person_page_, and focuses the search field.
     @param data  application state holding the database connection and UI
  */
  void enter_person_page(colman::AppData& data)
  {
    update_class_names(data);
    // Build the permission checkboxes now that the user is authenticated; in a
    // REST client get_permissions() returns nothing before login, so the
    // widget starts empty and is populated here on first entry.
    if(auto* pw = data.mw_.person_page_->findChild<colman::PermissionsWidget*>(
           "person_permissions_widget_")) {
      pw->reload_columns();
    }
    data.mw_.remove_person_->setEnabled(false);
    data.mw_.insert_person_->setEnabled(false);
    data.mw_.change_person_->setEnabled(false);
    reset_person_query(data);
    clear_person(data.mw_);
    data.mw_.pages_stack_->setCurrentWidget(data.mw_.person_page_);
    data.mw_.person_person_list_->clear_search_field();
    data.mw_.person_person_list_->focus_search_field();
  }

} // namespace


/**
   Refreshes the person page.  Resets all
   action buttons, reloads the full person list, clears the input widget
   and refocuses the search field.
   @param data  application state holding the database connection and UI
*/
void colman::refresh_person_page(colman::AppData& data)
{
  update_class_names(data);
  data.mw_.remove_person_->setEnabled(false);
  data.mw_.insert_person_->setEnabled(false);
  data.mw_.change_person_->setEnabled(false);
  reset_person_query(data);
  clear_person(data.mw_);
  if(auto* e = data.mw_.person_page_->findChild<QLineEdit*>(
         "person_external_id_edit_")) {
    e->clear();
  }
  if(auto* b = data.mw_.person_page_->findChild<QPushButton*>(
         "person_save_external_id_btn_")) {
    b->setEnabled(false);
  }
  data.mw_.person_person_list_->clear_search_field();
  data.mw_.person_person_list_->focus_search_field();
}


/**
   Sets up all signal/slot connections for the person page.  This function
   is called once at application startup.  It connects the back button,
   person list selection changes, field-change notifications, and the insert,
   remove and change buttons to their respective handlers.
   @param app   the QApplication instance used for connecting signals
   @param data  application state holding the database connection and UI
*/
void colman::setup_person_page(QApplication& app, colman::AppData& data)
{
  // Add page icon alongside existing heading label (vbox[0]=back, vbox[1]=label)
  {
    auto* vbox = qobject_cast<QVBoxLayout*>(
        data.mw_.person_page_->layout());
    QLayoutItem* heading_item = vbox->takeAt(1);
    QWidget* heading_widget = heading_item->widget();
    delete heading_item;
    QHBoxLayout* title_row = new QHBoxLayout;
    QLabel* icon_lbl = new QLabel(data.mw_.person_page_);
    QPixmap const pm(":/icons/owls/edit_persons.jpeg");
    if(!pm.isNull()) {
      icon_lbl->setPixmap(
          pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      icon_lbl->setFixedSize(52, 52);
      icon_lbl->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    }
    title_row->addWidget(icon_lbl);
    title_row->addWidget(heading_widget);
    title_row->addStretch();
    vbox->insertLayout(1, title_row);
  }

  // -- Username section (top of right column) --
  {
    auto current_username = std::make_shared<QString>();

    QHBoxLayout* uname_row = new QHBoxLayout;
    QLabel* uname_label =
        new QLabel(QApplication::tr("Username:"), data.mw_.person_page_);
    QLineEdit* uname_edit = new QLineEdit(data.mw_.person_page_);
    uname_edit->setObjectName("person_username_edit_");
    uname_edit->setPlaceholderText(
        QApplication::tr("No username set"));
    QPushButton* uname_set_btn = new QPushButton(
        QApplication::tr("Set Username"), data.mw_.person_page_);
    uname_set_btn->setEnabled(false);
    uname_row->addWidget(uname_label);
    uname_row->addWidget(uname_edit);
    uname_row->addWidget(uname_set_btn);
    data.mw_.verticalLayout_9->insertLayout(0, uname_row);

    auto check_uname_btn =
        [&data, uname_edit, uname_set_btn]() {
          int const pid =
              data.mw_.person_person_list_->get_selected_person_id();
          uname_set_btn->setEnabled(
              pid > 0 && may_manage_selected(data, pid) &&
              !uname_edit->text().trimmed().isEmpty());
        };

    app.connect(
        data.mw_.person_person_list_,
        &colman::PersonListWidget::selection_changed,
        [&data, uname_edit, current_username,
         check_uname_btn](int id) {
          if(id > 0) {
            *current_username = data.queries_->get_username(id);
            uname_edit->setText(*current_username);
          }
          else {
            current_username->clear();
            uname_edit->clear();
          }
          check_uname_btn();
        });

    app.connect(uname_edit, &QLineEdit::textChanged,
                [&data, check_uname_btn](QString const&) {
                  check_uname_btn();
                  // The username is required to create a new lender, so the
                  // insert button must react to changes in this field too.
                  check_insert_person(data.mw_);
                });

    app.connect(data.mw_.person_clear_, &QPushButton::clicked,
                [uname_edit, uname_set_btn, current_username]() {
                  current_username->clear();
                  uname_edit->clear();
                  uname_set_btn->setEnabled(false);
                });

    app.connect(
        uname_set_btn, &QPushButton::clicked,
        [&data, uname_edit, current_username]() {
          int const pid =
              data.mw_.person_person_list_->get_selected_person_id();
          if(pid <= 0) {
            return;
          }
          auto const new_uname = uname_edit->text().trimmed();
          if(new_uname.isEmpty()) {
            return;
          }
          if(!current_username->isEmpty()) {
            auto const reply = QMessageBox::question(
                data.mw_.person_page_,
                QApplication::tr("Change Username"),
                QApplication::tr(
                    "Change username from \"%1\" to \"%2\"?")
                    .arg(*current_username, new_uname),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Cancel);
            if(reply != QMessageBox::Yes) {
              return;
            }
          }
          if(!data.queries_->set_username(pid, new_uname)) {
            QMessageBox::warning(
                data.mw_.person_page_,
                QApplication::tr("Set Username"),
                QApplication::tr("Username already taken."));
            return;
          }
          *current_username = new_uname;
          data.mw_.person_person_list_->refresh_person_query();
        });
  }

  // -- Permissions section heading (right column) --
  {
    QHBoxLayout* perm_title = new QHBoxLayout;
    QLabel* perm_icon = new QLabel(data.mw_.person_page_);
    QPixmap const pm(":/icons/owls/permissions.jpeg");
    if(!pm.isNull()) {
      perm_icon->setPixmap(
          pm.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      perm_icon->setFixedSize(36, 36);
    }
    QLabel* perm_heading = new QLabel(
        QApplication::tr("Permissions"), data.mw_.person_page_);
    QFont f = perm_heading->font();
    f.setBold(true);
    perm_heading->setFont(f);
    perm_title->addWidget(perm_icon);
    perm_title->addWidget(perm_heading);
    perm_title->addStretch();
    data.mw_.verticalLayout_9->addLayout(perm_title);
  }

  // -- Inline permissions widget (right column) --
  auto* perms_w = new colman::PermissionsWidget(
      *data.queries_,
      /*read_only=*/false,
      /*person_id=*/-1,
      /*non_grantable=*/{},
      /*embedded=*/true,
      data.mw_.person_page_);
  perms_w->setObjectName("person_permissions_widget_");
  data.mw_.verticalLayout_9->addWidget(perms_w);

  app.connect(data.mw_.back_button2_, &QPushButton::clicked,
              [&data]() {
                leave_person_page(data);
              });
  data.mw_.person_person_list_->setup(data.person_model_);
  data.mw_.person_person_list_->enable_class_filter(true);
  data.mw_.person_person_list_->hide_column(
      colman::PersonListModel::TitleColumn);
  data.mw_.person_person_list_->set_query_function(
      [&data](QString const& s) {
        data.person_model_.set_persons(data.queries_->query_persons(-1, s));
      });
  app.connect(data.mw_.person_person_list_,
              &colman::PersonListWidget::selection_changed,
              [&data, perms_w](int id) {
                show_person(data, id);
                check_insert_person(data.mw_);
                check_change_person(data);
                check_remove_person(data);
                auto const non_grantable = non_grantable_permissions_for(
                    *data.queries_, data.current_data_collector_);
                perms_w->load_for_person(id, non_grantable);
                // Targets outside the logged-in user's manageable scope
                // (e.g. staff or admin accounts) are shown read-only.
                perms_w->set_read_only(!may_manage_selected(data, id));
              });
  app.connect(data.mw_.person_clear_, &QPushButton::clicked,
              [&data, perms_w]() {
                clear_person(data.mw_);
                data.mw_.person_person_list_->clear_search_field();
                data.mw_.person_person_list_->select_person(-1);
                data.mw_.remove_person_->setEnabled(false);
                data.mw_.change_person_->setEnabled(false);
                perms_w->load_for_person(-1, {});
              });
  app.connect(data.mw_.person_person_, &colman::PersonWidget::field_changed,
              [&data]() {
                // When entering a new lender (no person selected) and the
                // username field is still empty, propose one derived from the
                // name so the administrator does not have to invent it.
                if(auto* uname_edit =
                       data.mw_.person_page_->findChild<QLineEdit*>(
                           "person_username_edit_")) {
                  if(uname_edit->text().isEmpty() &&
                     data.mw_.person_person_->name_given() &&
                     data.mw_.person_person_list_->get_selected_person_id() <= 0)
                  {
                    uname_edit->setText(suggest_username(
                        data.mw_.person_person_->get_firstname(),
                        data.mw_.person_person_->get_lastname()));
                  }
                }
                check_insert_person(data.mw_);
                check_change_person(data);
              });
  app.connect(data.mw_.insert_person_, &QPushButton::clicked,
              [&data]() {
                int const new_id = insert_person(data);
                if(new_id > 0) {
                  run_set_initial_password_page(
                      data, new_id,
                      [&data]() { refresh_person_query(data); });
                }
              });
  app.connect(data.mw_.remove_person_, &QPushButton::clicked,
              [&data]() {
                ::remove_person(data);
              });
  app.connect(perms_w, &colman::PermissionsWidget::permissions_changed,
              [&data]() {
                if(data.mw_.person_person_list_->get_selected_person_id() > 0) {
                  data.mw_.change_person_->setEnabled(true);
                }
              });
  app.connect(data.mw_.change_person_, &QPushButton::clicked,
              [&data, perms_w]() {
                ::change_person(data);
                perms_w->save_permissions();
              });
  if(data.show_external_id_ui_) {
    // -- External ID (REST access) section --
    QHBoxLayout* eid_row = new QHBoxLayout;
    QLabel* eid_label = new QLabel(
        QApplication::tr("External ID:"), data.mw_.person_page_);
    QLineEdit* eid_edit = new QLineEdit(data.mw_.person_page_);
    eid_edit->setObjectName("person_external_id_edit_");
    eid_edit->setPlaceholderText(
        QApplication::tr("Authentik UUID (set by administrator)"));
    eid_edit->setReadOnly(false);
    QPushButton* eid_info_btn = new QPushButton(
        QApplication::tr("?"), data.mw_.person_page_);
    eid_info_btn->setFixedWidth(28);
    eid_info_btn->setToolTip(
        QApplication::tr("How to find the External ID"));
    QPushButton* eid_save_btn = new QPushButton(
        QApplication::tr("Save External ID"), data.mw_.person_page_);
    eid_save_btn->setObjectName("person_save_external_id_btn_");
    eid_save_btn->setEnabled(false);
    eid_row->addWidget(eid_label);
    eid_row->addWidget(eid_edit);
    eid_row->addWidget(eid_info_btn);
    eid_row->addWidget(eid_save_btn);
    data.mw_.verticalLayout_9->addLayout(eid_row);

    app.connect(eid_info_btn, &QPushButton::clicked,
                [&data]() {
                  QMessageBox::information(
                      data.mw_.person_page_,
                      QApplication::tr("How to find the External ID"),
                      QApplication::tr(
                          "The External ID links this person to their "
                          "Authentik account for REST API access.\n\n"
                          "How to find it in Authentik:\n"
                          "1. Log in to the Authentik admin UI.\n"
                          "2. Navigate to Directory → Users.\n"
                          "3. Click on the user to open their details.\n"
                          "4. Copy the UUID shown as \"External ID\" "
                          "or \"UUID\" in the user detail view\n"
                          "   (example: 123e4567-e89b-12d3-"
                          "a456-426614174000).\n"
                          "5. Paste the UUID into the External ID field "
                          "and click \"Save External ID\".\n\n"
                          "Without an External ID this person cannot "
                          "authenticate via the REST API.\n"
                          "Alternatively, enable link_by_email in the "
                          "server configuration to link accounts "
                          "automatically by e-mail address."));
                });

    app.connect(eid_save_btn, &QPushButton::clicked,
                [&data, eid_edit]() {
                  int const pid = data.mw_.person_person_list_
                                      ->get_selected_person_id();
                  if(pid <= 0) {
                    return;
                  }
                  data.queries_->set_person_external_id(
                      pid, eid_edit->text().trimmed());
                });

    app.connect(data.mw_.person_person_list_,
                &colman::PersonListWidget::selection_changed,
                [&data, eid_edit, eid_save_btn](int id) {
                  eid_save_btn->setEnabled(id > 0);
                  if(id > 0) {
                    colman::Person p;
                    data.queries_->get_person(id, p);
                    eid_edit->setText(p.get_external_id());
                  }
                  else {
                    eid_edit->clear();
                  }
                });

    app.connect(data.mw_.person_clear_, &QPushButton::clicked,
                [eid_edit, eid_save_btn]() {
                  eid_edit->clear();
                  eid_save_btn->setEnabled(false);
                });
  }

  app.connect(data.new_person_button_, &QToolButton::clicked,
              [&data]() {
                enter_person_page(data);
              });
}


/* EOF */
