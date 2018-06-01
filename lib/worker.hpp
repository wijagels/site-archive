#pragma once
#include "http_client.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <regex>
#include <sstream>
#include <string>

namespace site_archive::downloader {
class JobManager;

class Worker {
 public:
  Worker(boost::asio::io_context &io_ctx, JobManager &job_mgr, std::string host, std::string service);
  ~Worker() = default;
  Worker(const Worker &) = delete;
  Worker(Worker &&) = delete;
  Worker &operator=(const Worker &) = delete;
  Worker &operator=(Worker &&) = delete;

  void run();

 private:
  void run_coro(boost::asio::yield_context yield);

  boost::asio::io_context &m_io_ctx;
  HttpClient m_client;
  JobManager &m_job_mgr;
  std::string m_host;
  std::string m_service;
  std::regex m_re;
  std::ostringstream m_os;
};
}  // namespace site_archive::downloader
