// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// try catch support.
/**
   Some macros which help to catch all C++ exception, write some output
   and (optional) set some status variable.
   The macros are able to distinguish betweenttt::Error derived exceptions
   and std::exception derived exceptions.
   Examples:
   \verbatim
   COLMAN_TRY {
   // some action
   }
   COLMAN_CATCH;
   \endverbatim

   \verbatim
   enum Status{Ok=0, Error=-1};
   Status status=Ok;
   COLMAN_TRY {
   // some action
   }
   COLMAN_INT_CATCH(status, Error);
   \endverbatim

   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TRY_CATCH_HPP
#  define COLMAN_TRY_CATCH_HPP

#  ifndef COLMAN_ERROR_HPP
#    include "colman_error.hpp"
#  endif
#  ifndef COLMAN_ANONYMOUS_VARIABLE_HPP
#    include "colman_anonymous_variable.hpp"
#  endif
#  ifndef SYSTEM_IOSTREAM
#    include <iostream>
#    define SYSTEM_IOSTREAM
#  endif
#  ifndef SYSTEM_SSTREAM
#    include <sstream>
#    define SYSTEM_SSTREAM
#  endif

#  ifndef COLMAN_TRY
/// Opens a try block; pair with one of the COLMAN_CATCH* macros.
#    define COLMAN_TRY try
#  endif // #ifndef COLMAN_TRY


#  ifndef COLMAN_CATCH_2
/**
   Base catch macro that handles both colman::Error and std::exception.

   Catches colman::Error and std::exception, writes a formatted error message
   to @p _stream_, and executes the supplied action expressions.

   @param _stream_   output stream to write the error message to.
   @param _pre_      statement executed before writing the error (e.g. setup).
   @param _mse_      statement executed after writing a colman::Error message.
   @param _fatal_    statement executed for fatal colman::Error.
   @param _base_     statement executed at the end of the colman::Error handler.
   @param _standard_ statement executed at the end of the std::exception handler.
*/
#    define COLMAN_CATCH_2(_stream_, _pre_, _mse_, _fatal_, _base_, _standard_) \
      catch(colman::Error const& e) {                                           \
        try {                                                                   \
          _pre_;                                                                \
          _stream_ << "ERROR:\n";                                               \
          e.print_message(_stream_);                                            \
          _stream_ << std::endl;                                                \
          _base_;                                                               \
        } catch(...) {                                                          \
        }                                                                       \
      }                                                                         \
      catch(std::exception const& e) {                                          \
        try {                                                                   \
          _pre_;                                                                \
          _stream_ << "ERROR:\n"                                                \
                   << e.what()                                                  \
                   << std::endl;                                                \
          _standard_;                                                           \
        } catch(...) {                                                          \
        }                                                                       \
      }                                                                         \
      void(0)
#  endif // #ifndef COLMAN_CATCH_2

#  ifndef COLMAN_CATCH
/// catch version which writes error messages to std::cerr.
#    define COLMAN_CATCH        \
      COLMAN_CATCH_2(std::cerr, \
                     (void)0,   \
                     (void)0,   \
                     (void)0,   \
                     (void)0,   \
                     (void)0)
#  endif // #ifndef COLMAN_CATCH


#  ifndef COLMAN_INT_CATCH
/// catch version which writes error messages to std::cerr and sets an integer.
#    define COLMAN_INT_CATCH(_retVal_, _err_) \
      COLMAN_CATCH_2(std::cerr,               \
                     (void)0,                 \
                     _retVal_ = _err_,        \
                     _retVal_ = _err_,        \
                     _retVal_ = _err_,        \
                     _retVal_ = _err_)
#  endif // #ifndef COLMAN_INT_CATCH


#  ifndef COLMAN_STRING_CATCH
/// catch version which sets a string.
#    define COLMAN_STRING_CATCH(_text_)                                                \
      COLMAN_CATCH_2(COLMAN_ANONYMOUS_LINE_VARIABLE(colman_catch2_),                   \
                     std::ostringstream COLMAN_ANONYMOUS_LINE_VARIABLE(colman_catch_), \
                     _text_ = COLMAN_ANONYMOUS_LINE_VARIABLE(colman_catch_).str(),     \
                     _text_ = COLMAN_ANONYMOUS_LINE_VARIABLE(colman_catch_).str(),     \
                     _text_ = COLMAN_ANONYMOUS_LINE_VARIABLE(colman_catch_).str(),     \
                     _text_ = COLMAN_ANONYMOUS_LINE_VARIABLE(colman_catch_).str())
#  endif // #ifndef COLMAN_STRING_CATCH

#endif /* #ifndef COLMAN_TRY_CATCH_HPP */

/* EOF */
