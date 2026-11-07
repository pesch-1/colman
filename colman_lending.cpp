// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Lending.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_lending.hpp"

/**
   Sets the item id.
   @param item_id database id of the lent item.
*/
void colman::Lending::set_item_id(int item_id)
{
  item_id_ = item_id;
}


/**
   Sets the person id.
   @param person_id database id of the borrower.
*/
void colman::Lending::set_person_id(int person_id)
{
  person_id_ = person_id;
}


/**
   Sets the number of copies lent.
   @param count number of copies lent.
*/
void colman::Lending::set_count(unsigned int count)
{
  count_ = count;
}


/**
   Sets the lending date and time.
   @param date date and time the lending was made.
*/
void colman::Lending::set_date_time(QDateTime const& datetime)
{
  datetime_ = datetime;
}


/**
   Sets the optional due date.
   @param duedate date by which the items must be returned; may be null.
*/
void colman::Lending::set_duedate(QDate const& duedate)
{
  duedate_ = duedate;
}


/**
   Sets the remarks.
   @param remarks optional user-visible remarks about this lending.
*/
void colman::Lending::set_remarks(QString const& remarks)
{
  remarks_ = remarks;
}


/**
   Sets the data collector person id.
   @param data_collector_id database id of the data collector, or -1.
*/
void colman::Lending::set_data_collector_id(int data_collector_id)
{
  data_collector_id_ = data_collector_id;
}


/**
   Resets all members to initial values.
*/
void colman::Lending::clear()
{
  item_id_ = person_id_ = -1;
  count_ = 0u;
  duedate_ = QDate();
  datetime_ = QDateTime();
  remarks_.clear();
  data_collector_id_ = -1;
}

/* EOF */
