// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Type.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_type.hpp"

/**
   Default constructor.
*/
colman::Type::Type()
{
}


/**
   Constructs a Type with the given name and key list.
   @param name human-readable type name.
   @param keys list of attribute key descriptors.
*/
colman::Type::Type(QString const& name, KeyVector const& keys)
    : name_(name),
      keys_(keys)
{
}


/**
   Resets all fields to empty.
*/
void colman::Type::clear()
{
  id_ = 0;
  name_.clear();
  keys_.clear();
  media_kind_ = MediaKind::Unknown;
}


/**
   Removes all attribute keys.
*/
void colman::Type::clear_keys()
{
  keys_.clear();
}


/**
   Sets the database id.
   @param id primary key from the types table.
*/
void colman::Type::set_id(int id)
{
  id_ = id;
}


/**
   Sets the display name.
   @param name human-readable type name.
*/
void colman::Type::set_name(QString const& name)
{
  name_ = name;
}


/**
   Replaces the attribute key list.
   @param keys new list of attribute key descriptors.
*/
void colman::Type::set_keys(KeyVector const& keys)
{
  keys_ = keys;
}


/**
   Sets the search media kind.
   @param kind media kind used to map this type to an internet search category.
*/
void colman::Type::set_media_kind(MediaKind kind)
{
  media_kind_ = kind;
}


/**
   Appends a key to the attribute list.
   @param key attribute key descriptor to append.
*/
void colman::Type::add_key(KeyInfo const& key)
{
  keys_.push_back(key);
}


/**
   Appends a key and its DataTypeInfo to the attribute list.
   @param key  attribute key name.
   @param info data type information for the key.
*/
void colman::Type::add_key(QString const& key, DataTypeInfo const& info)
{
  keys_.push_back(KeyInfo(key, info));
}


/**
   Converts a QVariant (as stored in Qt's settings or undo stack) back to a
   KeyVector.  The variant must encode alternating key, type-int, info-map
   triples as produced by the inverse function.
   @param v variant encoding a sequence of (key, DataType int, QVariantMap) triples.
   @return the reconstructed KeyVector; empty if @p v is malformed.
*/
colman::KeyVector colman::variant_to_key_vector(QVariant const& v)
{
  colman::KeyVector keys;
  QList<QVariant> const list = v.toList();
  auto it = list.begin();
  while(it != list.end()) {
    QString const key = it->toString();
    if(++it != list.end()) {
      int const type = it->toInt();
      if(++it != list.end()) {
        QVariantMap const info = it->toMap();
        ++it;
        if(type < 0 ||
           type > static_cast<int>(colman::DataType::IntegerRanges)) {
          continue;
        }
        colman::DataTypeInfo const dinfo{colman::DataType(type), info};
        keys.push_back(colman::KeyInfo(key, dinfo));
      }
    }
  }
  return keys;
}


/**
   Returns true if \a info declares a repeatable (multi-value) field.
   @param info data type info to check.
   @return true if the field may hold multiple values.
*/
bool colman::is_repeatable(DataTypeInfo const& info)
{
  return info.info_.value("is_repeatable", false).toBool();
}


/* EOF */
