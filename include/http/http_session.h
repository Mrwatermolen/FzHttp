#ifndef __FZ_HTTP_HTTP_SESSION_H__
#define __FZ_HTTP_HTTP_SESSION_H__

#include "http/http_request_parse.h"
#include "net/common/buffer.h"
#include "net/session.h"

namespace fz::http {

class HttpSession : public fz::net::Session {
 public:
  explicit HttpSession(std::shared_ptr<fz::net::Loop> loop)
      : fz::net::Session{std::move(loop)} {}

  auto parseRequest(net::Buffer& buffer) -> void {
    _http_request_parse.run(buffer);
  }

  auto& httpRequestParse() const { return _http_request_parse; }

  auto& httpRequestParse() { return _http_request_parse; }

 private:
  HttpRequestParse _http_request_parse;
};

}  // namespace fz::http

#endif  // __FZ_HTTP_HTTP_SESSION_H__
