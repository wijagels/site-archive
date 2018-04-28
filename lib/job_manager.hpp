#pragma once
#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>

#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace site_archive::downloader {

class JobManager : public std::enable_shared_from_this<JobManager> {
 public:
  JobManager(boost::asio::io_context &io_ctx);

  void add_work(std::vector<std::string> &&work);

  template <typename CompletionToken>
  BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(boost::system::error_code, std::string))
  get_work(CompletionToken &&ct) {
    boost::asio::async_completion<CompletionToken, void(boost::system::error_code, std::string)> init{ct};
    JobHandler<BOOST_ASIO_HANDLER_TYPE(CompletionToken, void(boost::system::error_code, std::string))>
        handler{*this, init.completion_handler};
    boost::asio::bind_executor(m_strand, handler);
    handler({});
    return init.result.get();
  }

 private:
  boost::asio::io_context &m_io_ctx;
  boost::asio::io_context::strand m_strand;
  std::unordered_set<std::string> m_seen;
  std::deque<std::reference_wrapper<const std::string>> m_pending;

  template <typename Handler>
  struct JobHandler {
    struct State {
      explicit State(const Handler &, JobManager &manager) : m_manager{manager}, m_timer{manager.m_io_ctx} {}
      JobManager &m_manager;
      boost::asio::steady_timer m_timer;
    };
    template <typename DeducedHandler>
    JobHandler(JobManager &manager, DeducedHandler &&handler) : m_p{std::forward<DeducedHandler>(handler), manager} {}
    boost::beast::handler_ptr<State, Handler> m_p;

    // auto get_allocator() const noexcept { return boost::asio::get_associated_allocator(*m_p); }

    // auto get_executor() const noexcept {
    //   return (boost::asio::get_associated_executor)(m_p.handler(), m_p->m_manager.get_executor());
    // }

    void operator()(boost::system::error_code ec) {
      if (ec) return m_p.invoke(ec, std::string{});
      State &state = *m_p;
      if (!state.m_manager.m_pending.empty()) {
        ec.assign(0, boost::system::system_category());
        std::string str = state.m_manager.m_pending.front();
        state.m_manager.m_pending.pop_front();
        m_p.invoke(ec, str);
        return;
      } else {
        state.m_timer.expires_from_now(std::chrono::milliseconds{5});
        return state.m_timer.async_wait(std::move(*this));
      }
    }
  };
};
}  // namespace site_archive::downloader
