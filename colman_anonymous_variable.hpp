// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// anonymous variable macro
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ANONYMOUS_VARIABLE_HPP
#  define COLMAN_ANONYMOUS_VARIABLE_HPP

/// Implementation helper: concatenates two tokens at the preprocessor level.
#  define COLMAN_CONCATENATE_IMPL(s1, s2) s1##s2
/// Concatenates two tokens, expanding macros before concatenation.
#  define COLMAN_CONCATENATE(s1, s2) COLMAN_CONCATENATE_IMPL(s1, s2)

#  ifdef __COUNTER__
/// Appends a unique counter suffix to \a str, producing a unique identifier.
#    define COLMAN_ANONYMOUS_VARIABLE(str) COLMAN_CONCATENATE(str, __COUNTER__)
#  else
/// Appends the current line number to \a str as a unique identifier fallback.
#    define COLMAN_ANONYMOUS_VARIABLE(str) COLMAN_CONCATENATE(str, __LINE__)
#  endif
/// Appends the current line number to \a str, always using __LINE__.
#  define COLMAN_ANONYMOUS_LINE_VARIABLE(str) COLMAN_CONCATENATE(str, __LINE__)

#endif /* #ifndef COLMAN_ANONYMOUS_VARIABLE_HPP */

/* EOF */
