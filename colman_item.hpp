// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Item.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ITEM_HPP
#  define COLMAN_ITEM_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif
#  ifndef SYSTEM_MAP
#    include <map>
#    define SYSTEM_MAP
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif

namespace colman
{
  using StringMap = std::map<QString, QString>;
  using StringListMap = std::map<QString, QStringList>;
  using IntPairVec = std::vector<std::pair<int, int>>;

  /// Plain data class representing a collection item (e.g. a book).
  /**
     Stores all properties of a collection item, including type-specific
     key-value pairs, barcode, thumbnail, and optional school-item fields.
  */
  struct Item
  {
    inline int get_type_id() const { return type_id_; }       ///< Returns the item type id.
    inline QString const& get_name() const { return name_; }  ///< Returns the item name.
    inline QString const& get_barcode() const { return barcode_; } ///< Returns the barcode string.
    inline QString const& get_place() const { return place_; }     ///< Returns the storage place.
    inline QString const& get_description() const { return description_; } ///< Returns the description.
    inline StringMap const& get_values() const { return values_; } ///< Returns type-specific key-value pairs.
    inline StringListMap const& get_list_values() const { return list_values_; } ///< Returns repeatable field value lists.
    inline int get_thumbnail_id() const { return thumbnail_id_; }  ///< Returns the thumbnail db id.
    inline bool get_is_school_item() const { return is_school_item_; } ///< Returns true if this is a school item.
    inline QString const& get_subject() const { return subject_; } ///< Returns the school subject.
    inline IntPairVec const& get_academic_year() const { return academic_year_; } ///< Returns academic year ranges.
    inline QString const& get_type() const { return type_; }       ///< Returns the item subtype name.
    inline int get_data_collector_id() const { return data_collector_id_; } ///< Returns the data collector person id, or -1.
    inline QString const& get_signature_top() const { return signature_top_; } ///< Returns the top spine signature label.
    inline QString const& get_signature_bottom() const { return signature_bottom_; } ///< Returns the bottom spine signature label.
    inline QStringList const& get_keywords() const { return keywords_; } ///< Returns the search keywords.

    /// Resets all members to initial values.
    void clear();
    /// Merges \a item into this object (this object has precedence over \a item).
    Item& operator+=(Item const& item);
    /// Sets the item type id.
    void set_type_id(int type_id);
    /// Sets the item name.
    void set_name(QString const& name);
    /// Sets the barcode string.
    void set_barcode(QString const& barcode);
    /// Sets the storage place.
    void set_place(QString const& place);
    /// Sets the description.
    void set_description(QString const& description);
    /// Replaces all type-specific key-value pairs.
    void set_values(StringMap const& values);
    /// Sets a single type-specific key-value pair.
    void set_value(QString const& key, QString const& value);
    /// Appends \a value to an existing key-value pair.
    void append_to_value(QString const& key, QString const& value);
    /// Swaps the internal key-value map with \a values.
    void swap_values(StringMap& values);
    /// Replaces all repeatable field value lists.
    void set_list_values(StringListMap const& list_values);
    /// Sets the value list for a single repeatable field.
    void set_list_value(QString const& key, QStringList const& values);
    /// Appends a single string to the list for \a key.
    void append_list_value(QString const& key, QString const& value);
    /// Swaps the internal list-value map with \a list_values.
    void swap_list_values(StringListMap& list_values);
    /// Sets the thumbnail database id.
    void set_thumbnail_id(int thumbnail_id);
    /// Sets whether this item is a school item.
    void set_is_school_item(bool is_school_item);
    /// Sets the school subject.
    void set_subject(QString const& subject);
    /// Sets the academic year ranges.
    void set_academic_year(IntPairVec const& academic_year);
    /// Clears the academic year ranges.
    void clear_academic_year();
    /// Sets the item subtype name (e.g. "Lehrerband", "Arbeitsheft").
    void set_type(QString const& type);
    /// Sets the data collector person id (-1 if unknown).
    void set_data_collector_id(int data_collector_id);
    /// Sets the top spine signature label.
    void set_signature_top(QString const& signature_top);
    /// Sets the bottom spine signature label.
    void set_signature_bottom(QString const& signature_bottom);
    /// Sets the search keywords.
    void set_keywords(QStringList const& keywords);
    /// Sets the per-item reservation override (-1=inherit, 0=not allowed, 1=allowed).
    void set_allow_reservations(int v);
    /// Returns the per-item reservation override (-1=inherit, 0=not allowed, 1=allowed).
    inline int get_allow_reservations() const { return allow_reservations_; }
    /// Sets the per-item reservation period in days (-1=inherit, >=0=days).
    void set_reservation_period_days(int v);
    /// Returns the per-item reservation period in days (-1=inherit, >=0=days).
    inline int get_reservation_period_days() const { return reservation_period_days_; }
    /// Returns the optimistic-lock version counter.
    inline int get_version() const { return version_; }
    /// Sets the optimistic-lock version counter.
    void set_version(int version);

  private:
    int type_id_ = -1;           ///< item type id, -1 if unset
    QString name_;             ///< item name
    QString barcode_;          ///< barcode string
    QString place_;            ///< storage place
    QString description_;      ///< description text
    StringMap values_;         ///< type-specific key-value pairs
    StringListMap list_values_; ///< repeatable field value lists
    int thumbnail_id_ = -1;      ///< thumbnail database id, -1 if unset
    bool is_school_item_ = false; ///< true if this is a school item
    QString subject_;          ///< school subject (school items only)
    IntPairVec academic_year_; ///< vector of academic year ranges
    QString type_;             ///< item subtype name (e.g. Lehrerband)
    int data_collector_id_ = -1; ///< person id of the data collector, -1 if unset
    QString signature_top_;    ///< top spine signature label
    QString signature_bottom_; ///< bottom spine signature label
    QStringList keywords_;     ///< search keywords
    int allow_reservations_ = -1; ///< -1=inherit, 0=not allowed, 1=allowed
    int reservation_period_days_ = -1; ///< -1=inherit, >=0=override in days
    int version_ = 1;            ///< optimistic-lock version counter (from items.version column)
  };

  /// Converts a vector of academic year ranges to a comma-separated string.
  QString academic_years_to_string(IntPairVec const& ays);
  /// Returns true if any range in \a ays overlaps the grade span [from, to].
  /**
     Used by the search filter.  An empty \a ays (item without a grade level)
     never overlaps; callers decide how to treat those items.
     @param ays  academic year ranges of an item
     @param from lowest grade of the wanted span (inclusive)
     @param to   highest grade of the wanted span (inclusive)
  */
  bool academic_years_overlap(IntPairVec const& ays, int from, int to);
  /// Returns true if every range in \a ays is well-formed (first <= second).
  /** An empty \a ays is valid (item without a grade level). */
  bool academic_years_valid(IntPairVec const& ays);
  /// Parses a comma-separated string of academic year ranges.
  IntPairVec string_to_academic_years(QString const& str);
  /// Converts a list of keywords to a semicolon-separated string.
  QString keywords_to_string(QStringList const& keywords);
  /// Parses a semicolon-separated string into a list of keywords.
  QStringList string_to_keywords(QString const& str);
  /// Prints an item's fields to stdout (for debugging).
  void print_item(Item const& item);

} // namespace colman

#endif /* #ifndef COLMAN_ITEM_HPP */

/* EOF */
