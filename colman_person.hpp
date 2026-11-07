// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Person.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PERSON_HPP
#  define COLMAN_PERSON_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Plain data class representing a borrower (person).
  /**
     Stores all contact data of a borrower, including name, school class,
     address, e-mail, telephone, barcode, and a thumbnail reference.
  */
  class Person
  {
  public:
    /// Resets all fields to empty/default values.
    void clear();

    inline int get_id() const { return id_; }                         ///< Returns the database id.
    inline QString const& get_lastname() const { return lastname_; }  ///< Returns the last name.
    inline QString const& get_firstname() const { return firstname_; } ///< Returns the first name.
    inline QString const& get_title() const { return title_; }         ///< Returns the title (e.g. "Dr.").
    inline QString const& get_school_class() const { return school_class_; } ///< Returns the school class.
    inline QString const& get_barcode() const { return barcode_; }     ///< Returns the barcode string.
    inline QString const& get_emails() const { return emails_; }       ///< Returns e-mail address(es).
    inline QString const& get_telephone() const { return telephone_; } ///< Returns the telephone number.
    inline QString const& get_street() const { return street_; }       ///< Returns the street address.
    inline QString const& get_zip() const { return zip_; }             ///< Returns the postal code.
    inline QString const& get_city() const { return city_; }           ///< Returns the city.
    inline QString const& get_country() const { return country_; }     ///< Returns the country.
    inline QString const& get_remarks() const { return remarks_; }     ///< Returns remarks.
    inline int get_thumbnail_id() const { return thumbnail_id_; }      ///< Returns the thumbnail db id.
    inline int get_data_collector_id() const { return data_collector_id_; } ///< Returns the data collector person id, or -1.
    inline QString const& get_external_id() const { return external_id_; } ///< Returns the Authentik external UUID.
    inline QString const& get_username() const { return username_; } ///< Returns the login username (empty if none).
    inline int get_version() const { return version_; } ///< Returns the optimistic-lock version counter.

    /// Sets the database id.
    void set_id(int id);
    /// Sets the last name.
    void set_lastname(QString const& lastname);
    /// Sets the first name.
    void set_firstname(QString const& firstname);
    /// Sets the title (e.g. "Dr.").
    void set_title(QString const& title);
    /// Sets the school class.
    void set_school_class(QString const& school_class);
    /// Sets the barcode string.
    void set_barcode(QString const& barcode);
    /// Sets the e-mail address(es).
    void set_emails(QString const& emails);
    /// Sets the telephone number.
    void set_telephone(QString const& telephone);
    /// Sets the street address.
    void set_street(QString const& street);
    /// Sets the postal code.
    void set_zip(QString const& zip);
    /// Sets the city.
    void set_city(QString const& city);
    /// Sets the country.
    void set_country(QString const& country);
    /// Sets remarks.
    void set_remarks(QString const& remarks);
    /// Sets the thumbnail database id.
    void set_thumbnail_id(int thumbnail_id);
    /// Sets the data collector person id (-1 if unknown).
    void set_data_collector_id(int data_collector_id);
    /// Sets the Authentik external UUID.
    void set_external_id(QString const& external_id);
    /// Sets the login username.
    void set_username(QString const& username);
    /// Sets the optimistic-lock version counter.
    void set_version(int version);

  private:
    QString lastname_;      ///< last name
    QString firstname_;     ///< first name
    QString title_;         ///< academic or other title
    QString school_class_;  ///< school class (e.g. "5a")
    QString barcode_;     ///< barcode string
    QString emails_;      ///< e-mail address(es)
    QString telephone_;   ///< telephone number
    QString street_;      ///< street address
    QString zip_;         ///< postal code
    QString city_;        ///< city
    QString country_;     ///< country
    QString remarks_;     ///< remarks
    QString external_id_; ///< Authentik subject UUID, empty if not linked
    QString username_;    ///< login username from credentials table, empty if none
    int id_ = 0;            ///< database id
    int thumbnail_id_ = -1; ///< thumbnail database id, -1 if unset
    int data_collector_id_ = -1; ///< person id of the data collector, -1 if unset
    int version_ = 1;      ///< optimistic-lock version counter (from persons.version column)
  };

} // namespace colman

#endif /* #ifndef COLMAN_PERSON_HPP */

/* EOF */
