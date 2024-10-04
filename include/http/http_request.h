#ifndef __FZ_HTTP_HTTP_REQUEST_H__
#define __FZ_HTTP_HTTP_REQUEST_H__

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "http/type_define.h"

namespace fz::http {

class HttpRequest {
 public:
  HttpRequest() { clear(); }

 public:
  enum Method : std::uint8_t { INVALID, GET, POST, PUT, DELETE, HEAD };

  enum Version : std::uint8_t { UNKNOWN, HTTP_1_0, HTTP_1_1 };

  constexpr static auto methodToString(Method method) -> std::string_view {
    switch (method) {
      case GET:
        return "GET";
      case POST:
        return "POST";
      case PUT:
        return "PUT";
      case DELETE:
        return "DELETE";
      case HEAD:
        return "HEAD";
      default:
        return "INVALID";
    }
  }

  constexpr static auto versionToString(Version version) -> std::string_view {
    switch (version) {
      case HTTP_1_0:
        return "HTTP/1.0";
      case HTTP_1_1:
        return "HTTP/1.1";
      default:
        return "UNKNOWN";
    }
  }

  constexpr static auto methodFromString(std::string_view method) -> Method {
    if (method == "GET") {
      return GET;
    }
    if (method == "POST") {
      return POST;
    }
    if (method == "PUT") {
      return PUT;
    }
    if (method == "DELETE") {
      return DELETE;
    }
    if (method == "HEAD") {
      return HEAD;
    }
    return INVALID;
  }

  constexpr static auto versionFromString(std::string_view version) -> Version {
    if (version == "HTTP/1.0") {
      return HTTP_1_0;
    }
    if (version == "HTTP/1.1") {
      return HTTP_1_1;
    }
    return UNKNOWN;
  }

 public:
  auto method() const { return _method; }

  auto setMethod(Method method) { _method = method; }

  auto path() const { return _path; }

  auto setPath(std::string_view path) { _path = path; }

  auto& querys() const { return _querys; }

  auto addQuery(std::string_view key, std::string_view value) {
    _querys.emplace(key, value);
  }

  auto version() const { return _version; }

  auto setVersion(Version version) { _version = version; }

  auto& headers() const { return _headers; }

  auto addHeader(std::string_view key, std::string_view value) {
    _headers[std::string{key}] = std::string{value};
  }

  auto& body() const { return _body; }

  auto setBody(std::string_view body) { _body = body; }

  auto keepAlive() const -> bool {
    auto connection = _headers.find("Connection");
    if (connection == _headers.end()) {
      return false;
    }
    return connection->second == "keep-alive";
  }

  auto clear() -> void {
    _method = INVALID;
    _path.clear();
    _querys.clear();
    _version = UNKNOWN;
    _headers.clear();
    _body.clear();
  }

  auto toString() const -> std::string {
    std::stringstream ss;
    ss << methodToString(_method) << SPACE << _path;
    if (!_querys.empty()) {
      ss << "?";
      for (const auto& [key, value] : _querys) {
        ss << key << "=" << value << "&";
      }
      ss.seekp(-1, std::ios_base::end);
    }

    ss << SPACE << versionToString(_version) << CRLF;

    for (const auto& [key, value] : _headers) {
      ss << key << COLON << value << CRLF;
    }
    ss << CRLF;

    ss << _body;

    return ss.str();
  }

  auto parseRequestLine(std::string_view data) -> std::string::size_type {
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF

    const auto pos = data.find(CRLF);
    if (pos == std::string::npos) {
      return pos;
    }

    auto request_line = data.substr(0, pos);

    const auto method_pos = request_line.find(SPACE);
    if (method_pos == std::string::npos) {
      return method_pos;
    }
    auto method = methodFromString(request_line.substr(0, method_pos));
    if (method == INVALID) {
      return method_pos;
    }
    setMethod(method);

    const auto url_pos = request_line.find(SPACE, method_pos + 1);
    if (url_pos == std::string::npos) {
      return url_pos;
    }

    auto path = request_line.substr(method_pos + 1, url_pos - method_pos - 1);
    const auto query_pos = path.find('?');
    if (query_pos != std::string::npos) {
      setPath(path.substr(0, query_pos));
      auto querys = path.substr(query_pos + 1);
      while (true) {
        auto equal_pos = querys.find('=');
        if (equal_pos == std::string::npos) {
          break;
        }

        auto key = querys.substr(0, equal_pos);
        querys.remove_prefix(equal_pos + 1);
        auto and_pos = querys.find('&');
        if (and_pos == std::string::npos) {
          addQuery(key, querys);
          break;
        }

        auto value = querys.substr(0, and_pos);
        querys.remove_prefix(and_pos + 1);
        addQuery(key, value);
      }
    } else {
      setPath(path);
    }

    auto version = versionFromString(request_line.substr(url_pos + 1));
    if (version == UNKNOWN) {
      return url_pos;
    }
    setVersion(version);

    return pos + CRLF.size();  // skip CRLF
  }

  auto parseOneHeader(std::string_view data) -> std::string::size_type {
    const auto pos = data.find(CRLF);
    if (pos == std::string::npos) {
      return pos;
    }
    auto header = data.substr(0, pos);
    if (header.empty()) {
      return pos;
    }

    const auto colon_pos = header.find(COLON);
    if (colon_pos == std::string::npos) {
      return colon_pos;
    }
    auto key = header.substr(0, colon_pos);
    auto value = header.substr(colon_pos + 2);
    addHeader(key, value);

    return pos + CRLF.size();
  }

  auto parseHeaders(std::string_view data) -> std::string::size_type {
    if (data.empty()) {
      return std::string::npos;
    }

    std::string::size_type pos = 0;
    while (true) {
      if (data.empty()) {
        break;
      }

      auto bytes = parseOneHeader(data);
      if (bytes == std::string::npos) {
        return bytes;
      }
      if (bytes == 0) {
        break;
      }

      pos += bytes;
      data.remove_prefix(bytes);
    }

    return pos;
  }

  auto parse(std::string_view data) -> bool {
    auto bytes = parseRequestLine(data);
    if (bytes == std::string::npos) {
      return false;
    }

    data.remove_prefix(bytes);
    bytes = parseHeaders(data);
    if (bytes == std::string::npos) {
      return false;
    }

    data.remove_prefix(bytes + CRLF.size());
    setBody(data);

    return true;
  }

 private:
  Method _method;
  std::string _path;
  std::unordered_map<std::string, std::string> _querys;
  Version _version;
  std::unordered_map<std::string, std::string> _headers;
  std::string _body;
};

}  // namespace fz::http

#endif  // __FZ_HTTP_HTTP_REQUEST_H__
