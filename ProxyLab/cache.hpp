#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache_item {
  std::vector<char> data;
  std::string uri{};

  size_t size() const { return data.size(); }
  char *buffer() { return data.data(); }
};

class lru_cache {
public:
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }

  std::shared_ptr<cache_item> get(const std::string &uri);

  void push(std::shared_ptr<cache_item> item);

private:
  void pop();

private:
  size_t capacity_{MAX_CACHE_SIZE};
  size_t size_{};
  /*
  std::list is a container that supports constant time insertion and removal of
  elements from anywhere in the container. Fast random access is not supported.
  It is usually implemented as a doubly-linked list. Compared to
  std::forward_list this container provides bidirectional iteration capability
  while being less space efficient. Adding, removing and moving the elements
  within the list or across several lists does not invalidate the iterators or
  references. An iterator is invalidated only when the corresponding element is
  deleted.
  */
  std::list<std::shared_ptr<cache_item>> list_{};
  std::unordered_map<std::string,
                     std::list<std::shared_ptr<cache_item>>::iterator>
      map_{};
  std::mutex mutex_;
};