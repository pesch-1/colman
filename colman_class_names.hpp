// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Class name computation for school-year transitions.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CLASS_NAMES_HPP
#  define COLMAN_CLASS_NAMES_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QREGULAREXPRESSION
#    include <QRegularExpression>
#    define SYSTEM_QREGULAREXPRESSION
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif
#  ifndef COLMAN_PERSON_HPP
#    include "colman_person.hpp"
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_QCOLLATOR
#    include <QCollator>
#    define SYSTEM_QCOLLATOR
#  endif

#  include <algorithm>

namespace colman
{

/// Returns the suggested class name for the next school year.
/**
   - One number in the name: increment by 1; if the result exceeds @a max,
     return an empty string (the class is deleted).
   - Two numbers: increment both; if either exceeds @a max, reset it to
     @a min and swap the positions of both numbers in the string.
   - No numbers: return the name unchanged.

   Replacement is performed right-to-left to preserve string indices.

   @param name  current class name
   @param min   minimum grade level
   @param max   maximum grade level
   @return suggested new name, or "" if the class should be dissolved
*/
  inline QString compute_new_name(QString const& name, int min, int max)
  {
    QRegularExpression const rx("(\\d+)");
    auto it = rx.globalMatch(name);
    QList<QRegularExpressionMatch> matches;
    while(it.hasNext()) {
      matches.append(it.next());
    }
    if(matches.size() == 1) {
      int const n = matches[0].captured().toInt() + 1;
      if(n > max) {
        return QString();
      }
      QString result = name;
      result.replace(matches[0].capturedStart(),
                     matches[0].capturedLength(),
                     QString::number(n));
      return result;
    }
    if(matches.size() == 2) {
      int a = matches[0].captured().toInt() + 1;
      int b = matches[1].captured().toInt() + 1;
      bool const swap_needed = (a > max || b > max);
      if(a > max) {
        a = min;
      }
      if(b > max) {
        b = min;
      }
      if(swap_needed) {
        std::swap(a, b);
      }
      QString result = name;
      result.replace(matches[1].capturedStart(),
                     matches[1].capturedLength(),
                     QString::number(b));
      result.replace(matches[0].capturedStart(),
                     matches[0].capturedLength(),
                     QString::number(a));
      return result;
    }
    return name;
  }


/// Merges the defined class list with the classes actually used by persons.
/**
   Persons may carry class names that are missing from the class list,
   e.g. typed in freely, imported, or left over after a class was removed.
   Filters must offer those as well.  The result contains every non-empty
   name of both lists exactly once, in natural order ("5a" before "10a").

   @param defined class names from the class list
   @param used    school classes found in the person data
   @return sorted, duplicate-free, non-empty class names
*/
  inline QStringList merge_class_names(QStringList const& defined,
                                       QStringList const& used)
  {
    QStringList result;
    for(auto const* list : {&defined, &used}) {
      for(auto const& name : *list) {
        if(!name.isEmpty() && !result.contains(name)) {
          result << name;
        }
      }
    }
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(result.begin(), result.end(),
              [&collator](QString const& a, QString const& b) {
                return collator.compare(a, b) < 0;
              });
    return result;
  }

/// Merges the defined class list with the school classes of @p persons.
/**
   @param defined class names from the class list
   @param persons persons whose school classes are added
   @return sorted, duplicate-free, non-empty class names
*/
  inline QStringList merge_class_names(QStringList const& defined,
                                       std::vector<Person> const& persons)
  {
    QStringList used;
    for(auto const& person : persons) {
      used << person.get_school_class();
    }
    return merge_class_names(defined, used);
  }

} // namespace colman

#endif /* #ifndef COLMAN_CLASS_NAMES_HPP */

/* EOF */
