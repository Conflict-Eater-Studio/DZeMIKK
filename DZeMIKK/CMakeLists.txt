cmake_minimum_required(VERSION 3.20)
project(DZeMIKK VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

# Fetch GLFW from source to compile with the current toolchain (GCC/MinGW)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

file(GLOB_RECURSE ENGINE_SOURCE CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
add_library(DZeMIKK STATIC
        ${ENGINE_SOURCE}
)

target_include_directories(DZeMIKK
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

# No longer using the precompiled MSVC library
# add_library(glfw3 STATIC IMPORTED)
# set_target_properties(glfw3 PROPERTIES
#         IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/extern/GLFW/lib-vc2022/glfw3.lib"
# )

add_library(glad STATIC extern/glad/glad.c)
target_include_directories(glad
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

#add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/extern/assimp")

if(MSVC)
    target_compile_options(DZeMIKK PRIVATE /utf-8)
    target_compile_options(glad PRIVATE /utf-8)
endif()

target_link_libraries(DZeMIKK
        PUBLIC
            glad
            opengl32
            glfw
            glm::glm
 #           assimp
)
