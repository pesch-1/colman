// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries core implementation: database lifecycle, collection, encryption.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sql_queries.hpp"
#ifndef COLMAN_SQL_QUERIES_IMPL_HPP
#  include "colman_sql_queries_impl.hpp"
#  define COLMAN_SQL_QUERIES_IMPL_HPP
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#  define COLMAN_PASSWORDS_HPP
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
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
#ifndef SYSTEM_QSQLDRIVER
#  include <QSqlDriver>
#  define SYSTEM_QSQLDRIVER
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QBUFFER
#  include <QBuffer>
#  define SYSTEM_QBUFFER
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
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
#ifndef SYSTEM_MAP
#  include <map>
#  define SYSTEM_MAP
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif

namespace
{
  /// Default signature top categories (translated when seeding).
  char const* const DefaultSignatureTops[] = {
      QT_TRANSLATE_NOOP("colman::SignatureCategories", "Picture books"),
      QT_TRANSLATE_NOOP("colman::SignatureCategories", "Comics"),
      QT_TRANSLATE_NOOP("colman::SignatureCategories", "Stories"),
      QT_TRANSLATE_NOOP("colman::SignatureCategories", "Non-fiction"),
      QT_TRANSLATE_NOOP("colman::SignatureCategories", "Magazines")};

  /// A default signature bottom category below a top category.
  struct DefaultSignatureBottom
  {
    char const* top_;  ///< top category (untranslated)
    char const* name_; ///< bottom category (untranslated)
  };

  /// Default signature bottom categories (translated when seeding).
  DefaultSignatureBottom const DefaultSignatureBottoms[] = {
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Adventure")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Action")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Fantasy")},
      {"Stories",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Friendship")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Spooky")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Crime")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Funny")},
      {"Stories",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Fairy tales")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "School")},
      {"Stories", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Animals")},
      {"Magazines",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Class newspaper")},
      {"Magazines", QT_TRANSLATE_NOOP("colman::SignatureCategories",
                                      "Knowledge & discovery")},
      {"Magazines",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Animals & nature")},
      {"Magazines",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Puzzles & crafts")},
      {"Magazines",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Comics & heroes")},
      {"Magazines",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Sport & action")},
      {"Non-fiction", QT_TRANSLATE_NOOP("colman::SignatureCategories",
                                        "History & evolution")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Body & health")},
      {"Non-fiction", QT_TRANSLATE_NOOP("colman::SignatureCategories", "Art")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Encyclopedia")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Mathematics")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Music")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Nature")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Plants")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Religion")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Sport & hobbies")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Technology")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Animals")},
      {"Non-fiction",
       QT_TRANSLATE_NOOP("colman::SignatureCategories", "Space")}};

  /// version of current data format in database
  QString const DataVersion("1.0");

  /**
   * Checks whether \a db contains a valid collection table with at least
   * one row.  Used to distinguish a genuine colman database from an
   * arbitrary SQLite file.
   * @param db the open database connection to inspect
   * @return true if the collection table exists and has at least one row
   */
  bool is_collection(QSqlDatabase& db)
  {
    QSqlQuery query(db);
    query.prepare("SELECT key FROM collection");
    if(!query.exec()) {
      qWarning() << "no collection file";
      return false;
    }
    else {
      if(!query.next()) {
        qWarning() << "no collection file";
        return false;
      }
    }
    return true;
  }

  /// Upserts (INSERT OR REPLACE) a single key/value row in the
  /// collection table.
  bool upsert_collection_kv(QSqlDatabase& db, QString const& key,
                            QVariant const& value)
  {
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO collection (key, value) "
              "VALUES (:k, :v)");
    q.bindValue(":k", key);
    q.bindValue(":v", value);
    if(!q.exec()) {
      qWarning() << "upsert collection key" << key
                 << "failed:" << q.lastError();
      return false;
    }
    return true;
  }

  /// Extracts a typed value from \a values into \a x if \a key is present.
  /**
   * Looks up \a key in \a values and, if found, converts the stored
   * QVariant to type T and assigns it to \a x.  If the key is absent
   * \a x is left unchanged.
   * @param x      output variable to receive the value
   * @param values the map to look up
   * @param key    the key to look for
   */
  template <typename T>
  void set(T& x, std::map<QString, QVariant> const& values, QString const& key)
  {
    auto const it = values.find(key);
    if(it != values.end()) {
      x = it->second.value<T>();
    }
  }

  template <typename T = QString>
  void set(colman::Collection& col, void (colman::Collection::*func)(T const&),
           std::map<QString, QVariant> const& values, QString const& key)
  {
    auto const it = values.find(key);
    if(it != values.end()) {
      (col.*func)(it->second.value<T>());
    }
  }

  /// Calls a Collection setter with a by-value argument found in \a values.
  /**
   * Looks up \a key in \a values and, if found, calls \a func on \a col
   * with the extracted value.  Designed for Collection setter methods
   * that accept a plain (non-reference) parameter such as int or bool.
   * @param col    the Collection object to update
   * @param func   pointer to the Collection setter member function
   * @param values the map to look up
   * @param key    the key to look for
   */
  template <typename T>
  void setv(colman::Collection& col, void (colman::Collection::*func)(T),
            std::map<QString, QVariant> const& values, QString const& key)
  {
    auto const it = values.find(key);
    if(it != values.end()) {
      (col.*func)(it->second.value<T>());
    }
  }

  /// Name of the Qt SQL driver that supports database encryption.
  auto constexpr CipherDriver="SQLITE_MULTIPLE_CIPHERS";

  /// Returns the cipher-capable SQLite driver if available, plain
  /// QSQLITE otherwise.
  QString preferred_driver()
  {
    if(QSqlDatabase::isDriverAvailable(CipherDriver)) {
      return CipherDriver;
    }
    qWarning() << CipherDriver
               << "driver not available, falling back to QSQLITE;"
               << "encryption disabled";
    return "QSQLITE";
  }

} // namespace

colman::SQLQueries::SQLQueries(FileDb,
                               std::function<void()> on_backup_error,
                               std::function<bool()> on_old_backups)
    : on_backup_error_{std::move(on_backup_error)}
    , on_old_backups_{std::move(on_old_backups)}
{
  db_ = QSqlDatabase::addDatabase(preferred_driver());
}

colman::SQLQueries::SQLQueries(QString const& conn_name)
    : conn_name_{conn_name}
{
  db_ = QSqlDatabase::addDatabase(preferred_driver(), conn_name_);
  db_.setDatabaseName(":memory:");
  if(db_.open()) {
    QSqlQuery(db_).exec("PRAGMA foreign_keys = ON");
    QSqlQuery(db_).exec("PRAGMA busy_timeout = 5000");
  }
}

colman::SQLQueries::~SQLQueries()
{
  if(!db_.isValid()) {
    return;
  }
  db_.close();
  db_ = QSqlDatabase();
  QSqlDatabase::removeDatabase(
      conn_name_.isEmpty() ? QSqlDatabase::defaultConnection : conn_name_);
}

void colman::SQLQueries::set_thumbnail_dir(QString const& dir)
{
  thumbnail_dir_ = dir;
}

QString colman::SQLQueries::thumbnail_dir() const
{
  return thumbnail_dir_;
}

/**
   Writes a backup of the current database file unconditionally.
   The backup is written to the database path with a numeric suffix
   (e.g. \c .col.~3~). On success, resets the change counter and marks the
   session-start backup as done.  If the total number of backup files exceeds
   20, old ones are removed (keeping at least 10); if \a on_old_backups_ is
   set, it is called first and deletion is skipped if it returns false.
   If the write fails and \a on_backup_error_ is set, it is called.
   No action is taken when the database is in-memory (no file path).
*/
void colman::SQLQueries::backup_once()
{
  auto const filename = db_.databaseName();
  if(filename.isEmpty() || filename == ":memory:") {
    return;
  }
  QFileInfo const info(filename);
  QString const basename = info.fileName();
  QStringList const files = info.dir().entryList(
      QStringList(basename + ".~*"));
  int min_unused = 1;
  for(auto&& f : files) {
    QString ref = f.mid(basename.size() + 2);
    if(!ref.endsWith('~')) {
      continue;
    }
    bool ok = false;
    int const num = ref.chopped(1).toInt(&ok);
    if(ok && num + 1 > min_unused) {
      min_unused = num + 1;
    }
  }
  auto const backup_path =
      filename + ".~" + QString().setNum(min_unused) + "~";
  if(save_database(backup_path)) {
    changes_since_last_backup_ = 0;
    session_backup_done_ = true;
    int const MaxBackups = 20;
    int const MinBackups = 10;
    if(files.size() > MaxBackups) {
      std::vector<std::pair<int, QString>> backups;
      for(auto&& f : files) {
        QString ref = f.mid(basename.size() + 2);
        if(!ref.endsWith('~')) {
          continue;
        }
        bool ok = false;
        int const num = ref.chopped(1).toInt(&ok);
        if(ok) {
          backups.push_back(std::make_pair(num, info.dir().filePath(f)));
        }
      }
      bool const do_delete = !on_old_backups_ || on_old_backups_();
      if(do_delete && (int)backups.size() > MinBackups) {
        std::sort(backups.begin(), backups.end());
        auto const n = backups.size();
        auto const keep = static_cast<size_t>(MinBackups);
        auto const end = n > keep ? n - keep : size_t{0};
        for(size_t i = 0; i < end; ++i) {
          QFile::remove(backups[i].second);
        }
      }
    }
  }
  else {
    if(on_backup_error_) {
      on_backup_error_();
    }
  }
}

/**
   Writes a session-start backup if none has been written yet, then
   increments the change counter and writes a backup once the configured
   threshold is reached.  The threshold is read from the application
   settings key \c "changes_until_backup" (default: 16).
   Delegates to backup_once() in both cases.
*/
void colman::SQLQueries::backup_after_change()
{
  if(!session_backup_done_) {
    backup_once();
    return;
  }
  QSettings settings;
  int const n = settings.value("changes_until_backup", 16).toInt();
  if(++changes_since_last_backup_ >= n) {
    backup_once();
  }
}

bool colman::SQLQueries::is_open() const
{
  return db_.isOpen();
}

bool colman::SQLQueries::is_locale_supported() const
{
  return locale_supported(db_);
}

// -----------------------------------------------------------------------
// Database file management
// -----------------------------------------------------------------------

/**
 * Creates a new SQLite collection database at \a filename.
 * Opens the file, creates all tables, computes and stores the barcode
 * prefix from the collection name and organisation, writes the collection
 * metadata, inserts the predefined item types, and inserts a built-in
 * Administrator person with id 0.
 * @param collection collection metadata; the barcode_prefix and
 *                   data_version fields are filled in by this function
 * @param filename   filesystem path of the new .col file to create
 * @return true on success, false if the file could not be opened or any
 *         initialisation step fails
 */
bool colman::SQLQueries::create_new_database(Collection& collection,
                                             QString const& filename)
{
  return create_new_database(collection, filename, {});
}

/**
 * Creates a new SQLite collection database at \a filename and optionally
 * encrypts it using \a key.  When \a key is non-empty the database is
 * created unencrypted first, then immediately re-keyed (encrypted) so
 * that the final file on disk is always protected.
 * @param collection collection metadata
 * @param filename   filesystem path of the new .col file
 * @param key        encryption passphrase; empty = no encryption
 * @return true on success
 */
bool colman::SQLQueries::create_new_database(Collection& collection,
                                             QString const& filename,
                                             QString const& key)
{
  if(!key.isEmpty() && !supports_encryption()) {
    qWarning() << "cannot create encrypted database:" << CipherDriver
               << "driver not available";
    return false;
  }
  if(db_.isValid()) {
    db_.setDatabaseName(filename);
    if(!db_.open()) {
      qWarning() << "could not open database: " << db_.lastError();
      return false;
    }
    QSqlQuery(db_).exec("PRAGMA foreign_keys = ON");
    QSqlQuery(db_).exec("PRAGMA busy_timeout = 5000");
    invalidate_collection_cache();
    create_tables();
    collection.set_data_version(DataVersion);
    collection.set_barcode_prefix(generate_barcode_prefix(
        collection.get_name(), collection.get_organization()));
    if(!create_collection(collection)) {
      qWarning("could not create collection");
      return false;
    }
    if(!add_predefined_types()) {
      return false;
    }
    Person admin;
    admin.set_lastname("Administrator");
    if(!add_person(0, admin)) {
      return false;
    }
    if(!create_default_permissions(0)) {
      return false;
    }
    if(!key.isEmpty()) {
      return encrypt_database(key);
    }
    return true;
  }
  return false;
}

bool colman::SQLQueries::add_administrator()
{
  Person admin;
  admin.set_lastname("Administrator");
  if(!add_person(0, admin)) {
    return false;
  }
  return create_default_permissions(0);
}

bool colman::SQLQueries::create_new_database(Collection& collection)
{
  if(db_.isValid()) {
    db_.setDatabaseName(":memory:");
    if(!db_.open()) {
      qWarning() << "could not open database: " << db_.lastError();
      return false;
    }
    QSqlQuery(db_).exec("PRAGMA foreign_keys = ON");
    QSqlQuery(db_).exec("PRAGMA busy_timeout = 5000");
    invalidate_collection_cache();
    create_tables();
    collection.set_data_version(DataVersion);
    collection.set_barcode_prefix(generate_barcode_prefix(
        collection.get_name(), collection.get_organization()));
    if(!create_collection(collection)) {
      qWarning("could not create collection");
      return false;
    }
    if(!add_predefined_types()) {
      return false;
    }
    if(!add_administrator()) {
      return false;
    }
    return true;
  }
  return false;
}

/**
 * Opens an existing .col database file and verifies that it is a valid
 * colman collection.
 * @param filename filesystem path of the .col file to load
 * @return true if the file was opened and contains a valid collection,
 *         false otherwise
 */
bool colman::SQLQueries::load_database(QString const& filename)
{
  return load_database(filename, {});
}

/**
 * Opens an existing encrypted .col database file.
 * Sets the decryption key via the QSQLITE_MC_KEY connection option so the
 * driver's open() call authenticates and decrypts the database header.
 * Returns false if the file cannot be opened or the passphrase is wrong.
 * @param filename  filesystem path of the .col file to load
 * @param key       encryption passphrase; must be non-empty for encrypted files
 */
bool colman::SQLQueries::load_database(QString const& filename,
                                       QString const& key)
{
  db_.setDatabaseName(filename);
  if(!key.isEmpty()) {
    if(!supports_encryption()) {
      qWarning() << "cannot open encrypted database:" << CipherDriver
                 << "driver not available";
      return false;
    }
    db_.setConnectOptions("QSQLITE_MC_KEY=" + key);
  }
  else {
    db_.setConnectOptions({});
  }
  if(!db_.open()) {
    db_.setConnectOptions({});
    qWarning() << "could not open database: " << db_.lastError();
    return false;
  }
  db_.setConnectOptions({});
  QSqlQuery(db_).exec("PRAGMA foreign_keys = ON");
  QSqlQuery(db_).exec("PRAGMA busy_timeout = 5000");
  invalidate_collection_cache();
  encryption_key_ = key;
  if(!is_collection(db_)) {
    return false;
  }
  // Apply schema migrations that existing databases predate.  load_database()
  // does not run create_tables(), so column additions must be applied here too.
  if(!ensure_media_kind_column()) {
    return false;
  }
  if(!ensure_predefined_translations()) {
    return false;
  }
  return true;
}

/// Adds the types.media_kind column and back-fills built-in types if missing.
/**
 * Existing databases created before the media_kind column was introduced are
 * migrated on open: the column is added and the built-in types are back-filled
 * by their canonical name.  Idempotent — when the column already exists this is
 * a single read with no writes.
 * @return true on success or when the column already exists
 */
bool colman::SQLQueries::ensure_media_kind_column()
{
  std::vector<QString> const cols = get_table_columns("types", 0);
  if(std::find(cols.begin(), cols.end(), QString("media_kind")) != cols.end()) {
    return true;
  }
  QSqlQuery query(db_);
  if(!query.exec("ALTER TABLE types ADD COLUMN media_kind INTEGER DEFAULT 0")) {
    qWarning() << "adding media_kind column failed: " << query.lastError();
    return false;
  }
  static std::pair<char const*, int> const BuiltinMediaKinds[] = {
      {"book", static_cast<int>(MediaKind::Book)},
      {"cd", static_cast<int>(MediaKind::Cd)},
      {"dvd", static_cast<int>(MediaKind::Dvd)},
      {"bluray", static_cast<int>(MediaKind::BluRay)},
      {"board_game", static_cast<int>(MediaKind::BoardGame)}};
  for(auto const& bk : BuiltinMediaKinds) {
    query.prepare("UPDATE types SET media_kind = :kind WHERE name = :name");
    query.bindValue(":kind", bk.second);
    query.bindValue(":name", QString(bk.first));
    if(!query.exec()) {
      qWarning() << "back-filling media_kind for '" << bk.first
                 << "' failed: " << query.lastError();
      return false;
    }
  }
  return true;
}

/**
   Fills the signature category tables with the default categories.
   The names are translated into the language of the user interface (context
   "colman::SignatureCategories"), so a new collection starts with categories
   in its user's language.  Existing categories are kept (INSERT OR IGNORE).
   @return true on success
*/
bool colman::SQLQueries::seed_signature_categories()
{
  QSqlQuery query(db_);
  for(char const* top : DefaultSignatureTops) {
    query.prepare("INSERT OR IGNORE INTO signature_top_categories (name) "
                  "VALUES (:name)");
    query.bindValue(":name", QCoreApplication::translate(
                                 "colman::SignatureCategories", top));
    if(!query.exec()) {
      qWarning() << "seeding signature top categories failed: "
                 << query.lastError();
      return false;
    }
  }
  for(DefaultSignatureBottom const& bottom : DefaultSignatureBottoms) {
    query.prepare("INSERT OR IGNORE INTO signature_bottom_categories "
                  "(top_category, name) VALUES (:top, :name)");
    query.bindValue(":top", QCoreApplication::translate(
                                "colman::SignatureCategories", bottom.top_));
    query.bindValue(":name", QCoreApplication::translate(
                                 "colman::SignatureCategories", bottom.name_));
    if(!query.exec()) {
      qWarning() << "seeding signature bottom categories failed: "
                 << query.lastError();
      return false;
    }
  }
  return true;
}

/**
 * Saves a copy of the open database to \a filename.
 * Flushes the WAL first so the main file is self-contained, then copies
 * it verbatim (preserving encryption state).
 * @param filename  target file path
 * @return true on success
 */
bool colman::SQLQueries::save_database(QString const& filename)
{
  QSqlQuery(db_).exec("PRAGMA wal_checkpoint(TRUNCATE)");
  QFile::remove(filename);
  if(!QFile::copy(db_.databaseName(), filename)) {
    qWarning() << "save_database: copy failed:" << filename;
    return false;
  }
  return true;
}

/// Creates all application tables in \a db if they do not yet exist.
/**
 * Issues a CREATE TABLE IF NOT EXISTS statement for every table used by
 * the application: collection, types, items, schoolitems, thumbnails,
 * acquisitions, liquidations, persons, classes, lendings, returns,
 * translations, and data_types.  All statements are wrapped in a single
 * transaction.
 * @return true if all tables were created (or already existed) successfully
 */
bool colman::SQLQueries::create_tables()
{
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("CREATE TABLE IF NOT EXISTS collection "
                "(key VARCHAR PRIMARY KEY, value BLOB)");
  if(!query.exec()) {
    qWarning() << "creating table collection failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS types "
                "(id INTEGER PRIMARY KEY, name VARCHAR, tablename VARCHAR, "
                "media_kind INTEGER DEFAULT 0)");
  if(!query.exec()) {
    qWarning() << "creating table types failed: " << query.lastError();
    return false;
  }
  // Migration: add the media_kind column to databases created before it
  // existed, then back-fill the built-in types by their canonical name.
  if(!ensure_media_kind_column()) {
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS items "
                "(id INTEGER PRIMARY KEY, name VARCHAR, type INTEGER, "
                "count INTEGER, place VARCHAR, "
                "available INTEGER, description VARCHAR, "
                "thumbnail INTEGER, data_collector INTEGER, "
                "signature_top VARCHAR, signature_bottom VARCHAR, "
                "keywords VARCHAR, version INTEGER DEFAULT 1, "
                "allow_reservations INTEGER, "
                "reservation_period_days INTEGER)");
  if(!query.exec()) {
    qWarning() << "creating table items failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS schoolitems "
                "(item_id INTEGER PRIMARY KEY, subject VARCHAR, "
                "academic_year VARCHAR, type VARCHAR)");
  if(!query.exec()) {
    qWarning() << "creating table schoolitems failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS thumbnails "
                "(id INTEGER PRIMARY KEY, url VARCHAR, local_file VARCHAR)");
  if(!query.exec()) {
    qWarning() << "creating table thumbnails failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS acquisitions "
                "(id INTEGER PRIMARY KEY, item INTEGER, count INTEGER, "
                "datetime DATETIME, data_collector INTEGER, "
                "version INTEGER DEFAULT 1)");
  if(!query.exec()) {
    qWarning() << "creating table acquisitions failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS liquidations "
                "(id INTEGER PRIMARY KEY, item INTEGER, count INTEGER, "
                "datetime DATETIME, data_collector INTEGER, "
                "version INTEGER DEFAULT 1)");
  if(!query.exec()) {
    qWarning() << "creating table liquidations failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS persons "
                "(id INTEGER PRIMARY KEY, lastname VARCHAR, "
                "firstname VARCHAR, "
                "title VARCHAR, school_class VARCHAR, "
                "emails VARCHAR, telephonenumbers VARCHAR, street VARCHAR, "
                "zip VARCHAR, city VARCHAR, country VARCHAR, remarks VARCHAR, "
                "thumbnail INTEGER, data_collector INTEGER, "
                "version INTEGER DEFAULT 1)");
  if(!query.exec()) {
    qWarning() << "creating table persons failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS classes "
                "(name VARCHAR, academic_year VARCHAR, "
                "count INTEGER, PRIMARY KEY(name, academic_year))");
  if(!query.exec()) {
    qWarning() << "creating table classes failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS lendings "
                "(id INTEGER PRIMARY KEY, item INTEGER, count INTEGER, "
                "person INTEGER, datetime DATETIME, duedate VARCHAR(10), "
                "remarks VARCHAR, data_collector INTEGER, "
                "version INTEGER DEFAULT 1)");
  if(!query.exec()) {
    qWarning() << "creating table lendings failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS returns "
                "(id INTEGER PRIMARY KEY, lending INTEGER, count INTEGER, "
                "datetime DATETIME, remarks VARCHAR, data_collector INTEGER, "
                "version INTEGER DEFAULT 1)");
  if(!query.exec()) {
    qWarning() << "creating table returns failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS translations "
                "(tablename VARCHAR, column VARCHAR, "
                "language VARCHAR, name VARCHAR, "
                "PRIMARY KEY(tablename, column, language))");
  if(!query.exec()) {
    qWarning() << "creating table translations failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS data_types "
                "(type_id INTEGER, column VARCHAR, "
                "data_type INTEGER, infos BLOB, "
                "position INTEGER DEFAULT 0, "
                "marc21_tag VARCHAR DEFAULT '', "
                "PRIMARY KEY(type_id, column))");
  if(!query.exec()) {
    qWarning() << "creating table data_types failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS common_field_positions "
                "(type_id INTEGER REFERENCES types(id), "
                "field VARCHAR, "
                "position INTEGER, "
                "PRIMARY KEY(type_id, field))");
  if(!query.exec()) {
    qWarning() << "creating table common_field_positions failed: "
               << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS credentials "
                "(person_id INTEGER PRIMARY KEY REFERENCES persons(id), "
                "username TEXT UNIQUE, "
                "password_hash TEXT, "
                "external_id TEXT UNIQUE)");
  if(!query.exec()) {
    qWarning() << "creating table credentials failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS permissions "
                "(person_id INTEGER PRIMARY KEY REFERENCES persons(id), "
                "edit_persons BOOLEAN, "
                "create_lenders BOOLEAN, "
                "create_data_collectors BOOLEAN, "
                "collect_lendings BOOLEAN, "
                "collect_returns BOOLEAN, "
                "collect_acquisitions BOOLEAN, "
                "collect_liquidations BOOLEAN, "
                "view_history BOOLEAN, "
                "view_persons BOOLEAN, "
                "view_items BOOLEAN, "
                "edit_preferences BOOLEAN, "
                "print_ids BOOLEAN, "
                "print_barcodes BOOLEAN, "
                "edit_categories BOOLEAN, "
                "view_personal_data BOOLEAN, "
                "manage_label_layouts BOOLEAN, "
                "calibrate_printers BOOLEAN, "
                "edit_id_layout BOOLEAN, "
                "print_signatures BOOLEAN, "
                "change_actions BOOLEAN, "
                "edit_program_settings BOOLEAN, "
                "manage_classes BOOLEAN, "
                "reserve BOOLEAN DEFAULT 1, "
                "edit_reservations BOOLEAN DEFAULT 0, "
                "admin_permissions BOOLEAN DEFAULT 0)");
  if(!query.exec()) {
    qWarning() << "creating table permissions failed: " << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS signature_top_categories "
                "(name VARCHAR PRIMARY KEY)");
  if(!query.exec()) {
    qWarning() << "creating table signature_top_categories failed: "
               << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS signature_bottom_categories "
                "(top_category VARCHAR, name VARCHAR, "
                "PRIMARY KEY (top_category, name))");
  if(!query.exec()) {
    qWarning() << "creating table signature_bottom_categories failed: "
               << query.lastError();
    return false;
  }
  if(!seed_signature_categories()) {
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS item_list_values "
                "(item_id INTEGER REFERENCES items(id), "
                "column_name VARCHAR, "
                "position INTEGER, "
                "value VARCHAR, "
                "PRIMARY KEY(item_id, column_name, position))");
  if(!query.exec()) {
    qWarning() << "creating table item_list_values failed: "
               << query.lastError();
    return false;
  }
  query.prepare("CREATE TABLE IF NOT EXISTS reservations "
                "(id INTEGER PRIMARY KEY, "
                "item_id INTEGER NOT NULL REFERENCES items(id), "
                "person_id INTEGER NOT NULL REFERENCES persons(id), "
                "count INTEGER NOT NULL DEFAULT 1, "
                "reserved_at TEXT NOT NULL, "
                "expires_at TEXT NOT NULL, "
                "version INTEGER DEFAULT 1, "
                "data_collector INTEGER, "
                "UNIQUE(item_id, person_id))");
  if(!query.exec()) {
    qWarning() << "creating table reservations failed: " << query.lastError();
    return false;
  }
  query.exec("CREATE INDEX IF NOT EXISTS idx_reservations_item_id "
             "ON reservations(item_id)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_reservations_person_id "
             "ON reservations(person_id)");
  // Performance indexes on frequently joined/filtered columns.
  query.exec("CREATE INDEX IF NOT EXISTS idx_items_type "
             "ON items(type)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_lendings_item "
             "ON lendings(item)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_lendings_person "
             "ON lendings(person)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_returns_lending "
             "ON returns(lending)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_acq_item "
             "ON acquisitions(item)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_liq_item "
             "ON liquidations(item)");
  query.exec("CREATE INDEX IF NOT EXISTS idx_ilv_item "
             "ON item_list_values(item_id)");
  // FTS5 full-text search index (non-fatal: FTS5 may not be compiled in).
  if(!query.exec("CREATE VIRTUAL TABLE IF NOT EXISTS items_fts "
                 "USING fts5(name, description, keywords, authors, "
                 "isbn10, isbn13, subject, "
                 "tokenize='unicode61 remove_diacritics 1')"))
  {
    qWarning() << "FTS5 not available; full-text search disabled:"
               << query.lastError();
  }
  // Per-table change counters for REST client polling (sharded Stonebraker).
  query.prepare("CREATE TABLE IF NOT EXISTS colman_table_versions "
                "(table_name TEXT PRIMARY KEY, "
                "version     INTEGER NOT NULL DEFAULT 0)");
  if(!query.exec()) {
    qWarning() << "creating table colman_table_versions failed:"
               << query.lastError();
    return false;
  }
  query.exec("INSERT OR IGNORE INTO colman_table_versions"
             "(table_name, version) VALUES"
             " ('items',0),('persons',0),('lendings',0),"
             " ('returns',0),('acquisitions',0),"
             " ('liquidations',0),('reservations',0),"
             " ('types',0),('classes',0),('collection',0)");
  // Triggers: increment the change counter whenever a row is inserted,
  // updated, or deleted in any tracked table.
  static char const* kTracked[] = {
      "items",        "persons",      "lendings", "returns",
      "acquisitions", "liquidations", "reservations",
      "types",        "classes",      "collection"};
  static char const* kOps[] = {"ins", "upd", "del"};
  static char const* kWhen[] = {"INSERT", "UPDATE", "DELETE"};
  for(auto const* tbl : kTracked) {
    for(int i = 0; i < 3; ++i) {
      auto const sql =
          QString("CREATE TRIGGER IF NOT EXISTS tv_%1_%2 "
                  "AFTER %3 ON %1 BEGIN "
                  "UPDATE colman_table_versions "
                  "SET version=version+1 "
                  "WHERE table_name='%1'; END")
              .arg(tbl)
              .arg(kOps[i])
              .arg(kWhen[i]);
      if(!query.exec(sql)) {
        qWarning() << "creating trigger tv_" << tbl << "_"
                   << kOps[i] << "failed:" << query.lastError();
        return false;
      }
    }
  }
  rollback.commit();
  return true;
}

/**
 * Returns the column names of \a table, starting at the column with
 * index \a start_index (0-based).  Typically called with start_index=1
 * to skip the item_id primary key column.
 * @param table       name of the table to inspect
 * @param start_index first column index to include in the result
 * @return vector of column name strings
 */
std::vector<QString> colman::SQLQueries::get_table_columns(QString const& table,
                                                           int start_index)
{
  QSqlRecord record = db_.record(table);
  int const n = record.count();
  std::vector<QString> retval;
  retval.reserve(n);
  for(int i = start_index; i < n; ++i) {
    retval.push_back(record.fieldName(i));
  }
  return retval;
}

// -----------------------------------------------------------------------
// Collection metadata
// -----------------------------------------------------------------------

/**
 * Inserts a new row for each metadata field of \a collection into the
 * collection table.  This is called once when a database is first
 * created; use change_collection() for subsequent updates.
 * @param collection the collection metadata to persist
 * @return true on success, false if the INSERT query fails
 */
bool colman::SQLQueries::create_collection(Collection const& collection)
{
  // Individual single-row INSERTs — multi-row VALUES with named parameters
  // is not reliably supported by all Qt SQLite driver variants.
  QSqlQuery query(db_);
  auto ins = [&](QString const& key, QVariant const& value) -> bool {
    query.prepare("INSERT INTO collection (key, value) VALUES (:k, :v)");
    query.bindValue(":k", key);
    query.bindValue(":v", value);
    if(!query.exec()) {
      qWarning() << "inserting into collection failed:\n"
                 << query.lastError();
      return false;
    }
    return true;
  };

  QByteArray const arr = collection.get_logo_raw();

  int dct = collection.get_default_item_type();
  if(dct < 0) {
    dct = 1;
  }

  if(!ins("name", collection.get_name())) {
    return false;
  }
  if(!ins("organization", collection.get_organization())) {
    return false;
  }
  if(!ins("responsible", collection.get_responsible())) {
    return false;
  }
  if(!ins("barcode_prefix", collection.get_barcode_prefix())) {
    return false;
  }
  if(!ins("data_version", collection.get_data_version())) {
    return false;
  }
  if(!ins("max_person_id", 0)) {
    return false;
  }
  if(!ins("logo", arr)) {
    return false;
  }
  if(!ins("use_lending_period", collection.get_use_lending_period())) {
    return false;
  }
  if(!ins("lending_period_years", collection.get_lending_period_years())) {
    return false;
  }
  if(!ins("lending_period_months", collection.get_lending_period_months())) {
    return false;
  }
  if(!ins("lending_period_days", collection.get_lending_period_days())) {
    return false;
  }
  if(!ins("allow_reservations", collection.get_allow_reservations())) {
    return false;
  }
  if(!ins("reservation_period_days", collection.get_reservation_period_days())) {
    return false;
  }
  if(!ins("default_item_type", dct)) {
    return false;
  }
  if(!ins("default_is_school_item", collection.get_default_is_school_item())) {
    return false;
  }
  if(!ins("grade_range_min", collection.get_grade_range_min())) {
    return false;
  }
  if(!ins("grade_range_max", collection.get_grade_range_max())) {
    return false;
  }
  if(!ins("server_name", collection.get_server_name())) {
    return false;
  }
  return true;
}

/**
 * Updates every stored metadata field of an existing collection with
 * values from \a collection.  All changes are wrapped in a single
 * transaction that is rolled back on any failure.  An empty
 * barcode_prefix or data_version triggers a warning but does not abort.
 * @param collection the updated collection metadata
 * @return true if all UPDATE statements succeed, false on any error
 */
bool colman::SQLQueries::change_collection(Collection const& collection)
{
  AutoRollback rollback(db_);
  if(!upsert_collection_kv(db_, "name", collection.get_name())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "organization", collection.get_organization())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "responsible", collection.get_responsible())) {
    return false;
  }
  QString const barcode_prefix = collection.get_barcode_prefix();
  if(!barcode_prefix.isEmpty()) {
    if(!upsert_collection_kv(db_, "barcode_prefix", barcode_prefix)) {
      return false;
    }
  }
  else {
    qWarning() << "WARNING: empty barcode prefix!!!";
  }
  QString const data_version = collection.get_data_version();
  if(!data_version.isEmpty()) {
    if(!upsert_collection_kv(db_, "data_version", data_version)) {
      return false;
    }
  }
  else {
    qWarning() << "WARNING: empty data version!!!";
  }
  QByteArray const arr = collection.get_logo_raw();
  if(!upsert_collection_kv(db_, "logo", arr)) {
    return false;
  }
  if(!upsert_collection_kv(db_, "use_lending_period",
                           collection.get_use_lending_period())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "lending_period_years",
                           collection.get_lending_period_years())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "lending_period_months",
                           collection.get_lending_period_months())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "lending_period_days",
                           collection.get_lending_period_days())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "allow_reservations",
                           collection.get_allow_reservations())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "reservation_period_days",
                           collection.get_reservation_period_days())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "default_item_type",
                           collection.get_default_item_type())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "default_is_school_item",
                           collection.get_default_is_school_item())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "grade_range_min",
                           collection.get_grade_range_min())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "grade_range_max",
                           collection.get_grade_range_max())) {
    return false;
  }
  if(!upsert_collection_kv(db_, "server_name",
                           collection.get_server_name())) {
    return false;
  }
  rollback.commit();
  invalidate_collection_cache();
  backup_after_change();
  return true;
}

namespace
{
  /// Reads all key/value rows from the collection table into a map.
  /**
   * Executes a SELECT on the collection table and returns every row as
   * a key-to-QVariant mapping.  Used by read_collection() to decouple
   * the raw SQL query from the Collection population logic.
   * @param db the open database connection
   * @return map of collection key strings to their stored QVariant values
   */
  std::map<QString, QVariant> query_collection(QSqlDatabase& db)
  {
    std::map<QString, QVariant> retval;
    QSqlQuery query(db);
    query.prepare("SELECT key, value FROM collection");
    if(!query.exec()) {
      qWarning() << "query collection failed:\n"
                 << query.lastError();
    }
    else {
      while(query.next()) {
        retval[query.value(0).toString()] = query.value(1);
      }
    }
    return retval;
  }

  /// Reads one IdCardElement from \a values using the given \a prefix.
  colman::IdCardElement read_id_card_element(
      std::map<QString, QVariant> const& values, QString const& prefix)
  {
    colman::IdCardElement e;
    double d = 0.0;
    int i = 0;
    set(d, values, prefix + "_x");
    e.x_mm_ = d;
    set(d, values, prefix + "_y");
    e.y_mm_ = d;
    set(d, values, prefix + "_w");
    e.width_mm_ = d;
    set(d, values, prefix + "_h");
    e.height_mm_ = d;
    set(i, values, prefix + "_rot");
    e.orientation_ = i;
    return e;
  }

} // namespace

/**
 * Reads all collection metadata from \a db and returns it as a
 * Collection object.  Unknown or missing keys are silently ignored.
 * The result is cached until the collection table version changes, so
 * changes made by another process on the same file are picked up.
 * @param db the open database connection
 * @return the Collection populated with the stored metadata
 */
colman::Collection colman::SQLQueries::read_collection()
{
  // Another process (desktop app vs. REST server) may have changed the
  // collection settings; its triggers bump the collection table version.
  int version = -1;
  {
    QSqlQuery vq(db_);
    if(vq.exec("SELECT version FROM colman_table_versions"
               " WHERE table_name = 'collection'") &&
       vq.next())
    {
      version = vq.value(0).toInt();
    }
  }
  if(collection_cache_valid_ && version == collection_cache_version_) {
    return collection_cache_;
  }
  std::map<QString, QVariant> const collection = query_collection(db_);
  Collection col;
  set(col, &Collection::set_name, collection, "name");
  set(col, &Collection::set_organization, collection, "organization");
  set(col, &Collection::set_responsible, collection, "responsible");
  set(col, &Collection::set_barcode_prefix, collection, "barcode_prefix");
  set(col, &Collection::set_data_version, collection, "data_version");
  setv<int>(col, &Collection::set_max_person_id, collection, "max_person_id");
  setv<int>(col, &Collection::set_default_item_type, collection,
            "default_item_type");
  setv<bool>(col, &Collection::set_default_is_school_item, collection,
             "default_is_school_item");
  QByteArray arr;
  set(arr, collection, "logo");
  col.set_logo_raw(arr);
  setv<bool>(col, &Collection::set_use_lending_period, collection,
             "use_lending_period");
  setv<int>(col, &Collection::set_lending_period_years, collection,
            "lending_period_years");
  setv<int>(col, &Collection::set_lending_period_months, collection,
            "lending_period_months");
  setv<int>(col, &Collection::set_lending_period_days, collection,
            "lending_period_days");
  setv<bool>(col, &Collection::set_allow_reservations, collection,
             "allow_reservations");
  setv<int>(col, &Collection::set_reservation_period_days, collection,
            "reservation_period_days");
  setv<int>(col, &Collection::set_grade_range_min, collection,
            "grade_range_min");
  setv<int>(col, &Collection::set_grade_range_max, collection,
            "grade_range_max");
  set(col, &Collection::set_server_name, collection, "server_name");
  // ID card layout
  {
    colman::IdCardLayout lay;
    double d = 0.0;
    int i = 0;
    set(d, collection, "id_card_card_w");
    lay.card_width_mm_ = d;
    set(d, collection, "id_card_card_h");
    lay.card_height_mm_ = d;
    set(i, collection, "id_card_has_logo");
    lay.has_logo_ = static_cast<bool>(i);
    lay.barcode_ = read_id_card_element(collection, "id_card_bc");
    lay.barcode_.type_ = colman::IdCardElementType::Barcode;
    lay.text_ = read_id_card_element(collection, "id_card_tx");
    lay.text_.type_ = colman::IdCardElementType::Text;
    lay.logo_ = read_id_card_element(collection, "id_card_lg");
    lay.logo_.type_ = colman::IdCardElementType::Logo;
    col.set_id_card_layout(lay);
  }
  // Item card layout
  {
    colman::IdCardLayout lay;
    double d = 0.0;
    int i = 0;
    set(d, collection, "item_card_card_w");
    lay.card_width_mm_ = d;
    set(d, collection, "item_card_card_h");
    lay.card_height_mm_ = d;
    set(i, collection, "item_card_has_logo");
    lay.has_logo_ = static_cast<bool>(i);
    lay.barcode_ = read_id_card_element(collection, "item_card_bc");
    lay.barcode_.type_ = colman::IdCardElementType::Barcode;
    lay.text_ = read_id_card_element(collection, "item_card_tx");
    lay.text_.type_ = colman::IdCardElementType::Text;
    lay.logo_ = read_id_card_element(collection, "item_card_lg");
    lay.logo_.type_ = colman::IdCardElementType::Logo;
    col.set_item_card_layout(lay);
  }
  // Signature label layout
  {
    colman::SignatureLabelLayout sll;
    double d = 0.0;
    int i = 0;
    set(d, collection, "sig_lbl_card_w");
    sll.card_width_mm_ = d;
    set(d, collection, "sig_lbl_card_h");
    sll.card_height_mm_ = d;
    set(d, collection, "sig_lbl_margin");
    sll.margin_mm_ = d;
    set(d, collection, "sig_lbl_tx");
    sll.text_x_mm_ = d;
    set(d, collection, "sig_lbl_ty");
    sll.text_y_mm_ = d;
    set(d, collection, "sig_lbl_tw");
    sll.text_width_mm_ = d;
    set(d, collection, "sig_lbl_th");
    sll.text_height_mm_ = d;
    set(i, collection, "sig_lbl_ori");
    sll.text_orientation_ = i;
    set(sll.label_layout_name_, collection, "sig_lbl_preset");
    col.set_sig_label_layout(sll);
  }
  collection_cache_ = col;
  collection_cache_valid_ = true;
  collection_cache_version_ = version;
  return col;
}

void colman::SQLQueries::invalidate_collection_cache()
{
  collection_cache_valid_ = false;
}

/**
 * Reads the name of the last-used ID-label sheet layout from the collection
 * table.  Returns an empty string if no value has been stored yet.  Kept
 * read-only so the desktop's one-time migration can seed the local
 * QSettings value; the layout is no longer written back to the database.
 * @return the stored layout name, or an empty string
 */
QString colman::SQLQueries::read_id_label_layout_name()
{
  std::map<QString, QVariant> const collection = query_collection(db_);
  auto const it = collection.find("id_label_layout_name");
  if(it == collection.end()) {
    return QString();
  }
  return it->second.toString();
}

/**
 * Reads the name of the last-used item-card label sheet from the collection
 * table.  Returns an empty string if no value has been stored yet.  Kept
 * read-only for the desktop's one-time migration; no longer written.
 * @return the stored layout name, or an empty string
 */
QString colman::SQLQueries::read_item_card_label_layout_name()
{
  std::map<QString, QVariant> const collection = query_collection(db_);
  auto const it = collection.find("item_card_label_layout_name");
  if(it == collection.end()) {
    return QString();
  }
  return it->second.toString();
}

// -----------------------------------------------------------------------
// Encryption
// -----------------------------------------------------------------------

/**
 * Returns true if the connection uses the cipher-capable SQLite driver.
 * When the SQLITE_MULTIPLE_CIPHERS plugin is missing, the connection
 * falls back to plain QSQLITE and encryption is unavailable.
 */
bool colman::SQLQueries::supports_encryption() const
{
  return db_.driverName() == CipherDriver;
}

bool colman::SQLQueries::is_encrypted() const
{
  return !encryption_key_.isEmpty();
}

/**
 * Encrypts an unencrypted open database by re-keying it with \a new_key.
 * The connection is briefly closed and reopened with the QSQLITE_MC_UPDATE_KEY
 * option, which calls sqlite3_rekey() on the database file.
 * @param new_key  encryption passphrase; must not be empty
 * @return true on success; false if the operation fails (connection is
 *         restored to its previous state on failure)
 */
bool colman::SQLQueries::encrypt_database(QString const& new_key)
{
  if(new_key.isEmpty() || !supports_encryption()) {
    return false;
  }
  db_.close();
  db_.setConnectOptions("QSQLITE_MC_UPDATE_KEY=" + new_key);
  if(!db_.open()) {
    db_.setConnectOptions({});
    db_.open();
    qWarning() << "encrypt_database failed:" << db_.lastError();
    return false;
  }
  db_.setConnectOptions({});
  encryption_key_ = new_key;
  return true;
}

/**
 * Changes the encryption key from \a old_key to \a new_key.
 * Closes and reopens the connection with QSQLITE_MC_KEY (to unlock the
 * existing cipher) and QSQLITE_MC_UPDATE_KEY (to re-encrypt with the new key).
 * @param old_key  current passphrase
 * @param new_key  replacement passphrase; must not be empty
 * @return true on success
 */
bool colman::SQLQueries::change_encryption_key(QString const& old_key,
                                               QString const& new_key)
{
  if(old_key.isEmpty() || new_key.isEmpty() || !supports_encryption()) {
    return false;
  }
  QString const opts =
      "QSQLITE_MC_KEY=" + old_key + ";QSQLITE_MC_UPDATE_KEY=" + new_key;
  db_.close();
  db_.setConnectOptions(opts);
  if(!db_.open()) {
    db_.setConnectOptions("QSQLITE_MC_KEY=" + old_key);
    db_.open();
    db_.setConnectOptions({});
    qWarning() << "change_encryption_key failed:" << db_.lastError();
    return false;
  }
  db_.setConnectOptions({});
  encryption_key_ = new_key;
  return true;
}

/**
 * Removes encryption from the database.
 * Closes and reopens the connection with QSQLITE_MC_REMOVE_KEY which calls
 * sqlite3_key() (to unlock) followed by sqlite3_rekey(NULL,0) (to strip the
 * cipher from the database file).
 * @param current_key  current passphrase
 * @return true on success
 */
bool colman::SQLQueries::remove_encryption(QString const& current_key)
{
  if(current_key.isEmpty() || !supports_encryption()) {
    return false;
  }
  db_.close();
  db_.setConnectOptions("QSQLITE_MC_REMOVE_KEY=" + current_key);
  if(!db_.open()) {
    db_.setConnectOptions("QSQLITE_MC_KEY=" + current_key);
    db_.open();
    db_.setConnectOptions({});
    qWarning() << "remove_encryption failed:" << db_.lastError();
    return false;
  }
  db_.setConnectOptions({});
  encryption_key_.clear();
  return true;
}

/* EOF */
