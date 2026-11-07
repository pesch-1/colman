// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementations for colman_table_models.hpp.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TABLE_MODELS_HPP
#  include "colman_table_models.hpp"
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QCOLOR
#  include <QColor>
#  define SYSTEM_QCOLOR
#endif
#ifndef SYSTEM_QFONT
#  include <QFont>
#  define SYSTEM_QFONT
#endif

namespace colman
{

// --- TypeListModel -----------------------------------------------------------

  TypeListModel::TypeListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void TypeListModel::set_types(std::vector<Type> types)
  {
    beginResetModel();
    types_ = std::move(types);
    endResetModel();
  }

  int TypeListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(types_.size());
  }

  int TypeListModel::columnCount(QModelIndex const&) const
  {
    return 3;
  }

  QVariant TypeListModel::data(QModelIndex const& idx, int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(types_.size())) {
      return {};
    }
    Type const& t = types_[r];
    switch(idx.column()) {
    case 0:
      return t.get_id();
    case 1:
      return t.get_name();
    case 2:
      return t.get_name();
    default:
      return {};
    }
  }

  QVariant TypeListModel::headerData(int section,
                                     Qt::Orientation o,
                                     int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 2:
      return QCoreApplication::tr("name");
    default:
      return {};
    }
  }

// --- PersonListModel ---------------------------------------------------------

  PersonListModel::PersonListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void PersonListModel::set_persons(std::vector<Person> persons)
  {
    beginResetModel();
    persons_ = std::move(persons);
    endResetModel();
  }

  int PersonListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(persons_.size());
  }

  int PersonListModel::columnCount(QModelIndex const&) const
  {
    return 6;
  }

  QVariant PersonListModel::data(QModelIndex const& idx, int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(persons_.size())) {
      return {};
    }
    Person const& p = persons_[r];
    switch(idx.column()) {
    case 0:
      return p.get_id();
    case 1:
      return p.get_lastname();
    case 2:
      return p.get_firstname();
    case ClassColumn:
      return p.get_school_class();
    case TitleColumn:
      return p.get_title();
    case 5:
      return p.get_username();
    default:
      return {};
    }
  }

  QVariant PersonListModel::headerData(int section,
                                       Qt::Orientation o,
                                       int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("last name");
    case 2:
      return QCoreApplication::tr("first name");
    case ClassColumn:
      return QCoreApplication::tr("class");
    case TitleColumn:
      return QCoreApplication::tr("title");
    case 5:
      return QCoreApplication::tr("username");
    default:
      return {};
    }
  }

// --- StartPagePersonListModel ------------------------------------------------

  StartPagePersonListModel::StartPagePersonListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void StartPagePersonListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int StartPagePersonListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int StartPagePersonListModel::columnCount(QModelIndex const&) const
  {
    return 5;
  }

  QVariant StartPagePersonListModel::data(QModelIndex const& idx,
                                          int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.lastname;
    case 2:
      return row.firstname;
    case 3:
      return row.title;
    case 4:
      return row.has_password ? 1 : 0;
    default:
      return {};
    }
  }

  QVariant StartPagePersonListModel::headerData(int section,
                                                Qt::Orientation o,
                                                int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("last name");
    case 2:
      return QCoreApplication::tr("first name");
    case 3:
      return QCoreApplication::tr("title");
    case 4:
      return QCoreApplication::tr("password");
    default:
      return {};
    }
  }

// --- ItemListModel -----------------------------------------------------------

  ItemListModel::ItemListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void ItemListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  void ItemListModel::append_rows(std::vector<Row> rows)
  {
    if(rows.empty()) {
      return;
    }
    auto const first = static_cast<int>(rows_.size());
    auto const last = first + static_cast<int>(rows.size()) - 1;
    beginInsertRows({}, first, last);
    for(auto& r : rows) {
      rows_.push_back(std::move(r));
    }
    endInsertRows();
  }

  void ItemListModel::set_grayout_unavailable(bool enable)
  {
    grayout_unavailable_ = enable;
  }

  int ItemListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int ItemListModel::columnCount(QModelIndex const&) const
  {
    return 7;
  }

  QVariant ItemListModel::data(QModelIndex const& idx, int role) const
  {
    if(!idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    if(role == Qt::ForegroundRole) {
      if(grayout_unavailable_ && row.available == 0) {
        return QColor(Qt::gray);
      }
      return {};
    }
    if(role != Qt::DisplayRole) {
      return {};
    }
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.name;
    case 2:
      return row.count;
    case 3:
      return row.available;
    case 4:
      return row.type_translated;
    case 5:
      return row.type_name;
    case 6:
      return row.collector;
    default:
      return {};
    }
  }

  QVariant ItemListModel::headerData(int section,
                                     Qt::Orientation o,
                                     int role) const
  {
    if(o != Qt::Horizontal) {
      return {};
    }
    if(role == Qt::TextAlignmentRole) {
      // Left-align the headers so the first column's caption stays visible at
      // the left edge even when the column is much wider than the viewport
      // (long item names); a centred caption would scroll out of sight.
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
    if(role == Qt::FontRole) {
      // Keep the captions permanently bold; otherwise the header only turns
      // bold while a row is selected (QHeaderView::highlightSections).
      QFont font;
      font.setBold(true);
      return font;
    }
    if(role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("name");
    case 2:
      return QCoreApplication::tr("count");
    case 3:
      return QCoreApplication::tr("available");
    case 4:
      return QCoreApplication::tr("type");
    case 5:
      return QCoreApplication::tr("type");
    case 6:
      return QCoreApplication::tr("collector");
    default:
      return {};
    }
  }

// --- OpenLendingListModel ----------------------------------------------------

  OpenLendingListModel::OpenLendingListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void OpenLendingListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int OpenLendingListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int OpenLendingListModel::columnCount(QModelIndex const&) const
  {
    return 7;
  }

  QVariant OpenLendingListModel::data(QModelIndex const& idx, int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.lent_count;
    case 2:
      return row.returned_count;
    case 3:
      return row.datetime;
    case 4:
      return row.duedate;
    case 5:
      return row.remarks;
    case 6:
      return row.person_name;
    default:
      return {};
    }
  }

  QVariant OpenLendingListModel::headerData(int section,
                                            Qt::Orientation o,
                                            int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("lent");
    case 2:
      return QCoreApplication::tr("returned");
    case 3:
      return QCoreApplication::tr("date");
    case 4:
      return QCoreApplication::tr("due date");
    case 5:
      return QCoreApplication::tr("remarks");
    case 6:
      return QCoreApplication::tr("borrower");
    default:
      return {};
    }
  }


// --- HistoryAcquisitionListModel ---------------------------------------------

  HistoryAcquisitionListModel::HistoryAcquisitionListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void HistoryAcquisitionListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int HistoryAcquisitionListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int HistoryAcquisitionListModel::columnCount(QModelIndex const&) const
  {
    return 5;
  }

  QVariant HistoryAcquisitionListModel::data(QModelIndex const& idx,
                                             int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.datetime;
    case 2:
      return row.count;
    case 3:
      return row.item_name;
    case 4:
      return row.collector;
    default:
      return {};
    }
  }

  QVariant HistoryAcquisitionListModel::headerData(int section,
                                                   Qt::Orientation o,
                                                   int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("date");
    case 2:
      return QCoreApplication::tr("count");
    case 3:
      return QCoreApplication::tr("name");
    case 4:
      return QCoreApplication::tr("collector");
    default:
      return {};
    }
  }

// --- HistoryLiquidationListModel ---------------------------------------------

  HistoryLiquidationListModel::HistoryLiquidationListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void HistoryLiquidationListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int HistoryLiquidationListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int HistoryLiquidationListModel::columnCount(QModelIndex const&) const
  {
    return 5;
  }

  QVariant HistoryLiquidationListModel::data(QModelIndex const& idx,
                                             int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.datetime;
    case 2:
      return row.count;
    case 3:
      return row.item_name;
    case 4:
      return row.collector;
    default:
      return {};
    }
  }

  QVariant HistoryLiquidationListModel::headerData(int section,
                                                   Qt::Orientation o,
                                                   int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("date");
    case 2:
      return QCoreApplication::tr("count");
    case 3:
      return QCoreApplication::tr("name");
    case 4:
      return QCoreApplication::tr("collector");
    default:
      return {};
    }
  }

// --- HistoryLendingListModel -------------------------------------------------

  HistoryLendingListModel::HistoryLendingListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void HistoryLendingListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int HistoryLendingListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int HistoryLendingListModel::columnCount(QModelIndex const&) const
  {
    return 8;
  }

  QVariant HistoryLendingListModel::data(QModelIndex const& idx,
                                         int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.datetime;
    case 2:
      return row.count;
    case 3:
      return row.item_name;
    case 4:
      return row.lastname;
    case 5:
      return row.firstname;
    case 6:
      return row.duedate;
    case 7:
      return row.collector;
    default:
      return {};
    }
  }

  QVariant HistoryLendingListModel::headerData(int section,
                                               Qt::Orientation o,
                                               int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("date");
    case 2:
      return QCoreApplication::tr("count");
    case 3:
      return QCoreApplication::tr("name");
    case 4:
      return QCoreApplication::tr("last name");
    case 5:
      return QCoreApplication::tr("first name");
    case 6:
      return QCoreApplication::tr("due date");
    case 7:
      return QCoreApplication::tr("collector");
    default:
      return {};
    }
  }

// --- HistoryReturnListModel --------------------------------------------------

  HistoryReturnListModel::HistoryReturnListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void HistoryReturnListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  int HistoryReturnListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int HistoryReturnListModel::columnCount(QModelIndex const&) const
  {
    return 7;
  }

  QVariant HistoryReturnListModel::data(QModelIndex const& idx,
                                        int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.datetime;
    case 2:
      return row.count;
    case 3:
      return row.item_name;
    case 4:
      return row.lastname;
    case 5:
      return row.firstname;
    case 6:
      return row.collector;
    default:
      return {};
    }
  }

  QVariant HistoryReturnListModel::headerData(int section,
                                              Qt::Orientation o,
                                              int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("date");
    case 2:
      return QCoreApplication::tr("count");
    case 3:
      return QCoreApplication::tr("name");
    case 4:
      return QCoreApplication::tr("last name");
    case 5:
      return QCoreApplication::tr("first name");
    case 6:
      return QCoreApplication::tr("collector");
    default:
      return {};
    }
  }

// --- HistoryCombinedModel ----------------------------------------------------

  HistoryCombinedModel::HistoryCombinedModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void HistoryCombinedModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  void HistoryCombinedModel::append_rows(std::vector<Row> rows)
  {
    if(rows.empty()) {
      return;
    }
    auto const first = static_cast<int>(rows_.size());
    auto const last = first + static_cast<int>(rows.size()) - 1;
    beginInsertRows({}, first, last);
    for(auto& r : rows) {
      rows_.push_back(std::move(r));
    }
    endInsertRows();
  }

  int HistoryCombinedModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int HistoryCombinedModel::columnCount(QModelIndex const&) const
  {
    return 11;
  }

  QVariant HistoryCombinedModel::data(QModelIndex const& idx,
                                      int role) const
  {
    if(!idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || static_cast<size_t>(r) >= rows_.size()) {
      return {};
    }
    auto const& row = rows_[r];
    if(role == Qt::UserRole && idx.column() == 2) {
      return row.action;
    }
    if(role != Qt::DisplayRole) {
      return {};
    }
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.datetime;
    case 2:
      if(row.action == "acquisition") {
        return QCoreApplication::tr("acquisition");
      }
      if(row.action == "liquidation") {
        return QCoreApplication::tr("liquidation");
      }
      if(row.action == "lending") {
        return QCoreApplication::tr("lending");
      }
      if(row.action == "return") {
        return QCoreApplication::tr("return");
      }
      if(row.action == "reservation") {
        return QCoreApplication::tr("reservation");
      }
      return {};
    case 3:
      return row.item_name;
    case 4:
      return row.count;
    case 5:
      return row.user_name;
    case 6:
      return row.collector;
    case 7:
      return row.item_id;
    case 8:
      return row.person_id;
    case 9:
      return row.remarks;
    case 10:
      return row.duedate;
    default:
      return {};
    }
  }

  QVariant HistoryCombinedModel::headerData(int section,
                                            Qt::Orientation o,
                                            int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("date/time");
    case 2:
      return QCoreApplication::tr("action");
    case 3:
      return QCoreApplication::tr("name");
    case 4:
      return QCoreApplication::tr("number");
    case 5:
      return QCoreApplication::tr("user");
    case 6:
      return QCoreApplication::tr("data collector");
    default:
      return {};
    }
  }

// --- ChangePasswordsPersonListModel ------------------------------------------

  ChangePasswordsPersonListModel::ChangePasswordsPersonListModel(
      QObject* parent)
      : QAbstractTableModel(parent) {}

  void ChangePasswordsPersonListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  void ChangePasswordsPersonListModel::update_username(
      int id, QString const& username)
  {
    for(int r = 0; r < static_cast<int>(rows_.size()); ++r) {
      if(rows_[r].id == id) {
        rows_[r].username = username;
        QModelIndex const idx = index(r, 4);
        Q_EMIT dataChanged(idx, idx, {Qt::DisplayRole});
        return;
      }
    }
  }

  int ChangePasswordsPersonListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int ChangePasswordsPersonListModel::columnCount(QModelIndex const&) const
  {
    return 5;
  }

  QVariant ChangePasswordsPersonListModel::data(QModelIndex const& idx,
                                                int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.id;
    case 1:
      return row.lastname;
    case 2:
      return row.firstname;
    case 3:
      return row.title;
    case 4:
      return row.username;
    default:
      return {};
    }
  }

  QVariant ChangePasswordsPersonListModel::headerData(int section,
                                                      Qt::Orientation o,
                                                      int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("Last name");
    case 2:
      return QCoreApplication::tr("First name");
    case 3:
      return QCoreApplication::tr("Title");
    case 4:
      return QCoreApplication::tr("Username");
    default:
      return {};
    }
  }

// --- DueLendingListModel -----------------------------------------------------

  DueLendingListModel::DueLendingListModel(QObject* parent)
      : QAbstractTableModel(parent) {}

  void DueLendingListModel::set_rows(std::vector<Row> rows)
  {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
  }

  DueLendingListModel::Row const&
  DueLendingListModel::row_at(int index) const
  {
    static Row const empty;
    if(index < 0 || index >= static_cast<int>(rows_.size())) {
      return empty;
    }
    return rows_[index];
  }

  int DueLendingListModel::rowCount(QModelIndex const&) const
  {
    return static_cast<int>(rows_.size());
  }

  int DueLendingListModel::columnCount(QModelIndex const&) const
  {
    return 8;
  }

  QVariant DueLendingListModel::data(QModelIndex const& idx, int role) const
  {
    if(role != Qt::DisplayRole || !idx.isValid()) {
      return {};
    }
    int const r = idx.row();
    if(r < 0 || r >= static_cast<int>(rows_.size())) {
      return {};
    }
    Row const& row = rows_[r];
    switch(idx.column()) {
    case 0:
      return row.lending_id;
    case 1:
      return row.duedate.isValid()
                 ? QLocale().toString(row.duedate, QLocale::ShortFormat)
                 : QVariant{};
    case 2:
      return row.person_name;
    case 3:
      return row.school_class;
    case 4:
      return row.outstanding_count;
    case 5:
      return row.item_name;
    case 6:
      return row.person_id;
    case 7:
      return row.item_id;
    default:
      return {};
    }
  }

  QVariant DueLendingListModel::headerData(int section,
                                           Qt::Orientation o,
                                           int role) const
  {
    if(o != Qt::Horizontal || role != Qt::DisplayRole) {
      return {};
    }
    switch(section) {
    case 1:
      return QCoreApplication::tr("due date");
    case 2:
      return QCoreApplication::tr("borrower");
    case 3:
      return QCoreApplication::tr("class");
    case 4:
      return QCoreApplication::tr("count");
    case 5:
      return QCoreApplication::tr("name");
    default:
      return {};
    }
  }

} // namespace colman

/* EOF */
