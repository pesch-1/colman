// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries items, signatures, thumbnails, FTS, and availability domain.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sql_queries_items.hpp"
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
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif
#ifndef SYSTEM_QREGULAREXPRESSION
#  include <QRegularExpression>
#  define SYSTEM_QREGULAREXPRESSION
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{
  /// Writes raw bytes to disk, optionally skipping existing files.
  /**
   * Creates the necessary parent directories, then writes \a bytes to
   * \a filename.  If \a overwrite is false and the file already exists,
   * the write is skipped and false is returned.
   * @param bytes     bytes to write
   * @param filename  absolute path of the output file
   * @param overwrite whether to overwrite an existing file
   * @return true if the file was written successfully, false otherwise
   */
  bool write_bytes(QByteArray const& bytes, QString const& filename,
                   bool overwrite)
  {
    if(!filename.isEmpty()) {
      QDir const dir = QFileInfo(filename).dir();
      if(dir.mkpath(".")) {
        if(QFile::exists(filename) && !overwrite) {
          qWarning() << "file " << filename << " already exists";
        }
        else {
          QFile file(filename);
          if(file.open(QIODevice::WriteOnly)) {
            return file.write(bytes) == static_cast<qint64>(bytes.size());
          }
        }
      }
      else {
        qWarning() << "could not create directory " << dir;
      }
    }
    return false;
  }

  /// Builds the SQL filter clause for an item search string.
  /**
     The collection's own item barcode is checked first: a colman barcode is
     13 digits long for many item ids and then often carries a valid EAN-13
     check digit, so testing for an ISBN-13 first would misinterpret it.
     @param collection the collection whose barcode prefix to use.
     @param search non-empty ISBN, barcode or name prefix.
     @return SQL fragment starting with " AND ", to append to the JOIN.
  */
  QString item_search_filter(colman::Collection const& collection,
                             QString const& search)
  {
    int type_id = -1;
    int item_id = -1;
    if(colman::parse_item_barcode(collection, search, type_id, item_id)) {
      return " AND items.id = " + QString().setNum(item_id) + " ";
    }
    if(colman::is_isbn13(search)) {
      return " AND items.id IN (SELECT item_id FROM item_list_values"
             " WHERE column_name = 'ISBN13' AND value = '" +
             search + "') ";
    }
    if(colman::is_isbn10(search)) {
      return " AND items.id IN (SELECT item_id FROM item_list_values"
             " WHERE column_name = 'ISBN10' AND value = '" +
             search + "') ";
    }
    return " AND items.name LIKE '" + colman::like_escape(search) +
           "%' ESCAPE '\\' ";
  }

} // namespace

// -----------------------------------------------------------------------
// Signature categories
// -----------------------------------------------------------------------

/**
 * Returns all signature-top category names stored in the database,
 * sorted alphabetically.
 * @return    sorted list of category names
 */
std::vector<QString> colman::SQLQueries::read_signature_top_categories()
{
  std::vector<QString> result;
  QSqlQuery q(db_);
  q.prepare("SELECT name FROM signature_top_categories ORDER BY name");
  if(!q.exec()) {
    qWarning() << "read_signature_top_categories failed:" << q.lastError();
    return result;
  }
  while(q.next()) {
    result.push_back(q.value(0).toString());
  }
  return result;
}

/**
 * Inserts \a name into the signature_top_categories table.
 * Silently succeeds if the name already exists (PRIMARY KEY conflict).
 * @param name  category name to add
 * @return      true on success
 */
bool colman::SQLQueries::add_signature_top_category(QString const& name)
{
  QSqlQuery q(db_);
  q.prepare("INSERT OR IGNORE INTO signature_top_categories (name) "
            "VALUES (:name)");
  q.bindValue(":name", name);
  if(!q.exec()) {
    qWarning() << "add_signature_top_category failed:" << q.lastError();
    return false;
  }
  return true;
}

/**
 * Removes the entry with the given \a name from signature_top_categories.
 * @param name  category name to remove
 * @return      true on success
 */
bool colman::SQLQueries::remove_signature_top_category(QString const& name)
{
  QSqlQuery q(db_);
  q.prepare("DELETE FROM signature_top_categories WHERE name = :name");
  q.bindValue(":name", name);
  if(!q.exec()) {
    qWarning() << "remove_signature_top_category failed:" << q.lastError();
    return false;
  }
  return true;
}

/// Returns all bottom-category names for @a top, sorted alphabetically.
/**
  @param top  name of the parent top category.
  @return     sorted list of subcategory names; empty if none or on error.
*/
std::vector<QString> colman::SQLQueries::read_signature_bottom_categories(
    QString const& top)
{
  std::vector<QString> result;
  QSqlQuery q(db_);
  if(top.isEmpty()) {
    q.prepare("SELECT name FROM signature_bottom_categories "
              "ORDER BY name");
  }
  else {
    q.prepare("SELECT name FROM signature_bottom_categories "
              "WHERE top_category = :top ORDER BY name");
    q.bindValue(":top", top);
  }
  if(!q.exec()) {
    qWarning() << "read_signature_bottom_categories failed:" << q.lastError();
    return result;
  }
  while(q.next()) {
    result.push_back(q.value(0).toString());
  }
  return result;
}

/// Inserts a new bottom category for @a top.
/**
  Silently succeeds if the combination already exists.
  @param top   name of the parent top category.
  @param name  subcategory name to add.
  @return true on success.
*/
bool colman::SQLQueries::add_signature_bottom_category(QString const& top,
                                                       QString const& name)
{
  QSqlQuery q(db_);
  q.prepare("INSERT OR IGNORE INTO signature_bottom_categories "
            "(top_category, name) VALUES (:top, :name)");
  q.bindValue(":top", top);
  q.bindValue(":name", name);
  if(!q.exec()) {
    qWarning() << "add_signature_bottom_category failed:" << q.lastError();
    return false;
  }
  return true;
}

/// Removes the bottom category @a name under top category @a top.
/**
  @param top   name of the parent top category.
  @param name  subcategory name to remove.
  @return true on success.
*/
bool colman::SQLQueries::remove_signature_bottom_category(QString const& top,
                                                          QString const& name)
{
  QSqlQuery q(db_);
  q.prepare("DELETE FROM signature_bottom_categories "
            "WHERE top_category = :top AND name = :name");
  q.bindValue(":top", top);
  q.bindValue(":name", name);
  if(!q.exec()) {
    qWarning() << "remove_signature_bottom_category failed:" << q.lastError();
    return false;
  }
  return true;
}

// -----------------------------------------------------------------------
// View-model queries
// -----------------------------------------------------------------------

/// Returns the total number of items in the database.
int colman::SQLQueries::count_items()
{
  QSqlQuery query(db_);
  query.prepare("SELECT COUNT(*) FROM items");
  if(!query.exec() || !query.next()) {
    return 0;
  }
  return query.value(0).toInt();
}


/// Queries items filtered by id and text search.
/**
  @param item_id If greater than zero, filters to this item id.
  @param search  Optional ISBN, barcode or name prefix to filter items.
  @return        Vector of ItemListModel::Row ordered by name.
*/
std::vector<colman::ItemListModel::Row> colman::SQLQueries::query_items(
    int item_id, QString const& search, QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  QString sql("SELECT items.id, items.name, items.count, items.available, "
              "translations.name, types.name, "
              "CASE WHEN items.data_collector IS NULL"
              "          OR items.data_collector = 0 THEN 'Administrator'"
              "     ELSE TRIM(COALESCE(persons.firstname,'') || ' '"
              "              || COALESCE(persons.lastname,'')) END "
              "FROM items "
              "JOIN types ON items.type = types.id ");
  if(item_id > 0) {
    sql += "AND items.id = " + QString().setNum(item_id) + " ";
  }
  else if(!search.isEmpty()) {
    sql += item_search_filter(read_collection(), search);
  }
  sql += "LEFT JOIN persons ON items.data_collector = persons.id "
         "LEFT JOIN translations ON translations.tablename = 'types'"
         " AND translations.language = " +
         QString().setNum(lang) +
         " AND types.name = translations.column "
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
    r.collector = q.value(6).toString();
    result.push_back(std::move(r));
  }
  return result;
}

/// Queries acquired items (count > 0) and returns them as a list of rows.
/**
  @param item_id If greater than zero, filters to this item id.
  @param search  Optional ISBN, barcode or name prefix to filter items.
  @return        Vector of ItemListModel::Row ordered by name.
*/
std::vector<colman::ItemListModel::Row> colman::SQLQueries::
    query_acquired_items(int item_id, QString const& search,
                         int limit, int offset, QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  QString sql("SELECT items.id, items.name, items.count, items.available, "
              "translations.name, types.name "
              "FROM items "
              "JOIN types ON items.type = types.id AND items.count > 0 ");
  if(item_id > 0) {
    sql += "AND items.id = " + QString().setNum(item_id) + " ";
  }
  else if(!search.isEmpty()) {
    sql += item_search_filter(read_collection(), search);
  }
  sql += "LEFT JOIN translations ON translations.tablename = 'types'"
         " AND translations.language = " +
         QString().setNum(lang) +
         " AND types.name = translations.column "
         "ORDER BY items.name COLLATE NOCASE";
  if(limit > 0) {
    sql += " LIMIT " + QString::number(limit + 1) +
           " OFFSET " + QString::number(offset);
  }
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

/// Queries available items (count > 0, available > 0) as a list of rows.
/**
  @param item_id If greater than zero, filters to this item id.
  @param search  Optional ISBN, barcode or name prefix to filter items.
  @return        Vector of ItemListModel::Row ordered by name.
*/
std::vector<colman::ItemListModel::Row> colman::SQLQueries::
    query_available_items(int item_id, QString const& search,
                          int limit, int offset, QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  QString sql("SELECT items.id, items.name, items.count, items.available, "
              "translations.name, types.name "
              "FROM items "
              "JOIN types ON items.type = types.id AND items.count > 0 "
              " AND items.available > 0 ");
  if(item_id > 0) {
    sql += "AND items.id = " + QString().setNum(item_id) + " ";
  }
  else if(!search.isEmpty()) {
    sql += item_search_filter(read_collection(), search);
  }
  sql += "LEFT JOIN translations ON translations.tablename = 'types'"
         " AND translations.language = " +
         QString().setNum(lang) +
         " AND types.name = translations.column "
         "ORDER BY items.name COLLATE NOCASE";
  if(limit > 0) {
    sql += " LIMIT " + QString::number(limit + 1) +
           " OFFSET " + QString::number(offset);
  }
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

/// Queries items belonging to a specific type as a list of rows.
/**
  @param type_id The type id to filter items by (required, > 0).
  @param search  Optional ISBN, barcode or name prefix to further filter items.
  @return        Vector of ItemListModel::Row ordered by name.
*/
std::vector<colman::ItemListModel::Row> colman::SQLQueries::
    query_items_with_type(int type_id, QString const& search,
                          int limit, int offset,
                          QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  QString sql("SELECT items.id, items.name, items.count, items.available, "
              "translations.name, types.name, "
              "CASE WHEN items.data_collector IS NULL"
              "          OR items.data_collector = 0 THEN 'Administrator'"
              "     ELSE TRIM(COALESCE(persons.firstname,'') || ' '"
              "              || COALESCE(persons.lastname,'')) END "
              "FROM items "
              "JOIN types ON items.type = types.id AND types.id = ");
  sql += QString().setNum(type_id) + " ";
  if(!search.isEmpty()) {
    sql += item_search_filter(read_collection(), search);
  }
  sql += "LEFT JOIN persons ON items.data_collector = persons.id "
         "LEFT JOIN translations ON translations.tablename = 'types'"
         " AND translations.language = " +
         QString().setNum(lang) +
         " AND types.name = translations.column "
         "ORDER BY items.name COLLATE NOCASE";
  if(limit > 0) {
    sql += " LIMIT " + QString::number(limit + 1) +
           " OFFSET " + QString::number(offset);
  }
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
    r.collector = q.value(6).toString();
    result.push_back(std::move(r));
  }
  return result;
}

/// Searches items by multiple optional fields.
/**
   Builds a SELECT with LEFT JOINs on schoolitems and translations.
   For \a general, every space-separated word must match at least one
   indexed field (AND of per-word ORs).  Uses FTS5 MATCH when available,
   LIKE otherwise.  When \a type_id > 0 each \a type_fields filter checks
   both item_list_values and \a tablename; otherwise only item_list_values.
*/
std::vector<colman::IQueries::SearchItemRow> colman::SQLQueries::
    query_items_for_search(QString const& general, QString const& name,
                           QString const& subject, QString const& description,
                           QString const& keywords, int type_id,
                           std::vector<TypeFieldFilter> const& type_fields,
                           int grade_min, int grade_max,
                           int limit, int offset, QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  // Grade-level (academic year) span filter.  When active, the overlap test
  // runs in C++ against the stored academic_year range string, so the SQL
  // LIMIT/OFFSET cannot be used and pagination is applied while iterating.
  bool const grade_active = grade_min > 0 || grade_max > 0;
  int const grade_lo = grade_min > 0 ? grade_min : 0;
  int const grade_hi = grade_max > 0 ? grade_max : 1000000;
  bool const has_fts = table_exists(db_, "items_fts");
  // Split on any non-letter/non-digit character so FTS5 special chars
  // (comma, colon, etc.) become delimiters rather than syntax errors.
  QStringList const fts_terms = [&] {
    QStringList t;
    if(has_fts) {
      for(QString const& w : general.split(
              QRegularExpression("[^\\p{L}\\p{N}]+"), Qt::SkipEmptyParts))
      {
        t << (w + "*");
      }
    }
    return t;
  }();
  bool const use_fts = !fts_terms.isEmpty();
  QString sql = "SELECT items.id, items.name, "
                "COALESCE(tr.name, types.name), "
                "items.count, items.available, items.type, "
                "COALESCE(res_agg.total_reserved, 0), "
                "schoolitems.academic_year "
                "FROM items "
                "JOIN types ON items.type = types.id "
                "LEFT JOIN translations tr ON tr.tablename = 'types' "
                "  AND tr.column = types.name AND tr.language = :lang "
                "LEFT JOIN ("
                "  SELECT item_id, SUM(count) AS total_reserved "
                "  FROM reservations WHERE expires_at >= date('now', 'localtime') "
                "  GROUP BY item_id"
                ") res_agg ON res_agg.item_id = items.id ";
  sql += "LEFT JOIN schoolitems ON schoolitems.item_id = items.id ";
  if(use_fts) {
    sql += "JOIN items_fts ON items_fts.rowid = items.id ";
  }
  sql += "WHERE 1=1";

  // Helper: append one AND condition to sql.
  auto add = [&](QString const& cond) { sql += " AND " + cond; };

  // Restrict to a specific type.
  if(type_id > 0) {
    add("items.type = " + QString::number(type_id));
  }

  // General search via FTS5 MATCH (fast) or word-by-word LIKE (fallback).
  if(!general.isEmpty()) {
    if(use_fts) {
      add("items_fts MATCH :fts_query");
    }
    else {
      for(QString const& word : general.split(' ', Qt::SkipEmptyParts)) {
        QString const w = like_escape(word);
        QString cond = "(items.name LIKE '%" + w +
                       "%' ESCAPE '\\' "
                       "OR EXISTS (SELECT 1 FROM item_list_values"
                       " WHERE item_id = items.id"
                       " AND value LIKE '%" +
                       w +
                       "%' ESCAPE '\\') "
                       "OR COALESCE(schoolitems.subject,'') LIKE '%" +
                       w +
                       "%' ESCAPE '\\' "
                       "OR COALESCE(items.description,'') LIKE '%" +
                       w +
                       "%' ESCAPE '\\' "
                       "OR COALESCE(items.keywords,'') LIKE '%" +
                       w + "%' ESCAPE '\\')";
        add(cond);
      }
    }
  }

  // Common field conditions.
  if(!name.isEmpty()) {
    add("items.name LIKE '%" + like_escape(name) + "%' ESCAPE '\\'");
  }
  if(!subject.isEmpty()) {
    add("COALESCE(schoolitems.subject,'') LIKE '%" + like_escape(subject) +
        "%' ESCAPE '\\'");
  }
  if(!description.isEmpty()) {
    add("COALESCE(items.description,'') LIKE '%" + like_escape(description) +
        "%' ESCAPE '\\'");
  }
  if(!keywords.isEmpty()) {
    add("COALESCE(items.keywords,'') LIKE '%" + like_escape(keywords) +
        "%' ESCAPE '\\'");
  }

  // Type-specific field conditions.
  for(auto const& tf : type_fields) {
    if(tf.value_.isEmpty()) {
      continue;
    }
    QString const col = like_escape(tf.value_);
    QString const esc_col = tf.column_;
    QString cond = "EXISTS (SELECT 1 FROM item_list_values"
                   " WHERE item_id = items.id AND column_name = '" +
                   esc_col + "' AND value LIKE '%" + col + "%' ESCAPE '\\')";
    if(type_id > 0) {
      QString const tbl = get_type_tablename(type_id);
      if(!tbl.isEmpty()) {
        cond = "(" + cond + " OR EXISTS (SELECT 1 FROM \"" + tbl +
               "\""
               " WHERE item_id = items.id AND COALESCE(\"" +
               esc_col + "\", '') LIKE '%" + col + "%' ESCAPE '\\'))";
      }
    }
    add(cond);
  }

  sql += " ORDER BY items.name COLLATE NOCASE";
  // When grade filtering is active the LIMIT/OFFSET is applied in C++ below,
  // because the overlap test cannot be expressed in this SQL.
  bool const sql_paginate = limit > 0 && !grade_active;
  if(sql_paginate) {
    sql += " LIMIT :limit OFFSET :offset";
  }

  QSqlQuery q(db_);
  q.prepare(sql);
  q.bindValue(":lang", static_cast<int>(lang));
  if(use_fts) {
    q.bindValue(":fts_query", fts_terms.join(' '));
  }
  if(sql_paginate) {
    q.bindValue(":limit", limit + 1);
    q.bindValue(":offset", offset);
  }
  if(!q.exec()) {
    qWarning() << "query_items_for_search failed:" << q.lastError();
    return {};
  }
  std::vector<SearchItemRow> result;
  int skipped = 0;
  while(q.next()) {
    // When a grade span is requested, keep only school items whose grade
    // range overlaps it; items without a grade level are excluded.
    if(grade_active) {
      QString const ays = q.value(7).toString();
      if(ays.isEmpty() ||
         !academic_years_overlap(string_to_academic_years(ays),
                                 grade_lo, grade_hi))
      {
        continue;
      }
      // C++ pagination over the grade-filtered rows.
      if(skipped < offset) {
        ++skipped;
        continue;
      }
      if(limit > 0 && static_cast<int>(result.size()) > limit) {
        break;
      }
    }
    SearchItemRow r;
    r.id = q.value(0).toInt();
    r.name = q.value(1).toString();
    r.type_locale = q.value(2).toString();
    r.count = q.value(3).toInt();
    r.available = q.value(4).toInt();
    r.type_id = q.value(5).toInt();
    r.total_reserved = q.value(6).toInt();
    result.push_back(std::move(r));
  }
  return result;
}

QStringList colman::SQLQueries::query_item_field_completions(
    QString const& field)
{
  return completer_values_from(
      db_, get_table_columns("items", 0), "items", field);
}

QStringList colman::SQLQueries::query_school_item_field_completions(
    QString const& field)
{
  return completer_values_from(
      db_, get_table_columns("schoolitems", 0), "schoolitems", field);
}

/// Queries all persons currently lending \a item_id.
/**
  @param item_id the database id of the item to query
  @return list of LendingPersonInfo entries; empty if none or on error
*/
std::vector<colman::IQueries::LendingPersonInfo> colman::SQLQueries::
    query_item_lendings(int item_id)
{
  std::vector<LendingPersonInfo> ret_val;
  QSqlQuery query(db_);
  query.prepare("SELECT persons.lastname, persons.firstname, persons.title, "
                "lendings.id "
                "FROM lendings JOIN persons ON lendings.item = :id "
                "AND lendings.person = persons.id ");
  query.bindValue(":id", item_id);
  if(query.exec()) {
    while(query.next()) {
      LendingPersonInfo info;
      info.person_last_name_ = query.value(0).toString();
      info.person_first_name_ = query.value(1).toString();
      info.person_title_ = query.value(2).toString();
      info.lending_id_ = query.value(3).toInt();
      ret_val.push_back(std::move(info));
    }
  }
  return ret_val;
}

// -----------------------------------------------------------------------
// Item CRUD
// -----------------------------------------------------------------------

/// Inserts a new item and returns the auto-assigned database id.
/**
 * Inserts \a item into the items table (letting SQLite assign the id),
 * creates the schoolitems row when item.get_is_school_item() is true, and
 * inserts all non-empty attribute values into the type-specific table.
 * All steps are wrapped in a single transaction.
 * @param item the item data to insert
 * @return the new database id on success, or -1 on failure
 */
int colman::SQLQueries::add_new_item(Item const& item)
{
  // get tablename for type:
  QString const tablename = get_tablename(item.get_type_id());
  if(tablename == "") {
    return -1;
  }
  Collection const collection = read_collection();
  // insert item into items table:
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("INSERT INTO items (type, name, count, place, "
                "description, thumbnail, data_collector, "
                "signature_top, signature_bottom, keywords) "
                "VALUES (:type, :name, :count, :place, :description,"
                " :thumbnail, :data_collector,"
                " :signature_top, :signature_bottom, :keywords)");
  query.bindValue(":type", item.get_type_id());
  query.bindValue(":name", item.get_name());
  query.bindValue(":count", 0);
  query.bindValue(":place", item.get_place());
  query.bindValue(":description", item.get_description());
  query.bindValue(":thumbnail", item.get_thumbnail_id());
  query.bindValue(":data_collector", item.get_data_collector_id());
  query.bindValue(":signature_top", item.get_signature_top());
  query.bindValue(":signature_bottom", item.get_signature_bottom());
  query.bindValue(":keywords", keywords_to_string(item.get_keywords()));
  if(!query.exec()) {
    qWarning() << "inserting into items failed:\n"
               << query.lastError();
    return -1;
  }
  QVariant const item_id = query.lastInsertId();
  // insert schoolitem part, if needed:
  if(item.get_is_school_item()) {
    query.prepare("INSERT INTO schoolitems "
                  "(item_id, subject, academic_year, type) "
                  "VALUES (:id, :subject, :academic_year, :type)");
    query.bindValue(":id", item_id);
    query.bindValue(":subject", item.get_subject());
    QString const ays = academic_years_to_string(item.get_academic_year());
    query.bindValue(":academic_year", ays);
    query.bindValue(":type", item.get_type());
    if(!query.exec()) {
      qWarning() << "could not insert item into schoolitems: "
                 << query.lastError();
      return -1;
    }
  }
  // insert item into its special table (e.g. 'books'):
  std::vector<QString> columns = get_table_columns(tablename, 1);
  std::sort(columns.begin(), columns.end());
  QString querystring("INSERT INTO ");
  querystring += tablename + " (item_id";
  for(auto const& value : item.get_values()) {
    if(!value.second.isEmpty() &&
       std::binary_search(columns.begin(), columns.end(), value.first))
    {
      querystring += ", \"" + value.first + "\"";
    }
  }
  querystring += ") VALUES (:item_id";
  for(auto const& value : item.get_values()) {
    if(!value.second.isEmpty() &&
       std::binary_search(columns.begin(), columns.end(), value.first))
    {
      querystring += ", :" + value.first;
    }
  }
  querystring += ")";
  query.prepare(querystring);
  query.bindValue(":item_id", item_id);
  for(auto const& value : item.get_values()) {
    if(!value.second.isEmpty() &&
       std::binary_search(columns.begin(), columns.end(), value.first))
    {
      query.bindValue(":" + value.first, value.second);
    }
  }
  if(!query.exec()) {
    qWarning() << "inserting into " << tablename << " failed:\n"
               << query.lastError() << "\nquery: " << querystring
               << "\nitem_id=\"" << item_id << "\"\n";
    return -1;
  }
  if(!insert_list_values(query, item, item_id)) {
    return -1;
  }
  rollback.commit();
  int const new_id = item_id.toInt();
  rebuild_fts_for_item(new_id);
  backup_after_change();
  return new_id;
}

/// Reads a complete item record from the database.
/**
 * Fetches the main item fields, the optional schoolitems data, the type
 * name, and all type-specific attribute values for the item with \a id.
 * @param id        the database id of the item to read
 * @param item      output: populated with the retrieved item data
 * @param type_name output: the internal type name of the item
 * @return true if the item was found and read successfully, false otherwise
 */
bool colman::SQLQueries::get_item(int id, Item& item, QString& type_name)
{
  QSqlQuery query(db_);
  query.prepare(
      "SELECT items.name, items.type, items.place, items.description, "
      "items.thumbnail, "
      "schoolitems.item_id, schoolitems.subject, "
      "schoolitems.academic_year, schoolitems.type, "
      "items.data_collector, "
      "items.signature_top, items.signature_bottom, "
      "items.keywords, "
      "items.allow_reservations, items.reservation_period_days, "
      "items.version "
      "FROM items LEFT JOIN schoolitems "
      "ON items.id = schoolitems.item_id "
      "WHERE items.id = :id ");
  // count, available
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "could not query item " << id << ": " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown item id " << id;
    return false;
  }
  item.clear();
  item.set_name(query.value(0).toString());
  item.set_type_id(query.value(1).toInt());
  item.set_place(query.value(2).toString());
  item.set_description(query.value(3).toString());
  item.set_thumbnail_id(query.value(4).toInt());
  if(!query.value(5).isNull() && query.value(5).toInt() >= 0) {
    item.set_is_school_item(true);
    item.set_subject(query.value(6).toString());
    item.set_academic_year(string_to_academic_years(query.value(7).toString()));
    item.set_type(query.value(8).toString());
  }
  item.set_data_collector_id(query.value(9).toInt());
  item.set_signature_top(query.value(10).toString());
  item.set_signature_bottom(query.value(11).toString());
  item.set_keywords(string_to_keywords(query.value(12).toString()));
  item.set_allow_reservations(
      query.value(13).isNull() ? -1 : query.value(13).toInt());
  item.set_reservation_period_days(
      query.value(14).isNull() ? -1 : query.value(14).toInt());
  item.set_version(query.value(15).toInt());
  item.set_barcode(compute_item_barcode(item.get_type_id(), id));
  query.prepare("SELECT name, tablename FROM types WHERE id = :id");
  query.bindValue(":id", item.get_type_id());
  if(!query.exec()) {
    qWarning() << "query type table failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown type " << item.get_type_id();
    return false;
  }
  type_name = query.value(0).toString();
  QString const table = query.value(1).toString();
  query.prepare("SELECT * FROM " + table + " WHERE item_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "query table " << table << " failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown table " << table;
    return false;
  }
  QSqlRecord rec = query.record();
  int const cols = rec.count();
  for(int c = 1; c < cols; ++c) {
    if(!query.value(c).isNull()) {
      item.set_value(rec.fieldName(c), query.value(c).toString());
    }
  }
  query.prepare("SELECT column_name, value FROM item_list_values "
                "WHERE item_id = :id ORDER BY column_name, position");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "query item_list_values failed: " << query.lastError();
    return false;
  }
  while(query.next()) {
    item.append_list_value(query.value(0).toString(),
                           query.value(1).toString());
  }
  return true;
}

/// Removes an item only when its stock size is zero.
/**
 * Checks whether any copies of the item still exist in the stock.  If
 * the stock size is greater than zero the removal is refused and false
 * is returned.  Otherwise delegates to force_remove_item().
 * @param id the database id of the item to remove
 * @return true on success, false if the stock is non-zero or a database
 *         error occurs
 */
bool colman::SQLQueries::remove_item(int id)
{
  if(stock_size(id) > 0) {
    // we still have some items of this type, we are not allowed to remove
    return false;
  }
  return force_remove_item(id);
}

/// Deletes an item and all related records unconditionally.
/**
 * Removes the item with \a id from the items table, its type-specific
 * table, the schoolitems table, and all associated acquisitions,
 * liquidations, lendings, and returns.  No stock or lending checks are
 * performed — use remove_item() if you need those guards.  All changes
 * are wrapped in a single transaction.
 * @param id the database id of the item to remove
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::force_remove_item(int id)
{
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("SELECT types.tablename FROM "
                "types JOIN items ON types.id = items.type "
                "WHERE items.id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "query item tablename failed: " << query.lastError();
    return false;
  }
  if(!query.next()) {
    qWarning() << "unknown item tablename";
    return false;
  }
  QString const table = query.value(0).toString();

  query.prepare("DELETE FROM item_list_values WHERE item_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item_list_values failed: " << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM items WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item failed: " << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM schoolitems WHERE item_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "could not remove item from schoolitems: "
               << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM " + table + " WHERE item_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item from " << table
               << " failed: " << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM acquisitions WHERE item = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item from acquisitions failed: "
               << query.lastError();
    return false;
  }

  query.prepare("DELETE FROM liquidations WHERE item = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item from liquidations failed: "
               << query.lastError();
    return false;
  }

  query.prepare("SELECT id FROM lendings WHERE item = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "query lendings for items failed: " << query.lastError();
    return false;
  }
  while(query.next()) {
    int const lending = query.value(0).toInt();
    QSqlQuery inner(db_);
    inner.prepare("DELETE FROM returns WHERE lending = :lending");
    inner.bindValue(":lending", lending);
    if(!inner.exec()) {
      qWarning() << "deleting item from returns failed: " << inner.lastError();
      return false;
    }
  }
  query.prepare("DELETE FROM lendings WHERE item = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item from lendings failed: " << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM items_fts WHERE rowid = :id");
  query.bindValue(":id", id);
  query.exec(); // non-fatal: FTS5 may be unavailable
  rollback.commit();
  backup_after_change();
  return true;
}

/// Updates all stored fields of an existing item.
/**
 * Overwrites the items table row for \a id with the data from \a item,
 * updates or removes the corresponding schoolitems row depending on
 * item.get_is_school_item(), and updates the type-specific attribute
 * table (e.g. books).  All changes are wrapped in a single transaction.
 * @param id   the database id of the item to update
 * @param item the new item data
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::change_item(int id, Item const& item)
{
  AutoRollback rollback(db_);
  QString const tablename = get_tablename(item.get_type_id());
  if(tablename == "") {
    return false;
  }
  QSqlQuery query(db_);
  query.prepare("SELECT type FROM items WHERE id = :id");
  query.bindValue(":id", id);
  if(!query.exec() || !query.next()) {
    qWarning() << "could not read old type for item " << id;
    return false;
  }
  int const old_type_id = query.value(0).toInt();
  QString const old_tablename = get_tablename(old_type_id);
  bool const type_changed = (old_type_id != item.get_type_id());
  query.prepare("UPDATE items SET name = :name, type = :type, "
                "place = :place, description = :description, "
                "thumbnail = :thumbnail, "
                "data_collector = :data_collector, "
                "signature_top = :signature_top, "
                "signature_bottom = :signature_bottom, "
                "keywords = :keywords, "
                "allow_reservations = :allow_reservations, "
                "reservation_period_days = :reservation_period_days "
                "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":name", item.get_name());
  query.bindValue(":type", item.get_type_id());
  query.bindValue(":place", item.get_place());
  query.bindValue(":description", item.get_description());
  query.bindValue(":thumbnail", item.get_thumbnail_id());
  query.bindValue(":data_collector", item.get_data_collector_id());
  query.bindValue(":signature_top", item.get_signature_top());
  query.bindValue(":signature_bottom", item.get_signature_bottom());
  query.bindValue(":keywords", keywords_to_string(item.get_keywords()));
  query.bindValue(
      ":allow_reservations",
      item.get_allow_reservations() == -1
          ? QVariant()
          : QVariant(item.get_allow_reservations()));
  query.bindValue(
      ":reservation_period_days",
      item.get_reservation_period_days() == -1
          ? QVariant()
          : QVariant(item.get_reservation_period_days()));
  if(!query.exec()) {
    qWarning() << "could not change item: " << query.lastError();
    return false;
  }
  // change schoolitem part:
  if(item.get_is_school_item()) {
    query.prepare("INSERT OR REPLACE INTO schoolitems "
                  "(item_id, subject, academic_year, type) "
                  "VALUES (:id, :subject, :academic_year, :type)");
    query.bindValue(":id", id);
    query.bindValue(":subject", item.get_subject());
    query.bindValue(":academic_year",
                    academic_years_to_string(item.get_academic_year()));
    query.bindValue(":type", item.get_type());
    if(!query.exec()) {
      qWarning() << "could not insert item into schoolitems: "
                 << query.lastError();
      return false;
    }
  }
  else {
    query.prepare("DELETE FROM schoolitems WHERE item_id = :id");
    query.bindValue(":id", id);
    if(!query.exec()) {
      qWarning() << "could not remove item from schoolitems: "
                 << query.lastError();
      return false;
    }
  }
  // change item in its special table (e.g. 'books'):
  if(type_changed) {
    // Remove row from old type table and insert into new one.
    if(!old_tablename.isEmpty()) {
      query.prepare("DELETE FROM " + old_tablename +
                    " WHERE item_id = :id");
      query.bindValue(":id", id);
      if(!query.exec()) {
        qWarning() << "deleting from " << old_tablename
                   << " failed: " << query.lastError();
        return false;
      }
    }
    std::vector<QString> columns = get_table_columns(tablename, 1);
    std::sort(columns.begin(), columns.end());
    QString querystring("INSERT INTO ");
    querystring += tablename + " (item_id";
    for(auto const& value : item.get_values()) {
      if(!value.second.isEmpty() &&
         std::binary_search(columns.begin(), columns.end(), value.first)) {
        querystring += ", \"" + value.first + "\"";
      }
    }
    querystring += ") VALUES (:item_id";
    for(auto const& value : item.get_values()) {
      if(!value.second.isEmpty() &&
         std::binary_search(columns.begin(), columns.end(), value.first)) {
        querystring += ", :" + value.first;
      }
    }
    querystring += ")";
    query.prepare(querystring);
    query.bindValue(":item_id", id);
    for(auto const& value : item.get_values()) {
      if(!value.second.isEmpty() &&
         std::binary_search(columns.begin(), columns.end(), value.first)) {
        query.bindValue(":" + value.first, value.second);
      }
    }
    if(!query.exec()) {
      qWarning() << "inserting into " << tablename
                 << " failed: " << query.lastError()
                 << "\nquery: " << querystring;
      return false;
    }
  }
  else if(!item.get_values().empty()) {
    std::vector<QString> columns = get_table_columns(tablename, 1);
    std::sort(columns.begin(), columns.end());
    QString querystring("UPDATE ");
    querystring += tablename + " SET ";
    bool first = true;
    for(auto const& value : item.get_values()) {
      if(std::binary_search(columns.begin(), columns.end(), value.first)) {
        if(!first) {
          querystring += ", ";
        }
        querystring += "\"" + value.first + "\" = :" + value.first;
        first = false;
      }
    }
    querystring += " WHERE item_id = :item_id";
    query.prepare(querystring);
    query.bindValue(":item_id", id);
    for(auto const& value : item.get_values()) {
      if(std::binary_search(columns.begin(), columns.end(), value.first)) {
        query.bindValue(":" + value.first, value.second);
      }
    }
    if(!query.exec()) {
      qWarning() << "changing item in " << tablename << " failed:\n"
                 << query.lastError() << "\nquery: " << querystring;
      return false;
    }
  }
  query.prepare("DELETE FROM item_list_values WHERE item_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    qWarning() << "deleting item_list_values failed: " << query.lastError();
    return false;
  }
  if(!insert_list_values(query, item, id)) {
    return false;
  }
  rollback.commit();
  rebuild_fts_for_item(id);
  backup_after_change();
  return true;
}

// -----------------------------------------------------------------------
// Item status and lookup
// -----------------------------------------------------------------------

/// Removes attribute values that do not belong to the item's type.
/**
 * Queries the column list of the type-specific table for item.get_type_id()
 * and discards any key/value pairs in \a item whose key is not a valid
 * column.  This keeps the Item consistent with the current type definition
 * before an INSERT or UPDATE.
 * @param item the item whose values are filtered in-place
 */
void colman::SQLQueries::restrict_item_to_type(Item& item)
{
  QString const tablename = get_tablename(item.get_type_id());
  if(tablename.isEmpty()) {
    return;
  }
  std::vector<QString> columns = get_table_columns(tablename, 1);
  std::sort(columns.begin(), columns.end());
  if(!item.get_values().empty()) {
    StringMap new_map;
    for(auto const& i : item.get_values()) {
      if(std::binary_search(columns.begin(), columns.end(), i.first)) {
        new_map.insert(i);
      }
    }
    item.swap_values(new_map);
  }
  if(!item.get_list_values().empty()) {
    StringListMap new_list_map;
    for(auto const& i : item.get_list_values()) {
      if(std::binary_search(columns.begin(), columns.end(), i.first)) {
        new_list_map.insert(i);
      }
    }
    item.swap_list_values(new_list_map);
  }
}

/// Returns the number of currently available copies of an item.
/**
 * Reads the available column from the items table for \a item_id.
 * @param item_id the database id of the item to query
 * @return the number of available copies, or 0 if the item is unknown
 */
unsigned int colman::SQLQueries::available(int item_id)
{
  QSqlQuery query(db_);
  query.prepare("SELECT available FROM items WHERE id = :id");
  query.bindValue(":id", item_id);
  if(!query.exec()) {
    qWarning() << "get number of available items failed: " << query.lastError();
    return 0;
  }
  if(!query.next()) {
    qWarning() << "unknown item '" << item_id << "'\n";
    return 0;
  }
  return query.value(0).toUInt();
}

/// Returns the total stock count of an item.
/**
 * Reads the count column from the items table for \a item_id.
 * @param item_id the database id of the item to query
 * @return the total stock count, or 0 if the item is unknown
 */
unsigned int colman::SQLQueries::stock_size(int item_id)
{
  QSqlQuery query(db_);
  query.prepare("SELECT count FROM items WHERE id = :id");
  query.bindValue(":id", item_id);
  if(!query.exec()) {
    qWarning() << "get number of items failed: " << query.lastError();
    return 0;
  }
  if(!query.next()) {
    qWarning() << "unknown item '" << item_id << "'\n";
    return 0;
  }
  return query.value(0).toUInt();
}

/// Returns true if the item has at least one open (unreturned) lending.
/**
 * Queries the lendings and returns tables for \a item_id to find any
 * lending whose total returned count is less than the lent count.
 * @param item_id the database id of the item to check
 * @return true if there is at least one open lending, false otherwise
 */
bool colman::SQLQueries::have_lendings(int item_id)
{
  if(item_id > 0) {
    QSqlQuery query(db_);
    query.prepare("SELECT lendings.id FROM lendings LEFT JOIN returns ON "
                  "lendings.id = returns.lending "
                  "WHERE lendings.item = :item_id "
                  "GROUP BY lendings.id "
                  "HAVING (SUM(returns.count) IS NULL "
                  "OR SUM(returns.count) < lendings.count);");
    query.bindValue(":item_id", item_id);
    if(!query.exec()) {
      qWarning() << "query lendings failed: " << query.lastError();
      return 0;
    }
    if(query.next()) {
      return true;
    }
  }
  return false;
}

/// Returns (lending_id, person_id) pairs for all open lendings of an item.
/**
 * A lending is considered open when the total returned count is less than
 * the lent count.
 * @param item_id the item to query
 * @return        list of (lending_id, person_id) pairs; empty on error or
 *                if no open lendings exist
 */
std::vector<std::pair<int, int>> colman::SQLQueries::open_lendings_for_item(
    int item_id)
{
  std::vector<std::pair<int, int>> result;
  QSqlQuery query(db_);
  query.prepare(
      "SELECT lendings.id, lendings.person "
      "FROM lendings LEFT JOIN returns ON lendings.id = returns.lending "
      "WHERE lendings.item = :item_id "
      "GROUP BY lendings.id, lendings.person "
      "HAVING (SUM(returns.count) IS NULL "
      "OR SUM(returns.count) < lendings.count)");
  query.bindValue(":item_id", item_id);
  if(!query.exec()) {
    qWarning() << "open_lendings_for_item failed:" << query.lastError();
    return result;
  }
  while(query.next()) {
    result.emplace_back(query.value(0).toInt(), query.value(1).toInt());
  }
  return result;
}

/// Returns (lending_id, item_id) pairs for all open lendings of a person.
/**
 * A lending is considered open when the total returned count is less than
 * the lent count.
 * @param person_id the person to query
 * @return          list of (lending_id, item_id) pairs; empty on error or
 *                  if the person has no open lendings
 */
std::vector<std::pair<int, int>> colman::SQLQueries::open_lendings_for_person(
    int person_id)
{
  std::vector<std::pair<int, int>> result;
  QSqlQuery query(db_);
  query.prepare(
      "SELECT lendings.id, lendings.item "
      "FROM lendings LEFT JOIN returns ON lendings.id = returns.lending "
      "WHERE lendings.person = :person_id "
      "GROUP BY lendings.id, lendings.item "
      "HAVING (SUM(returns.count) IS NULL "
      "OR SUM(returns.count) < lendings.count)");
  query.bindValue(":person_id", person_id);
  if(!query.exec()) {
    qWarning() << "open_lendings_for_person failed:" << query.lastError();
    return result;
  }
  while(query.next()) {
    result.emplace_back(query.value(0).toInt(), query.value(1).toInt());
  }
  return result;
}

/// Returns the item ids of all items whose ISBN10 or ISBN13 equals \a isbn.
/**
 * Queries the books table joined with items.  Only book-type items are
 * returned; items of other types are not searched.
 * @param isbn the ISBN-10 or ISBN-13 string to look up
 * @return     list of matching item ids; empty when none found or on error
 */
std::vector<int> colman::SQLQueries::items_by_isbn(QString const& isbn)
{
  std::vector<int> result;
  QSqlQuery query(db_);
  query.prepare("SELECT DISTINCT items.id FROM items "
                "JOIN item_list_values ilv ON ilv.item_id = items.id "
                "WHERE ilv.column_name IN ('ISBN10', 'ISBN13') "
                "AND ilv.value = :isbn");
  query.bindValue(":isbn", isbn);
  if(!query.exec()) {
    qWarning() << "items_by_isbn failed:" << query.lastError();
    return result;
  }
  while(query.next()) {
    result.push_back(query.value(0).toInt());
  }
  return result;
}

/// Returns item ids of all items whose EAN matches \a ean.
/**
 * Queries all type-specific tables that have an "ean" column.
 * Both the given form and the 12-digit form (without leading zero when
 * \a ean is a 13-digit value starting with '0') are checked, because
 * EAN values may be stored without a leading zero (e.g. when imported
 * from DNB MARC21 field 024).
 * @param ean  EAN string to look up (13-digit or 12-digit)
 * @return     list of matching item ids; empty when none found or on error
 */
std::vector<int> colman::SQLQueries::items_by_ean(QString const& ean)
{
  std::vector<int> result;
  // Collect both EAN forms to search for.
  QStringList forms;
  forms << ean;
  if(ean.size() == 13 && ean.startsWith('0')) {
    forms << ean.mid(1);
  }
  // Find all type tablenames that have an "ean" column.
  QSqlQuery type_q(db_);
  type_q.prepare("SELECT t.tablename FROM types t "
                 "JOIN data_types dt ON dt.type_id = t.id "
                 "WHERE dt.column = 'ean'");
  if(!type_q.exec()) {
    qWarning() << "items_by_ean type query failed:" << type_q.lastError();
    return result;
  }
  while(type_q.next()) {
    QString const tbl = type_q.value(0).toString();
    for(QString const& form : forms) {
      QSqlQuery q(db_);
      q.prepare("SELECT item_id FROM \"" + tbl + "\" WHERE ean = :ean");
      q.bindValue(":ean", form);
      if(!q.exec()) {
        qWarning() << "items_by_ean query failed for" << tbl << ":"
                   << q.lastError();
        continue;
      }
      while(q.next()) {
        int const id = q.value(0).toInt();
        if(std::find(result.begin(), result.end(), id) == result.end()) {
          result.push_back(id);
        }
      }
    }
  }
  return result;
}

/// Computes the barcode string for an item using the collection's prefix.
/**
 * Reads the collection from \a db to obtain the barcode prefix, then
 * delegates to the Collection overload of compute_item_barcode().
 * @param type_id the database id of the item's type
 * @param item_id the database id of the item
 * @return the barcode string for the item
 */
QString colman::SQLQueries::compute_item_barcode(int type_id, int item_id)
{
  Collection col = read_collection();
  return colman::compute_item_barcode(col, type_id, item_id);
}

/// Parses a barcode string into its type and item id components.
/**
 * Reads the collection from \a db to obtain the barcode prefix, then
 * delegates to the Collection overload of parse_item_barcode().
 * @param barcode the barcode string to parse
 * @param type_id output: the type id encoded in the barcode
 * @param item_id output: the item id encoded in the barcode
 * @return true if \a barcode is a valid item barcode for this collection
 */
bool colman::SQLQueries::parse_item_barcode(QString const& barcode,
                                            int& type_id, int& item_id)
{
  Collection col = read_collection();
  return colman::parse_item_barcode(col, barcode, type_id, item_id);
}

/// Returns true if \a barcode is an item barcode for this collection.
/**
 * Reads the collection from \a db to obtain the barcode prefix, then
 * delegates to the Collection overload of is_item_barcode().
 * @param barcode the barcode string to test
 * @return true if \a barcode encodes an item belonging to this collection
 */
bool colman::SQLQueries::is_item_barcode(QString const& barcode)
{
  Collection col = read_collection();
  return colman::is_item_barcode(col, barcode);
}

/// Queries all items ordered by name.
/**
  @return Vector of (id, name) pairs ordered by item name.
*/
std::vector<std::pair<int, QString>> colman::SQLQueries::
    get_items_ordered_by_name()
{
  std::vector<std::pair<int, QString>> ret_val;
  QSqlQuery query(db_);
  if(!query.exec("SELECT id, name FROM items ORDER BY name")) {
    qWarning() << "get_items_ordered_by_name failed:" << query.lastError();
    return ret_val;
  }
  while(query.next()) {
    ret_val.push_back({query.value(0).toInt(), query.value(1).toString()});
  }
  return ret_val;
}

/// Queries the ids of the most recently acquired items.
/**
  Groups acquisitions by item, orders the items by their latest acquisition
  date/time (newest first) and returns at most \a count item ids. Items
  without any acquisition record are omitted.
  @param count maximum number of item ids to return
  @return item ids, newest acquisition first (at most \a count)
*/
std::vector<int> colman::SQLQueries::get_recently_acquired_item_ids(int count)
{
  std::vector<int> ret_val;
  if(count <= 0) {
    return ret_val;
  }
  QSqlQuery query(db_);
  query.prepare("SELECT item FROM acquisitions "
                "GROUP BY item "
                "ORDER BY MAX(datetime) DESC, MAX(id) DESC "
                "LIMIT :count");
  query.bindValue(":count", count);
  if(!query.exec()) {
    qWarning() << "get_recently_acquired_item_ids failed:" << query.lastError();
    return ret_val;
  }
  while(query.next()) {
    ret_val.push_back(query.value(0).toInt());
  }
  return ret_val;
}

/// Queries the ids of items acquired within a time span.
/**
  Returns the ids of items whose acquisition date/time falls in the inclusive
  range [\a from, \a to], each id at most once, ordered by id. Items without
  any matching acquisition record are omitted.
  @param from start of the time span (inclusive)
  @param to   end of the time span (inclusive)
  @return item ids ordered by id
*/
std::vector<int>
colman::SQLQueries::get_items_acquired_between(QDateTime const& from,
                                               QDateTime const& to)
{
  std::vector<int> ret_val;
  QSqlQuery query(db_);
  query.prepare("SELECT DISTINCT item FROM acquisitions "
                "WHERE datetime >= :from AND datetime <= :to "
                "ORDER BY item");
  query.bindValue(":from", from);
  query.bindValue(":to", to);
  if(!query.exec()) {
    qWarning() << "get_items_acquired_between failed:" << query.lastError();
    return ret_val;
  }
  while(query.next()) {
    ret_val.push_back(query.value(0).toInt());
  }
  return ret_val;
}

/// Queries all items with their type name and thumbnail id.
/**
  @return Vector of ItemTypeThumbnail ordered by item name.
*/
std::vector<colman::IQueries::ItemTypeThumbnail> colman::SQLQueries::
    get_items_with_type_and_thumbnails()
{
  std::vector<IQueries::ItemTypeThumbnail> result;
  QSqlQuery q(db_);
  if(!q.exec("SELECT items.id, items.name, types.name, items.thumbnail "
             "FROM items JOIN types ON items.type = types.id "
             "ORDER BY items.name COLLATE NOCASE"))
  {
    qWarning() << "get_items_with_type_and_thumbnails failed:" << q.lastError();
    return result;
  }
  while(q.next()) {
    ItemTypeThumbnail r;
    r.item_id_ = q.value(0).toInt();
    r.item_name_ = q.value(1).toString();
    r.type_name_ = q.value(2).toString();
    r.thumb_id_ = q.value(3).toInt();
    result.push_back(std::move(r));
  }
  return result;
}

// -----------------------------------------------------------------------
// Thumbnails
// -----------------------------------------------------------------------

/// Saves a thumbnail to disk and records its location in the database.
/**
 * Writes \a thumbnail to the path computed from \a key, then inserts a
 * row into the thumbnails table with the given \a url and the relative
 * filename.  If \a overwrite is false and the file already exists, the
 * write is skipped but the database entry is still created.
 * @param thumbnail the pixmap to save
 * @param key       the thumbnail key used to derive the filename and path
 * @param url       the remote URL from which the thumbnail was fetched
 * @param overwrite whether to overwrite an existing file on disk
 * @return the new thumbnail database id on success, or -1 on failure
 */
int colman::SQLQueries::store_thumbnail(QByteArray const& png,
                                        QString const& key, QString const& url,
                                        bool overwrite)
{
  Collection const col = read_collection();
  QString const fname = key_to_thumbnail_path(key, thumbnail_dir_);
  if(write_bytes(png, fname, overwrite) ||
     (!overwrite && QFileInfo::exists(fname)))
  {
    QSqlQuery query(db_);
    query.prepare("INSERT INTO thumbnails (url, local_file) "
                  "VALUES (:url, :file)");
    query.bindValue(":url", url);
    query.bindValue(":file", key_to_thumbnail_file(key));
    if(query.exec()) {
      int const id = query.lastInsertId().toInt();
      qWarning() << "stored thumbnail for " << key << " in "
                 << key_to_thumbnail_file(key) << " with id " << id;
      return id;
    }
    else {
      qWarning() << "inserting thumbnail info failed: " << query.lastError();
    }
  }
  return -1;
}

/// Loads a thumbnail's raw PNG bytes from disk by its database id.
/**
 * Looks up the local_file entry for \a id in the thumbnails table,
 * resolves the absolute path, and reads the file into \a png.
 * @param thumbnail_id the database id of the thumbnail to load
 * @param png output: the raw PNG bytes
 * @return true if the file was found and read successfully, false otherwise
 */
bool colman::SQLQueries::get_thumbnail(int thumbnail_id, QByteArray& png)
{
  QSqlQuery query(db_);
  query.prepare("SELECT local_file FROM thumbnails WHERE id = :id");
  query.bindValue(":id", thumbnail_id);
  if(!query.exec()) {
    qWarning() << "query thumbnail failed: " << query.lastError();
  }
  else if(query.next()) {
    QString const fname =
        filename_to_thumbnail_path(query.value(0).toString(), thumbnail_dir_);
    QFile file(fname);
    if(file.open(QIODevice::ReadOnly)) {
      png = file.readAll();
      return !png.isEmpty();
    }
  }
  else {
    qWarning() << "thumbnail not found: " << thumbnail_id;
  }
  return false;
}

/// Registers an already-existing local thumbnail file in the database.
/**
 * Checks whether the file for \a key already exists on disk.  If it does,
 * returns the id of an existing thumbnails row for that file, or inserts a
 * new row if none is found.  Returns -1 if the file does not exist.
 * @param key the thumbnail key used to derive the filename and path
 * @return the thumbnail database id on success, or -1 if the file is absent
 */
int colman::SQLQueries::use_local_thumbnail(QString const& key)
{
  Collection const col = read_collection();
  QString const fname = key_to_thumbnail_path(key, thumbnail_dir_);
  qWarning() << "looking for local thumbnail: " << fname;
  if(QFileInfo::exists(fname)) {
    QSqlQuery query(db_);
    query.prepare("SELECT id FROM thumbnails WHERE local_file = :file");
    query.bindValue(":file", key_to_thumbnail_file(key));
    if(query.exec() && query.next()) {
      qWarning() << "using already existing thumbnail database entry"
                 << query.value(0).toInt();
      return query.value(0).toInt();
    }
    query.prepare("INSERT INTO thumbnails (url, local_file) "
                  "VALUES (:url, :file)");
    query.bindValue(":url", "");
    query.bindValue(":file", key_to_thumbnail_file(key));
    if(query.exec()) {
      QUrl const url = QUrl::fromLocalFile(QFileInfo(fname).absoluteFilePath());
      qWarning() << "stored reference to local file: " << url.toString();
      return query.lastInsertId().toInt();
    }
    else {
      qWarning() << "inserting thumbnail info failed: " << query.lastError();
    }
  }
  else {
    qWarning() << "no local thumbnail found";
  }
  return -1;
}

/// Returns true if the local file for a thumbnail record exists on disk.
/**
 * Queries the thumbnails table for \a thumbnail_id, resolves the absolute
 * path of the stored local_file, and checks whether the file exists.
 * @param thumbnail_id the database id of the thumbnail to check
 * @return true if the corresponding file exists on disk, false otherwise
 */
bool colman::SQLQueries::have_local_thumbnail(int thumbnail_id)
{
  QSqlQuery query(db_);
  query.prepare("SELECT local_file FROM thumbnails WHERE id = :id");
  query.bindValue(":id", thumbnail_id);
  if(!query.exec()) {
    qWarning() << "query thumbnail failed: " << query.lastError();
  }
  else if(query.next()) {
    QString const fname =
        filename_to_thumbnail_path(query.value(0).toString(), thumbnail_dir_);
    return QFileInfo::exists(fname);
  }
  return false;
}

// -----------------------------------------------------------------------
// Private availability helpers
// -----------------------------------------------------------------------

/// Increments the available count for an item by \a count.
/**
 * Reads the current available value and writes back the sum.  Used when
 * items are returned to the collection.
 * @param item_id the database id of the item to update
 * @param count   the number of copies to add to the available count
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::add_to_available(int item_id, unsigned int count)
{
  unsigned int const old = available(item_id);
  QSqlQuery query(db_);
  query.prepare("UPDATE items SET available = :available "
                "WHERE id = :id");
  query.bindValue(":id", item_id);
  query.bindValue(":available", old + count);
  if(!query.exec()) {
    qWarning() << "updating number of available items failed: "
               << query.lastError();
    return false;
  }
  return true;
}

// Decrements the available count for an item by \a count.
/**
 * Reads the current available value and writes back the difference.
 * Returns false without modifying the database if available is less
 * than \a count.
 * @param item_id the database id of the item to update
 * @param count   the number of copies to subtract from the available count
 * @return true on success, false if available < count or a query fails
 */
bool colman::SQLQueries::subtract_from_available(int item_id,
                                                 unsigned int count)
{
  unsigned int avail = available(item_id);
  if(avail >= count) {
    QSqlQuery query(db_);
    query.prepare("UPDATE items SET available = :available "
                  "WHERE id = :id");
    query.bindValue(":id", item_id);
    query.bindValue(":available", avail - count);
    if(!query.exec()) {
      qWarning() << "updating number of available items failed: "
                 << query.lastError();
      return false;
    }
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------
// Private FTS helpers
// -----------------------------------------------------------------------

/// Rebuilds the FTS5 index row for a single item.
/**
 * Removes any stale row for \a item_id and re-inserts it by joining the
 * items, books, and schoolitems tables.  A no-op if FTS5 is unavailable.
 * @param item_id the database id of the item to re-index
 */
void colman::SQLQueries::rebuild_fts_for_item(int item_id)
{
  QSqlQuery q(db_);
  q.prepare("DELETE FROM items_fts WHERE rowid = :id");
  q.bindValue(":id", item_id);
  if(!q.exec()) {
    return; // FTS5 unavailable
  }
  q.prepare("INSERT INTO items_fts(rowid, name, description, keywords,"
            " authors, isbn10, isbn13, subject)"
            " SELECT i.id, COALESCE(i.name,''), COALESCE(i.description,''),"
            " COALESCE(i.keywords,''),"
            " COALESCE((SELECT GROUP_CONCAT(ilv.value,' ')"
            "   FROM item_list_values ilv"
            "   WHERE ilv.item_id=i.id AND ilv.column_name='authors'),''),"
            " COALESCE((SELECT GROUP_CONCAT(ilv.value,' ')"
            "   FROM item_list_values ilv"
            "   WHERE ilv.item_id=i.id AND ilv.column_name='ISBN10'),''),"
            " COALESCE((SELECT GROUP_CONCAT(ilv.value,' ')"
            "   FROM item_list_values ilv"
            "   WHERE ilv.item_id=i.id AND ilv.column_name='ISBN13'),''),"
            " COALESCE(s.subject,'')"
            " FROM items i"
            " LEFT JOIN schoolitems s ON s.item_id = i.id"
            " WHERE i.id = :id");
  q.bindValue(":id", item_id);
  q.exec();
}

/// Rebuilds the entire FTS5 items_fts table from scratch.
/**
 * Clears all existing FTS rows and re-populates from the current items,
 * books, and schoolitems tables.  A no-op if FTS5 is unavailable.
 */
void colman::SQLQueries::rebuild_all_fts()
{
  QSqlQuery q(db_);
  if(!q.exec("DELETE FROM items_fts")) {
    return; // FTS5 unavailable
  }
  q.exec("INSERT INTO items_fts(rowid, name, description, keywords,"
         " authors, isbn10, isbn13, subject)"
         " SELECT i.id, COALESCE(i.name,''), COALESCE(i.description,''),"
         " COALESCE(i.keywords,''),"
         " COALESCE((SELECT GROUP_CONCAT(ilv.value,' ')"
         "   FROM item_list_values ilv"
         "   WHERE ilv.item_id=i.id AND ilv.column_name='authors'),''),"
         " COALESCE((SELECT GROUP_CONCAT(ilv.value,' ')"
         "   FROM item_list_values ilv"
         "   WHERE ilv.item_id=i.id AND ilv.column_name='ISBN10'),''),"
         " COALESCE((SELECT GROUP_CONCAT(ilv.value,' ')"
         "   FROM item_list_values ilv"
         "   WHERE ilv.item_id=i.id AND ilv.column_name='ISBN13'),''),"
         " COALESCE(s.subject,'')"
         " FROM items i"
         " LEFT JOIN schoolitems s ON s.item_id = i.id");
}

/// Recomputes items.count and items.available from the history tables.
/**
   Used after a batch deletion to restore a consistent state.
   @param item_id the item to update
   @return true on success
*/
bool colman::SQLQueries::recompute_item_counts(int item_id)
{
  QSqlQuery q(db_);
  q.prepare(
      "SELECT "
      "COALESCE((SELECT SUM(count) FROM acquisitions WHERE item=:id1), 0)"
      " - COALESCE((SELECT SUM(count) FROM liquidations WHERE item=:id2), 0)");
  q.bindValue(":id1", item_id);
  q.bindValue(":id2", item_id);
  if(!q.exec()) {
    qWarning() << "recompute stock failed:" << q.lastError();
    return false;
  }
  if(!q.next()) {
    return false;
  }
  int const stock = q.value(0).toInt();
  q.prepare("SELECT "
            "COALESCE((SELECT SUM(count) FROM lendings WHERE item=:id1), 0)"
            " - COALESCE((SELECT SUM(r.count) FROM returns r"
            "   JOIN lendings le ON r.lending=le.id WHERE le.item=:id2), 0)");
  q.bindValue(":id1", item_id);
  q.bindValue(":id2", item_id);
  if(!q.exec()) {
    qWarning() << "recompute unreturned failed:" << q.lastError();
    return false;
  }
  if(!q.next()) {
    return false;
  }
  int const unreturned = q.value(0).toInt();
  q.prepare("UPDATE items SET count=:count, available=:avail WHERE id=:id");
  q.bindValue(":count", stock);
  q.bindValue(":avail", stock - unreturned);
  q.bindValue(":id", item_id);
  if(!q.exec()) {
    qWarning() << "recompute item update failed:" << q.lastError();
    return false;
  }
  return true;
}

/* EOF */
