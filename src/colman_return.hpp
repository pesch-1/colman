// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Return.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_RETURN_HPP
#  define COLMAN_RETURN_HPP

#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{
  /// Plain data class representing a return transaction.
  /**
     Records that a given number of copies belonging to a lending were returned
     on a certain date and time, with optional remarks.
  */
  class Return
  {
  public:
    inline int get_lending_id() const { return lending_id_; }    ///< Returns the associated lending id.
    inline unsigned int get_count() const { return count_; }     ///< Returns the number of copies returned.
    inline QDateTime const& get_date_time() const { return datetime_; }     ///< Returns the return date and time.
    inline QString const& get_remarks() const { return remarks_; } ///< Returns remarks.
    inline int get_data_collector_id() const { return data_collector_id_; } ///< Returns the data collector person id, or -1.

    /// Resets all fields to default values.
    void clear();
    /// Sets the database id of the associated lending.
    void set_lending_id(int lending_id);
    /// Sets the number of copies returned.
    void set_count(unsigned int count);
    /// Sets the return date and time.
    void set_date_time(QDateTime const& datetime);
    /// Sets remarks for this return.
    void set_remarks(QString const& remarks);
    /// Sets the data collector person id (-1 if unknown).
    void set_data_collector_id(int data_collector_id);

  private:
    int lending_id_ = 0;         ///< database id of the associated lending
    unsigned int count_ = 0;     ///< number of copies returned
    QDateTime datetime_;       ///< date and time of return
    QString remarks_;          ///< remarks
    int data_collector_id_ = -1; ///< person id of the data collector, -1 if unset
  };

} // namespace colman

#endif /* #ifndef COLMAN_RETURN_HPP */

/* EOF */
