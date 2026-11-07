// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of macro COLMAN_HERE.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_HERE_HPP
#  define COLMAN_HERE_HPP

#  ifndef SYSTEM_STRING
#    include <string>
#    define SYSTEM_STRING
#  endif

/**
   @see #COLMAN_LINE
*/
#  define COLMAN_LINE_1(x) #x
/**
   @see #COLMAN_LINE
*/
#  define COLMAN_LINE_2(x) COLMAN_LINE_1(x)
/// Macro which gives line number as a string (instead of int as in __LINE__)
#  define COLMAN_LINE COLMAN_LINE_2(__LINE__)

/// Macro that expands to a string describing the current function, file and line (with newline).
/**
   The format is suitable for use in error messages and assertion output.
   Example output: `my_function()\nat myfile.cpp:42:`
*/
#  ifdef _MSC_VER
// MS Visual C++ version (output can be used in developer studio)
#    define COLMAN_HERE __FUNCTION__ "()\nat " __FILE__ "(" COLMAN_LINE "):"
/// Macro that expands to a string describing the current function, file and line (without newline).
#    define COLMAN_HERE_NO_NEWLINE __FUNCTION__ "() at " __FILE__ "(" COLMAN_LINE ")"
#  else
#    ifdef __GNUC__
// GCC (output can be used in gdb)
#      define COLMAN_HERE (std::string(__func__) + "()\nat " + __FILE__ + ":" + COLMAN_LINE).c_str()
/// Macro that expands to a string describing the current function, file and line (without newline).
#      define COLMAN_HERE_NO_NEWLINE \
        (std::string(__func__) + "() at " + __FILE__ + ":" + COLMAN_LINE).c_str()
#    else
// Other targets (output can be used in gdb)
#      define COLMAN_HERE __FILE__ ":" COLMAN_LINE
/// Macro that expands to a string describing the current file and line (without newline).
#      define COLMAN_HERE_NO_NEWLINE __FILE__ ":" COLMAN_LINE
#    endif
#  endif

#endif /* #ifndef COLMAN_HERE_HPP */

/* EOF */
