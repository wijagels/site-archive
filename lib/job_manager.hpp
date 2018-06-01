#pragma once
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/handler_ptr.hpp>

#include <deque>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace site_archive::downloader {

class JobManager {
  using result_type = std::string;

 public:
  JobManager(boost::asio::io_context &io_ctx);
  ~JobManager() = default;
  JobManager(const JobManager &) = delete;
  JobManager(JobManager &&) = delete;
  JobManager &operator=(const JobManager &) = delete;
  JobManager &operator=(JobManager &&) = delete;

  void add_work(std::vector<std::string> &&work);

  template <typename Handler>
  BOOST_ASIO_INITFN_RESULT_TYPE(Handler, void(boost::system::error_code, result_type))
  get_work(Handler &&ct) {
    boost::asio::async_completion<Handler, void(boost::system::error_code, result_type)> init{ct};
    boost::asio::post(JobFetcher<BOOST_ASIO_HANDLER_TYPE(Handler, void(boost::system::error_code,
                                                                       result_type))>{*this, init.completion_handler});
    return init.result.get();
  }

 private:
  boost::asio::io_context &m_io_ctx;
  boost::asio::io_context::strand m_strand;
  std::unordered_set<std::string> m_seen;
  std::deque<std::string> m_pending;
  template <typename Handler>
  struct JobFetcher;
};

template <typename Handler>
struct JobManager::JobFetcher {
  struct State {
    explicit State(const Handler &, JobManager &manager) : m_manager{manager}, m_timer{manager.m_io_ctx} {}
    JobManager &m_manager;
    boost::asio::deadline_timer m_timer;
  };
  template <typename DeducedHandler>
  JobFetcher(JobManager &manager, DeducedHandler &&handler) : m_p{std::forward<DeducedHandler>(handler), manager} {}

  boost::beast::handler_ptr<State, Handler> m_p;

  using executor_type = decltype(m_p->m_manager.m_strand);
  executor_type get_executor() const noexcept { return m_p->m_manager.m_strand; }

  using allocator_type = boost::asio::associated_allocator_t<Handler>;
  allocator_type get_allocator() const noexcept { return boost::asio::get_associated_allocator(m_p.handler()); }

  void operator()(boost::system::error_code ec = {}) {
    if (ec) return m_p.invoke(ec, result_type{});
    State &state = *m_p;
    if (!state.m_manager.m_pending.empty()) {
      auto str = std::move(state.m_manager.m_pending.front());
      state.m_manager.m_pending.pop_front();
      return m_p.invoke(ec, std::move(str));
    } else {
      state.m_timer.expires_from_now(boost::posix_time::milliseconds{100});
      return state.m_timer.async_wait(std::move(*this));
    }
  }
};

}  // namespace site_archive::downloader
