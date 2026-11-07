// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Functions for populating item attribute rows in a QGridLayout.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ITEM_TABLE_HPP
#  define COLMAN_ITEM_TABLE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QGRIDLAYOUT
#    include <QGridLayout>
#    define SYSTEM_QGRIDLAYOUT
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif

namespace colman
{
  /// Returns true if \a s is a valid date in German format (d.M.yyyy etc.).
  /**
     Accepts year-only (yyyy), month+year (M.yyyy or MM.yyyy), and full
     dates (d.M.yyyy, dd.MM.yyyy) with optional leading zeros.  Empty
     strings are also considered valid (not required).
     @param s string to validate.
  */
  bool is_valid_date_input(QString const& s);
  /// Adds a read-write attribute row to \a grid at \a row.
  /**
     Creates a label for \a key and an appropriate editor widget for \a value
     (determined by \a type), wired to invoke \a check after every change.
     @param data    application state (for database access)
     @param grid    target layout
     @param row     grid row index to fill
     @param key     attribute name (displayed as label)
     @param value   current attribute value
     @param type_id database id of the item type (for labels and completers)
     @param type    data type info controlling the editor widget
     @param check   callback invoked after each user edit
  */
  void fill_row_rw(colman::AppData& data,
                   QGridLayout* grid,
                   int row,
                   QString const& key,
                   QString const& value,
                   int type_id,
                   DataTypeInfo const& type,
                   std::function<void(AppData&)> check);

  /// Adds a read-only attribute row to \a grid at \a row.
  /**
     Creates a label for \a key and a read-only display widget for \a value
     (determined by \a type).
     @param data    application state (for database access)
     @param grid    target layout
     @param row     grid row index to fill
     @param key     attribute name (displayed as label)
     @param value   current attribute value
     @param type_id database id of the item type (for labels)
     @param type    data type info controlling the display widget
  */
  void fill_row_ro(AppData& data,
                   QGridLayout* grid,
                   int row,
                   QString const& key,
                   QString const& value,
                   int type_id,
                   colman::DataTypeInfo const& type);

  /// Adds a read-write repeatable-field row to \a grid at \a row.
  /**
     Creates a label for \a key and a ListValueWidget pre-populated with
     \a values, wired to invoke \a check after every change.
     @param data    application state
     @param grid    target layout
     @param row     grid row index to fill
     @param key     attribute name (displayed as label)
     @param values  current list of values
     @param type_id database id of the item type (for labels)
     @param type    data type info (must satisfy is_repeatable())
     @param check   callback invoked after each user edit
  */
  void fill_row_rw(colman::AppData& data,
                   QGridLayout* grid,
                   int row,
                   QString const& key,
                   QStringList const& values,
                   int type_id,
                   DataTypeInfo const& type,
                   std::function<void(AppData&)> check);

  /// Adds a read-only repeatable-field row to \a grid at \a row.
  /**
     Creates a label for \a key and a read-only ListValueWidget for \a values.
     @param data    application state
     @param grid    target layout
     @param row     grid row index to fill
     @param key     attribute name (displayed as label)
     @param values  current list of values
     @param type_id database id of the item type (for labels)
     @param type    data type info (must satisfy is_repeatable())
  */
  void fill_row_ro(AppData& data,
                   QGridLayout* grid,
                   int row,
                   QString const& key,
                   QStringList const& values,
                   int type_id,
                   colman::DataTypeInfo const& type);

} // namespace colman

#endif /* #ifndef COLMAN_ITEM_TABLE_HPP */

/* EOF */
