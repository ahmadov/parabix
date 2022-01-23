cmake_minimum_required(VERSION 3.7)

project(vectorized_regex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address -mavx2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O2 -mavx2")

find_package(Threads REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)

include("${CMAKE_SOURCE_DIR}/cmake/clang-tidy.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/llvm.cmake")

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/perfevent
)

include_directories(SYSTEM
    ${LLVM_INCLUDE_DIRS}
)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

set(INCLUDE_H
    "${CMAKE_SOURCE_DIR}/include/stream/bit_stream.h"
    "${CMAKE_SOURCE_DIR}/include/parser/re_parser.h"
    "${CMAKE_SOURCE_DIR}/include/parser/cc.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/cc_compiler.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/expression_compiler_cpp.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/expression_compiler_llvm.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/expression_builder.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/operation_compiler.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/ast.h"
    "${CMAKE_SOURCE_DIR}/include/codegen/jit.h"
    "${CMAKE_SOURCE_DIR}/include/operations/marker.h"
    "${CMAKE_SOURCE_DIR}/include/operations/simd.h"
)

set(SRC_CC
    "${CMAKE_SOURCE_DIR}/src/stream/bit_stream.cc"
    "${CMAKE_SOURCE_DIR}/src/parser/re_parser.cc"
    "${CMAKE_SOURCE_DIR}/src/codegen/cc_compiler.cc"
    "${CMAKE_SOURCE_DIR}/src/codegen/expression_compiler_cpp.cc"
    "${CMAKE_SOURCE_DIR}/src/codegen/expression_compiler_llvm.cc"
    "${CMAKE_SOURCE_DIR}/src/codegen/expression_builder.cc"
    "${CMAKE_SOURCE_DIR}/src/codegen/operation_compiler.cc"
    "${CMAKE_SOURCE_DIR}/src/codegen/jit.cc"
    "${CMAKE_SOURCE_DIR}/tools/vgrep.cc"
)

set(TEST_CC
    "${CMAKE_SOURCE_DIR}/test/bit_stream.cc"
)

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

add_library(regex_vectorization STATIC ${SRC_CC})
target_link_libraries(regex_vectorization Threads::Threads ${LLVM_LIBS} ${LLVM_LDFLAGS})

add_executable(vgrep tools/vgrep.cc)
target_link_libraries(vgrep regex_vectorization)

add_executable(vgrep_block tools/vgrep_block.cc)
target_link_libraries(vgrep_block regex_vectorization)

add_executable(vgrep_llvm tools/vgrep_llvm.cc)
target_link_libraries(vgrep_llvm regex_vectorization)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

add_executable(tester test/tester.cc ${TEST_CC})
target_link_libraries(tester regex_vectorization gtest Threads::Threads)
enable_testing()
add_test(regex_vectorization tester)

# ---------------------------------------------------------------------------
# Linting
# ---------------------------------------------------------------------------

add_clang_tidy_target(src_linting "${SRC_CC}")
add_clang_tidy_target(include_linting "${INCLUDE_H}")

add_custom_target(lint)
list(APPEND lint_targets include_linting src_linting)
add_dependencies(lint ${lint_targets})
