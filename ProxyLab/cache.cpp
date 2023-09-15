#include "cache.hpp"

std::shared_ptr<cache_item> lru_cache::get(const std::string &uri) {
  std::lock_guard<std::mutex> lock{mutex_};
  if (map_.find(uri) == map_.end()) {
    return {nullptr};
  }
  auto item_iter = map_[uri];
  auto item = *item_iter;
  list_.erase(item_iter);
  list_.push_back(item);
  auto end = list_.end();
  map_[uri] = --end;
  return item;
}

void lru_cache::push(std::shared_ptr<cache_item> item) {
  if (item->size() > MAX_OBJECT_SIZE) {
    return;
  }
  std::lock_guard<std::mutex> lock{mutex_};
  if (size_ + item->size() > capacity_) {
    // perform eviction
    while (size_ + item->size() > capacity_) {
      pop();
    }
  }
  list_.push_back(item);
  auto end = list_.end();
  map_[item->uri] = --end;
  size_ += item->size();
}

void lru_cache::pop() {
  auto lru_item = list_.front();
  list_.pop_front();
  map_.erase(lru_item->uri);
  size_ -= lru_item->size();
}