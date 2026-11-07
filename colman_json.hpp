// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// JSON serialization and deserialization for colman domain entities.
/**
   Each entity type has a pair of free functions:
   - to_json(entity) produces a QJsonObject.
   - from_json(obj, entity) populates entity from obj.

   from_json() returns true on success.  It returns false only when a
   required field is absent or has the wrong JSON type.  Optional fields
   fall back to the entity's default values when absent.

   Required fields per entity:
   - Item: "type_id" (integer ≥ 0)
   - Person: none
   - Lending: "item_id", "person_id", "count"
   - Return: "lending_id", "count"
   - Acquisition: "item_id", "count"
   - Liquidation: "item_id", "count"

   Date/time fields use ISO 8601 strings (Qt::ISODate).
   An absent or null "date_time" is accepted; the caller is responsible for
   filling in a server-side timestamp before persisting.

   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_JSON_HPP
#  define COLMAN_JSON_HPP

#  ifndef SYSTEM_QJSONOBJECT
#    include <QJsonObject>
#    define SYSTEM_QJSONOBJECT
#  endif
#  ifndef COLMAN_ACQUISITION_HPP
#    include "colman_acquisition.hpp"
#  endif
#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_LENDING_HPP
#    include "colman_lending.hpp"
#  endif
#  ifndef COLMAN_LIQUIDATION_HPP
#    include "colman_liquidation.hpp"
#  endif
#  ifndef COLMAN_PERSON_HPP
#    include "colman_person.hpp"
#  endif
#  ifndef COLMAN_RETURN_HPP
#    include "colman_return.hpp"
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif

namespace colman
{

// ---------------------------------------------------------------------------
// Item
// ---------------------------------------------------------------------------

/// Serializes \a item to a JSON object.
  QJsonObject to_json(Item const& item);

/// Populates \a item from \a obj.
/** Required: "type_id" (integer ≥ 0). */
  bool from_json(QJsonObject const& obj, Item& item);

// ---------------------------------------------------------------------------
// Person
// ---------------------------------------------------------------------------

/// Serializes \a person to a JSON object.
  QJsonObject to_json(Person const& person);

/// Populates \a person from \a obj.
  bool from_json(QJsonObject const& obj, Person& person);

// ---------------------------------------------------------------------------
// Lending
// ---------------------------------------------------------------------------

/// Serializes \a lending to a JSON object.
  QJsonObject to_json(Lending const& lending);

/// Populates \a lending from \a obj.
/** Required: "item_id", "person_id", "count". */
  bool from_json(QJsonObject const& obj, Lending& lending);

// ---------------------------------------------------------------------------
// Return
// ---------------------------------------------------------------------------

/// Serializes \a ret to a JSON object.
  QJsonObject to_json(Return const& ret);

/// Populates \a ret from \a obj.
/** Required: "lending_id", "count". */
  bool from_json(QJsonObject const& obj, Return& ret);

// ---------------------------------------------------------------------------
// Acquisition
// ---------------------------------------------------------------------------

/// Serializes \a acquisition to a JSON object.
  QJsonObject to_json(Acquisition const& acquisition);

/// Populates \a acquisition from \a obj.
/** Required: "item_id", "count". */
  bool from_json(QJsonObject const& obj, Acquisition& acquisition);

// ---------------------------------------------------------------------------
// Liquidation
// ---------------------------------------------------------------------------

/// Serializes \a liquidation to a JSON object.
  QJsonObject to_json(Liquidation const& liquidation);

/// Populates \a liquidation from \a obj.
/** Required: "item_id", "count". */
  bool from_json(QJsonObject const& obj, Liquidation& liquidation);

/// Serializes \a reservation to a JSON object.
  QJsonObject to_json(IQueries::Reservation const& reservation);

/// Populates \a reservation from \a obj.
/** Required: "item_id", "person_id", "count", "expires_at". */
  bool from_json(QJsonObject const& obj, IQueries::Reservation& reservation);

} // namespace colman

#endif /* #ifndef COLMAN_JSON_HPP */

/* EOF */
