#pragma once

#include <string>
#include <unordered_map>

/// A request received from a client.
struct request {
public:
  std::string method{};
  // this may be parsed?
  std::string request_target{};
  int http_version_major{};
  int http_version_minor{};
  std::unordered_map<std::string, std::string> headers{};
};