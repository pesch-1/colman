// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries persons, authentication, permissions, and passwords domain.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sql_queries_persons.hpp"
#ifndef COLMAN_SQL_QUERIES_IMPL_HPP
#  include "colman_sql_queries_impl.hpp"
#  define COLMAN_SQL_QUERIES_IMPL_HPP
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_PERMISSION_DEPS_HPP
#  include "colman_permission_deps.hpp"
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#  define COLMAN_PASSWORDS_HPP
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSQLRECORD
#  include <QSqlRecord>
#  define SYSTEM_QSQLRECORD
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

// -----------------------------------------------------------------------
// View-model queries
// -----------------------------------------------------------------------

/// Queries persons with password flag for the login page.
/**
  @param search Name prefix filter; empty means all persons.
  @return       Rows ordered by lastname, firstname.
*/
std::vector<colman::StartPagePersonListModel::Row> colman::SQLQueries::
    query_start_page_persons(QString const& search)
{
  QString sql("SELECT p.id, p.lastname, p.firstname, p.title, "
              "CASE WHEN c.password_hash IS NOT NULL THEN 1 ELSE 0 END "
              "FROM persons p "
              "LEFT JOIN credentials c ON c.person_id = p.id "
              "WHERE p.id > 0 ");
  if(!search.isEmpty()) {
    sql += "AND (p.lastname LIKE '" + like_escape(search) +
           "%' ESCAPE '\\' "
           "OR p.firstname LIKE '" +
           like_escape(search) + "%' ESCAPE '\\') ";
  }
  sql += "ORDER BY p.lastname, p.firstname";
  QSqlQuery q(db_);
  q.exec(sql);
  std::vector<StartPagePersonListModel::Row> rows;
  while(q.next()) {
    StartPagePersonListModel::Row r;
    r.id = q.value(0).toInt();
    r.lastname = q.value(1).toString();
    r.firstname = q.value(2).toString();
    r.title = q.value(3).toString();
    r.has_password = q.value(4).toBool();
    rows.push_back(std::move(r));
  }
  return rows;
}

/// Queries the persons table and returns the result as a list of Person
/// objects.
/**
  Selects id, lastname, firstname, title and school class from `persons`.
  When @p person_id is positive the result is restricted to that id.
  When @p search is non-empty it is tried as a person barcode first; if that
  fails a LIKE filter on lastname and firstname is applied.

  @param person_id If greater than zero, filters to this person id.
  @param search    Optional barcode or name prefix to filter persons.
  @return          Vector of Person objects ordered by lastname, firstname.
*/
std::vector<colman::Person> colman::SQLQueries::query_persons(
    int person_id, QString const& search)
{
  // id > 0 excludes the Administrator (id 0) from all person lists.
  QString sql("SELECT persons.id, persons.lastname, persons.firstname, "
              "persons.title, COALESCE(credentials.username, ''), "
              "COALESCE(persons.school_class, '') "
              "FROM persons "
              "LEFT JOIN credentials ON credentials.person_id = persons.id "
              "WHERE persons.id > 0 ");
  if(person_id > 0) {
    sql += "AND persons.id = " + QString().setNum(person_id) + " ";
  }
  else if(!search.isEmpty()) {
    Collection const collection = read_collection();
    if(colman::parse_person_barcode(collection, search, person_id)) {
      sql += "AND persons.id = " + QString().setNum(person_id) + " ";
    }
    else {
      sql += "AND (persons.lastname LIKE '" + like_escape(search) +
             "%' ESCAPE '\\' "
             "OR persons.firstname LIKE '" +
             like_escape(search) + "%' ESCAPE '\\') ";
    }
  }
  sql += "ORDER BY persons.lastname, persons.firstname";
  QSqlQuery q(sql, db_);
  std::vector<Person> result;
  while(q.next()) {
    Person p;
    p.set_id(q.value(0).toInt());
    p.set_lastname(q.value(1).toString());
    p.set_firstname(q.value(2).toString());
    p.set_title(q.value(3).toString());
    p.set_username(q.value(4).toString());
    p.set_school_class(q.value(5).toString());
    result.push_back(std::move(p));
  }
  return result;
}

/// Queries the ids of the most recently created persons.
/**
  Orders persons by their (monotonically increasing) id descending and
  returns at most \a count ids.  As the persons table carries no creation
  timestamp, the id serves as the creation-order proxy.  The Administrator
  (id 0) is excluded, matching query_persons().
  @param count maximum number of person ids to return
  @return person ids, newest first (at most \a count)
*/
std::vector<int> colman::SQLQueries::get_recently_created_person_ids(int count)
{
  std::vector<int> ret_val;
  if(count <= 0) {
    return ret_val;
  }
  QSqlQuery query(db_);
  query.prepare("SELECT id FROM persons WHERE id > 0 "
                "ORDER BY id DESC LIMIT :count");
  query.bindValue(":count", count);
  if(!query.exec()) {
    qWarning() << "get_recently_created_person_ids failed:" << query.lastError();
    return ret_val;
  }
  while(query.next()) {
    ret_val.push_back(query.value(0).toInt());
  }
  return ret_val;
}

/// Queries persons for the change-passwords admin page.
/**
  Returns the Administrator row (id 0) first, then all regular persons
  (id > 0) ordered by lastname, firstname.
  @return Vector of rows in display order.
*/
std::vector<colman::ChangePasswordsPersonListModel::Row> colman::SQLQueries::
    query_change_passwords_persons()
{
  QSqlQuery q(db_);
  if(!q.exec("SELECT 0 AS id, 0 AS sort_key,"
             "       'Administrator' AS lastname,"
             "       '' AS firstname, '' AS title,"
             "       (SELECT username FROM credentials"
             "         WHERE person_id = 0) AS username "
             "UNION ALL "
             "SELECT p.id, 1, p.lastname, p.firstname, p.title,"
             "       c.username "
             "  FROM persons p"
             "  LEFT JOIN credentials c ON c.person_id = p.id"
             "  WHERE p.id > 0 "
             "ORDER BY sort_key, lastname, firstname"))
  {
    qWarning() << "query_change_passwords_persons failed:" << q.lastError();
    return {};
  }
  std::vector<ChangePasswordsPersonListModel::Row> rows;
  while(q.next()) {
    ChangePasswordsPersonListModel::Row r;
    r.id = q.value(0).toInt();
    r.lastname = q.value(2).toString();
    r.firstname = q.value(3).toString();
    r.title = q.value(4).toString();
    r.username = q.value(5).toString();
    rows.push_back(std::move(r));
  }
  return rows;
}

QStringList colman::SQLQueries::query_person_field_completions(
    QString const& field)
{
  return completer_values_from(
      db_, get_table_columns("persons", 0), "persons", field);
}

// -----------------------------------------------------------------------
// Persons
// -----------------------------------------------------------------------

/// Inserts a person with a caller-supplied id.
/**
 * Used by add_new_person() to insert a
 * person row with an explicit \a id.  Also updates max_person_id in the
 * collection table when \a id exceeds the stored maximum.  All changes
 * are wrapped in a single transaction.
 * @param id     the person id to use
 * @param person the person data to insert
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::add_person(int id, Person const& person)
{
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare(
      "INSERT INTO persons (id, lastname, firstname, title, school_class, "
      "emails, telephonenumbers, street, "
      "zip, city, country, remarks, thumbnail, data_collector) VALUES "
      "(:id, :lastname, :firstname, :title, :school_class, "
      ":emails, :telephonenumbers, :street, "
      ":zip, :city, :country, :remarks, :thumbnail, :data_collector)");
  query.bindValue(":id", id);
  query.bindValue(":lastname", person.get_lastname());
  query.bindValue(":firstname", person.get_firstname());
  query.bindValue(":title", person.get_title());
  query.bindValue(":school_class", person.get_school_class());
  query.bindValue(":emails", person.get_emails());
  query.bindValue(":telephonenumbers", person.get_telephone());
  query.bindValue(":street", person.get_street());
  query.bindValue(":zip", person.get_zip());
  query.bindValue(":city", person.get_city());
  query.bindValue(":country", person.get_country());
  query.bindValue(":remarks", person.get_remarks());
  query.bindValue(":thumbnail", person.get_thumbnail_id());
  query.bindValue(":data_collector", person.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "could not insert person: " << query.lastError();
    return false;
  }
  Collection collection = read_collection();
  if(id > collection.get_max_person_id()) {
    query.prepare("UPDATE collection SET value = :max WHERE key = :key");
    query.bindValue(":key", QString("max_person_id"));
    query.bindValue(":max", id);
    if(!query.exec()) {
      qWarning() << "could not update max person id: " << query.lastError();
      return false;
    }
    invalidate_collection_cache();
  }
  rollback.commit();
  return true;
}

/// Inserts a new person and returns the assigned person id.
/**
 * Generates a new person id from the collection's max_person_id counter
 * and delegates to add_person().
 * @param person the person data to insert
 * @return the new person id on success, or -1 on failure
 */
int colman::SQLQueries::add_new_person(Person const& person)
{
  Collection const collection = read_collection();
  int const pid = collection.new_person_id();
  if(!add_person(pid, person)) {
    return -1;
  }
  create_default_permissions(pid);
  backup_after_change();
  return pid;
}

/// Reads a person record by numeric id.
/**
 * Queries the persons table for the row with \a id and populates
 * \a person with all stored fields.
 * @param id     the database id of the person to read
 * @param person output: populated with the retrieved person data
 * @return true if the person was found, false otherwise
 */
bool colman::SQLQueries::get_person(int id, Person& person)
{
  QSqlQuery query(db_);
  query.prepare("SELECT p.lastname, p.firstname, p.title, p.school_class, "
                "p.emails, p.telephonenumbers, p.street, p.zip, p.city, "
                "p.country, p.remarks, p.thumbnail, p.data_collector, "
                "c.external_id, p.version "
                "FROM persons p "
                "LEFT JOIN credentials c ON c.person_id = p.id "
                "WHERE p.id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "could not query person " << id << ": " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown person id " << id;
    return false;
  }
  person.set_lastname(query.value(0).toString());
  person.set_firstname(query.value(1).toString());
  person.set_title(query.value(2).toString());
  person.set_school_class(query.value(3).toString());
  person.set_emails(query.value(4).toString());
  person.set_telephone(query.value(5).toString());
  person.set_street(query.value(6).toString());
  person.set_zip(query.value(7).toString());
  person.set_city(query.value(8).toString());
  person.set_country(query.value(9).toString());
  person.set_remarks(query.value(10).toString());
  person.set_thumbnail_id(query.value(11).toInt());
  person.set_data_collector_id(query.value(12).toInt());
  person.set_external_id(query.value(13).toString());
  person.set_version(query.value(14).toInt());
  person.set_id(id);
  return true;
}

/**
   @deprecated use parse_person_barcode, instead
*/
int colman::SQLQueries::get_person(QString const& barcode)
{
  int pid = -1;
  (void)parse_person_barcode(barcode, pid);
  return pid;
}

/// Updates all stored fields of an existing person.
/**
 * Overwrites the persons table row for \a id with the data from \a person.
 * @param id     the database id of the person to update
 * @param person the new person data
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::change_person(int id, Person const& person)
{
  if(id == 0) {
    qWarning() << "change_person: Administrator (id 0) must not be modified";
    return false;
  }
  QSqlQuery query(db_);
  query.prepare("UPDATE persons SET lastname = :lastname, "
                "firstname = :firstname, title = :title, "
                "school_class = :school_class, "
                "emails = :emails, "
                "telephonenumbers = :telephonenumbers, street = :street, "
                "zip = :zip, city = :city, "
                "country = :country, remarks = :remarks, "
                "thumbnail = :thumbnail, "
                "data_collector = :data_collector "
                "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":lastname", person.get_lastname());
  query.bindValue(":firstname", person.get_firstname());
  query.bindValue(":title", person.get_title());
  query.bindValue(":school_class", person.get_school_class());
  query.bindValue(":emails", person.get_emails());
  query.bindValue(":telephonenumbers", person.get_telephone());
  query.bindValue(":street", person.get_street());
  query.bindValue(":zip", person.get_zip());
  query.bindValue(":city", person.get_city());
  query.bindValue(":country", person.get_country());
  query.bindValue(":remarks", person.get_remarks());
  query.bindValue(":thumbnail", person.get_thumbnail_id());
  query.bindValue(":data_collector", person.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "could not change person: " << query.lastError();
    return false;
  }
  backup_after_change();
  return true;
}

/// Returns true if a person with the given name exists in the database.
/**
 * Queries the persons table for a row matching both \a lastname and
 * \a firstname.
 * @param lastname  the last name to search for
 * @param firstname the first name to search for
 * @return true if at least one matching person is found, false otherwise
 */
bool colman::SQLQueries::person_known(QString const& lastname,
                                      QString const& firstname)
{
  QSqlQuery query(db_);
  query.prepare("SELECT lastname, firstname "
                "FROM persons "
                "WHERE lastname = :lastname AND firstname = :firstname");
  query.bindValue(":lastname", lastname);
  query.bindValue(":firstname", firstname);
  if(!query.exec()) {
    qWarning() << "select person failed: " << query.lastError();
    return false;
  }
  return query.next();
}

/// Returns true if any person already has \a emails as their email field.
/**
 * Used by insert_person() to warn the user before creating a second account
 * with the same email address (which is legitimate but unusual).
 * @param emails the email string to look up (exact match)
 * @return true if at least one existing person has the same emails value
 */
bool colman::SQLQueries::email_known(QString const& emails)
{
  if(emails.isEmpty()) {
    return false;
  }
  QSqlQuery query(db_);
  query.prepare("SELECT 1 FROM persons WHERE emails = :emails LIMIT 1");
  query.bindValue(":emails", emails);
  if(!query.exec()) {
    qWarning() << "email_known query failed: " << query.lastError();
    return false;
  }
  return query.next();
}

/// Returns true if \a username is already taken by any person.
/**
 * Used by insert_person() to reject a new lender whose username collides
 * with an existing account before the person row is created.
 * @param username the username to look up (exact match)
 * @return true if at least one credentials row already uses this username
 */
bool colman::SQLQueries::username_known(QString const& username)
{
  if(username.isEmpty()) {
    return false;
  }
  QSqlQuery query(db_);
  query.prepare(
      "SELECT 1 FROM credentials WHERE username = :username LIMIT 1");
  query.bindValue(":username", username);
  if(!query.exec()) {
    qWarning() << "username_known query failed: " << query.lastError();
    return false;
  }
  return query.next();
}

/// Removes a person and all their lending history from the database.
/**
 * Refuses to remove the person if they have any current lendings.
 * Otherwise deletes the persons row, all associated returns, and all
 * associated lendings.  All changes are wrapped in a single transaction.
 * @param id the database id of the person to remove
 * @return true on success, false if the person has active lendings or a
 *         database error occurs
 */
bool colman::SQLQueries::remove_person(int id)
{
  if(id == 0) {
    qWarning() << "remove_person: Administrator (id 0) must not be removed";
    return false;
  }
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("SELECT person, count FROM lendings "
                "WHERE person = :person");
  query.bindValue(":person", id);
  if(!query.exec()) {
    qWarning() << "select lendings for person failed: " << query.lastError();
    return false;
  }
  if(query.next()) {
    // person still has some current lendings, we are not allowed to remove
    return false;
  }

  // Delete referencing rows before the person row (FK constraints).
  query.prepare("DELETE FROM credentials WHERE person_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting credentials for person failed: "
               << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM permissions WHERE person_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting permissions for person failed: "
               << query.lastError();
    return false;
  }

  query.prepare("SELECT id FROM lendings WHERE person = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "query lendings for person failed: " << query.lastError();
    return false;
  }
  while(query.next()) {
    int const lending = query.value(0).toInt();
    QSqlQuery inner(db_);
    inner.prepare("DELETE FROM returns WHERE lending = :lending");
    inner.bindValue(":lending", lending);
    if(!inner.exec()) {
      qWarning() << "deleting person from returns failed: "
                 << inner.lastError();
    }
  }
  query.prepare("DELETE FROM lendings WHERE person = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting person from lendings failed: " << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM persons WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting person failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  backup_after_change();
  return true;
}

/// Returns true if any person other than the Administrator (id 0) exists.
/**
 * @return true when at least one person with id > 0 is in the persons table
 */
bool colman::SQLQueries::has_non_admin_persons()
{
  QSqlQuery query(db_);
  query.prepare("SELECT COUNT(*) FROM persons WHERE id > 0");
  if(!query.exec() || !query.next()) {
    return false;
  }
  return query.value(0).toInt() > 0;
}

/// Computes the barcode string for a person using the collection's prefix.
/**
 * Reads the collection from \a db to obtain the barcode prefix, then
 * delegates to the Collection overload of compute_person_barcode().
 * @param person_id the database id of the person
 * @return the barcode string for the person
 */
QString colman::SQLQueries::compute_person_barcode(int person_id)
{
  Collection col = read_collection();
  return colman::compute_person_barcode(col, person_id);
}

/// Returns true if \a str is a person barcode for this collection.
/**
 * Reads the collection from \a db to obtain the barcode prefix, then
 * delegates to the Collection overload of is_person_barcode().
 * @param str the barcode string to test
 * @return true if \a str encodes a person belonging to this collection
 */
bool colman::SQLQueries::is_person_barcode(QString const& str)
{
  Collection col = read_collection();
  return colman::is_person_barcode(col, str);
}

/// Parses a person barcode string and extracts the person id.
/**
 * Reads the collection from \a db to obtain the barcode prefix, then
 * delegates to the Collection overload of parse_person_barcode().
 * @param str the barcode string to parse
 * @param person_id  output: the person id encoded in the barcode
 * @return true if \a str is a valid person barcode for this collection
 */
bool colman::SQLQueries::parse_person_barcode(QString const& str,
                                              int& person_id)
{
  Collection col = read_collection();
  return colman::parse_person_barcode(col, str, person_id);
}

// -----------------------------------------------------------------------
// Permissions
// -----------------------------------------------------------------------

std::vector<QString> colman::SQLQueries::get_permissions() const
{
  return {"edit_persons", "create_lenders",
          "create_data_collectors",
          "collect_lendings", "collect_returns",
          "collect_acquisitions",
          "collect_liquidations", "view_history",
          "view_persons",
          "view_items", "edit_preferences",
          "print_ids",
          "print_barcodes", "edit_categories",
          "view_personal_data",
          "manage_label_layouts", "calibrate_printers",
          "edit_id_layout",
          "print_signatures", "change_actions",
          "edit_program_settings",
          "manage_classes",
          "reserve", "edit_reservations",
          "admin_permissions"};
}

/// Queries all permission flags for a person.
/**
  Looks up the permissions row for \a person_id.  If no row exists,
  creates default permissions first and retries.

  @param person_id the database id of the person to query
  @return vector of permission bools in the order defined by get_permissions()
*/
std::vector<bool> colman::SQLQueries::get_user_permissions(int person_id)
{
  std::vector<bool> ret_val(get_permissions().size(), false);
  QSqlQuery q(db_);
  QString query("SELECT ");
  bool first = true;
  for(auto const& perm : get_permissions()) {
    if(first) {
      first = false;
    }
    else {
      query += ", ";
    }
    query += perm;
  }
  query += " FROM permissions WHERE person_id = :pid";
  q.prepare(query);
  q.bindValue(":pid", person_id);
  if(q.exec() && q.next()) {
    auto const n = ret_val.size();
    for(auto i = decltype(n){0}; i < n; ++i) {
      ret_val[i] = q.value(i).toBool();
    }
  }
  else {
    // No permissions row yet — create default permissions and reload.
    create_default_permissions(person_id);
    QSqlQuery q2(db_);
    q2.prepare(query);
    q2.bindValue(":pid", person_id);
    if(q2.exec() && q2.next()) {
      auto const n = ret_val.size();
      for(auto i = decltype(n){0}; i < n; ++i) {
        ret_val[i] = q2.value(i).toBool();
      }
    }
  }
  return ret_val;
}

/// Creates default permissions for a person.
/**
 * Inserts a row into the permissions table.  For the Administrator
 * (person_id 0) every permission is set to true.  For all other persons
 * only view_personal_data is true; every other flag is false.
 * @param person_id the person to create permissions for
 * @return true on success
 */
bool colman::SQLQueries::create_default_permissions(int person_id)
{
  bool const is_admin = (person_id == 0);
  QSqlQuery query(db_);
  query.prepare(
      "INSERT INTO permissions "
      "(person_id, edit_persons, create_lenders, create_data_collectors, "
      "collect_lendings, collect_returns, collect_acquisitions, "
      "collect_liquidations, view_history, view_persons, view_items, "
      "edit_preferences, print_ids, print_barcodes, edit_categories, "
      "view_personal_data, manage_label_layouts, calibrate_printers, "
      "edit_id_layout, print_signatures, change_actions, "
      "edit_program_settings, manage_classes, reserve, "
      "edit_reservations, admin_permissions) VALUES "
      "(:person_id, :edit_persons, :create_lenders, "
      ":create_data_collectors, :collect_lendings, :collect_returns, "
      ":collect_acquisitions, :collect_liquidations, :view_history, "
      ":view_persons, :view_items, :edit_preferences, :print_ids, "
      ":print_barcodes, :edit_categories, :view_personal_data, "
      ":manage_label_layouts, :calibrate_printers, :edit_id_layout, "
      ":print_signatures, :change_actions, :edit_program_settings, "
      ":manage_classes, :reserve, :edit_reservations, "
      ":admin_permissions)");
  query.bindValue(":person_id", person_id);
  query.bindValue(":edit_persons", is_admin);
  query.bindValue(":create_lenders", is_admin);
  query.bindValue(":create_data_collectors", is_admin);
  query.bindValue(":collect_lendings", is_admin);
  query.bindValue(":collect_returns", is_admin);
  query.bindValue(":collect_acquisitions", is_admin);
  query.bindValue(":collect_liquidations", is_admin);
  query.bindValue(":view_history", is_admin);
  query.bindValue(":view_persons", is_admin);
  query.bindValue(":view_items", is_admin);
  query.bindValue(":edit_preferences", is_admin);
  query.bindValue(":print_ids", is_admin);
  query.bindValue(":print_barcodes", is_admin);
  query.bindValue(":edit_categories", is_admin);
  query.bindValue(":view_personal_data", true);
  query.bindValue(":manage_label_layouts", is_admin);
  query.bindValue(":calibrate_printers", is_admin);
  query.bindValue(":edit_id_layout", is_admin);
  query.bindValue(":print_signatures", is_admin);
  query.bindValue(":change_actions", is_admin);
  query.bindValue(":edit_program_settings", is_admin);
  query.bindValue(":manage_classes", is_admin);
  query.bindValue(":reserve", true);
  query.bindValue(":edit_reservations", is_admin);
  query.bindValue(":admin_permissions", is_admin);
  if(!query.exec()) {
    qWarning() << "creating default permissions failed: " << query.lastError();
    return false;
  }
  return true;
}

/// Reads a single permission flag for a person.
/**
 * @param person_id the person whose permission is queried
 * @param column    the permission column name (e.g. "edit_persons")
 * @return true if the permission is granted, false otherwise or on error
 */
bool colman::SQLQueries::has_permission(int person_id, QString const& column)
{
  auto const allowed = get_permissions();
  if(std::find(allowed.begin(), allowed.end(), column) == allowed.end()) {
    qWarning() << "has_permission: unknown column" << column;
    return false;
  }
  QSqlQuery query(db_);
  query.prepare(QString("SELECT %1 FROM permissions "
                        "WHERE person_id = :person_id")
                    .arg(column));
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "querying permission failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    return false;
  }
  return query.value(0).toBool();
}

/// Sets a single permission flag for a person.
/**
 * @param person_id the person whose permission is changed
 * @param column    the permission column name (e.g. "edit_persons")
 * @param value     the new value for the permission
 * @return true on success
 */
bool colman::SQLQueries::set_permission(int person_id, QString const& column,
                                        bool value)
{
  auto const allowed = get_permissions();
  if(std::find(allowed.begin(), allowed.end(), column) == allowed.end()) {
    qWarning() << "set_permission: unknown column" << column;
    return false;
  }
  // Apply the dependency cascade so the permission invariant from
  // get_permission_dependencies() always holds, regardless of the entry
  // point (UI cascade, REST, direct call): granting a right also grants its
  // prerequisites, revoking one also revokes its dependents.
  auto const affected = colman::permission_cascade(column, value);
  QStringList assignments;
  for(auto const& col : affected) {
    if(std::find(allowed.begin(), allowed.end(), col) != allowed.end()) {
      assignments << QString("%1 = :value").arg(col);
    }
  }
  QSqlQuery query(db_);
  query.prepare(QString("UPDATE permissions SET %1 "
                        "WHERE person_id = :person_id")
                    .arg(assignments.join(QStringLiteral(", "))));
  query.bindValue(":value", value ? 1 : 0);
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "setting permission failed: " << query.lastError();
    return false;
  }
  return true;
}

/// Removes all permissions for a person.
/**
 * @param person_id the person whose permissions are removed
 * @return true on success
 */
bool colman::SQLQueries::remove_permissions(int person_id)
{
  QSqlQuery query(db_);
  query.prepare("DELETE FROM permissions WHERE person_id = :person_id");
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "removing permissions failed: " << query.lastError();
    return false;
  }
  return true;
}

/// Returns true if \a person_id has at most reserve-only permissions.
/**
 * Checks that every permission column other than view_personal_data, reserve,
 * and view_items is false.  Returns false if no permissions row exists.
 */
bool colman::SQLQueries::is_reserve_only(int person_id)
{
  QSqlQuery query(db_);
  query.prepare(
      "SELECT (edit_persons OR create_lenders OR create_data_collectors OR "
      "collect_lendings OR collect_returns OR collect_acquisitions OR "
      "collect_liquidations OR view_history OR view_persons OR "
      "edit_preferences OR print_ids OR print_barcodes OR "
      "edit_categories OR manage_label_layouts OR calibrate_printers OR "
      "edit_id_layout OR print_signatures OR change_actions OR "
      "edit_program_settings OR manage_classes OR "
      "edit_reservations OR admin_permissions) = 0 "
      "FROM permissions WHERE person_id = :person_id");
  query.bindValue(":person_id", person_id);
  if(!query.exec() || !query.next()) {
    return false;
  }
  return query.value(0).toBool();
}

/// Adds missing permission prerequisites for every person in the database.
// -----------------------------------------------------------------------
// Passwords
// -----------------------------------------------------------------------

/// Sets the password hash for a person, creating or replacing the entry.
/**
 * Inserts or replaces the row in the passwords table for \a person_id
 * with the given \a hash.
 * @param person_id the database id of the person
 * @param hash      the password hash to store
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::set_password(int person_id, QString const& hash)
{
  QSqlQuery query(db_);
  query.prepare(
      "INSERT INTO credentials (person_id, password_hash) "
      "VALUES (:person_id, :hash) "
      "ON CONFLICT(person_id) DO UPDATE SET "
      "password_hash = excluded.password_hash");
  query.bindValue(":person_id", person_id);
  query.bindValue(":hash", hash);
  if(!query.exec()) {
    qWarning() << "setting password failed: " << query.lastError();
    return false;
  }
  return true;
}

/// Reads the stored password hash for a person.
/**
 * @param person_id the database id of the person
 * @param hash      output: the stored hash, unchanged if not found
 * @return true if a hash was found, false if no entry exists or a
 *         database error occurs
 */
bool colman::SQLQueries::get_password(int person_id, QString& hash)
{
  QSqlQuery query(db_);
  query.prepare(
      "SELECT password_hash FROM credentials "
      "WHERE person_id = :person_id AND password_hash IS NOT NULL");
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "querying password failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    return false;
  }
  hash = query.value(0).toString();
  return true;
}

/// Changes a person's own password after verifying the current one.
/**
 * @param person_id    the database id of the person
 * @param old_password the current password in clear text
 * @param new_hash     the new password, already Argon2id-hashed
 * @return true if the old password matched and the new hash was stored
 */
bool colman::SQLQueries::change_own_password(int person_id,
                                             QString const& old_password,
                                             QString const& new_hash)
{
  QString stored;
  if(!get_password(person_id, stored)) {
    return false;
  }
  if(!colman::verify_password(old_password, stored)) {
    return false;
  }
  return set_password(person_id, new_hash);
}

/// Returns true if a password entry exists for \a person_id.
/**
 * @param person_id the database id of the person to check
 * @return true if a password row exists, false otherwise
 */
bool colman::SQLQueries::has_password(int person_id)
{
  QSqlQuery query(db_);
  query.prepare(
      "SELECT 1 FROM credentials "
      "WHERE person_id = :person_id AND password_hash IS NOT NULL LIMIT 1");
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "checking password failed: " << query.lastError();
    return false;
  }
  return query.next();
}

/// Removes the password entry for \a person_id.
/**
 * @param person_id the database id of the person
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::remove_password(int person_id)
{
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE credentials SET password_hash = NULL WHERE person_id = :person_id");
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "removing password failed: " << query.lastError();
    return false;
  }
  return true;
}

// -----------------------------------------------------------------------
// Authentication and identity
// -----------------------------------------------------------------------

/// Returns the person_id whose external_id equals \a sub.
/**
 * Used by the REST auth middleware to map an Authentik subject claim to a
 * colman person.
 * @param sub  the Authentik subject UUID
 * @return person_id on success, -1 if not found or on error
 */
int colman::SQLQueries::get_person_by_external_id(QString const& sub)
{
  QSqlQuery q(db_);
  q.prepare(
      "SELECT person_id FROM credentials WHERE external_id = :sub LIMIT 1");
  q.bindValue(":sub", sub);
  if(!q.exec() || !q.next()) {
    return -1;
  }
  return q.value(0).toInt();
}

/// Stores \a sub as the external_id for an existing person.
/**
 * Links an Authentik user (identified by their subject UUID) to a colman
 * person record.
 * @param person_id  the colman person to update
 * @param sub        the Authentik subject UUID to store
 * @return true on success, false on error
 */
bool colman::SQLQueries::set_person_external_id(int person_id,
                                                QString const& sub)
{
  QSqlQuery q(db_);
  q.prepare(
      "INSERT INTO credentials (person_id, external_id) "
      "VALUES (:id, :sub) "
      "ON CONFLICT(person_id) DO UPDATE SET external_id = excluded.external_id");
  q.bindValue(":sub", sub);
  q.bindValue(":id", person_id);
  if(!q.exec()) {
    qWarning() << "set_person_external_id failed: " << q.lastError();
    return false;
  }
  return true;
}

/// Checks local REST credentials and returns the person_id on success.
/**
 * @param username  REST username to look up
 * @param password  plaintext password to verify against the stored Argon2 hash
 * @return person_id on success, -1 on wrong credentials or database error
 */
int colman::SQLQueries::check_local_credentials(
    QString const& username, QString const& password)
{
  QSqlQuery q(db_);
  q.prepare(
      "SELECT person_id, password_hash FROM credentials "
      "WHERE username = :username AND password_hash IS NOT NULL LIMIT 1");
  q.bindValue(":username", username);
  if(!q.exec() || !q.next()) {
    return -1;
  }
  int const person_id = q.value(0).toInt();
  QString const hash = q.value(1).toString();
  if(!verify_password(password, hash)) {
    return -1;
  }
  return person_id;
}

/// Creates or replaces the local REST account for \a person_id.
/**
 * @param person_id    the colman person to assign credentials to
 * @param username     unique login name
 * @param password_hash Argon2 hash of the password (use hash_password())
 * @return true on success
 */
bool colman::SQLQueries::set_local_credentials(
    int person_id, QString const& username, QString const& password_hash)
{
  QSqlQuery q(db_);
  q.prepare(
      "INSERT INTO credentials (person_id, username, password_hash) "
      "VALUES (:person_id, :username, :hash) "
      "ON CONFLICT(person_id) DO UPDATE SET "
      "username = excluded.username, "
      "password_hash = excluded.password_hash");
  q.bindValue(":person_id", person_id);
  q.bindValue(":username", username);
  q.bindValue(":hash", password_hash);
  if(!q.exec()) {
    qWarning() << "set_local_credentials failed: " << q.lastError();
    return false;
  }
  return true;
}

/// Removes the local REST account for \a person_id.
/**
 * @return true if a row was deleted, false on error or if no row existed
 */
bool colman::SQLQueries::delete_local_credentials(int person_id)
{
  QSqlQuery q(db_);
  q.prepare(
      "UPDATE credentials SET username = NULL, password_hash = NULL "
      "WHERE person_id = :id");
  q.bindValue(":id", person_id);
  if(!q.exec()) {
    qWarning() << "delete_local_credentials failed: " << q.lastError();
    return false;
  }
  return true;
}

/// Returns the username stored for \a person_id, or an empty string.
QString colman::SQLQueries::get_username(int person_id)
{
  QSqlQuery q(db_);
  q.prepare(
      "SELECT username FROM credentials WHERE person_id = :id");
  q.bindValue(":id", person_id);
  if(!q.exec() || !q.next()) {
    return {};
  }
  return q.value(0).toString();
}

/// Stores \a username for \a person_id (upsert).
/**
 * @return false if the username is already taken by another person
 */
bool colman::SQLQueries::set_username(int person_id,
                                      QString const& username)
{
  QSqlQuery q(db_);
  q.prepare(
      "INSERT INTO credentials (person_id, username) "
      "VALUES (:person_id, :username) "
      "ON CONFLICT(person_id) DO UPDATE SET username = excluded.username");
  q.bindValue(":person_id", person_id);
  q.bindValue(":username", username);
  if(!q.exec()) {
    qWarning() << "set_username failed: " << q.lastError();
    return false;
  }
  return true;
}

/// Clears the username for \a person_id.
bool colman::SQLQueries::remove_username(int person_id)
{
  QSqlQuery q(db_);
  q.prepare(
      "UPDATE credentials SET username = NULL WHERE person_id = :id");
  q.bindValue(":id", person_id);
  if(!q.exec()) {
    qWarning() << "remove_username failed: " << q.lastError();
    return false;
  }
  return true;
}

// -----------------------------------------------------------------------
// Private helpers
/* EOF */
