// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class AssertContext.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_assert_context.hpp"

/**
   Default constructor of the AssertContext class.
*/
colman::AssertContext::AssertContext()
    : level_(COLMAN_DEBUG_LEVEL)
{
}


/**
   Destructor of the AssertContext class.
   @exception none
*/
colman::AssertContext::~AssertContext()
{
}


/**
   Sets location where the assertion failed.
   Use the COLMAN_HERE macro with this member function.
*/
void colman::AssertContext::set_location(char const* location)
{
  location_ = (location) ? location : "";
}


/**
   Returns location where the assertion failed.
*/
std::string const& colman::AssertContext::get_location() const
{
  return location_;
}


/**
   Sets expression that failed.
*/
void colman::AssertContext::set_expression(std::string const& expr)
{
  expression_ = expr;
}


/**
   Returns expression that failed
*/
std::string const& colman::AssertContext::get_expression() const
{
  return expression_;
}


/**
   Add one value and corresponding string
*/
void colman::AssertContext::add_value(std::string const& value, std::string const& str)
{
  values_.push_back(StringPair(value, str));
}


/**
   Returns values and corresponding strings in a vector of string pairs.
*/
colman::AssertContext::StringPairVector const& colman::AssertContext::get_values() const
{
  return values_;
}


/**
   Sets the level of assertion to \a level.
 */
void colman::AssertContext::set_level(int level)
{
  level_ = level;
}


/**
   Gets the level of assertion.
 */
int colman::AssertContext::get_level() const
{
  return level_;
}


/**
   Sets a user friendly message.
 */
void colman::AssertContext::set_message(char const* msg)
{
  message_ = (msg) ? msg : "";
}


/**
   Sets a user friendly message.
 */
void colman::AssertContext::set_message(std::string const& msg)
{
  message_ = msg;
}


/**
   Gets a user friendly message.
 */
std::string const& colman::AssertContext::get_message() const
{
  return message_;
}

/* EOF */
