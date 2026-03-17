cmake_minimum_required(VERSION 3.20)
project(DZeMIKK VERSION 1.0 LANGUAGES C CXX)


set(ENGINE_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/engine.cpp"
)
add_library(DZeMIKK STATIC
        ${ENGINE_SOURCE}
)

target_include_directories(DZeMIKK
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)


add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/extern/GLFW/lib-mingw-w64/libglfw3.a"
)

add_library(glad STATIC extern/glad/glad.c)
target_include_directories(glad
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad"
)

target_link_libraries(DZeMIKK
        PUBLIC
            glad
            opengl32
            glfw3
)


