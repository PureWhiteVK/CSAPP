#include "csapp.h"
#include <stdio.h>

#include <chrono>
#include <filesystem>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

#define TO_SOCKADDR(x) reinterpret_cast<struct sockaddr *>(&(x))

namespace fs = std::filesystem;
namespace ch = std::chrono;

int main(int argc, char *argv[]) {

  std::string server_domain = "localhost";
  ch::steady_clock::time_point s = ch::steady_clock::now();
  struct hostent *hostent = Gethostbyname(server_domain.c_str());
  ch::steady_clock::time_point t = ch::steady_clock::now();
  spdlog::info("Gethostbyname costs {:.3f} ms",ch::duration_cast<ch::microseconds>(t-s).count() / 1000.0);
  char **h = hostent->h_addr_list;
  std::vector<uint8_t> addr_data;
  addr_data.resize(hostent->h_length);
  // 只是用 IPv4 地址，因此需要确保数值为 host ?
  // 然后输出的时候确保结果为1000？
  while (*h != nullptr) {
    std::memcpy(addr_data.data(), *h, addr_data.size());
    spdlog::info("addr data: {}", addr_data);
    h++;
  }
  exit(0);

  if (argc != 2) {
    fmt::print("Usage: {} <proxy_port>\n", fs::path(argv[0]).c_str());
    exit(EXIT_FAILURE);
  }

  signal(SIGPIPE, SIG_IGN);

  uint16_t port = std::stoi(argv[1]);
  spdlog::info("running http proxy, listen on port: {}", port);

  int listen_fd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  // actually we should listen for connections
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  Bind(listen_fd, TO_SOCKADDR(addr), sizeof(addr));
  Listen(listen_fd, 1);
  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof(remote_addr);
  rio_t rio;
  std::array<char, 8192> buffer;
  int sockfd = Accept(listen_fd, TO_SOCKADDR(remote_addr), &addrlen);
  // 初始化 buffer 
  Rio_readinitb(&rio, sockfd);
  while (true) {
    // read from remote
    ssize_t recv_bytes = Rio_readlineb(&rio, buffer.data(), buffer.size());
    if (recv_bytes == 0) {
      break;
    }
    spdlog::info("recv {} bytes", recv_bytes);
    spdlog::info("recv line {}", buffer.data());
    // 解析网络请求，然后不断读取数据？
  }
  close(sockfd);
  close(listen_fd);
  return 0;
}
