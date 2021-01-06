//
// detail/solaris_fenced_block.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SOLARIS_FENCED_BLOCK_HPP
#define ASIO_DETAIL_SOLARIS_FENCED_BLOCK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "RCF/external/asio/asio/detail/config.hpp"

#if defined(__sun)

#include <atomic.h>

#include "RCF/external/asio/asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class solaris_fenced_block
  : private noncopyable
{
public:
  // Constructor.
  solaris_fenced_block()
  {
    membar_consumer();
  }

  // Destructor.
  ~solaris_fenced_block()
  {
    membar_producer();
  }
};

} // namespace detail
} // namespace asio

#include "RCF/external/asio/asio/detail/pop_options.hpp"

#endif // defined(__sun)

#endif // ASIO_DETAIL_SOLARIS_FENCED_BLOCK_HPP
