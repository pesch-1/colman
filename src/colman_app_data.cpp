// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Application.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_app_data.hpp"
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_MSGBOX_HPP
#  include "colman_msgbox.hpp"
#endif
#ifndef SYSTEM_QAPPLICATION
#  include <QApplication>
#  define SYSTEM_QAPPLICATION
#endif
#ifndef SYSTEM_QMESSAGEBOX
#  include <QMessageBox>
#  define SYSTEM_QMESSAGEBOX
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif

namespace
{
  auto make_backup_error_cb()
  {
    return []() {
      QMessageBox::warning(nullptr,
                           QApplication::tr("Collection Manager"),
                           QApplication::tr("could not write backup file"));
    };
  }

  auto make_old_backups_cb()
  {
    return []() {
      return colman::information_yes_no(
                 nullptr,
                 QApplication::tr("Collection Manager"),
                 QApplication::tr("Delete old backup files?")) ==
             QMessageBox::Yes;
    };
  }
} // namespace

void colman::AppData::set_thumbnail_dir(QString const& dir)
{
  thumbnail_dir_ = dir;
  auto* sql = dynamic_cast<SQLQueries*>(queries_.get());
  if(sql) {
    sql->set_thumbnail_dir(dir);
  }
}

QString colman::AppData::thumbnail_dir() const
{
  return thumbnail_dir_;
}

/// Constructs AppData with a file-backed SQLite database.
colman::AppData::AppData()
{
  queries_ = std::make_unique<SQLQueries>(
      SQLQueries::FileDb{}, make_backup_error_cb(), make_old_backups_cb());
}

/// Constructs AppData with a named in-memory SQLite database (for tests).
colman::AppData::AppData(QString const& conn_name)
{
  queries_ = std::make_unique<SQLQueries>(conn_name);
}

/// Constructs AppData with an externally supplied IQueries implementation.
colman::AppData::AppData(std::unique_ptr<IQueries> queries, bool show_external_id_ui)
    : queries_(std::move(queries))
{
  show_external_id_ui_ = show_external_id_ui;
}

/* EOF */
