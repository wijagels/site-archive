#include "log.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/post.hpp>
#include <boost/hana/accessors.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/intersperse.hpp>
#include <boost/hana/keys.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/zip_with.hpp>
#include <boost/lexical_cast.hpp>

#include <istream>
#include <ostream>

namespace hana = boost::hana;

namespace site_archive::downloader::log {
template <typename T>
static std::ostream &serialize(std::ostream &os, T &&e) {
  hana::for_each(hana::intersperse(hana::members(e), '|'), [&](auto elem) { os << elem; });
  return os;
}

template <typename T>
static std::istream &deserialize(std::istream &is, T &e) {
  std::vector<std::string> tok;
  std::string line;
  is >> line;
  boost::split(tok, line, boost::is_any_of("|"));
  constexpr size_t data_members = hana::size(hana::keys(e));
  if (tok.size() != data_members) throw std::runtime_error{"Invalid log line: " + line};
  size_t i = 0;
  hana::for_each(hana::accessors<T>(), [&](auto &&p) {
    auto getter = hana::second(p);
    getter(e) = boost::lexical_cast<std::decay_t<decltype(getter(e))>>(tok[i++]);
  });
  return is;
}

std::ostream &operator<<(std::ostream &os, const entry &e) { return serialize(os, e); }
std::istream &operator>>(std::istream &is, entry &e) { return deserialize(is, e); }
std::ostream &operator<<(std::ostream &os, const header &e) { return serialize(os, e); }
std::istream &operator>>(std::istream &is, header &e) { return deserialize(is, e); }

Writer::Writer(boost::asio::io_context &io_ctx, const std::filesystem::path &file)
    : m_io_ctx{io_ctx}, m_strand{io_ctx}, m_fs{file} {
    m_fs << header{"helloworld"} << '\n';
}

}  // namespace site_archive::downloader::log
