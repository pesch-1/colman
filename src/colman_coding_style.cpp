// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class CodingStyle.

// include of corresponding header first:
#include "colman_coding_style.hpp"

// further includes shall be guarded:
//#ifndef COLMAN_ERROR_HPP
//#  include "colman_error.hpp"
//#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef SYSTEM_VECTOR
#  include <vector>
#  define SYSTEM_VECTOR
#endif


// avoid global variables!
// use anonymous namespace for local functions, local constants, local types, ...!
// do not use "using namespace"!

namespace
{
  // file local constants with upper case first char and camel case in anonymous namespace.
  // All primitives types should be initialized on declaration.

  /// A file local constant.
  int constexpr TheAnswer = 42;

  /// using instead of typedef, same style for types as for class names.
  using IntVect = std::vector<int>;

  /// camel case and upper-case first character for enum struct.
  enum struct MyEnum {
    Unknown,       ///< if value is unknown
    FirstChoice,   ///< first choice
    SecondChoice,  ///< second choice
    ThirstChoice   ///< third choice
  };

} // namespace


/**
  long documentation in cpp file.
*/
void colman::CodingStyle::my_member_function() const
{
  std::vector<int> h;
  h.reserve(10);
}


/**
  This is the long documentation of free_function().
  @param x the input parameter
*/
void colman::free_function(int x)
{
  // local variables are lower case, snake case.
  // use east const declaration.
  // Pointer * or reference & character have no space after type declaration and a space before variable name.
  // No spaces in assignment before or after = for variable declaration.

  int my_local_int = 42;
  int const& my_local_ref = my_local_int;

  // space before and after = in all other assignments but variable declarations.
  my_local_int = 1;

  // example of short if (one line if statement):
  // no extra spaces before or after braces,
  // no line break before brace in case of one-line if statement
  if(x < 0) {
  }
  // example of long if (multi-line if statement):
  // no extra spaces before or after braces,
  // line break before brace in case of multi-line if statement
  if(x > 0 && x + 17 > 0 && x - 42 > 42)
  {
    std::cout << x << std::endl;
  }

  // example of if else:
  if(x > 0) {
    std::cout << "positive" << std::endl;
  }
  else {
    std::cout << "negative" << std::endl;
  }

  // indentation and braces for for, while, do in the same way as if.
}

/* EOF */
