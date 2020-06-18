//
// execution/bulk_guarantee.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_BULK_GUARANTEE_HPP
#define ASIO_EXECUTION_BULK_GUARANTEE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution/executor.hpp"
#include "asio/is_applicable_property.hpp"
#include "asio/query.hpp"
#include "asio/traits/query_free.hpp"
#include "asio/traits/query_member.hpp"
#include "asio/traits/query_static_constexpr_member.hpp"
#include "asio/traits/static_query.hpp"
#include "asio/traits/static_require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace execution {
namespace detail {
namespace bulk_guarantee {

template <int I> struct unsequenced_t;
template <int I> struct sequenced_t;
template <int I> struct parallel_t;

} // namespace bulk_guarantee

template <int I = 0>
struct bulk_guarantee_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = false);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = false);
  typedef bulk_guarantee_t polymorphic_query_result_type;

  typedef detail::bulk_guarantee::unsequenced_t<I> unsequenced_t;
  typedef detail::bulk_guarantee::sequenced_t<I> sequenced_t;
  typedef detail::bulk_guarantee::parallel_t<I> parallel_t;

  ASIO_CONSTEXPR bulk_guarantee_t()
    : value_(-1)
  {
  }

  ASIO_CONSTEXPR bulk_guarantee_t(unsequenced_t)
    : value_(0)
  {
  }

  ASIO_CONSTEXPR bulk_guarantee_t(sequenced_t)
    : value_(1)
  {
  }

  ASIO_CONSTEXPR bulk_guarantee_t(parallel_t)
    : value_(2)
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<
      T, bulk_guarantee_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, bulk_guarantee_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, bulk_guarantee_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, unsequenced_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, bulk_guarantee_t>::is_valid
          && !traits::query_member<T, bulk_guarantee_t>::is_valid
          && traits::static_query<T, unsequenced_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, unsequenced_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, sequenced_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, bulk_guarantee_t>::is_valid
          && !traits::query_member<T, bulk_guarantee_t>::is_valid
          && !traits::static_query<T, unsequenced_t>::is_valid
          && traits::static_query<T, sequenced_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, sequenced_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, parallel_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, bulk_guarantee_t>::is_valid
          && !traits::query_member<T, bulk_guarantee_t>::is_valid
          && !traits::static_query<T, unsequenced_t>::is_valid
          && !traits::static_query<T, sequenced_t>::is_valid
          && traits::static_query<T, parallel_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, parallel_t>::value();
  }

  template <typename E,
      typename T = decltype(bulk_guarantee_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = bulk_guarantee_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  friend ASIO_CONSTEXPR bool operator==(
      const bulk_guarantee_t& a, const bulk_guarantee_t& b)
  {
    return a.value_ == b.value_;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const bulk_guarantee_t& a, const bulk_guarantee_t& b)
  {
    return a.value_ != b.value_;
  }

  struct convertible_from_bulk_guarantee_t
  {
    ASIO_CONSTEXPR convertible_from_bulk_guarantee_t(bulk_guarantee_t) {}
  };

  template <typename Executor>
  friend ASIO_CONSTEXPR bulk_guarantee_t query(
      const Executor& ex, convertible_from_bulk_guarantee_t,
      typename enable_if<
        can_query<const Executor&, unsequenced_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&,
        bulk_guarantee_t<>::unsequenced_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, unsequenced_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, unsequenced_t());
  }

  template <typename Executor>
  friend ASIO_CONSTEXPR bulk_guarantee_t query(
      const Executor& ex, convertible_from_bulk_guarantee_t,
      typename enable_if<
        !can_query<const Executor&, unsequenced_t>::value
          && can_query<const Executor&, sequenced_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&,
        bulk_guarantee_t<>::sequenced_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, sequenced_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, sequenced_t());
  }

  template <typename Executor>
  friend ASIO_CONSTEXPR bulk_guarantee_t query(
      const Executor& ex, convertible_from_bulk_guarantee_t,
      typename enable_if<
        !can_query<const Executor&, unsequenced_t>::value
          && !can_query<const Executor&, sequenced_t>::value
          && can_query<const Executor&, parallel_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, bulk_guarantee_t<>::parallel_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, parallel_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, parallel_t());
  }

  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(unsequenced_t, unsequenced);
  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(sequenced_t, sequenced);
  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(parallel_t, parallel);

#if !defined(ASIO_HAS_CONSTEXPR)
  static const bulk_guarantee_t instance;
#endif // !defined(ASIO_HAS_CONSTEXPR)

private:
  int value_;
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T bulk_guarantee_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

#if !defined(ASIO_HAS_CONSTEXPR)
template <int I>
const bulk_guarantee_t<I> bulk_guarantee_t<I>::instance;
#endif

template <int I>
const typename bulk_guarantee_t<I>::unsequenced_t
bulk_guarantee_t<I>::unsequenced;

template <int I>
const typename bulk_guarantee_t<I>::sequenced_t
bulk_guarantee_t<I>::sequenced;

template <int I>
const typename bulk_guarantee_t<I>::parallel_t
bulk_guarantee_t<I>::parallel;

namespace bulk_guarantee {

template <int I = 0>
struct unsequenced_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef bulk_guarantee_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR unsequenced_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, unsequenced_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, unsequenced_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, unsequenced_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR unsequenced_t static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, unsequenced_t>::is_valid
          && !traits::query_member<T, unsequenced_t>::is_valid
          && !traits::query_free<T, unsequenced_t>::is_valid
          && !can_query<T, sequenced_t<I> >::value
          && !can_query<T, parallel_t<I> >::value
      >::type* = 0) ASIO_NOEXCEPT
  {
    return unsequenced_t();
  }

  template <typename E, typename T = decltype(unsequenced_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = unsequenced_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR bulk_guarantee_t<I> value()
  {
    return unsequenced_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const unsequenced_t&, const unsequenced_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const unsequenced_t&, const unsequenced_t&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const unsequenced_t&, const sequenced_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const unsequenced_t&, const sequenced_t<I>&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const unsequenced_t&, const parallel_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const unsequenced_t&, const parallel_t<I>&)
  {
    return true;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T unsequenced_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <int I = 0>
struct sequenced_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef bulk_guarantee_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR sequenced_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, sequenced_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, sequenced_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, sequenced_t>::value();
  }

  template <typename E, typename T = decltype(sequenced_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = sequenced_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR bulk_guarantee_t<I> value()
  {
    return sequenced_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const sequenced_t&, const sequenced_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const sequenced_t&, const sequenced_t&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const sequenced_t&, const unsequenced_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const sequenced_t&, const unsequenced_t<I>&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const sequenced_t&, const parallel_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const sequenced_t&, const parallel_t<I>&)
  {
    return true;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T sequenced_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <int I>
struct parallel_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef bulk_guarantee_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR parallel_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, parallel_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, parallel_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, parallel_t>::value();
  }

  template <typename E, typename T = decltype(parallel_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = parallel_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR bulk_guarantee_t<I> value()
  {
    return parallel_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const parallel_t&, const parallel_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const parallel_t&, const parallel_t&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const parallel_t&, const unsequenced_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const parallel_t&, const unsequenced_t<I>&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator==(
      const parallel_t&, const sequenced_t<I>&)
  {
    return false;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const parallel_t&, const sequenced_t<I>&)
  {
    return true;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T parallel_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

} // namespace bulk_guarantee
} // namespace detail

typedef detail::bulk_guarantee_t<> bulk_guarantee_t;

#if defined(ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
constexpr bulk_guarantee_t bulk_guarantee;
#else // defined(ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
namespace { static const bulk_guarantee_t&
  bulk_guarantee = bulk_guarantee_t::instance; }
#endif

} // namespace execution

#if !defined(ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T>
struct is_applicable_property<T, execution::bulk_guarantee_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::bulk_guarantee_t::unsequenced_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::bulk_guarantee_t::sequenced_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::bulk_guarantee_t::parallel_t>
  : execution::is_executor<T>
{
};

#endif // !defined(ASIO_HAS_VARIABLE_TEMPLATES)

namespace traits {

#if !defined(ASIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

template <typename T>
struct query_free_default<T, execution::bulk_guarantee_t,
  typename enable_if<
    can_query<T, execution::bulk_guarantee_t::unsequenced_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::bulk_guarantee_t::unsequenced_t>::value));

  typedef execution::bulk_guarantee_t result_type;
};

template <typename T>
struct query_free_default<T, execution::bulk_guarantee_t,
  typename enable_if<
    !can_query<T, execution::bulk_guarantee_t::unsequenced_t>::value
      && can_query<T, execution::bulk_guarantee_t::sequenced_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::bulk_guarantee_t::sequenced_t>::value));

  typedef execution::bulk_guarantee_t result_type;
};

template <typename T>
struct query_free_default<T, execution::bulk_guarantee_t,
  typename enable_if<
    !can_query<T, execution::bulk_guarantee_t::unsequenced_t>::value
      && !can_query<T, execution::bulk_guarantee_t::sequenced_t>::value
      && can_query<T, execution::bulk_guarantee_t::parallel_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::bulk_guarantee_t::parallel_t>::value));

  typedef execution::bulk_guarantee_t result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  || !defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <typename T>
struct static_query<T, execution::bulk_guarantee_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::bulk_guarantee_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
        execution::bulk_guarantee_t>::is_valid
      && !traits::query_member<T,
        execution::bulk_guarantee_t>::is_valid
      && traits::static_query<T,
        execution::bulk_guarantee_t::unsequenced_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::bulk_guarantee_t::unsequenced_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T,
        execution::bulk_guarantee_t::unsequenced_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
        execution::bulk_guarantee_t>::is_valid
      && !traits::query_member<T,
        execution::bulk_guarantee_t>::is_valid
      && !traits::static_query<T,
        execution::bulk_guarantee_t::unsequenced_t>::is_valid
      && traits::static_query<T,
        execution::bulk_guarantee_t::sequenced_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::bulk_guarantee_t::sequenced_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T,
        execution::bulk_guarantee_t::sequenced_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
        execution::bulk_guarantee_t>::is_valid
      && !traits::query_member<T,
        execution::bulk_guarantee_t>::is_valid
      && !traits::static_query<T,
        execution::bulk_guarantee_t::unsequenced_t>::is_valid
      && !traits::static_query<T,
        execution::bulk_guarantee_t::sequenced_t>::is_valid
      && traits::static_query<T,
        execution::bulk_guarantee_t::parallel_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::bulk_guarantee_t::parallel_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T,
        execution::bulk_guarantee_t::parallel_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t::unsequenced_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::unsequenced_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::bulk_guarantee_t::unsequenced_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::unsequenced_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t::unsequenced_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::unsequenced_t>::is_valid
      && !traits::query_member<T,
        execution::bulk_guarantee_t::unsequenced_t>::is_valid
      && !traits::query_free<T,
        execution::bulk_guarantee_t::unsequenced_t>::is_valid
      && !can_query<T, execution::bulk_guarantee_t::sequenced_t>::value
      && !can_query<T, execution::bulk_guarantee_t::parallel_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef execution::bulk_guarantee_t::unsequenced_t result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return result_type();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t::sequenced_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::sequenced_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::bulk_guarantee_t::sequenced_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::sequenced_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::bulk_guarantee_t::parallel_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::parallel_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::bulk_guarantee_t::parallel_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::bulk_guarantee_t::parallel_t>::value();
  }
};

#endif // !defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   || !defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

#if !defined(ASIO_HAS_DEDUCED_STATIC_REQUIRE_TRAIT)

template <typename T>
struct static_require<T, execution::bulk_guarantee_t::unsequenced_t,
  typename enable_if<
    static_query<T, execution::bulk_guarantee_t::unsequenced_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::bulk_guarantee_t::unsequenced_t>::result_type,
        execution::bulk_guarantee_t::unsequenced_t>::value));
};

template <typename T>
struct static_require<T, execution::bulk_guarantee_t::sequenced_t,
  typename enable_if<
    static_query<T, execution::bulk_guarantee_t::sequenced_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::bulk_guarantee_t::sequenced_t>::result_type,
        execution::bulk_guarantee_t::sequenced_t>::value));
};

template <typename T>
struct static_require<T, execution::bulk_guarantee_t::parallel_t,
  typename enable_if<
    static_query<T, execution::bulk_guarantee_t::parallel_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::bulk_guarantee_t::parallel_t>::result_type,
        execution::bulk_guarantee_t::parallel_t>::value));
};

#endif // !defined(ASIO_HAS_DEDUCED_STATIC_REQUIRE_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_BULK_GUARANTEE_HPP
