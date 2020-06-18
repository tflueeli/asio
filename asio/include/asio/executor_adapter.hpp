//
// executor_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTOR_ADAPTER_HPP
#define ASIO_EXECUTOR_ADAPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/memory.hpp"
#include "asio/execution/blocking.hpp"
#include "asio/execution/context.hpp"
#include "asio/execution/executor.hpp"
#include "asio/execution/outstanding_work.hpp"
#include "asio/execution_context.hpp"
#include "asio/is_executor.hpp"
#include "asio/query.hpp"
#include "asio/require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

struct executor_adapter_bits
{
  ASIO_STATIC_CONSTEXPR(unsigned int, blocking_never = 1);
  ASIO_STATIC_CONSTEXPR(unsigned int, relationship_continuation = 2);
  ASIO_STATIC_CONSTEXPR(unsigned int, outstanding_work_tracked = 4);
};

} // namespace detail

/// Adapts an older executor to the new standard executor form.
template <typename Executor,
    typename Allocator = std::allocator<void>,
    unsigned int Bits = 0>
class executor_adapter : detail::executor_adapter_bits
{
public:
  /// Construct an adapter around another executor.
  template <typename E>
  ASIO_CONSTEXPR explicit executor_adapter(ASIO_MOVE_ARG(E) e,
      const Allocator& a = Allocator(), unsigned int bits = 0)
    : executor_(ASIO_MOVE_CAST(E)(e)),
      allocator_(a),
      bits_(bits)
  {
    if (Bits & outstanding_work_tracked)
      executor_.on_work_started();
  }

  /// Copy constructor.
  ASIO_CONSTEXPR executor_adapter(
      const executor_adapter& other) ASIO_NOEXCEPT
    : executor_(other.executor_),
      allocator_(other.allocator_),
      bits_(other.bits_)
  {
    if (Bits & outstanding_work_tracked)
      executor_.on_work_started();
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.
  executor_adapter(executor_adapter&& other) ASIO_NOEXCEPT
    : executor_(ASIO_MOVE_CAST(Executor)(other.executor_)),
      allocator_(ASIO_MOVE_CAST(Allocator)(other.allocator_)),
      bits_(other.bits_)
  {
    if (Bits & outstanding_work_tracked)
      executor_.on_work_started();
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destructor.
  ~executor_adapter()
  {
    if (Bits & outstanding_work_tracked)
      executor_.on_work_finished();
  }

  /// Obtain an executor with the @c blocking.possibly property.
  ASIO_CONSTEXPR executor_adapter require(
      execution::blocking_t::possibly_t) const
  {
    return executor_adapter(executor_, allocator_, bits_ & ~blocking_never);
  }

  /// Obtain an executor with the @c blocking.never property.
  ASIO_CONSTEXPR executor_adapter require(
      execution::blocking_t::never_t) const
  {
    return executor_adapter(executor_, allocator_, bits_ | blocking_never);
  }

  /// Obtain an executor with the @c relationship.fork property.
  ASIO_CONSTEXPR executor_adapter require(
      execution::relationship_t::fork_t) const
  {
    return executor_adapter(executor_, allocator_,
        bits_ & ~relationship_continuation);
  }

  /// Obtain an executor with the @c relationship.continuation property.
  ASIO_CONSTEXPR executor_adapter require(
      execution::relationship_t::continuation_t) const
  {
    return executor_adapter<Executor, Allocator, Bits>(
        executor_, allocator_, bits_ | relationship_continuation);
  }

  /// Obtain an executor with the @c outstanding_work.tracked property.
  ASIO_CONSTEXPR executor_adapter<Executor, Allocator,
      Bits | outstanding_work_tracked>
  require(execution::outstanding_work_t::tracked_t) const
  {
    return executor_adapter<Executor, Allocator,
        Bits | outstanding_work_tracked>(
          executor_, allocator_, bits_);
  }

  /// Obtain an executor with the @c outstanding_work.untracked property.
  ASIO_CONSTEXPR executor_adapter<Executor, Allocator,
      Bits & ~outstanding_work_tracked>
  require(execution::outstanding_work_t::untracked_t) const
  {
    return executor_adapter<Executor, Allocator,
        Bits & ~outstanding_work_tracked>(executor_, allocator_, bits_);
  }

  /// Obtain an executor with the specified @c allocator property.
  template <typename OtherAllocator>
  ASIO_CONSTEXPR executor_adapter<Executor, OtherAllocator, Bits>
  require(execution::allocator_t<OtherAllocator> a) const
  {
    return executor_adapter<Executor, OtherAllocator, Bits>(
        executor_, a.value(), bits_);
  }

  /// Obtain an executor with the default @c allocator property.
  ASIO_CONSTEXPR executor_adapter<Executor, std::allocator<void>, Bits>
  require(execution::allocator_t<void>) const
  {
    return executor_adapter<Executor, std::allocator<void>, Bits>(
        executor_, std::allocator<void>(), bits_);
  }

  /// Query the current value of the @c mapping property.
  static ASIO_CONSTEXPR execution::mapping_t query(
      execution::mapping_t) ASIO_NOEXCEPT
  {
    return execution::mapping.thread;
  }

  /// Query the current value of the @c context property.
  execution_context& query(execution::context_t) const ASIO_NOEXCEPT
  {
    return executor_.context();
  }

  /// Query the current value of the @c blocking property.
  ASIO_CONSTEXPR execution::blocking_t query(
      execution::blocking_t) const ASIO_NOEXCEPT
  {
    return (bits_ & blocking_never)
      ? execution::blocking_t(
          execution::blocking.never)
      : execution::blocking_t(
          execution::blocking.possibly);
  }

  /// Query the current value of the @c relationship property.
  ASIO_CONSTEXPR execution::relationship_t query(
      execution::relationship_t) const ASIO_NOEXCEPT
  {
    return (bits_ & relationship_continuation)
      ? execution::relationship_t(
          execution::relationship.continuation)
      : execution::relationship_t(
          execution::relationship.fork);
  }

  /// Query the current value of the @c outstanding_work property.
  static ASIO_CONSTEXPR execution::outstanding_work_t
  query(execution::outstanding_work_t) ASIO_NOEXCEPT
  {
    return (Bits & outstanding_work_tracked)
      ? execution::outstanding_work_t(
          execution::outstanding_work.tracked)
      : execution::outstanding_work_t(
          execution::outstanding_work.untracked);
  }

  /// Query the current value of the @c allocator property.
  ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<Allocator>) const ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the current value of the @c allocator property.
  ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<void>) const ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Compare two executors for equality.
  /**
   * Two executors are equal if they refer to the same underlying executor,
   * allocator, and configured propeties.
   */
  friend bool operator==(const executor_adapter& a,
      const executor_adapter& b) ASIO_NOEXCEPT
  {
    return a.executor_ == b.executor_
      && a.allocator_ == b.allocator_
      && a.bits_ == b.bits_;
  }

  /// Compare two executors for inequality.
  /**
   * Two executors are equal if they refer to the same underlying executor,
   * allocator, and configured propeties.
   */
  friend bool operator!=(const executor_adapter& a,
      const executor_adapter& b) ASIO_NOEXCEPT
  {
    return a.executor_ != b.executor_
      || a.allocator_ != b.allocator_
      || a.bits_ != b.bits_;
  }

  /// Submit a function object for execution.
  template <typename Function>
  void execute(ASIO_MOVE_ARG(Function) f) const
  {
    if (bits_ & blocking_never)
    {
      if (bits_ & relationship_continuation)
      {
        executor_.defer(ASIO_MOVE_CAST(Function)(f), allocator_);
      }
      else
      {
        executor_.post(ASIO_MOVE_CAST(Function)(f), allocator_);
      }
    }
    else
    {
      executor_.dispatch(ASIO_MOVE_CAST(Function)(f), allocator_);
    }
  }

private:
  Executor executor_;
  Allocator allocator_;
  unsigned int bits_;
};

/// Type trait to adapt an older executor to the new standard executor form, if
/// necessary.
template <typename Executor>
struct adapted_executor_type
{
  /// The adapted type.
  typedef typename conditional<
      execution::is_executor<typename decay<Executor>::type>::value,
      typename decay<Executor>::type,
      executor_adapter<typename decay<Executor>::type>
    >::type type;
};

namespace execution {

#if !defined(ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

template <typename Executor, typename Allocator, unsigned int Bits>
struct is_executor<
    asio::executor_adapter<Executor, Allocator, Bits>
  > : true_type
{
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

} // namespace execution
namespace traits {

#if !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Executor, unsigned int Bits,
    typename Allocator, typename Function>
struct execute_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    Function
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::blocking_t::possibly_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::executor_adapter<Executor,
      Allocator, Bits> result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::blocking_t::never_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor,
      Allocator, Bits> result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::relationship_t::fork_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor,
      Allocator, Bits> result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::relationship_t::continuation_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor,
      Allocator, Bits> result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::outstanding_work_t::tracked_t
  > : asio::detail::executor_adapter_bits
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor, Allocator,
      Bits | outstanding_work_tracked> result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::outstanding_work_t::untracked_t
  > : asio::detail::executor_adapter_bits
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor, Allocator,
      Bits & ~outstanding_work_tracked> result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::allocator_t<void>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor,
      std::allocator<void>, Bits> result_type;
};

template <typename Executor, unsigned int Bits,
    typename Allocator, typename OtherAllocator>
struct require_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::allocator_t<OtherAllocator>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef asio::executor_adapter<Executor,
      OtherAllocator, Bits> result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Executor, unsigned int Bits,
    typename Allocator, typename Property>
struct query_static_constexpr_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::outstanding_work_t
      >::value
    >::type
  > : asio::detail::executor_adapter_bits
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::outstanding_work_t result_type;

  static ASIO_CONSTEXPR result_type value() ASIO_NOEXCEPT
  {
    return (Bits & outstanding_work_tracked)
      ? execution::outstanding_work_t(execution::outstanding_work.tracked)
      : execution::outstanding_work_t(execution::outstanding_work.untracked);
  }
};

template <typename Executor, unsigned int Bits,
    typename Allocator, typename Property>
struct query_static_constexpr_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::mapping_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::mapping_t::thread_t result_type;

  static ASIO_CONSTEXPR result_type value() ASIO_NOEXCEPT
  {
    return result_type();
  }
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename Executor, unsigned int Bits,
    typename Allocator, typename Property>
struct query_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::blocking_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::blocking_t result_type;
};

template <typename Executor, unsigned int Bits,
    typename Allocator, typename Property>
struct query_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    Property,
    typename asio::enable_if<
      asio::is_convertible<
        Property,
        asio::execution::relationship_t
      >::value
    >::type
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution::relationship_t result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct query_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::context_t
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef asio::execution_context& result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct query_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::allocator_t<void>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

template <typename Executor, typename Allocator, unsigned int Bits>
struct query_member<
    asio::executor_adapter<Executor, Allocator, Bits>,
    asio::execution::allocator_t<Allocator>
  >
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTOR_ADAPTER_HPP
