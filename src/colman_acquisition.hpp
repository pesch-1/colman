// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Acquisition.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ACQUISITION_HPP
#  define COLMAN_ACQUISITION_HPP

#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif

namespace colman
{
  /// Plain data class representing an acquisition (stock increase).
  /**
     Records that a number of copies of an item were added to stock
     on a certain date.
  */
  class Acquisition
  {
  public:
    inline int get_item_id() const { return item_id_; }        ///< Returns the item database id.
    inline unsigned int get_count() const { return count_; }   ///< Returns the number of copies acquired.
    inline QDateTime const& get_date_time() const { return datetime_; } ///< Returns the acquisition date and time.
    inline int get_data_collector_id() const { return data_collector_id_; } ///< Returns the data collector person id, or -1.

    /// Resets all fields to default values.
    void clear();
    /// Sets the database id of the acquired item.
    void set_item_id(int item_id);
    /// Sets the number of copies acquired.
    void set_count(unsigned int count);
    /// Sets the acquisition date and time.
    void set_date_time(QDateTime const& datetime);
    /// Sets the data collector person id (-1 if unknown).
    void set_data_collector_id(int data_collector_id);

  private:
    int item_id_ = -1;           ///< database id of the item
    unsigned int count_ = 0;     ///< number of copies acquired
    QDateTime datetime_;       ///< acquisition date and time
    int data_collector_id_ = -1; ///< person id of the data collector, -1 if unset
  };
} // namespace colman

#endif /* #ifndef COLMAN_ACQUISITION_HPP */

/* EOF */
