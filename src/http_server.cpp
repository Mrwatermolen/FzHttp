#include "http/http_server.h"

#include "net/common/log.h"

namespace fz::http {

auto HttpServer::registerHandler(
    std::string_view path,
    std::function<HttpResponse(const HttpRequest& request)> handler) -> void {
  _handlers.emplace(path, handler);
}

auto HttpServer::response(const std::shared_ptr<HttpSession>& http_session,
                          const HttpResponse& response) -> void {
  auto buffer = net::Buffer();
  buffer.append(response.toString());
  http_session->send(buffer);
}

auto HttpServer::readCallback(const std::shared_ptr<net::Session>& session,
                              net::Buffer& buffer) -> void {
  auto http_session = std::dynamic_pointer_cast<HttpSession>(session);
  if (!http_session) {
    LOG_ERROR("dynamic_pointer_cast failed", "");
  }

  http_session->parseRequest(buffer);
  if (http_session->httpRequestParse().status() !=
      HttpRequestParse::Status::OK) {
    return;
  }

  if (http_session->httpRequestParse().status() ==
      HttpRequestParse::Status::INVALID) {
    this->response(http_session, HttpResponse::makeBadRequest());
    http_session->httpRequestParse().reset();
    return;
  }

  auto& request = http_session->httpRequestParse().request();
  auto path = request.path();
  if (path.empty()) {
    this->response(http_session, HttpResponse::makeNotFound());
    http_session->httpRequestParse().reset();
    return;
  }

  if (path.size() != 1 && path.back() == '/') {
    path.pop_back();
  }

  auto handler_it = _handlers.find(path);
  if (handler_it == _handlers.end()) {
    this->response(http_session, HttpResponse::makeNotFound());
    http_session->httpRequestParse().reset();
    return;
  }

  auto response = handler_it->second(request);
  this->response(http_session, response);
  http_session->httpRequestParse().reset();
}

}  // namespace fz::http
