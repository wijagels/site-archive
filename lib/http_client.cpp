#include "http_client.hpp"

namespace site_archive::downloader {
template class basic_http_client<net::TcpTransport>;
template class basic_http_client<net::SslTransport>;
}  // namespace site_archive::downloader
