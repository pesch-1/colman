// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of colman::Error class
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ERROR_HPP
#  define COLMAN_ERROR_HPP

#  ifndef COLMAN_HERE_HPP
#    include "colman_here.hpp"
#  endif
#  ifndef SYSTEM_EXCEPTION
#    include <exception>
#    define SYSTEM_EXCEPTION
#  endif
#  ifndef SYSTEM_IOSFWD
#    include <iosfwd>
#    define SYSTEM_IOSFWD
#  endif
#  ifndef SYSTEM_STRING
#    include <string>
#    define SYSTEM_STRING
#  endif

namespace colman
{
  /// Exception base class.
  /**
     An Error object stores an error description and a description
     of the location where the exception has been thrown.
     You may use the macro COLMAN_HERE to set the location description.
     Example:
     \verbatim
     throw colman::Error("my error description", COLMAN_HERE);
     \endverbatim
  */
  class Error : public std::exception
  {
  public:
    /// constructor taking description and location as arguments
    Error(std::string const& desc, std::string const& loc = "");
    /// constructor taking description and location as arguments
    Error(char const* desc, char const* loc);
    /// copy constructor
    Error(Error const& obj);
    /// assignment operator
    Error& operator=(Error const& obj);
    /// destructor
    virtual ~Error();

    /// add some text to error description
    Error& operator()(std::string const& text);

    /// return C-string description (std::exception interface)
    char const* what() const noexcept override;
    /// return error description
    virtual std::string const& get_description() const;
    /// return location
    virtual std::string const& get_location() const;
    /// print error message to stream
    virtual void print_message(std::ostream& out) const;

    /// appends some text to the error description
    void append_to_description(std::string const& text);

  private:
    // hidden default constructor
    Error();
    /// error description
    std::string description_;
    /// errror location
    std::string location_;
  };

} // namespace colman

#endif /* #ifndef COLMAN_ERROR_HPP */

/* EOF */
