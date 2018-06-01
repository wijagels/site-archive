#include "job_manager.hpp"
#include "worker.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/format.h>

#include <regex>
#include <string>
#include <vector>

namespace asio = boost::asio;

namespace site_archive::downloader {
Worker::Worker(boost::asio::io_context &io_ctx, JobManager &job_mgr, std::string host, std::string service)
    : m_io_ctx{io_ctx},
      m_client{io_ctx},
      m_job_mgr{job_mgr},
      m_host{std::move(host)},
      m_service{std::move(service)},
      m_re{fmt::format(R"x("([\w\.]*(?:{})?/[%\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+)")x",
                       boost::replace_all_copy(m_host, ".", R"(\.)")),
           std::regex_constants::ECMAScript | std::regex_constants::optimize} {}

void Worker::run() {
  m_client.connect(m_host, m_service, [=](auto ec) {
    if (ec) {
      return;
    }
    asio::spawn(m_io_ctx, [this](auto yield) { run_coro(yield); });
  });
}

void Worker::run_coro(boost::asio::yield_context yield) {
  for (;;) {
    boost::system::error_code ec;
    auto work = m_job_mgr.get_work(yield[ec]);
    if (ec) break;
    auto resp = m_client.get(work, yield[ec]);
    if (ec) break;
    m_os.str("");
    m_os << resp;
    std::string str = m_os.str();

    auto begin = std::sregex_iterator(str.begin(), str.end(), m_re);
    auto end = std::sregex_iterator();
    std::vector<std::string> vec;

    for (auto it = begin; it != end; ++it) {
      vec.emplace_back(it->operator[](1).str());
    }

    m_job_mgr.add_work(std::move(vec));
  }
}
}  // namespace site_archive::downloader
