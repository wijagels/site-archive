#pragma once
#include "net/ssl_transport.hpp"
#include "net/tcp_transport.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <functional>
#include <memory>
#include <string>

namespace site_archive::downloader {
template <typename Transport>
class basic_http_client : public std::enable_shared_from_this<basic_http_client<Transport>> {
  using std::enable_shared_from_this<basic_http_client<Transport>>::shared_from_this;

 public:
  using error_callback = std::function<void(boost::system::error_code)>;

  basic_http_client(boost::asio::io_context &io_ctx) : m_io_ctx{io_ctx}, m_transport{io_ctx} {}

  void connect(const std::string &host, const std::string &service, error_callback on_connect) {
    m_host = host;
    return m_transport.connect(host, service, [on_connect = std::move(on_connect), self = shared_from_this()](auto ec) {
      on_connect(ec);
    });
  }

  void connect(const std::string &host, const std::string &service, boost::asio::yield_context yield) {
    m_host = host;
    m_transport.connect(host, service, yield);
  }

  boost::beast::http::response<boost::beast::http::dynamic_body> get(const std::string &target,
                                                                     boost::asio::yield_context yield) {
    namespace http = boost::beast::http;
    boost::system::error_code ec;
    http::request<http::string_body> req{http::verb::get, target, 11};
    req.set(http::field::host, m_host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    http::async_write(m_transport, req, yield[ec]);
    if (ec) return {};
    boost::beast::flat_buffer buf;
    http::response<http::dynamic_body> response;
    http::async_read(m_transport, buf, response, yield[ec]);
    return response;
  }

 private:
  boost::asio::io_context &m_io_ctx;
  Transport m_transport;
  std::string m_host;
};

extern template class basic_http_client<net::TcpTransport>;
using HttpClient = basic_http_client<net::TcpTransport>;
extern template class basic_http_client<net::SslTransport>;
using HttpsClient = basic_http_client<net::SslTransport>;
}  // namespace site_archive::downloader
