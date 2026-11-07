// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Private implementation helpers shared by all colman_sql_queries_*.cpp files.
/**
   @file
   @author Christoph Pesch

   This header is not part of the public API.  It is included only by the
   colman_sql_queries_*.cpp translation units and must never be included by
   callers of SQLQueries.
*/

#ifndef COLMAN_SQL_QUERIES_IMPL_HPP
#  define COLMAN_SQL_QUERIES_IMPL_HPP

#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_DB_UTIL_HPP
#    include "colman_db_util.hpp"
#  endif
#  ifndef SYSTEM_QLOCALE
#    include <QLocale>
#    define SYSTEM_QLOCALE
#  endif
#  ifndef SYSTEM_QSQLDATABASE
#    include <QSqlDatabase>
#    define SYSTEM_QSQLDATABASE
#  endif
#  ifndef SYSTEM_QSQLQUERY
#    include <QSqlQuery>
#    define SYSTEM_QSQLQUERY
#  endif
#  ifndef SYSTEM_QSQLERROR
#    include <QSqlError>
#    define SYSTEM_QSQLERROR
#  endif
#  ifndef SYSTEM_QVARIANT
#    include <QVariant>
#    define SYSTEM_QVARIANT
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif
#  ifndef SYSTEM_ALGORITHM
#    include <algorithm>
#    define SYSTEM_ALGORITHM
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif

namespace colman
{

  /**
   * RAII guard that begins a database transaction on construction and
   * automatically rolls it back on destruction unless commit() is called
   * first.  This makes exception- and early-return-safe transactions
   * straightforward: call commit() only at the successful end of an
   * operation; any other exit path causes a rollback.
   *
   * Uses BEGIN IMMEDIATE so the write lock is acquired at transaction start.
   * This prevents concurrent writers from reading stale availability data
   * and then racing to commit — the second writer blocks until the first
   * finishes, then re-reads the current state.
   */
  struct AutoRollback
  {
    /**
     * Begins an IMMEDIATE transaction on \a db.
     * @param db the database connection to manage
     */
    AutoRollback(QSqlDatabase& db)
        : db_(db), rollback_(true)
    {
      QSqlQuery(db_).exec("BEGIN IMMEDIATE");
    }

    /**
     * Rolls back the transaction if commit() has not been called.
     */
    ~AutoRollback()
    {
      if(rollback_) {
        QSqlQuery(db_).exec("ROLLBACK");
      }
    }

    /**
     * Commits the transaction and disarms the automatic rollback.
     * After this call the destructor will not roll back.
     */
    void commit()
    {
      QSqlQuery q(db_);
      if(q.exec("COMMIT")) {
        rollback_ = false;
      }
    }

  private:
    QSqlDatabase& db_; ///< the database connection being managed
    bool rollback_; ///< true while the transaction should still be rolled back
  };

  /// Returns true if \a lang has a translation in the database.
  /**
   * AnyLanguage (0) is the internal fallback used to store raw type names and
   * must not be treated as a real locale: if system_language() cannot detect
   * a meaningful language, we report "not supported" so the caller falls back
   * to displaying the raw type name rather than an identical-looking English
   * fallback that would be mistaken for a translation.
   * @param lang  language to check; AnyLanguage causes an immediate false return
   */
  inline bool locale_supported(QSqlDatabase const& db, QLocale::Language lang)
  {
    if(lang == QLocale::AnyLanguage || lang == QLocale::C) {
      return false;
    }
    QSqlQuery query(
        "SELECT DISTINCT language FROM translations "
        "WHERE tablename = 'types' AND language = " +
            QString().setNum(lang),
        db);
    return query.next();
  }

  /// Returns true if the system locale has a translation in the database.
  inline bool locale_supported(QSqlDatabase const& db)
  {
    return locale_supported(db, colman::system_language());
  }

  /// Checks that the row \a id in \a table has the given \a expected_version.
  /**
   * Must be called inside an active BEGIN IMMEDIATE transaction.
   * @param db               the database connection
   * @param table            name of the entity table (e.g. "items")
   * @param id               primary key of the row to check
   * @param expected_version version the caller expects (from ETag / If-Match)
   * @return Ok if the version matched; VersionMismatch, NotFound, or Error
   *         otherwise
   */
  inline IQueries::UpdateResult check_version(QSqlDatabase& db,
                                              char const* table, int id,
                                              int expected_version)
  {
    if(expected_version < 0) {
      return IQueries::UpdateResult::Ok;
    }
    QSqlQuery q(db);
    q.prepare(
        QString("SELECT version FROM \"%1\" WHERE id = :id").arg(table));
    q.bindValue(":id", id);
    if(!q.exec()) {
      return IQueries::UpdateResult::Error;
    }
    if(!q.next()) {
      return IQueries::UpdateResult::NotFound;
    }
    if(q.value(0).toInt() != expected_version) {
      return IQueries::UpdateResult::VersionMismatch;
    }
    return IQueries::UpdateResult::Ok;
  }

  /// Escapes a string for use in a SQL LIKE pattern.
  /**
   * Doubles single quotes and escapes %, _, and \ using backslash so the
   * result can be used inside `LIKE 'val%' ESCAPE '\'`.
   * @param s the user-supplied search string
   * @return the escaped string
   */
  inline QString like_escape(QString const& s)
  {
    return QString(s)
        .replace("\\", "\\\\")
        .replace("'", "''")
        .replace("%", "\\%")
        .replace("_", "\\_");
  }

  /// Returns true when the table named \a name exists in the database.
  inline bool table_exists(QSqlDatabase& db, QString const& name)
  {
    QSqlQuery q(db);
    q.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='" +
           QString(name).replace("'", "''") + "'");
    return q.next();
  }

  /// Retrieves both the stock size and the available count for an item.
  /**
   * Queries the count and available columns of the items table for
   * \a item_id and stores the results in the output parameters.
   * @param db         the open database connection
   * @param item_id    the database id of the item to query
   * @param stock_size output: total stock count
   * @param available  output: number of currently available copies
   * @return true on success, false if the item is unknown or the query fails
   */
  inline bool stock_size_and_available(QSqlDatabase& db, int item_id,
                                       unsigned int& stock_size,
                                       unsigned int& available)
  {
    QSqlQuery query(db);
    query.prepare("SELECT count, available FROM items WHERE id = :id");
    query.bindValue(":id", item_id);
    if(!query.exec()) {
      qWarning() << "get number of available items failed: "
                 << query.lastError();
      return false;
    }
    if(!query.next()) {
      qWarning() << "unknown item '" << item_id << "'\n";
      return false;
    }
    stock_size = query.value(0).toUInt();
    available = query.value(1).toUInt();
    return true;
  }

  /// Decrements both the stock size and the available count by \a count.
  /**
   * Retrieves the current stock size and available count, checks that
   * \a count does not exceed either, and writes back the decremented
   * values in a single UPDATE.  Used when a liquidation reduces the
   * total stock.
   * @param db      the open database connection
   * @param item_id the database id of the item to update
   * @param count   the number of copies to remove from stock and available
   * @return true on success, false if count exceeds available or stock, or
   *         a query fails
   */
  inline bool subtract_from_stock_size_and_available(QSqlDatabase& db,
                                                     int item_id,
                                                     unsigned int count)
  {
    unsigned int stock_size = 0;
    unsigned int available = 0;
    if(stock_size_and_available(db, item_id, stock_size, available) &&
       count <= available && count <= stock_size)
    {
      QSqlQuery query(db);
      query.prepare(
          "UPDATE items SET available = :available, count = :count "
          "WHERE id = :id");
      query.bindValue(":id", item_id);
      query.bindValue(":count", stock_size - count);
      query.bindValue(":available", available - count);
      if(!query.exec()) {
        qWarning()
            << "updating number of available items and stock size failed: "
            << query.lastError();
        return false;
      }
      return true;
    }
    return false;
  }

  /// Increments both the stock size and the available count by \a count.
  /**
   * Retrieves the current stock size and available count and writes back
   * the incremented values in a single UPDATE.
   * @param db      the open database connection
   * @param item_id the database id of the item to update
   * @param count   the number of copies to add to stock and available
   * @return true on success, false if the query fails
   */
  inline bool add_to_stock_size_and_available(QSqlDatabase& db, int item_id,
                                              unsigned int count)
  {
    unsigned int stock_size = 0;
    unsigned int available = 0;
    if(stock_size_and_available(db, item_id, stock_size, available)) {
      QSqlQuery query(db);
      query.prepare(
          "UPDATE items SET available = :available, count = :count "
          "WHERE id = :id");
      query.bindValue(":id", item_id);
      query.bindValue(":count", stock_size + count);
      query.bindValue(":available", available + count);
      if(!query.exec()) {
        qWarning()
            << "updating number of available items and stock size failed: "
            << query.lastError();
        return false;
      }
      return true;
    }
    return false;
  }

  /// Queries distinct values from \a field in \a tablename for a QCompleter.
  /**
   * @param db         the database connection
   * @param valid_cols whitelist of column names that may be queried
   * @param tablename  the database table to read from
   * @param field      the column whose distinct values are selected
   * @return sorted list of distinct non-empty values, or empty list if
   *         \a field is not in \a valid_cols
   */
  inline QStringList completer_values_from(
      QSqlDatabase& db, std::vector<QString> const& valid_cols,
      QString const& tablename, QString const& field)
  {
    if(std::find(valid_cols.begin(), valid_cols.end(), field) ==
       valid_cols.end()) {
      return {};
    }
    QSqlQuery q(db);
    q.exec(QString("SELECT DISTINCT ") + field + " FROM " + tablename +
           " WHERE " + field + " IS NOT NULL AND " + field +
           " != '' ORDER BY " + field);
    QStringList result;
    while(q.next()) {
      result << q.value(0).toString();
    }
    return result;
  }

  /// Inserts all repeatable field values from \a item into item_list_values.
  /**
   * @param query    pre-constructed QSqlQuery bound to the database
   * @param item     source item
   * @param item_id  the database id of the item row already inserted
   * @return true on success
   */
  inline bool insert_list_values(QSqlQuery& query, Item const& item,
                                 QVariant const& item_id)
  {
    for(auto const& lval : item.get_list_values()) {
      int pos = 0;
      for(auto const& val : lval.second) {
        query.prepare("INSERT INTO item_list_values "
                      "(item_id, column_name, position, value) "
                      "VALUES (:item_id, :col, :pos, :val)");
        query.bindValue(":item_id", item_id);
        query.bindValue(":col", lval.first);
        query.bindValue(":pos", pos);
        query.bindValue(":val", val);
        if(!query.exec()) {
          qWarning() << "inserting into item_list_values failed: "
                     << query.lastError();
          return false;
        }
        ++pos;
      }
    }
    return true;
  }

  /// Deletes expired reservations for \a item_id inside an open transaction.
  inline void purge_expired_reservation_unlocked(QSqlDatabase& db,
                                                 int item_id)
  {
    QSqlQuery q(db);
    q.prepare("DELETE FROM reservations "
              "WHERE item_id = :item_id "
              "AND expires_at < date('now', 'localtime')");
    q.bindValue(":item_id", item_id);
    q.exec();
  }

} // namespace colman

#endif /* #ifndef COLMAN_SQL_QUERIES_IMPL_HPP */

/* EOF */
