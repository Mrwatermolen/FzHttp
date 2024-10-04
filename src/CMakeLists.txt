aux_source_directory(. FZ_HTTP_SOURCES)
add_library(fz_http ${FZ_HTTP_SOURCES})

set(FZ_HTTP_PUBLIC_LIBRARIES fz::fz_net)

target_include_directories(fz_http PUBLIC ${FZ_HTTP_PUBLIC_INCLUDE_DIR})
target_compile_options(fz_http PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(fz_http PUBLIC ${FZ_HTTP_PUBLIC_LIBRARIES})
