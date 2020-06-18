//
// impl/post.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_POST_HPP
#define ASIO_IMPL_POST_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/associated_allocator.hpp"
#include "asio/associated_executor.hpp"
#include "asio/detail/work_dispatcher.hpp"
#include "asio/execution/allocator.hpp"
#include "asio/execution/blocking.hpp"
#include "asio/execution/relationship.hpp"
#include "asio/executor_adapter.hpp"
#include "asio/prefer.hpp"
#include "asio/require.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class initiate_post
{
public:
  template <typename CompletionHandler>
  void operator()(ASIO_MOVE_ARG(CompletionHandler) handler) const
  {
    typedef typename decay<CompletionHandler>::type handler_t;

    typename adapted_executor_type<
        typename associated_executor<handler_t>::type
      >::type ex((get_associated_executor)(handler));

    typename associated_allocator<handler_t>::type alloc(
        (get_associated_allocator)(handler));

    execution::execute(
        asio::prefer(
          asio::require(ex, execution::blocking.never),
          execution::relationship.fork,
          execution::allocator(alloc)),
        ASIO_MOVE_CAST(CompletionHandler)(handler));
  }
};

template <typename Executor>
class initiate_post_with_executor
{
public:
  typedef typename adapted_executor_type<Executor>::type executor_type;

  explicit initiate_post_with_executor(const Executor& ex)
    : ex_(ex)
  {
  }

  executor_type get_executor() const ASIO_NOEXCEPT
  {
    return ex_;
  }

  template <typename CompletionHandler>
  void operator()(ASIO_MOVE_ARG(CompletionHandler) handler) const
  {
    typedef typename decay<CompletionHandler>::type handler_t;

    typedef typename adapted_executor_type<
        typename associated_executor<handler_t, executor_type>::type
      >::type handler_ex_t;

    handler_ex_t handler_ex((get_associated_executor)(handler, ex_));

    typename associated_allocator<handler_t>::type alloc(
        (get_associated_allocator)(handler));

    if (this->is_same_executor(ex_, handler_ex))
    {
      execution::execute(
          asio::prefer(
            asio::require(ex_, execution::blocking.never),
            execution::relationship.fork,
            execution::allocator(alloc)),
          ASIO_MOVE_CAST(CompletionHandler)(handler));
    }
    else
    {
      execution::execute(
          asio::prefer(
            asio::require(ex_, execution::blocking.never),
            execution::relationship.fork,
            execution::allocator(alloc)),
          detail::work_dispatcher<handler_t, handler_ex_t>(
            ASIO_MOVE_CAST(CompletionHandler)(handler), handler_ex));
    }
  }

private:
  template <typename T, typename U>
  bool is_same_executor(const T&, const U&) const ASIO_NOEXCEPT
  {
    return false;
  }

  template <typename T>
  bool is_same_executor(const T& a, const T& b) const ASIO_NOEXCEPT
  {
    return a == b;
  }

  executor_type ex_;
};

} // namespace detail

template <ASIO_COMPLETION_TOKEN_FOR(void()) CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void()) post(
    ASIO_MOVE_ARG(CompletionToken) token)
{
  return async_initiate<CompletionToken, void()>(
      detail::initiate_post(), token);
}

template <typename Executor,
    ASIO_COMPLETION_TOKEN_FOR(void()) CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void()) post(
    const Executor& ex, ASIO_MOVE_ARG(CompletionToken) token,
    typename enable_if<
      execution::is_executor<Executor>::value || is_executor<Executor>::value
    >::type*)
{
  return async_initiate<CompletionToken, void()>(
      detail::initiate_post_with_executor<Executor>(ex), token);
}

template <typename ExecutionContext,
    ASIO_COMPLETION_TOKEN_FOR(void()) CompletionToken>
inline ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void()) post(
    ExecutionContext& ctx, ASIO_MOVE_ARG(CompletionToken) token,
    typename enable_if<is_convertible<
      ExecutionContext&, execution_context&>::value>::type*)
{
  return (post)(ctx.get_executor(),
      ASIO_MOVE_CAST(CompletionToken)(token));
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_POST_HPP
