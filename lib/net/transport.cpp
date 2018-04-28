#include "transport.hpp"

namespace site_archive::net {

Transport::Transport(boost::asio::io_context &io_ctx)
    : m_io_ctx{io_ctx}, m_socket{io_ctx}, m_resolver{io_ctx} {}

}  // namespace site_archive::net
