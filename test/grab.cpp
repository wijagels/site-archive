#include "http_client.hpp"

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <vector>

using site_archive::downloader::HttpClient;
using site_archive::downloader::HttpsClient;
namespace http = boost::beast::http;

auto console = spdlog::stdout_color_mt("console");

int main() {
  spdlog::set_async_mode(128);
  boost::asio::io_context ioc;

  for (auto n = 0; n < 10; ++n) {
    boost::asio::spawn(ioc, [=, client = std::make_shared<HttpClient>(ioc)](auto yield) {
      for (auto i = 0; i < 100; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        console->info("Started {}", n);
        std::ofstream f{fmt::format("dl-{}-{}", n, i)};
        boost::system::error_code ec;
        client->connect("127.0.0.1", "http", yield[ec]);
        if (ec) return console->error("Connection failed {}", ec.message());
        auto resp = client->get("http://localhost/test.bin", yield[ec]);
        if (ec) console->warn("Failed with {}", ec.message());
        if (resp.result() != http::status::ok) console->warn("Failed with {}", resp.result());
        f << resp;
        auto end = std::chrono::high_resolution_clock::now();
        console->info("Ended {}, elapsed {} ms", n,
                      std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
      }
    });
  }
  std::vector<std::future<void>> v;
  v.reserve(10);
  for (auto n = 0; n < 4; ++n) {
    v.emplace_back(std::async(std::launch::async, [&]() { ioc.run(); }));
  }
}
