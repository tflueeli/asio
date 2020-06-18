//
// execution/blocking_adaptation.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_BLOCKING_ADAPTATION_HPP
#define ASIO_EXECUTION_BLOCKING_ADAPTATION_HPP

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
namespace blocking_adaptation {

template <int I> struct disallowed_t;
template <int I> struct allowed_t;

} // namespace blocking_adaptation

template <int I = 0>
struct blocking_adaptation_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = false);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = false);
  typedef blocking_adaptation_t polymorphic_query_result_type;

  typedef detail::blocking_adaptation::disallowed_t<I> disallowed_t;
  typedef detail::blocking_adaptation::allowed_t<I> allowed_t;

  ASIO_CONSTEXPR blocking_adaptation_t()
    : value_(-1)
  {
  }

  ASIO_CONSTEXPR blocking_adaptation_t(disallowed_t)
    : value_(0)
  {
  }

  ASIO_CONSTEXPR blocking_adaptation_t(allowed_t)
    : value_(1)
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<
      T, blocking_adaptation_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<
        T, blocking_adaptation_t
      >::is_noexcept))
  {
    return traits::query_static_constexpr_member<
        T, blocking_adaptation_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, disallowed_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<
            T, blocking_adaptation_t>::is_valid
          && !traits::query_member<T, blocking_adaptation_t>::is_valid
          && traits::static_query<T, disallowed_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, disallowed_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::static_query<T, allowed_t>::result_type
  static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<
            T, blocking_adaptation_t>::is_valid
          && !traits::query_member<T, blocking_adaptation_t>::is_valid
          && !traits::static_query<T, disallowed_t>::is_valid
          && traits::static_query<T, allowed_t>::is_valid
      >::type* = 0) ASIO_NOEXCEPT
  {
    return traits::static_query<T, allowed_t>::value();
  }

  template <typename E,
      typename T = decltype(blocking_adaptation_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = blocking_adaptation_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  friend ASIO_CONSTEXPR bool operator==(
      const blocking_adaptation_t& a, const blocking_adaptation_t& b)
  {
    return a.value_ == b.value_;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const blocking_adaptation_t& a, const blocking_adaptation_t& b)
  {
    return a.value_ != b.value_;
  }

  struct convertible_from_blocking_adaptation_t
  {
    ASIO_CONSTEXPR convertible_from_blocking_adaptation_t(
        blocking_adaptation_t)
    {
    }
  };

  template <typename Executor>
  friend ASIO_CONSTEXPR blocking_adaptation_t query(
      const Executor& ex, convertible_from_blocking_adaptation_t,
      typename enable_if<
        can_query<const Executor&, disallowed_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&,
        blocking_adaptation_t<>::disallowed_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, disallowed_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, disallowed_t());
  }

  template <typename Executor>
  friend ASIO_CONSTEXPR blocking_adaptation_t query(
      const Executor& ex, convertible_from_blocking_adaptation_t,
      typename enable_if<
        !can_query<const Executor&, disallowed_t>::value
          && can_query<const Executor&, allowed_t>::value
      >::type* = 0)
#if defined(_MSC_VER) // Visual C++ wants the type to be qualified.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&,
        blocking_adaptation_t<>::allowed_t>::value))
#elif !defined(__clang__) // Clang crashes if noexcept is used here.
    ASIO_NOEXCEPT_IF((
      is_nothrow_query<const Executor&, allowed_t>::value))
#endif // !defined(__clang__)
  {
    return asio::query(ex, allowed_t());
  }

  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(disallowed_t, disallowed);
  ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(allowed_t, allowed);

#if !defined(ASIO_HAS_CONSTEXPR)
  static const blocking_adaptation_t instance;
#endif // !defined(ASIO_HAS_CONSTEXPR)

private:
  int value_;
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T blocking_adaptation_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

#if !defined(ASIO_HAS_CONSTEXPR)
template <int I>
const blocking_adaptation_t<I> blocking_adaptation_t<I>::instance;
#endif

template <int I>
const typename blocking_adaptation_t<I>::disallowed_t
blocking_adaptation_t<I>::disallowed;

template <int I>
const typename blocking_adaptation_t<I>::allowed_t
blocking_adaptation_t<I>::allowed;

namespace blocking_adaptation {

template <int I = 0>
struct disallowed_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef blocking_adaptation_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR disallowed_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, disallowed_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, disallowed_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, disallowed_t>::value();
  }

  template <typename T>
  static ASIO_CONSTEXPR disallowed_t static_query(
      typename enable_if<
        !traits::query_static_constexpr_member<T, disallowed_t>::is_valid
          && !traits::query_member<T, disallowed_t>::is_valid
          && !traits::query_free<T, disallowed_t>::is_valid
          && !can_query<T, allowed_t<I> >::value
      >::type* = 0) ASIO_NOEXCEPT
  {
    return disallowed_t();
  }

  template <typename E, typename T = decltype(disallowed_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = disallowed_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR blocking_adaptation_t<I> value()
  {
    return disallowed_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const disallowed_t&, const disallowed_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const disallowed_t&, const disallowed_t&)
  {
    return false;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T disallowed_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <int I = 0>
struct allowed_t
{
#if defined(ASIO_HAS_VARIABLE_TEMPLATES)
  template <typename T>
  ASIO_STATIC_CONSTEXPR(bool,
    is_applicable_property_v = is_executor<T>::value);
#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

  ASIO_STATIC_CONSTEXPR(bool, is_requirable = true);
  ASIO_STATIC_CONSTEXPR(bool, is_preferable = true);
  typedef blocking_adaptation_t<I> polymorphic_query_result_type;

  ASIO_CONSTEXPR allowed_t()
  {
  }

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
  template <typename T>
  static ASIO_CONSTEXPR
  typename traits::query_static_constexpr_member<T, allowed_t>::result_type
  static_query()
    ASIO_NOEXCEPT_IF((
      traits::query_static_constexpr_member<T, allowed_t>::is_noexcept))
  {
    return traits::query_static_constexpr_member<T, allowed_t>::value();
  }

  template <typename E, typename T = decltype(allowed_t::static_query<E>())>
  static ASIO_CONSTEXPR const T static_query_v
    = allowed_t::static_query<E>();
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

  static ASIO_CONSTEXPR blocking_adaptation_t<I> value()
  {
    return allowed_t();
  }

  friend ASIO_CONSTEXPR bool operator==(
      const allowed_t&, const allowed_t&)
  {
    return true;
  }

  friend ASIO_CONSTEXPR bool operator!=(
      const allowed_t&, const allowed_t&)
  {
    return false;
  }
};

#if defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)
template <int I> template <typename E, typename T>
const T allowed_t<I>::static_query_v;
#endif // defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   && defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

} // namespace blocking_adaptation
} // namespace detail

typedef detail::blocking_adaptation_t<> blocking_adaptation_t;

#if defined(ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
constexpr blocking_adaptation_t blocking_adaptation;
#else // defined(ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
namespace { static const blocking_adaptation_t&
  blocking_adaptation = blocking_adaptation_t::instance; }
#endif

} // namespace execution

#if !defined(ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T>
struct is_applicable_property<T, execution::blocking_adaptation_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::blocking_adaptation_t::disallowed_t>
  : execution::is_executor<T>
{
};

template <typename T>
struct is_applicable_property<T, execution::blocking_adaptation_t::allowed_t>
  : execution::is_executor<T>
{
};

#endif // !defined(ASIO_HAS_VARIABLE_TEMPLATES)

namespace traits {

#if !defined(ASIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

template <typename T>
struct query_free_default<T, execution::blocking_adaptation_t,
  typename enable_if<
    can_query<T, execution::blocking_adaptation_t::disallowed_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = (is_nothrow_query<T,
      execution::blocking_adaptation_t::disallowed_t>::value));

  typedef execution::blocking_adaptation_t result_type;
};

template <typename T>
struct query_free_default<T, execution::blocking_adaptation_t,
  typename enable_if<
    !can_query<T, execution::blocking_adaptation_t::disallowed_t>::value
      && can_query<T, execution::blocking_adaptation_t::allowed_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (is_nothrow_query<T, execution::blocking_adaptation_t::allowed_t>::value));

  typedef execution::blocking_adaptation_t result_type;
};

#endif // !defined(ASIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

#if !defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT) \
  || !defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

template <typename T>
struct static_query<T, execution::blocking_adaptation_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_adaptation_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_adaptation_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_adaptation_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_adaptation_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
        execution::blocking_adaptation_t>::is_valid
      && !traits::query_member<T,
        execution::blocking_adaptation_t>::is_valid
      && traits::static_query<T,
        execution::blocking_adaptation_t::disallowed_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::blocking_adaptation_t::disallowed_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T,
        execution::blocking_adaptation_t::disallowed_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_adaptation_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
        execution::blocking_adaptation_t>::is_valid
      && !traits::query_member<T,
        execution::blocking_adaptation_t>::is_valid
      && !traits::static_query<T,
        execution::blocking_adaptation_t::disallowed_t>::is_valid
      && traits::static_query<T,
        execution::blocking_adaptation_t::allowed_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::static_query<T,
    execution::blocking_adaptation_t::allowed_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::static_query<T,
        execution::blocking_adaptation_t::allowed_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_adaptation_t::disallowed_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_adaptation_t::disallowed_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_adaptation_t::disallowed_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_adaptation_t::disallowed_t>::value();
  }
};

template <typename T>
struct static_query<T, execution::blocking_adaptation_t::disallowed_t,
  typename enable_if<
    !traits::query_static_constexpr_member<T,
        execution::blocking_adaptation_t::disallowed_t>::is_valid
      && !traits::query_member<T,
        execution::blocking_adaptation_t::disallowed_t>::is_valid
      && !traits::query_free<T,
        execution::blocking_adaptation_t::disallowed_t>::is_valid
      && !can_query<T, execution::blocking_adaptation_t::allowed_t>::value
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef execution::blocking_adaptation_t::disallowed_t result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return result_type();
  }
};

template <typename T>
struct static_query<T, execution::blocking_adaptation_t::allowed_t,
  typename enable_if<
    traits::query_static_constexpr_member<T,
      execution::blocking_adaptation_t::allowed_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

  typedef typename traits::query_static_constexpr_member<T,
    execution::blocking_adaptation_t::allowed_t>::result_type result_type;

  static ASIO_CONSTEXPR result_type value()
  {
    return traits::query_static_constexpr_member<T,
      execution::blocking_adaptation_t::allowed_t>::value();
  }
};

#endif // !defined(ASIO_HAS_DEDUCED_STATIC_QUERY_TRAIT)
       //   || !defined(ASIO_HAS_SFINAE_VARIABLE_TEMPLATES)

#if !defined(ASIO_HAS_DEDUCED_STATIC_REQUIRE_TRAIT)

template <typename T>
struct static_require<T, execution::blocking_adaptation_t::disallowed_t,
  typename enable_if<
    static_query<T, execution::blocking_adaptation_t::disallowed_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::blocking_adaptation_t::disallowed_t>::result_type,
        execution::blocking_adaptation_t::disallowed_t>::value));
};

template <typename T>
struct static_require<T, execution::blocking_adaptation_t::allowed_t,
  typename enable_if<
    static_query<T, execution::blocking_adaptation_t::allowed_t>::is_valid
  >::type>
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid =
    (is_same<typename static_query<T,
      execution::blocking_adaptation_t::allowed_t>::result_type,
        execution::blocking_adaptation_t::allowed_t>::value));
};

#endif // !defined(ASIO_HAS_DEDUCED_STATIC_REQUIRE_TRAIT)

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_BLOCKING_ADAPTATION_HPP
