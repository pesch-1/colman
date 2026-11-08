// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// colman_transfer: command-line import/export tool for persons and items.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_json.hpp"
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#  define COLMAN_ITEM_HPP
#endif
#ifndef COLMAN_PERSON_HPP
#  include "colman_person.hpp"
#  define COLMAN_PERSON_HPP
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#  define COLMAN_SQL_QUERIES_HPP
#endif
#ifndef COLMAN_CONSOLE_HPP
#  include "colman_console.hpp"
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
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
#ifndef SYSTEM_QSQLDATABASE
#  include <QSqlDatabase>
#  define SYSTEM_QSQLDATABASE
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSTRINGLIST
#  include <QStringList>
#  define SYSTEM_QSTRINGLIST
#endif
#ifndef SYSTEM_QTEXTSTREAM
#  include <QTextStream>
#  define SYSTEM_QTEXTSTREAM
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif

namespace
{

  // -------------------------------------------------------------------------
  // CSV utilities
  // -------------------------------------------------------------------------

  /// Returns \a field quoted per RFC 4180 (wraps with double-quotes if needed).
  QString csv_quote(QString const& field)
  {
    bool const needs_quoting = field.contains(',') || field.contains('"') ||
                               field.contains('\n') || field.contains('\r');
    if(!needs_quoting) {
      return field;
    }
    QString result;
    result.reserve(field.size() + 2);
    result.append('"');
    for(QChar const c : field) {
      if(c == '"') {
        result.append("\"\"");
      }
      else {
        result.append(c);
      }
    }
    result.append('"');
    return result;
  }

  /// Builds one RFC 4180 CSV record string (no trailing newline).
  QString csv_row(QStringList const& fields)
  {
    QStringList quoted;
    quoted.reserve(fields.size());
    for(auto const& f : fields) {
      quoted.append(csv_quote(f));
    }
    return quoted.join(',');
  }

  /// Parses \a text into a vector of CSV rows.
  /**
     Handles quoted fields, escaped double-quotes (""), and CRLF/LF endings.
  */
  std::vector<QStringList> parse_csv(QString const& text)
  {
    std::vector<QStringList> result;
    QStringList current_row;
    QString current_field;
    enum struct State { Normal, Quoted, QuoteEnd };
    auto state = State::Normal;
    bool row_started = false;

    for(int i = 0; i < text.size(); ++i) {
      QChar const c = text[i];
      if(state == State::Normal) {
        if(c == '"') {
          state = State::Quoted;
          row_started = true;
        }
        else if(c == ',') {
          current_row.append(current_field);
          current_field.clear();
          row_started = true;
        }
        else if(c == '\n') {
          if(row_started || !current_row.isEmpty()) {
            current_row.append(current_field);
            result.push_back(current_row);
          }
          current_row.clear();
          current_field.clear();
          row_started = false;
        }
        else if(c != '\r') {
          current_field.append(c);
          row_started = true;
        }
      }
      else if(state == State::Quoted) {
        if(c == '"') {
          state = State::QuoteEnd;
        }
        else {
          current_field.append(c);
        }
      }
      else { // QuoteEnd
        if(c == '"') {
          current_field.append('"');
          state = State::Quoted;
        }
        else if(c == ',') {
          current_row.append(current_field);
          current_field.clear();
          state = State::Normal;
          row_started = true;
        }
        else if(c == '\n') {
          current_row.append(current_field);
          result.push_back(current_row);
          current_row.clear();
          current_field.clear();
          state = State::Normal;
          row_started = false;
        }
        else if(c != '\r') {
          current_field.append(c);
          state = State::Normal;
        }
      }
    }
    if(row_started || !current_row.isEmpty()) {
      current_row.append(current_field);
      result.push_back(current_row);
    }
    return result;
  }

  // -------------------------------------------------------------------------
  // Bulk clear helpers
  // -------------------------------------------------------------------------

  /// Executes \a sql; prints an error and returns false on failure.
  bool exec_sql(QSqlDatabase& db, QString const& sql)
  {
    QSqlQuery q(db);
    if(!q.exec(sql)) {
      QTextStream err(stderr);
      err << "SQL error: " << q.lastError().text() << "\n"
          << "  Query: " << sql << "\n";
      return false;
    }
    return true;
  }

  /// Deletes all persons (id > 0) plus their credentials, permissions,
  /// lendings, returns, and reservations in a single transaction.
  bool clear_persons(QSqlDatabase db)
  {
    QTextStream err(stderr);
    if(!db.transaction()) {
      err << "Could not start transaction.\n";
      return false;
    }
    QStringList const sqls = {
        "DELETE FROM returns WHERE lending IN "
        "(SELECT id FROM lendings "
        " WHERE person IN (SELECT id FROM persons WHERE id > 0))",
        "DELETE FROM reservations "
        "WHERE person_id IN (SELECT id FROM persons WHERE id > 0)",
        "DELETE FROM lendings "
        "WHERE person IN (SELECT id FROM persons WHERE id > 0)",
        "DELETE FROM credentials "
        "WHERE person_id IN (SELECT id FROM persons WHERE id > 0)",
        "DELETE FROM permissions "
        "WHERE person_id IN (SELECT id FROM persons WHERE id > 0)",
        "DELETE FROM persons WHERE id > 0"};
    for(auto const& sql : sqls) {
      if(!exec_sql(db, sql)) {
        db.rollback();
        return false;
      }
    }
    if(!db.commit()) {
      err << "Could not commit transaction.\n";
      db.rollback();
      return false;
    }
    return true;
  }

  /// Deletes all items and their lendings, returns, reservations,
  /// acquisitions, liquidations, and type-specific rows in a single
  /// transaction.
  bool clear_items(QSqlDatabase db)
  {
    QTextStream err(stderr);
    // Collect type-specific table names before starting the transaction.
    QStringList type_tables;
    {
      QSqlQuery q(db);
      if(!q.exec("SELECT tablename FROM types")) {
        err << "Could not query type table names: "
            << q.lastError().text() << "\n";
        return false;
      }
      while(q.next()) {
        type_tables.append(q.value(0).toString());
      }
    }
    if(!db.transaction()) {
      err << "Could not start transaction.\n";
      return false;
    }
    QStringList sqls = {"DELETE FROM returns",
                        "DELETE FROM reservations",
                        "DELETE FROM lendings",
                        "DELETE FROM acquisitions",
                        "DELETE FROM liquidations",
                        "DELETE FROM schoolitems",
                        "DELETE FROM item_list_values"};
    for(auto const& table : type_tables) {
      sqls.append("DELETE FROM \"" + table + "\"");
    }
    sqls.append("DELETE FROM items");
    for(auto const& sql : sqls) {
      if(!exec_sql(db, sql)) {
        db.rollback();
        return false;
      }
    }
    // items_fts is a FTS5 virtual table that may be unavailable — non-fatal.
    QSqlQuery q_fts(db);
    q_fts.exec("DELETE FROM items_fts");
    if(!db.commit()) {
      err << "Could not commit transaction.\n";
      db.rollback();
      return false;
    }
    return true;
  }

  // -------------------------------------------------------------------------
  // Export
  // -------------------------------------------------------------------------

  /// Exports all persons (id > 0) to a RFC 4180 CSV file at \a path.
  bool export_persons(QString const& path)
  {
    auto db = QSqlDatabase::database();
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream err(stderr);
      err << "Cannot open output file: " << path << "\n";
      return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << csv_row({"id", "lastname", "firstname", "title", "school_class",
                    "emails", "telephone", "street", "zip", "city", "country",
                    "remarks"})
        << "\n";

    QSqlQuery q(db);
    bool const ok =
        q.exec("SELECT id, lastname, firstname, title, school_class, "
               "emails, telephonenumbers, street, zip, city, country, "
               "remarks "
               "FROM persons WHERE id > 0 "
               "ORDER BY lastname, firstname");
    if(!ok) {
      QTextStream err(stderr);
      err << "Query failed: " << q.lastError().text() << "\n";
      return false;
    }
    int count = 0;
    while(q.next()) {
      QStringList fields;
      for(int c = 0; c < 12; ++c) {
        fields.append(q.value(c).toString());
      }
      out << csv_row(fields) << "\n";
      ++count;
    }
    QTextStream info(stdout);
    info << "Exported " << count << " persons.\n";
    return true;
  }

  /// Exports all items to a JSON file at \a path.
  bool export_items(colman::SQLQueries& sq, QString const& path)
  {
    auto db = QSqlDatabase::database();
    struct ItemRow_
    {
      int id = 0;
      int count = 0;
      int available = 0;
    };
    std::vector<ItemRow_> rows;
    {
      QSqlQuery q(db);
      if(!q.exec("SELECT id, count, available FROM items ORDER BY id")) {
        QTextStream err(stderr);
        err << "Query items failed: " << q.lastError().text() << "\n";
        return false;
      }
      while(q.next()) {
        ItemRow_ r;
        r.id = q.value(0).toInt();
        r.count = q.value(1).toInt();
        r.available = q.value(2).toInt();
        rows.push_back(r);
      }
    }

    QJsonArray array;
    QTextStream err(stderr);
    for(auto const& row : rows) {
      colman::Item item;
      QString type_name;
      if(!sq.get_item(row.id, item, type_name)) {
        err << "Cannot read item id=" << row.id << " — skipped.\n";
        continue;
      }
      auto obj = colman::to_json(item);
      obj.insert("id", row.id);
      obj.insert("type_name", type_name);
      obj.insert("count", row.count);
      obj.insert("available", row.available);
      array.append(obj);
    }

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly)) {
      err << "Cannot open output file: " << path << "\n";
      return false;
    }
    file.write(QJsonDocument(array).toJson());

    QTextStream info(stdout);
    info << "Exported " << array.size() << " items.\n";
    return true;
  }

  // -------------------------------------------------------------------------
  // Import
  // -------------------------------------------------------------------------

  /// Returns the value at \a col in \a row, or an empty string.
  QString csv_field(QStringList const& row, int col)
  {
    if(col < 0 || col >= row.size()) {
      return {};
    }
    return row[col];
  }

  /// Holds the column index for each expected person CSV field.
  struct PersonCols_
  {
    int id = -1;
    int lastname = -1;
    int firstname = -1;
    int title = -1;
    int school_class = -1;
    int emails = -1;
    int telephone = -1;
    int street = -1;
    int zip = -1;
    int city = -1;
    int country = -1;
    int remarks = -1;
  };

  PersonCols_ parse_person_header(QStringList const& hdr)
  {
    PersonCols_ c;
    for(int i = 0; i < hdr.size(); ++i) {
      auto const key = hdr[i].toLower().trimmed();
      if(key == "id") {
        c.id = i;
      }
      else if(key == "lastname") {
        c.lastname = i;
      }
      else if(key == "firstname") {
        c.firstname = i;
      }
      else if(key == "title") {
        c.title = i;
      }
      else if(key == "school_class") {
        c.school_class = i;
      }
      else if(key == "emails") {
        c.emails = i;
      }
      else if(key == "telephone") {
        c.telephone = i;
      }
      else if(key == "street") {
        c.street = i;
      }
      else if(key == "zip") {
        c.zip = i;
      }
      else if(key == "city") {
        c.city = i;
      }
      else if(key == "country") {
        c.country = i;
      }
      else if(key == "remarks") {
        c.remarks = i;
      }
    }
    return c;
  }

  /// Imports persons from the RFC 4180 CSV file at \a path.
  bool import_persons(colman::SQLQueries& sq, QString const& path)
  {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream err(stderr);
      err << "Cannot open input file: " << path << "\n";
      return false;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    auto const rows = parse_csv(in.readAll());
    if(rows.empty()) {
      QTextStream err(stderr);
      err << "CSV file is empty.\n";
      return false;
    }

    auto const cols = parse_person_header(rows[0]);
    if(cols.lastname < 0) {
      QTextStream err(stderr);
      err << "CSV header must contain at least 'lastname'.\n";
      return false;
    }

    auto db = QSqlDatabase::database();
    int added = 0;
    int updated = 0;
    int errors = 0;

    for(std::size_t r = 1; r < rows.size(); ++r) {
      auto const& row = rows[r];
      if(row.size() == 1 && row[0].trimmed().isEmpty()) {
        continue;
      }
      colman::Person p;
      p.set_lastname(csv_field(row, cols.lastname));
      p.set_firstname(csv_field(row, cols.firstname));
      p.set_title(csv_field(row, cols.title));
      p.set_school_class(csv_field(row, cols.school_class));
      p.set_emails(csv_field(row, cols.emails));
      p.set_telephone(csv_field(row, cols.telephone));
      p.set_street(csv_field(row, cols.street));
      p.set_zip(csv_field(row, cols.zip));
      p.set_city(csv_field(row, cols.city));
      p.set_country(csv_field(row, cols.country));
      p.set_remarks(csv_field(row, cols.remarks));
      p.set_thumbnail_id(-1);
      p.set_data_collector_id(-1);

      bool ok = false;
      int const id = csv_field(row, cols.id).toInt(&ok);
      if(ok && id > 0) {
        QSqlQuery chk(db);
        chk.prepare("SELECT 1 FROM persons WHERE id = :id");
        chk.bindValue(":id", id);
        if(chk.exec() && chk.next()) {
          if(sq.change_person(id, p)) {
            ++updated;
          }
          else {
            QTextStream err(stderr);
            err << "Row " << r + 1 << ": cannot update person id="
                << id << ".\n";
            ++errors;
          }
          continue;
        }
      }
      if(sq.add_new_person(p) >= 0) {
        ++added;
      }
      else {
        QTextStream err(stderr);
        err << "Row " << r + 1 << ": cannot insert person.\n";
        ++errors;
      }
    }

    QSet<QString> classes;
    for(std::size_t r = 1; r < rows.size(); ++r) {
      auto const& row = rows[r];
      auto const cls = csv_field(row, cols.school_class).trimmed();
      if(!cls.isEmpty()) {
        classes.insert(cls);
      }
    }
    for(auto const& cls : classes) {
      sq.add_class_name(cls);
    }

    QTextStream info(stdout);
    info << "Persons: " << added << " added, " << updated << " updated, "
         << errors << " errors.\n";
    return errors == 0;
  }

  /// Imports items from the JSON file at \a path.
  bool import_items(colman::SQLQueries& sq, QString const& path)
  {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
      QTextStream err(stderr);
      err << "Cannot open input file: " << path << "\n";
      return false;
    }
    QJsonParseError pe;
    auto const doc = QJsonDocument::fromJson(file.readAll(), &pe);
    if(doc.isNull()) {
      QTextStream err(stderr);
      err << "JSON parse error: " << pe.errorString() << "\n";
      return false;
    }
    if(!doc.isArray()) {
      QTextStream err(stderr);
      err << "JSON root must be an array.\n";
      return false;
    }

    auto db = QSqlDatabase::database();
    QJsonArray const array = doc.array();
    int added = 0;
    int updated = 0;
    int errors = 0;

    for(auto const& elem : array) {
      if(!elem.isObject()) {
        ++errors;
        continue;
      }
      QJsonObject obj = elem.toObject();

      // Resolve type: prefer type_name for portability across databases.
      QString const type_name = obj.value("type_name").toString();
      int type_id = -1;
      if(!type_name.isEmpty()) {
        type_id = sq.get_type_id(type_name);
      }
      if(type_id < 0) {
        auto const v = obj.value("type_id");
        if(v.isDouble()) {
          type_id = v.toInt();
        }
      }
      if(type_id < 0) {
        QTextStream err(stderr);
        err << "Cannot resolve type '" << type_name
            << "' — item skipped.\n";
        ++errors;
        continue;
      }
      obj["type_id"] = type_id;

      colman::Item item;
      if(!colman::from_json(obj, item)) {
        QTextStream err(stderr);
        err << "Cannot parse item JSON — skipped.\n";
        ++errors;
        continue;
      }

      int const id = obj.value("id").toInt(-1);
      if(id > 0) {
        QSqlQuery chk(db);
        chk.prepare("SELECT 1 FROM items WHERE id = :id");
        chk.bindValue(":id", id);
        if(chk.exec() && chk.next()) {
          if(sq.change_item(id, item)) {
            ++updated;
          }
          else {
            QTextStream err(stderr);
            err << "Cannot update item id=" << id << ".\n";
            ++errors;
          }
          continue;
        }
      }

      int const new_id = sq.add_new_item(item);
      if(new_id < 0) {
        QTextStream err(stderr);
        err << "Cannot insert item — skipped.\n";
        ++errors;
        continue;
      }
      ++added;

      // add_new_item inserts with count=0; set the exported stock directly.
      int const count = obj.value("count").toInt(0);
      if(count > 0) {
        int const avail = qMin(obj.value("available").toInt(0), count);
        QSqlQuery upd(db);
        upd.prepare("UPDATE items SET count = :c, available = :a "
                    "WHERE id = :id");
        upd.bindValue(":c", count);
        upd.bindValue(":a", avail);
        upd.bindValue(":id", new_id);
        if(!upd.exec()) {
          QTextStream err(stderr);
          err << "Cannot set stock for item id=" << new_id << ": "
              << upd.lastError().text() << "\n";
        }
      }
    }

    QTextStream info(stdout);
    info << "Items: " << added << " added, " << updated << " updated, "
         << errors << " errors.\n";
    return errors == 0;
  }

} // namespace

/// Entry point.
/**
   Parses arguments, opens the database, and dispatches to the
   appropriate export/import function.
*/
int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("colman_transfer");
  QCoreApplication::setApplicationVersion(COLMAN_VERSION);

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Import/export persons (CSV) or items (JSON) from/to a colman "
      "database.");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption const opt_users({"u", "users"},
                                     "Operate on persons/users.");
  QCommandLineOption const opt_items({"i", "items"},
                                     "Operate on items/media.");
  QCommandLineOption const opt_export({"e", "export"},
                                      "Export data to file.");
  QCommandLineOption const opt_import({"m", "import"},
                                      "Import data from file.");
  QCommandLineOption const opt_clear(
      {"c", "clear"},
      "Delete all existing records of the selected type before import.");
  QCommandLineOption const opt_password(
      {"p", "password"}, "Database encryption password.", "password");

  parser.addOption(opt_users);
  parser.addOption(opt_items);
  parser.addOption(opt_export);
  parser.addOption(opt_import);
  parser.addOption(opt_clear);
  parser.addOption(opt_password);
  parser.addPositionalArgument("database.col",
                               "Path to the colman database file.");
  parser.addPositionalArgument("data-file",
                               "CSV file for persons; JSON file for items.");
  parser.process(app);

  QTextStream err(stderr);
  bool const do_users = parser.isSet(opt_users);
  bool const do_items = parser.isSet(opt_items);
  bool const do_export = parser.isSet(opt_export);
  bool const do_import = parser.isSet(opt_import);
  bool const do_clear = parser.isSet(opt_clear);

  if(do_users == do_items) {
    err << "Error: specify exactly one of --users or --items.\n\n";
    parser.showHelp(1);
  }
  if(do_export == do_import) {
    err << "Error: specify exactly one of --export or --import.\n\n";
    parser.showHelp(1);
  }
  if(do_clear && do_export) {
    err << "Error: --clear is only meaningful with --import.\n\n";
    parser.showHelp(1);
  }

  QStringList const pos_args = parser.positionalArguments();
  if(pos_args.size() != 2) {
    err << "Error: two positional arguments required "
           "(database.col and data-file).\n\n";
    parser.showHelp(1);
  }
  QString const db_path = pos_args[0];
  QString const data_path = pos_args[1];

  // Open database.
  colman::SQLQueries sq(colman::SQLQueries::FileDb{});
  bool opened = false;
  QString key;

  if(parser.isSet(opt_password)) {
    key = parser.value(opt_password);
    opened = sq.load_database(db_path, key);
  }
  else {
    // Try without password first; if that fails, prompt interactively.
    opened = sq.load_database(db_path);
    if(!opened) {
      key = colman::read_password("Password: ").value_or(QString());
      opened = sq.load_database(db_path, key);
    }
  }
  if(!opened) {
    err << "Error: cannot open '" << db_path << "'.\n";
    return 1;
  }

  auto db = QSqlDatabase::database();

  if(do_clear) {
    QTextStream info(stdout);
    if(do_users) {
      info << "Clearing all persons...\n";
      if(!clear_persons(db)) {
        return 1;
      }
      info << "Cleared.\n";
    }
    else {
      info << "Clearing all items...\n";
      if(!clear_items(db)) {
        return 1;
      }
      info << "Cleared.\n";
    }
  }

  if(do_export) {
    if(do_users) {
      return export_persons(data_path) ? 0 : 1;
    }
    return export_items(sq, data_path) ? 0 : 1;
  }
  if(do_users) {
    return import_persons(sq, data_path) ? 0 : 1;
  }
  return import_items(sq, data_path) ? 0 : 1;
}

/* EOF */
