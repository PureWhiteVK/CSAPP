#pragma once

#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>

/// Parser for incoming requests.
class response_parser {
public:
  /// Construct ready to parse the request method.
  response_parser() = default;

  /// Reset to initial parser state.
  void clear() {
    m_state = parser_state::http_version_h;
    m_buffer.str("");
  }

  /// Result of parse.
  enum parse_result { PASS, FAIL, CONTINUE };

  /// Parse some data. The enum return value is good when a complete request has
  /// been parsed, bad if the data is invalid, indeterminate when more data is
  /// required. The InputIterator return value indicates how much of the input
  /// has been consumed.
  std::tuple<parse_result, size_t> parse(std::string_view data);

  size_t total_size() const { return m_content_length + m_header_length; }

private:
  /// Handle the next character of input.
  parse_result consume(uint8_t input);

  static bool is_obs_text(uint8_t ch) { return ch >= 0x80; }

  static bool is_vchar(uint8_t ch) { return ch > 0x20 && ch <= 0x7e; }

  static bool is_tchar(uint8_t ch);

  static bool is_field_vchar(uint8_t ch) {
    return is_vchar(ch) || is_obs_text(ch);
  }

  /// The current state of the parser.
  enum parser_state {
    http_version_h,
    http_version_t_0,
    http_version_t_1,
    http_version_p,
    http_version_slash,
    http_version_major,
    http_version_dot,
    http_version_minor,
    status_sp_before,
    status_code1,
    status_code2,
    status_code3,
    status_sp_after,
    reason_phrase,
    status_line_lf,
    field_line,
    field_name,
    field_value,
    field_value_ows,
    field_line_lf,
    body_lf,
  } m_state{http_version_h};

  std::ostringstream m_buffer;
  std::string m_last_field_name;
  bool m_parsing_content_length{false};
  size_t m_header_length{0};
  size_t m_content_length{0};
};