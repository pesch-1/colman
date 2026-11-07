// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Type.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TYPE_HPP
#  define COLMAN_TYPE_HPP

#  ifndef COLMAN_MEDIA_KIND_HPP
#    include "colman_media_kind.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QVARIANTMAP
#    include <QVariantMap>
#    define SYSTEM_QVARIANTMAP
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_UTILITY
#    include <utility>
#    define SYSTEM_UTILITY
#  endif

namespace colman
{
  /// Enum for the data type of a custom item attribute.
  enum struct DataType {
    String = 0,      ///< strings that may not fit into one line (QPlainTextEdit)
    ShortString = 1, ///< should fit into one line (QLineEdit)
    Date = 2,        ///< date value
    Time = 3,        ///< time value
    Integer = 4,     ///< integer value
    Float = 5,       ///< floating-point value
    Duration = 6,    ///< duration value
    Table = 7,       ///< table of values
    IntegerRanges = 8 ///< list of integer ranges
  };


  /// Combines a DataType with optional extra information.
  struct DataTypeInfo
  {
    DataType type_;   ///< the data type
    QVariantMap info_; ///< optional type-specific extra information
  };

  /// A key name paired with its DataTypeInfo.
  using KeyInfo = std::pair<QString, DataTypeInfo>;
  /// Ordered list of key descriptions for a Type.
  using KeyVector = std::vector<KeyInfo>;


  /// Describes a custom item type with its attribute definitions.
  /**
     Each item in the collection has a Type that defines which custom attributes
     (key-value pairs) it carries.  The Type stores the display name and the
     ordered list of attribute keys together with their DataTypeInfo.
     The SQL table name used for persistence is an internal storage detail
     managed by the database layer and is not part of this domain object.
  */
  class Type
  {
  public:
    /// Default constructor.
    Type();
    /// Constructs a Type with the given name and key list.
    Type(QString const& name, KeyVector const& keys);

    inline int get_id() const { return id_; }         ///< Returns the database id.
    inline QString const& get_name() const { return name_; } ///< Returns the display name.
    inline KeyVector const& get_keys() const { return keys_; } ///< Returns the attribute key list.
    inline MediaKind get_media_kind() const { return media_kind_; } ///< Returns the search media kind.

    /// Resets all fields to empty.
    void clear();
    /// Removes all attribute keys.
    void clear_keys();
    /// Sets the database id.
    void set_id(int id);
    /// Sets the display name.
    void set_name(QString const& name);
    /// Replaces the attribute key list.
    void set_keys(KeyVector const& keys);
    /// Sets the search media kind.
    void set_media_kind(MediaKind kind);
    /// Appends a key to the attribute list.
    void add_key(KeyInfo const& key);
    /// Appends a key and its DataTypeInfo to the attribute list.
    void add_key(QString const& key, DataTypeInfo const& info);

  private:
    int id_ = 0;       ///< database id
    QString name_;   ///< display name
    KeyVector keys_; ///< ordered attribute key list
    MediaKind media_kind_ = MediaKind::Unknown; ///< search media kind
  };


  /// Converts a QVariant (encoded as list of key, type-int, info-map triples) to a KeyVector.
  KeyVector variant_to_key_vector(QVariant const& v);
  /// Returns true if \a info declares a repeatable (multi-value) field.
  bool is_repeatable(DataTypeInfo const& info);

} // namespace colman

#endif /* #ifndef COLMAN_TYPE_HPP */

/* EOF */
