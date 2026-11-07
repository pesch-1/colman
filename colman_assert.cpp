// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class Assert.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_assert.hpp"
#ifndef COLMAN_ERROR_HPP
#  include "colman_error.hpp"
#endif
#if(defined _MSC_VER && _MSC_VER > 1000)
// *this used in base-member initialization; it's ok
#  pragma warning(disable : 4355)
#else
#  if(defined HAVE_UNISTD_H && HAVE_UNISTD_H == 1)
#    include <unistd.h>
#    include <sys/types.h>
#  endif
#endif // _MSC_VER > 1000
#ifndef SYSTEM_FSTREAM
#  include <fstream>
#  define SYSTEM_FSTREAM
#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef SYSTEM_SET
#  include <set>
#  define SYSTEM_SET
#endif
#ifndef SYSTEM_MAP
#  include <map>
#  define SYSTEM_MAP
#endif
#ifndef SYSTEM_CSTDLIB
#  include <cstdlib>
#  define SYSTEM_CSTDLIB
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif

namespace
{

  // helpers
  /**
     @return string describing level type for \a level.
   */
  std::string get_typeof_level(int level)
  {
    switch(level) {
    case colman::COLMAN_WARN_LEVEL:
      return "Warning";
    case colman::COLMAN_DEBUG_LEVEL:
      return "Assertion failed";
    case colman::COLMAN_ERROR_LEVEL:
      return "Assertion failed (Error)";
    case colman::COLMAN_FATAL:
      return "Assertion failed (FATAL)";
    default: {
      std::ostringstream out;
      out << "Assertion failed (level=" << level << ")";
      return out.str();
    }
    }
  }


  /**
     Dumps details of the assertion context \a context to the stream \a out.
   */
  void dump_context_detail(colman::AssertContext const& context,
                           std::ostream& out)
  {
    out << get_typeof_level(context.get_level())
        << " in " << context.get_location() << '\n';
    if(!context.get_message().empty()) {
      out << "message: '"
          << context.get_message() << "'\n";
    }
    out << "Expression: '" << context.get_expression() << "'";

    typedef colman::AssertContext::StringPairVector SPVector;
    SPVector const& values = context.get_values();
    SPVector::const_iterator v = values.begin();
    if(v != values.end()) {
      out << "\nValues: "
          << v->second << "='" << v->first << "'";
      for(++v; v != values.end(); ++v) {
        out << "\n        "
            << v->second << "='" << v->first << "'";
      }
    }
    out << '\n';
    out.flush();
  }


  /// in case we're logging using the default logger
  struct StreamHolder
  {
    /// default constructor
    StreamHolder()
        : out_(nullptr),
          free_(nullptr)
    {
    }
    /// destructor
    ~StreamHolder()
    {
      out_ = nullptr;
    }
    std::ostream* out_;  /**< pointer to output stream */
    /// pointer to stream which needs to be freed
    std::unique_ptr<std::ostream> free_;

  private:
    StreamHolder(StreamHolder const&); // disabled
  };


  /**
     Initializes and holds the default stream.
  */
  StreamHolder& default_logger_info()
  {
    static StreamHolder holder;
    return holder;
  }


  /// The default logger.
  void default_logger(colman::AssertContext const& context)
  {
    if((default_logger_info().out_)) {
      dump_context_detail(context, *(default_logger_info().out_));
    }
  }


  /// sets the default logger to write to \a out
  void set_default_log_stream(std::ostream& out)
  {
    default_logger_info().out_ = &out;
    default_logger_info().free_ = std::unique_ptr<std::ostream>(nullptr);
  }


  /// sets the default logger to write to file \a filename
  void set_default_log_name(char const* filename)
  {
    default_logger_info().free_ = std::unique_ptr<std::ostream>(nullptr);
    default_logger_info().free_ = std::make_unique<std::ofstream>(filename);
    default_logger_info().out_ = default_logger_info().free_.get();
  }


  /// the logger
  colman::assert_f& logger()
  {
    static colman::assert_f inst = &default_logger;
    return inst;
  }


  // -----------------------------------------------------------------------
  // handlers
  // -----------------------------------------------------------------------

  /**
     Invoke debugger. 
   */
  void break_into_debugger()
  {
#if defined(__WIN32__) && !defined(UNDER_CE)
    __asm { int 3 }
#elif(defined HAVE_UNISTD_H && HAVE_UNISTD_H == 1)
    pid_t pid = getpid();
    pid_t forkPid = fork();
    if(-1 == forkPid) {
      // fork failed
      std::cerr << "sorry, could not start debugger" << std::endl;
      //__asm ("int $0x3");
    }
    else if(0 == forkPid) {
      // childs process
      std::ostringstream ost1, ost2;
      ost1 << "/proc/" << pid << "/exe ";
      ost2 << pid;
      std::string const s1 = ost1.str(), s2 = ost2.str();
      char const* const argv[] = {"-nw", s1.c_str(), s2.c_str(), nullptr};
      (void)execvp("gdb", (char* const*)argv);
      std::cerr << "sorry, could not start gdb" << std::endl;
      exit(1);
    }
    else {
      //parents process
      bool loop = true;
      while(loop) {
      }
    }
#else
    std::cerr << "no debugger available" << std::endl;
#endif
  }

  /**
     A mapping from integers to assert functions.
     It is used to map assertion levels to handlers.
   */
  struct HandlersCollection : public std::map<int, colman::assert_f>
  {
    HandlersCollection()
        : std::map<int, colman::assert_f>()
    {
      (*this)[colman::COLMAN_WARN_LEVEL] = &colman::default_warn_handler;
      (*this)[colman::COLMAN_DEBUG_LEVEL] = &colman::default_debug_handler;
      (*this)[colman::COLMAN_ERROR_LEVEL] = &colman::default_error_handler;
      (*this)[colman::COLMAN_FATAL] = &colman::default_fatal_handler;
    }
  };

  /**
     @return reference to static HandlersCollection object.
   */
  HandlersCollection& handlers()
  {
    static HandlersCollection inst;
    return inst;
  }

  /**
     @return handler for given assertion level \a level.
   */
  colman::assert_f get_handler(int level)
  {
    HandlersCollection::const_iterator found = handlers().find(level);
    if(found != handlers().end()) {
      return found->second;
    }
    else {
      // we always assume the debug handler has been set
      return handlers().find(colman::COLMAN_DEBUG_LEVEL)->second;
    }
  }

} // namespace


// -------------------------------------------------------------------------
// colman::Assert
// -------------------------------------------------------------------------

#undef COLMAN_ASSERT_A
#undef COLMAN_ASSERT_B

/**
   Default constructor.
*/
colman::Assert::Assert(char const* expr)
    : COLMAN_ASSERT_A(*this), COLMAN_ASSERT_B(*this), needsHandling_(true)
{
  context_.set_expression(expr);
}


/**
   Copy constructor.
*/
colman::Assert::Assert(Assert const& obj)
    : COLMAN_ASSERT_A(*this), COLMAN_ASSERT_B(*this), context_(obj.context_),
      needsHandling_(true)
{
  obj.needsHandling_ = false;
}


/**
   Destructor of the Assert class.
   @exception none
*/
colman::Assert::~Assert()
#if(defined(__cplusplus) && __cplusplus >= 201103L)
    noexcept(false)
#endif
{
  if(needsHandling_) {
    handle_assert();
  }
}


/**
   Handles current assertion.
*/
void colman::Assert::handle_assert()
{
  logger()(context_);
  get_handler(context_.get_level())(context_);
}


/**
   Sets current value, checks \a val against 0 and uses "null" in this case.
 */
colman::Assert& colman::Assert::set_current_value(char* const& val, char const* mesg)
{
  context_.add_value((val) ? val : "null", mesg);
  return *this;
}

/**
   Sets current value, checks \a val against 0 and uses "null" in this case.
 */
colman::Assert& colman::Assert::set_current_value(char const* const& val, char const* mesg)
{
  context_.add_value((val) ? val : "null", mesg);
  return *this;
}


/**
   Sets current value, checks \a val against 0 and uses "null" in this case.
 */
colman::Assert& colman::Assert::set_current_value(std::string const& val, char const* mesg)
{
  context_.add_value(val, mesg);
  return *this;
}


/**
   Sets current location, usually given by the macro COLMAN_HERE.
 */
colman::Assert& colman::Assert::set_location(char const* loc)
{
  context_.set_location(loc);
  return *this;
}


/**
   Sets message.
 */
colman::Assert& colman::Assert::msg(char const* mesg)
{
  context_.set_message(mesg);
  return *this;
}


/**
   Sets message.
 */
colman::Assert& colman::Assert::msg(std::string const& mesg)
{
  context_.set_message(mesg);
  return *this;
}


/**
   Sets level to \a lev and message to \a mesg.
 */
colman::Assert& colman::Assert::level(int lev, char const* mesg)
{
  context_.set_level(lev);
  context_.set_message(mesg);
  return *this;
}


/**
   Sets level to \a lev and message to \a mesg.
 */
colman::Assert& colman::Assert::level(int lev, std::string const& mesg)
{
  context_.set_level(lev);
  context_.set_message(mesg);
  return *this;
}


/**
   Sets level to colman::COLMAN_WARN_LEVEL and message to \a mesg.
 */
colman::Assert& colman::Assert::warn(char const* mesg)
{
  return level(colman::COLMAN_WARN_LEVEL, mesg);
}


/**
   Sets level to colman::COLMAN_WARN_LEVEL and message to \a mesg.
 */
colman::Assert& colman::Assert::warn(std::string const& mesg)
{
  return level(colman::COLMAN_WARN_LEVEL, mesg);
}


/**
   Sets level to colman::COLMAN_DEBUG_LEVEL and message to \a mesg.
 */
colman::Assert& colman::Assert::debug(char const* mesg)
{
  return level(colman::COLMAN_DEBUG_LEVEL, mesg);
}


/**
   Sets level to colman::COLMAN_DEBUG_LEVEL and message to \a mesg.
 */
colman::Assert& colman::Assert::debug(std::string const& mesg)
{
  return level(colman::COLMAN_DEBUG_LEVEL, mesg);
}


/**
   Sets level to colman::COLMAN_ERROR_LEVEL and message to \a mesg.
 */
colman::Assert& colman::Assert::error(char const* mesg)
{
  return level(colman::COLMAN_ERROR_LEVEL, mesg);
}


/**
   Sets level to colman::COLMAN_ERROR_LEVEL and message to \a mesg.
 */
colman::Assert& colman::Assert::error(std::string const& mesg)
{
  return level(colman::COLMAN_ERROR_LEVEL, mesg);
}


/**
   Sets level to colman::COLMAN_FATAL and message to \a mesg.
 */
colman::Assert& colman::Assert::fatal(char const* mesg)
{
  return level(colman::COLMAN_FATAL, mesg);
}


/**
   Sets level to colman::COLMAN_FATAL and message to \a mesg.
 */
colman::Assert& colman::Assert::fatal(std::string const& mesg)
{
  return level(colman::COLMAN_FATAL, mesg);
}


/**
   Sets the default logger.
   Makes it write everything to \a filename.
*/
void colman::Assert::set_log(char const* filename)
{
  set_default_log_name(filename);
  logger() = &default_logger;
}


/**
   Sets the default logger to write assertion messages to the given stream.
   @param out stream to write assertion messages to.
*/
void colman::Assert::set_log(std::ostream& out)
{
  set_default_log_stream(out);
  logger() = &default_logger;
}


/**
   Sets a custom logger function called on every assertion.
   @param l function to call when an assertion is logged.
*/
void colman::Assert::set_log(assert_f l)
{
  logger() = l;
}

/**
   Sets the handler for a specific assertion level.
   @param lev assertion level (e.g. colman::COLMAN_ERROR_LEVEL).
   @param handler function to invoke when an assertion of that level fires.
*/
void colman::Assert::set_handler(int lev, assert_f handler)
{
  handlers()[lev] = handler;
}


// default handlers
/**
   Dumps detailed description of assertion to std::clog.
*/
void colman::default_warn_handler(AssertContext const& context)
{
  dump_context_detail(context, std::clog);
}

/**
   Asks user what to do.
*/
void colman::default_debug_handler(AssertContext const& context)
{
  static bool ignore_all = false;
  if(ignore_all) {
    return; // ignore ALL asserts
  }
  static std::set<std::string> ignored;
  if(ignored.find(context.get_location()) != ignored.end()) {
    return; // this location is ignored forever
  }
  dump_context_detail(context, std::cerr);
  std::cerr << "\nPress (I)gnore / Ignore (F)orever / Ignore (A)ll"
            << " / (D)ebug break / A(b)ort: ";
  std::cerr.flush();
  char ch = 0;
  bool bContinue = true;
  while(bContinue && std::cin.get(ch)) {
    bContinue = false;
    switch(ch) {
    case 'i':
    case 'I':
      break; // ignore
    case 'f':
    case 'F':
      // ignore forever
      ignored.insert(context.get_location());
      break;
    case 'a':
    case 'A':
      // ignore all
      ignore_all = true;
      break;
    case 'd':
    case 'D':
      // break
      break_into_debugger();
      break;
    case 'b':
    case 'B':
      abort();
      break;
    default:
      bContinue = true;
      break;
    }
  }
}

/**
   Throws an exception of type colman::Error containing a detailed
   error description.
   @exception colman::Error
*/
void colman::default_error_handler(AssertContext const& context)
{
  std::ostringstream out;
  dump_context_detail(context, out);
  throw Error(out.str());
}

/**
   Default handler for fatal errors.
   Dumps a detailed assertion description and calls abort().
*/
void colman::default_fatal_handler(AssertContext const& context)
{
  dump_context_detail(context, std::cerr);
  abort();
}

/* EOF */
