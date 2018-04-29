#include "log.hpp"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

using namespace site_archive::downloader::log;

int main() {
  boost::asio::io_context ioc;
  boost::asio::thread_pool tp{4};

  Writer w{ioc, "test.log"};
  for (auto i = 0; i < 1e6; i++)
    boost::asio::spawn(ioc, [&](auto yield) { w.write_entry(entry{69, "secret", "/tmp/"}, yield); });

  for (auto i = 0; i < 4; i++) boost::asio::post(tp, [&]() { ioc.run(); });
  tp.join();
}
