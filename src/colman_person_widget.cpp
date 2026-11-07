// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PersonWidget.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person_widget.hpp"
#ifndef SYSTEM_QABSTRACTSCROLLAREA
#  include <QAbstractScrollArea>
#  define SYSTEM_QABSTRACTSCROLLAREA
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QEVENT
#  include <QEvent>
#  define SYSTEM_QEVENT
#endif
#ifndef SYSTEM_QCOMBOBOX
#  include <QComboBox>
#  define SYSTEM_QCOMBOBOX
#endif
#ifndef SYSTEM_QGRIDLAYOUT
#  include <QGridLayout>
#  define SYSTEM_QGRIDLAYOUT
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QLINEEDIT
#  include <QLineEdit>
#  define SYSTEM_QLINEEDIT
#endif
#ifndef SYSTEM_QPLAINTEXTEDIT
#  include <QPlainTextEdit>
#  define SYSTEM_QPLAINTEXTEDIT
#endif
#ifndef SYSTEM_QVBOXLAYOUT
#  include <QVBoxLayout>
#  define SYSTEM_QVBOXLAYOUT
#endif

namespace
{
  /// Stops a widget from reacting to the mouse wheel at all.
  /**
    The school-class combo box should never change its value when the
    user happens to scroll over it.  This filter swallows every wheel
    event and forwards it to the nearest scroll-area ancestor, so an
    enclosing list still scrolls while the combo box stays put — even
    when it has keyboard focus.
  */
  class WheelBlockFilter : public QObject
  {
  public:
    explicit WheelBlockFilter(QObject* parent = nullptr)
        : QObject(parent) {}
    bool eventFilter(QObject* obj, QEvent* event) override
    {
      if(event->type() == QEvent::Wheel) {
        QWidget* w = qobject_cast<QWidget*>(obj);
        QWidget* p = w ? w->parentWidget() : nullptr;
        while(p) {
          if(auto* sa = qobject_cast<QAbstractScrollArea*>(p)) {
            QCoreApplication::sendEvent(sa->viewport(), event);
            return true;
          }
          p = p->parentWidget();
        }
        return true;
      }
      return QObject::eventFilter(obj, event);
    }
  };

} // namespace

/// Constructs the PersonWidget instance.
/**
   @param parent parent widget, or nullptr for top-level.
*/
colman::PersonWidget::PersonWidget(QWidget* parent)
    : QWidget(parent)
{
  build_ui();
  connect(lastname_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(firstname_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(title_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(school_class_, &QComboBox::currentTextChanged,
          this, &PersonWidget::some_field_changed);
  connect(emails_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(telephone_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(street_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(zip_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(city_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(country_, &QLineEdit::editingFinished,
          this, &PersonWidget::some_field_changed);
  connect(remarks_, &QPlainTextEdit::textChanged,
          this, &PersonWidget::some_field_changed);
}


/// Builds the form layout and all input widgets.
void colman::PersonWidget::build_ui()
{
  setWindowTitle(tr("Person"));

  auto* outer_layout = new QVBoxLayout(this);
  auto* person_widget = new QWidget(this);
  outer_layout->addWidget(person_widget);

  auto* grid = new QGridLayout(person_widget);
  grid->setContentsMargins(0, 0, 0, 0);

  lastname_ = new QLineEdit(person_widget);
  firstname_ = new QLineEdit(person_widget);
  title_ = new QLineEdit(person_widget);
  school_class_ = new QComboBox(person_widget);
  school_class_->setEditable(true);
  school_class_->setInsertPolicy(QComboBox::NoInsert);
  // The wheel must not change the school class by accident.
  school_class_->setFocusPolicy(Qt::StrongFocus);
  school_class_->installEventFilter(new WheelBlockFilter(school_class_));
  emails_ = new QLineEdit(person_widget);
  emails_->setToolTip(tr("semicolon separated email addresses"));
  telephone_ = new QLineEdit(person_widget);
  telephone_->setToolTip(tr("semicolon separated telephone numbers"));
  street_ = new QLineEdit(person_widget);
  zip_ = new QLineEdit(person_widget);
  city_ = new QLineEdit(person_widget);
  country_ = new QLineEdit(person_widget);
  remarks_ = new QPlainTextEdit(person_widget);
  // Object names let the tutorial screenshot scenario locate the fields.
  lastname_->setObjectName("person_lastname_");
  firstname_->setObjectName("person_firstname_");
  title_->setObjectName("person_title_");
  school_class_->setObjectName("person_school_class_");
  emails_->setObjectName("person_emails_");
  telephone_->setObjectName("person_telephone_");
  street_->setObjectName("person_street_");
  zip_->setObjectName("person_zip_");
  city_->setObjectName("person_city_");
  country_->setObjectName("person_country_");
  remarks_->setObjectName("person_remarks_");

  int row = 0;
  grid->addWidget(new QLabel(tr("last name:"), person_widget), row, 0);
  grid->addWidget(lastname_, row++, 1);
  grid->addWidget(new QLabel(tr("first name:"), person_widget), row, 0);
  grid->addWidget(firstname_, row++, 1);
  grid->addWidget(new QLabel(tr("title:"), person_widget), row, 0);
  grid->addWidget(title_, row++, 1);
  grid->addWidget(new QLabel(tr("school class:"), person_widget), row, 0);
  grid->addWidget(school_class_, row++, 1);
  grid->addWidget(new QLabel(tr("emails:"), person_widget), row, 0);
  grid->addWidget(emails_, row++, 1);
  grid->addWidget(new QLabel(tr("telephone:"), person_widget), row, 0);
  grid->addWidget(telephone_, row++, 1);
  grid->addWidget(new QLabel(tr("street:"), person_widget), row, 0);
  grid->addWidget(street_, row++, 1);
  grid->addWidget(new QLabel(tr("zip:"), person_widget), row, 0);
  grid->addWidget(zip_, row++, 1);
  grid->addWidget(new QLabel(tr("city:"), person_widget), row, 0);
  grid->addWidget(city_, row++, 1);
  grid->addWidget(new QLabel(tr("country:"), person_widget), row, 0);
  grid->addWidget(country_, row++, 1);
  grid->addWidget(new QLabel(tr("remarks:"), person_widget), row, 0);
  grid->addWidget(remarks_, row++, 1);
}


/// Slot called when any input field changes; emits field_changed().
void colman::PersonWidget::some_field_changed()
{
  Q_EMIT field_changed();
}


/// Sets all input fields to read-only or editable mode.
/**
   @param ro true to make all fields read-only, false to make them editable.
*/
void colman::PersonWidget::set_read_only(bool ro)
{
  lastname_->setReadOnly(ro);
  firstname_->setReadOnly(ro);
  title_->setReadOnly(ro);
  school_class_->setEditable(!ro);
  school_class_->setEnabled(!ro);
  if(school_class_->lineEdit()) {
    school_class_->lineEdit()->setReadOnly(ro);
  }
  emails_->setReadOnly(ro);
  telephone_->setReadOnly(ro);
  street_->setReadOnly(ro);
  zip_->setReadOnly(ro);
  city_->setReadOnly(ro);
  country_->setReadOnly(ro);
  remarks_->setReadOnly(ro);
}


/// Populates all input fields from @p person.
/**
   @param person the person whose data to display.
*/
void colman::PersonWidget::person_to_fields(Person const& person)
{
  lastname_->setText(person.get_lastname());
  firstname_->setText(person.get_firstname());
  title_->setText(person.get_title());
  school_class_->setCurrentText(person.get_school_class());
  emails_->setText(person.get_emails());
  telephone_->setText(person.get_telephone());
  street_->setText(person.get_street());
  zip_->setText(person.get_zip());
  city_->setText(person.get_city());
  country_->setText(person.get_country());
  remarks_->setPlainText(person.get_remarks());
}


/// Clears all input fields.
void colman::PersonWidget::clear_fields()
{
  lastname_->clear();
  firstname_->clear();
  title_->clear();
  school_class_->setCurrentText(QString());
  emails_->clear();
  telephone_->clear();
  street_->clear();
  zip_->clear();
  city_->clear();
  country_->clear();
  remarks_->clear();
}


/// Reads all input fields into @p person.
/**
   @param person the Person object to populate from the widget fields.
*/
void colman::PersonWidget::fields_to_person(Person& person) const
{
  person.set_lastname(lastname_->text());
  person.set_firstname(firstname_->text());
  person.set_title(title_->text());
  person.set_school_class(school_class_->currentText());
  person.set_emails(emails_->text());
  person.set_telephone(telephone_->text());
  person.set_street(street_->text());
  person.set_zip(zip_->text());
  person.set_city(city_->text());
  person.set_country(country_->text());
  person.set_remarks(remarks_->toPlainText());
}


/// Returns true if the widget fields differ from @p person.
/**
   @param person the person to compare against.
   @return true if at least one field differs.
*/
bool colman::PersonWidget::modified(Person const& person) const
{
  return lastname_->text() != person.get_lastname() || firstname_->text() != person.get_firstname() || title_->text() != person.get_title() || school_class_->currentText() != person.get_school_class() || emails_->text() != person.get_emails() || telephone_->text() != person.get_telephone() || street_->text() != person.get_street() || zip_->text() != person.get_zip() || city_->text() != person.get_city() || country_->text() != person.get_country() || remarks_->toPlainText() != person.get_remarks();
}


/// Returns true if both last and first name fields are non-empty.
/**
   @return true if the last name and first name fields both contain text.
*/
bool colman::PersonWidget::name_given() const
{
  return lastname_->text().size() > 0 && firstname_->text().size() > 0;
}


/// Returns the text currently entered in the last-name field.
/**
   @return text of the last-name input field.
*/
QString colman::PersonWidget::get_lastname() const
{
  return lastname_->text();
}


/// Returns the text currently entered in the first-name field.
/**
   @return text of the first-name input field.
*/
QString colman::PersonWidget::get_firstname() const
{
  return firstname_->text();
}


/// Populates the school-class combo-box drop-down with \a names.
/**
   Preserves the current text in the editable line-edit.
   @param names sorted list of current class names
*/
void colman::PersonWidget::set_class_names(QStringList const& names)
{
  QString const current = school_class_->currentText();
  school_class_->clear();
  school_class_->addItems(names);
  school_class_->setCurrentText(current);
}

/* EOF */
