#pragma once
#include "transport.hpp"

#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <string>

namespace site_archive::net {
class SslTransport final : Transport {
 public:
  SslTransport(boost::asio::io_context &io_ctx);
  void connect(const std::string &addr, const std::string &service, error_callback on_connect);
  void connect(const std::string &addr, const std::string &service, boost::asio::yield_context yield);
  void shutdown(error_callback on_shutdown);
  template <typename B, typename T>
  auto async_read_some(B &&mb, T &&t) {
    return m_ssl_stream.async_read_some(std::forward<B>(mb), std::forward<T>(t));
  }
  template <typename B, typename T>
  auto async_write_some(B &&cb, T &&t) {
    return m_ssl_stream.async_write_some(std::forward<B>(cb), std::forward<T>(t));
  }
  auto &get_executor() { return m_io_ctx; }

 private:
  void on_resolve(boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type &results,
                  error_callback on_connect);

  void on_resolve(boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type &results,
                  boost::asio::yield_context yield);

  boost::asio::ssl::context m_ssl_context;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket &> m_ssl_stream;
};
}  // namespace site_archive::net
