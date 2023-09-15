#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

class thread_pool {
public:
  explicit thread_pool(size_t threads);

  ~thread_pool();

  template <class F, class... Args>
  std::future<std::invoke_result_t<F, Args...>> emplace(F &&f, Args &&...args);

private:
  void work();
  void wait();

private:
  std::vector<std::thread> workers_{};
  std::queue<std::function<void()>> tasks_{};
  std::mutex mutex_{};
  std::condition_variable cv_{};
  std::atomic<bool> stop_{false};
};

template <class F, class... Args>
std::future<std::invoke_result_t<F, Args...>>
thread_pool::emplace(F &&f, Args &&...args) {
  if (stop_) {
    throw std::runtime_error("emplace task on stopped thread pool is invalid!");
  }
  using return_type = std::invoke_result_t<F, Args...>;
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock{mutex_};
    tasks_.emplace([task]() { task->operator()(); });
  }
  cv_.notify_one();
  return res;
}