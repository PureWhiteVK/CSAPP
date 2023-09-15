#include "response_parser.hpp"
#include "string_utils.hpp"

#include <unordered_set>

static constexpr uint8_t SP = ' ';
static constexpr uint8_t CR = '\r';
static constexpr uint8_t LF = '\n';

bool response_parser::is_tchar(uint8_t ch) {
  static std::unordered_set<uint8_t> tchar_set{'!',  '#', '$', '%', '&',
                                               '\'', '*', '+', '-', '.',
                                               '^',  '_', '`', '|', '~'};
  return std::isalnum(ch) || tchar_set.find(ch) != tchar_set.end();
}

std::tuple<response_parser::parse_result, size_t>
response_parser::parse(std::string_view data) {
  parse_result result;
  size_t pos;
  for (pos = 0; pos < data.size(); pos++) {
    result = consume(data[pos]);
    m_header_length++;
    if (result != CONTINUE) {
      break;
    }
  }
  return std::make_tuple(result, pos);
}

response_parser::parse_result response_parser::consume(uint8_t ch) {
  parse_result res{CONTINUE};
  switch (m_state) {
  case http_version_h: {
    if (ch == 'H') {
      m_state = http_version_t_0;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_t_0: {
    if (ch == 'T') {
      m_state = http_version_t_1;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_t_1: {
    if (ch == 'T') {
      m_state = http_version_p;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_p: {
    if (ch == 'P') {
      m_state = http_version_slash;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_slash: {
    if (ch == '/') {
      m_state = http_version_major;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_major: {
    if (std::isdigit(ch)) {
      m_state = http_version_dot;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_dot: {
    if (ch == '.') {
      m_state = http_version_minor;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_minor: {
    if (std::isdigit(ch)) {
      m_state = status_sp_before;
    } else {
      res = FAIL;
    }
    break;
  }
  case status_sp_before: {
    if (ch == SP) {
      m_state = status_code1;
    } else {
      res = FAIL;
    }
    break;
  }
  case status_code1: {
    if (std::isdigit(ch)) {
      m_state = status_code2;
    } else {
      res = FAIL;
    }
    break;
  }
  case status_code2: {
    if (std::isdigit(ch)) {
      m_state = status_code3;
    } else {
      res = FAIL;
    }
    break;
  }
  case status_code3: {
    if (std::isdigit(ch)) {
      m_state = status_sp_after;
    } else {
      res = FAIL;
    }
    break;
  }
  case status_sp_after: {
    if (ch == SP) {
      m_state = reason_phrase;
    } else {
      res = FAIL;
    }
    break;
  }
  case reason_phrase: {
    if (ch == CR) {
      m_state = status_line_lf;
    } else if (ch <= 31 || ch == 127) {
      res = FAIL;
    } else {
      // no-op
    }
    break;
  }
  case status_line_lf: {
    if (ch == LF) {
      m_state = field_line;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_line: {
    if (ch == CR) {
      m_state = body_lf;
    } else if (is_tchar(ch)) {
      m_buffer << ch;
      m_state = field_name;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_name: {
    if (ch == ':') {
      m_last_field_name = m_buffer.str();
      if (string_utils::lower(string_utils::trim(m_last_field_name)) ==
          "content-length") {
        m_parsing_content_length = true;
      }
      m_buffer.str("");
      m_state = field_value;
    } else if (is_tchar(ch)) {
      m_buffer << ch;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_value: {
    if (ch == CR) {
      // we have to strip buffer_value here
      if (m_parsing_content_length) {
        m_content_length =
            string_utils::parse_ull(string_utils::trim(m_buffer.str()));
        m_parsing_content_length = false;
      }
      m_last_field_name.clear();
      m_buffer.str("");
      m_state = field_line_lf;
    } else if (is_field_vchar(ch)) {
      m_buffer << ch;
    } else if (std::isblank(ch)) {
      m_buffer << ch;
      m_state = field_value_ows;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_value_ows: {
    if (ch == CR) {
      // we have to strip buffer value here
      if (m_parsing_content_length) {
        m_content_length =
            string_utils::parse_ull(string_utils::trim(m_buffer.str()));
        m_parsing_content_length = false;
      }
      m_last_field_name.clear();
      m_buffer.str("");
      m_state = field_line_lf;
    } else if (is_field_vchar(ch)) {
      m_buffer << ch;
      m_state = field_value;
    } else if (std::isblank(ch)) {
      // skip
    } else {
      res = FAIL;
    }
    break;
  }
  case field_line_lf: {
    if (ch == LF) {
      m_state = field_line;
    } else {
      res = FAIL;
    }
    break;
  }
  case body_lf: {
    res = (ch == LF) ? PASS : FAIL;
    break;
  }
  }
  return res;
}
