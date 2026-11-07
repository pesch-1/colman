// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Acquisition.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_acquisition.hpp"

/**
   Resets all members to initial values.
*/
void colman::Acquisition::clear()
{
  item_id_ = -1;
  count_ = 0u;
  datetime_ = QDateTime();
  data_collector_id_ = -1;
}

/**
   Sets the item id.
   @param item_id database id of the item acquired.
*/
void colman::Acquisition::set_item_id(int item_id)
{
  item_id_ = item_id;
}


/**
   Sets the number of copies acquired.
   @param count number of copies acquired.
*/
void colman::Acquisition::set_count(unsigned int count)
{
  count_ = count;
}


/**
   Sets the acquisition date and time.
   @param datetime date and time of the acquisition.
*/
void colman::Acquisition::set_date_time(QDateTime const& datetime)
{
  datetime_ = datetime;
}


/**
   Sets the data collector person id.
   @param data_collector_id database id of the data collector, or -1.
*/
void colman::Acquisition::set_data_collector_id(int data_collector_id)
{
  data_collector_id_ = data_collector_id;
}

/* EOF */
