// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class AssertContext.
/**
   @see "Enhancing Assertions", Andrei Alexandrescu, John Torjo
   CUJ Expert Forum, August 2003,
   http://www.cuj.com/documents/s=8464/cujcexp0308alexandr/
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ASSERT_CONTEXT_HPP
#  define COLMAN_ASSERT_CONTEXT_HPP

#  ifndef SYSTEM_STRING
#    include <string>
#    define SYSTEM_STRING
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_UTILITY
#    include <utility>
#    define SYSTEM_UTILITY
#  endif

namespace colman
{
  /// Default assert levels used in AssertContext.
  enum {
    /// just loggs this assert (show a message)
    COLMAN_WARN_LEVEL = 100,
    /// asks the user what to do (ignore, retry, etc.)
    COLMAN_DEBUG_LEVEL = 200,
    /// throws an error
    COLMAN_ERROR_LEVEL = 300,
    /// dumps all assert context to console and aborts
    COLMAN_FATAL = 1000
  };


  /// This helper class contains details about a failed assertion.
  /**
     It stores the location, the failed expression, corresponding values,
     the level and a description. It is used in colman::Assert,
     @see ttt_assert.hpp
     @see "Enhancing Assertions", Andrei Alexandrescu, John Torjo
     CUJ Expert Forum, August 2003,
     http://www.cuj.com/documents/s=8464/cujcexp0308alexandr/
  */
  class AssertContext
  {
  public:
    /// Default constructor
    AssertContext();
    /// Destructor
    ~AssertContext();

    /// sets location where the assertion failed
    void set_location(char const* location);
    /// returns location where the assertion failed
    std::string const& get_location() const;

    /// sets the expression that failed
    void set_expression(std::string const& expr);
    /// returns the expression that failed
    std::string const& get_expression() const;

    /// pair of strings
    using StringPair = std::pair<std::string, std::string>;
    /// vector of string pairs
    using StringPairVector = std::vector<StringPair>;
    /// set one value and its corresponding string
    void add_value(std::string const& value, std::string const& str);
    /// return values as a vector of pairs (value, corresponding string)
    StringPairVector const& get_values() const;

    /// set level of assertion
    void set_level(int level);
    /// get level of assertion
    int get_level() const;

    /// set (user-friendly) message
    void set_message(char const* msg);
    /// set (user-friendly) message
    void set_message(std::string const& msg);
    /// get (user-friendly) message
    std::string const& get_message() const;

  private:
    std::string location_;    ///< location of assertion
    std::string expression_;  ///< assertion expression
    StringPairVector values_; ///< values and their corresponding string representations
    int level_;               ///< assertion level (e.g. COLMAN_ERROR_LEVEL)
    std::string message_;     ///< optional user-friendly message
  };

  /// assert function type
  using assert_f = void (*)(AssertContext const&);

} // namespace colman

#endif /* #ifndef COLMAN_ASSERT_CONTEXT_HPP */

/* EOF */
