cmake_minimum_required(VERSION 3.20)
project(Engine VERSION 1.0 LANGUAGES C CXX)

add_library(Engine STATIC
        src/core/engine.cpp
)

target_include_directories(Engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/lib-mingw-w64/libglfw3.a"
)

add_library(glad STATIC src/glad.c)
target_include_directories(glad
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/third_party/
)

target_link_libraries(Engine
        PUBLIC
            glad
            opengl32
            glfw3
)

add_library(Engine::engine ALIAS Engine)