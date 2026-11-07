// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Computation of possible actions like lending or return
/**
   @file
   @author Christoph Pesch
*/

#include "colman_actions.hpp"
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#  define COLMAN_IQUERIES_HPP
#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_EAN_QUERIES_HPP
#  include "colman_ean_queries.hpp"
#  define COLMAN_EAN_QUERIES_HPP
#endif

namespace
{
  /// Returns a short string representation of \a action.
  char const* action_name(colman::Action action)
  {
    switch(action) {
    case colman::Action::None:
      return "None";
    case colman::Action::Acquisition:
      return "Acquisition";
    case colman::Action::Liquidation:
      return "Liquidation";
    case colman::Action::Lending:
      return "Lending";
    case colman::Action::Return:
      return "Return";
    }
    return "?";
  }

  /// Appends Return actions for each open lending of \a item_id, and a
  /// Lending action if copies are available.
  /// \a barcode is stored in each action so callers can filter item lists.
  void append_item_actions(colman::IQueries& queries,
                           int item_id,
                           std::vector<colman::ActionInfo>& result,
                           QString const& barcode = {})
  {
    for(auto const& lend : queries.open_lendings_for_item(item_id)) {
      colman::ActionInfo ai;
      ai.action_ = colman::Action::Return;
      ai.item_id_ = item_id;
      ai.lending_id_ = lend.first;
      ai.person_id_ = lend.second;
      ai.barcode_ = barcode;
      result.push_back(ai);
    }
    if(queries.available(item_id) > 0) {
      colman::ActionInfo ai;
      ai.action_ = colman::Action::Lending;
      ai.item_id_ = item_id;
      ai.barcode_ = barcode;
      result.push_back(ai);
    }
  }

} // namespace

/**
   First determines whether the barcode is an item barcode, a person
   barcode, a product EAN/UPC-A, or an ISBN:

   1. Item barcode.
      1.1 one or more copies borrowed: add Return (item_id, lending_id,
          person_id) for each open lending.
      1.2 one or more copies available: add Lending (item_id).

   2. Person barcode.
      2.1 person has open lendings: add Return (item_id, lending_id,
          person_id) for each open lending.
      2.2 add Lending (person_id) unconditionally.

   3. Product EAN-13 or UPC-A barcode (CD, DVD, Blu-ray, …).
      3.1 item not in collection: add Acquisition (barcode set).
      3.2 item in collection: apply rules 1.1 and 1.2 for each item.

   4. ISBN barcode.
      4.1 book not in collection: add Acquisition (barcode set).
      4.2 book in collection: apply rules 1.1 and 1.2 for each item.
*/
std::vector<colman::ActionInfo> colman::possible_actions(IQueries& queries, QString const& barcode)
{
  std::vector<ActionInfo> result;
  Collection const col = queries.read_collection();

  // 1. Item barcode.
  int type_id = -1;
  int item_id = -1;
  if(parse_item_barcode(col, barcode, type_id, item_id)) {
    append_item_actions(queries, item_id, result, barcode);
    return result;
  }

  // 2. Person barcode.
  int person_id = -1;
  if(parse_person_barcode(col, barcode, person_id)) {
    for(auto const& lend : queries.open_lendings_for_person(person_id)) {
      ActionInfo ai;
      ai.action_ = Action::Return;
      ai.item_id_ = lend.second;
      ai.lending_id_ = lend.first;
      ai.person_id_ = person_id;
      result.push_back(ai);
    }
    // 2.2 Person can always borrow another item.
    {
      ActionInfo ai;
      ai.action_ = Action::Lending;
      ai.person_id_ = person_id;
      result.push_back(ai);
    }
    return result;
  }

  // 3. Product EAN / UPC-A barcode (CD, DVD, Blu-ray, …).
  bool const is_ean = is_product_ean(barcode);
  bool const is_upc = !is_ean && is_upc_a(barcode);
  if(is_ean || is_upc) {
    QString const ean13 = is_upc ? QString("0") + barcode : barcode;
    for(int const iid : queries.items_by_ean(ean13)) {
      append_item_actions(queries, iid, result, barcode);
    }
    if(result.empty()) {
      ActionInfo ai;
      ai.action_ = Action::Acquisition;
      ai.barcode_ = barcode;
      result.push_back(ai);
    }
    return result;
  }

  // 4. ISBN barcode.
  for(int const iid : queries.items_by_isbn(barcode)) {
    // 4.2 Book in collection.
    append_item_actions(queries, iid, result, barcode);
  }
  // 4.1 Book not in collection.
  if(result.empty()) {
    ActionInfo ai;
    ai.action_ = Action::Acquisition;
    ai.barcode_ = barcode;
    result.push_back(ai);
  }
  return result;
}


/// Prints each action on its own line:
/// "action=<name> item_id=<n> lending_id=<n> person_id=<n>".
/// Fields that are -1 are omitted.
void colman::print_actions(std::vector<ActionInfo> const& actions)
{
  for(ActionInfo const& ai : actions) {
    std::cout << "action=" << action_name(ai.action_);
    if(ai.item_id_ >= 0) {
      std::cout << " item_id=" << ai.item_id_;
    }
    if(ai.lending_id_ >= 0) {
      std::cout << " lending_id=" << ai.lending_id_;
    }
    if(ai.person_id_ >= 0) {
      std::cout << " person_id=" << ai.person_id_;
    }
    std::cout << '\n';
  }
  std::cout << std::flush;
}


/* EOF */
