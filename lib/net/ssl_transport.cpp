#include "ssl_transport.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/rfc2818_verification.hpp>

#include <string>

namespace site_archive::net {

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;

SslTransport::SslTransport(boost::asio::io_context &io_ctx)
    : Transport{io_ctx}, m_ssl_context{ssl::context::sslv23_client}, m_ssl_stream{m_socket, m_ssl_context} {
  m_ssl_context.set_default_verify_paths();
  m_ssl_stream.set_verify_mode(ssl::verify_peer);
}

void SslTransport::connect(std::string_view addr, std::string_view service, error_callback on_connect) {
  m_ssl_stream.set_verify_callback(ssl::rfc2818_verification{std::string{addr}});
  m_resolver.async_resolve(addr, service,
                           [this, on_connect = std::move(on_connect)](boost::system::error_code ec,
                                                                      const tcp::resolver::results_type &results) {
                             return on_resolve(ec, results, std::move(on_connect));
                           });
}

void SslTransport::connect(std::string_view addr, std::string_view service, boost::asio::yield_context yield) {
  m_ssl_stream.set_verify_callback(ssl::rfc2818_verification{std::string{addr}});
  boost::system::error_code ec;
  auto results = m_resolver.async_resolve(addr, service, yield[ec]);
  if (ec) return;
  asio::async_connect(m_socket, results, yield[ec]);
  if (ec) return;
  m_ssl_stream.async_handshake(decltype(m_ssl_stream)::client, yield[ec]);
}

void SslTransport::on_resolve(boost::system::error_code ec, const tcp::resolver::results_type &results,
                              error_callback on_connect) {
  if (!ec) {
    asio::async_connect(m_socket, results,
                        [this, on_connect = std::move(on_connect)](auto connect_ec, auto /*endpoint*/) {
                          if (!connect_ec)
                            return m_ssl_stream.async_handshake(decltype(m_ssl_stream)::client,
                                                                [on_connect = std::move(on_connect)](
                                                                    auto handshake_ec) { on_connect(handshake_ec); });
                          return on_connect(connect_ec);
                        });
  } else {
    on_connect(ec);
  }
}

void SslTransport::shutdown(error_callback on_shutdown) {
  m_ssl_stream.async_shutdown([this, on_shutdown = std::move(on_shutdown)](auto ec) {
    if (ec == boost::asio::error::eof) {
      ec.assign(0, ec.category());
    }
    if (!ec) {
      m_socket.shutdown(m_socket.shutdown_both, ec);
      return on_shutdown(ec);
    }
    return on_shutdown(ec);
  });
}
}  // namespace site_archive::net
