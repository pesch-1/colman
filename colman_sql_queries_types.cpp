// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries type-management domain implementation.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sql_queries_types.hpp"
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
#ifndef SYSTEM_QDATASTREAM
#  include <QDataStream>
#  define SYSTEM_QDATASTREAM
#endif
#ifndef SYSTEM_QIODEVICE
#  include <QIODevice>
#  define SYSTEM_QIODEVICE
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_MAP
#  include <map>
#  define SYSTEM_MAP
#endif
#ifndef SYSTEM_SET
#  include <set>
#  define SYSTEM_SET
#endif

namespace
{
  /// Display names of a column of a predefined media type.
  struct PredefinedName
  {
    char const* table_;   ///< "types" or the type's field table
    char const* column_;  ///< type name or field column
    char const* german_;  ///< German display name
    char const* english_; ///< English display name
  };

  /// Display names of the predefined media types and their fields.
  /**
     Stored in the translations table by ensure_predefined_translations()
     for German and English, so that a collection shows readable names in
     either user interface language, regardless of the language it was
     created in.
  */
  PredefinedName const PredefinedNames[] = {
      {"types", "book", "Buch", "Book"},
      {"types", "periodical", "Zeitschrift", "Periodical"},
      {"types", "cd", "CD", "CD"},
      {"types", "dvd", "DVD", "DVD"},
      {"types", "bluray", "Blu-ray", "Blu-ray"},
      {"types", "board_game", "Gesellschaftsspiel", "Board game"},
      {"books", "authors", "Autoren", "Authors"},
      {"books", "edition", "Ausgabe", "Edition"},
      {"books", "publication_date", "Erscheinungsdatum", "Publication date"},
      {"books", "publisher", "Verlag", "Publisher"},
      {"books", "pages", "Seiten", "Pages"},
      {"books", "ISBN10", "ISBN-10", "ISBN-10"},
      {"books", "ISBN13", "ISBN-13", "ISBN-13"},
      {"books", "ddc", "DDC", "DDC"},
      {"periodicals", "issn", "ISSN", "ISSN"},
      {"periodicals", "publisher", "Verlag", "Publisher"},
      {"periodicals", "publication_date", "Erscheinungsjahr",
       "Year of publication"},
      {"periodicals", "extent", "Umfang", "Extent"},
      {"periodicals", "note", "Anmerkung", "Note"},
      {"cds", "artist", "Interpret", "Artist"},
      {"cds", "publisher_number", "Verlagsnummer", "Publisher number"},
      {"cds", "publisher", "Verlag", "Publisher"},
      {"cds", "publication_date", "Erscheinungsjahr", "Year of publication"},
      {"cds", "language", "Sprache", "Language"},
      {"cds", "extent", "Umfang", "Extent"},
      {"cds", "content_type", "Inhaltstyp", "Content type"},
      {"cds", "media_type", "Medientyp", "Media type"},
      {"cds", "carrier_type", "Datenträgertyp", "Carrier type"},
      {"cds", "ean", "EAN", "EAN"},
      {"dvds", "director", "Regie/Darsteller", "Director/cast"},
      {"dvds", "publication_date", "Erscheinungsjahr", "Year of publication"},
      {"dvds", "language", "Sprache", "Language"},
      {"dvds", "extent", "Umfang", "Extent"},
      {"dvds", "content_type", "Inhaltstyp", "Content type"},
      {"dvds", "media_type", "Medientyp", "Media type"},
      {"dvds", "carrier_type", "Datenträgertyp", "Carrier type"},
      {"dvds", "participants", "Mitwirkende", "Participants"},
      {"dvds", "target_audience", "Zielgruppe", "Target audience"},
      {"dvds", "publisher", "Verlag", "Publisher"},
      {"dvds", "ean", "EAN", "EAN"},
      {"blurays", "director", "Regie/Darsteller", "Director/cast"},
      {"blurays", "publication_date", "Erscheinungsjahr",
       "Year of publication"},
      {"blurays", "language", "Sprache", "Language"},
      {"blurays", "extent", "Umfang", "Extent"},
      {"blurays", "content_type", "Inhaltstyp", "Content type"},
      {"blurays", "media_type", "Medientyp", "Media type"},
      {"blurays", "carrier_type", "Datenträgertyp", "Carrier type"},
      {"blurays", "participants", "Mitwirkende", "Participants"},
      {"blurays", "target_audience", "Zielgruppe", "Target audience"},
      {"blurays", "publisher", "Verlag", "Publisher"},
      {"blurays", "ean", "EAN", "EAN"},
      {"board_games", "designer", "Spieldesigner", "Designer"},
      {"board_games", "artist", "Illustrator", "Illustrator"},
      {"board_games", "publisher", "Verlag", "Publisher"},
      {"board_games", "publication_date", "Erscheinungsjahr",
       "Year of publication"},
      {"board_games", "min_players", "Min. Spieler", "Min. players"},
      {"board_games", "max_players", "Max. Spieler", "Max. players"},
      {"board_games", "min_age", "Mindestalter", "Minimum age"},
      {"board_games", "playing_time", "Spielzeit (Min.)",
       "Playing time (min.)"},
      {"board_games", "language", "Sprache", "Language"},
      {"board_games", "genre", "Genre", "Genre"},
      {"board_games", "ean", "EAN", "EAN"}};

  /// Canonical field display order for the built-in "book" type.
  QStringList default_book_field_order()
  {
    return {"__name__", "authors", "__thumbnail__",
            "__sig_top__", "__sig_bottom__", "__description__",
            "__keywords__", "__school_item__", "edition",
            "publication_date", "publisher", "pages",
            "ISBN10", "ISBN13", "ddc",
            "__place__"};
  }

  QStringList default_periodical_field_order()
  {
    return {"__name__", "__thumbnail__",
            "__sig_top__", "__sig_bottom__",
            "__description__", "__keywords__",
            "__school_item__", "issn",
            "publisher", "publication_date",
            "extent", "note",
            "__place__"};
  }

  QStringList default_cd_field_order()
  {
    return {"__name__",
            "artist",
            "__thumbnail__",
            "__sig_top__",
            "__sig_bottom__",
            "__description__",
            "__keywords__",
            "__school_item__",
            "publisher_number",
            "publisher",
            "publication_date",
            "language",
            "extent",
            "content_type",
            "media_type",
            "carrier_type",
            "ean",
            "__place__"};
  }

  QStringList default_dvd_field_order()
  {
    return {"__name__",
            "director",
            "__thumbnail__",
            "__sig_top__",
            "__sig_bottom__",
            "__description__",
            "__keywords__",
            "__school_item__",
            "publisher",
            "publication_date",
            "language",
            "extent",
            "content_type",
            "media_type",
            "carrier_type",
            "participants",
            "target_audience",
            "ean",
            "__place__"};
  }

  QStringList default_board_game_field_order()
  {
    return {"__name__",
            "designer",
            "__thumbnail__",
            "__sig_top__",
            "__sig_bottom__",
            "__description__",
            "__keywords__",
            "__school_item__",
            "artist",
            "publisher",
            "publication_date",
            "min_players",
            "max_players",
            "min_age",
            "playing_time",
            "language",
            "genre",
            "ean",
            "__place__"};
  }

  /**
   * Deserialises a QVariantMap that was previously serialised with
   * serialize_map().  The raw bytes in \a variant are expected to be
   * base64-encoded QDataStream output.
   * @param variant base64-encoded blob retrieved from the database
   * @return the decoded QVariantMap
   */
  QVariantMap deserialize_map(QVariant const& variant)
  {
    QByteArray arr = variant.toByteArray();
    arr = QByteArray::fromBase64(arr);
    QDataStream ds(&arr, QIODevice::ReadOnly);
    QVariant v;
    ds >> v;
    return v.toMap();
  }

  /**
   * Serialises \a varmap to a base64-encoded byte array suitable for
   * storage in a SQLite BLOB column.  Use deserialize_map() to reverse
   * the process.
   * @param varmap the map to serialise
   * @return base64-encoded byte array
   */
  QByteArray serialize_map(QVariantMap const& varmap)
  {
    QVariant infovar = QVariant::fromValue(varmap);
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << infovar;
    data = data.toBase64();
    return data;
  }

  /**
   * Computes the set difference and intersection between two key vectors.
   * Used by change_type() to determine which columns to drop, keep, and
   * add when a type definition is modified.
   * @param old_kv      the previous set of key/type pairs
   * @param new_kv      the new set of key/type pairs
   * @param old_keys    output: keys present in old_kv but not in new_kv
   * @param common_keys output: keys present in both old_kv and new_kv
   * @param new_keys    output: keys present in new_kv but not in old_kv
   */
  void key_vector_diff(colman::KeyVector const& old_kv,
                       colman::KeyVector const& new_kv,
                       std::vector<QString>& old_keys,
                       std::vector<QString>& common_keys,
                       colman::KeyVector& new_keys)
  {
    std::set<QString> keys;
    for(auto const& kv : old_kv) {
      keys.insert(kv.first);
    }
    for(auto const& kv : new_kv) {
      if(keys.count(kv.first)) {
        common_keys.push_back(kv.first);
        keys.erase(kv.first);
      }
      else {
        new_keys.push_back(kv);
      }
    }
    for(auto const& key : keys) {
      old_keys.push_back(key);
    }
  }

  /// Default positions for common fields when none are configured.
  /// Negative values place them before type-specific fields (which start at 0).
  QMap<QString, int> default_common_positions()
  {
    return {{"__name__", -80}, {"__barcode__", -75}, {"__thumbnail__", -70}, {"__school_item__", -60}, {"__place__", -50}, {"__sig_top__", -40}, {"__sig_bottom__", -30}, {"__description__", -20}, {"__keywords__", -10}};
  }

  /**
   * Builds a DataTypeInfo with the given base type and optional range
   * constraints.
   * @param type        the data type (e.g. ShortString, Integer, Date)
   * @param with_minimum whether a lower bound constraint is active
   * @param minimum     the lower bound value (ignored when with_minimum is
   * false)
   * @param with_maximum whether an upper bound constraint is active
   * @param maximum     the upper bound value (ignored when with_maximum is
   * false)
   * @return a fully initialised DataTypeInfo ready for insertion
   */
  colman::DataTypeInfo make_type(colman::DataType type, bool with_minimum,
                                 QVariant const& minimum, bool with_maximum,
                                 QVariant const& maximum,
                                 bool is_repeatable = false,
                                 QString const& marc21_tag = {})
  {
    colman::DataTypeInfo retval;
    retval.type_ = type;
    retval.info_["with_minimum"] = QVariant(with_minimum);
    retval.info_["minimum"] = minimum;
    retval.info_["with_maximum"] = QVariant(with_maximum);
    retval.info_["maximum"] = maximum;
    retval.info_["is_repeatable"] = QVariant(is_repeatable);
    retval.info_["marc21_tag"] = marc21_tag;
    return retval;
  }

} // namespace

/**
  Selects type id, tablename and display name from the `types` table.
  If a translation for the system locale exists in the database the
  translated name is returned; otherwise the raw name column is used.
  Optionally filters to a single type by id.

  @param type_id  If greater than zero, restricts the query to this type id.
  @return         Vector of Type objects ordered by display name.
*/
std::vector<colman::Type> colman::SQLQueries::query_types(int type_id,
                                                          QLocale::Language lang)
{
  if(lang == QLocale::AnyLanguage) {
    lang = system_language();
  }
  QString sql;
  if(locale_supported(db_, lang)) {
    sql = "SELECT types.id, types.tablename, "
          "COALESCE(translations.name, types.name), types.media_kind "
          "FROM types LEFT JOIN translations "
          "ON translations.tablename = 'types' "
          "AND translations.column = types.name "
          "AND translations.language = " +
          QString().setNum(lang) + " ";
    if(type_id > 0) {
      sql += "WHERE types.id = " + QString().setNum(type_id) + " ";
    }
    sql += "ORDER BY COALESCE(translations.name, types.name)";
  }
  else {
    sql = "SELECT id, tablename, name, media_kind FROM types ";
    if(type_id > 0) {
      sql += " WHERE id = " + QString().setNum(type_id) + " ";
    }
    sql += "ORDER BY name";
  }
  QSqlQuery q(sql, db_);
  std::vector<Type> result;
  while(q.next()) {
    Type t;
    t.set_id(q.value(0).toInt());
    t.set_name(q.value(2).toString());
    t.set_media_kind(static_cast<MediaKind>(q.value(3).toInt()));
    result.push_back(std::move(t));
  }
  return result;
}

/// Returns field metadata for all columns of \a type_id.
std::vector<colman::IQueries::TypeFieldMeta>
colman::SQLQueries::get_type_field_metas(int type_id)
{
  QSqlQuery q(db_);
  q.prepare("SELECT column, marc21_tag "
            "FROM data_types "
            "WHERE type_id = :type_id "
            "ORDER BY position, column");
  q.bindValue(":type_id", type_id);
  if(!q.exec()) {
    qWarning() << "get_type_field_metas failed:" << q.lastError();
    return {};
  }
  std::vector<TypeFieldMeta> result;
  while(q.next()) {
    TypeFieldMeta m;
    m.column = q.value(0).toString();
    m.marc21_tag = q.value(1).toString();
    result.push_back(std::move(m));
  }
  return result;
}

QStringList colman::SQLQueries::query_type_field_completions(
    int type_id, QString const& field)
{
  QString const tbl = get_tablename(type_id);
  if(tbl.isEmpty()) {
    return {};
  }
  return completer_values_from(db_, get_table_columns(tbl, 0), tbl, field);
}

/**
 * Inserts a new item type into the database, creating the corresponding
 * type-specific table (e.g. "books"), populating the translations and
 * data_types tables, and registering the type in the types table.
 * If a type with the same name already exists the call fails.
 * @param db   the open database connection
 * @param type the type definition to insert
 * @return the new database id of the type, or -1 on failure
 */
int colman::SQLQueries::add_new_type(Type const& type,
                                     QString const& tablename)
{
  QString const tbl = tablename.isEmpty()
                          ? "_" + string_to_column_name(type.get_name())
                          : tablename;
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  // first check whether type already exists:
  query.prepare("SELECT id FROM types WHERE name = :name");
  query.bindValue(":name", type.get_name());
  if(!query.exec()) {
    qWarning() << "query type failed:\n"
               << query.lastError();
    return -1;
  }
  if(query.next()) {
    qWarning() << "type '" << type.get_name() << "' already exists\n";
    return -1;
  }
  // now create a new table for the new type:
  QString querytext("CREATE TABLE IF NOT EXISTS \"" + tbl + "\"");
  querytext += " (item_id INTEGER PRIMARY KEY";
  for(auto const& key : type.get_keys()) {
    querytext += ", \"" + string_to_column_name(key.first) + "\" VARCHAR ";
  }
  querytext += ")";
  query.prepare(querytext);
  if(!query.exec()) {
    qWarning() << "failed to create table: " << query.lastError()
               << "\nwith query '" << querytext << "'\n";
    return -1;
  }
  // add columns into translations table:
  QLocale::Language const sys_lang = system_language();
  // Use INSERT OR IGNORE so that existing translations are kept; built-in
  // types get their display names from ensure_predefined_translations().
  querytext = "INSERT OR IGNORE INTO translations "
              "(tablename, column, language, name) "
              "VALUES (:tablename, :column, :language, :name)";
  for(auto const& key : type.get_keys()) {
    QString const col = string_to_column_name(key.first);
    query.prepare(querytext);
    query.bindValue(":tablename", tbl);
    query.bindValue(":column", col);
    query.bindValue(":language", (int)QLocale::AnyLanguage);
    query.bindValue(":name", col);
    if(!query.exec()) {
      qWarning() << "failed to insert type into translations table:\n"
                 << query.lastError();
      return -1;
    }
    if(sys_lang != QLocale::AnyLanguage) {
      query.prepare(querytext);
      query.bindValue(":tablename", tbl);
      query.bindValue(":column", col);
      query.bindValue(":language", (int)sys_lang);
      query.bindValue(":name", key.first);
      if(!query.exec()) {
        qWarning() << "failed to insert type translation:\n"
                   << query.lastError();
        return -1;
      }
    }
  }
  query.prepare(querytext);
  query.bindValue(":tablename", QString("types"));
  query.bindValue(":column", type.get_name());
  query.bindValue(":language", (int)QLocale::AnyLanguage);
  query.bindValue(":name", type.get_name());
  if(!query.exec()) {
    qWarning() << "failed to insert type into types table:\n"
               << query.lastError();
    return -1;
  }
  if(sys_lang != QLocale::AnyLanguage) {
    query.prepare(querytext);
    query.bindValue(":tablename", QString("types"));
    query.bindValue(":column", type.get_name());
    query.bindValue(":language", (int)sys_lang);
    query.bindValue(":name", type.get_name());
    if(!query.exec()) {
      qWarning() << "failed to insert type name translation:\n"
                 << query.lastError();
      return -1;
    }
  }
  // insert type into types table:
  querytext = "INSERT INTO types (name, tablename, media_kind) VALUES "
              "(:name, :tablename, :media_kind)";
  query.prepare(querytext);
  query.bindValue(":name", type.get_name());
  query.bindValue(":tablename", tbl);
  query.bindValue(":media_kind", static_cast<int>(type.get_media_kind()));
  if(!query.exec()) {
    qWarning() << "failed to insert type into types table:\n"
               << query.lastError();
    return -1;
  }
  int const type_id = query.lastInsertId().toInt();
  // insert into data_types table:
  {
    int pos = 0;
    for(auto const& key : type.get_keys()) {
      querytext = "INSERT INTO data_types (type_id, column, "
                  "data_type, infos, position, marc21_tag) "
                  "VALUES (:type_id, :column, :data_type, :infos, "
                  ":position, :marc21_tag)";
      query.prepare(querytext);
      query.bindValue(":type_id", type_id);
      query.bindValue(":column", string_to_column_name(key.first));
      query.bindValue(":data_type", (int)key.second.type_);
      query.bindValue(":infos", serialize_map(key.second.info_));
      query.bindValue(":position", pos++);
      query.bindValue(":marc21_tag", key.second.info_["marc21_tag"].toString());
      if(!query.exec()) {
        qWarning() << "failed to insert type into data_types table:\n"
                   << query.lastError();
        return -1;
      }
    }
  }
  rollback.commit();
  backup_after_change();
  return type_id;
}

/**
 * Returns the database id of the type with the given internal \a type
 * name.
 * @param type the internal type name to look up
 * @return the type id, or -1 if the type is unknown
 */
int colman::SQLQueries::get_type_id(QString const& type)
{
  QSqlQuery query(db_);
  query.prepare("SELECT id FROM types WHERE name = :name");
  query.bindValue(":name", type);
  if(!query.exec()) {
    qWarning() << "Failed to query id for type '" << type
               << "': " << query.lastError();
  }
  else if(query.next()) {
    return query.value(0).toInt();
  }
  return -1;
}

/**
 * Returns the database id of the type with the given media \a kind.  When
 * several types share the kind the lowest id is returned, so that the
 * built-in types (created first) take precedence over user-defined ones.
 * @param kind the media kind to look up
 * @return the type id, or -1 if no type carries this media kind
 */
int colman::SQLQueries::type_id_for_media_kind(MediaKind kind)
{
  if(kind == MediaKind::Unknown) {
    return -1;
  }
  QSqlQuery query(db_);
  query.prepare("SELECT id FROM types WHERE media_kind = :kind "
                "ORDER BY id LIMIT 1");
  query.bindValue(":kind", static_cast<int>(kind));
  if(!query.exec()) {
    qWarning() << "Failed to query type for media kind "
               << static_cast<int>(kind) << ": " << query.lastError();
  }
  else if(query.next()) {
    return query.value(0).toInt();
  }
  return -1;
}

/**
 * Returns the media kind of the type with the given \a type_id.
 * @param type_id the database id of the type
 * @return the media kind, or MediaKind::Unknown if the type is unknown or
 *         has no media kind assigned
 */
colman::MediaKind colman::SQLQueries::media_kind_for_type_id(int type_id)
{
  QSqlQuery query(db_);
  query.prepare("SELECT media_kind FROM types WHERE id = :type_id");
  query.bindValue(":type_id", type_id);
  if(!query.exec()) {
    qWarning() << "Failed to query media kind for type " << type_id << ": "
               << query.lastError();
  }
  else if(query.next()) {
    return static_cast<MediaKind>(query.value(0).toInt());
  }
  return MediaKind::Unknown;
}

/**
 * Reads the full Type definition for \a type_id from the database,
 * including the type name, table name, and all key/DataTypeInfo pairs
 * stored in the data_types table.
 * @param type_id the database id of the type to load
 * @return the Type object, or a default-constructed Type if the id is
 *         unknown or invalid
 */
colman::Type colman::SQLQueries::get_type(int type_id)
{
  colman::Type retval;
  if(type_id > 0) {
    QSqlQuery query(db_);
    query.prepare("SELECT name, tablename, media_kind FROM types "
                  "WHERE id = :type_id");
    query.bindValue(":type_id", type_id);
    if(!query.exec()) {
      qWarning() << "Failed to query type " << type_id << ": "
                 << query.lastError();
    }
    if(!query.next()) {
      qWarning() << "Unknown type id " << type_id;
      return retval;
    }
    retval.set_id(type_id);
    retval.set_name(query.value(0).toString());
    retval.set_media_kind(
        static_cast<colman::MediaKind>(query.value(2).toInt()));
    query.finish();
    query.prepare("SELECT column, data_type, infos, marc21_tag "
                  "FROM data_types WHERE type_id = :type_id "
                  "ORDER BY position");
    query.bindValue(":type_id", type_id);
    if(!query.exec()) {
      qWarning() << "Failed to query data types for " << type_id << " ("
                 << retval.get_name() << "): " << query.lastError();
    }
    while(query.next()) {
      colman::KeyInfo keyinfo;
      keyinfo.first = query.value(0).toString();
      keyinfo.second.type_ = colman::DataType(query.value(1).toInt());
      keyinfo.second.info_ = deserialize_map(query.value(2));
      keyinfo.second.info_["marc21_tag"] = query.value(3).toString();
      retval.add_key(keyinfo);
    }
  }
  return retval;
}

/// Returns the ordered list of all fields (common + type-specific) for \a
/// type_id.
/**
  Common fields are interleaved at positions stored in common_field_positions;
  defaults are used when no custom order has been saved.
  @param type_id the database id of the item type
  @return field names in display order
*/
QStringList colman::SQLQueries::get_combined_field_order(int type_id)
{
  // Collect type-specific fields with their positions.
  QList<QPair<int, QString>> entries; // (position, column_name)
  {
    QSqlQuery query(db_);
    query.prepare("SELECT column, position FROM data_types "
                  "WHERE type_id = :type_id ORDER BY position");
    query.bindValue(":type_id", type_id);
    if(query.exec()) {
      while(query.next()) {
        entries.append({query.value(1).toInt(), query.value(0).toString()});
      }
    }
  }
  // Collect common field positions (from DB or defaults).
  QMap<QString, int> common_pos;
  {
    QSqlQuery query(db_);
    query.prepare("SELECT field, position FROM common_field_positions "
                  "WHERE type_id = :type_id");
    query.bindValue(":type_id", type_id);
    if(query.exec()) {
      while(query.next()) {
        common_pos[query.value(0).toString()] = query.value(1).toInt();
      }
    }
  }
  if(common_pos.isEmpty()) {
    common_pos = default_common_positions();
  }
  for(auto it = common_pos.begin(); it != common_pos.end(); ++it) {
    entries.append({it.value(), it.key()});
  }
  if(!common_pos.contains("__barcode__")) {
    entries.append({-75, "__barcode__"});
  }
  // Sort by position.
  std::sort(entries.begin(), entries.end(),
            [](QPair<int, QString> const& a, QPair<int, QString> const& b) {
              return a.first < b.first;
            });
  QStringList result;
  for(auto const& e : entries) {
    result.append(e.second);
  }
  return result;
}

/// Returns the default display order for common (non-type-specific) fields.
/**
  Uses the built-in default_common_positions() map; no database access.
  @return common field names sorted by their default position values
*/
QStringList colman::SQLQueries::get_default_common_field_order()
{
  QMap<QString, int> const pos = default_common_positions();
  QList<QPair<int, QString>> entries;
  for(auto it = pos.begin(); it != pos.end(); ++it) {
    entries.append({it.value(), it.key()});
  }
  std::sort(entries.begin(), entries.end(),
            [](QPair<int, QString> const& a, QPair<int, QString> const& b) {
              return a.first < b.first;
            });
  QStringList result;
  for(auto const& e : entries) {
    result.append(e.second);
  }
  return result;
}

/// Persists the field display order for a type.
/**
  Replaces the common_field_positions entries for \a type_id with the
  positions inferred from \a field_order, and updates the position column
  of data_types for type-specific fields.  All changes are wrapped in a
  single transaction.
  @param type_id    the database id of the item type
  @param field_order field names in the desired display order
  @return true on success, false on any database error
*/
bool colman::SQLQueries::save_field_order(int type_id,
                                          QStringList const& field_order)
{
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  // Update common field positions.
  query.prepare("DELETE FROM common_field_positions WHERE type_id = :type_id");
  query.bindValue(":type_id", type_id);
  if(!query.exec()) {
    qWarning() << "save_field_order: delete common_field_positions failed: "
               << query.lastError();
    return false;
  }
  int pos = 0;
  for(auto const& field : field_order) {
    if(field.startsWith("__")) {
      query.prepare("INSERT INTO common_field_positions "
                    "(type_id, field, position) VALUES (:t, :f, :p)");
      query.bindValue(":t", type_id);
      query.bindValue(":f", field);
      query.bindValue(":p", pos);
    }
    else {
      query.prepare("UPDATE data_types SET position = :p "
                    "WHERE type_id = :t AND column = :f");
      query.bindValue(":p", pos);
      query.bindValue(":t", type_id);
      query.bindValue(":f", field);
    }
    if(!query.exec()) {
      qWarning() << "save_field_order: update failed for '" << field
                 << "': " << query.lastError();
      return false;
    }
    ++pos;
  }
  rollback.commit();
  return true;
}

/**
 * Returns the internal name of the type with the given \a type_id.
 * @param type_id the database id of the type to look up
 * @return the type name, or an empty string if \a type_id is unknown
 */
QString colman::SQLQueries::get_type_name(int type_id)
{
  if(type_id >= 0) {
    QSqlQuery query(db_);
    query.prepare("SELECT name from types WHERE id = :id");
    query.bindValue(":id", type_id);
    if(!query.exec()) {
      qWarning() << "failed to retrieve type:\n"
                 << query.lastError();
      return "";
    }
    if(query.next()) {
      return query.value(0).toString();
    }
  }
  return "";
}

/**
 * Returns the SQLite table name used to store type-specific attributes
 * for the type with the given \a type_id.
 * @param type_id the database id of the type to look up
 * @return the table name, or an empty string if \a type_id is unknown
 */
QString colman::SQLQueries::get_type_tablename(int type_id)
{
  if(type_id >= 0) {
    QSqlQuery query(db_);
    query.prepare("SELECT tablename from types WHERE id = :id");
    query.bindValue(":id", type_id);
    if(!query.exec()) {
      qWarning() << "failed to retrieve type:\n"
                 << query.lastError();
      return "";
    }
    if(query.next()) {
      return query.value(0).toString();
    }
  }
  return "";
}

/**
 * Returns all (id, localised name) pairs for every type in the database.
 * The localised name is taken from the translations table for the current
 * system locale; the raw type name is used as a fallback if no
 * translation is available.
 * @return vector of (type_id, display_name) pairs for all types
 */
std::vector<std::pair<int, QString>>
colman::SQLQueries::get_type_locale(QLocale::Language lang)
{
  std::vector<std::pair<int, QString>> retval;
  QSqlQuery query(db_);
  query.prepare("SELECT types.id, types.name, translations.name FROM types "
                "LEFT JOIN translations ON translations.tablename = :tbl "
                "AND translations.language = :lang "
                "AND types.name = translations.column");
  query.bindValue(":tbl", QString("types"));
  auto const resolved = (lang == QLocale::AnyLanguage)
                            ? system_language()
                            : lang;
  query.bindValue(":lang", resolved);
  if(!query.exec()) {
    qWarning() << "failed to retrieve types and translations:\n"
               << query.lastError();
  }
  else {
    while(query.next()) {
      int const id = query.value(0).toInt();
      QString const type = query.value(1).toString();
      QString const trans = query.value(2).toString();
      retval.push_back(std::make_pair(id, (trans.isEmpty() ? type : trans)));
    }
  }
  return retval;
}

/**
 * Returns the localised display name of the type with the given
 * \a type_id.  Falls back to the raw type name if no translation for
 * the current system locale is available.
 * @param type_id the database id of the type
 * @return the localised name, or an empty string if the type is unknown
 */
QString colman::SQLQueries::get_type_locale(
    int type_id, QLocale::Language lang)
{
  QSqlQuery query(db_);
  query.prepare("SELECT types.name, translations.name FROM types "
                "LEFT JOIN translations ON translations.tablename = :tbl "
                "AND translations.language = :lang "
                "AND types.name = translations.column "
                "WHERE types.id = :type");
  query.bindValue(":tbl", QString("types"));
  auto const resolved = (lang == QLocale::AnyLanguage)
                            ? system_language()
                            : lang;
  query.bindValue(":lang", resolved);
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "failed to retrieve types and translations:\n"
               << query.lastError();
  }
  if(query.next()) {
    QString const type = query.value(0).toString();
    QString const trans = query.value(1).toString();
    return trans.isEmpty() ? type : trans;
  }
  return QString();
}

/**
 * Returns the localised display name of the type identified by its
 * internal \a type name.  Falls back to \a type itself if no
 * translation for the current system locale is available.
 * @param type the internal (non-localised) type name
 * @return the localised name, or an empty string if the type is unknown
 */
QString colman::SQLQueries::get_type_locale(
    QString const& type, QLocale::Language lang)
{
  QSqlQuery query(db_);
  query.prepare("SELECT translations.name FROM types "
                "LEFT JOIN translations ON translations.tablename = :tbl "
                "AND translations.language = :lang "
                "AND types.name = translations.column "
                "WHERE types.name = :type");
  query.bindValue(":tbl", QString("types"));
  auto const resolved = (lang == QLocale::AnyLanguage)
                            ? system_language()
                            : lang;
  query.bindValue(":lang", resolved);
  query.bindValue(":type", type);
  if(!query.exec()) {
    qWarning() << "failed to retrieve types and translations:\n"
               << query.lastError();
  }
  if(query.next()) {
    QString const trans = query.value(0).toString();
    return trans.isEmpty() ? type : trans;
  }
  return QString();
}

/**
 * Modifies an existing type definition to match \a new_type.  The
 * implementation renames the type-specific table to a temporary name,
 * recreates it with the new column set, copies data for common columns,
 * drops the temporary table, and then updates the translations and
 * data_types tables accordingly.  Columns that are removed from the
 * type will have their data discarded.
 * @param db       the open database connection
 * @param type_id  the database id of the type to update
 * @param new_type the new type definition
 * @return true on success, false on any database error
 */
bool colman::SQLQueries::change_type(
    int type_id, Type const& new_type,
    QMap<QString, QString> const& display_names)
{
  backup_once();
  colman::Type const old_type = get_type(type_id);
  QString const tbl = get_type_tablename(type_id);
  std::vector<QString> old_keys;
  std::vector<QString> common_keys;
  KeyVector new_keys;
  key_vector_diff(old_type.get_keys(), new_type.get_keys(), old_keys,
                  common_keys, new_keys);

  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  QString querytext("ALTER TABLE \"" + tbl +
                    "\" RENAME TO "
                    "\"__tmp__\"");
  query.prepare(querytext);
  if(!query.exec()) {
    qWarning() << "alter tablename failed: " << query.lastError();
    return false;
  }
  querytext = "CREATE TABLE IF NOT EXISTS \"" + tbl + "\"";
  querytext += " (item_id INTEGER PRIMARY KEY";
  for(auto const& key : new_type.get_keys()) {
    querytext += ", \"" + string_to_column_name(key.first) + "\" VARCHAR ";
  }
  querytext += ")";
  if(!query.exec(querytext)) {
    qWarning() << "failed to create table: " << query.lastError()
               << "\nwith query \"" << querytext << "\"\n";
    return false;
  }
  if(!common_keys.empty()) {
    querytext = "INSERT INTO \"" + tbl + "\" (item_id";
    for(auto const& key : common_keys) {
      querytext += ", \"";
      querytext += string_to_column_name(key);
      querytext += '"';
    }
    querytext += ") SELECT item_id";
    for(auto const& key : common_keys) {
      querytext += ", \"";
      querytext += string_to_column_name(key);
      querytext += '"';
    }
    querytext += " FROM __tmp__";
    if(!query.exec(querytext)) {
      qWarning() << "failed to copy old table to new table "
                 << query.lastError() << "\nwith query '" << querytext << "'\n";
      return false;
    }
  }
  query.clear();
  querytext = "DROP TABLE \"__tmp__\"";
  if(!query.exec(querytext)) {
    qWarning() << "failed to drop old table: " << query.lastError();
    return false;
  }
  if(!old_keys.empty()) {
    querytext =
        "DELETE FROM data_types WHERE type_id = " + QString().setNum(type_id) +
        " AND (";
    bool first = true;
    for(auto const& key : old_keys) {
      if(!first) {
        querytext += "OR ";
      }
      first = false;
      querytext += "column = '" + key + "' ";
    }
    querytext += ")";
    if(!query.exec(querytext)) {
      qWarning() << "failed to delete from data_types: " << query.lastError()
                 << "\nwith query: " << querytext;
      return false;
    }
  }
  {
    int pos = 0;
    for(auto const& key : new_type.get_keys()) {
      querytext = "INSERT OR REPLACE INTO data_types (type_id, column, "
                  "data_type, infos, position, marc21_tag) "
                  "VALUES (:type_id, :column, :data_type, :infos, "
                  ":position, :marc21_tag)";
      query.prepare(querytext);
      query.bindValue(":type_id", type_id);
      query.bindValue(":column", string_to_column_name(key.first));
      query.bindValue(":data_type", (int)key.second.type_);
      query.bindValue(":infos", serialize_map(key.second.info_));
      query.bindValue(":position", pos++);
      query.bindValue(":marc21_tag", key.second.info_["marc21_tag"].toString());
      if(!query.exec()) {
        qWarning() << "failed to insert or update type into data_types table:\n"
                   << query.lastError();
        return false;
      }
    }
  }
  if(!old_keys.empty()) {
    querytext = "DELETE FROM translations WHERE tablename = '" +
                tbl + "' AND (";
    bool first = true;
    for(auto const& key : old_keys) {
      if(!first) {
        querytext += "OR ";
      }
      first = false;
      querytext += "column = '" + key + "' ";
    }
    querytext += ")";
    if(!query.exec(querytext)) {
      qWarning() << "failed to delete from translations: " << query.lastError()
                 << "\nwith query: " << querytext;
      return false;
    }
  }
  QLocale::Language const sys_lang = system_language();
  QString const ins_or_rep = "INSERT OR REPLACE INTO translations "
                             "(tablename, column, language, name) VALUES "
                             "(:tablename, :column, :language, :name)";
  for(auto const& key : new_keys) {
    QString const col = string_to_column_name(key.first);
    query.prepare(ins_or_rep);
    query.bindValue(":tablename", tbl);
    query.bindValue(":column", col);
    query.bindValue(":language", (int)QLocale::AnyLanguage);
    query.bindValue(":name", col);
    if(!query.exec()) {
      qWarning() << "failed to insert into translations: " << query.lastError();
      return false;
    }
    if(sys_lang != QLocale::AnyLanguage) {
      query.prepare(ins_or_rep);
      query.bindValue(":tablename", tbl);
      query.bindValue(":column", col);
      query.bindValue(":language", (int)sys_lang);
      query.bindValue(":name", display_names.value(col, key.first));
      if(!query.exec()) {
        qWarning() << "failed to insert translation: " << query.lastError();
        return false;
      }
    }
  }
  if(sys_lang != QLocale::AnyLanguage) {
    for(auto const& key : common_keys) {
      if(display_names.contains(key)) {
        query.prepare(ins_or_rep);
        query.bindValue(":tablename", tbl);
        query.bindValue(":column", key);
        query.bindValue(":language", (int)sys_lang);
        query.bindValue(":name", display_names.value(key));
        if(!query.exec()) {
          qWarning() << "failed to update translation: " << query.lastError();
          return false;
        }
      }
    }
  }
  query.prepare("UPDATE translations SET name = :name "
                "WHERE tablename = 'types' AND column = :column "
                "AND language = :language");
  query.bindValue(":name", new_type.get_name());
  query.bindValue(":column", old_type.get_name());
  query.bindValue(":language", (int)QLocale::AnyLanguage);
  if(!query.exec()) {
    qWarning() << "failed to update translations table:\n"
               << query.lastError();
    return false;
  }
  if(sys_lang != QLocale::AnyLanguage) {
    query.prepare(ins_or_rep);
    query.bindValue(":tablename", "types");
    query.bindValue(":column", old_type.get_name());
    query.bindValue(":language", (int)sys_lang);
    query.bindValue(":name", new_type.get_name());
    if(!query.exec()) {
      qWarning() << "failed to update type name translation:\n"
                 << query.lastError();
      return false;
    }
  }
  query.prepare("UPDATE types SET media_kind = :media_kind WHERE id = :id");
  query.bindValue(":media_kind", static_cast<int>(new_type.get_media_kind()));
  query.bindValue(":id", type_id);
  if(!query.exec()) {
    qWarning() << "failed to update media_kind:\n"
               << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Removes a type and all items, lendings, returns, acquisitions, and
/// liquidations of that type.
/**
 * Cascades the deletion: first removes all returns belonging to lendings
 * of items of this type, then the lendings, liquidations, acquisitions,
 * schoolitems, and items themselves, then drops the type-specific table,
 * and finally deletes the types, translations, and data_types rows.  All
 * changes are wrapped in a single transaction.
 * @param type_id the database id of the type to remove
 * @return true on success, false if the type is unknown or a database
 *         error occurs
 */
bool colman::SQLQueries::remove_type(int type_id)
{
  backup_once();
  QString const tablename = get_type_tablename(type_id);
  if(tablename.isEmpty()) {
    return false;
  }
  AutoRollback rollback(db_);
  QSqlQuery query(db_);
  query.prepare("DELETE FROM returns WHERE lending IN "
                "(SELECT lendings.id FROM lendings JOIN items ON "
                "lendings.item = items.id AND items.type = :type)");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove returns with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM lendings WHERE item IN "
                "(SELECT id FROM items WHERE type = :type)");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove lendings with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM liquidations WHERE item IN "
                "(SELECT id FROM items WHERE type = :type)");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove acquisitions with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM acquisitions WHERE item IN "
                "(SELECT id FROM items WHERE type = :type)");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove acquisitions with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM schoolitems WHERE item_id IN "
                "(SELECT id FROM items WHERE type = :type)");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove schoolitems with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM item_list_values WHERE item_id IN "
                "(SELECT id FROM items WHERE type = :type)");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove item_list_values with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM items WHERE type = :type");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove items with given type: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM translations WHERE tablename = :tbl "
                "AND column IN (SELECT name FROM types "
                "WHERE id = :type)");
  query.bindValue(":tbl", QString("types"));
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove type translation: " << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM translations WHERE tablename = :tablename");
  query.bindValue(":tablename", tablename);
  if(!query.exec()) {
    qWarning() << "could not remove translations: " << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM data_types WHERE type_id = :type");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove data_type: " << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM common_field_positions WHERE type_id = :type");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove common_field_positions: "
               << query.lastError();
    return false;
  }
  query.prepare("DELETE FROM types WHERE id = :type");
  query.bindValue(":type", type_id);
  if(!query.exec()) {
    qWarning() << "could not remove type: " << query.lastError();
    return false;
  }
  query.prepare("DROP TABLE \"" + tablename + "\"");
  if(!query.exec()) {
    qWarning() << "could not drop table " << tablename << ": "
               << query.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/**
 * Returns the internal names of all item types stored in the database.
 * The names are returned in database order (insertion order).
 * @return vector of type name strings
 */
std::vector<QString> colman::SQLQueries::get_type_names()
{
  QString const querytext("SELECT name FROM types");
  QSqlQuery query(db_);
  query.prepare(querytext);
  if(!query.exec()) {
    qWarning() << query.lastError();
    qFatal("Failed to query types");
  }
  std::vector<QString> retval;
  while(query.next()) {
    retval.push_back(query.value(0).toString());
  }
  return retval;
}

/**
 * Returns the SQLite table name for the type with the given \a type_id.
 * Equivalent to get_type_tablename(); used internally throughout the
 * item CRUD functions.
 * @param type_id the database id of the type
 * @return the table name, or an empty string if \a type_id is unknown
 */
QString colman::SQLQueries::get_tablename(int type_id)
{
  QString retval;
  QSqlQuery query(db_);
  query.prepare("SELECT tablename FROM types WHERE id = :type_id");
  query.bindValue(":type_id", type_id);
  if(!query.exec()) {
    qWarning() << "Failed to query tablename for id: " << query.lastError();
  }
  else if(query.next()) {
    retval = query.value(0).toString();
  }
  return retval;
}

/**
 * Returns the localised display label for \a field of the type \a type_id.
 * Looks up the SQLite table name internally and delegates to get_translation().
 */
QString colman::SQLQueries::get_type_field_label(
    int type_id, QString const& field, QLocale::Language lang)
{
  return get_translation(get_type_tablename(type_id), field, lang);
}

/**
 * Returns the translation stored for a specific table column and
 * language code (legacy string overload).
 * @param table    the tablename key in the translations table
 * @param column   the column key in the translations table
 * @param language the language code string to look up
 * @return the translated name, or an empty string if not found
 */
QString colman::SQLQueries::get_translation(QString const& table,
                                            QString const& column,
                                            QString const& language)
{
  QSqlQuery query(db_);
  query.prepare("SELECT name from translations WHERE "
                "tablename = :tablename AND column = :column "
                "AND language = :language");
  query.bindValue(":tablename", table);
  query.bindValue(":column", column);
  query.bindValue(":language", language);
  if(!query.exec()) {
    qWarning() << "failed to retrieve translation:\n"
               << query.lastError();
    return "";
  }
  if(query.next()) {
    return query.value(0).toString();
  }
  return "";
}

/**
 * Returns the unlocalized default display name for a table column by
 * querying the "default" language entry in the translations table.
 * @param table  the tablename key in the translations table
 * @param column the column key in the translations table
 * @return the default name, or an empty string if not found
 */
QString colman::SQLQueries::get_default_name(QString const& table,
                                             QString const& column)
{
  return get_translation(table, column, "default");
}

/// Inserts or updates a localised name for a table column.
/**
 * Attempts to INSERT a new row into the translations table.  If the row
 * already exists (duplicate primary key), falls back to an UPDATE to
 * overwrite the name.  The column name is sanitised with
 * string_to_column_name() before storage.
 * @param tablename the table the translation belongs to (e.g. "books")
 * @param column    the column whose display name is being set
 * @param language  the locale language the name applies to
 * @param name      the localised display name to store
 * @return true on success, false if both INSERT and UPDATE fail
 */
bool colman::SQLQueries::add_translation(QString const& tablename,
                                         QString const& column,
                                         QLocale::Language language,
                                         QString const& name)
{
  QSqlQuery query(db_);
  query.prepare("INSERT INTO translations (tablename, column, language, name) "
                "VALUES (:tablename, :column, :language, :name)");
  query.bindValue(":tablename", tablename);
  query.bindValue(":column", string_to_column_name(column));
  query.bindValue(":language", language);
  query.bindValue(":name", name);
  if(!query.exec()) {
    query.prepare("UPDATE translations SET name = :name "
                  "WHERE tablename = :tablename AND column = :column "
                  "AND language = :language");
    query.bindValue(":tablename", tablename);
    query.bindValue(":column", string_to_column_name(column));
    query.bindValue(":language", language);
    query.bindValue(":name", name);
    if(!query.exec()) {
      qWarning() << "failed to insert type into translations table:\n"
                 << query.lastError();
      return false;
    }
  }
  return true;
}

/// Returns the translation for a table column in a specific language.
/**
 * Queries the translations table for the row matching \a tablename,
 * \a column, and \a language and returns the stored name.
 * @param db        the open database connection
 * @param tablename the table the translation belongs to (e.g. "books")
 * @param column    the column whose display name is requested
 * @param language  the locale language to retrieve
 * @return the localised name, or an empty string if not found
 */
QString colman::SQLQueries::get_translation(QString const& tablename,
                                            QString const& column,
                                            QLocale::Language language)
{
  QSqlQuery query(db_);
  query.prepare("SELECT name FROM translations "
                "WHERE tablename = :tablename "
                "AND column = :column "
                "AND language = :language");
  query.bindValue(":tablename", tablename);
  query.bindValue(":column", column);
  query.bindValue(":language", language);
  if(!query.exec()) {
    qWarning() << "could not query translation: " << query.lastError();
    return QString();
  }
  if(!query.next()) {
    return QString();
  }
  return query.value(0).toString();
}

bool colman::SQLQueries::add_predefined_books_type()
{
  colman::DataTypeInfo type;
  colman::KeyVector const kv{
      {"authors", make_type(colman::DataType::ShortString, false, QVariant(),
                            false, QVariant(), true, "100$a")},
      {"edition", make_type(colman::DataType::ShortString, false, QVariant(),
                            false, QVariant(), false, "250$a")},
      {"publication_date", make_type(colman::DataType::Date, false, QVariant(),
                                     false, QVariant(), false, "260$c")},
      {"publisher", make_type(colman::DataType::ShortString, false, QVariant(),
                              false, QVariant(), false, "260$b")},
      {"pages", make_type(colman::DataType::Integer, true, QVariant(1), false,
                          QVariant(), false, "300$a")},
      {"ISBN10", make_type(colman::DataType::ShortString, true, QVariant(10),
                           true, QVariant(10), true, "020$a")},
      {"ISBN13", make_type(colman::DataType::ShortString, true, QVariant(13),
                           true, QVariant(13), true, "020$a")},
      {"ddc", make_type(colman::DataType::ShortString, false, QVariant(), false,
                        QVariant(), true, "082$a")}};
  colman::Type book_type{"book", kv};
  book_type.set_media_kind(colman::MediaKind::Book);
  int const type_id = add_new_type(book_type, "books");
  if(type_id < 0) {
    return false;
  }
  save_field_order(type_id, default_book_field_order());
  return true;
}

bool colman::SQLQueries::add_predefined_periodical_type()
{
  using DT = colman::DataType;
  colman::KeyVector const kv{
      {"issn", make_type(DT::ShortString, false, QVariant(), false, QVariant(),
                         false, "022$a")},
      {"publisher", make_type(DT::ShortString, false, QVariant(), false,
                              QVariant(), false, "260$b")},
      {"publication_date", make_type(DT::ShortString, false, QVariant(), false,
                                     QVariant(), false, "260$c")},
      {"extent", make_type(DT::ShortString, false, QVariant(), false,
                           QVariant(), false, "300$a")},
      {"note", make_type(DT::String, false, QVariant(), false, QVariant(),
                         false, "500$a")}};
  int const type_id =
      add_new_type(colman::Type{"periodical", kv}, "periodicals");
  if(type_id < 0) {
    return false;
  }
  save_field_order(type_id, default_periodical_field_order());
  return true;
}

bool colman::SQLQueries::add_predefined_cd_type()
{
  using DT = colman::DataType;
  colman::KeyVector const kv{
      {"artist", make_type(DT::ShortString, false, QVariant(), false,
                           QVariant(), true, "100$a")},
      {"publisher_number", make_type(DT::ShortString, false, QVariant(), false,
                                     QVariant(), false, "028$a")},
      {"publisher", make_type(DT::ShortString, false, QVariant(), false,
                              QVariant(), false, "260$b")},
      {"publication_date", make_type(DT::ShortString, false, QVariant(), false,
                                     QVariant(), false, "260$c")},
      {"language", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), false, "041$a")},
      {"extent", make_type(DT::ShortString, false, QVariant(), false,
                           QVariant(), false, "300$a")},
      {"content_type", make_type(DT::ShortString, false, QVariant(), false,
                                 QVariant(), false, "336$a")},
      {"media_type", make_type(DT::ShortString, false, QVariant(), false,
                               QVariant(), false, "337$a")},
      {"carrier_type", make_type(DT::ShortString, false, QVariant(), false,
                                 QVariant(), false, "338$a")},
      {"ean", make_type(DT::ShortString, false, QVariant(), false, QVariant(),
                        false, "")}};
  colman::Type cd_type{"cd", kv};
  cd_type.set_media_kind(colman::MediaKind::Cd);
  int const type_id = add_new_type(cd_type, "cds");
  if(type_id < 0) {
    return false;
  }
  save_field_order(type_id, default_cd_field_order());
  return true;
}

bool colman::SQLQueries::add_predefined_dvd_type()
{
  using DT = colman::DataType;
  colman::KeyVector const kv{
      {"director", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), true, "700$a")},
      {"publication_date", make_type(DT::ShortString, false, QVariant(), false,
                                     QVariant(), false, "260$c")},
      {"language", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), false, "041$a")},
      {"extent", make_type(DT::ShortString, false, QVariant(), false,
                           QVariant(), false, "300$a")},
      {"content_type", make_type(DT::ShortString, false, QVariant(), false,
                                 QVariant(), false, "336$a")},
      {"media_type", make_type(DT::ShortString, false, QVariant(), false,
                               QVariant(), false, "337$a")},
      {"carrier_type", make_type(DT::ShortString, false, QVariant(), false,
                                 QVariant(), false, "338$a")},
      {"participants", make_type(DT::String, false, QVariant(), false,
                                 QVariant(), false, "511$a")},
      {"target_audience", make_type(DT::ShortString, false, QVariant(), false,
                                    QVariant(), false, "521$a")},
      {"publisher", make_type(DT::ShortString, false, QVariant(), false,
                              QVariant(), false, "264$b")},
      {"ean", make_type(DT::ShortString, false, QVariant(), false, QVariant(),
                        false, "")}};
  colman::Type dvd_type{"dvd", kv};
  dvd_type.set_media_kind(colman::MediaKind::Dvd);
  int const type_id = add_new_type(dvd_type, "dvds");
  if(type_id < 0) {
    return false;
  }
  save_field_order(type_id, default_dvd_field_order());
  return true;
}

bool colman::SQLQueries::add_predefined_bluray_type()
{
  using DT = colman::DataType;
  colman::KeyVector const kv{
      {"director", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), true, "700$a")},
      {"publication_date", make_type(DT::ShortString, false, QVariant(), false,
                                     QVariant(), false, "260$c")},
      {"language", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), false, "041$a")},
      {"extent", make_type(DT::ShortString, false, QVariant(), false,
                           QVariant(), false, "300$a")},
      {"content_type", make_type(DT::ShortString, false, QVariant(), false,
                                 QVariant(), false, "336$a")},
      {"media_type", make_type(DT::ShortString, false, QVariant(), false,
                               QVariant(), false, "337$a")},
      {"carrier_type", make_type(DT::ShortString, false, QVariant(), false,
                                 QVariant(), false, "338$a")},
      {"participants", make_type(DT::String, false, QVariant(), false,
                                 QVariant(), false, "511$a")},
      {"target_audience", make_type(DT::ShortString, false, QVariant(), false,
                                    QVariant(), false, "521$a")},
      {"publisher", make_type(DT::ShortString, false, QVariant(), false,
                              QVariant(), false, "264$b")},
      {"ean", make_type(DT::ShortString, false, QVariant(), false, QVariant(),
                        false, "")}};
  colman::Type bluray_type{"bluray", kv};
  bluray_type.set_media_kind(colman::MediaKind::BluRay);
  int const type_id = add_new_type(bluray_type, "blurays");
  if(type_id < 0) {
    return false;
  }
  save_field_order(type_id, default_dvd_field_order());
  return true;
}

bool colman::SQLQueries::add_predefined_board_game_type()
{
  using DT = colman::DataType;
  colman::KeyVector const kv{
      {"designer", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), true, "700$a")},
      {"artist", make_type(DT::ShortString, false, QVariant(), false,
                           QVariant(), true, "700$a")},
      {"publisher", make_type(DT::ShortString, false, QVariant(), false,
                              QVariant(), false, "264$b")},
      {"publication_date", make_type(DT::ShortString, false, QVariant(), false,
                                     QVariant(), false, "264$c")},
      {"min_players",
       make_type(DT::Integer, true, QVariant(1), false, QVariant(), false,
                 "521$a")},
      {"max_players",
       make_type(DT::Integer, true, QVariant(1), false, QVariant(), false,
                 "521$a")},
      {"min_age",
       make_type(DT::Integer, true, QVariant(0), false, QVariant(), false,
                 "521$a")},
      {"playing_time",
       make_type(DT::Integer, true, QVariant(1), false, QVariant(), false,
                 "300$a")},
      {"language", make_type(DT::ShortString, false, QVariant(), false,
                             QVariant(), true, "546$a")},
      {"genre", make_type(DT::ShortString, false, QVariant(), false,
                          QVariant(), true, "655$a")},
      {"ean", make_type(DT::ShortString, false, QVariant(), false, QVariant(),
                        false, "024$a")}};
  colman::Type board_game_type{"board_game", kv};
  board_game_type.set_media_kind(colman::MediaKind::BoardGame);
  int const type_id = add_new_type(board_game_type, "board_games");
  if(type_id < 0) {
    return false;
  }
  save_field_order(type_id, default_board_game_field_order());
  return true;
}

bool colman::SQLQueries::add_predefined_types()
{
  return add_predefined_books_type() && add_predefined_periodical_type() &&
         add_predefined_cd_type() && add_predefined_dvd_type() &&
         add_predefined_bluray_type() && add_predefined_board_game_type() &&
         ensure_predefined_translations();
}

/**
   Stores the German and English display names of the predefined media types
   and their fields (see PredefinedNames) in the translations table.  A name
   is only written where the translation is missing or still the raw column
   name, so names changed by the user are kept.  Types or fields that no
   longer exist are skipped.  Idempotent and read-only when everything is in
   place, so it runs on every database open and migrates collections created
   before the English names existed.
   @return true on success
*/
bool colman::SQLQueries::ensure_predefined_translations()
{
  QSqlQuery query(db_);
  for(PredefinedName const& n : PredefinedNames) {
    QString const table = QString::fromLatin1(n.table_);
    QString const column = string_to_column_name(QString::fromLatin1(n.column_));
    if(table == "types") {
      query.prepare("SELECT 1 FROM types WHERE name = :name");
      query.bindValue(":name", column);
    }
    else {
      query.prepare("SELECT 1 FROM translations "
                    "WHERE tablename = :tablename AND column = :column");
      query.bindValue(":tablename", table);
      query.bindValue(":column", column);
    }
    if(!query.exec()) {
      qWarning() << "checking predefined type failed: " << query.lastError();
      return false;
    }
    if(!query.next()) {
      continue;
    }
    std::pair<QLocale::Language, QString> const names[] = {
        {QLocale::German, QString::fromUtf8(n.german_)},
        {QLocale::English, QString::fromUtf8(n.english_)}};
    for(auto const& name : names) {
      query.prepare("SELECT name FROM translations WHERE tablename = :tablename "
                    "AND column = :column AND language = :language");
      query.bindValue(":tablename", table);
      query.bindValue(":column", column);
      query.bindValue(":language", static_cast<int>(name.first));
      if(!query.exec()) {
        qWarning() << "reading translation failed: " << query.lastError();
        return false;
      }
      if(query.next()) {
        QString const current = query.value(0).toString();
        if(current == name.second ||
           current.compare(column, Qt::CaseInsensitive) != 0)
        {
          continue;
        }
      }
      if(!add_translation(table, column, name.first, name.second)) {
        return false;
      }
    }
  }
  return true;
}

/* EOF */
