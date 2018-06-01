#include "job_manager.hpp"

#include <boost/asio/post.hpp>

#include <string>
#include <vector>

namespace asio = boost::asio;

namespace site_archive::downloader {
JobManager::JobManager(asio::io_context &io_ctx) : m_io_ctx{io_ctx}, m_strand{io_ctx} {}

void JobManager::add_work(std::vector<std::string> &&work) {
  return asio::post(m_strand, [this, work = std::move(work)]() {
    for (auto &e : work) {
      auto [it, inserted] = m_seen.emplace(e);
      if (inserted) m_pending.emplace_back(*it);
    }
  });
}

}  // namespace site_archive::downloader
