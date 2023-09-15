#include "url_visitor.hpp"

std::any url_visitor::visitScheme(Parser::SchemeContext *context) {
  scheme_ = context->getText();
  if (scheme_ == "http") {
    port_ = "80";
  } else if (scheme_ == "https") {
    port_ = "443";
  }
  visitChildren(context);
  return {};
}

std::any url_visitor::visitHost(Parser::HostContext *context) {
  host_ = context->getText();
  return {};
}

std::any url_visitor::visitPort(Parser::PortContext *context) {
  port_ = context->getText();
  return {};
}

std::any url_visitor::visitQuery(Parser::QueryContext *context) {
  query_ = context->getText();
  return {};
}

std::any url_visitor::visitFrag(Parser::FragContext *context) {
  frag_ = context->getText();
  return {};
}

std::any url_visitor::visitUrl(Parser::UrlContext *context) {
  visitChildren(context);
  return {};
}
std::any url_visitor::visitUri(Parser::UriContext *context) {
  visitChildren(context);
  return {};
}
std::any url_visitor::visitDomainNameOrIPv4Host(
    Parser::DomainNameOrIPv4HostContext *context) {
  return {};
}
std::any url_visitor::visitIPv6Host(Parser::IPv6HostContext *context) {
  return {};
}
std::any url_visitor::visitV6host(Parser::V6hostContext *context) { return {}; }
std::any url_visitor::visitPath(Parser::PathContext *context) {
  path_ = context->getText();
  return {};
}
std::any url_visitor::visitUser(Parser::UserContext *context) { return {}; }
std::any url_visitor::visitLogin(Parser::LoginContext *context) { return {}; }
std::any url_visitor::visitPassword(Parser::PasswordContext *context) {
  return {};
}
std::any url_visitor::visitSearch(Parser::SearchContext *context) { return {}; }
std::any
url_visitor::visitSearchparameter(Parser::SearchparameterContext *context) {
  return {};
}
std::any url_visitor::visitString(Parser::StringContext *context) { return {}; }

std::string url_visitor::request_path() {
  std::stringstream s;
  s << '/' << path_;
  if (!query_.empty()) {
    s << '?' << query_;
  }
  if (!frag_.empty()) {
    s << '#' << frag_;
  }
  return s.str();
}