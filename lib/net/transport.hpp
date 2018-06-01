#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>

namespace site_archive::net {
/**
 * Common class for tcp based network transports
 * Not intended for polymorphic use
 */
class Transport {
 protected:
  using error_callback = std::function<void(boost::system::error_code)>;

  explicit Transport(boost::asio::io_context &io_ctx);
  ~Transport() = default;
  Transport(const Transport &) = delete;
  Transport(Transport &&) = default;
  Transport &operator=(const Transport &) = delete;
  Transport &operator=(Transport &&) = default;

  boost::asio::io_context &m_io_ctx;
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::ip::tcp::resolver m_resolver;
};
}  // namespace site_archive::net
