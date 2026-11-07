// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Lending.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_LENDING_HPP
#  define COLMAN_LENDING_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif

namespace colman
{
  /// Plain data class representing a lending transaction.
  /**
     Records that a person borrowed a given number of items on a certain date,
     with an optional due date and remarks.
  */
  class Lending
  {
  public:
    /// Sets the database id of the lent item.
    void set_item_id(int item_id);
    /// Sets the database id of the borrowing person.
    void set_person_id(int person_id);
    /// Sets the number of copies lent.
    void set_count(unsigned int count);
    /// Sets the lending date.
    void set_date_time(QDateTime const& datetime);
    /// Sets the optional due date.
    void set_duedate(QDate const& duedate);
    /// Sets remarks for this lending.
    void set_remarks(QString const& remarks);
    /// Sets the data collector person id (-1 if unknown).
    void set_data_collector_id(int data_collector_id);
    /// Resets all fields to default values.
    void clear();

    inline int get_item_id() const { return item_id_; }          ///< Returns the item database id.
    inline int get_person_id() const { return person_id_; }      ///< Returns the person database id.
    inline unsigned int get_count() const { return count_; }     ///< Returns the number of copies lent.
    inline QDateTime const& get_date_time() const { return datetime_; } ///< Returns the lending date and time.
    inline QDate const& get_duedate() const { return duedate_; } ///< Returns the optional due date.
    inline QString const& get_remarks() const { return remarks_; } ///< Returns remarks.
    inline int get_data_collector_id() const { return data_collector_id_; } ///< Returns the data collector person id, or -1.

  private:
    int item_id_ = -1;          ///< id of item in database file
    unsigned int count_ = 0;    ///< number of items lent
    int person_id_ = -1;        ///< id of person in database file
    QDateTime datetime_;      ///< date and time of lending
    QDate duedate_;           ///< (optional) due date
    QString remarks_;         ///< remarks concerning lending
    int data_collector_id_ = -1; ///< person id of the data collector, -1 if unset
  };

} // namespace colman

#endif /* #ifndef COLMAN_LENDING_HPP */

/* EOF */
