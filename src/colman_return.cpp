// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Return.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_return.hpp"

/**
   Resets all members to initial values.
*/
void colman::Return::clear()
{
  lending_id_ = 0;
  count_ = 0u;
  datetime_ = QDateTime();
  remarks_.clear();
  data_collector_id_ = -1;
}


/**
   Sets the lending id.
   @param lending_id database id of the lending this return is applied to.
*/
void colman::Return::set_lending_id(int lending_id)
{
  lending_id_ = lending_id;
}


/**
   Sets the number of copies returned.
   @param count number of copies returned.
*/
void colman::Return::set_count(unsigned int count)
{
  count_ = count;
}


/**
   Sets the return datetime.
   @param datetime date and time of the return.
*/
void colman::Return::set_date_time(QDateTime const& datetime)
{
  datetime_ = datetime;
}


/**
   Sets the remarks.
   @param remarks optional user-visible remarks about this return.
*/
void colman::Return::set_remarks(QString const& remarks)
{
  remarks_ = remarks;
}


/**
   Sets the data collector person id.
   @param data_collector_id database id of the data collector, or -1.
*/
void colman::Return::set_data_collector_id(int data_collector_id)
{
  data_collector_id_ = data_collector_id;
}

/* EOF */
