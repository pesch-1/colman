// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries lendings, returns, acquisitions, and liquidations domain.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sql_queries_movements.hpp"
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
#ifndef COLMAN_ISBN_QUERIES_HPP
#  include "colman_isbn_queries.hpp"
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
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QSET
#  include <QSet>
#  define SYSTEM_QSET
#endif
#ifndef SYSTEM_QHASH
#  include <QHash>
#  define SYSTEM_QHASH
#endif

/// Builds the shared FROM/JOIN fragment used by lending-related queries.
/**
   @param person_id     If >= 0, filters to this person id.
   @param item_id       If >= 0, filters to this item id.
   @param lending_id    If >= 0, filters to this lending id.
   @param person_search Barcode or name prefix used to search for persons.
   @param item_search   Barcode, ISBN or name prefix used to search for items.
   @param show_returned When false, only open (not fully returned) lendings
   are included; when true all lendings are included.
   @return              An SQL FROM/JOIN fragment string.
 */
QString colman::SQLQueries::lending_query_base(int person_id, int item_id,
                                               int lending_id,
                                               QString const& person_search,
                                               QString const& item_search,
                                               bool show_returned)
{
  QString query = "FROM items JOIN types ON ";
  if(!show_returned) {
    query += "items.count > items.available AND ";
  }
  query += "items.type = types.id ";
  colman::Collection const collection = read_collection();
  if(item_id >= 0) {
    query += "AND items.id = " + QString().setNum(item_id) + " ";
  }
  else if(!item_search.isEmpty()) {
    int type_id = -1;
    if(colman::parse_item_barcode(collection, item_search, type_id, item_id)) {
      query += "AND items.id = " + QString().setNum(item_id) + " ";
    }
    else if(colman::is_isbn10(item_search)) {
      query += "AND items.id IN (SELECT item_id FROM item_list_values"
               " WHERE column_name = 'ISBN10' AND value = '" +
               item_search + "') ";
    }
    else if(colman::is_isbn13(item_search)) {
      query += "AND items.id IN (SELECT item_id FROM item_list_values"
               " WHERE column_name = 'ISBN13' AND value = '" +
               item_search + "') ";
    }
    else {
      query += "AND items.name LIKE '" + like_escape(item_search) +
               "%' ESCAPE '\\' ";
    }
  }
  query += "JOIN (SELECT a.*, SUM(b.count) FROM lendings a LEFT JOIN returns b"
           " ON a.id=b.lending GROUP BY a.id ";
  if(!show_returned) {
    query += "HAVING (SUM(b.count) IS NULL OR SUM(b.count) < a.count)";
  }
  query += ") c ON c.item = items.id ";
  if(lending_id >= 0) {
    query += "AND c.id = " + QString().setNum(lending_id) + " ";
  }
  query += "JOIN persons ON persons.id = c.person AND persons.id > 0 ";
  if(person_id >= 0) {
    query += "AND persons.id = " + QString().setNum(person_id) + " ";
  }
  else if(!person_search.isEmpty()) {
    if(colman::parse_person_barcode(collection, person_search, person_id)) {
      query += "AND persons.id = " + QString().setNum(person_id) + " ";
    }
    else {
      query += "AND (persons.lastname LIKE '" + like_escape(person_search) +
               "%' ESCAPE '\\' OR persons.firstname LIKE '" +
               like_escape(person_search) + "%' ESCAPE '\\') ";
    }
  }
  return query;
}

/// Queries persons currently lending an item (open lendings), as Person
/// objects.
/**
  @param item_id       If >= 0, restricts to persons lending this item.
  @param lending_id    If >= 0, restricts to the person of this lending.
  @param person_search Barcode or name prefix to filter persons.
  @param item_search   Barcode, ISBN or name prefix to filter items.
  @return              Vector of Person objects ordered by lastname.
*/
std::vector<colman::Person> colman::SQLQueries::query_lending_persons(
    int item_id, int lending_id, QString const& person_search,
    QString const& item_search)
{
  QString const sql =
      "SELECT DISTINCT persons.id, persons.lastname, persons.firstname, "
      "persons.title, COALESCE(persons.school_class, '') " +
      lending_query_base(-1, item_id, lending_id, person_search, item_search,
                         false) +
      "ORDER BY persons.lastname, persons.firstname COLLATE NOCASE";
  QSqlQuery q(sql, db_);
  std::vector<Person> result;
  while(q.next()) {
    Person p;
    p.set_id(q.value(0).toInt());
    p.set_lastname(q.value(1).toString());
    p.set_firstname(q.value(2).toString());
    p.set_title(q.value(3).toString());
    p.set_school_class(q.value(4).toString());
    result.push_back(std::move(p));
  }
  return result;
}

/// Queries items currently on loan (open lendings) as a list of rows.
/**
  @param person_id     If >= 0, restricts to items lent by this person.
  @param lending_id    If >= 0, restricts to the item of this lending.
  @param person_search Barcode or name prefix to filter by person.
  @param item_search   Barcode, ISBN or name prefix to filter by item.
  @return              Vector of ItemListModel::Row ordered by name.
*/
std::vector<colman::ItemListModel::Row> colman::SQLQueries::query_lent_items(
    int person_id, int lending_id, QString const& person_search,
    QString const& item_search, QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  QString const sql =
      "SELECT DISTINCT items.id, items.name, items.count, items.available, "
      "translations.name, types.name " +
      lending_query_base(person_id, -1, lending_id, person_search, item_search,
                         false) +
      "LEFT JOIN translations ON translations.tablename = 'types'"
      " AND translations.language = " +
      QString().setNum(lang) + " AND types.name = translations.column " +
      "ORDER BY items.name COLLATE NOCASE";
  QSqlQuery q(db_);
  q.exec(sql);
  std::vector<ItemListModel::Row> result;
  while(q.next()) {
    ItemListModel::Row r;
    r.id = q.value(0).toInt();
    r.name = q.value(1).toString();
    r.count = q.value(2).toInt();
    r.available = q.value(3).toInt();
    r.type_translated = q.value(4).toString();
    r.type_name = q.value(5).toString();
    result.push_back(std::move(r));
  }
  return result;
}

/// Queries open (unreturned) lendings filtered by person and item.
/**
  @param person_id If > 0, restricts to open lendings by this person.
  @param item_id   If > 0, restricts to open lendings of this item.
  @return          Vector of rows ordered by due date.
*/
std::vector<colman::OpenLendingListModel::Row> colman::SQLQueries::
    query_open_lendings(int person_id, int item_id)
{
  QString sql =
      "SELECT lendings.id, lendings.count, SUM(returns.count),"
      " lendings.datetime, lendings.duedate, lendings.remarks,"
      " persons.lastname || ', ' || persons.firstname"
      " || CASE WHEN persons.school_class IS NULL OR persons.school_class = ''"
      " THEN '' ELSE ' (' || persons.school_class || ')' END"
      " FROM lendings"
      " JOIN persons ON lendings.person = persons.id"
      " LEFT JOIN returns ON lendings.id = returns.lending ";
  if(item_id > 0 || person_id > 0) {
    sql += "WHERE ";
    if(item_id > 0) {
      sql += "lendings.item = " + QString().setNum(item_id) + " ";
      if(person_id > 0) {
        sql += "AND ";
      }
    }
    if(person_id > 0) {
      sql += " lendings.person = " + QString().setNum(person_id) + " ";
    }
  }
  sql += "GROUP BY lendings.id "
         "HAVING (SUM(returns.count) IS NULL "
         "OR SUM(returns.count) < lendings.count) "
         "ORDER BY lendings.duedate";
  QSqlQuery q(db_);
  q.exec(sql);
  std::vector<OpenLendingListModel::Row> rows;
  while(q.next()) {
    OpenLendingListModel::Row row;
    row.id = q.value(0).toInt();
    row.lent_count = q.value(1).toInt();
    row.returned_count = q.value(2).toInt();
    row.datetime = q.value(3).toDateTime();
    row.duedate = q.value(4).toDate();
    row.remarks = q.value(5).toString();
    row.person_name = q.value(6).toString();
    rows.push_back(std::move(row));
  }
  return rows;
}

// -----------------------------------------------------------------------
// Acquisitions
// -----------------------------------------------------------------------

/// Inserts a new acquisition record and updates the item stock.
/**
 * Increments the item's count and available columns by
 * acquisition.get_count(), inserts a new row into the acquisitions table,
 * and returns the auto-assigned id.  All changes are wrapped in a single
 * transaction.
 * @param acquisition the acquisition data to record
 * @return the new acquisition id on success, or -1 on failure
 */
int colman::SQLQueries::add_new_acquisition(Acquisition const& acquisition)
{
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("SELECT count, available FROM items WHERE id = :id");
  query.bindValue(":id", acquisition.get_item_id());
  if(!query.exec()) {
    qWarning() << "get number of items failed: " << query.lastError();
    return -1;
  }
  if(!query.next()) {
    qWarning() << "unknown item '" << acquisition.get_item_id() << "'\n";
    return -1;
  }
  unsigned int const old_num = query.value(0).toUInt();
  unsigned int const old_available = query.value(1).toUInt();
  query.prepare("UPDATE items SET count = :count, available = :available "
                "WHERE id = :id");
  query.bindValue(":id", acquisition.get_item_id());
  query.bindValue(":count", acquisition.get_count() + old_num);
  query.bindValue(":available", acquisition.get_count() + old_available);
  if(!query.exec()) {
    qWarning() << "updating number of items failed: " << query.lastError();
    return -1;
  }
  query.prepare(
      "INSERT INTO acquisitions (item, count, datetime, data_collector) "
      "VALUES (:item, :count, :datetime, :data_collector)");
  query.bindValue(":item", acquisition.get_item_id());
  query.bindValue(":count", acquisition.get_count());
  query.bindValue(":datetime", acquisition.get_date_time());
  query.bindValue(":data_collector", acquisition.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "inserting new acquisition failed: " << query.lastError();
    return -1;
  }
  QVariant const acqu_id = query.lastInsertId();
  rollback.commit();
  backup_after_change();
  return acqu_id.toInt();
}

/// Reads an acquisition record by id.
/**
 * Queries the acquisitions table for the row with \a id and populates
 * \a acquisition with the item id, count, and datetime.
 * @param id          the database id of the acquisition to read
 * @param acquisition output: populated with the retrieved acquisition data
 * @return true if the acquisition was found, false otherwise
 */
bool colman::SQLQueries::get_acquisition(int id, Acquisition& acquisition)
{
  QSqlQuery query(db_);
  query.prepare("SELECT item, count, datetime, data_collector "
                "FROM acquisitions WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "get acquisition from database failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown acquisition: " << id;
    return false;
  }
  acquisition.set_item_id(query.value(0).toInt());
  acquisition.set_count(query.value(1).toInt());
  acquisition.set_date_time(query.value(2).toDateTime());
  acquisition.set_data_collector_id(query.value(3).toInt());
  return true;
}

/// Updates an existing acquisition record and adjusts the item stock.
/**
 * Reads the existing acquisition, computes the difference between the
 * old and new count, and adjusts the item's stock size and available
 * count accordingly.  Then updates the acquisitions row with the new
 * count and date.  All changes are wrapped in a single transaction.
 * @param id          the database id of the acquisition to update
 * @param acquisition the new acquisition data (item id is ignored; the
 *                    existing item association is preserved)
 * @return true on success, false if the acquisition is unknown or a
 *         database error occurs
 */
bool colman::SQLQueries::change_acquisition(int id,
                                            Acquisition const& acquisition)
{
  Acquisition old;
  if(!get_acquisition(id, old)) {
    qWarning() << "unknown acquisition id " << id;
    return false;
  }
  AutoRollback rollback(db_);
  if(old.get_count() > acquisition.get_count()) {
    unsigned int const diff = old.get_count() - acquisition.get_count();
    if(!subtract_from_stock_size_and_available(db_, old.get_item_id(), diff)) {
      return false;
    }
  }
  else if(old.get_count() < acquisition.get_count()) {
    unsigned int const diff = acquisition.get_count() - old.get_count();
    if(!add_to_stock_size_and_available(db_, old.get_item_id(), diff)) {
      return false;
    }
  }

  QSqlQuery query(db_);
  query.prepare("UPDATE acquisitions SET count = :count, datetime = :datetime, "
                "data_collector = :data_collector WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":count", acquisition.get_count());
  query.bindValue(":datetime", acquisition.get_date_time());
  query.bindValue(":data_collector", acquisition.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "updating acquisitions failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Deletes an acquisition record and reduces the item stock accordingly.
/**
 * Reads the existing acquisition, reduces the item's stock size and
 * available count by the acquisition's count, then deletes the
 * acquisitions row.  All changes are wrapped in a single transaction.
 * If this was the last acquisition for the item, the item itself and all
 * associated records are removed entirely.
 * @param id the database id of the acquisition to remove
 * @return true on success, false if the acquisition is unknown or a
 *         database error occurs
 */
bool colman::SQLQueries::remove_acquisition(int id)
{
  Acquisition old;
  if(!get_acquisition(id, old)) {
    qWarning() << "unknown acquisition id " << id;
    return false;
  }
  AutoRollback rollback(db_);
  if(old.get_count() > 0) {
    if(!subtract_from_stock_size_and_available(db_, old.get_item_id(),
                                               old.get_count()))
    {
      qWarning() << "reducing stock size and number of available items failed";
      return false;
    }
  }

  QSqlQuery query(db_);
  query.prepare("DELETE FROM acquisitions WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "updating acquisitions failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  QSqlQuery check(db_);
  check.prepare("SELECT COUNT(*) FROM acquisitions WHERE item = :item_id");
  check.bindValue(":item_id", old.get_item_id());
  if(check.exec() && check.next() && check.value(0).toInt() == 0) {
    remove_item(old.get_item_id());
  }
  return true;
}

// -----------------------------------------------------------------------
// Liquidations
// -----------------------------------------------------------------------

/// Inserts a new liquidation record and reduces the item stock.
/**
 * Checks that the liquidation count does not exceed the available stock,
 * decrements both count and available on the item, inserts a row into the
 * liquidations table, and returns the auto-assigned id.  All changes are
 * wrapped in a single transaction.
 * @param liquidation the liquidation data to record
 * @return the new liquidation id on success, or -1 if the count exceeds
 *         available stock or any database error occurs
 */
int colman::SQLQueries::add_new_liquidation(Liquidation const& liquidation)
{
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("SELECT count, available FROM items WHERE id = :id");
  query.bindValue(":id", liquidation.get_item_id());
  if(!query.exec()) {
    qWarning() << "get number of items failed: " << query.lastError();
    return -1;
  }
  if(!query.next()) {
    qWarning() << "unknown item '" << liquidation.get_item_id() << "'\n";
    return -1;
  }
  unsigned int const old_num = query.value(0).toUInt();
  unsigned int const old_available = query.value(1).toUInt();
  if(liquidation.get_count() > old_available ||
     liquidation.get_count() > old_num)
  {
    // qWarning() << "can not liquidate more items than available";
    return -1;
  }

  query.prepare("UPDATE items SET count = :count, available = :available "
                "WHERE id = :id");
  query.bindValue(":id", liquidation.get_item_id());
  query.bindValue(":count", old_num - liquidation.get_count());
  query.bindValue(":available", old_available - liquidation.get_count());
  if(!query.exec()) {
    qWarning() << "updating number of items failed: " << query.lastError();
    return -1;
  }
  query.prepare(
      "INSERT INTO liquidations (item, count, datetime, data_collector) "
      "VALUES (:item, :count, :datetime, :data_collector)");
  query.bindValue(":item", liquidation.get_item_id());
  query.bindValue(":count", liquidation.get_count());
  query.bindValue(":datetime", liquidation.get_date_time());
  query.bindValue(":data_collector", liquidation.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "inserting new liquidation failed: " << query.lastError();
    return -1;
  }
  QVariant const liqu_id = query.lastInsertId();
  rollback.commit();
  backup_after_change();
  return liqu_id.toInt();
}

/// Reads a liquidation record by id.
/**
 * Queries the liquidations table for the row with \a id and populates
 * \a liquidation with the item id, count, and date and time.
 * @param id          the database id of the liquidation to read
 * @param liquidation output: populated with the retrieved liquidation data
 * @return true if the liquidation was found, false otherwise
 */
bool colman::SQLQueries::get_liquidation(int id, Liquidation& liquidation)
{
  QSqlQuery query(db_);
  query.prepare("SELECT item, count, datetime, data_collector "
                "FROM liquidations WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "get liquidation from database failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown liquidation: " << id;
    return false;
  }
  liquidation.set_item_id(query.value(0).toInt());
  liquidation.set_count(query.value(1).toInt());
  liquidation.set_date_time(query.value(2).toDateTime());
  liquidation.set_data_collector_id(query.value(3).toInt());
  return true;
}

/// Updates an existing liquidation record and adjusts the item stock.
/**
 * Reads the existing liquidation, computes the difference between the
 * old and new count, and adjusts the item's stock size and available
 * count accordingly (adding back if the new count is smaller, removing
 * more if larger).  Then updates the liquidations row.  All changes are
 * wrapped in a single transaction.
 * @param id          the database id of the liquidation to update
 * @param liquidation the new liquidation data
 * @return true on success, false if the liquidation is unknown or a
 *         database error occurs
 */
bool colman::SQLQueries::change_liquidation(int id,
                                            Liquidation const& liquidation)
{
  Liquidation old;
  if(!get_liquidation(id, old)) {
    qWarning() << "unknown liquidation";
    return false;
  }
  AutoRollback rollback(db_);
  if(old.get_count() < liquidation.get_count()) {
    unsigned int const diff = liquidation.get_count() - old.get_count();
    if(!subtract_from_stock_size_and_available(db_, old.get_item_id(), diff)) {
      return false;
    }
  }
  else if(old.get_count() > liquidation.get_count()) {
    unsigned int const diff = old.get_count() - liquidation.get_count();
    if(!add_to_stock_size_and_available(db_, old.get_item_id(), diff)) {
      return false;
    }
  }
  QSqlQuery query(db_);
  query.prepare("UPDATE liquidations SET count = :count, datetime = :datetime, "
                "data_collector = :data_collector WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":count", liquidation.get_count());
  query.bindValue(":datetime", liquidation.get_date_time());
  query.bindValue(":data_collector", liquidation.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "updating liquidations failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Deletes a liquidation record and restores the item stock.
/**
 * Reads the existing liquidation, adds the liquidation count back to the
 * item's stock size and available count, then deletes the liquidations row.
 * All changes are wrapped in a single transaction.
 * @param id the database id of the liquidation to remove
 * @return true on success, false if the liquidation is unknown or a
 *         database error occurs
 */
bool colman::SQLQueries::remove_liquidation(int id)
{
  Liquidation old;
  if(!get_liquidation(id, old)) {
    qWarning() << "unknown liquidation id " << id;
    return false;
  }
  AutoRollback rollback(db_);
  if(old.get_count() > 0) {
    if(!add_to_stock_size_and_available(db_, old.get_item_id(),
                                        old.get_count()))
    {
      qWarning()
          << "increasing stock size and number of available items failed";
      return false;
    }
  }
  QSqlQuery query(db_);
  query.prepare("DELETE FROM liquidations WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "removing liquidation failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

// -----------------------------------------------------------------------
// Returns
// -----------------------------------------------------------------------

/// Inserts a new return record and updates the available count.
/**
 * Validates that the return does not exceed the number of outstanding
 * lent items for the associated lending, inserts a row into the returns
 * table, and increments the item's available count.  All changes are
 * wrapped in a single transaction.
 * @param returning the return data to record
 * @return the new return id on success, or -1 if validation fails or a
 *         database error occurs
 */
int colman::SQLQueries::add_new_return(Return const& returning)
{
  size_t lent = 0u, returned = 0u;
  if(!lending_balance(returning.get_lending_id(), lent, returned)) {
    return -1;
  }
  // check consistency:
  if(returning.get_count() + returned > lent) {
    qWarning() << "can not return more items (" << returning.get_count()
               << ") than lent (" << lent << ", " << returned
               << " already returned), lending id="
               << returning.get_lending_id();
    return -1;
  }
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  // add return to returns table:
  query.prepare(
      "INSERT INTO returns "
      "(lending, count, datetime, remarks, data_collector) "
      "VALUES (:lending, :count, :datetime, :remarks, :data_collector)");
  query.bindValue(":lending", returning.get_lending_id());
  query.bindValue(":count", returning.get_count());
  query.bindValue(":datetime", returning.get_date_time());
  query.bindValue(":remarks", returning.get_remarks());
  query.bindValue(":data_collector", returning.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "insertion of new return failed:\n"
               << query.lastError();
    return -1;
  }
  QVariant const return_id = query.lastInsertId();
  int const item_id = get_lend_item(returning.get_lending_id());
  if(item_id < 0) {
    return -1;
  }
  if(!add_to_available(item_id, returning.get_count())) {
    return -1;
  }
  rollback.commit();
  backup_after_change();
  return return_id.toInt();
}

/// Returns all open copies of an item held by several persons at once.
/**
 * All returns are inserted in a single transaction; if one person has no
 * open lending of the item, the transaction is rolled back and nothing is
 * changed.
 * @param item_id    the item to return
 * @param person_ids the returning persons
 * @param tmpl       template carrying date/time, remarks and data collector
 * @return outcome with the new return ids on success
 */
colman::IQueries::GroupReturnResult colman::SQLQueries::add_group_return(
    int item_id, std::vector<int> const& person_ids, Return const& tmpl)
{
  using Status = GroupReturnResult::Status;
  GroupReturnResult result;
  if(person_ids.empty()) {
    result.status_ = Status::Ok;
    return result;
  }
  AutoRollback rollback(db_);
  // Open lendings of the item with their remaining (not returned) count.
  QSqlQuery open_q(db_);
  open_q.prepare(
      "SELECT lendings.id, lendings.person, "
      "lendings.count - COALESCE(SUM(returns.count), 0) "
      "FROM lendings LEFT JOIN returns ON lendings.id = returns.lending "
      "WHERE lendings.item = :item_id "
      "GROUP BY lendings.id, lendings.person, lendings.count "
      "HAVING (SUM(returns.count) IS NULL "
      "OR SUM(returns.count) < lendings.count)");
  open_q.bindValue(":item_id", item_id);
  if(!open_q.exec()) {
    qWarning() << "add_group_return: open lendings query failed:"
               << open_q.lastError();
    return result;
  }
  QMultiHash<int, std::pair<int, int>> open_by_person;
  while(open_q.next()) {
    open_by_person.insert(open_q.value(1).toInt(),
                          {open_q.value(0).toInt(), open_q.value(2).toInt()});
  }
  for(int const person_id : person_ids) {
    if(!open_by_person.contains(person_id)) {
      result.status_ = Status::NotLent;
      result.person_id_ = person_id;
      return result;
    }
  }
  QSqlQuery insert_q(db_);
  insert_q.prepare(
      "INSERT INTO returns "
      "(lending, count, datetime, remarks, data_collector) "
      "VALUES (:lending, :count, :datetime, :remarks, :data_collector)");
  auto total = 0;
  for(int const person_id : person_ids) {
    for(auto const& open : open_by_person.values(person_id)) {
      insert_q.bindValue(":lending", open.first);
      insert_q.bindValue(":count", open.second);
      insert_q.bindValue(":datetime", tmpl.get_date_time());
      insert_q.bindValue(":remarks", tmpl.get_remarks());
      insert_q.bindValue(":data_collector", tmpl.get_data_collector_id());
      if(!insert_q.exec()) {
        qWarning() << "add_group_return: inserting return failed:"
                   << insert_q.lastError();
        return result;
      }
      result.return_ids_.push_back(insert_q.lastInsertId().toInt());
      total += open.second;
    }
  }
  if(!add_to_available(item_id, static_cast<unsigned int>(total))) {
    result.return_ids_.clear();
    return result;
  }
  rollback.commit();
  backup_after_change();
  result.returned_count_ = total;
  result.status_ = Status::Ok;
  return result;
}

/**
   Searches for return with \a id and, if found, stores it in \a ret.
   @return true, iff operation was successfully
*/
bool colman::SQLQueries::get_return(int return_id, Return& ret)
{
  QSqlQuery query(db_);
  query.prepare("SELECT lending, count, datetime, remarks, data_collector "
                "FROM returns WHERE id = :id");
  query.bindValue(":id", return_id);
  if(!query.exec()) {
    qWarning() << "could not query return " << return_id << ": "
               << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown return id " << return_id;
    return false;
  }
  ret.set_lending_id(query.value(0).toInt());
  ret.set_count(query.value(1).toUInt());
  ret.set_date_time(query.value(2).toDateTime());
  ret.set_remarks(query.value(3).toString());
  ret.set_data_collector_id(query.value(4).toInt());
  return true;
}

/// Updates an existing return record and adjusts the item's available count.
/**
 * Reads the existing return, checks that the new count is still
 * consistent with the lending balance, adjusts the item's available
 * count for the difference, and updates the returns row.  All changes
 * are wrapped in a single transaction.
 * @param return_id the database id of the return to update
 * @param ret       the new return data
 * @return true on success, false if the return is unknown, the count
 *         exceeds the lending balance, or a database error occurs
 */
bool colman::SQLQueries::change_return(int return_id, Return const& ret)
{
  Return old;
  if(!get_return(return_id, old)) {
    qWarning() << "unknown return";
    return false;
  }
  size_t lent = 0, returned = 0;
  if(!lending_balance(old.get_lending_id(), lent, returned)) {
    return false;
  }
  if(lent + old.get_count() < returned + ret.get_count()) {
    return false;
  }
  int const item_id = get_lend_item(old.get_lending_id());
  AutoRollback rollback(db_);
  if(old.get_count() > ret.get_count()) {
    unsigned int const diff = old.get_count() - ret.get_count();
    if(!subtract_from_available(item_id, diff)) {
      return false;
    }
  }
  else if(old.get_count() < ret.get_count()) {
    unsigned int const diff = ret.get_count() - old.get_count();
    if(!add_to_available(item_id, diff)) {
      return false;
    }
  }
  QSqlQuery query(db_);
  query.prepare("UPDATE returns SET count = :count, "
                "datetime = :datetime, remarks = :remarks, "
                "data_collector = :data_collector WHERE id = :id");
  query.bindValue(":id", return_id);
  query.bindValue(":count", ret.get_count());
  query.bindValue(":datetime", ret.get_date_time());
  query.bindValue(":remarks", ret.get_remarks());
  query.bindValue(":data_collector", ret.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "updating returns failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Deletes a return record and decrements the item's available count.
/**
 * Reads the existing return, reduces the item's available count by the
 * return's count (reversing the return), then deletes the returns row.
 * All changes are wrapped in a single transaction.
 * @param return_id the database id of the return to remove
 * @return true on success, false if the return is unknown or a database
 *         error occurs
 */
bool colman::SQLQueries::remove_return(int return_id)
{
  Return old;
  if(!get_return(return_id, old)) {
    qWarning() << "unknown return";
    return false;
  }
  int const item_id = get_lend_item(old.get_lending_id());
  AutoRollback rollback(db_);
  if(!subtract_from_available(item_id, old.get_count())) {
    return false;
  }
  QSqlQuery query(db_);
  query.prepare("DELETE FROM returns WHERE id = :id");
  query.bindValue(":id", return_id);
  if(!query.exec()) {
    qWarning() << "deleting return failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

// -----------------------------------------------------------------------
// Lendings
// -----------------------------------------------------------------------

/// Returns the item id associated with a lending.
/**
 * Queries the item column of the lendings table for \a lend_id.
 * @param lend_id the database id of the lending to inspect
 * @return the item id on success, or -1 if the lending is unknown or the
 *         query fails
 */
int colman::SQLQueries::get_lend_item(int lend_id)
{
  QSqlQuery query(db_);
  query.prepare("SELECT item FROM lendings WHERE id = :id");
  query.bindValue(":id", lend_id);
  if(!query.exec()) {
    qWarning() << "could not query item for lending " << lend_id << ": "
               << query.lastError();
    return -1;
  }
  if(!query.next()) {
    qWarning() << "unknown lending id " << lend_id;
    return -1;
  }
  return query.value(0).toInt();
}

namespace
{
  /// Inserts a lending inside an already open transaction.
  /**
     Checks availability and reservations of other persons, consumes the
     borrower's own reservation, inserts the lending row and decrements the
     item's available count.  The caller owns the transaction.
     @param db      database connection with an open transaction
     @param lending the lending data to record
     @return the new lending id, -2 if copies reserved by other persons
             block the lending, or -1 on any other failure
  */
  int insert_lending_unlocked(QSqlDatabase& db,
                              colman::Lending const& lending)
  {
    QSqlQuery query(db);
    // Purge expired reservations for this item (inside the write lock).
    colman::purge_expired_reservation_unlocked(db, lending.get_item_id());
    // Check how many copies others have reserved.
    QSqlQuery res_q(db);
    res_q.prepare("SELECT COALESCE(SUM(count),0) FROM reservations "
                  "WHERE item_id = :item AND person_id != :person "
                  "AND expires_at >= date('now', 'localtime')");
    res_q.bindValue(":item", lending.get_item_id());
    res_q.bindValue(":person", lending.get_person_id());
    if(!res_q.exec() || !res_q.next()) {
      qWarning() << "add_new_lending: reservation check failed:"
                 << res_q.lastError();
      return -1;
    }
    int const reserved_for_others = res_q.value(0).toInt();
    // first ask for available items:
    query.prepare("SELECT available FROM items WHERE id = :item_id");
    query.bindValue(":item_id", lending.get_item_id());
    if(!query.exec()) {
      qWarning() << "query number of available items failed:\n"
                 << query.lastError();
      return -1;
    }
    if(!query.next()) {
      qWarning() << "unknown item '" << lending.get_item_id() << "'\n";
      return -1;
    }
    unsigned int const old_available = query.value(0).toUInt();
    if(lending.get_count() > old_available) {
      qWarning() << "can not lend more items than available";
      return -1;
    }
    int const not_reserved =
        std::max(0, static_cast<int>(old_available) - reserved_for_others);
    if(static_cast<int>(lending.get_count()) > not_reserved) {
      return -2;
    }
    // Reduce or delete own reservation if person has one.
    {
      QSqlQuery own_q(db);
      own_q.prepare("SELECT id, count FROM reservations "
                    "WHERE item_id = :item AND person_id = :person "
                    "AND expires_at >= date('now', 'localtime')");
      own_q.bindValue(":item", lending.get_item_id());
      own_q.bindValue(":person", lending.get_person_id());
      if(own_q.exec() && own_q.next()) {
        int const own_id = own_q.value(0).toInt();
        int const own_count = own_q.value(1).toInt();
        int const from_own =
            std::min(static_cast<int>(lending.get_count()), own_count);
        int const remaining = own_count - from_own;
        QSqlQuery upd(db);
        if(remaining == 0) {
          upd.prepare("DELETE FROM reservations WHERE id = :id");
          upd.bindValue(":id", own_id);
        }
        else {
          upd.prepare("UPDATE reservations SET count = :cnt WHERE id = :id");
          upd.bindValue(":cnt", remaining);
          upd.bindValue(":id", own_id);
        }
        upd.exec();
      }
    }
    // add new lending to lendings:
    query.prepare(
        "INSERT INTO lendings "
        "(item, count, person, datetime, duedate, remarks, data_collector)"
        " VALUES (:item, :count, :person, :datetime, :duedate, "
        ":remarks, :data_collector)");
    query.bindValue(":item", lending.get_item_id());
    query.bindValue(":count", lending.get_count());
    query.bindValue(":person", lending.get_person_id());
    query.bindValue(":datetime", lending.get_date_time());
    query.bindValue(":duedate", lending.get_duedate());
    query.bindValue(":remarks", lending.get_remarks());
    query.bindValue(":data_collector", lending.get_data_collector_id());
    if(!query.exec()) {
      qWarning() << "inserting into lendings failed:\n"
                 << query.lastError();
      return -1;
    }
    int const lending_id = query.lastInsertId().toInt();
    // update available number of items:
    query.prepare("UPDATE items SET available = :available WHERE id = :id");
    query.bindValue(":available", old_available - lending.get_count());
    query.bindValue(":id", lending.get_item_id());
    if(!query.exec()) {
      qWarning() << "updating number in items failed:\n"
                 << query.lastError();
      return -1;
    }
    return lending_id;
  }

} // namespace


/// Inserts a new lending record and decrements the available count.
/**
 * Checks that enough copies are available, inserts a row into the
 * lendings table, and decrements the item's available count.  All
 * changes are wrapped in a single transaction.
 * @param lending the lending data to record
 * @return the new lending id on success, -2 if copies reserved by other
 *         persons block the lending, or -1 if insufficient copies are
 *         available or a database error occurs
 */
int colman::SQLQueries::add_new_lending(Lending const& lending)
{
  AutoRollback rollback(db_);
  int const lending_id = insert_lending_unlocked(db_, lending);
  if(lending_id < 0) {
    return lending_id;
  }
  rollback.commit();
  backup_after_change();
  return lending_id;
}


/// Lends one copy of an item to each of several persons at once.
/**
 * All lendings are inserted in a single transaction; on any failure the
 * transaction is rolled back and nothing is changed.
 * @param lending    template carrying item, date/time, due date, remarks
 *                   and data collector; person and count are ignored
 * @param person_ids the borrowers
 * @return outcome with the new lending ids on success
 */
colman::IQueries::GroupLendingResult colman::SQLQueries::add_group_lending(
    Lending const& lending, std::vector<int> const& person_ids)
{
  using Status = GroupLendingResult::Status;
  GroupLendingResult result;
  if(person_ids.empty()) {
    result.status_ = Status::Ok;
    return result;
  }
  AutoRollback rollback(db_);
  int const item_id = lending.get_item_id();
  // Reject persons who already hold an open lending of this item.
  QSqlQuery open_q(db_);
  open_q.prepare(
      "SELECT lendings.person "
      "FROM lendings LEFT JOIN returns ON lendings.id = returns.lending "
      "WHERE lendings.item = :item_id "
      "GROUP BY lendings.id, lendings.person "
      "HAVING (SUM(returns.count) IS NULL "
      "OR SUM(returns.count) < lendings.count)");
  open_q.bindValue(":item_id", item_id);
  if(!open_q.exec()) {
    qWarning() << "add_group_lending: open lendings query failed:"
               << open_q.lastError();
    return result;
  }
  QSet<int> holders;
  while(open_q.next()) {
    holders.insert(open_q.value(0).toInt());
  }
  for(int const person_id : person_ids) {
    if(holders.contains(person_id)) {
      result.status_ = Status::AlreadyLent;
      result.person_id_ = person_id;
      return result;
    }
  }
  // Copies that may be lent to the group: available copies minus those
  // still reserved for others after the group got its copies.
  purge_expired_reservation_unlocked(db_, item_id);
  QSqlQuery avail_q(db_);
  avail_q.prepare("SELECT available FROM items WHERE id = :item_id");
  avail_q.bindValue(":item_id", item_id);
  if(!avail_q.exec() || !avail_q.next()) {
    qWarning() << "add_group_lending: availability query failed:"
               << avail_q.lastError();
    return result;
  }
  int lendable = avail_q.value(0).toInt();
  QSet<int> const group(person_ids.begin(), person_ids.end());
  QSqlQuery res_q(db_);
  res_q.prepare("SELECT person_id, count FROM reservations "
                "WHERE item_id = :item "
                "AND expires_at >= date('now', 'localtime')");
  res_q.bindValue(":item", item_id);
  if(!res_q.exec()) {
    qWarning() << "add_group_lending: reservation query failed:"
               << res_q.lastError();
    return result;
  }
  while(res_q.next()) {
    int const count = res_q.value(1).toInt();
    if(group.contains(res_q.value(0).toInt())) {
      // A group member takes one copy of their own reservation; the rest
      // stays reserved for them.
      lendable -= std::max(0, count - 1);
    }
    else {
      lendable -= count;
    }
  }
  lendable = std::max(0, lendable);
  if(static_cast<int>(group.size()) > lendable) {
    result.status_ = Status::NotEnoughAvailable;
    result.lendable_ = lendable;
    return result;
  }
  for(int const person_id : person_ids) {
    Lending single = lending;
    single.set_person_id(person_id);
    single.set_count(1);
    int const lending_id = insert_lending_unlocked(db_, single);
    if(lending_id < 0) {
      // Should not happen after the checks above; report the current
      // lendable count so the caller can show a meaningful message.
      result.status_ = (lending_id == -2) ? Status::NotEnoughAvailable
                                          : Status::Error;
      result.lendable_ = lendable;
      result.lending_ids_.clear();
      return result;
    }
    result.lending_ids_.push_back(lending_id);
  }
  rollback.commit();
  backup_after_change();
  result.status_ = Status::Ok;
  return result;
}

/**
   Searches for lending with \a id and, if found, stores it in \a lending.
   @return true, iff operation was successfully
*/
bool colman::SQLQueries::get_lending(int id, Lending& lending)
{
  QSqlQuery query(db_);
  query.prepare("SELECT item, count, person, datetime, duedate, remarks, "
                "data_collector FROM lendings WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "could not query lending " << id << ": " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown lending id " << id;
    return false;
  }
  lending.set_item_id(query.value(0).toInt());
  lending.set_count(query.value(1).toUInt());
  lending.set_person_id(query.value(2).toInt());
  lending.set_date_time(query.value(3).toDateTime());
  lending.set_duedate(query.value(4).toDate());
  lending.set_remarks(query.value(5).toString());
  lending.set_data_collector_id(query.value(6).toInt());
  return true;
}

/// Updates an existing lending record and adjusts the item's available count.
/**
 * Reads the existing lending, verifies that enough copies are available
 * for any increase in count, adjusts the available count for the
 * difference, and updates the lendings row.  All changes are wrapped in
 * a single transaction.
 * @param id      the database id of the lending to update
 * @param lending the new lending data
 * @return true on success, false if the lending is unknown, the new count
 *         exceeds available copies, or a database error occurs
 */
bool colman::SQLQueries::change_lending(int id, Lending const& lending)
{
  Lending old;
  if(!get_lending(id, old)) {
    qWarning() << "unknown lending";
    return false;
  }
  size_t lent = 0, returned = 0;
  if(!lending_balance(id, lent, returned)) {
    return false;
  }
  if(lent + lending.get_count() < returned + old.get_count()) {
    return false;
  }
  AutoRollback rollback(db_);
  if(old.get_count() < lending.get_count()) {
    unsigned int const diff = lending.get_count() - old.get_count();
    if(!subtract_from_available(old.get_item_id(), diff)) {
      return false;
    }
  }
  else if(old.get_count() > lending.get_count()) {
    unsigned int const diff = old.get_count() - lending.get_count();
    if(!add_to_available(old.get_item_id(), diff)) {
      return false;
    }
  }
  QSqlQuery query(db_);
  query.prepare("UPDATE lendings SET item = :item, count = :count, "
                "person = :person, datetime = :datetime, "
                "duedate = :duedate, remarks = :remarks, "
                "data_collector = :data_collector WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":item", lending.get_item_id());
  query.bindValue(":count", lending.get_count());
  query.bindValue(":person", lending.get_person_id());
  query.bindValue(":datetime", lending.get_date_time());
  query.bindValue(":duedate", lending.get_duedate());
  query.bindValue(":remarks", lending.get_remarks());
  query.bindValue(":data_collector", lending.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "updating lendings failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Deletes a lending and all its associated return records.
/**
 * Reads the lending, checks that it has no partially-returned state
 * (either fully returned or not returned at all), restores the
 * outstanding count to available, and deletes the lendings and all
 * associated returns rows.  All changes are wrapped in a single
 * transaction.
 * @param id the database id of the lending to remove
 * @return true on success, false if the lending is unknown, is partially
 *         returned, or a database error occurs
 */
bool colman::SQLQueries::remove_lending(int id)
{
  Lending old;
  if(!get_lending(id, old)) {
    qWarning() << "unknown lending id " << id;
    return false;
  }
  size_t lent = 0, returned = 0;
  if(!lending_balance(id, lent, returned)) {
    return false;
  }
  if(lent > returned && returned > 0) {
    return false;
  }
  AutoRollback rollback(db_);
  if(old.get_count() > 0) {
    if(!add_to_available(old.get_item_id(), lent - returned)) {
      qWarning() << "increasing number of available items failed";
      return false;
    }
  }
  QSqlQuery query(db_);
  query.prepare("DELETE FROM lendings WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "removing lending failed: " << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM returns WHERE lending = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "removing returns failed: " << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Computes the total lent and total returned counts for a lending.
/**
 * Sums the count column of the lendings table for \a lending_id into
 * \a lent and the count column of the returns table into \a returned.
 * @param lending_id the database id of the lending to inspect
 * @param lent       output: total number of items lent
 * @param returned   output: total number of items returned so far
 * @return true on success, false if either query fails
 */
bool colman::SQLQueries::lending_balance(int lending_id, std::size_t& lent,
                                         std::size_t& returned)
{
  QSqlQuery query(db_);
  query.prepare("SELECT SUM(count) FROM lendings WHERE id = :id");
  query.bindValue(":id", lending_id);
  if(!query.exec()) {
    qWarning() << "query sum of lendings for " << lending_id << " failed";
    return false;
  }
  size_t tmp = 0u;
  if(query.next()) {
    tmp = query.value(0).toUInt();
  }
  query.prepare("SELECT SUM(count) FROM returns WHERE lending = :id");
  query.bindValue(":id", lending_id);
  if(!query.exec()) {
    qWarning() << "query sum of returns for " << lending_id << " failed";
    return false;
  }
  size_t tmp2 = 0u;
  while(query.next()) {
    tmp2 += query.value(0).toUInt();
  }
  lent = tmp;
  returned = tmp2;
  return true;
}

/// Queries all lending records for a person.
/**
  @param person_id the database id of the person to query
  @return list of LendingInfo entries; empty if none or on error
*/
std::vector<colman::IQueries::LendingInfo> colman::SQLQueries::query_lendings(
    int person_id)
{
  std::vector<LendingInfo> ret_val;
  QSqlQuery query(db_);
  query.prepare("SELECT items.name, lendings.id, lendings.count "
                "FROM lendings JOIN items ON lendings.item = items.id "
                "AND lendings.person = :pid");
  query.bindValue(":pid", person_id);
  if(!query.exec()) {
    qWarning() << "query_lendings failed: " << query.lastError();
    return {};
  }
  while(query.next()) {
    LendingInfo info;
    info.item_name_ = query.value(0).toString();
    info.lending_id_ = query.value(1).toInt();
    info.lending_count_ = query.value(2).toInt();
    ret_val.push_back(std::move(info));
  }
  return ret_val;
}

std::vector<colman::DueLendingListModel::Row> colman::SQLQueries::
    query_due_lendings(QDate const& from_date, QDate const& to_date,
                       int person_id, QString const& school_class_filter,
                       QString const& person_name_filter)
{
  QString sql =
      "SELECT lendings.id, lendings.duedate,"
      " persons.lastname || ', ' || persons.firstname,"
      " (lendings.count - COALESCE(SUM(returns.count), 0)),"
      " items.name, persons.id, items.id,"
      " COALESCE(persons.school_class, '')"
      " FROM lendings"
      " JOIN persons ON lendings.person = persons.id"
      " JOIN items ON lendings.item = items.id"
      " LEFT JOIN returns ON lendings.id = returns.lending";
  QStringList conditions;
  if(from_date.isValid()) {
    conditions << "lendings.duedate >= :from AND lendings.duedate <= :to";
  }
  if(person_id >= 0) {
    conditions << "persons.id = " + QString::number(person_id);
  }
  if(!school_class_filter.isEmpty()) {
    conditions << "persons.school_class = :school_class";
  }
  if(!person_name_filter.isEmpty()) {
    conditions << "persons.lastname LIKE :pname";
  }
  if(!conditions.isEmpty()) {
    sql += " WHERE " + conditions.join(" AND ");
  }
  sql += " GROUP BY lendings.id"
         " HAVING COALESCE(SUM(returns.count), 0) < lendings.count"
         " ORDER BY"
         "  CASE WHEN lendings.duedate IS NULL OR lendings.duedate = ''"
         "  THEN 1 ELSE 0 END,"
         "  lendings.duedate";
  QSqlQuery q(db_);
  q.prepare(sql);
  if(from_date.isValid()) {
    q.bindValue(":from", from_date.toString(Qt::ISODate));
    q.bindValue(":to", to_date.toString(Qt::ISODate));
  }
  if(!school_class_filter.isEmpty()) {
    q.bindValue(":school_class", school_class_filter);
  }
  if(!person_name_filter.isEmpty()) {
    q.bindValue(":pname", person_name_filter + "%");
  }
  if(!q.exec()) {
    qWarning() << "query_due_lendings failed:" << q.lastError();
    return {};
  }
  std::vector<DueLendingListModel::Row> rows;
  while(q.next()) {
    DueLendingListModel::Row row;
    row.lending_id = q.value(0).toInt();
    row.duedate = q.value(1).toDate();
    row.person_name = q.value(2).toString();
    row.outstanding_count = q.value(3).toInt();
    row.item_name = q.value(4).toString();
    row.person_id = q.value(5).toInt();
    row.item_id = q.value(6).toInt();
    row.school_class = q.value(7).toString();
    rows.push_back(std::move(row));
  }
  return rows;
}

/**
   Sets a new due date for the given lendings in a single transaction.
   Increments the version of each changed lending so concurrent editors
   notice the change.
   @param lending_ids ids of the lendings to change
   @param duedate     new due date (invalid = no due date)
   @return true on success, false if a lending does not exist or the
           update failed
*/
bool colman::SQLQueries::extend_lendings(std::vector<int> const& lending_ids,
                                         QDate const& duedate)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare("UPDATE lendings SET duedate = :duedate,"
            " version = version + 1 WHERE id = :id");
  for(int const id : lending_ids) {
    q.bindValue(":duedate", duedate);
    q.bindValue(":id", id);
    if(!q.exec()) {
      qWarning() << "extend_lendings failed:" << q.lastError();
      return false;
    }
    if(q.numRowsAffected() != 1) {
      return false;
    }
  }
  rollback.commit();
  return true;
}

/* EOF */
