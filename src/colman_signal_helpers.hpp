// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// RAII helpers for temporarily blocking Qt signals.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SIGNAL_HELPERS_HPP
#  define COLMAN_SIGNAL_HELPERS_HPP

#  ifndef SYSTEM_QOBJECT
#    include <QObject>
#    define SYSTEM_QOBJECT
#  endif

namespace colman
{
  /// RAII guard that blocks signals on an arbitrary object type for its lifetime.
  /**
     Blocks signals on construction and restores the previous state on
     destruction.  Works with any type that exposes a blockSignals(bool)
     member (typically a QObject subclass or a QSignalBlocker-compatible
     wrapper).
  */
  template <typename T>
  class AutoBlockSignals2
  {
  public:
    /// Blocks signals on \a obj, saving the previous state.
    explicit inline AutoBlockSignals2(T* obj)
        : obj_(obj)
    {
      if((obj_)) {
        old_ = obj_->blockSignals(true);
      }
    }

    /// Restores the previous signal-blocking state.
    inline ~AutoBlockSignals2()
    {
      if((obj_)) {
        obj_->blockSignals(old_);
        obj_ = nullptr;
      }
    }

  private:
    T* obj_;         ///< the guarded object
    bool old_ = false; ///< previous signal-blocking state
  };

  /// Convenience alias for the common QObject case.
  using AutoBlockSignals = AutoBlockSignals2<QObject>;

} // namespace colman

#endif /* #ifndef COLMAN_SIGNAL_HELPERS_HPP */

/* EOF */
