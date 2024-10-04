#ifndef __FZ_HTTP_HTTP_RESPONSE_H__
#define __FZ_HTTP_HTTP_RESPONSE_H__

#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>

#include "http/type_define.h"

namespace fz::http {

class HttpResponse {
 public:
  enum StatusCode : std::uint16_t {
    UNKNOW = 0,
    OK = 200,
    MOVED_PERMANENTLY = 301,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405
  };

  enum Version : std::uint8_t { UNKNOWN, HTTP_1_0, HTTP_1_1 };

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

  constexpr static auto statusCodeToString(StatusCode status_code)
      -> std::string_view {
    switch (status_code) {
      case OK:
        return "OK";
      case MOVED_PERMANENTLY:
        return "Moved Permanently";
      case BAD_REQUEST:
        return "Bad Request";
      case NOT_FOUND:
        return "Not Found";
      case METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
      default:
        return "Unknow";
    }
  }

  static auto makeOk() -> HttpResponse {
    auto response = HttpResponse{};
    response.setVersion(HTTP_1_1);
    response.setStatusCode(OK);
    response.addHeader("Content-Length", "0");
    return response;
  }

  static auto makeMovedPermanently() -> HttpResponse {
    auto response = HttpResponse{};
    response.setVersion(HTTP_1_1);
    response.setStatusCode(MOVED_PERMANENTLY);
    return response;
  }

  static auto makeBadRequest() -> HttpResponse {
    auto response = HttpResponse{};
    response.setStatusCode(BAD_REQUEST);
    response.setVersion(HTTP_1_1);
    response.addHeader("Content-Length", "0");
    return response;
  }

  static auto makeNotFound() -> HttpResponse {
    auto response = HttpResponse{};
    response.setVersion(HTTP_1_1);
    response.setStatusCode(NOT_FOUND);
    response.addHeader("Content-Length", "0");
    return response;
  }

  static auto makeMethodNotAllowed() -> HttpResponse {
    auto response = HttpResponse{};
    response.setVersion(HTTP_1_1);
    response.setStatusCode(METHOD_NOT_ALLOWED);
    response.addHeader("Content-Length", "0");
    return response;
  }

 public:
  auto version() const -> Version { return _version; }

  auto setVersion(Version version) -> void { _version = version; }

  auto statusCode() const -> StatusCode { return _status_code; }

  auto setStatusCode(StatusCode status_code) -> void {
    _status_code = status_code;
  }

  auto& headers() const { return _headers; }

  auto addHeader(std::string_view key, std::string_view value) -> void {
    _headers[std::string{key}] = std::string{value};
  }

  auto& body() const { return _body; }

  auto setBody(std::string_view body) -> void { _body = body; }

  auto toString() const -> std::string {
    auto ss = std::stringstream{};
    ss << versionToString(_version) << SPACE << statusCode() << SPACE
       << statusCodeToString(_status_code) << CRLF;

    for (const auto& [key, value] : _headers) {
      ss << key << COLON << value << CRLF;
    }
    ss << CRLF;

    ss << _body;

    return ss.str();
  }

 private:
  Version _version{UNKNOWN};
  StatusCode _status_code{UNKNOW};
  std::unordered_map<std::string, std::string> _headers;
  std::string _body;
  std::string _status_message;
};

}  // namespace fz::http

#endif  // __FZ_HTTP_HTTP_RESPONSE_H__
