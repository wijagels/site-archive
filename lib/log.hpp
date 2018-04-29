#pragma once
#include "stdfs.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/handler_ptr.hpp>
#include <boost/hana/define_struct.hpp>

#include <array>
#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <string>

namespace site_archive::downloader::log {
struct header {
  BOOST_HANA_DEFINE_STRUCT(header, (std::string, timestamp));

  friend std::ostream &operator<<(std::ostream &os, const header &e);
  friend std::istream &operator>>(std::ostream &is, header &e);
};
struct entry {
  BOOST_HANA_DEFINE_STRUCT(entry, (int64_t, size), (std::string, sha256_digest), (std::filesystem::path, path));

  friend std::ostream &operator<<(std::ostream &os, const entry &e);
  friend std::istream &operator>>(std::ostream &is, entry &e);
};

class Writer {
  boost::asio::io_context &m_io_ctx;
  boost::asio::io_context::strand m_strand;
  std::ofstream m_fs;

  template <typename Handler>
  struct write_op;

 public:
  Writer(boost::asio::io_context &io_ctx, const std::filesystem::path &file);
  template <typename Handler>
  BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code))
  write_entry(entry e, Handler &&ct) {
    boost::asio::async_completion<Handler, void(boost::system::error_code)> init{ct};
    boost::asio::post(
        write_op<BOOST_ASIO_HANDLER_TYPE(Handler, void(boost::system::error_code))>{*this, std::move(e),
                                                                                    init.completion_handler});
    return init.result.get();
  }
};

template <typename Handler>
struct Writer::write_op {
  struct State {
    explicit State(const Handler &, Writer &w, entry e) : m_writer{w}, m_entry{std::move(e)} {}
    Writer &m_writer;
    entry m_entry;
  };
  boost::beast::handler_ptr<State, Handler> m_p;

  using executor_type = decltype(m_p->m_writer.m_strand);
  executor_type get_executor() const noexcept { return m_p->m_writer.m_strand; }

  using allocator_type = boost::asio::associated_allocator_t<Handler>;
  allocator_type get_allocator() const noexcept { return boost::asio::get_associated_allocator(m_p.handler()); }

  template <typename DeducedHandler>
  write_op(Writer &w, entry e, DeducedHandler &&h) : m_p{std::forward<DeducedHandler>(h), w, std::move(e)} {}
  void operator()() {
    m_p->m_writer.m_fs << m_p->m_entry << '\n';
    return m_p.invoke();
  }
};
}  // namespace site_archive::downloader::log
