#include "http_client.hpp"
#include "job_manager.hpp"
#include "worker.hpp"

#include <boost/asio/thread_pool.hpp>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <vector>
#include <list>

using site_archive::downloader::HttpClient;
using site_archive::downloader::HttpsClient;
using site_archive::downloader::JobManager;
using site_archive::downloader::Worker;
namespace asio = boost::asio;
namespace http = boost::beast::http;

static auto console = spdlog::stdout_color_mt("console");

// int main() {
//   boost::asio::io_context ioc;
//   HttpClient client{ioc};
//   boost::asio::spawn([&](asio::yield_context yield) {
//     boost::system::error_code ec;
//     client.connect("wtfismyip.com", "http", yield[ec]);
//     client.get_file("/json", "/tmp/coro.json", yield[ec]);
//     console->info(ec.message());
//   });

//   boost::asio::post([&]() {
//     boost::system::error_code ec;
//     client.connect("wtfismyip.com", "http", [&](auto) {
//       client.get_file("/json", "/tmp/cb.json", [&](auto ec) { console->info(ec.message()); });
//     });
//   });
//   ioc.run();
// }

int main(int argc, char *argv[]) {
  assert(argc == 3);
  boost::asio::io_context ioc;
  JobManager mgr = JobManager{ioc};
  mgr.add_work({"/test-sites/"});

  std::list<Worker> workers; // Worker is an immovable type
  for (auto i = 0; i < atoi(argv[1]); i++) workers.emplace_back(ioc, mgr, "webscraper.io", "http").run();

  std::vector<std::thread> threads;
  auto n_threads = atoi(argv[2]);
  for (auto i = 0; i < n_threads; i++) threads.emplace_back([&]() { ioc.run(); });
  for (auto &e : threads) e.join();
}
