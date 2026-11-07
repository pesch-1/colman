// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class SortModel.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SORT_MODEL_HPP
#  define COLMAN_SORT_MODEL_HPP

#  ifndef SYSTEM_QSORTFILTERPROXYMODEL
#    include <QSortFilterProxyModel>
#    define SYSTEM_QSORTFILTERPROXYMODEL
#  endif
#  ifndef SYSTEM_QMODELINDEX
#    include <QModelIndex>
#    define SYSTEM_QMODELINDEX
#  endif

namespace colman
{
  /// realizes logical or of comparison with a number of integers (template args)
  /**
     This is the termination of the recursively defined variadic function
     template int_cmp_or().
  */
  template <int X>
  bool int_cmp_or(int x)
  {
    return (X == x);
  }


  /// realizes logical or of comparison with a number of integers (template args)
  /**
     Example: int_cmp_or<1,2,4>(x) returns true, iff x == 1 or x == 2 or x == 4.
     This is a recursive variadic function template.
  */
  template <int X, int Y, int... Z>
  bool int_cmp_or(int x)
  {
    return (X == x || int_cmp_or<Y, Z...>(x));
  }


  /**
     A model proxy enabling sorting and centering all columns listed
     as template arguments.
  */
  template <int X, int... Y>
  class SortModel : public QSortFilterProxyModel
  {
  public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    /**
       Returns the data for the given @p index and @p role.
       Overrides the base implementation to return Qt::AlignCenter for
       Qt::TextAlignmentRole on any of the centered columns (X, Y...).
       @param index model index to query.
       @param role  data role requested by the view.
       @return alignment variant for centered columns; base data otherwise.
    */
    QVariant data(QModelIndex const& index, int role) const override
    {
      int const col = index.column();
      if(int_cmp_or<X, Y...>(col) && role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
      }
      else {
        return QSortFilterProxyModel::data(index, role);
      }
    }
  };

} // namespace colman

#endif /* #ifndef COLMAN_SORT_MODEL_HPP */

/* EOF */
