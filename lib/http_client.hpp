#pragma once
#include "net/ssl_transport.hpp"
#include "net/tcp_transport.hpp"
#include "stdfs.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/handler_ptr.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/type_traits.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/version.hpp>

#include <functional>
#include <memory>
#include <string>

namespace site_archive::downloader {
template <typename Transport>
class basic_http_client {
 public:
  using error_callback = std::function<void(boost::system::error_code)>;

  basic_http_client(boost::asio::io_context &io_ctx) : m_io_ctx{io_ctx}, m_transport{io_ctx} {}

  void connect(const std::string &host, const std::string &service, error_callback on_connect) {
    m_host = host;
    return m_transport.connect(host, service, [on_connect = std::move(on_connect)](auto ec) { on_connect(ec); });
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

  template <typename CompletionToken>
  BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::system::error_code))
  get_file(const std::string &target, const std::filesystem::path &pth, CompletionToken &&ct) {
    boost::asio::async_completion<CompletionToken, void(boost::system::error_code)> init{ct};
    GetFileOp<BOOST_ASIO_HANDLER_TYPE(CompletionToken,
                                      void(boost::system::error_code))>{*this,
                                                                        std::move(init.completion_handler)}(target,
                                                                                                            pth);
    return init.result.get();
  }

 private:
  boost::asio::io_context &m_io_ctx;
  Transport m_transport;
  std::string m_host;
  template <typename Handler>
  struct GetFileOp;
};

template <typename Transport>
template <typename Handler>
struct basic_http_client<Transport>::GetFileOp {
  struct State {
    State(const Handler &, basic_http_client<Transport> &client) : m_client{client} {}
    basic_http_client<Transport> &m_client;
    boost::beast::flat_buffer m_buf;
    boost::beast::http::response<boost::beast::http::file_body> m_response;
    std::filesystem::path m_path;
    boost::beast::http::request<boost::beast::http::string_body> m_req;
    int m_stage = 0;
  };

  template <typename DeducedHandler>
  GetFileOp(basic_http_client<Transport> &client, DeducedHandler &&handler)
      : m_p{std::forward<DeducedHandler>(handler), client} {}

  using executor_type =
      boost::asio::associated_executor_t<Handler, decltype(std::declval<Transport &>().get_executor())>;
  executor_type get_executor() const noexcept {
    return boost::asio::get_associated_executor(m_p.handler(), m_p->m_client.m_transport.get_executor());
  }

  using allocator_type = boost::asio::associated_allocator_t<Handler>;
  allocator_type get_allocator() const noexcept { return boost::asio::get_associated_allocator(m_p.handler()); }

  auto operator()(boost::system::error_code ec, size_t) {
    namespace http = boost::beast::http;
    auto p = m_p;
    if (p->m_stage == 0) {
      if (ec) return p.invoke(ec);

      p->m_response.body().open(p->m_path.c_str(), boost::beast::file_mode::write, ec);
      if (ec) return p.invoke(ec);

      ++p->m_stage;
      return http::async_read(p->m_client.m_transport, p->m_buf, p->m_response, std::move(*this));
    } else {
      return p.invoke(ec);
    }
  }

  auto operator()(const std::string &target, std::filesystem::path path) {
    namespace http = boost::beast::http;
    auto p = m_p;
    p->m_path = std::move(path);
    p->m_req = {http::verb::get, target, 11};
    p->m_req.set(http::field::host, p->m_client.m_host);
    p->m_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    return http::async_write(p->m_client.m_transport, p->m_req, std::move(*this));
  }

  boost::beast::handler_ptr<State, Handler> m_p;
};

extern template class basic_http_client<net::TcpTransport>;
using HttpClient = basic_http_client<net::TcpTransport>;
extern template class basic_http_client<net::SslTransport>;
using HttpsClient = basic_http_client<net::SslTransport>;
}  // namespace site_archive::downloader
