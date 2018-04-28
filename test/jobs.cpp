#include "job_manager.hpp"

#include <boost/asio.hpp>

#include <iostream>

using namespace site_archive::downloader;
namespace asio = boost::asio;

int main() {
  boost::asio::io_context ioc;
  boost::asio::thread_pool tp;

  auto manager = std::make_shared<JobManager>(ioc);
  for (auto n = 0; n < 1e2; ++n) {
    asio::spawn(ioc, [=](auto yield) {
      boost::system::error_code ec;
      auto work = manager->get_work(yield[ec]);
      if (ec)
        std::cout << ec.message() << '\n';
      else
        std::cout << work << '\n';
    });
    // boost::asio::post(ioc, [=]() {
    //   boost::system::error_code ec;
    //   manager->get_work([](boost::system::error_code ec, const std::string &y) {
    //     if (ec)
    //       std::cout << ec.message() << '\n';
    //     else
    //       std::cout << y << '\n';
    //   });
    // });
  }

  boost::asio::post(tp, [&]() { ioc.run(); });

  for (auto n = 0; n < 1e2; ++n) {
    manager->add_work({"hello" + std::to_string(n)});
  }
  tp.join();
}
