// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Cutter-Sanborn call number generation for library items.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CUTTER_HPP
#  define COLMAN_CUTTER_HPP

#  ifndef SYSTEM_STRING
#    include <string>
#    define SYSTEM_STRING
#  endif
#  ifndef SYSTEM_SET
#    include <set>
#    define SYSTEM_SET
#  endif

namespace colman
{
  /// Computes a call number signature for a book in a library.
  std::string generate_signature(std::string const& author_lastname,
                                 std::string const& title,
                                 std::string const& year,
                                 std::string const& ddc,
                                 std::set<std::string> const& existing_signatures);
} // namespace colman

#endif /* #ifndef COLMAN_CUTTER_HPP */

/* EOF */
