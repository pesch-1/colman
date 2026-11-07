// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class ClearSelectionFilter.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP
#  define COLMAN_CLEAR_SELECTION_FILTER_HPP

#  ifndef SYSTEM_QOBJECT
#    include <QObject>
#    define SYSTEM_QOBJECT
#  endif
#  ifndef SYSTEM_QTABLEVIEW
#    include <QTableView>
#    define SYSTEM_QTABLEVIEW
#  endif
#  ifndef SYSTEM_QMOUSEEVENT
#    include <QMouseEvent>
#    define SYSTEM_QMOUSEEVENT
#  endif

namespace colman
{
  /// Event filter that deselects table rows on click in an inactive area.
  /**
     Install an instance of this class on the viewport of a QTableView to
     automatically clear the selection when the user clicks outside any row.
  */
  class ClearSelectionFilter : public QObject
  {
    Q_OBJECT

  public:
    /// Constructs the filter for \a view.
    /**
       @param view the table view whose selection is managed by this filter.
    */
    explicit ClearSelectionFilter(QTableView* view);

  protected:
    /// Clears the table selection when a mouse press hits an empty area.
    /**
       @param obj   the object that received the event.
       @param event the event to filter.
       @return true if the event was consumed, false otherwise.
    */
    bool eventFilter(QObject* obj, QEvent* event) override;

  private:
    QTableView* view_; ///< the monitored table view
  };

} // namespace colman

#endif /* #ifndef COLMAN_CLEAR_SELECTION_FILTER_HPP */

/* EOF */
