// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// MARC21 field definitions and type templates.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_MARC21_HPP
#  define COLMAN_MARC21_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QVECTOR
#    include <QVector>
#    define SYSTEM_QVECTOR
#  endif

#  ifndef COLMAN_TYPE_HPP
#    include "colman_type.hpp"
#  endif

namespace colman
{
  /// A single MARC21 field (tag + subfield) with display metadata.
  struct Marc21Field
  {
    QString tag;           ///< MARC21 tag, e.g. "245"
    QString subfield;      ///< subfield code, e.g. "a"
    QString display_name;  ///< English display name, e.g. "Title"
    QString description;   ///< tooltip/help text
    QString category;      ///< group name for dropdown separator
    DataType suggested_type; ///< recommended DataType for this field
    bool is_repeatable = false; ///< true if the field may occur multiple times
  };

  /// A named set of MARC21 fields for a given media type.
  struct Marc21Template
  {
    QString name;               ///< template name, e.g. "Book"
    QVector<QString> field_ids; ///< ordered list of marc21_id strings
  };

  /// Returns the canonical MARC21 identifier, e.g. "245$a".
  QString marc21_id(Marc21Field const& field);

  /// Returns the curated list of MARC21 fields (all groups).
  QVector<Marc21Field> const& marc21_fields();

  /// Finds a field by its id (e.g. "100$a"); returns nullptr if not found.
  Marc21Field const* find_marc21_field(QString const& id);

  /// Returns predefined media-type templates.
  QVector<Marc21Template> const& marc21_templates();

  /// Returns the MARC21 id for a common-field sentinel, or "" if none.
  /// E.g. "__name__" -> "245$a", "__description__" -> "520$a".
  QString marc21_common_field_tag(QString const& sentinel);

  /// Returns the locale-translated display name of \a field.
  QString marc21_translated_display_name(Marc21Field const& field);

  /// Returns the locale-translated category label of \a field.
  QString marc21_translated_category(Marc21Field const& field);

} // namespace colman

#endif /* #ifndef COLMAN_MARC21_HPP */

/* EOF */
