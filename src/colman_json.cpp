// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_JSON_HPP
#  include "colman_json.hpp"
#endif
#ifndef SYSTEM_QJSONARRAY
#  include <QJsonArray>
#  define SYSTEM_QJSONARRAY
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
#endif

namespace
{

  QString json_string(QJsonObject const& obj,
                      QString const& key,
                      QString fallback = {})
  {
    auto const v = obj.value(key);
    if(v.isString()) {
      return v.toString();
    }
    return fallback;
  }

  int json_int(QJsonObject const& obj, QString const& key, int fallback = -1)
  {
    auto const v = obj.value(key);
    if(v.isDouble()) {
      return v.toInt();
    }
    return fallback;
  }

  bool json_bool(QJsonObject const& obj,
                 QString const& key,
                 bool fallback = false)
  {
    auto const v = obj.value(key);
    if(v.isBool()) {
      return v.toBool();
    }
    return fallback;
  }

  QDateTime json_datetime(QJsonObject const& obj, QString const& key)
  {
    auto const v = obj.value(key);
    if(v.isString()) {
      return QDateTime::fromString(v.toString(), Qt::ISODate);
    }
    return QDateTime{};
  }

  QDate json_date(QJsonObject const& obj, QString const& key)
  {
    auto const v = obj.value(key);
    if(v.isString()) {
      return QDate::fromString(v.toString(), Qt::ISODate);
    }
    return QDate{};
  }

  QStringList json_stringlist(QJsonObject const& obj, QString const& key)
  {
    QStringList result;
    auto const v = obj.value(key);
    if(!v.isArray()) {
      return result;
    }
    for(auto const& elem : v.toArray()) {
      if(elem.isString()) {
        result.append(elem.toString());
      }
    }
    return result;
  }

  colman::StringMap json_stringmap(QJsonObject const& obj,
                                   QString const& key)
  {
    colman::StringMap result;
    auto const v = obj.value(key);
    if(!v.isObject()) {
      return result;
    }
    auto const inner = v.toObject();
    for(auto it = inner.constBegin(); it != inner.constEnd(); ++it) {
      if(it.value().isString()) {
        result[it.key()] = it.value().toString();
      }
    }
    return result;
  }

  colman::StringListMap json_stringlistmap(QJsonObject const& obj,
                                           QString const& key)
  {
    colman::StringListMap result;
    auto const v = obj.value(key);
    if(!v.isObject()) {
      return result;
    }
    auto const inner = v.toObject();
    for(auto it = inner.constBegin(); it != inner.constEnd(); ++it) {
      QStringList lst;
      if(it.value().isArray()) {
        for(auto const& elem : it.value().toArray()) {
          if(elem.isString()) {
            lst.append(elem.toString());
          }
        }
      }
      result[it.key()] = lst;
    }
    return result;
  }

  colman::IntPairVec json_intpairvec(QJsonObject const& obj,
                                     QString const& key)
  {
    colman::IntPairVec result;
    auto const v = obj.value(key);
    if(!v.isArray()) {
      return result;
    }
    for(auto const& elem : v.toArray()) {
      if(!elem.isArray()) {
        continue;
      }
      auto const pair = elem.toArray();
      if(pair.size() >= 2 && pair[0].isDouble() && pair[1].isDouble()) {
        result.push_back({pair[0].toInt(), pair[1].toInt()});
      }
    }
    return result;
  }

  bool required_int(QJsonObject const& obj, QString const& key, int& out)
  {
    auto const v = obj.value(key);
    if(!v.isDouble()) {
      return false;
    }
    out = v.toInt();
    return true;
  }

  bool required_uint(QJsonObject const& obj,
                     QString const& key,
                     unsigned int& out)
  {
    auto const v = obj.value(key);
    if(!v.isDouble()) {
      return false;
    }
    out = static_cast<unsigned int>(v.toInt());
    return true;
  }

  QJsonObject stringmap_to_json(colman::StringMap const& m)
  {
    QJsonObject obj;
    for(auto const& kv : m) {
      obj.insert(kv.first, kv.second);
    }
    return obj;
  }

  QJsonObject stringlistmap_to_json(colman::StringListMap const& m)
  {
    QJsonObject obj;
    for(auto const& kv : m) {
      QJsonArray arr;
      for(auto const& s : kv.second) {
        arr.append(s);
      }
      obj.insert(kv.first, arr);
    }
    return obj;
  }

  QJsonArray intpairvec_to_json(colman::IntPairVec const& v)
  {
    QJsonArray arr;
    for(auto const& p : v) {
      QJsonArray pair;
      pair.append(p.first);
      pair.append(p.second);
      arr.append(pair);
    }
    return arr;
  }

  QJsonValue datetime_to_json(QDateTime const& dt)
  {
    if(dt.isValid()) {
      return QJsonValue{dt.toString(Qt::ISODate)};
    }
    return QJsonValue{};
  }

  QJsonValue date_to_json(QDate const& d)
  {
    if(d.isValid()) {
      return QJsonValue{d.toString(Qt::ISODate)};
    }
    return QJsonValue{};
  }

  QJsonArray stringlist_to_json(QStringList const& lst)
  {
    QJsonArray arr;
    for(auto const& s : lst) {
      arr.append(s);
    }
    return arr;
  }

} // namespace

// ---------------------------------------------------------------------------
// Item
// ---------------------------------------------------------------------------

/// Serializes \a item to a JSON object.
QJsonObject colman::to_json(Item const& item)
{
  QJsonObject obj;
  obj.insert("type_id", item.get_type_id());
  obj.insert("name", item.get_name());
  obj.insert("barcode", item.get_barcode());
  obj.insert("place", item.get_place());
  obj.insert("description", item.get_description());
  obj.insert("values", stringmap_to_json(item.get_values()));
  obj.insert("list_values",
             stringlistmap_to_json(item.get_list_values()));
  obj.insert("thumbnail_id", item.get_thumbnail_id());
  obj.insert("is_school_item", item.get_is_school_item());
  obj.insert("subject", item.get_subject());
  obj.insert("academic_year",
             intpairvec_to_json(item.get_academic_year()));
  obj.insert("type", item.get_type());
  obj.insert("data_collector_id", item.get_data_collector_id());
  obj.insert("signature_top", item.get_signature_top());
  obj.insert("signature_bottom", item.get_signature_bottom());
  obj.insert("keywords", stringlist_to_json(item.get_keywords()));
  if(item.get_allow_reservations() == -1) {
    obj.insert("allow_reservations", QJsonValue::Null);
  }
  else {
    obj.insert("allow_reservations", item.get_allow_reservations());
  }
  if(item.get_reservation_period_days() == -1) {
    obj.insert("reservation_period_days", QJsonValue::Null);
  }
  else {
    obj.insert("reservation_period_days", item.get_reservation_period_days());
  }
  return obj;
}

/// Populates \a item from \a obj.
/** Required: "type_id" (integer ≥ 0). */
bool colman::from_json(QJsonObject const& obj, Item& item)
{
  int type_id = 0;
  if(!required_int(obj, "type_id", type_id) || type_id < 0) {
    return false;
  }
  item.set_type_id(type_id);
  item.set_name(json_string(obj, "name"));
  item.set_barcode(json_string(obj, "barcode"));
  item.set_place(json_string(obj, "place"));
  item.set_description(json_string(obj, "description"));
  item.set_values(json_stringmap(obj, "values"));
  item.set_list_values(json_stringlistmap(obj, "list_values"));
  item.set_thumbnail_id(json_int(obj, "thumbnail_id"));
  item.set_is_school_item(json_bool(obj, "is_school_item"));
  item.set_subject(json_string(obj, "subject"));
  item.set_academic_year(json_intpairvec(obj, "academic_year"));
  item.set_type(json_string(obj, "type"));
  item.set_data_collector_id(json_int(obj, "data_collector_id"));
  item.set_signature_top(json_string(obj, "signature_top"));
  item.set_signature_bottom(json_string(obj, "signature_bottom"));
  item.set_keywords(json_stringlist(obj, "keywords"));
  {
    auto const v = obj.value(QStringLiteral("allow_reservations"));
    item.set_allow_reservations(v.isNull() || v.isUndefined() ? -1 : v.toInt(-1));
  }
  {
    auto const v = obj.value(QStringLiteral("reservation_period_days"));
    item.set_reservation_period_days(
        v.isNull() || v.isUndefined() ? -1 : v.toInt(-1));
  }
  return true;
}

// ---------------------------------------------------------------------------
// Person
// ---------------------------------------------------------------------------

/// Serializes \a person to a JSON object.
QJsonObject colman::to_json(Person const& person)
{
  QJsonObject obj;
  obj.insert("id", person.get_id());
  obj.insert("lastname", person.get_lastname());
  obj.insert("firstname", person.get_firstname());
  obj.insert("title", person.get_title());
  obj.insert("school_class", person.get_school_class());
  obj.insert("barcode", person.get_barcode());
  obj.insert("emails", person.get_emails());
  obj.insert("telephone", person.get_telephone());
  obj.insert("street", person.get_street());
  obj.insert("zip", person.get_zip());
  obj.insert("city", person.get_city());
  obj.insert("country", person.get_country());
  obj.insert("remarks", person.get_remarks());
  obj.insert("thumbnail_id", person.get_thumbnail_id());
  obj.insert("data_collector_id", person.get_data_collector_id());
  obj.insert("username", person.get_username());
  return obj;
}

/// Populates \a person from \a obj.
bool colman::from_json(QJsonObject const& obj, Person& person)
{
  person.set_id(json_int(obj, "id", 0));
  person.set_lastname(json_string(obj, "lastname"));
  person.set_firstname(json_string(obj, "firstname"));
  person.set_title(json_string(obj, "title"));
  person.set_school_class(json_string(obj, "school_class"));
  person.set_barcode(json_string(obj, "barcode"));
  person.set_emails(json_string(obj, "emails"));
  person.set_telephone(json_string(obj, "telephone"));
  person.set_street(json_string(obj, "street"));
  person.set_zip(json_string(obj, "zip"));
  person.set_city(json_string(obj, "city"));
  person.set_country(json_string(obj, "country"));
  person.set_remarks(json_string(obj, "remarks"));
  person.set_thumbnail_id(json_int(obj, "thumbnail_id"));
  person.set_data_collector_id(json_int(obj, "data_collector_id"));
  person.set_username(json_string(obj, "username"));
  return true;
}

// ---------------------------------------------------------------------------
// Lending
// ---------------------------------------------------------------------------

/// Serializes \a lending to a JSON object.
QJsonObject colman::to_json(Lending const& lending)
{
  QJsonObject obj;
  obj.insert("item_id", lending.get_item_id());
  obj.insert("person_id", lending.get_person_id());
  obj.insert("count", static_cast<int>(lending.get_count()));
  obj.insert("date_time", datetime_to_json(lending.get_date_time()));
  obj.insert("duedate", date_to_json(lending.get_duedate()));
  obj.insert("remarks", lending.get_remarks());
  obj.insert("data_collector_id", lending.get_data_collector_id());
  return obj;
}

/// Populates \a lending from \a obj.
/** Required: "item_id", "person_id", "count". */
bool colman::from_json(QJsonObject const& obj, Lending& lending)
{
  int item_id = 0;
  int person_id = 0;
  unsigned int count = 0;
  if(!required_int(obj, "item_id", item_id) ||
     !required_int(obj, "person_id", person_id) ||
     !required_uint(obj, "count", count)) {
    return false;
  }
  lending.set_item_id(item_id);
  lending.set_person_id(person_id);
  lending.set_count(count);
  lending.set_date_time(json_datetime(obj, "date_time"));
  lending.set_duedate(json_date(obj, "duedate"));
  lending.set_remarks(json_string(obj, "remarks"));
  lending.set_data_collector_id(json_int(obj, "data_collector_id"));
  return true;
}

// ---------------------------------------------------------------------------
// Return
// ---------------------------------------------------------------------------

/// Serializes \a ret to a JSON object.
QJsonObject colman::to_json(Return const& ret)
{
  QJsonObject obj;
  obj.insert("lending_id", ret.get_lending_id());
  obj.insert("count", static_cast<int>(ret.get_count()));
  obj.insert("date_time", datetime_to_json(ret.get_date_time()));
  obj.insert("remarks", ret.get_remarks());
  obj.insert("data_collector_id", ret.get_data_collector_id());
  return obj;
}

/// Populates \a ret from \a obj.
/** Required: "lending_id", "count". */
bool colman::from_json(QJsonObject const& obj, Return& ret)
{
  int lending_id = 0;
  unsigned int count = 0;
  if(!required_int(obj, "lending_id", lending_id) ||
     !required_uint(obj, "count", count)) {
    return false;
  }
  ret.set_lending_id(lending_id);
  ret.set_count(count);
  ret.set_date_time(json_datetime(obj, "date_time"));
  ret.set_remarks(json_string(obj, "remarks"));
  ret.set_data_collector_id(json_int(obj, "data_collector_id"));
  return true;
}

// ---------------------------------------------------------------------------
// Acquisition
// ---------------------------------------------------------------------------

/// Serializes \a acquisition to a JSON object.
QJsonObject colman::to_json(Acquisition const& acquisition)
{
  QJsonObject obj;
  obj.insert("item_id", acquisition.get_item_id());
  obj.insert("count", static_cast<int>(acquisition.get_count()));
  obj.insert("date_time",
             datetime_to_json(acquisition.get_date_time()));
  obj.insert("data_collector_id",
             acquisition.get_data_collector_id());
  return obj;
}

/// Populates \a acquisition from \a obj.
/** Required: "item_id", "count". */
bool colman::from_json(QJsonObject const& obj, Acquisition& acquisition)
{
  int item_id = 0;
  unsigned int count = 0;
  if(!required_int(obj, "item_id", item_id) ||
     !required_uint(obj, "count", count)) {
    return false;
  }
  acquisition.set_item_id(item_id);
  acquisition.set_count(count);
  acquisition.set_date_time(json_datetime(obj, "date_time"));
  acquisition.set_data_collector_id(
      json_int(obj, "data_collector_id"));
  return true;
}

// ---------------------------------------------------------------------------
// Liquidation
// ---------------------------------------------------------------------------

/// Serializes \a liquidation to a JSON object.
QJsonObject colman::to_json(Liquidation const& liquidation)
{
  QJsonObject obj;
  obj.insert("item_id", liquidation.get_item_id());
  obj.insert("count", static_cast<int>(liquidation.get_count()));
  obj.insert("date_time",
             datetime_to_json(liquidation.get_date_time()));
  obj.insert("data_collector_id",
             liquidation.get_data_collector_id());
  return obj;
}

/// Populates \a liquidation from \a obj.
/** Required: "item_id", "count". */
bool colman::from_json(QJsonObject const& obj, Liquidation& liquidation)
{
  int item_id = 0;
  unsigned int count = 0;
  if(!required_int(obj, "item_id", item_id) ||
     !required_uint(obj, "count", count)) {
    return false;
  }
  liquidation.set_item_id(item_id);
  liquidation.set_count(count);
  liquidation.set_date_time(json_datetime(obj, "date_time"));
  liquidation.set_data_collector_id(
      json_int(obj, "data_collector_id"));
  return true;
}

// ---------------------------------------------------------------------------
// Reservation
// ---------------------------------------------------------------------------

/// Serializes \a reservation to a JSON object.
QJsonObject colman::to_json(IQueries::Reservation const& reservation)
{
  QJsonObject obj;
  obj.insert("id", reservation.id);
  obj.insert("item_id", reservation.item_id);
  obj.insert("person_id", reservation.person_id);
  obj.insert("count", reservation.count);
  obj.insert("reserved_at",
             datetime_to_json(reservation.reserved_at));
  obj.insert("expires_at", date_to_json(reservation.expires_at));
  obj.insert("version", reservation.version);
  obj.insert("data_collector_id", reservation.data_collector_id);
  return obj;
}

/// Populates \a reservation from \a obj.
/** Required: "item_id", "person_id", "count", "expires_at". */
bool colman::from_json(QJsonObject const& obj,
                       IQueries::Reservation& reservation)
{
  int item_id = 0;
  int person_id = 0;
  int count = 0;
  if(!required_int(obj, "item_id", item_id) ||
     !required_int(obj, "person_id", person_id) ||
     !required_int(obj, "count", count)) {
    return false;
  }
  QDate const expires = json_date(obj, "expires_at");
  if(!expires.isValid()) {
    return false;
  }
  reservation.item_id = item_id;
  reservation.person_id = person_id;
  reservation.count = count;
  reservation.expires_at = expires;
  reservation.reserved_at = json_datetime(obj, "reserved_at");
  reservation.id = json_int(obj, "id");
  reservation.version = json_int(obj, "version", 1);
  reservation.data_collector_id = json_int(obj, "data_collector_id");
  return true;
}

/* EOF */
