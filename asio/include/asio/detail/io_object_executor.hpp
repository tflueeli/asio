//
// io_object_executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IO_OBJECT_EXECUTOR_HPP
#define ASIO_DETAIL_IO_OBJECT_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/handler_invoke_helpers.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution/blocking.hpp"
#include "asio/execution/context.hpp"
#include "asio/execution/execute.hpp"
#include "asio/execution/executor.hpp"
#include "asio/execution/outstanding_work.hpp"
#include "asio/execution_context.hpp"
#include "asio/query.hpp"
#include "asio/require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// Wrap the (potentially polymorphic) executor so that we can bypass it when
// dispatching on a target executor that has a native I/O implementation.
template <typename Executor, bool HasNativeImpl, typename = void>
class io_object_executor
{
public:
  template <typename Ex>
  io_object_executor(ASIO_MOVE_ARG(Ex) ex,
      bool native_implementation) ASIO_NOEXCEPT
    : executor_(ASIO_MOVE_CAST(Ex)(ex)),
      has_native_impl_(native_implementation)
  {
  }

  io_object_executor(const io_object_executor& other) ASIO_NOEXCEPT
    : executor_(other.executor_),
      has_native_impl_(other.has_native_impl_)
  {
  }

  template <typename Executor1, bool OtherHasNativeImpl>
  io_object_executor(
      const io_object_executor<Executor1,
        OtherHasNativeImpl>& other) ASIO_NOEXCEPT
    : executor_(other.inner_executor()),
      has_native_impl_(other.has_native_implementation())
  {
  }

#if defined(ASIO_HAS_MOVE)
  io_object_executor(io_object_executor&& other) ASIO_NOEXCEPT
    : executor_(ASIO_MOVE_CAST(Executor)(other.executor_)),
      has_native_impl_(other.has_native_impl_)
  {
  }
#endif // defined(ASIO_HAS_MOVE)

  const Executor& inner_executor() const ASIO_NOEXCEPT
  {
    return executor_;
  }

  bool has_native_implementation() const ASIO_NOEXCEPT
  {
    return has_native_impl_;
  }

  execution_context& context() const ASIO_NOEXCEPT
  {
    return asio::query(executor_, execution::context);
  }

  template <typename T>
  io_object_executor<
      typename decay<
        typename require_result_type<const Executor&, const T&>::type
      >::type, HasNativeImpl>
  require(const T& t,
      typename enable_if<
        can_require<const Executor&, const T&>::value
        &&
        (
          // The outstanding_work property is specifically not forwarded when
          // using a native implementation, as work is already counted by the
          // execution context.
          !is_same<T, execution::outstanding_work_t::tracked_t>::value
            || !HasNativeImpl
        )
      >::type* = 0) const
    ASIO_NOEXCEPT_IF((
      is_nothrow_require<const Executor&, const T&>::value))
  {
    return io_object_executor<
        typename decay<
          typename require_result_type<const Executor&, const T&>::type
        >::type, HasNativeImpl>(
          asio::require(executor_, t), has_native_impl_);
  }

  template <typename T>
  typename query_result_type<const Executor&, const T&>::type
  query(const T& t,
      typename enable_if<
        can_query<const Executor&, const T&>::value
      >::type* = 0) const
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, const T&>::value))
  {
    return asio::query(executor_, t);
  }

  template <typename F>
  void execute(ASIO_MOVE_ARG(F) f) const
  {
    if (HasNativeImpl || has_native_impl_)
    {
      if (asio::query(executor_, execution::blocking)
          == execution::blocking.possibly)
      {
        // When using a native implementation, I/O completion handlers are
        // already dispatched according to the execution context's executor's
        // rules. We can call the function directly.
#if defined(ASIO_HAS_MOVE)
        if (is_same<F, typename decay<F>::type>::value)
        {
          asio_handler_invoke_helpers::invoke(f, f);
          return;
        }
#endif // defined(ASIO_HAS_MOVE)
        typename decay<F>::type function(ASIO_MOVE_CAST(F)(f));
        asio_handler_invoke_helpers::invoke(function, function);
        return;
      }
    }

    execution::execute(executor_, ASIO_MOVE_CAST(F)(f));
  }

  friend bool operator==(const io_object_executor& a,
      const io_object_executor& b) ASIO_NOEXCEPT
  {
    return a.executor_ == b.executor_
      && a.has_native_impl_ == b.has_native_impl_;
  }

  friend bool operator!=(const io_object_executor& a,
      const io_object_executor& b) ASIO_NOEXCEPT
  {
    return a.executor_ != b.executor_
      || a.has_native_impl_ != b.has_native_impl_;
  }

private:
  Executor executor_;
  const bool has_native_impl_;
};

template <typename Executor, bool HasNativeImpl>
class io_object_executor<Executor, HasNativeImpl,
    typename enable_if<
      !execution::is_executor<Executor>::value
    >::type>
{
public:
  io_object_executor(const Executor& ex,
      bool native_implementation) ASIO_NOEXCEPT
    : executor_(ex),
      has_native_impl_(native_implementation)
  {
  }

  io_object_executor(const io_object_executor& other) ASIO_NOEXCEPT
    : executor_(other.executor_),
      has_native_impl_(other.has_native_impl_)
  {
  }

  template <typename Executor1, bool OtherHasNativeImpl>
  io_object_executor(
      const io_object_executor<Executor1,
        OtherHasNativeImpl>& other) ASIO_NOEXCEPT
    : executor_(other.inner_executor()),
      has_native_impl_(other.has_native_implementation())
  {
  }

#if defined(ASIO_HAS_MOVE)
  io_object_executor(io_object_executor&& other) ASIO_NOEXCEPT
    : executor_(ASIO_MOVE_CAST(Executor)(other.executor_)),
      has_native_impl_(other.has_native_impl_)
  {
  }
#endif // defined(ASIO_HAS_MOVE)

  const Executor& inner_executor() const ASIO_NOEXCEPT
  {
    return executor_;
  }

  bool has_native_implementation() const ASIO_NOEXCEPT
  {
    return has_native_impl_;
  }

  execution_context& context() const ASIO_NOEXCEPT
  {
    return executor_.context();
  }

  void on_work_started() const ASIO_NOEXCEPT
  {
    if (HasNativeImpl || has_native_impl_)
    {
      // When using a native implementation, work is already counted by the
      // execution context.
    }
    else
    {
      executor_.on_work_started();
    }
  }

  void on_work_finished() const ASIO_NOEXCEPT
  {
    if (HasNativeImpl || has_native_impl_)
    {
      // When using a native implementation, work is already counted by the
      // execution context.
    }
    else
    {
      executor_.on_work_finished();
    }
  }

  template <typename F, typename A>
  void dispatch(ASIO_MOVE_ARG(F) f, const A& a) const
  {
    if (HasNativeImpl || has_native_impl_)
    {
      // When using a native implementation, I/O completion handlers are
      // already dispatched according to the execution context's executor's
      // rules. We can call the function directly.
#if defined(ASIO_HAS_MOVE)
      if (is_same<F, typename decay<F>::type>::value)
      {
        asio_handler_invoke_helpers::invoke(f, f);
        return;
      }
#endif // defined(ASIO_HAS_MOVE)
      typename decay<F>::type function(ASIO_MOVE_CAST(F)(f));
      asio_handler_invoke_helpers::invoke(function, function);
    }
    else
    {
      executor_.dispatch(ASIO_MOVE_CAST(F)(f), a);
    }
  }

  template <typename F, typename A>
  void post(ASIO_MOVE_ARG(F) f, const A& a) const
  {
    executor_.post(ASIO_MOVE_CAST(F)(f), a);
  }

  template <typename F, typename A>
  void defer(ASIO_MOVE_ARG(F) f, const A& a) const
  {
    executor_.defer(ASIO_MOVE_CAST(F)(f), a);
  }

  friend bool operator==(const io_object_executor& a,
      const io_object_executor& b) ASIO_NOEXCEPT
  {
    return a.executor_ == b.executor_
      && a.has_native_impl_ == b.has_native_impl_;
  }

  friend bool operator!=(const io_object_executor& a,
      const io_object_executor& b) ASIO_NOEXCEPT
  {
    return a.executor_ != b.executor_
      || a.has_native_impl_ != b.has_native_impl_;
  }

private:
  Executor executor_;
  const bool has_native_impl_;
};

} // namespace detail
namespace execution {

template <typename Executor, bool HasNativeImpl>
struct is_executor<
  asio::detail::io_object_executor<Executor, HasNativeImpl> > :
    is_executor<Executor>
{
};

} // namespace execution
namespace traits {

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Executor, bool HasNativeImpl, typename Function>
struct execute_member<
    asio::detail::io_object_executor<Executor, HasNativeImpl>,
    Function,
    typename enable_if<
      execution::is_executor<Executor>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <typename Executor, bool HasNativeImpl>
struct equality_comparable<
    asio::detail::io_object_executor<Executor, HasNativeImpl>,
    typename enable_if<
      execution::is_executor<Executor>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef void result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename Executor, typename T>
struct require_member<
    asio::detail::io_object_executor<Executor, false>, T,
    typename enable_if<
      execution::is_executor<Executor>::value
        && can_require<const Executor&, const T&>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
      (is_nothrow_require<const Executor&, const T&>::value));
  typedef asio::detail::io_object_executor<
      typename decay<
        typename require_result_type<const Executor&, const T&>::type
      >::type, false> result_type;
};

template <typename Executor, typename T>
struct require_member<
    asio::detail::io_object_executor<Executor, true>, T,
    typename enable_if<
      execution::is_executor<Executor>::value
        && can_require<const Executor&, const T&>::value
        && !is_convertible<T, execution::outstanding_work_t::tracked_t>::value
    >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
      (is_nothrow_require<const Executor&, const T&>::value));
  typedef asio::detail::io_object_executor<
      typename decay<
        typename require_result_type<const Executor&, const T&>::type
      >::type, true> result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename Executor, bool HasNativeImpl, typename T>
struct query_member<
    asio::detail::io_object_executor<Executor, HasNativeImpl>, T,
    typename enable_if<
      execution::is_executor<Executor>::value
        && can_query<const Executor&, const T&>::value
    >::type> : query_member<const Executor&, const T&>
{
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IO_OBJECT_EXECUTOR_HPP
