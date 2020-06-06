//
// detail/handler_work.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_HANDLER_WORK_HPP
#define ASIO_DETAIL_HANDLER_WORK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/associated_executor.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution/allocator.hpp"
#include "asio/execution/blocking.hpp"
#include "asio/execution/execute.hpp"
#include "asio/execution/executor.hpp"
#include "asio/execution/outstanding_work.hpp"
#include "asio/executor_work_guard.hpp"
#include "asio/prefer.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename IoExecutor, typename = void>
class handler_work_io_executor
{
public:
  explicit handler_work_io_executor(const IoExecutor& io_ex)
    : io_executor_(asio::prefer(io_ex,
          execution::outstanding_work.tracked))
  {
  }

private:
  typename decay<
      typename prefer_result_type<IoExecutor,
        execution::outstanding_work_t::tracked_t
      >::type
    >::type io_executor_;
};

template <typename IoExecutor>
class handler_work_io_executor<IoExecutor,
    typename enable_if<
      !execution::is_executor<IoExecutor>::value
    >::type>
{
public:
  explicit handler_work_io_executor(const IoExecutor& io_ex) ASIO_NOEXCEPT
    : io_executor_(io_ex)
  {
  }

private:
  executor_work_guard<IoExecutor> io_executor_;
};

template <typename Handler, typename IoExecutor, typename = void>
class handler_work : public handler_work_io_executor<IoExecutor>
{
public:
  handler_work(Handler& handler, const IoExecutor& io_ex)
    : handler_work_io_executor<IoExecutor>(io_ex),
      executor_(asio::prefer(
            (get_associated_executor)(handler, io_ex),
            execution::allocator((get_associated_allocator)(handler)),
            execution::blocking.possibly,
            execution::outstanding_work.tracked))
  {
  }

  template <typename Function>
  void complete(Function& function, Handler&)
  {
    execution::execute(executor_, ASIO_MOVE_CAST(Function)(function));
  }

private:
  typename decay<
      typename prefer_result_type<
        typename associated_executor<Handler, IoExecutor>::type,
        execution::allocator_t<typename associated_allocator<Handler>::type>,
        execution::blocking_t::possibly_t,
        execution::outstanding_work_t::tracked_t
      >::type
    >::type executor_;
};

template <typename Handler, typename IoExecutor>
class handler_work<Handler, IoExecutor,
    typename enable_if<
      !execution::is_executor<
        typename associated_executor<Handler, IoExecutor>::type
      >::value
    >::type> : public handler_work_io_executor<IoExecutor>
{
public:
  handler_work(Handler& handler, const IoExecutor& io_ex)
    : handler_work_io_executor<IoExecutor>(io_ex),
      work_((get_associated_executor)(handler, io_ex))
  {
  }

  template <typename Function>
  void complete(Function& function, Handler& handler)
  {
    work_.get_executor().dispatch(
        ASIO_MOVE_CAST(Function)(function),
        asio::get_associated_allocator(handler));
  }

private:
  executor_work_guard<
      typename associated_executor<Handler, IoExecutor>::type
    > work_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_HANDLER_WORK_HPP
