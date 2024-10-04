#include <asio.hpp>
#include <fstream>
#include <string_view>

#include "asio/io_context.hpp"
#include "http/http_server.h"

int main() {
  asio::io_context io_context;

  fz::http::HttpServer server{2, "0.0.0.0", 80};
  server.registerHandler("/hello", [](const auto& request) {
    if (request.method() != fz::http::HttpRequest::Method::GET) {
      auto res = fz::http::HttpResponse::makeMethodNotAllowed();
      return res;
    }

    auto response = fz::http::HttpResponse::makeOk();
    response.addHeader("Server", "fz");
    response.addHeader("Content-Length", "11");
    response.addHeader("Content-Type", "text/plain");
    response.setBody("hello world");

    return response;
  });

  server.registerHandler("/post", [](const auto& request) {
    if (request.method() != fz::http::HttpRequest::Method::POST) {
      auto res = fz::http::HttpResponse::makeMethodNotAllowed();
      return res;
    }

    auto response = fz::http::HttpResponse::makeOk();
    std::string_view body = "<html><body><h1>POST</h1></body></html>";
    response.addHeader("Server", "fz");
    response.addHeader("Content-Length", std::to_string(body.size()));
    response.addHeader("Content-Type", "text/html");
    response.setBody(body);

    return response;
  });

  server.registerHandler("/", [](const auto& request) {
    constexpr auto index_file = "./tmp/index.html";
    auto response = fz::http::HttpResponse::makeOk();
    response.addHeader("Server", "fz");
    response.addHeader("Content-Type", "text/html; charset=utf-8");
    response.addHeader("Connection", "keep-alive");

    auto ifs = std::ifstream(index_file, std::ios::in | std::ios::binary);
    if (!ifs) {
      auto body = "<html><body><h1>Web Server Home</h1></body></html>";
      response.addHeader("Content-Length", std::to_string(strlen(body)));
      response.setBody(body);
      return response;
    }

    ifs.seekg(0, std::ios::end);
    auto length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    response.addHeader("Content-Length", std::to_string(length));
    auto body = std::string(length, '\0');
    ifs.read(body.data(), length);
    response.setBody(body);

    return response;
  });

  server.start();

  asio::signal_set signals(io_context, SIGINT, SIGTERM);
  signals.async_wait([&](const auto&, const auto&) { server.stop(); });

  io_context.run();

  return 0;
}
