// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// COLMAN_ON_EXIT macro following talk of Andrei Alexandrescu
/**
   @file
   following the talk "Declarative Control Flow" by Andrei Alexandrescu
   prepared for the C++ and Beyond seminar
   Stuttgart, Germany, Sep 29-Oct 1, 2014
   @author Christoph Pesch
*/

#ifndef COLMAN_ON_EXIT_HPP
#  define COLMAN_ON_EXIT_HPP

#  ifndef COLMAN_ANONYMOUS_VARIABLE_HPP
#    include "colman_anonymous_variable.hpp"
#  endif
#  ifndef SYSTEM_UTILITY
#    include <utility>
#    define SYSTEM_UTILITY
#  endif


/// Executes a lambda when the enclosing scope exits (always).
/** Usage: COLMAN_ON_EXIT{ cleanup_code; }; */
#  define COLMAN_ON_EXIT \
    auto COLMAN_ANONYMOUS_VARIABLE(colman_scope_exit_state_) = ::colman::detail::ScopeGuardOnExit() + [&]()

/// Executes a lambda only when the enclosing scope exits via an exception.
/** Usage: COLMAN_ON_FAIL{ rollback_code; }; */
#  define COLMAN_ON_FAIL \
    auto COLMAN_ANONYMOUS_VARIABLE(colman_scope_fail_state_) = ::colman::detail::ScopeGuardOnFail() + [&]()

/// Executes a lambda only when the enclosing scope exits normally (no exception).
/** Usage: COLMAN_ON_SUCCESS{ commit_code; }; */
#  define COLMAN_ON_SUCCESS \
    auto COLMAN_ANONYMOUS_VARIABLE(colman_scope_success_state_) = ::colman::detail::ScopeGuardOnSuccess() + [&]()

/// Restores \a _var_ to its current value when the enclosing scope exits.
/** Usage: COLMAN_RESTORE_ON_EXIT(my_variable); */
#  define COLMAN_RESTORE_ON_EXIT(_var_)              \
    ::colman::detail::RestoreOnExit<decltype(_var_)> \
    COLMAN_ANONYMOUS_VARIABLE(colman_restore_exit_)(_var_)


// unfortunately we need some compiler specific hacks, here:
#  if(defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ >= 4 && __GNUC_MINOR__ >= 9))) || (defined(__clang__) && (__clang_major__ >= 3))
// instead of #include <unwind-cxx.h>, which is not present, usually, we have:
namespace __cxxabiv1
{
  struct __cxa_eh_globals;
  extern "C" __cxa_eh_globals* __cxa_get_globals() noexcept;
} // namespace __cxxabiv1
#  elif(defined(_MSC_VER) && _MSC_VER >= 1610)
struct _tiddata;
extern "C" _tiddata* _getptd();
#  else
#    error "this compiler version is not supported"
#  endif


namespace colman
{
  /// Implementation details of the scope-guard macros.  Do not use directly.
  namespace detail
  {
    /// Sentinel tag type used by operator+ to construct a ScopeGuard.
    enum struct ScopeGuardOnExit {};

    /// RAII scope guard that unconditionally calls a function on destruction.
    template <typename Function>
    class ScopeGuard
    {
    public:
      /// Constructs the guard, storing a copy of \a func.
      explicit ScopeGuard(Function const& func)
          : function_(func)
      {
      }
      /// Constructs the guard, moving \a func into storage.
      explicit ScopeGuard(Function&& func)
          : function_(std::move(func))
      {
      }
      ScopeGuard(ScopeGuard const&) = delete;
      ScopeGuard& operator=(ScopeGuard const&) = delete;

      /// Calls the stored function.
      ~ScopeGuard()
      {
        function_();
      }

    private:
      Function function_; ///< the stored callable
    };

    /// Combines a ScopeGuardOnExit tag with a callable to produce a ScopeGuard.
    template <typename Func>
    ScopeGuard<Func> operator+(ScopeGuardOnExit, Func&& func)
    {
      return ScopeGuard<Func>(std::forward<Func>(func));
    }


    /// RAII guard that restores a variable to its value at construction time.
    template <typename T>
    class RestoreOnExit
    {
    public:
      /// Saves the current value of \a val.
      explicit RestoreOnExit(T& val)
          : orig_value_(val),
            orig_reference_(val)
      {
      }
      /// Restores the saved value.
      ~RestoreOnExit()
      {
        orig_reference_ = orig_value_;
      }

    private:
      T orig_value_;     ///< saved original value
      T& orig_reference_; ///< reference to the variable being guarded
    };


    /// Counts uncaught exceptions using ABI internals.
    /**
       Captures the current uncaught-exception count on construction so that
       ScopeGuardForNewException can detect whether a new exception was thrown.
    */
    class UncaughtExceptionCounter
    {
    public:
      /// Captures the current uncaught-exception count.
      UncaughtExceptionCounter()
          : exception_count_(get_uncaught_exception_count())
      {
      }

      /// Returns true if a new uncaught exception appeared since construction.
      inline bool is_new_uncaught_exception() const noexcept
      {
        return get_uncaught_exception_count() > exception_count_;
      }

    private:
      /// Returns the current total number of uncaught exceptions.
      inline int get_uncaught_exception_count() const noexcept
      {
#  if(defined(__GNUC__) || defined(__clang__))
        return *(reinterpret_cast<int*>(static_cast<char*>(static_cast<void*>(__cxxabiv1::__cxa_get_globals())) + sizeof(void*)));
#  elif(defined(_MSC_VER))
        return *(reinterpret_cast<int*>(static_cast<char*>(static_cast<void*>(_getptd())) + sizeof(void*) * 28 + 0x4 * 8));
#  else
#    error "this compiler version is not supported"
#  endif
      }

      int exception_count_; ///< uncaught-exception count at construction
    };


    /// RAII guard that calls a function on scope exit, conditionally on exceptions.
    /**
       When \a executeOnException is true the function is called only if a new
       exception was thrown (COLMAN_ON_FAIL).  When false it is called only on
       normal exit (COLMAN_ON_SUCCESS).
    */
    template <typename FunctionType, bool executeOnException>
    class ScopeGuardForNewException
    {
    public:
      /// Constructs the guard, storing a copy of \a fn.
      explicit ScopeGuardForNewException(FunctionType const& fn)
          : function_(fn)
      {
      }
      /// Constructs the guard, moving \a fn into storage.
      explicit ScopeGuardForNewException(FunctionType&& fn)
          : function_(std::move(fn))
      {
      }
      ScopeGuardForNewException(ScopeGuardForNewException const&) = delete;
      ScopeGuardForNewException& operator=(
          ScopeGuardForNewException const&) = delete;

      /// Conditionally calls the stored function based on exception state.
      ~ScopeGuardForNewException() noexcept(executeOnException)
      {
        if(executeOnException == ec_.is_new_uncaught_exception()) {
          function_();
        }
      }

    private:
      FunctionType function_;       ///< the stored callable
      UncaughtExceptionCounter ec_; ///< exception counter captured at construction
    };


    /// Sentinel tag type used by operator+ to construct a ScopeGuardForNewException<true>.
    enum struct ScopeGuardOnFail {};

    /// Combines a ScopeGuardOnFail tag with a callable.
    template <typename FunctionType>
    auto operator+(ScopeGuardOnFail, FunctionType&& fn)
    {
      using decay_t = typename std::decay<FunctionType>::type;
      return ScopeGuardForNewException<decay_t, true>(std::forward<FunctionType>(fn));
    }


    /// Sentinel tag type used by operator+ to construct a ScopeGuardForNewException<false>.
    enum struct ScopeGuardOnSuccess {};

    /// Combines a ScopeGuardOnSuccess tag with a callable.
    template <typename FunctionType>
    auto operator+(ScopeGuardOnSuccess, FunctionType&& fn)
    {
      using decay_t = typename std::decay<FunctionType>::type;
      return ScopeGuardForNewException<decay_t, false>(std::forward<FunctionType>(fn));
    }

  } // namespace detail

} // namespace colman

#endif /* #ifndef COLMAN_ON_EXIT_HPP */

/* EOF */
