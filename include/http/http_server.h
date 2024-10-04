#ifndef __FZ_HTTP_HTTP_SERVER_H__
#define __FZ_HTTP_HTTP_SERVER_H__

#include <memory>
#include <unordered_map>

#include "http/http_request.h"
#include "http/http_response.h"
#include "http/http_session.h"
#include "net/session.h"
#include "net/tcp_server.h"

namespace fz::http {

// TODO(franzero): is necessary to inherit from TcpServer?
class HttpServer : private fz::net::TcpServer {
 public:
  HttpServer(std::size_t thread_num, std::string_view ip, uint16_t port)
      : fz::net::TcpServer(thread_num, ip, port) {
    this->setReadCallback([this](const auto& session, auto& buffer) {
      this->readCallback(session, buffer);
    });
    this->setNewSessionCallback<HttpSession>();
  }

  auto start() -> void { TcpServer::start(); }

  auto stop() -> void { TcpServer::stop(); }

  auto registerHandler(
      std::string_view path,
      std::function<HttpResponse(const HttpRequest& request)> handler) -> void;

  auto response(const std::shared_ptr<HttpSession>& http_session,
                const HttpResponse& response) -> void;

 private:
  auto readCallback(const std::shared_ptr<net::Session>& session,
                    net::Buffer& buffer) -> void;

 private:
  std::unordered_map<std::string,
                     std::function<HttpResponse(const HttpRequest& request)>>
      _handlers;
};

}  // namespace fz::http

#endif  // __FZ_HTTP_HTTP_SERVER_H__
