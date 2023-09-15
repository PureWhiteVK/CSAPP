#include "urlVisitor.h"

class url_visitor : public parser::urlVisitor {
private:
  using Base = parser::urlVisitor;
  using Parser = parser::urlParser;

public:
  std::any visitScheme(Parser::SchemeContext *context);
  std::any visitHost(Parser::HostContext *context);
  std::any visitPort(Parser::PortContext *context);
  std::any visitQuery(Parser::QueryContext *context);
  std::any visitFrag(Parser::FragContext *context);
  std::any visitUrl(Parser::UrlContext *context);
  std::any visitUri(Parser::UriContext *context);
  std::any
  visitDomainNameOrIPv4Host(Parser::DomainNameOrIPv4HostContext *context);
  std::any visitIPv6Host(Parser::IPv6HostContext *context);
  std::any visitV6host(Parser::V6hostContext *context);
  std::any visitPath(Parser::PathContext *context);
  std::any visitUser(Parser::UserContext *context);
  std::any visitLogin(Parser::LoginContext *context);
  std::any visitPassword(Parser::PasswordContext *context);
  std::any visitSearch(Parser::SearchContext *context);
  std::any visitSearchparameter(Parser::SearchparameterContext *context);
  std::any visitString(Parser::StringContext *context);

  std::string request_path();

public:
  std::string scheme_{};
  std::string host_{};
  std::string port_{};
  std::string path_{};
  std::string query_{};
  std::string frag_{};
};