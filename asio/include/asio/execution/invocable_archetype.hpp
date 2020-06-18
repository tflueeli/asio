//
// execution/invocable_archetype.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_INVOCABLE_ARCHETYPE_HPP
#define ASIO_EXECUTION_INVOCABLE_ARCHETYPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/detail/variadic_templates.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace execution {

struct invocable_archetype
{
#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename... Args>
  void operator()(ASIO_MOVE_ARG(Args)...)
  {
  }

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  void operator()()
  {
  }

#define ASIO_PRIVATE_INVOCABLE_ARCHETYPE_CALL_DEF(n) \
  template <ASIO_VARIADIC_TPARAMS(n)> \
  void operator()(ASIO_VARIADIC_UNNAMED_MOVE_PARAMS(n)) \
  { \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_INVOCABLE_ARCHETYPE_CALL_DEF)
#undef ASIO_PRIVATE_INVOCABLE_ARCHETYPE_CALL_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)
};

} // namespace execution
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_INVOCABLE_ARCHETYPE_HPP

