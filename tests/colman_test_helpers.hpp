// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Shared test fixture helpers used by all Qt Test binaries.
/**
   Each test binary includes this header exactly once.  The helpers live in
   an anonymous namespace so that each translation unit gets its own static
   copy without linker conflicts.
   @file
   @author Christoph Pesch
 */

#ifndef COLMAN_TEST_HELPERS_HPP
#  define COLMAN_TEST_HELPERS_HPP

#  ifndef COLMAN_COLLECTION_HPP
#    include "colman_collection.hpp"
#  endif
#  ifndef COLMAN_PERSON_HPP
#    include "colman_person.hpp"
#  endif
#  ifndef COLMAN_TYPE_HPP
#    include "colman_type.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Builds a minimal Collection suitable for in-memory test databases.
  inline Collection make_test_collection()
  {
    Collection c;
    c.set_name("Test Library");
    c.set_organization("Test School");
    c.set_responsible("Test Person");
    c.set_barcode_prefix("TBIB0");
    c.set_data_version("1.0");
    c.set_max_person_id(0);
    c.set_use_lending_period(true);
    c.set_lending_period_years(0);
    c.set_lending_period_months(1);
    c.set_lending_period_days(0);
    return c;
  }

  /// Populates a Person with predictable test data.
  /**
     @param lastname  last name (default "Doe")
     @param firstname first name (default "John")
   */
  inline Person make_test_person(QString const& lastname = "Doe",
                                 QString const& firstname = "John")
  {
    Person p;
    p.set_lastname(lastname);
    p.set_firstname(firstname);
    p.set_title("Dr.");
    p.set_emails("test@example.com");
    p.set_telephone("555-1234");
    p.set_street("Main Street 1");
    p.set_zip("12345");
    p.set_city("Sample City");
    p.set_country("DE");
    p.set_remarks("Test Person");
    return p;
  }


  /// Builds a book Type with isbn and author short-string attributes.
  colman::Type make_book_type()
  {
    colman::KeyVector keys;
    keys.push_back({"isbn", {colman::DataType::ShortString, {}}});
    keys.push_back({"author", {colman::DataType::ShortString, {}}});
    return colman::Type("Book", keys);
  }

  /// Returns a Type with one repeatable ShortString field ("tags").
  colman::Type make_repeatable_type()
  {
    colman::KeyVector keys;
    QVariantMap rep_info;
    rep_info["is_repeatable"] = true;
    keys.push_back({"tags", {colman::DataType::ShortString, rep_info}});
    return colman::Type("Tagged", keys);
  }

  /// Appends the legacy (barcode format 1) check digit to @p body.
  inline QString with_legacy_check_digit(QString const& body)
  {
    auto sum = 0;
    for(auto i = 0; i < body.size(); ++i) {
      sum += (i + 1) * body[i].digitValue();
    }
    return body + QChar('0' + sum % 10);
  }

  /// Builds a legacy (barcode format 1) item barcode as printed by old
  /// colman versions.
  inline QString legacy_item_barcode(QString const& prefix, int type_id,
                                     int item_id)
  {
    auto const type_str = QString::number(type_id);
    auto const item_str = QString::number(item_id);
    return with_legacy_check_digit(
        prefix + QString::number(type_str.size()) + type_str +
        QString::number(item_str.size()) + item_str);
  }

  /// Builds a legacy (barcode format 1) person barcode as printed by old
  /// colman versions.
  inline QString legacy_person_barcode(QString const& prefix, int person_id)
  {
    auto const id_str = QString::number(person_id);
    return with_legacy_check_digit(
        prefix + "0" + QString::number(id_str.size()) + id_str);
  }


} // namespace colman

#endif /* #ifndef COLMAN_TEST_HELPERS_HPP */

/* EOF */
