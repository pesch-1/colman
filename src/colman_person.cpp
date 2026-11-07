// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Person.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_person.hpp"


/**
   Resets all members to empty.
*/
void colman::Person::clear()
{
  lastname_.clear();
  firstname_.clear();
  title_.clear();
  school_class_.clear();
  barcode_.clear();
  emails_.clear();
  telephone_.clear();
  street_.clear();
  zip_.clear();
  city_.clear();
  country_.clear();
  remarks_.clear();
  external_id_.clear();
  username_.clear();
  thumbnail_id_ = -1;
  id_ = 0;
  data_collector_id_ = -1;
  version_ = 1;
}


/**
   Sets the database id.
   @param id primary key from the persons table.
*/
void colman::Person::set_id(int id)
{
  id_ = id;
}


/**
   Sets the family name.
   @param lastname family name of the person.
*/
void colman::Person::set_lastname(QString const& lastname)
{
  lastname_ = lastname;
}


/**
   Sets the given name.
   @param firstname given name of the person.
*/
void colman::Person::set_firstname(QString const& firstname)
{
  firstname_ = firstname;
}


/**
   Sets the academic/honorific title.
   @param title academic or honorific title; may be empty.
*/
void colman::Person::set_title(QString const& title)
{
  title_ = title;
}


/**
   Sets the school class.  Leading and trailing whitespace is removed so
   that the name matches the class list, which stores trimmed names.
   @param school_class school class designation, e.g. "5a"; may be empty.
*/
void colman::Person::set_school_class(QString const& school_class)
{
  school_class_ = school_class.trimmed();
}


/**
   Sets the barcode string.
   @param barcode barcode string for the person ID card.
*/
void colman::Person::set_barcode(QString const& barcode)
{
  barcode_ = barcode;
}


/**
   Sets the e-mail addresses.
   @param emails comma-separated e-mail addresses; may be empty.
*/
void colman::Person::set_emails(QString const& emails)
{
  emails_ = emails;
}


/**
   Sets the telephone number.
   @param telephone telephone number; may be empty.
*/
void colman::Person::set_telephone(QString const& telephone)
{
  telephone_ = telephone;
}


/**
   Sets the street address.
   @param street street address; may be empty.
*/
void colman::Person::set_street(QString const& street)
{
  street_ = street;
}


/**
   Sets the postal code.
   @param zip postal code; may be empty.
*/
void colman::Person::set_zip(QString const& zip)
{
  zip_ = zip;
}


/**
   Sets the city.
   @param city city name; may be empty.
*/
void colman::Person::set_city(QString const& city)
{
  city_ = city;
}


/**
   Sets the country.
   @param country country name; may be empty.
*/
void colman::Person::set_country(QString const& country)
{
  country_ = country;
}


/**
   Sets the remarks.
   @param remarks optional user-visible remarks.
*/
void colman::Person::set_remarks(QString const& remarks)
{
  remarks_ = remarks;
}


/**
   Sets the thumbnail database id.
   @param thumbnail_id database id of the person's thumbnail image, or -1.
*/
void colman::Person::set_thumbnail_id(int thumbnail_id)
{
  thumbnail_id_ = thumbnail_id;
}


/**
   Sets the data collector person id.
   @param data_collector_id database id of the data collector, or -1.
*/
void colman::Person::set_data_collector_id(int data_collector_id)
{
  data_collector_id_ = data_collector_id;
}


/**
   Sets the Authentik external UUID.
   @param external_id UUID string from the Authentik JWT sub claim.
*/
void colman::Person::set_external_id(QString const& external_id)
{
  external_id_ = external_id;
}


/**
   Sets the login username.
   @param username username from the credentials table (empty if none).
*/
void colman::Person::set_username(QString const& username)
{
  username_ = username;
}


/**
   Sets the optimistic-lock version counter.
   @param version value from the `version` column in the persons table.
*/
void colman::Person::set_version(int version)
{
  version_ = version;
}


/* EOF */
