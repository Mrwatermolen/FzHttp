#include <cassert>
#include <iostream>
#include <string_view>

#include "http/http_request_parse.h"

int main() {
  using namespace std::string_view_literals;

  auto http_request_str =
      "GET /index?name=hello&password=123456 HTTP/1.1\r\n"
      "Host: www.baidu.com\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
      "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 "
      "Safari/537.36\r\n"
      "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,"
      "image/webp,*/*;q=0.8\r\n"
      "Accept-Encoding: gzip, deflate, sdch\r\n"
      "Accept-Language: zh-CN,zh;q=0.8\r\n"
      "Connection: keep-alive\r\n"
      "Upgrade-Insecure-Requests: 1\r\n"
      "Cache-Control: max-age=0\r\n"
      "Content-Length: 26\r\n"
      "\r\n"
      "abcdefghigklmnopqrstuvwxyz"sv;

  auto assert_func = [](const auto& http_request) {
    assert(http_request.method() == fz::http::HttpRequest::Method::GET);
    assert(http_request.path() == "/index");
    assert(http_request.querys().size() == 2);
    assert(http_request.querys().at("name") == "hello");
    assert(http_request.querys().at("password") == "123456");
    assert(http_request.version() == fz::http::HttpRequest::Version::HTTP_1_1);
    assert(http_request.headers().size() == 9);
    assert(http_request.headers().at("Host") == "www.baidu.com");
    assert(
        http_request.headers().at("User-Agent") ==
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
        "like Gecko) Chrome/58.0.3029.110 Safari/537.36");
    assert(http_request.headers().at("Accept") ==
           "text/html,application/xhtml+xml,"
           "application/xml;q=0.9,image/webp,"
           "*/*;q=0.8");
    assert(http_request.headers().at("Accept-Encoding") ==
           "gzip, deflate, sdch");
    assert(http_request.headers().at("Accept-Language") == "zh-CN,zh;q=0.8");
    assert(http_request.headers().at("Connection") == "keep-alive");
    assert(http_request.headers().at("Upgrade-Insecure-Requests") == "1");
    assert(http_request.headers().at("Cache-Control") == "max-age=0");
    assert(http_request.body() == "abcdefghigklmnopqrstuvwxyz");
  };

  auto http_request = fz::http::HttpRequest();
  auto http_request_parse = fz::http::HttpRequestParse();

  http_request.clear();
  http_request.parse(http_request_str);

  std::cout << http_request.toString() << "\n";

  assert_func(http_request);

  std::cout << "Test passed\n";

  http_request_parse.reset();
  auto buffer = fz::net::Buffer();
  buffer.append(http_request_str.data(), http_request_str.size());
  http_request_parse.run(buffer);
  assert(http_request_parse.status() == fz::http::HttpRequestParse::Status::OK);
  std::cout << http_request_parse.request().toString() << '\n';
  assert_func(http_request_parse.request());
  std::cout << "Test passed\n";
}
