// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Item.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item.hpp"
#ifndef SYSTEM_QTEXTSTREAM
#  include <QTextStream>
#  define SYSTEM_QTEXTSTREAM
#endif
#ifndef SYSTEM_QDEBUG
#  include <QDebug>
#  define SYSTEM_QDEBUG
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif

namespace
{
  /// Returns the value for \a key from \a item's type-specific map, or empty.
  /**
     @param item the item whose type-specific value map is searched.
     @param key attribute key to look up.
     @return the value for @p key, or an empty string if the key is absent.
  */
  QString value(colman::Item const& item, QString const& key)
  {
    auto const it = item.get_values().find(key);
    if(it != item.get_values().end()) {
      return it->second;
    }
    return QString();
  }

  /// Returns @p have, or @p add when @p have is blank.
  QString fill_blank(QString const& have, QString const& add)
  {
    return have.trimmed().isEmpty() ? add : have;
  }

  /// Returns true when @p values contains no non-blank entry.
  bool all_blank(QStringList const& values)
  {
    for(QString const& v : values) {
      if(!v.trimmed().isEmpty()) {
        return false;
      }
    }
    return true;
  }

} // namespace


/// Prints all fields of @p item to stdout.
/**
   @param item the item to print.
*/
void colman::print_item(Item const& item)
{
  QTextStream(stdout) << "name:        " << item.get_name() << "\n"
                      << "type id:     " << item.get_type_id() << "\n"
                      << "barcode:     " << item.get_barcode() << "\n"
                      << "place:       " << item.get_place() << "\n"
                      << "description: " << item.get_description() << "\n"
                      << "authors:     " << value(item, "authors") << "\n"
                      << "edition:     " << value(item, "edition") << "\n"
                      << "publishers:  " << value(item, "publisher") << "\n"
                      << "published:   " << value(item, "publication_date") << "\n"
                      << "ISBN-10:     " << value(item, "ISBN10") << "\n"
                      << "ISBN-13:     " << value(item, "ISBN13") << "\n";
  if(item.get_is_school_item()) {
    QTextStream(stdout) << "subject:     " << item.get_subject() << "\n"
                        << "academic years: "
                        << academic_years_to_string(item.get_academic_year())
                        << "\n"
                        << "type:        " << item.get_type() << "\n";
  }

  QString const pages = value(item, "pages");
  if(pages.size() > 0) {
    QTextStream(stdout) << "pages:       " << pages << "\n";
  }
  QString const ddc = value(item, "ddc");
  if(ddc.size() > 0) {
    QTextStream(stdout) << "DDC:         " << ddc << "\n";
  }
  QString const genre = value(item, "genre");
  if(genre.size() > 0) {
    QTextStream(stdout) << "genre:       " << genre << "\n";
  }
}


/**
   Sets the item type id.
   @param type_id database id of the item type.
*/
void colman::Item::set_type_id(int type_id)
{
  type_id_ = type_id;
}


/**
   Sets the item name.
   @param name display name of the item.
*/
void colman::Item::set_name(QString const& name)
{
  name_ = name;
}


/**
   Sets the barcode string.
   @param barcode barcode string for the item.
*/
void colman::Item::set_barcode(QString const& barcode)
{
  barcode_ = barcode;
}


/**
   Sets the storage location.
   @param place storage location identifier.
*/
void colman::Item::set_place(QString const& place)
{
  place_ = place;
}


/**
   Sets the description.
   @param description free-text description of the item.
*/
void colman::Item::set_description(QString const& description)
{
  description_ = description;
}


/**
   Replaces the type-specific key-value map.
   @param values new type-specific key-value map.
*/
void colman::Item::set_values(StringMap const& values)
{
  values_ = values;
}


/**
   Sets a single key-value pair.
   @param key attribute key.
   @param value attribute value.
*/
void colman::Item::set_value(QString const& key, QString const& value)
{
  values_[key] = value;
}


/**
   Appends \a value to the existing value for \a key.
   @param key attribute key.
   @param value string to append.
*/
void colman::Item::append_to_value(QString const& key, QString const& value)
{
  values_[key] += value;
}


/**
   Swaps the key-value map with \a values.
   @param values map to swap with the internal map.
*/
void colman::Item::swap_values(StringMap& values)
{
  values_.swap(values);
}


/**
   Sets the thumbnail database id.
   @param thumbnail_id database id of the thumbnail image, or -1.
*/
void colman::Item::set_thumbnail_id(int thumbnail_id)
{
  thumbnail_id_ = thumbnail_id;
}


/**
   Sets the is_school_item flag.
   @param is_school_item true if this item is a school item.
*/
void colman::Item::set_is_school_item(bool is_school_item)
{
  is_school_item_ = is_school_item;
}


/**
   Sets the school subject.
   @param subject school subject name.
*/
void colman::Item::set_subject(QString const& subject)
{
  subject_ = subject;
}


/**
   Sets the list of academic year ranges.
   @param academic_year list of academic year ranges for this item.
*/
void colman::Item::set_academic_year(IntPairVec const& academic_year)
{
  academic_year_ = academic_year;
}


/**
   Clears the list of academic year ranges.
*/
void colman::Item::clear_academic_year()
{
  academic_year_.clear();
}


/**
   Sets the type name string.
   @param type type name string.
*/
void colman::Item::set_type(QString const& type)
{
  type_ = type;
}


/**
   Sets the data collector person id.
   @param data_collector_id database id of the data collector, or -1.
*/
void colman::Item::set_data_collector_id(int data_collector_id)
{
  data_collector_id_ = data_collector_id;
}


/**
   Sets the top spine signature label.
   @param signature_top free-text signature printed on the top of the spine.
*/
void colman::Item::set_signature_top(QString const& signature_top)
{
  signature_top_ = signature_top;
}


/**
   Sets the bottom spine signature label.
   @param signature_bottom free-text signature printed on the bottom of the spine.
*/
void colman::Item::set_signature_bottom(QString const& signature_bottom)
{
  signature_bottom_ = signature_bottom;
}


/**
   Sets the search keywords.
   @param keywords list of keyword strings used for searching.
*/
void colman::Item::set_keywords(QStringList const& keywords)
{
  keywords_ = keywords;
}


/**
   Replaces all repeatable field value lists.
   @param list_values new map of repeatable field value lists.
*/
void colman::Item::set_list_values(StringListMap const& list_values)
{
  list_values_ = list_values;
}


/**
   Sets the value list for a single repeatable field.
   @param key    attribute key.
   @param values list of values for this key.
*/
void colman::Item::set_list_value(QString const& key, QStringList const& values)
{
  list_values_[key] = values;
}


/**
   Appends a single string to the list for \a key.
   @param key   attribute key.
   @param value string to append.
*/
void colman::Item::append_list_value(QString const& key, QString const& value)
{
  list_values_[key].append(value);
}


/**
   Swaps the list-value map with \a list_values.
   @param list_values map to swap with the internal map.
*/
void colman::Item::swap_list_values(StringListMap& list_values)
{
  list_values_.swap(list_values);
}


/// Resets all fields to their initial empty/invalid values.
void colman::Item::clear()
{
  type_id_ = -1;
  name_.clear();
  barcode_.clear();
  place_.clear();
  description_.clear();
  values_.clear();
  list_values_.clear();
  thumbnail_id_ = -1;
  is_school_item_ = false;
  subject_.clear();
  academic_year_.clear();
  type_.clear();
  data_collector_id_ = -1;
  signature_top_.clear();
  signature_bottom_.clear();
  keywords_.clear();
  allow_reservations_ = -1;
  reservation_period_days_ = -1;
}


/// Merges @p item into this item, filling in any empty fields.
/**
   @param item source item to merge from.
   @return reference to this item.
*/
colman::Item& colman::Item::operator+=(Item const& item)
{
  if(type_id_ < 0) {
    type_id_ = item.get_type_id();
  }
  if(name_.isEmpty()) {
    name_ = item.get_name();
  }
  if(barcode_.isEmpty()) {
    barcode_ = item.get_barcode();
  }
  if(place_.isEmpty()) {
    place_ = item.get_place();
  }
  if(description_.isEmpty()) {
    description_ = item.get_description();
  }
  for(auto const& val : item.get_values()) {
    auto it = values_.find(val.first);
    if(it == values_.end() || it->second.isEmpty()) {
      values_[val.first] = val.second;
    }
  }
  for(auto const& lval : item.get_list_values()) {
    auto it = list_values_.find(lval.first);
    if(it == list_values_.end() || it->second.isEmpty()) {
      list_values_[lval.first] = lval.second;
    }
  }
  if(thumbnail_id_ < 0) {
    thumbnail_id_ = item.get_thumbnail_id();
  }
  if(!is_school_item_ && item.get_is_school_item()) {
    is_school_item_ = true;
  }
  if(subject_.isEmpty()) {
    subject_ = item.get_subject();
  }
  if(academic_year_.empty()) {
    academic_year_ = item.get_academic_year();
  }
  if(type_.isEmpty()) {
    type_ = item.get_type();
  }
  if(signature_top_.isEmpty()) {
    signature_top_ = item.get_signature_top();
  }
  if(signature_bottom_.isEmpty()) {
    signature_bottom_ = item.get_signature_bottom();
  }
  if(keywords_.empty()) {
    keywords_ = item.get_keywords();
  }
  return *this;
}


/// Converts academic year range vector to a comma-separated string.
/**
   @param ays vector of (first, last) academic year range pairs.
   @return comma-separated string representation of the year ranges.
*/
QString colman::academic_years_to_string(IntPairVec const& ays)
{
  QStringList parts;
  for(auto const& year : ays) {
    if(year.first == year.second) {
      parts += QString::number(year.first);
    }
    else {
      parts += QString("%1-%2").arg(year.first).arg(year.second);
    }
  }
  return parts.join(',');
}


/// Returns true if any range in \a ays overlaps the grade span [from, to].
/**
   @param ays  academic year ranges of an item
   @param from lowest grade of the wanted span (inclusive)
   @param to   highest grade of the wanted span (inclusive)
   @return true when at least one range intersects [from, to].
*/
bool colman::academic_years_overlap(IntPairVec const& ays, int from, int to)
{
  for(auto const& year : ays) {
    int const lo = std::min(year.first, year.second);
    int const hi = std::max(year.first, year.second);
    if(lo <= to && hi >= from) {
      return true;
    }
  }
  return false;
}


/// Returns true if every range in \a ays is well-formed (first <= second).
/**
   An empty \a ays is valid (item without a grade level).  No upper bound is
   imposed on the grade numbers; only the order within each range is checked.
   @param ays academic year ranges of an item
   @return true when no range has its first grade above its second.
*/
bool colman::academic_years_valid(IntPairVec const& ays)
{
  for(auto const& year : ays) {
    if(year.first > year.second) {
      return false;
    }
  }
  return true;
}


/// Parses a comma-separated academic year range string into a vector.
/**
   @param str comma-separated string of year ranges (e.g. "1,3-5").
   @return vector of (first, last) academic year range pairs.
*/
colman::IntPairVec colman::string_to_academic_years(QString const& str)
{
  IntPairVec retval;
  QStringList const ranges = str.split(',');
  retval.reserve(ranges.size());
  for(auto const& range : ranges) {
    if(range.size() > 0) {
      QStringList const years = range.split('-');
      if(years.size() == 1) {
        int const x = years[0].toInt();
        retval.push_back(std::make_pair(x, x));
      }
      else if(years.size() == 2) {
        int const x = years[0].toInt();
        int const y = years[1].toInt();
        retval.push_back(std::make_pair(x, y));
      }
    }
  }
  return retval;
}


/// Converts a keyword list to a semicolon-separated string.
/**
   @param keywords list of keyword strings.
   @return semicolon-separated string of all keywords.
*/
QString colman::keywords_to_string(QStringList const& keywords)
{
  return keywords.join(';');
}


/// Parses a semicolon-separated string into a keyword list.
/**
   @param str semicolon-separated string of keywords.
   @return list of non-empty keyword strings.
*/
QStringList colman::string_to_keywords(QString const& str)
{
  if(str.isEmpty()) {
    return {};
  }
  return str.split(';', Qt::SkipEmptyParts);
}


/**
   Sets the per-item reservation override.
   @param v -1=inherit, 0=not allowed, 1=allowed
*/
void colman::Item::set_allow_reservations(int v)
{
  allow_reservations_ = v;
}


/**
   Sets the per-item reservation period override.
   @param v -1=inherit, >=0=override in days
*/
void colman::Item::set_reservation_period_days(int v)
{
  reservation_period_days_ = v;
}


/**
   Sets the optimistic-lock version counter.
   @param version value from the items.version database column
*/
void colman::Item::set_version(int version)
{
  version_ = version;
}


/**
   Used by the item edit page ("Fill in from the internet"): a lookup may
   add what is missing but must never overwrite data the user entered.
   @param current        the item as it is now
   @param found          the lookup result
   @param take_thumbnail true when the current cover is missing
   @return the completed item
*/
colman::Item colman::complete_item(Item current, Item const& found,
                                   bool take_thumbnail)
{
  current.set_name(fill_blank(current.get_name(), found.get_name()));
  current.set_place(fill_blank(current.get_place(), found.get_place()));
  current.set_description(
      fill_blank(current.get_description(), found.get_description()));
  if(all_blank(current.get_keywords())) {
    current.set_keywords(found.get_keywords());
  }
  for(auto const& value : found.get_values()) {
    auto const& have = current.get_values();
    auto const it = have.find(value.first);
    if(it == have.end() || it->second.trimmed().isEmpty()) {
      current.set_value(value.first, value.second);
    }
  }
  for(auto const& list : found.get_list_values()) {
    auto const& have = current.get_list_values();
    auto const it = have.find(list.first);
    if(it == have.end() || all_blank(it->second)) {
      current.set_list_value(list.first, list.second);
    }
  }
  if(take_thumbnail && found.get_thumbnail_id() > 0) {
    current.set_thumbnail_id(found.get_thumbnail_id());
  }
  return current;
}

/* EOF */
