// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// QAbstractTableModel subclasses backed by std::vector<Row>.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TABLE_MODELS_HPP
#  define COLMAN_TABLE_MODELS_HPP

#  ifndef SYSTEM_QABSTRACTTABLEMODEL
#    include <QAbstractTableModel>
#    define SYSTEM_QABSTRACTTABLEMODEL
#  endif
#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef COLMAN_TYPE_HPP
#    include "colman_type.hpp"
#  endif
#  ifndef COLMAN_PERSON_HPP
#    include "colman_person.hpp"
#  endif

namespace colman
{
  /// Read-only table model backed by a list of Type objects.
  /**
     Presents three columns: id (0), tablename (1), and display name (2).
     Columns 0 and 1 are intended to be hidden in the view.
  */
  class TypeListModel : public QAbstractTableModel
  {
  public:
    explicit TypeListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_types(std::vector<Type> types);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Type> types_;
  };

  /// Read-only table model backed by a list of Person objects.
  /**
     Presents six columns: id (0, hidden), lastname (1), firstname (2),
     school class (3), title (4), username (5).
  */
  class PersonListModel : public QAbstractTableModel
  {
  public:
    static constexpr int ClassColumn = 3;  ///< column of the school class
    static constexpr int TitleColumn = 4;  ///< column of the title
    explicit PersonListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_persons(std::vector<Person> persons);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Person> persons_;
  };

  /// Read-only table model for the login page (five columns including password flag).
  /**
     Columns: 0=id (hidden), 1=last name, 2=first name, 3=title,
     4=has_password (hidden, used by HasPasswordDelegate).
  */
  class StartPagePersonListModel : public QAbstractTableModel
  {
  public:
    /// One row of the login-page person table.
    struct Row
    {
      int id = 0;
      QString lastname;
      QString firstname;
      QString title;
      bool has_password = false;
    };

    explicit StartPagePersonListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Read-only table model backed by a list of item rows.
  /**
     Presents seven columns: 0=id (hidden), 1=name, 2=count, 3=available,
     4=type translated (hidden when locale unsupported), 5=type name
     (hidden when locale supported), 6=collector (hidden by default).
  */
  class ItemListModel : public QAbstractTableModel
  {
  public:
    /// One row of an item list.
    struct Row
    {
      int id = 0;
      QString name;
      int count = 0;
      int available = 0;
      QString type_translated; ///< localised type name (may be empty)
      QString type_name;       ///< raw type name from types table
      QString collector;       ///< data-collector display name (may be empty)
    };

    explicit ItemListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);
    /// Appends rows to the end of the model.
    void append_rows(std::vector<Row> rows);

    /// If enabled, rows with available == 0 are rendered in grey.
    void set_grayout_unavailable(bool enable);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
    bool grayout_unavailable_ = false;
  };

  /// Model for open (unreturned) lendings (return page).
  class OpenLendingListModel : public QAbstractTableModel
  {
  public:
    /// One row of an open-lending list.
    struct Row
    {
      int id = 0;
      int lent_count = 0;
      int returned_count = 0;
      QDateTime datetime;
      QDate duedate;
      QString remarks;
      QString person_name;
    };

    explicit OpenLendingListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };


  /// Model for the acquisition history view.
  class HistoryAcquisitionListModel : public QAbstractTableModel
  {
  public:
    /// One row: id, date, count, item_name, collector.
    struct Row
    {
      int id = 0;
      QDateTime datetime;
      int count = 0;
      QString item_name;
      QString collector;
    };

    explicit HistoryAcquisitionListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Model for the liquidation history view.
  class HistoryLiquidationListModel : public QAbstractTableModel
  {
  public:
    /// One row: id, date, count, item_name, collector.
    struct Row
    {
      int id = 0;
      QDateTime datetime;
      int count = 0;
      QString item_name;
      QString collector;
    };

    explicit HistoryLiquidationListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Model for the lending history view.
  class HistoryLendingListModel : public QAbstractTableModel
  {
  public:
    /// One row: id, date, count, item_name, lastname, firstname, duedate, collector.
    struct Row
    {
      int id = 0;
      QDateTime datetime;
      int count = 0;
      QString item_name;
      QString lastname;
      QString firstname;
      QDate duedate;
      QString collector;
    };

    explicit HistoryLendingListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Model for the return history view.
  class HistoryReturnListModel : public QAbstractTableModel
  {
  public:
    /// One row: id, date, count, item_name, lastname, firstname, collector.
    struct Row
    {
      int id = 0;
      QDateTime datetime;
      int count = 0;
      QString item_name;
      QString lastname;
      QString firstname;
      QString collector;
    };

    explicit HistoryReturnListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Model for the combined history view.
  class HistoryCombinedModel : public QAbstractTableModel
  {
  public:
    /// One row: id, datetime, action, item_name, count, user_name, collector,
    /// item_id, person_id, remarks, collector_id, duedate.
    struct Row
    {
      int id = 0;
      QDateTime datetime;
      QString action;
      QString item_name;
      int count = 0;
      QString user_name;
      QString collector;
      int item_id = 0;
      int person_id = 0;
      QString remarks;
      int collector_id = 0;   ///< id of the data collector (0 = administrator)
      QDate duedate;        ///< due date (lending rows only)
    };

    explicit HistoryCombinedModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);
    /// Appends rows to the end of the model.
    void append_rows(std::vector<Row> rows);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Model for the change-passwords admin page.
  /**
     Presents four columns: 0=id (hidden), 1=last name, 2=first name,
     3=title.  The Administrator entry (id 0) is always the first row.
  */
  class ChangePasswordsPersonListModel : public QAbstractTableModel
  {
  public:
    /// One row: id, lastname, firstname, title, username.
    struct Row
    {
      int id = 0;
      QString lastname;
      QString firstname;
      QString title;
      QString username;
    };

    explicit ChangePasswordsPersonListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);

    /// Updates the username for the row with the given id in-place.
    void update_username(int id, QString const& username);

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

  /// Model for the open / due lendings page.
  /**
     Presents eight columns:
     0 = lending_id (hidden), 1 = duedate, 2 = person_name,
     3 = school_class, 4 = outstanding_count, 5 = item_name,
     6 = person_id (hidden), 7 = item_id (hidden).
  */
  class DueLendingListModel : public QAbstractTableModel
  {
  public:
    /// One row of the due/open lendings list.
    struct Row
    {
      int lending_id = 0;
      int item_id = 0;
      int person_id = 0;
      QDate duedate;
      QString person_name;   ///< "lastname, firstname"
      QString school_class;  ///< school class of the person, may be empty
      int outstanding_count = 0;
      QString item_name;

      /// Returns "lastname, firstname (class)", or just the name if
      /// the person has no class.
      QString person_with_class() const
      {
        return school_class.isEmpty()
                   ? person_name
                   : person_name + " (" + school_class + ")";
      }
    };

    explicit DueLendingListModel(QObject* parent = nullptr);

    /// Replaces the data and resets the model.
    void set_rows(std::vector<Row> rows);
    /// Returns the row at \a index, or a default row if out of range.
    Row const& row_at(int index) const;

    int rowCount(QModelIndex const& = {}) const override;
    int columnCount(QModelIndex const& = {}) const override;
    QVariant data(QModelIndex const& idx,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation o,
                        int role = Qt::DisplayRole) const override;

  private:
    std::vector<Row> rows_;
  };

} // namespace colman

#endif /* #ifndef COLMAN_TABLE_MODELS_HPP */

/* EOF */
