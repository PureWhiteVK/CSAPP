#include "csapp.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

#include "cache.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "response_parser.hpp"
#include "string_utils.hpp"
#include "thread_pool.hpp"
#include "url_visitor.hpp"

#include "antlr4-runtime.h"
#include "urlLexer.h"
#include "urlParser.h"

#define TO_SOCKADDR(x) reinterpret_cast<struct sockaddr *>(&(x))

void handle_connection(int fd);

void proxy(int fd, std::shared_ptr<request> req);

std::tuple<std::string, std::string, std::string>
parse_uri(std::string_view uri);

void clienterror(int fd, const char *cause, const char *errnum,
                 const char *shortmsg, const char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[NI_MAXHOST], port[NI_MAXSERV];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  thread_pool pool{4};

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, sizeof(hostname), port,
                sizeof(port), 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    pool.emplace(
        [](int fd) {
          handle_connection(fd); // line:netp:tiny:doit
          Close(fd);             // line:netp:tiny:close
        },
        connfd);
  }
}

void handle_connection(int fd) {
  rio_t rio;
  char recv_buf[MAXLINE];
  request_parser parser;
  std::shared_ptr<request> req = std::make_shared<request>();
  Rio_readinitb(&rio, fd);
  do {
    ssize_t bytes_recv = Rio_readlineb(&rio, recv_buf, sizeof(recv_buf));
    std::string_view data(recv_buf, bytes_recv);
    spdlog::info("recv {} bytes\n{}", bytes_recv, string_utils::escaped(data));
    auto [parse_result, bytes_parsed] = parser.parse(req, data);
    spdlog::info("parse_result: {}",
                 (parse_result == request_parser::PASS
                      ? "PASS"
                      : (parse_result == request_parser::CONTINUE ? "CONTINUE"
                                                                  : "FAIL")));
    if (parse_result == request_parser::FAIL) {
      clienterror(fd, req->method.c_str(), "400", "Bad Request",
                  "Failed to parse http request header");
      return;
    } else if (parse_result == request_parser::PASS) {
      proxy(fd, req);
      return;
    }
  } while (1);
}

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, const char *cause, const char *errnum,
                 const char *shortmsg, const char *longmsg) {
  char buf[MAXLINE];

  /* Print the HTTP response headers */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));

  /* Print the HTTP response body */
  sprintf(buf, "<html><title>Tiny Error</title>");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<body bgcolor="
               "ffffff"
               ">\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
  Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */

std::tuple<std::string, std::string, std::string>
parse_uri(std::string_view uri) {
  antlr4::ANTLRInputStream inputs{uri};
  parser::urlLexer lexer{&inputs};
  antlr4::CommonTokenStream tokens{&lexer};
  parser::urlParser parser{&tokens};
  url_visitor visitor;
  visitor.visit(parser.url());
  return {visitor.host_, visitor.port_, visitor.request_path()};
}

void proxy(int fd, std::shared_ptr<request> req) {
  static lru_cache cache;
  if (req->method != "GET") {
    clienterror(fd, req->method.c_str(), "501", "Not Implemented",
                "Proxy does not implement this method");
    return;
  }
  std::stringstream s;
  ssize_t bytes_recv;
  rio_t rio;
  char recv_buf[MAX_OBJECT_SIZE];
  // parse finished, forward request and perform other
  auto [host, port, uri] = parse_uri(req->request_target);

  // 首先检查是否有 cache
  auto resp = cache.get(uri);
  if (resp) {
    Rio_writen(fd, resp->buffer(), resp->size());
    return;
  }

  int clientfd = Open_clientfd(const_cast<char *>(host.c_str()),
                               const_cast<char *>(port.c_str()));
  // 构造请求头拿到返回结果
  s << fmt::format("{} {} HTTP/1.0\r\n", req->method, uri);
  // write header
  req->headers["Host"] = host;
  req->headers["User-Agent"] = "Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) "
                               "Gecko/20120305 Firefox/10.0.3";
  req->headers["Connection"] = "close";
  req->headers["Proxy-Connection"] = "close";
  for (auto &[key, value] : req->headers) {
    s << fmt::format("{}: {}\r\n", key, value);
  }
  s << "\r\n";
  spdlog::info("send {} bytes\n{}", s.str().size(), s.str());
  Rio_writen(clientfd, const_cast<char *>(s.str().c_str()), s.str().size());
  Rio_readinitb(&rio, clientfd);
  size_t total_recv = 0;
  bool need_cache = true;
  bool parsed = false;
  char *buf = recv_buf;
  size_t buf_size = sizeof(recv_buf);
  response_parser parser;
  do {
    bytes_recv = Rio_readlineb(&rio, buf, buf_size);
    total_recv += bytes_recv;
    std::string_view data(buf, bytes_recv);
    spdlog::info("recv {} bytes\n{}", bytes_recv, string_utils::escaped(data));
    if (!parsed) {
      auto [ret, _] = parser.parse(data);
      if (ret == response_parser::PASS) {
        parsed = true;
        need_cache = parser.total_size() <= MAX_OBJECT_SIZE;
      } else if (ret == response_parser::FAIL) {
        spdlog::error("failed to parse response header");
        clienterror(fd, req->method.c_str(), "500", "Internal Server Error",
                    "failed to parse response header");
        Close(clientfd);
        return;
      }
    }
    if (!need_cache) {
      Rio_writen(fd, buf, bytes_recv);
    } else {
      buf += bytes_recv;
    }
  } while (bytes_recv > 0);
  Close(clientfd);
  spdlog::info("actual size: {} bytes, estimated size: {} bytes", total_recv,
               parser.total_size());
  if (need_cache) {
    resp = std::make_shared<cache_item>();
    resp->uri = uri;
    resp->data.resize(total_recv);
    memcpy(resp->data.data(), recv_buf, total_recv);
    cache.push(resp);
    Rio_writen(fd, resp->buffer(), resp->size());
  }
  return;
}