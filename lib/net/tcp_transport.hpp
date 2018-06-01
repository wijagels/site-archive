#pragma once
#include "transport.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>

#include <string_view>

namespace site_archive::net {
class TcpTransport final : Transport {
 public:
  TcpTransport(boost::asio::io_context &io_ctx);
  void connect(std::string_view addr, std::string_view service, error_callback on_connect);
  void connect(std::string_view addr, std::string_view service, boost::asio::yield_context yield);
  void shutdown(error_callback on_shutdown);
  template <typename B, typename T>
  auto async_read_some(B &&mb, T &&t) {
    return m_socket.async_read_some(std::forward<B>(mb), std::forward<T>(t));
  }
  template <typename B, typename T>
  auto async_write_some(B &&cb, T &&t) {
    return m_socket.async_write_some(std::forward<B>(cb), std::forward<T>(t));
  }

  using executor_type = decltype(m_socket.get_executor());
  auto get_executor() noexcept { return m_socket.get_executor(); }

 private:
  void on_resolve(boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type &results,
                  error_callback on_connect);
};
}  // namespace site_archive::net
