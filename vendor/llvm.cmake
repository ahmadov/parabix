if (APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/llvm@12/lib/cmake")
endif (APPLE)

find_package(LLVM 13 REQUIRED CONFIG)
message(STATUS "LLVM_INCLUDE_DIRS = ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_INSTALL_PREFIX = ${LLVM_INSTALL_PREFIX}")

add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(LLVM_LIBS core support mcjit x86codegen OrcJIT)
