cmake_minimum_required(VERSION 3.20)
project(DZeMIKK VERSION 1.0 LANGUAGES C CXX)


set(ENGINE_SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/engine.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.cpp"
)
add_library(DZeMIKK STATIC
        ${ENGINE_SOURCE}
)

target_include_directories(DZeMIKK
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)


add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/extern/GLFW/lib-vc2022/glfw3.lib"
)
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/extern/freetype/freetype.lib"
)

add_library(glad STATIC
        "extern/glad/glad.c")

target_include_directories(glad
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/extern/assimp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/extern/glm/glm")

add_library(imgui STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/imgui_widgets.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/backends/imgui_impl_glfw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/backends"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)
target_link_libraries(imgui PUBLIC glfw3 opengl32)

target_compile_options(DZeMIKK PRIVATE /utf-8)
target_compile_options(glad PRIVATE /utf-8)
target_compile_options(imgui PRIVATE /utf-8)


target_link_libraries(DZeMIKK
        PUBLIC
            glad
            opengl32
            glfw3
            assimp
            glm::glm
            freetype
            imgui
)


