#include "thread_pool.hpp"

void thread_pool::work() {
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock{mutex_};
      cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
      if (stop_ && tasks_.empty()) {
        return;
      }
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}

thread_pool::thread_pool(size_t threads) {
  workers_.reserve(threads);
  for (auto i = 0; i < threads; i++) {
    workers_.emplace_back([this] { work(); });
  }
}

thread_pool::~thread_pool() { wait(); }

void thread_pool::wait() {
  stop_ = true;
  cv_.notify_all();
  for (auto &worker : workers_) {
    worker.join();
  }
}