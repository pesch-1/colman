// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries history, versioning, reservations, and school year domain.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_sql_queries_history.hpp"
#ifndef COLMAN_SQL_QUERIES_IMPL_HPP
#  include "colman_sql_queries_impl.hpp"
#  define COLMAN_SQL_QUERIES_IMPL_HPP
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef SYSTEM_QSQLQUERY
#  include <QSqlQuery>
#  define SYSTEM_QSQLQUERY
#endif
#ifndef SYSTEM_QSQLRECORD
#  include <QSqlRecord>
#  define SYSTEM_QSQLRECORD
#endif
#ifndef SYSTEM_QSQLERROR
#  include <QSqlError>
#  define SYSTEM_QSQLERROR
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_ALGORITHM
#  include <algorithm>
#  define SYSTEM_ALGORITHM
#endif
#ifndef SYSTEM_SET
#  include <set>
#  define SYSTEM_SET
#endif
#ifndef SYSTEM_UTILITY
#  include <utility>
#  define SYSTEM_UTILITY
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif

namespace
{

  /// Builds the collector CASE expression for history queries.
  QString collector_expr(QString const& table)
  {
    return ", CASE WHEN " + table +
           ".data_collector IS NULL"
           "          OR " +
           table +
           ".data_collector = 0"
           "     THEN 'Administrator'"
           "     ELSE TRIM(COALESCE(c.firstname,'') || ' '"
           "              || COALESCE(c.lastname,'')) END ";
  }

  /// Builds the collector LEFT JOIN clause for history queries.
  QString collector_join(QString const& table)
  {
    return "LEFT JOIN persons AS c ON " + table + ".data_collector = c.id ";
  }

  /// Fills \a r from the current row of \a q.
  void fill_reservation(QSqlQuery const& q, colman::IQueries::Reservation& r)
  {
    r.id = q.value(0).toInt();
    r.item_id = q.value(1).toInt();
    r.person_id = q.value(2).toInt();
    r.count = q.value(3).toInt();
    r.reserved_at = q.value(4).toDateTime();
    r.expires_at = q.value(5).toDate();
    r.version = q.value(6).toInt();
    r.data_collector_id = q.value(7).toInt();
  }

} // namespace

std::vector<colman::HistoryCombinedModel::Row> colman::SQLQueries::
    query_combined_history(bool is_admin, QDate const& from_date,
                           QDate const& to_date, bool show_all,
                           int limit, int offset,
                           QString const& action_filter)
{
  QDate const from = from_date.isValid() ? from_date : QDate(1, 1, 1);
  QDate const to = to_date.isValid() ? to_date : QDate(9999, 12, 31);
  auto const mk_collector = [is_admin](QString const& t) {
    return is_admin ? collector_expr(t) : QString(", ''");
  };
  auto const mk_collector_join = [is_admin](QString const& t) {
    return is_admin ? collector_join(t) : QString();
  };
  auto const mk_where = [show_all](QString const& dt_expr) -> QString {
    if(show_all) {
      return {};
    }
    return " WHERE date(" + dt_expr + ") BETWEEN :from AND :to ";
  };
  // One SELECT per action type; only those matching action_filter are
  // combined.  The datetime column is aliased in every part because the
  // first part determines the column name used by ORDER BY.
  std::vector<std::pair<QString, QString>> const parts = {
      {"acquisition",
       "SELECT acquisitions.id, acquisitions.datetime AS datetime,"
       " 'acquisition', items.name, acquisitions.count, ''" +
           mk_collector("acquisitions") +
           ", items.id, -1, '', NULL"
           " FROM acquisitions "
           "JOIN items ON acquisitions.item = items.id " +
           mk_collector_join("acquisitions") +
           mk_where("acquisitions.datetime")},
      {"liquidation",
       "SELECT liquidations.id, liquidations.datetime AS datetime,"
       " 'liquidation', items.name, liquidations.count, ''" +
           mk_collector("liquidations") +
           ", items.id, -1, '', NULL"
           " FROM liquidations "
           "JOIN items ON liquidations.item = items.id " +
           mk_collector_join("liquidations") +
           mk_where("liquidations.datetime")},
      {"lending",
       "SELECT lendings.id, lendings.datetime AS datetime,"
       " 'lending', items.name, lendings.count,"
       " persons.lastname || ', ' || persons.firstname" +
           mk_collector("lendings") +
           ", items.id, persons.id, lendings.remarks, lendings.duedate"
           " FROM lendings "
           "JOIN items ON lendings.item = items.id "
           "JOIN persons ON lendings.person = persons.id " +
           mk_collector_join("lendings") +
           mk_where("lendings.datetime")},
      {"return",
       "SELECT returns.id, returns.datetime AS datetime,"
       " 'return', items.name, returns.count,"
       " persons.lastname || ', ' || persons.firstname" +
           mk_collector("returns") +
           ", items.id, persons.id, returns.remarks, NULL"
           " FROM returns "
           "JOIN lendings ON returns.lending = lendings.id "
           "JOIN items ON lendings.item = items.id "
           "JOIN persons ON persons.id = lendings.person " +
           mk_collector_join("returns") +
           mk_where("returns.datetime")},
      {"reservation",
       "SELECT reservations.id, reservations.reserved_at AS datetime,"
       " 'reservation', items.name, reservations.count,"
       " persons.lastname || ', ' || persons.firstname" +
           mk_collector("reservations") +
           ", items.id, persons.id, '', reservations.expires_at"
           " FROM reservations "
           "JOIN items ON reservations.item_id = items.id "
           "JOIN persons ON reservations.person_id = persons.id " +
           mk_collector_join("reservations") +
           mk_where("reservations.reserved_at")}};
  QStringList selects;
  for(auto const& part : parts) {
    if(action_filter.isEmpty() || action_filter == part.first) {
      selects << part.second;
    }
  }
  if(selects.isEmpty()) {
    return {};
  }
  QString sql = selects.join("UNION ALL ") + "ORDER BY datetime DESC";
  if(limit > 0) {
    sql += " LIMIT " + QString::number(limit + 1) +
           " OFFSET " + QString::number(offset);
  }
  QSqlQuery q(db_);
  q.prepare(sql);
  if(!show_all) {
    q.bindValue(":from", from.toString(Qt::ISODate));
    q.bindValue(":to", to.toString(Qt::ISODate));
  }
  if(!q.exec()) {
    qWarning() << "query_combined_history failed:" << q.lastError();
    return {};
  }
  std::vector<HistoryCombinedModel::Row> rows;
  while(q.next()) {
    HistoryCombinedModel::Row row;
    row.id = q.value(0).toInt();
    row.datetime = q.value(1).toDateTime();
    row.action = q.value(2).toString();
    row.item_name = q.value(3).toString();
    row.count = q.value(4).toInt();
    row.user_name = q.value(5).toString();
    row.collector = q.value(6).toString();
    row.item_id = q.value(7).toInt();
    row.person_id = q.value(8).toInt();
    row.remarks = q.value(9).toString();
    row.duedate = q.value(10).toDate();
    rows.push_back(std::move(row));
  }
  return rows;
}

/// Deletes all history actions in \a actions and recomputes item counters.
/**
   Runs all deletions in a single transaction.  Returns are deleted before
   their parent lendings; item counts are recomputed from scratch at the end.
   @param actions rows to delete
   @return true on success, false if any database error occurred
*/
bool colman::SQLQueries::batch_delete_history_actions(
    std::vector<HistoryCombinedModel::Row> const& actions)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  std::set<int> item_ids;
  for(auto const& row : actions) {
    if(row.item_id > 0) {
      item_ids.insert(row.item_id);
    }
  }
  // Delete returns first.
  for(auto const& row : actions) {
    if(row.action != "return") {
      continue;
    }
    q.prepare("DELETE FROM returns WHERE id=:id");
    q.bindValue(":id", row.id);
    if(!q.exec()) {
      qWarning() << "batch delete return failed:" << q.lastError();
      return false;
    }
  }
  // Delete lendings (cascades any not-yet-deleted returns via explicit SQL).
  for(auto const& row : actions) {
    if(row.action != "lending") {
      continue;
    }
    q.prepare("DELETE FROM returns WHERE lending=:id");
    q.bindValue(":id", row.id);
    if(!q.exec()) {
      qWarning() << "batch delete cascading returns failed:" << q.lastError();
    }
    q.prepare("DELETE FROM lendings WHERE id=:id");
    q.bindValue(":id", row.id);
    if(!q.exec()) {
      qWarning() << "batch delete lending failed:" << q.lastError();
      return false;
    }
  }
  // Delete acquisitions.
  for(auto const& row : actions) {
    if(row.action != "acquisition") {
      continue;
    }
    q.prepare("DELETE FROM acquisitions WHERE id=:id");
    q.bindValue(":id", row.id);
    if(!q.exec()) {
      qWarning() << "batch delete acquisition failed:" << q.lastError();
      return false;
    }
  }
  // Delete liquidations.
  for(auto const& row : actions) {
    if(row.action != "liquidation") {
      continue;
    }
    q.prepare("DELETE FROM liquidations WHERE id=:id");
    q.bindValue(":id", row.id);
    if(!q.exec()) {
      qWarning() << "batch delete liquidation failed:" << q.lastError();
      return false;
    }
  }
  // Delete reservations.
  for(auto const& row : actions) {
    if(row.action != "reservation") {
      continue;
    }
    q.prepare("DELETE FROM reservations WHERE id=:id");
    q.bindValue(":id", row.id);
    if(!q.exec()) {
      qWarning() << "batch delete reservation failed:" << q.lastError();
      return false;
    }
  }
  // Recompute items.count and items.available from the remaining rows.
  for(int id : item_ids) {
    if(!recompute_item_counts(id)) {
      return false;
    }
  }
  rollback.commit();
  for(int id : item_ids) {
    QSqlQuery check(db_);
    check.prepare("SELECT COUNT(*) FROM acquisitions WHERE item = :item_id");
    check.bindValue(":item_id", id);
    if(check.exec() && check.next() && check.value(0).toInt() == 0) {
      if(!remove_item(id)) {
        qWarning() << "batch_delete_history_actions: remove_item failed for"
                   << id;
      }
    }
  }
  return true;
}

/// Returns all history rows that directly conflict with deleting \a action.
/**
   A later lending or liquidation is a conflict only when the running available
   at its time, recomputed without the action being deleted, would have been
   insufficient.  Liquidations have no conflicts: deleting a liquidation only
   increases available stock, which never invalidates any later action.
   @param action the history row whose deletion is being considered
   @return vector of conflicting rows that would be invalidated by deletion
*/
std::vector<colman::HistoryCombinedModel::Row> colman::SQLQueries::
    find_conflicting_actions(HistoryCombinedModel::Row const& action)
{
  std::vector<HistoryCombinedModel::Row> result;
  QSqlQuery q(db_);

  auto const read_rows = [&]() {
    if(!q.exec()) {
      qWarning() << "find_conflicting_actions failed:" << q.lastError();
      return;
    }
    while(q.next()) {
      HistoryCombinedModel::Row row;
      row.id = q.value(0).toInt();
      row.datetime = q.value(1).toDateTime();
      row.action = q.value(2).toString();
      row.item_name = q.value(3).toString();
      row.count = q.value(4).toInt();
      row.user_name = q.value(5).toString();
      row.collector = q.value(6).toString();
      row.item_id = q.value(7).toInt();
      row.person_id = q.value(8).toInt();
      row.remarks = q.value(9).toString();
      result.push_back(std::move(row));
    }
  };

  // Subquery fragment: running available just before event E for item E.item,
  // optionally excluding one acquisition (:excl_acq_id, use -1 to skip) or
  // one return (:excl_ret_id, use -1 to skip).
  // The event alias is the outer table alias (l for lending, lq for liq).
  auto const avail_before = [](QString const& alias, bool excl_acq,
                               bool excl_ret) -> QString {
    QString excl_acq_cond = excl_acq ? " AND a.id != :action_id" : "";
    QString excl_ret_cond = excl_ret ? " AND r.id != :action_id" : "";
    return "COALESCE((SELECT SUM(a.count) FROM acquisitions a"
           " WHERE a.item=" +
           alias +
           ".item"
           " AND a.datetime < " +
           alias + ".datetime" + excl_acq_cond +
           "), 0)"
           " - COALESCE((SELECT SUM(lq2.count) FROM liquidations lq2"
           " WHERE lq2.item=" +
           alias +
           ".item"
           " AND lq2.datetime < " +
           alias +
           ".datetime), 0)"
           " - COALESCE((SELECT SUM(le.count) FROM lendings le"
           " WHERE le.item=" +
           alias +
           ".item"
           " AND le.datetime < " +
           alias +
           ".datetime), 0)"
           " + COALESCE((SELECT SUM(r.count) FROM returns r"
           " JOIN lendings le2 ON r.lending=le2.id"
           " WHERE le2.item=" +
           alias +
           ".item"
           " AND r.datetime < " +
           alias + ".datetime" + excl_ret_cond + "), 0)";
  };

  if(action.action == "acquisition") {
    // Lendings that needed this acquisition's copies.
    q.prepare("SELECT l.id, l.datetime, 'lending', i.name, l.count,"
              " p.lastname || ', ' || p.firstname, '', i.id, p.id, l.remarks"
              " FROM lendings l"
              " JOIN items i ON l.item = i.id"
              " JOIN persons p ON l.person = p.id"
              " WHERE l.item = :item_id AND l.datetime > :dt"
              "   AND (" +
              avail_before("l", true, false) +
              ") < l.count"
              " ORDER BY l.datetime");
    q.bindValue(":item_id", action.item_id);
    q.bindValue(":dt", action.datetime);
    q.bindValue(":action_id", action.id);
    read_rows();
    // Liquidations that needed this acquisition's copies.
    q.prepare("SELECT lq.id, lq.datetime, 'liquidation', i.name, lq.count,"
              " '', '', i.id, -1, ''"
              " FROM liquidations lq"
              " JOIN items i ON lq.item = i.id"
              " WHERE lq.item = :item_id AND lq.datetime > :dt"
              "   AND (" +
              avail_before("lq", true, false) +
              ") < lq.count"
              " ORDER BY lq.datetime");
    q.bindValue(":item_id", action.item_id);
    q.bindValue(":dt", action.datetime);
    q.bindValue(":action_id", action.id);
    read_rows();
  }
  else if(action.action == "lending") {
    // Returns that reference this lending — the only DB-level dependency.
    q.prepare("SELECT r.id, r.datetime, 'return', i.name, r.count,"
              " p.lastname || ', ' || p.firstname, '', i.id, p.id, r.remarks"
              " FROM returns r"
              " JOIN lendings l ON r.lending = l.id"
              " JOIN items i ON l.item = i.id"
              " JOIN persons p ON l.person = p.id"
              " WHERE r.lending = :action_id"
              " ORDER BY r.datetime");
    q.bindValue(":action_id", action.id);
    read_rows();
  }
  else if(action.action == "return") {
    // Lendings that needed the availability freed by this return.
    q.prepare("SELECT l.id, l.datetime, 'lending', i.name, l.count,"
              " p.lastname || ', ' || p.firstname, '', i.id, p.id, l.remarks"
              " FROM lendings l"
              " JOIN items i ON l.item = i.id"
              " JOIN persons p ON l.person = p.id"
              " WHERE l.item = :item_id AND l.datetime > :dt"
              "   AND (" +
              avail_before("l", false, true) +
              ") < l.count"
              " ORDER BY l.datetime");
    q.bindValue(":item_id", action.item_id);
    q.bindValue(":dt", action.datetime);
    q.bindValue(":action_id", action.id);
    read_rows();
    // Liquidations that needed the availability freed by this return.
    q.prepare("SELECT lq.id, lq.datetime, 'liquidation', i.name, lq.count,"
              " '', '', i.id, -1, ''"
              " FROM liquidations lq"
              " JOIN items i ON lq.item = i.id"
              " WHERE lq.item = :item_id AND lq.datetime > :dt"
              "   AND (" +
              avail_before("lq", false, true) +
              ") < lq.count"
              " ORDER BY lq.datetime");
    q.bindValue(":item_id", action.item_id);
    q.bindValue(":dt", action.datetime);
    q.bindValue(":action_id", action.id);
    read_rows();
  }
  // liquidation: no conflicts — deletion only increases available stock.
  return result;
}

/// Returns all history rows that conflict with changing \a old_action.
/**
   Checks availability (all lendings/liquidations that would become
   infeasible), self-validity (action at new position / count), ordering
   (return ↔ lending), and count (total returns vs. new lending count).
   @param old_action current stored row
   @param new_action proposed replacement (same id and action type)
   @return conflicting rows (empty means the change is safe to apply)
*/
std::vector<colman::HistoryCombinedModel::Row> colman::SQLQueries::
    find_change_conflicts(HistoryCombinedModel::Row const& old_action,
                          HistoryCombinedModel::Row const& new_action)
{
  std::vector<HistoryCombinedModel::Row> result;
  QSqlQuery q(db_);

  QString const& action = old_action.action;
  QString const old_id = QString::number(old_action.id);
  QString const item_id = QString::number(old_action.item_id);

  std::set<std::pair<QString, int>> seen;

  auto const add_row = [&](HistoryCombinedModel::Row row) {
    auto const key = std::make_pair(row.action, row.id);
    if(seen.count(key)) {
      return;
    }
    seen.insert(key);
    result.push_back(std::move(row));
  };

  auto const read_rows = [&]() {
    if(!q.exec()) {
      qWarning() << "find_change_conflicts query failed:" << q.lastError();
      return;
    }
    while(q.next()) {
      HistoryCombinedModel::Row row;
      row.id = q.value(0).toInt();
      row.datetime = q.value(1).toDateTime();
      row.action = q.value(2).toString();
      row.item_name = q.value(3).toString();
      row.count = q.value(4).toInt();
      row.user_name = q.value(5).toString();
      row.collector = q.value(6).toString();
      row.item_id = q.value(7).toInt();
      row.person_id = q.value(8).toInt();
      row.remarks = q.value(9).toString();
      add_row(std::move(row));
    }
  };

  // avail_with_change(alias): availability at alias.datetime assuming
  // old_action is removed and new_action is inserted at :new_dt/:new_count.
  auto const avail_with_change = [&](QString const& alias) -> QString {
    QString const acq_excl =
        (action == "acquisition") ? " AND a.id != " + old_id : "";
    QString const liq_excl =
        (action == "liquidation") ? " AND lq2.id != " + old_id : "";
    QString const lend_excl =
        (action == "lending") ? " AND le.id != " + old_id : "";
    QString const ret_excl =
        (action == "return") ? " AND r.id != " + old_id : "";
    QString const sign =
        (action == "acquisition" || action == "return") ? "+" : "-";
    return "COALESCE((SELECT SUM(a.count) FROM acquisitions a"
           " WHERE a.item=" +
           alias +
           ".item"
           " AND a.datetime < " +
           alias + ".datetime" + acq_excl +
           "), 0)"
           " - COALESCE((SELECT SUM(lq2.count) FROM liquidations lq2"
           " WHERE lq2.item=" +
           alias +
           ".item"
           " AND lq2.datetime < " +
           alias + ".datetime" + liq_excl +
           "), 0)"
           " - COALESCE((SELECT SUM(le.count) FROM lendings le"
           " WHERE le.item=" +
           alias +
           ".item"
           " AND le.datetime < " +
           alias + ".datetime" + lend_excl +
           "), 0)"
           " + COALESCE((SELECT SUM(r.count) FROM returns r"
           " JOIN lendings le2 ON r.lending=le2.id"
           " WHERE le2.item=" +
           alias +
           ".item"
           " AND r.datetime < " +
           alias + ".datetime" + ret_excl +
           "), 0)"
           " " +
           sign + " CASE WHEN :new_dt < " + alias +
           ".datetime"
           " THEN :new_count ELSE 0 END";
  };

  // Check A: lendings (excl. old if lending) where avail_with_change < count.
  {
    QString const self_excl =
        (action == "lending") ? " AND l.id != " + old_id : "";
    q.prepare("SELECT l.id, l.datetime, 'lending', i.name, l.count,"
              " p.lastname || ', ' || p.firstname, '', i.id, p.id, l.remarks"
              " FROM lendings l"
              " JOIN items i ON l.item = i.id"
              " JOIN persons p ON l.person = p.id"
              " WHERE l.item = " +
              item_id + self_excl + "   AND (" + avail_with_change("l") +
              ") < l.count"
              " ORDER BY l.datetime");
    q.bindValue(":new_dt", new_action.datetime);
    q.bindValue(":new_count", new_action.count);
    read_rows();
  }

  // Check B: liquidations (excl. old if liquidation) where avail_with_change
  // < count.
  {
    QString const self_excl =
        (action == "liquidation") ? " AND lq.id != " + old_id : "";
    q.prepare("SELECT lq.id, lq.datetime, 'liquidation', i.name, lq.count,"
              " '', '', i.id, -1, ''"
              " FROM liquidations lq"
              " JOIN items i ON lq.item = i.id"
              " WHERE lq.item = " +
              item_id + self_excl + "   AND (" + avail_with_change("lq") +
              ") < lq.count"
              " ORDER BY lq.datetime");
    q.bindValue(":new_dt", new_action.datetime);
    q.bindValue(":new_count", new_action.count);
    read_rows();
  }

  // Check C: self-validity for lending/liquidation — availability just before
  // :new_dt (old action excluded) must be >= new_count.
  if(action == "lending" || action == "liquidation") {
    QString const liq_excl =
        (action == "liquidation") ? " AND id != " + old_id : "";
    QString const lend_excl =
        (action == "lending") ? " AND id != " + old_id : "";
    q.prepare("SELECT "
              "COALESCE((SELECT SUM(count) FROM acquisitions"
              "  WHERE item=" +
              item_id +
              " AND datetime < :new_dt), 0)"
              " - COALESCE((SELECT SUM(count) FROM liquidations"
              "  WHERE item=" +
              item_id + " AND datetime < :new_dt" + liq_excl +
              "), 0)"
              " - COALESCE((SELECT SUM(count) FROM lendings"
              "  WHERE item=" +
              item_id + " AND datetime < :new_dt" + lend_excl +
              "), 0)"
              " + COALESCE((SELECT SUM(r.count) FROM returns r"
              "  JOIN lendings le ON r.lending=le.id"
              "  WHERE le.item=" +
              item_id + " AND r.datetime < :new_dt), 0)");
    q.bindValue(":new_dt", new_action.datetime);
    if(!q.exec()) {
      qWarning() << "find_change_conflicts self-check failed:" << q.lastError();
    }
    else if(q.next() && q.value(0).toInt() < new_action.count) {
      // Stock insufficient — show all acquisitions for the item so the
      // user can see which acquisition(s) limit the available count.
      q.prepare("SELECT a.id, a.datetime, 'acquisition', i.name, a.count,"
                " '', '', i.id, -1, ''"
                " FROM acquisitions a"
                " JOIN items i ON a.item = i.id"
                " WHERE a.item = " +
                item_id + " ORDER BY a.datetime");
      read_rows();
    }
  }

  // Checks D/E/F/G — return/lending-specific ordering and count constraints.
  if(action == "return") {
    // Look up the parent lending id.
    q.prepare("SELECT lending FROM returns WHERE id=" + old_id);
    int lending_id = -1;
    if(q.exec() && q.next()) {
      lending_id = q.value(0).toInt();
    }
    else {
      qWarning() << "find_change_conflicts: parent lending lookup failed:"
                 << q.lastError();
    }
    if(lending_id > 0) {
      QString const lend_id_str = QString::number(lending_id);

      // Check D: self-validity — outstanding copies at :new_dt >= new_count.
      q.prepare("SELECT l.count -"
                " COALESCE((SELECT SUM(r.count) FROM returns r"
                "  WHERE r.lending=" +
                lend_id_str +
                "  AND r.datetime < :new_dt AND r.id != " + old_id +
                "), 0)"
                " FROM lendings l WHERE l.id=" +
                lend_id_str);
      q.bindValue(":new_dt", new_action.datetime);
      if(!q.exec()) {
        qWarning() << "find_change_conflicts return self-check failed:"
                   << q.lastError();
      }
      else if(q.next() && q.value(0).toInt() < new_action.count) {
        // Not enough unreturned copies — show the parent lending.
        q.prepare(
            "SELECT l.id, l.datetime, 'lending', i.name, l.count,"
            " p.lastname || ', ' || p.firstname, '', i.id, p.id, l.remarks"
            " FROM lendings l"
            " JOIN items i ON l.item = i.id"
            " JOIN persons p ON l.person = p.id"
            " WHERE l.id = " +
            lend_id_str);
        read_rows();
      }

      // Check F: parent lending must come strictly before :new_dt.
      q.prepare("SELECT l.id, l.datetime, 'lending', i.name, l.count,"
                " p.lastname || ', ' || p.firstname, '', i.id, p.id, l.remarks"
                " FROM lendings l"
                " JOIN items i ON l.item = i.id"
                " JOIN persons p ON l.person = p.id"
                " WHERE l.id=" +
                lend_id_str + " AND l.datetime >= :new_dt");
      q.bindValue(":new_dt", new_action.datetime);
      read_rows();
    }
  }

  if(action == "lending") {
    // Check E: returns of THIS lending at or before :new_dt.
    q.prepare("SELECT r.id, r.datetime, 'return', i.name, r.count,"
              " p.lastname || ', ' || p.firstname, '', i.id, p.id, r.remarks"
              " FROM returns r"
              " JOIN lendings l ON r.lending = l.id"
              " JOIN items i ON l.item = i.id"
              " JOIN persons p ON l.person = p.id"
              " WHERE r.lending=" +
              old_id +
              " AND r.datetime <= :new_dt"
              " ORDER BY r.datetime");
    q.bindValue(":new_dt", new_action.datetime);
    read_rows();

    // Check G: all returns of THIS lending if their total > new_count.
    q.prepare("SELECT r.id, r.datetime, 'return', i.name, r.count,"
              " p.lastname || ', ' || p.firstname, '', i.id, p.id, r.remarks"
              " FROM returns r"
              " JOIN lendings l ON r.lending = l.id"
              " JOIN items i ON l.item = i.id"
              " JOIN persons p ON l.person = p.id"
              " WHERE r.lending=" +
              old_id +
              "   AND (SELECT COALESCE(SUM(r2.count),0) FROM returns r2"
              "        WHERE r2.lending=" +
              old_id +
              ") > :new_count"
              " ORDER BY r.datetime");
    q.bindValue(":new_count", new_action.count);
    read_rows();
  }

  return result;
}

/// Updates a single history row and recomputes item counters.
/**
   Updates count, datetime, and (for lending/return) remarks and
   data_collector.  Wrapped in a single transaction; rolls back on failure.
   @param old_action identifies the row to update (action type and id)
   @param new_action new values (count, datetime, remarks, collector_id)
   @return true on success, false if any database error occurred
*/
bool colman::SQLQueries::apply_action_change(
    HistoryCombinedModel::Row const& old_action,
    HistoryCombinedModel::Row const& new_action)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  QString const& action = old_action.action;

  if(action == "acquisition") {
    q.prepare("UPDATE acquisitions SET count=:count, datetime=:dt,"
              " data_collector=:dc WHERE id=:id");
    q.bindValue(":count", new_action.count);
    q.bindValue(":dt", new_action.datetime);
    q.bindValue(":dc", new_action.collector_id);
    q.bindValue(":id", old_action.id);
    if(!q.exec()) {
      qWarning() << "apply_action_change acquisition failed:" << q.lastError();
      return false;
    }
  }
  else if(action == "liquidation") {
    q.prepare("UPDATE liquidations SET count=:count, datetime=:dt,"
              " data_collector=:dc WHERE id=:id");
    q.bindValue(":count", new_action.count);
    q.bindValue(":dt", new_action.datetime);
    q.bindValue(":dc", new_action.collector_id);
    q.bindValue(":id", old_action.id);
    if(!q.exec()) {
      qWarning() << "apply_action_change liquidation failed:" << q.lastError();
      return false;
    }
  }
  else if(action == "lending") {
    q.prepare(
        "UPDATE lendings SET count=:count, datetime=:dt, remarks=:rem,"
        " data_collector=:dc, person=:person, duedate=:duedate WHERE id=:id");
    q.bindValue(":count", new_action.count);
    q.bindValue(":dt", new_action.datetime);
    q.bindValue(":rem", new_action.remarks);
    q.bindValue(":dc", new_action.collector_id);
    q.bindValue(":person", new_action.person_id);
    q.bindValue(":duedate", new_action.duedate.isValid()
                                ? new_action.duedate.toString(Qt::ISODate)
                                : QString());
    q.bindValue(":id", old_action.id);
    if(!q.exec()) {
      qWarning() << "apply_action_change lending failed:" << q.lastError();
      return false;
    }
  }
  else if(action == "return") {
    q.prepare("UPDATE returns SET count=:count, datetime=:dt, remarks=:rem,"
              " data_collector=:dc WHERE id=:id");
    q.bindValue(":count", new_action.count);
    q.bindValue(":dt", new_action.datetime);
    q.bindValue(":rem", new_action.remarks);
    q.bindValue(":dc", new_action.collector_id);
    q.bindValue(":id", old_action.id);
    if(!q.exec()) {
      qWarning() << "apply_action_change return failed:" << q.lastError();
      return false;
    }
  }
  else if(action == "reservation") {
    q.prepare("UPDATE reservations SET count=:count, expires_at=:expires"
              " WHERE id=:id");
    q.bindValue(":count", new_action.count);
    q.bindValue(":expires",
                new_action.duedate.isValid()
                    ? new_action.duedate.toString(Qt::ISODate)
                    : QString());
    q.bindValue(":id", old_action.id);
    if(!q.exec()) {
      qWarning() << "apply_action_change reservation failed:"
                 << q.lastError();
      return false;
    }
    rollback.commit();
    return true;
  }
  else {
    qWarning() << "apply_action_change: unknown action type:" << action;
    return false;
  }

  if(!recompute_item_counts(old_action.item_id)) {
    return false;
  }
  rollback.commit();
  return true;
}

/// Loads the sorted list of current class names (academic_year='').
/**
 * @return sorted list of class names, or empty list on error
 */
QStringList colman::SQLQueries::load_class_names()
{
  QStringList result;
  QSqlQuery q(db_);
  q.prepare("SELECT name FROM classes WHERE academic_year='' ORDER BY name");
  if(q.exec()) {
    while(q.next()) {
      result << q.value(0).toString();
    }
  }
  else {
    qWarning() << "load_class_names failed:" << q.lastError();
  }
  return result;
}

/// Adds a class name to the current class list; no-op if already present.
/**
 * @param name  class name to add
 * @return true on success
 */
bool colman::SQLQueries::add_class_name(QString const& name)
{
  QSqlQuery q(db_);
  q.prepare("INSERT OR IGNORE INTO classes(name, academic_year, count) "
            "VALUES(:name, '', 0)");
  q.bindValue(":name", name);
  if(!q.exec()) {
    qWarning() << "add_class_name failed:" << q.lastError();
    return false;
  }
  return true;
}

/// Removes a class name from the current class list.
/**
 * @param name  class name to remove
 * @return true on success
 */
bool colman::SQLQueries::remove_class_name(QString const& name)
{
  QSqlQuery q(db_);
  q.prepare("DELETE FROM classes WHERE name=:name AND academic_year=''");
  q.bindValue(":name", name);
  if(!q.exec()) {
    qWarning() << "remove_class_name failed:" << q.lastError();
    return false;
  }
  return true;
}

/// Renames a class and updates all persons with that class (one transaction).
/**
 * @param old_name  current class name
 * @param new_name  desired new class name
 * @return true on success
 */
bool colman::SQLQueries::rename_class_name(QString const& old_name,
                                           QString const& new_name)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare("UPDATE classes SET name=:new "
            "WHERE name=:old AND academic_year=''");
  q.bindValue(":new", new_name);
  q.bindValue(":old", old_name);
  if(!q.exec()) {
    qWarning() << "rename_class_name (classes) failed:" << q.lastError();
    return false;
  }
  q.prepare("UPDATE persons SET school_class=:new "
            "WHERE school_class=:old");
  q.bindValue(":new", new_name);
  q.bindValue(":old", old_name);
  if(!q.exec()) {
    qWarning() << "rename_class_name (persons) failed:" << q.lastError();
    return false;
  }
  rollback.commit();
  return true;
}

/// Returns persons whose school_class matches a key in old_to_new.
/**
 * @param old_to_new  maps old class name → new class name
 * @return affected persons sorted by old class, then last/first name
 */
std::vector<colman::IQueries::AffectedPerson>
colman::SQLQueries::get_affected_persons(
    QMap<QString, QString> const& old_to_new)
{
  std::vector<AffectedPerson> result;
  if(old_to_new.isEmpty()) {
    return result;
  }
  QSqlQuery q(db_);
  q.prepare("SELECT id, lastname, firstname, school_class FROM persons "
            "WHERE school_class != '' AND id > 0 "
            "ORDER BY school_class, lastname, firstname");
  if(!q.exec()) {
    qWarning() << "get_affected_persons query failed:" << q.lastError();
    return result;
  }
  while(q.next()) {
    QString const cls = q.value(3).toString();
    if(old_to_new.contains(cls)) {
      AffectedPerson ap;
      ap.person_id_ = q.value(0).toInt();
      ap.display_name_ =
          q.value(1).toString() + ", " + q.value(2).toString();
      ap.old_class_ = cls;
      ap.new_class_ = old_to_new[cls];
      result.push_back(ap);
    }
  }
  return result;
}

/// Applies per-person class changes and replaces the classes list.
/**
 * Runs everything in a single transaction.
 * @param person_id_to_new_class  person_id → new school_class (empty = clear)
 * @param persons_to_delete       person ids to remove (must have no open lendings)
 * @param new_class_list          replaces all academic_year='' rows in classes
 * @return true on success
 */
bool colman::SQLQueries::apply_school_year_changes(
    QMap<int, QString> const& person_id_to_new_class,
    QList<int> const& persons_to_delete,
    QStringList const& new_class_list)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  for(auto it = person_id_to_new_class.begin();
      it != person_id_to_new_class.end(); ++it) {
    q.prepare("UPDATE persons SET school_class=:new "
              "WHERE id=:pid AND id > 0");
    q.bindValue(":new", it.value());
    q.bindValue(":pid", it.key());
    if(!q.exec()) {
      qWarning() << "apply_school_year_changes: update person failed:"
                 << q.lastError();
      return false;
    }
  }
  for(int const pid : persons_to_delete) {
    if(!remove_person(pid)) {
      qWarning() << "apply_school_year_changes: remove_person failed for id"
                 << pid;
      return false;
    }
  }
  q.prepare("DELETE FROM classes WHERE academic_year=''");
  if(!q.exec()) {
    qWarning() << "apply_school_year_changes: delete classes failed:"
               << q.lastError();
    return false;
  }
  for(QString const& name : new_class_list) {
    q.prepare("INSERT INTO classes(name, academic_year, count) "
              "VALUES(:n, '', 0)");
    q.bindValue(":n", name);
    if(!q.exec()) {
      qWarning() << "apply_school_year_changes: insert class failed:"
                 << q.lastError();
      return false;
    }
  }
  rollback.commit();
  return true;
}

/// Returns the \c version value of the row with \a id in the table for
/// \a entity.
/**
 * Used by the REST layer to build \c ETag response headers.
 * @param entity  which entity table to query
 * @param id      primary key of the row
 * @return version (>= 1) on success, -1 if the row does not exist or on
 *         database error
 */
int colman::SQLQueries::get_version(
    IQueries::VersionedEntity entity, int id)
{
  static char const* kTableNames[] = {
      "items",        // Item
      "persons",      // Person
      "lendings",     // Lending
      "returns",      // Return
      "acquisitions", // Acquisition
      "liquidations"  // Liquidation
  };
  auto const idx = static_cast<int>(entity);
  QSqlQuery q(db_);
  q.prepare(
      QString("SELECT version FROM \"%1\" WHERE id = :id")
          .arg(kTableNames[idx]));
  q.bindValue(":id", id);
  if(!q.exec() || !q.next()) {
    return -1;
  }
  return q.value(0).toInt();
}

/// Returns the per-table change counters from colman_table_versions.
colman::IQueries::TableVersions colman::SQLQueries::query_table_versions()
{
  QSqlQuery q(db_);
  TableVersions result;
  if(!q.exec("SELECT table_name, version FROM colman_table_versions")) {
    return result;
  }
  while(q.next()) {
    result.insert(q.value(0).toString(), q.value(1).toInt());
  }
  return result;
}

// -----------------------------------------------------------------------
// Versioned updates (ETag / If-Match support)
// -----------------------------------------------------------------------

/// Updates an item only if its stored version equals \a expected_version.
/**
 * Runs entirely inside a BEGIN IMMEDIATE transaction.  On a version match
 * the item data is written and the version column is incremented.
 * @param id               primary key of the item to update
 * @param item             new item data
 * @param expected_version version from the client's If-Match header
 * @return Ok, VersionMismatch, NotFound, or Error
 */
colman::IQueries::UpdateResult colman::SQLQueries::change_item_versioned(
    int id, Item const& item, int expected_version)
{
  AutoRollback rollback(db_);
  auto const r = check_version(db_, "items", id, expected_version);
  if(r != IQueries::UpdateResult::Ok) {
    return r;
  }
  QString const tablename = get_tablename(item.get_type_id());
  if(tablename.isEmpty()) {
    return IQueries::UpdateResult::Error;
  }
  QSqlQuery old_type_query(db_);
  old_type_query.prepare("SELECT type FROM items WHERE id = :id");
  old_type_query.bindValue(":id", id);
  if(!old_type_query.exec() || !old_type_query.next()) {
    qWarning() << "change_item_versioned: could not read old type for item "
               << id;
    return IQueries::UpdateResult::Error;
  }
  int const old_type_id = old_type_query.value(0).toInt();
  QString const old_tablename = get_tablename(old_type_id);
  bool const type_changed = (old_type_id != item.get_type_id());
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE items SET name = :name, type = :type, "
      "place = :place, description = :description, "
      "thumbnail = :thumbnail, "
      "data_collector = :data_collector, "
      "signature_top = :signature_top, "
      "signature_bottom = :signature_bottom, "
      "keywords = :keywords, "
      "allow_reservations = :allow_reservations, "
      "reservation_period_days = :reservation_period_days, "
      "version = version + 1 "
      "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":name", item.get_name());
  query.bindValue(":type", item.get_type_id());
  query.bindValue(":place", item.get_place());
  query.bindValue(":description", item.get_description());
  query.bindValue(":thumbnail", item.get_thumbnail_id());
  query.bindValue(":data_collector", item.get_data_collector_id());
  query.bindValue(":signature_top", item.get_signature_top());
  query.bindValue(":signature_bottom", item.get_signature_bottom());
  query.bindValue(":keywords", keywords_to_string(item.get_keywords()));
  query.bindValue(
      ":allow_reservations",
      item.get_allow_reservations() == -1
          ? QVariant()
          : QVariant(item.get_allow_reservations()));
  query.bindValue(
      ":reservation_period_days",
      item.get_reservation_period_days() == -1
          ? QVariant()
          : QVariant(item.get_reservation_period_days()));
  if(!query.exec()) {
    qWarning() << "change_item_versioned: could not update item: "
               << query.lastError();
    return IQueries::UpdateResult::Error;
  }
  if(item.get_is_school_item()) {
    query.prepare("INSERT OR REPLACE INTO schoolitems "
                  "(item_id, subject, academic_year, type) "
                  "VALUES (:id, :subject, :academic_year, :type)");
    query.bindValue(":id", id);
    query.bindValue(":subject", item.get_subject());
    query.bindValue(":academic_year",
                    academic_years_to_string(item.get_academic_year()));
    query.bindValue(":type", item.get_type());
    if(!query.exec()) {
      return IQueries::UpdateResult::Error;
    }
  }
  else {
    query.prepare("DELETE FROM schoolitems WHERE item_id = :id");
    query.bindValue(":id", id);
    if(!query.exec()) {
      return IQueries::UpdateResult::Error;
    }
  }
  if(type_changed) {
    if(!old_tablename.isEmpty()) {
      query.prepare("DELETE FROM " + old_tablename +
                    " WHERE item_id = :id");
      query.bindValue(":id", id);
      if(!query.exec()) {
        qWarning() << "change_item_versioned: deleting from " << old_tablename
                   << " failed: " << query.lastError();
        return IQueries::UpdateResult::Error;
      }
    }
    std::vector<QString> columns = get_table_columns(tablename, 1);
    std::sort(columns.begin(), columns.end());
    QString querystring("INSERT INTO ");
    querystring += tablename + " (item_id";
    for(auto const& value : item.get_values()) {
      if(!value.second.isEmpty() &&
         std::binary_search(columns.begin(), columns.end(), value.first)) {
        querystring += ", \"" + value.first + "\"";
      }
    }
    querystring += ") VALUES (:item_id";
    for(auto const& value : item.get_values()) {
      if(!value.second.isEmpty() &&
         std::binary_search(columns.begin(), columns.end(), value.first)) {
        querystring += ", :" + value.first;
      }
    }
    querystring += ")";
    query.prepare(querystring);
    query.bindValue(":item_id", id);
    for(auto const& value : item.get_values()) {
      if(!value.second.isEmpty() &&
         std::binary_search(columns.begin(), columns.end(), value.first)) {
        query.bindValue(":" + value.first, value.second);
      }
    }
    if(!query.exec()) {
      qWarning() << "change_item_versioned: inserting into " << tablename
                 << " failed: " << query.lastError()
                 << "\nquery: " << querystring;
      return IQueries::UpdateResult::Error;
    }
  }
  else if(!item.get_values().empty()) {
    std::vector<QString> columns = get_table_columns(tablename, 1);
    std::sort(columns.begin(), columns.end());
    QString querystring("UPDATE ");
    querystring += tablename + " SET ";
    bool first = true;
    for(auto const& value : item.get_values()) {
      if(std::binary_search(columns.begin(), columns.end(), value.first)) {
        if(!first) {
          querystring += ", ";
        }
        querystring += "\"" + value.first + "\" = :" + value.first;
        first = false;
      }
    }
    querystring += " WHERE item_id = :item_id";
    query.prepare(querystring);
    query.bindValue(":item_id", id);
    for(auto const& value : item.get_values()) {
      if(std::binary_search(columns.begin(), columns.end(), value.first)) {
        query.bindValue(":" + value.first, value.second);
      }
    }
    if(!query.exec()) {
      return IQueries::UpdateResult::Error;
    }
  }
  query.prepare("DELETE FROM item_list_values WHERE item_id = :id");
  query.bindValue(":id", id);
  if(!query.exec()) {
    return IQueries::UpdateResult::Error;
  }
  if(!insert_list_values(query, item, id)) {
    return IQueries::UpdateResult::Error;
  }
  rollback.commit();
  rebuild_fts_for_item(id);
  return IQueries::UpdateResult::Ok;
}

/// Updates a person only if its stored version equals \a expected_version.
/**
 * @param id               primary key of the person to update
 * @param person           new person data
 * @param expected_version version from the client's If-Match header
 * @return Ok, VersionMismatch, NotFound, or Error
 */
colman::IQueries::UpdateResult colman::SQLQueries::change_person_versioned(
    int id, Person const& person, int expected_version)
{
  if(id == 0) {
    qWarning() << "change_person_versioned: Administrator must not be modified";
    return IQueries::UpdateResult::Error;
  }
  AutoRollback rollback(db_);
  auto const r = check_version(db_, "persons", id, expected_version);
  if(r != IQueries::UpdateResult::Ok) {
    return r;
  }
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE persons SET lastname = :lastname, "
      "firstname = :firstname, title = :title, "
      "school_class = :school_class, "
      "emails = :emails, "
      "telephonenumbers = :telephonenumbers, street = :street, "
      "zip = :zip, city = :city, "
      "country = :country, remarks = :remarks, "
      "thumbnail = :thumbnail, "
      "data_collector = :data_collector, "
      "version = version + 1 "
      "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":lastname", person.get_lastname());
  query.bindValue(":firstname", person.get_firstname());
  query.bindValue(":title", person.get_title());
  query.bindValue(":school_class", person.get_school_class());
  query.bindValue(":emails", person.get_emails());
  query.bindValue(":telephonenumbers", person.get_telephone());
  query.bindValue(":street", person.get_street());
  query.bindValue(":zip", person.get_zip());
  query.bindValue(":city", person.get_city());
  query.bindValue(":country", person.get_country());
  query.bindValue(":remarks", person.get_remarks());
  query.bindValue(":thumbnail", person.get_thumbnail_id());
  query.bindValue(":data_collector", person.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "change_person_versioned: could not update person: "
               << query.lastError();
    return IQueries::UpdateResult::Error;
  }
  rollback.commit();
  return IQueries::UpdateResult::Ok;
}

/// Updates a lending only if its stored version equals \a expected_version.
/**
 * Reads the current lending state and the lending balance inside the
 * transaction so all checks are consistent.
 * @param id               primary key of the lending to update
 * @param lending          new lending data
 * @param expected_version version from the client's If-Match header
 * @return Ok, VersionMismatch, NotFound, or Error
 */
colman::IQueries::UpdateResult colman::SQLQueries::change_lending_versioned(
    int id, Lending const& lending, int expected_version)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare("SELECT version, count, item FROM lendings WHERE id = :id");
  q.bindValue(":id", id);
  if(!q.exec()) {
    return IQueries::UpdateResult::Error;
  }
  if(!q.next()) {
    return IQueries::UpdateResult::NotFound;
  }
  if(q.value(0).toInt() != expected_version) {
    return IQueries::UpdateResult::VersionMismatch;
  }
  unsigned int const old_count = q.value(1).toUInt();
  int const item_id = q.value(2).toInt();
  std::size_t lent = 0, returned = 0;
  if(!lending_balance(id, lent, returned)) {
    return IQueries::UpdateResult::Error;
  }
  if(lent + lending.get_count() < returned + old_count) {
    return IQueries::UpdateResult::Error;
  }
  if(old_count < lending.get_count()) {
    unsigned int const diff = lending.get_count() - old_count;
    if(!subtract_from_available(item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  else if(old_count > lending.get_count()) {
    unsigned int const diff = old_count - lending.get_count();
    if(!add_to_available(item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE lendings SET item = :item, count = :count, "
      "person = :person, datetime = :datetime, "
      "duedate = :duedate, remarks = :remarks, "
      "data_collector = :data_collector, "
      "version = version + 1 "
      "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":item", lending.get_item_id());
  query.bindValue(":count", lending.get_count());
  query.bindValue(":person", lending.get_person_id());
  query.bindValue(":datetime", lending.get_date_time());
  query.bindValue(":duedate", lending.get_duedate());
  query.bindValue(":remarks", lending.get_remarks());
  query.bindValue(":data_collector", lending.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "change_lending_versioned: update failed: "
               << query.lastError();
    return IQueries::UpdateResult::Error;
  }
  rollback.commit();
  return IQueries::UpdateResult::Ok;
}

/// Updates a return only if its stored version equals \a expected_version.
/**
 * @param return_id        primary key of the return to update
 * @param ret              new return data
 * @param expected_version version from the client's If-Match header
 * @return Ok, VersionMismatch, NotFound, or Error
 */
colman::IQueries::UpdateResult colman::SQLQueries::change_return_versioned(
    int return_id, Return const& ret, int expected_version)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare(
      "SELECT version, count, lending FROM returns WHERE id = :id");
  q.bindValue(":id", return_id);
  if(!q.exec()) {
    return IQueries::UpdateResult::Error;
  }
  if(!q.next()) {
    return IQueries::UpdateResult::NotFound;
  }
  if(q.value(0).toInt() != expected_version) {
    return IQueries::UpdateResult::VersionMismatch;
  }
  unsigned int const old_count = q.value(1).toUInt();
  int const lending_id = q.value(2).toInt();
  std::size_t lent = 0, returned = 0;
  if(!lending_balance(lending_id, lent, returned)) {
    return IQueries::UpdateResult::Error;
  }
  if(lent + old_count < returned + ret.get_count()) {
    return IQueries::UpdateResult::Error;
  }
  int const item_id = get_lend_item(lending_id);
  if(item_id < 0) {
    return IQueries::UpdateResult::Error;
  }
  if(old_count > ret.get_count()) {
    unsigned int const diff = old_count - ret.get_count();
    if(!subtract_from_available(item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  else if(old_count < ret.get_count()) {
    unsigned int const diff = ret.get_count() - old_count;
    if(!add_to_available(item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE returns SET count = :count, "
      "datetime = :datetime, remarks = :remarks, "
      "data_collector = :data_collector, "
      "version = version + 1 "
      "WHERE id = :id");
  query.bindValue(":id", return_id);
  query.bindValue(":count", ret.get_count());
  query.bindValue(":datetime", ret.get_date_time());
  query.bindValue(":remarks", ret.get_remarks());
  query.bindValue(":data_collector", ret.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "change_return_versioned: update failed: "
               << query.lastError();
    return IQueries::UpdateResult::Error;
  }
  rollback.commit();
  return IQueries::UpdateResult::Ok;
}

/// Updates an acquisition only if its stored version equals
/// \a expected_version.
/**
 * @param id               primary key of the acquisition to update
 * @param acquisition      new acquisition data
 * @param expected_version version from the client's If-Match header
 * @return Ok, VersionMismatch, NotFound, or Error
 */
colman::IQueries::UpdateResult colman::SQLQueries::change_acquisition_versioned(
    int id, Acquisition const& acquisition, int expected_version)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare(
      "SELECT version, count, item FROM acquisitions WHERE id = :id");
  q.bindValue(":id", id);
  if(!q.exec()) {
    return IQueries::UpdateResult::Error;
  }
  if(!q.next()) {
    return IQueries::UpdateResult::NotFound;
  }
  if(q.value(0).toInt() != expected_version) {
    return IQueries::UpdateResult::VersionMismatch;
  }
  unsigned int const old_count = q.value(1).toUInt();
  int const item_id = q.value(2).toInt();
  if(old_count > acquisition.get_count()) {
    unsigned int const diff = old_count - acquisition.get_count();
    if(!subtract_from_stock_size_and_available(db_, item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  else if(old_count < acquisition.get_count()) {
    unsigned int const diff = acquisition.get_count() - old_count;
    if(!add_to_stock_size_and_available(db_, item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE acquisitions SET count = :count, datetime = :datetime, "
      "data_collector = :data_collector, "
      "version = version + 1 "
      "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":count", acquisition.get_count());
  query.bindValue(":datetime", acquisition.get_date_time());
  query.bindValue(":data_collector", acquisition.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "change_acquisition_versioned: update failed: "
               << query.lastError();
    return IQueries::UpdateResult::Error;
  }
  rollback.commit();
  return IQueries::UpdateResult::Ok;
}

/// Updates a liquidation only if its stored version equals
/// \a expected_version.
/**
 * @param id               primary key of the liquidation to update
 * @param liquidation      new liquidation data
 * @param expected_version version from the client's If-Match header
 * @return Ok, VersionMismatch, NotFound, or Error
 */
colman::IQueries::UpdateResult colman::SQLQueries::change_liquidation_versioned(
    int id, Liquidation const& liquidation, int expected_version)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare(
      "SELECT version, count, item FROM liquidations WHERE id = :id");
  q.bindValue(":id", id);
  if(!q.exec()) {
    return IQueries::UpdateResult::Error;
  }
  if(!q.next()) {
    return IQueries::UpdateResult::NotFound;
  }
  if(q.value(0).toInt() != expected_version) {
    return IQueries::UpdateResult::VersionMismatch;
  }
  unsigned int const old_count = q.value(1).toUInt();
  int const item_id = q.value(2).toInt();
  if(old_count < liquidation.get_count()) {
    unsigned int const diff = liquidation.get_count() - old_count;
    if(!subtract_from_stock_size_and_available(db_, item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  else if(old_count > liquidation.get_count()) {
    unsigned int const diff = old_count - liquidation.get_count();
    if(!add_to_stock_size_and_available(db_, item_id, diff)) {
      return IQueries::UpdateResult::Error;
    }
  }
  QSqlQuery query(db_);
  query.prepare(
      "UPDATE liquidations SET count = :count, datetime = :datetime, "
      "data_collector = :data_collector, "
      "version = version + 1 "
      "WHERE id = :id");
  query.bindValue(":id", id);
  query.bindValue(":count", liquidation.get_count());
  query.bindValue(":datetime", liquidation.get_date_time());
  query.bindValue(":data_collector", liquidation.get_data_collector_id());
  if(!query.exec()) {
    qWarning() << "change_liquidation_versioned: update failed: "
               << query.lastError();
    return IQueries::UpdateResult::Error;
  }
  rollback.commit();
  return IQueries::UpdateResult::Ok;
}

// -----------------------------------------------------------------------
// Reservations
// -----------------------------------------------------------------------

/// Inserts or tops up a reservation within BEGIN IMMEDIATE.
/**
   If the person already has an active reservation for the item, its
   count is increased by \a count instead of inserting a new row; the
   existing expiry date is kept.  The requested copies must not exceed
   the available copies minus those reserved (by anyone).
   @param item_id    item to reserve
   @param person_id  person placing the reservation
   @param count      number of copies to reserve (additionally)
   @param expires_at expiry date for a new reservation
   @param new_id     output: id of the new or topped-up row on success
   @return Ok, NotEnoughAvailable, or Error
*/
colman::IQueries::ReservationResult
colman::SQLQueries::add_reservation(int item_id, int person_id, int count,
                                    QDate const& expires_at,
                                    int data_collector_id, int& new_id)
{
  AutoRollback rollback(db_);
  purge_expired_reservation_unlocked(db_, item_id);
  QSqlQuery q(db_);
  q.prepare("SELECT available FROM items WHERE id = :item_id");
  q.bindValue(":item_id", item_id);
  if(!q.exec() || !q.next()) {
    qWarning() << "add_reservation: unknown item" << item_id;
    return ReservationResult::Error;
  }
  int const avail = q.value(0).toInt();
  QSqlQuery qs(db_);
  qs.prepare("SELECT COALESCE(SUM(count),0) FROM reservations "
             "WHERE item_id = :item_id AND person_id != :person_id "
             "AND expires_at >= date('now', 'localtime')");
  qs.bindValue(":item_id", item_id);
  qs.bindValue(":person_id", person_id);
  if(!qs.exec() || !qs.next()) {
    return ReservationResult::Error;
  }
  int const reserved_for_others = qs.value(0).toInt();
  // Any remaining own reservation is active: expired ones for this item
  // were purged above.
  QSqlQuery qo(db_);
  qo.prepare("SELECT id, count FROM reservations "
             "WHERE item_id = :item_id AND person_id = :person_id");
  qo.bindValue(":item_id", item_id);
  qo.bindValue(":person_id", person_id);
  if(!qo.exec()) {
    return ReservationResult::Error;
  }
  int own_id = -1;
  int own_count = 0;
  if(qo.next()) {
    own_id = qo.value(0).toInt();
    own_count = qo.value(1).toInt();
  }
  if(own_count + count > avail - reserved_for_others) {
    return ReservationResult::NotEnoughAvailable;
  }
  if(own_id >= 0) {
    QSqlQuery upd(db_);
    upd.prepare("UPDATE reservations SET count = count + :count, "
                "version = version + 1 WHERE id = :id");
    upd.bindValue(":count", count);
    upd.bindValue(":id", own_id);
    if(!upd.exec()) {
      qWarning() << "add_reservation top-up failed:" << upd.lastError();
      return ReservationResult::Error;
    }
    new_id = own_id;
    rollback.commit();
    return ReservationResult::Ok;
  }
  QSqlQuery ins(db_);
  ins.prepare("INSERT INTO reservations "
              "(item_id, person_id, count, reserved_at, expires_at, "
              "data_collector) "
              "VALUES (:item_id, :person_id, :count, "
              ":reserved_at, :expires_at, :data_collector)");
  ins.bindValue(":item_id", item_id);
  ins.bindValue(":person_id", person_id);
  ins.bindValue(":count", count);
  ins.bindValue(":reserved_at", QDateTime::currentDateTimeUtc());
  ins.bindValue(":expires_at", expires_at.toString(Qt::ISODate));
  ins.bindValue(":data_collector", data_collector_id);
  if(!ins.exec()) {
    QString const err = ins.lastError().databaseText();
    if(err.contains("UNIQUE", Qt::CaseInsensitive)) {
      return ReservationResult::AlreadyExists;
    }
    qWarning() << "add_reservation insert failed:" << ins.lastError();
    return ReservationResult::Error;
  }
  new_id = ins.lastInsertId().toInt();
  rollback.commit();
  return ReservationResult::Ok;
}

/// Deletes the reservation with \a reservation_id.
/**
   @param reservation_id primary key of the reservation to delete
   @return true on success
*/
bool colman::SQLQueries::remove_reservation(int reservation_id)
{
  AutoRollback rollback(db_);
  QSqlQuery q(db_);
  q.prepare("DELETE FROM reservations WHERE id = :id");
  q.bindValue(":id", reservation_id);
  if(!q.exec()) {
    qWarning() << "remove_reservation failed:" << q.lastError();
    return false;
  }
  if(q.numRowsAffected() < 1) {
    return false;
  }
  rollback.commit();
  return true;
}

/// Returns all active reservations for \a item_id (lazy-purges expired ones).
/**
   @param item_id item to query
   @return vector of active Reservation records (may be empty)
*/
std::vector<colman::IQueries::Reservation>
colman::SQLQueries::get_reservations_for_item(int item_id)
{
  QSqlQuery del(db_);
  del.prepare("DELETE FROM reservations "
              "WHERE item_id = :item_id AND expires_at < date('now', 'localtime')");
  del.bindValue(":item_id", item_id);
  del.exec();
  QSqlQuery q(db_);
  q.prepare("SELECT id, item_id, person_id, count, "
            "reserved_at, expires_at, version, data_collector "
            "FROM reservations "
            "WHERE item_id = :item_id AND expires_at >= date('now', 'localtime')");
  q.bindValue(":item_id", item_id);
  std::vector<Reservation> result;
  if(q.exec()) {
    while(q.next()) {
      Reservation r;
      fill_reservation(q, r);
      result.push_back(std::move(r));
    }
  }
  else {
    qWarning() << "get_reservations_for_item failed:" << q.lastError();
  }
  return result;
}

/// Fills \a r with the active reservation of \a person_id for \a item_id.
/**
   @param item_id   item to query
   @param person_id person whose reservation to look up
   @param r         output reservation
   @return false if no active reservation exists
*/
bool colman::SQLQueries::get_reservation(int item_id, int person_id,
                                         Reservation& r)
{
  QSqlQuery del(db_);
  del.prepare("DELETE FROM reservations "
              "WHERE item_id = :item_id AND expires_at < date('now', 'localtime')");
  del.bindValue(":item_id", item_id);
  del.exec();
  QSqlQuery q(db_);
  q.prepare("SELECT id, item_id, person_id, count, "
            "reserved_at, expires_at, version, data_collector "
            "FROM reservations "
            "WHERE item_id = :item_id AND person_id = :person_id "
            "AND expires_at >= date('now', 'localtime')");
  q.bindValue(":item_id", item_id);
  q.bindValue(":person_id", person_id);
  if(!q.exec() || !q.next()) {
    return false;
  }
  fill_reservation(q, r);
  return true;
}

/// Returns all active reservations, optionally filtered by person.
/**
   @param person_id filter by person; pass -1 for all persons
   @return vector of active Reservation records
*/
std::vector<colman::IQueries::Reservation>
colman::SQLQueries::get_reservations(int person_id)
{
  QSqlQuery q(db_);
  if(person_id >= 0) {
    q.prepare("SELECT id, item_id, person_id, count, "
              "reserved_at, expires_at, version, data_collector "
              "FROM reservations "
              "WHERE person_id = :person_id AND expires_at >= date('now', 'localtime')");
    q.bindValue(":person_id", person_id);
  }
  else {
    q.prepare("SELECT id, item_id, person_id, count, "
              "reserved_at, expires_at, version, data_collector "
              "FROM reservations "
              "WHERE expires_at >= date('now', 'localtime')");
  }
  std::vector<Reservation> result;
  if(q.exec()) {
    while(q.next()) {
      Reservation r;
      fill_reservation(q, r);
      result.push_back(std::move(r));
    }
  }
  else {
    qWarning() << "get_reservations failed:" << q.lastError();
  }
  return result;
}

/* EOF */
