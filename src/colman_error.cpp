// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Error.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_error.hpp"
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif

/**
   Default constructor.
 */
colman::Error::Error()
{
}


/**
   Constructor taking description and location given as std::string
   as arguments.
*/
colman::Error::Error(std::string const& desc, std::string const& loc)
    : description_(desc),
      location_(loc)
{
}


/**
   Constructor taking description and location given as c-string as arguments.
*/
colman::Error::Error(char const* desc, char const* loc)
    : description_((desc) ? desc : ""),
      location_((loc) ? loc : "")
{
}


/**
   Error constructor.
*/
colman::Error::Error(Error const& obj)
    : description_(obj.description_),
      location_(obj.location_)
{
}


/**
   Copy assignment operator.
   @param obj source object to copy from.
   @return reference to this object.
*/
colman::Error& colman::Error::operator=(Error const& obj)
{
  if(this != &obj) {
    description_ = obj.description_;
    location_ = obj.location_;
  }
  return *this;
}


/**
   Destructor. Empty.
*/
colman::Error::~Error()
{
}


/**
   Add \a text to error description
 */
colman::Error& colman::Error::operator()(std::string const& text)
{
  description_ += text;
  return *this;
}


/**
   Appends @p text to the error description.
   @param text string to append.
*/
void colman::Error::append_to_description(std::string const& text)
{
  description_ += text;
}


/**
   Returns an error description which has been set in the constructor.
*/
std::string const& colman::Error::get_description() const
{
  return description_;
}


/**
   Returns the location description which has been set in the constructor
*/
std::string const& colman::Error::get_location() const
{
  return location_;
}


/**
   Returns a C-string description (std::exception interface).
*/
char const* colman::Error::what() const noexcept
{
  return description_.c_str();
}


/**
   Prints an error message to out.
*/
void colman::Error::print_message(std::ostream& out) const
{
  out << description_;
  if(!location_.empty()) {
    out << "\nThe exception was thrown in " << location_ << '.';
  }
  out << std::endl;
}

/* EOF */
