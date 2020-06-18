//
// prefer.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_PREFER_HPP
#define ASIO_PREFER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/is_applicable_property.hpp"
#include "asio/traits/prefer_free.hpp"
#include "asio/traits/prefer_member.hpp"
#include "asio/traits/require_free.hpp"
#include "asio/traits/require_member.hpp"
#include "asio/traits/static_require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio_prefer_fn {

using asio::decay;
using asio::declval;
using asio::enable_if;
using asio::is_applicable_property;
using asio::traits::prefer_free;
using asio::traits::prefer_member;
using asio::traits::require_free;
using asio::traits::require_member;
using asio::traits::static_require;

void prefer();
void require();

enum overload_type
{
  identity,
  call_require_member,
  call_require_free,
  call_prefer_member,
  call_prefer_free,
  two_props,
  n_props,
  ill_formed
};

template <typename T, typename Properties, typename = void>
struct call_traits
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = ill_formed);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

template <typename T, typename Property>
struct call_traits<T, void(Property),
  typename enable_if<
    (
      is_applicable_property<
        typename decay<T>::type,
        typename decay<Property>::type
      >::value
      &&
      decay<Property>::type::is_preferable
      &&
      static_require<T, Property>::is_valid
    )
  >::type>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = identity);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

#if defined(ASIO_HAS_MOVE)
  typedef ASIO_MOVE_ARG(T) result_type;
#else // defined(ASIO_HAS_MOVE)
  typedef ASIO_MOVE_ARG(typename decay<T>::type) result_type;
#endif // defined(ASIO_HAS_MOVE)
};

template <typename T, typename Property>
struct call_traits<T, void(Property),
  typename enable_if<
    (
      is_applicable_property<
        typename decay<T>::type,
        typename decay<Property>::type
      >::value
      &&
      decay<Property>::type::is_preferable
      &&
      !static_require<T, Property>::is_valid
      &&
      require_member<T, Property>::is_valid
    )
  >::type> :
  require_member<T, Property>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = call_require_member);
};

template <typename T, typename Property>
struct call_traits<T, void(Property),
  typename enable_if<
    (
      is_applicable_property<
        typename decay<T>::type,
        typename decay<Property>::type
      >::value
      &&
      decay<Property>::type::is_preferable
      &&
      !static_require<T, Property>::is_valid
      &&
      !require_member<T, Property>::is_valid
      &&
      require_free<T, Property>::is_valid
    )
  >::type> :
  require_free<T, Property>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = call_require_free);
};

template <typename T, typename Property>
struct call_traits<T, void(Property),
  typename enable_if<
    (
      is_applicable_property<
        typename decay<T>::type,
        typename decay<Property>::type
      >::value
      &&
      decay<Property>::type::is_preferable
      &&
      !static_require<T, Property>::is_valid
      &&
      !require_member<T, Property>::is_valid
      &&
      !require_free<T, Property>::is_valid
      &&
      prefer_member<T, Property>::is_valid
    )
  >::type> :
  prefer_member<T, Property>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = call_prefer_member);
};

template <typename T, typename Property>
struct call_traits<T, void(Property),
  typename enable_if<
    (
      is_applicable_property<
        typename decay<T>::type,
        typename decay<Property>::type
      >::value
      &&
      decay<Property>::type::is_preferable
      &&
      !static_require<T, Property>::is_valid
      &&
      !require_member<T, Property>::is_valid
      &&
      !require_free<T, Property>::is_valid
      &&
      !prefer_member<T, Property>::is_valid
      &&
      prefer_free<T, Property>::is_valid
    )
  >::type> :
  prefer_free<T, Property>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = call_prefer_free);
};

template <typename T, typename Property>
struct call_traits<T, void(Property),
  typename enable_if<
    (
      is_applicable_property<
        typename decay<T>::type,
        typename decay<Property>::type
      >::value
      &&
      decay<Property>::type::is_preferable
      &&
      !static_require<T, Property>::is_valid
      &&
      !require_member<T, Property>::is_valid
      &&
      !require_free<T, Property>::is_valid
      &&
      !prefer_member<T, Property>::is_valid
      &&
      !prefer_free<T, Property>::is_valid
    )
  >::type>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = identity);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);

#if defined(ASIO_HAS_MOVE)
  typedef ASIO_MOVE_ARG(T) result_type;
#else // defined(ASIO_HAS_MOVE)
  typedef ASIO_MOVE_ARG(typename decay<T>::type) result_type;
#endif // defined(ASIO_HAS_MOVE)
};

template <typename T, typename P0, typename P1>
struct call_traits<T, void(P0, P1),
  typename enable_if<
    call_traits<T, void(P0)>::overload != ill_formed
    &&
    call_traits<
      typename call_traits<T, void(P0)>::result_type,
      void(P1)
    >::overload != ill_formed
  >::type>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = two_props);

  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (
      call_traits<T, void(P0)>::is_noexcept
      &&
      call_traits<
        typename call_traits<T, void(P0)>::result_type,
        void(P1)
      >::is_noexcept
    ));

  typedef typename decay<
    typename call_traits<
      typename call_traits<T, void(P0)>::result_type,
      void(P1)
    >::result_type
  >::type result_type;
};

template <typename T, typename P0, typename P1, typename ASIO_ELLIPSIS PN>
struct call_traits<T, void(P0, P1, PN ASIO_ELLIPSIS),
  typename enable_if<
    call_traits<T, void(P0)>::overload != ill_formed
    &&
    call_traits<
      typename call_traits<T, void(P0)>::result_type,
      void(P1, PN ASIO_ELLIPSIS)
    >::overload != ill_formed
  >::type>
{
  ASIO_STATIC_CONSTEXPR(overload_type, overload = n_props);

  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    (
      call_traits<T, void(P0)>::is_noexcept
      &&
      call_traits<
        typename call_traits<T, void(P0)>::result_type,
        void(P1, PN ASIO_ELLIPSIS)
      >::is_noexcept
    ));

  typedef typename decay<
    typename call_traits<
      typename call_traits<T, void(P0)>::result_type,
      void(P1, PN ASIO_ELLIPSIS)
    >::result_type
  >::type result_type;
};

struct impl
{
  template <typename T, typename Property>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(Property)>::overload == identity,
    typename call_traits<T, void(Property)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(Property)) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(Property)>::is_noexcept))
  {
    return ASIO_MOVE_CAST(T)(t);
  }

  template <typename T, typename Property>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(Property)>::overload == call_require_member,
    typename call_traits<T, void(Property)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(Property) p) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(Property)>::is_noexcept))
  {
    return ASIO_MOVE_CAST(T)(t).require(
        ASIO_MOVE_CAST(Property)(p));
  }

  template <typename T, typename Property>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(Property)>::overload == call_require_free,
    typename call_traits<T, void(Property)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(Property) p) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(Property)>::is_noexcept))
  {
    return require(
        ASIO_MOVE_CAST(T)(t),
        ASIO_MOVE_CAST(Property)(p));
  }

  template <typename T, typename Property>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(Property)>::overload == call_prefer_member,
    typename call_traits<T, void(Property)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(Property) p) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(Property)>::is_noexcept))
  {
    return ASIO_MOVE_CAST(T)(t).prefer(
        ASIO_MOVE_CAST(Property)(p));
  }

  template <typename T, typename Property>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(Property)>::overload == call_prefer_free,
    typename call_traits<T, void(Property)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(Property) p) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(Property)>::is_noexcept))
  {
    return prefer(
        ASIO_MOVE_CAST(T)(t),
        ASIO_MOVE_CAST(Property)(p));
  }

  template <typename T, typename P0, typename P1>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(P0, P1)>::overload == two_props,
    typename call_traits<T, void(P0, P1)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(P0) p0,
      ASIO_MOVE_ARG(P1) p1) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(P0, P1)>::is_noexcept))
  {
    return (*this)(
        (*this)(
          ASIO_MOVE_CAST(T)(t),
          ASIO_MOVE_CAST(P0)(p0)),
        ASIO_MOVE_CAST(P1)(p1));
  }

  template <typename T, typename P0, typename P1,
    typename ASIO_ELLIPSIS PN>
  ASIO_CONSTEXPR typename enable_if<
    call_traits<T, void(P0, P1, PN ASIO_ELLIPSIS)>::overload == n_props,
    typename call_traits<T, void(P0, P1, PN ASIO_ELLIPSIS)>::result_type
  >::type
  operator()(
      ASIO_MOVE_ARG(T) t,
      ASIO_MOVE_ARG(P0) p0,
      ASIO_MOVE_ARG(P1) p1,
      ASIO_MOVE_ARG(PN) ASIO_ELLIPSIS pn) const
    ASIO_NOEXCEPT_IF((
      call_traits<T, void(P0, P1, PN ASIO_ELLIPSIS)>::is_noexcept))
  {
    return (*this)(
        (*this)(
          ASIO_MOVE_CAST(T)(t),
          ASIO_MOVE_CAST(P0)(p0)),
        ASIO_MOVE_CAST(P1)(p1),
        ASIO_MOVE_CAST(PN)(pn) ASIO_ELLIPSIS);
  }
};

template <typename T = impl>
struct static_instance
{
  static const T instance;
};

template <typename T>
const T static_instance<T>::instance = {};

} // namespace asio_prefer_fn
namespace asio {
namespace {

static ASIO_CONSTEXPR const asio_prefer_fn::impl&
  prefer = asio_prefer_fn::static_instance<>::instance;

} // namespace

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename T, typename... Properties>
struct can_prefer :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(Properties...)>::overload
      != asio_prefer_fn::ill_formed>
{
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename T, typename P0 = void,
    typename P1 = void, typename P2 = void>
struct can_prefer :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(P0, P1, P2)>::overload
      != asio_prefer_fn::ill_formed>
{
};

template <typename T, typename P0, typename P1>
struct can_prefer<T, P0, P1> :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(P0, P1)>::overload
      != asio_prefer_fn::ill_formed>
{
};

template <typename T, typename P0>
struct can_prefer<T, P0> :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(P0)>::overload
      != asio_prefer_fn::ill_formed>
{
};

template <typename T>
struct can_prefer<T> :
  false_type
{
};

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T, typename ASIO_ELLIPSIS Properties>
constexpr bool can_prefer_v
  = can_prefer<T, Properties ASIO_ELLIPSIS>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename T, typename... Properties>
struct is_nothrow_prefer :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(Properties...)>::is_noexcept>
{
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename T, typename P0 = void,
    typename P1 = void, typename P2 = void>
struct is_nothrow_prefer :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(P0, P1, P2)>::is_noexcept>
{
};

template <typename T, typename P0, typename P1>
struct is_nothrow_prefer<T, P0, P1> :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(P0, P1)>::is_noexcept>
{
};

template <typename T, typename P0>
struct is_nothrow_prefer<T, P0> :
  integral_constant<bool,
    asio_prefer_fn::call_traits<T, void(P0)>::is_noexcept>
{
};

template <typename T>
struct is_nothrow_prefer<T> :
  false_type
{
};

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T, typename ASIO_ELLIPSIS Properties>
constexpr bool is_nothrow_prefer_v
  = is_nothrow_prefer<T, Properties ASIO_ELLIPSIS>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename T, typename... Properties>
struct prefer_result_type
{
  typedef typename asio_prefer_fn::call_traits<
      T, void(Properties...)>::result_type type;
};

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename T, typename P0 = void,
    typename P1 = void, typename P2 = void>
struct prefer_result_type
{
  typedef typename asio_prefer_fn::call_traits<
      T, void(P0, P1, P2)>::result_type type;
};

template <typename T, typename P0, typename P1>
struct prefer_result_type<T, P0, P1>
{
  typedef typename asio_prefer_fn::call_traits<
      T, void(P0, P1)>::result_type type;
};

template <typename T, typename P0>
struct prefer_result_type<T, P0>
{
  typedef typename asio_prefer_fn::call_traits<
      T, void(P0)>::result_type type;
};

template <typename T>
struct prefer_result_type<T>
{
};

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_PREFER_HPP
