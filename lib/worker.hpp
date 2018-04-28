#pragma once
#include "http_client.hpp"
#include "job_manager.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

namespace site_archive::downloader {
class Worker {
 public:
  Worker(boost::asio::io_context &io_ctx) : m_io_ctx{io_ctx}, m_client{io_ctx} {}
  void do_work(boost::asio::yield_context yield);

 private:
  boost::asio::io_context &m_io_ctx;
  HttpClient m_client;
  std::shared_ptr<JobManager> m_job_mgr;
};
}  // namespace site_archive::downloader
