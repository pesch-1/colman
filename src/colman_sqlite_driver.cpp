// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Built-in Qt SQL driver for SQLite with encryption support
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sqlite_driver.hpp"
#ifndef SYSTEM_SQLITE3MC_AMALGAMATION_H
#  include <sqlite3mc_amalgamation.h>
#  define SYSTEM_SQLITE3MC_AMALGAMATION_H
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QDEBUG
#  include <QDebug>
#  define SYSTEM_QDEBUG
#endif
#ifndef SYSTEM_QSQLDRIVER
#  include <QSqlDriver>
#  define SYSTEM_QSQLDRIVER
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
#endif
#ifndef SYSTEM_QSQLFIELD
#  include <QSqlField>
#  define SYSTEM_QSQLFIELD
#endif
#ifndef SYSTEM_QSQLRECORD
#  include <QSqlRecord>
#  define SYSTEM_QSQLRECORD
#endif
#ifndef SYSTEM_QSQLRESULT
#  include <QSqlResult>
#  define SYSTEM_QSQLRESULT
#endif
#ifndef SYSTEM_QSTRINGLIST
#  include <QStringList>
#  define SYSTEM_QSTRINGLIST
#endif
#ifndef SYSTEM_QTIME
#  include <QTime>
#  define SYSTEM_QTIME
#endif
#ifndef SYSTEM_QVARIANT
#  include <QVariant>
#  define SYSTEM_QVARIANT
#endif

namespace
{
  /// Time in milliseconds to wait for a locked database (as in Qt's driver).
  int constexpr BusyTimeout = 5000;

  /// Cipher used for encrypted databases (ChaCha20-Poly1305).
  char constexpr CipherName[] = "chacha20";

  /// Returns a QSqlError for the last SQLite error on \a db.
  /**
   * @param text  description of the failed operation
   * @param db    connection the error occurred on (may be null)
   * @param type  Qt error category
   * @param code  SQLite result code
   */
  QSqlError make_error(QString const& text, sqlite3* db,
                       QSqlError::ErrorType type, int code)
  {
    QString const message =
        db ? QString::fromUtf8(sqlite3_errmsg(db)) : QString();
    return QSqlError(text, message, type, QString::number(code));
  }

  /// Returns true if a bound \a value is stored as SQL NULL.
  /**
   * Follows the rules of Qt's own drivers: besides invalid variants, null
   * strings and byte arrays, null dates and invalid times and date-times
   * count as NULL.
   */
  bool is_null_value(QVariant const& value)
  {
    if(value.isNull()) {
      return true;
    }
    switch(value.userType()) {
    case QMetaType::QString:
      return static_cast<QString const*>(value.constData())->isNull();
    case QMetaType::QByteArray:
      return static_cast<QByteArray const*>(value.constData())->isNull();
    case QMetaType::QDateTime:
      return !static_cast<QDateTime const*>(value.constData())->isValid();
    case QMetaType::QDate:
      return static_cast<QDate const*>(value.constData())->isNull();
    case QMetaType::QTime:
      return !static_cast<QTime const*>(value.constData())->isValid();
    default:
      return false;
    }
  }

  /// Binds \a str as UTF-16 text to parameter \a index of \a stmt.
  int bind_text(sqlite3_stmt* stmt, int index, QString const& str)
  {
    return sqlite3_bind_text16(stmt, index, str.utf16(),
                               int(str.size() * sizeof(QChar)),
                               SQLITE_TRANSIENT);
  }

  /// Binds \a value to parameter \a index of \a stmt.
  /**
   * Values are stored the same way as by Qt's QSQLITE driver, so existing
   * text formats (e.g. of date-times) are kept.
   * @return SQLite result code
   */
  int bind_value(sqlite3_stmt* stmt, int index, QVariant const& value)
  {
    if(is_null_value(value)) {
      return sqlite3_bind_null(stmt, index);
    }
    switch(value.userType()) {
    case QMetaType::QByteArray: {
      auto const* bytes =
          static_cast<QByteArray const*>(value.constData());
      return sqlite3_bind_blob(stmt, index, bytes->constData(),
                               int(bytes->size()), SQLITE_TRANSIENT);
    }
    case QMetaType::Int:
    case QMetaType::Bool:
      return sqlite3_bind_int(stmt, index, value.toInt());
    case QMetaType::Double:
      return sqlite3_bind_double(stmt, index, value.toDouble());
    case QMetaType::UInt:
    case QMetaType::LongLong:
      return sqlite3_bind_int64(stmt, index, value.toLongLong());
    case QMetaType::QDateTime:
      return bind_text(stmt, index,
                       value.toDateTime().toString(Qt::ISODateWithMs));
    case QMetaType::QTime:
      return bind_text(stmt, index,
                       value.toTime().toString(QStringLiteral(
                           "hh:mm:ss.zzz")));
    default:
      return bind_text(stmt, index, value.toString());
    }
  }

  /// Returns the value of \a column in the current row of \a stmt.
  QVariant column_value(sqlite3_stmt* stmt, int column)
  {
    switch(sqlite3_column_type(stmt, column)) {
    case SQLITE_INTEGER:
      return QVariant(qint64(sqlite3_column_int64(stmt, column)));
    case SQLITE_FLOAT:
      return QVariant(sqlite3_column_double(stmt, column));
    case SQLITE_BLOB: {
      auto const* data =
          static_cast<char const*>(sqlite3_column_blob(stmt, column));
      return QVariant(QByteArray(data, sqlite3_column_bytes(stmt, column)));
    }
    case SQLITE_NULL:
      return QVariant(QMetaType::fromType<QString>());
    default: {
      auto const* text =
          static_cast<QChar const*>(sqlite3_column_text16(stmt, column));
      int const length =
          sqlite3_column_bytes16(stmt, column) / int(sizeof(QChar));
      return QVariant(QString(text, length));
    }
    }
  }

  /// Maps a declared SQL column type to the Qt type of its values.
  QMetaType column_meta_type(char const* declared_type)
  {
    QString const type =
        QString::fromUtf8(declared_type ? declared_type : "").toLower();
    if(type.contains(QStringLiteral("int"))) {
      return QMetaType::fromType<qint64>();
    }
    if(type.startsWith(QStringLiteral("real")) ||
       type.startsWith(QStringLiteral("double")) ||
       type.startsWith(QStringLiteral("float")))
    {
      return QMetaType::fromType<double>();
    }
    if(type.startsWith(QStringLiteral("blob"))) {
      return QMetaType::fromType<QByteArray>();
    }
    if(type.startsWith(QStringLiteral("bool"))) {
      return QMetaType::fromType<bool>();
    }
    return QMetaType::fromType<QString>();
  }

  /// Result set of one statement on a SqliteDriver connection.
  /**
   * Rows are read from the SQLite statement as the query advances.  Moving
   * backwards re-executes the statement from the start, so random access
   * works but forward iteration is the efficient case.  The query size is
   * not known in advance (QSqlDriver::QuerySize is not supported).
   */
  class SqliteResult : public QSqlResult
  {
  public:
    /// Creates a result for the connection \a db of \a driver.
    SqliteResult(QSqlDriver const* driver, sqlite3* db)
        : QSqlResult(driver),
          db_(db)
    {
    }

    ~SqliteResult() override
    {
      finalize();
    }

  protected:
    /// Prepares and executes \a query without bound values.
    bool reset(QString const& query) override
    {
      return prepare(query) && exec();
    }

    /// Prepares \a query; placeholders are positional (`?`).
    /**
     * QSqlResult converts named placeholders to positional ones before
     * calling this function, because the driver does not report
     * QSqlDriver::NamedPlaceholders.
     */
    bool prepare(QString const& query) override
    {
      finalize();
      if(!db_) {
        setLastError(QSqlError(QStringLiteral("Database is not open"),
                               QString(), QSqlError::ConnectionError));
        return false;
      }
      void const* tail = nullptr;
      int const rc = sqlite3_prepare16_v2(
          db_, query.utf16(), int(query.size() * sizeof(QChar)), &stmt_,
          &tail);
      if(rc != SQLITE_OK) {
        setLastError(make_error(QStringLiteral("Unable to prepare statement"),
                                db_, QSqlError::StatementError, rc));
        finalize();
        return false;
      }
      QString const rest =
          tail ? QString::fromUtf16(static_cast<char16_t const*>(tail))
               : QString();
      if(!rest.trimmed().isEmpty()) {
        setLastError(QSqlError(
            QStringLiteral("Unable to execute multiple statements at a time"),
            QString(), QSqlError::StatementError));
        finalize();
        return false;
      }
      return true;
    }

    /// Binds the bound values and executes the prepared statement.
    /**
     * Steps onto the first row right away so that errors are reported by
     * exec() and not only by the first fetch.
     */
    bool exec() override
    {
      setAt(QSql::BeforeFirstRow);
      row_pending_ = false;
      if(!stmt_) {
        setLastError(QSqlError(QStringLiteral("No statement to execute"),
                               QString(), QSqlError::StatementError));
        return false;
      }
      sqlite3_reset(stmt_);
      sqlite3_clear_bindings(stmt_);
      QVariantList const values = boundValues();
      if(sqlite3_bind_parameter_count(stmt_) != int(values.size())) {
        setLastError(QSqlError(QStringLiteral("Parameter count mismatch"),
                               QString(), QSqlError::StatementError));
        return false;
      }
      for(int i = 0; i < int(values.size()); ++i) {
        int const rc = bind_value(stmt_, i + 1, values.at(i));
        if(rc != SQLITE_OK) {
          setLastError(make_error(QStringLiteral("Unable to bind parameters"),
                                  db_, QSqlError::StatementError, rc));
          return false;
        }
      }
      setSelect(sqlite3_column_count(stmt_) > 0);
      int const rc = sqlite3_step(stmt_);
      if(rc == SQLITE_ROW) {
        row_pending_ = true;
      }
      else if(rc == SQLITE_DONE) {
        sqlite3_reset(stmt_);
      }
      else {
        setLastError(make_error(QStringLiteral("Unable to execute statement"),
                                db_, QSqlError::StatementError, rc));
        sqlite3_reset(stmt_);
        return false;
      }
      setActive(true);
      return true;
    }

    /// Moves to the next row.
    bool fetchNext() override
    {
      if(!stmt_ || !isSelect() || at() == QSql::AfterLastRow) {
        return false;
      }
      if(row_pending_) {
        row_pending_ = false;
        setAt(0);
        return true;
      }
      if(at() == QSql::BeforeFirstRow) {
        return seek_from_start(0);
      }
      if(!step_row()) {
        return false;
      }
      setAt(at() + 1);
      return true;
    }

    /// Moves to row \a i.
    bool fetch(int i) override
    {
      if(i < 0 || !stmt_ || !isSelect()) {
        return false;
      }
      if(i == at()) {
        return true;
      }
      if(at() != QSql::AfterLastRow && i == at() + 1) {
        return fetchNext();
      }
      return seek_from_start(i);
    }

    /// Moves to the first row.
    bool fetchFirst() override
    {
      return fetch(0);
    }

    /// Moves to the last row.
    bool fetchLast() override
    {
      if(!seek_from_start(0)) {
        return false;
      }
      int last = 0;
      while(step_row()) {
        ++last;
      }
      return seek_from_start(last);
    }

    /// Returns the value of \a field in the current row.
    QVariant data(int field) override
    {
      if(!valid_field(field)) {
        return QVariant();
      }
      return column_value(stmt_, field);
    }

    /// Returns true if \a field in the current row is NULL.
    bool isNull(int field) override
    {
      if(!valid_field(field)) {
        return true;
      }
      return sqlite3_column_type(stmt_, field) == SQLITE_NULL;
    }

    /// Not supported: always -1 (see QSqlDriver::QuerySize).
    int size() override
    {
      return -1;
    }

    /// Returns the number of rows changed by the last statement.
    int numRowsAffected() override
    {
      return db_ ? sqlite3_changes(db_) : -1;
    }

    /// Returns the rowid of the last inserted row, if any.
    QVariant lastInsertId() const override
    {
      if(isActive() && db_) {
        qint64 const id = sqlite3_last_insert_rowid(db_);
        if(id != 0) {
          return QVariant(id);
        }
      }
      return QVariant();
    }

    /// Returns the column names and types of the result set.
    QSqlRecord record() const override
    {
      QSqlRecord record;
      if(!stmt_ || !isActive() || !isSelect()) {
        return record;
      }
      int const count = sqlite3_column_count(stmt_);
      for(int i = 0; i < count; ++i) {
        QString const name = QString::fromUtf16(
            static_cast<char16_t const*>(sqlite3_column_name16(stmt_, i)));
        record.append(QSqlField(
            name, column_meta_type(sqlite3_column_decltype(stmt_, i))));
      }
      return record;
    }

    /// Releases the locks held by a partially read result set.
    void detachFromResultSet() override
    {
      row_pending_ = false;
      if(stmt_) {
        sqlite3_reset(stmt_);
      }
    }

  private:
    sqlite3* db_ = nullptr;          ///< connection (owned by the driver)
    sqlite3_stmt* stmt_ = nullptr;   ///< prepared statement (owned)
    bool row_pending_ = false;       ///< exec() stepped onto the first row

    /// Finalizes the prepared statement.
    void finalize()
    {
      row_pending_ = false;
      if(stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
      }
    }

    /// Returns true if \a field can be read from the current row.
    bool valid_field(int field) const
    {
      return stmt_ && at() >= 0 && field >= 0 &&
             field < sqlite3_column_count(stmt_);
    }

    /// Steps to the next row; resets the statement at the end.
    /** @return true if a row is available */
    bool step_row()
    {
      int const rc = sqlite3_step(stmt_);
      if(rc == SQLITE_ROW) {
        return true;
      }
      if(rc != SQLITE_DONE) {
        setLastError(make_error(QStringLiteral("Unable to fetch row"), db_,
                                QSqlError::StatementError, rc));
      }
      sqlite3_reset(stmt_);
      return false;
    }

    /// Re-executes the statement and moves to row \a i.
    /** The bound values are kept.  @return false if there is no row \a i */
    bool seek_from_start(int i)
    {
      if(isForwardOnly() && at() > i) {
        return false;
      }
      row_pending_ = false;
      sqlite3_reset(stmt_);
      setAt(QSql::BeforeFirstRow);
      for(int row = 0; row <= i; ++row) {
        if(!step_row()) {
          return false;
        }
      }
      setAt(i);
      return true;
    }
  };

  /// Qt SQL driver for SQLite3MultipleCiphers connections.
  class SqliteDriver : public QSqlDriver
  {
  public:
    SqliteDriver() = default;

    ~SqliteDriver() override
    {
      close();
    }

    /// Reports the features supported by this driver.
    bool hasFeature(DriverFeature feature) const override
    {
      switch(feature) {
      case Transactions:
      case Unicode:
      case PreparedQueries:
      case PositionalPlaceholders:
      case BLOB:
      case LastInsertId:
      case SimpleLocking:
      case FinishQuery:
        return true;
      default:
        return false;
      }
    }

    /// Opens (or creates) the database file \a db.
    /**
     * User name, password, host, port and connection options are ignored;
     * an encrypted database is unlocked with set_key() after opening.
     */
    bool open(QString const& db, QString const&, QString const&,
              QString const&, int, QString const&) override
    {
      if(isOpen()) {
        close();
      }
      int const flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_NOMUTEX;
      int const rc =
          sqlite3_open_v2(db.toUtf8().constData(), &db_, flags, nullptr);
      if(rc != SQLITE_OK) {
        setLastError(make_error(QStringLiteral("Error opening database"),
                                db_, QSqlError::ConnectionError, rc));
        close();
        setOpenError(true);
        return false;
      }
      sqlite3_busy_timeout(db_, BusyTimeout);
      if(sqlite3mc_config(db_, "default:cipher",
                          sqlite3mc_cipher_index(CipherName)) < 0)
      {
        qWarning() << "SQLite driver: cannot select cipher" << CipherName;
      }
      setOpen(true);
      setOpenError(false);
      return true;
    }

    /// Closes the connection.
    /**
     * Uses sqlite3_close_v2(), so results that are still alive keep a valid
     * (zombie) connection until they are destroyed.
     */
    void close() override
    {
      if(db_) {
        sqlite3_close_v2(db_);
        db_ = nullptr;
      }
      if(isOpen()) {
        setOpen(false);
        setOpenError(false);
      }
    }

    /// Creates a result object for this connection.
    QSqlResult* createResult() const override
    {
      return new SqliteResult(this, db_);
    }

    bool beginTransaction() override
    {
      return exec_simple("BEGIN", QStringLiteral("Unable to begin transaction"));
    }

    bool commitTransaction() override
    {
      return exec_simple("COMMIT",
                         QStringLiteral("Unable to commit transaction"));
    }

    bool rollbackTransaction() override
    {
      return exec_simple("ROLLBACK",
                         QStringLiteral("Unable to roll back transaction"));
    }

    /// Returns the names of the tables and/or views of the database.
    QStringList tables(QSql::TableType type) const override
    {
      QStringList names;
      if(!db_) {
        return names;
      }
      QStringList kinds;
      if(type & QSql::Tables) {
        kinds << QStringLiteral("'table'");
      }
      if(type & QSql::Views) {
        kinds << QStringLiteral("'view'");
      }
      if(!kinds.isEmpty()) {
        QString const sql =
            QStringLiteral("SELECT name FROM sqlite_master WHERE type IN (") +
            kinds.join(',') +
            QStringLiteral(") AND name NOT LIKE 'sqlite_%'");
        for_each_row(sql, [&names](sqlite3_stmt* stmt) {
          names << column_value(stmt, 0).toString();
        });
      }
      if(type & QSql::SystemTables) {
        names << QStringLiteral("sqlite_master");
      }
      return names;
    }

    /// Returns the columns of \a table (empty if there is no such table).
    QSqlRecord record(QString const& table) const override
    {
      QSqlRecord record;
      QString const sql = QStringLiteral("PRAGMA table_info(") +
                          escapeIdentifier(table, TableName) + ')';
      // Columns of table_info: cid, name, type, notnull, dflt_value, pk
      for_each_row(sql, [&record](sqlite3_stmt* stmt) {
        QString const name = column_value(stmt, 1).toString();
        auto const* type =
            reinterpret_cast<char const*>(sqlite3_column_text(stmt, 2));
        record.append(QSqlField(name, column_meta_type(type), QString()));
      });
      return record;
    }

    /// Quotes \a identifier with double quotes unless already quoted.
    QString escapeIdentifier(QString const& identifier,
                             IdentifierType) const override
    {
      if(identifier.size() > 1 && identifier.startsWith('"') &&
         identifier.endsWith('"'))
      {
        return identifier;
      }
      QString escaped = identifier;
      escaped.replace('"', QStringLiteral("\"\""));
      return '"' + escaped + '"';
    }

    /// Unlocks the encrypted database with \a key.
    /** @return false if the key does not fit the database */
    bool set_key(QString const& key)
    {
      if(!db_) {
        return false;
      }
      QByteArray const utf8 = key.toUtf8();
      if(sqlite3_key_v2(db_, "main", utf8.constData(), int(utf8.size())) !=
         SQLITE_OK)
      {
        return false;
      }
      // The key is only checked when the database is first read.
      return sqlite3_exec(db_, "SELECT count(*) FROM sqlite_master",
                          nullptr, nullptr, nullptr) == SQLITE_OK;
    }

    /// Re-encrypts the database with \a key; an empty key decrypts it.
    bool rekey(QString const& key)
    {
      if(!db_) {
        return false;
      }
      QByteArray const utf8 = key.toUtf8();
      int const rc = sqlite3_rekey_v2(
          db_, "main", utf8.isEmpty() ? nullptr : utf8.constData(),
          int(utf8.size()));
      if(rc != SQLITE_OK) {
        qWarning() << "SQLite driver: rekey failed:" << sqlite3_errmsg(db_);
        return false;
      }
      return true;
    }

  private:
    sqlite3* db_ = nullptr; ///< open connection, null when closed

    /// Executes \a sql without results; sets the last error on failure.
    bool exec_simple(char const* sql, QString const& text)
    {
      if(!db_) {
        return false;
      }
      int const rc = sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
      if(rc != SQLITE_OK) {
        setLastError(
            make_error(text, db_, QSqlError::TransactionError, rc));
        return false;
      }
      return true;
    }

    /// Runs \a sql and calls \a on_row for each row of the result.
    template <typename OnRow>
    void for_each_row(QString const& sql, OnRow on_row) const
    {
      sqlite3_stmt* stmt = nullptr;
      if(!db_ ||
         sqlite3_prepare16_v2(db_, sql.utf16(),
                              int(sql.size() * sizeof(QChar)), &stmt,
                              nullptr) != SQLITE_OK)
      {
        return;
      }
      while(sqlite3_step(stmt) == SQLITE_ROW) {
        on_row(stmt);
      }
      sqlite3_finalize(stmt);
    }
  };

  /// Returns the built-in driver of \a db, or null for other drivers.
  SqliteDriver* sqlite_driver(QSqlDatabase const& db)
  {
    return dynamic_cast<SqliteDriver*>(db.driver());
  }

} // anonymous namespace

/**
 * Registers SqliteDriver under colman::SqliteDriverName.  QSqlDatabase
 * takes ownership of the driver creator.
 */
void colman::register_sqlite_driver()
{
  if(!QSqlDatabase::isDriverAvailable(SqliteDriverName)) {
    QSqlDatabase::registerSqlDriver(SqliteDriverName,
                                    new QSqlDriverCreator<SqliteDriver>);
  }
}

/**
 * Passes \a key to SQLite3MultipleCiphers and checks it by reading the
 * schema.  Must be called right after opening, before any other statement.
 * @param db   open connection using the built-in driver
 * @param key  passphrase of the database
 * @return false if \a db does not use the built-in driver or the key is
 *         wrong
 */
bool colman::set_database_key(QSqlDatabase const& db, QString const& key)
{
  auto* const driver = sqlite_driver(db);
  return driver && driver->set_key(key);
}

/**
 * Re-encrypts the database file with \a new_key, encrypts an unencrypted
 * file, or removes the encryption if \a new_key is empty.
 * @param db       open (and, if encrypted, unlocked) connection using the
 *                 built-in driver
 * @param new_key  new passphrase; empty to remove the encryption
 * @return true on success
 */
bool colman::change_database_key(QSqlDatabase const& db,
                                 QString const& new_key)
{
  auto* const driver = sqlite_driver(db);
  return driver && driver->rekey(new_key);
}

/* EOF */
