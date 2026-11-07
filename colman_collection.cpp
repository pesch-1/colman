// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Collection.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_collection.hpp"

/**
   Sets the collection name.
   @param name human-readable name of the collection.
*/
void colman::Collection::set_name(QString const& name)
{
  name_ = name;
}


/**
   Sets the organisation name.
   @param organization name of the organisation that owns the collection.
*/
void colman::Collection::set_organization(QString const& organization)
{
  organization_ = organization;
}


/**
   Sets the responsible person.
   @param responsible name of the responsible person.
*/
void colman::Collection::set_responsible(QString const& responsible)
{
  responsible_ = responsible;
}


/// Loads the logo from raw image data and stores the bytes for printing.
/**
   @param arr  raw image data.
   @param type ignored; format is auto-detected from the data.
*/
void colman::Collection::
    load_logo_from_data(QByteArray const& arr, char const* /*type*/)
{
  logo_raw_ = arr;
}


/**
   Loads the logo from raw bytes, storing them for high-resolution SVG
   printing and rasterising into the display pixmap.
   @param raw raw image data (SVG, PNG, JPEG, …).
*/
void colman::Collection::set_logo_raw(QByteArray const& raw)
{
  logo_raw_ = raw;
}


/**
   Returns true if the stored raw logo bytes contain SVG data.
*/
bool colman::Collection::has_svg_logo() const
{
  QByteArray const t = logo_raw_.trimmed();
  return t.startsWith("<?xml") || t.startsWith("<svg");
}


/**
   Sets whether a default lending period is used.
   @param use_lending_period true to enforce a default lending period.
*/
void colman::Collection::set_use_lending_period(bool use_lending_period)
{
  use_lending_period_ = use_lending_period;
}


/**
   Sets the years component of the default lending period.
   @param lending_period_years years component of the default lending period.
*/
void colman::Collection::set_lending_period_years(int lending_period_years)
{
  lending_period_years_ = lending_period_years;
}


/**
   Sets the months component of the default lending period.
   @param lending_period_months months component of the default lending period.
*/
void colman::Collection::set_lending_period_months(int lending_period_months)
{
  lending_period_months_ = lending_period_months;
}


/**
   Sets the days component of the default lending period.
   @param lending_period_days days component of the default lending period.
*/
void colman::Collection::set_lending_period_days(int lending_period_days)
{
  lending_period_days_ = lending_period_days;
}


/**
   Sets whether reservations are allowed.
   @param allow_reservations true to allow reservations.
*/
void colman::Collection::set_allow_reservations(bool allow_reservations)
{
  allow_reservations_ = allow_reservations;
}


/**
   Sets the default reservation period in days.
   @param reservation_period_days default reservation period in days.
*/
void colman::Collection::
    set_reservation_period_days(int reservation_period_days)
{
  reservation_period_days_ = reservation_period_days;
}


/**
   Sets the collection-specific barcode prefix.
   @param barcode_prefix collection-specific barcode prefix string.
*/
void colman::Collection::set_barcode_prefix(QString const& barcode_prefix)
{
  barcode_prefix_ = barcode_prefix;
}


/**
   Sets the data format version string.
   @param data_version data format version string.
*/
void colman::Collection::set_data_version(QString const& data_version)
{
  data_version_ = data_version;
}


/**
   Sets the highest person id used so far.
   @param max_person_id highest person id used so far.
*/
void colman::Collection::set_max_person_id(int max_person_id)
{
  max_person_id_ = max_person_id;
}


/**
   Sets the default item type id.
   @param default_item_type database id of the default item type.
*/
void colman::Collection::set_default_item_type(int default_item_type)
{
  default_item_type_ = default_item_type;
}


/**
   Adds the default lending period (years, months, days) to a date.
   The period is added regardless of get_use_lending_period(); callers
   decide whether a default due date applies at all.
   @param from start date, usually today.
   @return \a from shifted by the default lending period.
*/
QDate colman::Collection::add_lending_period(QDate const& from) const
{
  return from.addYears(lending_period_years_)
      .addMonths(lending_period_months_)
      .addDays(lending_period_days_);
}


/**
   Sets the default value for is_school_item.
   @param default_is_school_item true if newly added items are school items by
   default.
*/
void colman::Collection::set_default_is_school_item(bool default_is_school_item)
{
  default_is_school_item_ = default_is_school_item;
}


/// Sets the ID card layout.
/**
   @param layout the new ID card layout.
*/
void colman::Collection::set_id_card_layout(IdCardLayout const& layout)
{
  id_card_layout_ = layout;
}


/// Sets the item card layout.
/**
   @param layout the new item card layout.
*/
void colman::Collection::set_item_card_layout(IdCardLayout const& layout)
{
  item_card_layout_ = layout;
}


/**
   Sets the signature label layout.
   @param layout the new signature label layout.
*/
void colman::Collection::set_sig_label_layout(
    SignatureLabelLayout const& layout)
{
  sig_label_layout_ = layout;
}


/**
   Sets the human-readable name used for LAN discovery (mDNS).
   @param name display name announced on the local network.
*/
void colman::Collection::set_server_name(QString const& name)
{
  server_name_ = name;
}


namespace
{

  /// Computes the weighted modulo-10 check digit of format 1 barcodes.
  /**
     Sums each character's digit value weighted by its 1-based position, then
     reduces modulo 10.  @p s must NOT include the check digit itself.
     Only used to verify legacy (format 1) barcodes.
     @param s barcode string without trailing check digit.
     @return check digit value in [0, 9].
  */
  int barcode_check_digit(QString const& s)
  {
    int sum = 0;
    int const n = s.size();
    for(int i = 0; i < n; ++i) {
      sum += (i + 1) * s[i].digitValue();
    }
    return sum % 10;
  }


  /// Computes the GTIN check digit (EAN-8/UPC-A/EAN-13/GTIN-14).
  /**
     Weights the digits 3, 1, 3, 1, ... starting from the rightmost one, so
     the same algorithm applies to all GTIN lengths.
     @param s digit string without trailing check digit.
     @return check digit value in [0, 9].
  */
  int gtin_check_digit(QString const& s)
  {
    int sum = 0;
    int const n = s.size();
    for(int i = 0; i < n; ++i) {
      int const weight = (i % 2 == 0) ? 3 : 1;
      sum += weight * s[n - 1 - i].digitValue();
    }
    return ((10 - sum % 10) % 10 + 10) % 10;
  }


  /// Computes the check digit of format 2 barcodes.
  /**
     The GTIN check digit shifted by 5.  As it always differs from the GTIN
     check digit, a format 2 barcode is never a valid EAN-8, UPC-A, EAN-13
     (ISBN-13) or GTIN-14, whatever its length.  Like the GTIN check digit
     it detects every single-digit error.
     @param s barcode string without trailing check digit.
     @return check digit value in [0, 9].
  */
  int barcode_check_digit_v2(QString const& s)
  {
    return (gtin_check_digit(s) + 5) % 10;
  }


  /// Appends the check digit of format 2 to @p body.
  /**
     @param body barcode string without check digit.
     @return complete barcode.
  */
  QString with_check_digit_v2(QString const& body)
  {
    return body + QChar('0' + barcode_check_digit_v2(body));
  }


  /// Returns @p n as decimal string preceded by its number of digits.
  /**
     @param n non-negative number.
     @return e.g. "41027" for 1027.
  */
  QString length_prefixed(int n)
  {
    QString const num = QString::number(n);
    return QString::number(num.size()) + num;
  }


  /// Parses a length-prefixed number at @p pos of @p barcode.
  /**
     @param barcode barcode string.
     @param pos position of the length digit; advanced behind the number.
     @param value receives the number.
     @return true if a length digit in [1, 9] and that many digits follow.
  */
  bool parse_length_prefixed(QString const& barcode, int& pos, int& value)
  {
    if(pos >= barcode.size()) {
      return false;
    }
    int const len = barcode[pos].digitValue();
    if(len < 1 || pos + 1 + len > barcode.size()) {
      return false;
    }
    bool ok = false;
    value = barcode.mid(pos + 1, len).toInt(&ok);
    pos += 1 + len;
    return ok;
  }


  /// Verifies the trailing check digit of @p barcode.
  /**
     @param barcode complete barcode string.
     @param pos position of the check digit (must be the last character).
     @param format barcode format (1 = legacy, 2 = current).
     @return true if the check digit is the last character and matches.
  */
  bool check_digit_ok(QString const& barcode, int pos, int format)
  {
    if(pos + 1 != barcode.size()) {
      return false;
    }
    QString const body = barcode.left(pos);
    int const expected = (format == 1) ? barcode_check_digit(body)
                                       : barcode_check_digit_v2(body);
    return barcode[pos].digitValue() == expected;
  }


  /// Determines format and kind of a barcode behind the collection prefix.
  /**
     Format 1 item: t...; format 1 person: 0 i...; format 2 person: 000 i...;
     format 2 item: 00 t... (t, i: number of digits, always >= 1).
     @param barcode barcode string.
     @param prefix collection barcode prefix.
     @param format receives 1 or 2.
     @param pos receives the position of the first length digit.
     @return true for a person barcode, false for an item barcode; @p format
             is 0 if @p barcode does not start like a colman barcode.
  */
  bool classify_barcode(QString const& barcode, QString const& prefix,
                        int& format, int& pos)
  {
    format = 0;
    pos = prefix.size();
    if(!barcode.startsWith(prefix) || barcode.size() < pos + 3) {
      return false;
    }
    int const d1 = barcode[pos].digitValue();
    int const d2 = barcode[pos + 1].digitValue();
    int const d3 = barcode[pos + 2].digitValue();
    if(d1 > 0) {
      format = 1;
      return false;
    }
    if(d1 == 0 && d2 > 0) {
      format = 1;
      pos += 1;
      return true;
    }
    if(d1 == 0 && d2 == 0 && d3 == 0) {
      format = 2;
      pos += 3;
      return true;
    }
    if(d1 == 0 && d2 == 0 && d3 > 0) {
      format = 2;
      pos += 2;
      return false;
    }
    return false;
  }

} // namespace


/// Generates the barcode prefix from collection name and organisation.
/**
   @param name collection name.
   @param organization organisation name.
   @return five-digit zero-padded checksum string.
*/
QString colman::generate_barcode_prefix(QString const& name,
                                        QString const& organization)
{
  QString tmp(name);
  tmp += organization;
  quint16 const check = qChecksum(QByteArrayView(tmp.toUtf8()));
  QString retval;
  retval = QString("%1").arg(check, 5, 10, QChar('0'));
  return retval;
}


/// Computes the barcode string for the given item.
/**
   Generates format 2: prefix, "00", number of digits of the type id, type
   id, number of digits of the item id, item id, check digit.  The marker
   "00" never occurs in format 1 barcodes, and the check digit makes the
   barcode an invalid GTIN, so it cannot be mistaken for an ISBN or EAN.
   @param collection the collection whose prefix to use.
   @param type_id item type id.
   @param item_id item database id.
   @return encoded barcode string with check digit.
*/
QString colman::compute_item_barcode(Collection const& collection,
                                     int type_id, int item_id)
{
  return with_check_digit_v2(collection.get_barcode_prefix() + "00" +
                             length_prefixed(type_id) +
                             length_prefixed(item_id));
}


/// Parses an item barcode string and extracts type_id and item_id.
/**
   Accepts format 1 (legacy) and format 2 barcodes.
   @param collection the collection whose prefix to use.
   @param barcode barcode string to parse.
   @param type_id receives the extracted item type id.
   @param item_id receives the extracted item database id.
   @return true if the barcode was successfully parsed.
*/
bool colman::parse_item_barcode(Collection const& collection,
                                QString const& barcode,
                                int& type_id, int& item_id)
{
  int format = 0;
  int pos = 0;
  if(classify_barcode(barcode, collection.get_barcode_prefix(), format,
                      pos) ||
     format == 0)
  {
    return false;
  }
  int tid = 0;
  int iid = 0;
  if(parse_length_prefixed(barcode, pos, tid) &&
     parse_length_prefixed(barcode, pos, iid) &&
     check_digit_ok(barcode, pos, format))
  {
    type_id = tid;
    item_id = iid;
    return true;
  }
  return false;
}


/// Returns true if barcode is a valid item barcode for collection.
/**
   @param collection the collection whose prefix to use.
   @param barcode barcode string to test.
   @return true if barcode is a valid item barcode for collection.
*/
bool colman::is_item_barcode(Collection const& collection,
                             QString const& barcode)
{
  int tmp1 = 0, tmp2 = 0;
  return parse_item_barcode(collection, barcode, tmp1, tmp2);
}


/**
   Computes the person barcode for the given person id.
   Generates format 2: prefix, "000", number of digits of the person id,
   person id, check digit (see compute_item_barcode()).
   @param collection the collection whose prefix to use.
   @param person_id person database id.
   @return encoded barcode string with check digit.
*/
QString colman::compute_person_barcode(Collection const& collection,
                                       int person_id)
{
  return with_check_digit_v2(collection.get_barcode_prefix() + "000" +
                             length_prefixed(person_id));
}


/**
   Parses a person barcode string and extracts person_id.
   Accepts format 1 (legacy) and format 2 barcodes.
   @param collection the collection whose prefix to use.
   @param barcode barcode string to parse.
   @param person_id receives the extracted person database id.
   @return true if the barcode was successfully parsed.
*/
bool colman::parse_person_barcode(Collection const& collection,
                                  QString const& barcode, int& person_id)
{
  int format = 0;
  int pos = 0;
  if(!classify_barcode(barcode, collection.get_barcode_prefix(), format,
                       pos))
  {
    return false;
  }
  int pid = 0;
  if(parse_length_prefixed(barcode, pos, pid) &&
     check_digit_ok(barcode, pos, format))
  {
    person_id = pid;
    return true;
  }
  return false;
}


/**
   Returns true if barcode is a valid person barcode for collection.
   @param collection the collection whose prefix to use.
   @param barcode barcode string to test.
   @return true if barcode is a valid person barcode for collection.
*/
bool colman::is_person_barcode(Collection const& collection,
                               QString const& barcode)
{
  int pid = 0;
  return parse_person_barcode(collection, barcode, pid);
}

/* EOF */
