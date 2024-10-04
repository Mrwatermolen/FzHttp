#ifndef __FZ_HTTP_HTTP_REQUEST_PARSE_H__
#define __FZ_HTTP_HTTP_REQUEST_PARSE_H__

#include <cstddef>
#include <cstdint>

#include "http/http_request.h"
#include "net/common/buffer.h"

namespace fz::http {

class HttpRequestParse {
 public:
  constexpr static auto MAX_REQUEST_LINE_SIZE = 4096;

  enum class Status : std::uint8_t { INVALID, RequestLine, Headers, Body, OK };

  auto status() const { return _status; }

  auto& request() const { return _request; }

  auto& request() { return _request; }

  auto reset() {
    _status = Status::RequestLine;
    _request.clear();
    _data.clear();
    _body_size = std::numeric_limits<std::size_t>::max();
  }

  auto markAsInvalid() {
    _status = Status::INVALID;
    _request.clear();
    _data.clear();
    _body_size = std::numeric_limits<std::size_t>::max();
  }

  auto run(net::Buffer& buffer) {
    if (buffer.empty()) {
      return;
    }

    if (status() == Status::INVALID || status() == Status::OK) {
      buffer.retrieve(buffer.readableBytes());  // clear buffer
      return;
    }

    _data += buffer.retrieveAllAsString();
    while (parse()) {
    }
  }

 private:
  auto parse() -> bool {
    switch (status()) {
      case Status::RequestLine: {
        const auto bytes = _request.parseRequestLine(_data);
        if (bytes == std::string::npos) {
          if (MAX_REQUEST_LINE_SIZE < _data.size()) {
            markAsInvalid();
          }

          return false;
        }

        _data.erase(0, bytes);
        _status = Status::Headers;
        return !_data.empty();
      }
      case Status::Headers: {
        const auto bytes = _request.parseHeaders(_data);
        if (bytes == std::string::npos) {
          return false;
        }

        if (bytes == 0) {
          _status = Status::Body;
          auto it = _request.headers().find("Content-Length");
          if (it != _request.headers().end()) {
            _body_size = std::stoul(it->second);
          } else {
            _body_size = 0;
          }

          return !_data.empty();
        }

        _data.erase(0, bytes);
        return !_data.empty();
      }
      case Status::Body: {
        if (_body_size == 0) {
          _status = Status::OK;
          return false;
        }

        if (!_data.empty() && _data.substr(0, 2) == CRLF) {
          _data.erase(0, 2);
        }

        if (_data.size() < _body_size) {
          return false;
        }

        _request.setBody(_data.substr(0, _body_size));
        _data.clear();
        _status = Status::OK;
        return false;
      }
      default:
        break;
    }

    return false;
  }

 private:
  Status _status{Status::RequestLine};
  HttpRequest _request;
  std::string _data;
  std::size_t _body_size{std::numeric_limits<std::size_t>::max()};
};

}  // namespace fz::http

#endif  // __FZ_HTTP_HTTP_REQUEST_PARSE_H__
