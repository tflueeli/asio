//
// any_executor.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/execution/any_executor.hpp"

#include "asio/thread_pool.hpp"
#include "../unit_test.hpp"

#if defined(ASIO_HAS_BOOST_BIND)
# include <boost/bind/bind.hpp>
#else // defined(ASIO_HAS_BOOST_BIND)
# include <functional>
#endif // defined(ASIO_HAS_BOOST_BIND)

using namespace asio;

#if defined(ASIO_HAS_BOOST_BIND)
namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
namespace bindns = std;
#endif

void increment(int* count)
{
  ++(*count);
}

void any_executor_executor_query_test()
{
  thread_pool pool;
  execution::any_executor<
      execution::blocking_t,
      execution::outstanding_work_t,
      execution::relationship_t,
      execution::mapping_t::thread_t>
    ex(pool.executor());

  ASIO_CHECK(
      asio::query(ex, asio::execution::blocking)
        == asio::execution::blocking.possibly);

  ASIO_CHECK(
      asio::query(ex, asio::execution::blocking.possibly)
        == asio::execution::blocking.possibly);

  ASIO_CHECK(
      asio::query(ex, asio::execution::outstanding_work)
        == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(ex, asio::execution::outstanding_work.untracked)
        == asio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      asio::query(ex, asio::execution::relationship)
        == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(ex, asio::execution::relationship.fork)
        == asio::execution::relationship.fork);

  ASIO_CHECK(
      asio::query(ex, asio::execution::mapping)
        == asio::execution::mapping.thread);
}

void any_executor_executor_execute_test()
{
  int count = 0;
  thread_pool pool(1);
  execution::any_executor<
      execution::blocking_t::possibly_t,
      execution::blocking_t::never_t,
      execution::outstanding_work_t::untracked_t,
      execution::outstanding_work_t::tracked_t,
      execution::relationship_t::continuation_t>
    ex(pool.executor());

  asio::execution::execute(pool.executor(),
      bindns::bind(increment, &count));

  asio::execution::execute(
      asio::require(pool.executor(),
        asio::execution::blocking.possibly),
      bindns::bind(increment, &count));

  asio::execution::execute(
      asio::require(pool.executor(),
        asio::execution::blocking.never),
      bindns::bind(increment, &count));

  asio::execution::execute(
      asio::require(pool.executor(),
        asio::execution::blocking.never,
        asio::execution::outstanding_work.tracked),
      bindns::bind(increment, &count));

  asio::execution::execute(
      asio::require(pool.executor(),
        asio::execution::blocking.never,
        asio::execution::outstanding_work.untracked),
      bindns::bind(increment, &count));

  asio::execution::execute(
      asio::require(pool.executor(),
        asio::execution::blocking.never,
        asio::execution::outstanding_work.untracked,
        asio::execution::relationship.continuation),
      bindns::bind(increment, &count));

  pool.wait();

  ASIO_CHECK(count == 6);
}

ASIO_TEST_SUITE
(
  "any_executor",
  ASIO_TEST_CASE(any_executor_executor_query_test)
  ASIO_TEST_CASE(any_executor_executor_execute_test)
)
