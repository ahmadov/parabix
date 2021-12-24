cmake_minimum_required(VERSION 3.7)

project(vectorized_regex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address")

find_package(Threads REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)

include("${CMAKE_SOURCE_DIR}/cmake/clang-tidy.cmake")

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

set(INCLUDE_H
    "${CMAKE_SOURCE_DIR}/include/stream/basis_bit_stream.h"
    "${CMAKE_SOURCE_DIR}/include/stream/cc_bit_stream.h"
    "${CMAKE_SOURCE_DIR}/include/stream/marker_bit_stream.h"
    "${CMAKE_SOURCE_DIR}/include/parser/re_parser.h"
)

set(SRC_CC
    "${CMAKE_SOURCE_DIR}/src/stream/basis_bit_stream.cc"
    "${CMAKE_SOURCE_DIR}/src/stream/cc_bit_stream.cc"
    "${CMAKE_SOURCE_DIR}/src/stream/marker_bit_stream.cc"
    "${CMAKE_SOURCE_DIR}/src/parser/re_parser.cc"
    "${CMAKE_SOURCE_DIR}/tools/vgrep.cc"
)

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

add_library(regex_vectorization STATIC ${SRC_CC})
target_link_libraries(regex_vectorization Threads::Threads)

add_executable(vgrep tools/vgrep.cc)
target_link_libraries(vgrep regex_vectorization)

# ---------------------------------------------------------------------------
# Linting
# ---------------------------------------------------------------------------

add_clang_tidy_target(src_linting "${SRC_CC}")
add_clang_tidy_target(include_linting "${INCLUDE_H}")

add_custom_target(lint)
list(APPEND lint_targets include_linting src_linting)
add_dependencies(lint ${lint_targets})
