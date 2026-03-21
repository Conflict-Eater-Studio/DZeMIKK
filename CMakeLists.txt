cmake_minimum_required(VERSION 3.30)
project(TUL_PBL_DZeMIKK VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(DZeMIKK)
add_subdirectory(game)

SET(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        --config-file="${CMAKE_SOURCE_DIR}/.clang-tidy"
)
set(CMAKE_VERBOSE_MAKEFILE ON)
