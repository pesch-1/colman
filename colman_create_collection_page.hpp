// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Stacked-widget flow for creating a new collection.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CREATE_COLLECTION_PAGE_HPP
#  define COLMAN_CREATE_COLLECTION_PAGE_HPP

#  ifndef COLMAN_APP_DATA_HPP
#    include "colman_app_data.hpp"
#  endif
#  ifndef SYSTEM_QAPPLICATION
#    include <QApplication>
#    define SYSTEM_QAPPLICATION
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif

namespace colman
{
  /// Starts the create-collection step flow on the main window's page stack.
  /**
     Builds a transient sequence of step pages (name, logo, lending period,
     school, file, optional encryption, admin password), navigates to the first
     step, and removes itself when finished or cancelled.  On success the new
     database has been created, optionally encrypted and given an administrator
     password; @p on_created is then invoked with the new file name so the
     caller can open it.  Cancelling returns to the originating page without
     invoking the callback.
     @param data       central application state.
     @param on_created callback receiving the created collection's file name.
  */
  void enter_create_collection_flow(
      AppData& data, std::function<void(QString const&)> on_created);

} // namespace colman

#endif /* #ifndef COLMAN_CREATE_COLLECTION_PAGE_HPP */

/* EOF */
