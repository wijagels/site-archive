#include "tcp_transport.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>

namespace site_archive::net {

using tcp = boost::asio::ip::tcp;
namespace asio = boost::asio;

TcpTransport::TcpTransport(boost::asio::io_context &io_ctx) : Transport{io_ctx} {}

void TcpTransport::connect(const std::string &addr, const std::string &service, error_callback on_connect) {
  return m_resolver.async_resolve({addr, service},
                                  [=, on_connect = std::move(on_connect), self = shared_from_this()](
                                      boost::system::error_code ec, const tcp::resolver::results_type &results) {
                                    return on_resolve(ec, results, std::move(on_connect));
                                  });
}

void TcpTransport::connect(const std::string &addr, const std::string &service, boost::asio::yield_context yield) {
  boost::system::error_code ec;
  auto results = m_resolver.async_resolve({addr, service}, yield[ec]);
  if (ec) return;
  asio::async_connect(m_socket, results, yield[ec]);
}

void TcpTransport::on_resolve(boost::system::error_code ec, const tcp::resolver::results_type &results,
                              error_callback on_connect) {
  if (!ec)
    return asio::async_connect(m_socket, results,
                               [=, on_connect = std::move(on_connect), self = shared_from_this()](
                                   auto connect_ec, auto /*endpoint*/) { return on_connect(connect_ec); });
  return on_connect(ec);
}

void TcpTransport::shutdown(error_callback on_shutdown) {
  boost::system::error_code ec;
  m_socket.shutdown(m_socket.shutdown_both);
  return on_shutdown(ec);
}
}  // namespace site_archive::net
