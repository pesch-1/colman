// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Enhanced assertions (as introduced by Alexandrescu & Torjo).
/**
   Main part of the assertion support is the #COLMAN_ASSERT macro.
   See below for a description and some examples.

   Customization of assertions can be done with the colman::Assert class
   described below.
 
   @see "Enhancing Assertions", Andrei Alexandrescu, John Torjo
   CUJ Expert Forum, August 2003,
   http://www.cuj.com/documents/s=8464/cujcexp0308alexandr/
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ASSERT_HPP
#  define COLMAN_ASSERT_HPP

#  ifndef COLMAN_ASSERT_CONTEXT_HPP
#    include "colman_assert_context.hpp"
#  endif
#  ifndef COLMAN_HERE_HPP
#    include "colman_here.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_UTILITY
#    include <utility>
#    define SYSTEM_UTILITY
#  endif
#  ifndef SYSTEM_SSTREAM
#    include <sstream>
#    define SYSTEM_SSTREAM
#  endif

namespace colman
{
  /// An assertion class.
  /**
     When an assertion fails, two things happen in sequence:
     - The failure information is being logged
     - The failure is being handled depending on its level.
     
     The two activities are completely orthogonal and can be customized in
     separation. You can customize logging by passing your own
     logger function to the static member function
     colman::Assert::set_log(colman::asert_f). 
     Or you can use colman::Assert::set_log(char const* filename)
     to write into a file or colman::Assert::set_log(std::ostream& out)
     to write to a stream. By default logging is switched off.
   
     There are four levels of assertions:
     - COLMAN_WARN_LEVEL (it's just a warning, the program may continue)
     - COLMAN_DEBUG_LEVEL (it's the default, the usual assert)
     - COLMAN_ERROR_LEVEL (an error)
     - COLMAN_FATAL (it's fatal, program got unstable).
     
     Each level can be handled differently. Thus we have one handle per level.
     The defaults are
     - Warning: dump a message and continue program
     - Debug: dump a message and ask the user what to do (ignore/debug/etc.)
     - Error: throw an exception containing a message
     - Fatal: dump a message and abort program
     
     You can rely on the defaults, or change the handler with the
     colman::Assert::set_handler() member function, e.g.:
     \verbatim
     colman::Assert::set_handler(colman::COLMAN_ERROR_LEVEL, my_error_handler);
     \endverbatim
     You can use the assertions with the #COLMAN_ASSERT and #COLMAN_VERIFY
     macros described below.
     
     @see "Enhancing Assertions", Andrei Alexandrescu, John Torjo
     CUJ Expert Forum, August 2003,
     http://www.cuj.com/documents/s=8464/cujcexp0308alexandr/
  */
  class Assert
  {
  public:
    /// Default constructor
    Assert(char const* expr = nullptr);
    /// Copy constructor
    Assert(Assert const& obj);
    /// Destructor
    ~Assert()
#  if(defined(__cplusplus) && __cplusplus >= 201103L)
        noexcept(false)
#  endif
        ;

    /**
       Sets the current value to \a val and the corresponding message to \a m.
    */
    template <typename T>
    inline Assert& set_current_value(T const& val, char const* mesg)
    {
      std::ostringstream ost;
      ost << val;
      context_.add_value(ost.str(), mesg);
      return *this;
    }
    /**
       Sets the current value to \a val and the corresponding message to \a m.
    */
    inline Assert& set_current_value(QString const& val, char const* mesg)
    {
      std::ostringstream ost;
      ost << val.toStdString();
      context_.add_value(ost.str(), mesg);
      return *this;
    }
    /// char* version of set_current value (0 pointer check)
    Assert& set_current_value(char* const& val, char const* mesg);
    /// char const* version of set_current value (0 pointer check)
    Assert& set_current_value(char const* const& val,
                              char const* mesg);
    /// std::string version of set_current value
    Assert& set_current_value(std::string const& val,
                              char const* mesg);

    /// Sets the source location of the failed assertion.
    /**
       @param location location string, typically produced by the #COLMAN_HERE macro.
       @return reference to this object.
    */
    Assert& set_location(char const* location);
    /// sets message
    Assert& msg(char const* mesg);
    /// sets message, string version
    Assert& msg(std::string const& mesg);
    /// sets level and message
    Assert& level(int level, char const* mesg = 0);
    /// sets level and message
    Assert& level(int level, std::string const& mesg);
    /// shortcut for level(COLMAN_WARN_LEVEL, mesg)
    Assert& warn(char const* mesg = 0);
    /// shortcut for level(COLMAN_WARN_LEVEL, mesg)
    Assert& warn(std::string const& mesg);
    /// shortcut for level(COLMAN_DEBUG_LEVEL, mesg)
    Assert& debug(char const* strMsg = 0);
    /// shortcut for level(COLMAN_DEBUG_LEVEL, mesg)
    Assert& debug(std::string const& strMsg);
    /// shortcut for level(COLMAN_ERROR_LEVEL, mesg)
    Assert& error(char const* strMsg = 0);
    /// shortcut for level(COLMAN_ERROR_LEVEL, mesg)
    Assert& error(std::string const& strMsg);
    /// shortcut for level(COLMAN_FATAL, mesg)
    Assert& fatal(char const* strMsg = 0);
    /// shortcut for level(COLMAN_FATAL, mesg)
    Assert& fatal(std::string const& strMsg);

    /// some magic explained in the above mentioned online article
    Assert& COLMAN_ASSERT_A;
    /// some magic explained in the above mentioned online article
    Assert& COLMAN_ASSERT_B;

    /// set the default logger, and make it write everything to a file
    static void set_log(char const* filename);
    /// set the default logger, and make it write everything to a stream
    static void set_log(std::ostream& out);
    /// set logger
    static void set_log(assert_f log);
    /// set handler
    static void set_handler(int level, assert_f handler);

  private:
    AssertContext context_;        ///< assertion context
    mutable bool needsHandling_ = true; ///< true if this object is responsible for handling the assertion

    /// handles the current assertion.
    void handle_assert();
  };


  /// Creates a new anonymous Assert object.
  /**
     The constructor of this object takes \a expr as its argument.
  */
  inline Assert make_assert(char const* expr /**< assertion expression */)
  {
    return Assert(expr);
  }


  /// default handler for debug level
  void default_debug_handler(AssertContext const& context);
  /// default handler for warning level
  void default_warn_handler(AssertContext const& context);
  /// default handler for error level
  void default_error_handler(AssertContext const& context);
  /// default handler for fatal level
  void default_fatal_handler(AssertContext const& context);

} // namespace colman


/// Enable/disable assertions.
/**
   Default is that assertions are disabled in release mode and enabled
   in debug mode.
   In order to force assertions enabled (even in release mode) use
   \verbatim
   #define COLMAN_ASSERT_DEBUG_MODE 1
   \endverbatim
   In order to force assertions disabled (even in debug mode) use 
   \verbatim
   #define COLMAN_ASSERT_DEBUG_MODE 0
   \endverbatim
*/
#  ifdef COLMAN_ASSERT_DEBUG_MODE
#    if COLMAN_ASSERT_DEBUG_MODE == 1
#      define COLMAN_ASSERT_DEBUG
#    else
#      undef COLMAN_ASSERT_DEBUG
#    endif
#  else
// defaults
#    ifndef NDEBUG
#      define COLMAN_ASSERT_DEBUG
#    else
#      undef COLMAN_ASSERT_DEBUG
#    endif
#  endif


#  ifdef COLMAN_ASSERT_DEBUG
// Debug mode
/// An assertion mechanism.
/**
   It is best explained with an example:
   You want to assert that two strings s1 and s2 are empty.
   If not, you want an error message which includes the current values
   of the strings. This can be done like this:
   \verbatim
   std::string s1("first"), s2("second");
   COLMAN_ASSERT(s1.empty() && s2.empty())(s1)(s2);
   \endverbatim
   If the assertion fails, the message displayed and logged will look
   like this:
   \verbatim
   Assertion failed in myexample.cpp: 42
   Expression: 's1.empty() && s2.empty()'
   Values: s1 = "first"
   s2 = "second"
   \endverbatim
   The list of values that should be displayed may be extended easily
   by simply appending them (s3)(s4)...
 
   By default COLMAN_ASSERT is active in debug mode only and switched off
   in release mode. Therefor this macro should be used for internal
   consistency checking while testing the software. COLMAN_ASSERT may be
   switched on in release mode by defining #COLMAN_ASSERT_DEBUG_MODE as 1.
   If you want to catch wrong usage of your software you should use
   #COLMAN_VERIFY or #COLMAN_ENFORCE instead. #COLMAN_VERIFY is like
   COLMAN_ASSERT but active in release mode, too. By default it throws
   an exception.
  
   User friendly messages can be done the following way:
   \verbatim
   COLMAN_ASSERT(nUsers < 100)(nUsers).msg("too many users!");
   \endverbatim
   To set the assertion level you may use the level member function:
   \verbatim
   COLMAN_ASSERT(users < 100)(users).level(COLMAN_ERROR_LEVEL,"too many users");
   \endverbatim
   or easier one of the following helpers:
   \verbatim
   COLMAN_ASSERT(users < 100)(users).warn("too many users");
   COLMAN_ASSERT(users < 100)(users).debug("too many users");
   COLMAN_ASSERT(users < 100)(users).error("too many users");
   COLMAN_ASSERT(users < 100)(users).fatal("too many users");
   \endverbatim
*/
#    define COLMAN_ASSERT(expr)                                              \
      if((expr))                                                             \
        ;                                                                    \
      else                                                                   \
        colman::make_assert(#expr).set_location(COLMAN_HERE).COLMAN_ASSERT_A \
         /**/

#  else // #ifdef COLMAN_ASSERT_DEBUG
// Release mode (do nothing):
#    define COLMAN_ASSERT(expr)                 \
      if(true)                                  \
        ;                                       \
      else                                      \
        colman::make_assert("").COLMAN_ASSERT_A \
         /**/
#  endif // #ifdef COLMAN_ASSERT_DEBUG

/// An assertion mechanism for release mode.
/**
   COLMAN_VERIFY acts like #COLMAN_ASSERT, with two differences:
   it works in both debug and release mode and the default
   level is error instead of debug.

   The main difference to the #COLMAN_ENFORCE macro is the missing property
   of returning the tested object. With #COLMAN_ENFORCE you may do the
   following:

   \verbatim
   std::ofstream out("filename");
   ...
   COLMAN_ENFORCE(out)("could not open filename") << "some string" << std::endl;
   \endverbatim
   
   This is not possible with COLMAN_VERIFY. But notice that this feature
   has a major drawback. If you want to compute the error message and do
   not use a string constant, e.g. COLMAN_ENFORCE(true)(my_function()),
   then this function will be invoked even if the test succeeds. In the
   above example the function my_function() will be invoked and its output
   ignored. With COLMAN_VERIFY(true).msg(my_function()) the function will
   not be invoked. Beside this, there are more less important differences:
   - COLMAN_VERIFY can be customized like described in the Assert class,
   - COLMAN_VERIFY accepts strings only for descriptions, while
   #COLMAN_ENFORCE handles arbitrary types for which the output operator <<
   is defined,
   - COLMAN_VERIFY may be customized to log the error message.
   @see #COLMAN_ENFORCE
*/
#  define COLMAN_VERIFY(expr)                                                      \
    if((expr))                                                                     \
      ;                                                                            \
    else                                                                           \
      colman::make_assert(#expr).error().set_location(COLMAN_HERE).COLMAN_ASSERT_A \
         /**/

/// Helper assertion macros.
/**
   The following macros provide something like an "infinitely expansible
   macro mechanism". When you invoke COLMAN_ASSERT_A(xyz), it will expand
   into something that ends with COLMAN_ASSERT_B and the other way round.
   During the expansion the two macros grab the value xyz and its
   representation as string.
   The #COLMAN_ASSERT macro starts the tandem action of the two macros
   COLMAN_ASSERT_A and COLMAN_ASSERT_B.
   @see "Enhancing Assertions", Andrei Alexandrescu, John Torjo
   CUJ Expert Forum, August 2003,
*/
#  define COLMAN_ASSERT_A(x) COLMAN_ASSERT_OP(x, B)
/**
   @see #COLMAN_ASSERT_A
*/
#  define COLMAN_ASSERT_B(x) COLMAN_ASSERT_OP(x, A)
/**
   @see #COLMAN_ASSERT_A
*/
#  define COLMAN_ASSERT_OP(x, next) \
    COLMAN_ASSERT_A.set_current_value((x), #x).COLMAN_ASSERT_##next

#endif /* #ifndef COLMAN_ASSERT_HPP */

/* EOF */
