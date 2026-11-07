// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Contains coding style examples.
/**
   @file
   Documentation language is english.
   Documentation is in doxygen style.
   Brief documentation is prefered as one-liner starting with ///.
   Indentation level is 2. No tabs shall be used for indentation.
   Default line width is 80 characters.
   Directory name is prefix of filename and used as namespace name.
   Use C++-20 as default C++ standard.
   @author Christoph Pesch
*/

// each header file should have header guards
#ifndef CODING_STYLE_HPP
#  define CODING_STYLE_HPP

// system includes should be guarded like this:
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_MAP
#    include <map>
#    define SYSTEM_MAP
#  endif

// namespace  is equal to directory name and is in lower case.
namespace colman
{
  // standard indentation is 2 spaces (do not use tab)

  /// Brief documentation as one liner.
  /**
     Long documentation as comment block. End comment should have same indentation
     as opening comment.
     Class names are camel case with first character uppercase.
  */
  class CodingStyle
  {
  public:
    CodingStyle() = default;
    ~CodingStyle() = default;

    /// brief documentation in header file
    void my_member_function() const;

    /// brief and long documentation of pure virtual functions both in header file
    /**
      Demonstrates coding style.  
    */
    virtual void pure_virtual_member_function() = 0;

    /// brief and long documentation of inline functions both in header file.
    /**
       member functions are snake case style with all characters lower case
       @param a key
       @param b value
       @return a map containing {a,b}
    */
    inline std::map<int, int> create_a_map(int a, int b) const { return {{a, b}}; }

  private:
    // members are written in snake case, lower case with a terminating underscore.
    // all primitive types are initialized on declaration.

    std::vector<int> my_vector_;   ///< brief documentation after member
    int my_int_ = 0;                 ///< brief documentation for int member.
    void* my_bad_pointer_ = nullptr; ///< brief documentation for pointer member.
  };

  /// this is a free function example.
  void free_function(int x);

} // namespace colman

#endif /* #ifndef CODING_STYLE_HPP */

/* EOF */
