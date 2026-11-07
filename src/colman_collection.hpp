// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Collection.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_COLLECTION_HPP
#  define COLMAN_COLLECTION_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QDATE
#    include <QDate>
#    define SYSTEM_QDATE
#  endif
#  ifndef COLMAN_ID_CARD_LAYOUT_HPP
#    include "colman_id_card_layout.hpp"
#  endif
#  ifndef COLMAN_SIGNATURE_LABEL_LAYOUT_HPP
#    include "colman_signature_label_layout.hpp"
#  endif

namespace colman
{
  /// Stores collection-wide settings and metadata.
  /**
     Holds the library name, organisation, responsible person, logo,
     barcode prefix, default lending period, and other collection-level
     configuration values that are persisted in the SQLite database.
  */
  class Collection
  {
  public:
    inline QString const& get_name() const { return name_; }                ///< Returns the collection name.
    inline QString const& get_organization() const { return organization_; } ///< Returns the organisation name.
    inline QString const& get_responsible() const { return responsible_; }   ///< Returns the responsible person.
    inline QByteArray const& get_logo_raw() const { return logo_raw_; }      ///< Returns the raw logo file bytes (SVG or raster).
    /// Returns true if the logo was loaded from SVG data.
    bool has_svg_logo() const;
    inline bool get_use_lending_period() const { return use_lending_period_; } ///< Returns true if a default lending period is set.
    inline int get_lending_period_years() const { return lending_period_years_; } ///< Returns years of the default lending period.
    inline int get_lending_period_months() const { return lending_period_months_; } ///< Returns months of the default lending period.
    inline int get_lending_period_days() const { return lending_period_days_; } ///< Returns days of the default lending period.
    /// Returns \a from plus the default lending period.
    QDate add_lending_period(QDate const& from) const;
    inline bool get_allow_reservations() const { return allow_reservations_; } ///< Returns true if reservations are allowed.
    inline int get_reservation_period_days() const { return reservation_period_days_; } ///< Returns the default reservation period in days.
    inline QString const& get_barcode_prefix() const { return barcode_prefix_; } ///< Returns the collection-specific barcode prefix.
    inline QString const& get_data_version() const { return data_version_; }     ///< Returns the data format version string.
    inline int get_max_person_id() const { return max_person_id_; }              ///< Returns the highest person id used so far.
    inline int new_person_id() const { return max_person_id_ + 1; }              ///< Returns the next available person id.
    inline int get_default_item_type() const { return default_item_type_; }      ///< Returns the default item type id.
    inline bool get_default_is_school_item() const { return default_is_school_item_; } ///< Returns the default value for is_school_item.
    inline int get_grade_range_min() const { return grade_range_min_; } ///< Returns the minimum grade level.
    inline int get_grade_range_max() const { return grade_range_max_; } ///< Returns the maximum grade level.
    inline QString const& get_server_name() const { return server_name_; } ///< Returns the LAN discovery name.

    /// Sets the minimum grade level.
    inline void set_grade_range_min(int v) { grade_range_min_ = v; }
    /// Sets the maximum grade level.
    inline void set_grade_range_max(int v) { grade_range_max_ = v; }
    /// Sets the LAN discovery name.
    void set_server_name(QString const& name);

    /// Sets the collection name.
    void set_name(QString const& name);
    /// Sets the organisation name.
    void set_organization(QString const& organization);
    /// Sets the responsible person.
    void set_responsible(QString const& responsible);
    /// Loads the logo from raw image data and stores the bytes for high-res printing.
    /**
       Stores @p arr as raw bytes (used at print time), then rasterises
       into the display pixmap.  The format type parameter is ignored;
       format is auto-detected from the data.
       @param arr  raw image data (SVG, PNG, JPEG, …)
       @param type ignored; kept for API compatibility
    */
    void load_logo_from_data(QByteArray const& arr, char const* type);
    /// Loads the logo from raw bytes, auto-detecting the format.
    /**
       Stores @p raw for high-resolution SVG printing and rasterises it
       into the display pixmap.
       @param raw raw image data (SVG, PNG, JPEG, …)
    */
    void set_logo_raw(QByteArray const& raw);
    /// Sets whether a default lending period is used.
    void set_use_lending_period(bool use_lending_period);
    /// Sets the years component of the default lending period.
    void set_lending_period_years(int lending_period_years);
    /// Sets the months component of the default lending period.
    void set_lending_period_months(int lending_period_months);
    /// Sets the days component of the default lending period.
    void set_lending_period_days(int lending_period_days);
    /// Sets whether reservations are allowed.
    void set_allow_reservations(bool allow_reservations);
    /// Sets the default reservation period in days.
    void set_reservation_period_days(int reservation_period_days);
    /// Sets the collection-specific barcode prefix.
    void set_barcode_prefix(QString const& barcode_prefix);
    /// Sets the data format version string.
    void set_data_version(QString const& data_version);
    /// Sets the highest person id used so far.
    void set_max_person_id(int max_person_id);
    /// Sets the default item type id.
    void set_default_item_type(int default_item_type);
    /// Sets the default value for is_school_item.
    void set_default_is_school_item(bool default_is_school_item);
    /// Returns the stored ID card layout.
    inline IdCardLayout const& get_id_card_layout() const
    {
      return id_card_layout_;
    }
    /// Sets the ID card layout.
    void set_id_card_layout(IdCardLayout const& layout);
    /// Returns the stored item card layout.
    inline IdCardLayout const& get_item_card_layout() const
    {
      return item_card_layout_;
    }
    /// Sets the item card layout.
    void set_item_card_layout(IdCardLayout const& layout);
    /// Returns the stored signature label layout.
    inline SignatureLabelLayout const& get_sig_label_layout() const
    {
      return sig_label_layout_;
    }
    /// Sets the signature label layout.
    void set_sig_label_layout(SignatureLabelLayout const& layout);

  private:
    QString name_;                     ///< collection name
    QString organization_;             ///< organisation name
    QString responsible_;              ///< responsible person
    QByteArray logo_raw_;              ///< raw logo bytes (SVG or raster)
    bool use_lending_period_ = false;    ///< true if a default lending period is configured
    int lending_period_years_ = 0;       ///< default lending period: years
    int lending_period_months_ = 1;      ///< default lending period: months
    int lending_period_days_ = 0;        ///< default lending period: days
    bool allow_reservations_ = true;     ///< true if reservations are allowed
    int reservation_period_days_ = 14;   ///< default reservation period in days
    QString barcode_prefix_;           ///< collection-specific barcode prefix
    QString data_version_;             ///< data format version string
    int max_person_id_ = 0;              ///< highest person id used so far
    int default_item_type_ = -1;         ///< default item type id
    bool default_is_school_item_ = false; ///< default value for is_school_item
    int grade_range_min_ = 1;             ///< minimum grade level
    int grade_range_max_ = 13;            ///< maximum grade level
    QString server_name_;               ///< human-readable LAN discovery name
    IdCardLayout id_card_layout_;              ///< ID card layout (zero = not configured)
    IdCardLayout item_card_layout_;            ///< item card layout (zero = not configured)
    SignatureLabelLayout sig_label_layout_;    ///< signature label layout (zero = not configured)
  };

  /// Generates a 5-digit barcode prefix from the collection name and organisation.
  QString generate_barcode_prefix(QString const& name,
                                  QString const& organization);

  /// Computes the item barcode (format 2) for the given type and item ids.
  QString compute_item_barcode(Collection const& collection,
                               int type_id, int item_id);
  /// Parses an item barcode (format 1 or 2), extracts type_id and item_id.
  /**
     @return true if \a barcode is a valid item barcode for \a collection
  */
  bool parse_item_barcode(Collection const& collection,
                          QString const& barcode,
                          int& type_id, int& item_id);
  /// Returns true if \a barcode is a valid item barcode for \a collection.
  bool is_item_barcode(Collection const& collection,
                       QString const& barcode);

  /// Computes the person barcode (format 2) for the given person id.
  QString compute_person_barcode(Collection const& collection, int person_id);
  /// Parses a person barcode (format 1 or 2) and extracts person_id.
  /**
     @return true if \a barcode is a valid person barcode for \a collection
  */
  bool parse_person_barcode(Collection const& collection,
                            QString const& barcode,
                            int& person_id);
  /// Returns true if \a barcode is a valid person barcode for \a collection.
  bool is_person_barcode(Collection const& collection,
                         QString const& barcode);

} // namespace colman

#endif /* #ifndef COLMAN_COLLECTION_HPP */

/* EOF */
