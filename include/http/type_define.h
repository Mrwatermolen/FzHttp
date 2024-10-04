#ifndef __FZ_HTTP_TYPE_DEFINE_H__
#define __FZ_HTTP_TYPE_DEFINE_H__

#include <string_view>

namespace fz::http {

constexpr inline std::string_view CRLF = "\r\n";
constexpr inline std::string_view COLON = ": ";
constexpr inline std::string_view SPACE = " ";

}  // namespace fz::http

#endif  // __FZ_HTTP_TYPE_DEFINE_H__
