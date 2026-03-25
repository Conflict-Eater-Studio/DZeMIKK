cmake_minimum_required(VERSION 3.20)
project(DZeMIKK VERSION 1.0 LANGUAGES C CXX)

set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_DIR    "${ENGINE_DIR}/src")
set(INC_DIR    "${ENGINE_DIR}/include")
set(EXTERN_DIR "${ENGINE_DIR}/extern")
set(DZEMIKK_DEV_TOOLS OFF)
set(DZeMIKK_VERSION_MAJOR 1)
set(DZeMIKK_VERSION_MAJOR 0)
set(DZeMIKK_VERSION_REVISION 0)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(DZEMIKK_DEV_TOOLS ON)
endif()

add_library(DZeMIKK STATIC)
set(ENGINE_SOURCES
    "${SRC_DIR}/core/engine.cpp"
    "${SRC_DIR}/core/window.cpp"
    "${SRC_DIR}/renderer/renderer.cpp"
    "${SRC_DIR}/renderer/mesh.cpp"
    "${SRC_DIR}/renderer/shader.cpp"
    "${SRC_DIR}/renderer/material.cpp"
    "${SRC_DIR}/ecs/components/transform.cpp"
    "${SRC_DIR}/ecs/components/meshRenderer.cpp"
    "${SRC_DIR}/ecs/components/spriteRenderer.cpp"
)

set(ENGINE_HEADERS
    "${INC_DIR}/core/engine.h"
    "${INC_DIR}/core/window.h"
    "${INC_DIR}/core/iEngineModule.h"
    "${INC_DIR}/renderer/renderer.h"
    "${INC_DIR}/renderer/mesh.h"
    "${INC_DIR}/renderer/shader.h"
    "${INC_DIR}/renderer/material.h"
    "${INC_DIR}/ecs/component.h"
    "${INC_DIR}/ecs/components/transform.h"
    "${INC_DIR}/ecs/components/monoBehaviour.h"
    "${INC_DIR}/ecs/components/meshRenderer.h"
    "${INC_DIR}/ecs/components/spriteRenderer.h"
)

target_sources(DZeMIKK PRIVATE ${ENGINE_SOURCES} ${ENGINE_HEADERS})

source_group(TREE "${SRC_DIR}" PREFIX "Source" FILES ${ENGINE_SOURCES})
source_group(TREE "${INC_DIR}" PREFIX "Include" FILES ${ENGINE_HEADERS})

target_include_directories(DZeMIKK
        PUBLIC
        "${INC_DIR}"
        PRIVATE
        "${EXTERN_DIR}"
)

add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
        IMPORTED_LOCATION "${EXTERN_DIR}/GLFW/lib-vc2022/glfw3.lib"
)

add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION "${EXTERN_DIR}/freetype/freetype.lib"
)

add_library(glad STATIC
        "${EXTERN_DIR}/glad/glad.c"
)
target_include_directories(glad
        PRIVATE
        "${EXTERN_DIR}/glad"
        "${EXTERN_DIR}"
)

add_subdirectory("${EXTERN_DIR}/assimp")
add_subdirectory("${EXTERN_DIR}/glm/glm")
if (DZEMIKK_DEV_TOOLS)
    add_library(imgui STATIC
            "${EXTERN_DIR}/imgui/imgui.cpp"
            "${EXTERN_DIR}/imgui/imgui_draw.cpp"
            "${EXTERN_DIR}/imgui/imgui_tables.cpp"
            "${EXTERN_DIR}/imgui/imgui_widgets.cpp"
            "${EXTERN_DIR}/imgui/backends/imgui_impl_glfw.cpp"
            "${EXTERN_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    )

    target_include_directories(imgui
            PUBLIC
            "${EXTERN_DIR}/imgui"
            "${EXTERN_DIR}/imgui/backends"
            "${EXTERN_DIR}"
    )

    target_link_libraries(imgui PUBLIC glfw3 opengl32)
    target_compile_options(imgui PRIVATE /utf-8)
endif ()

target_compile_options(DZeMIKK PRIVATE /utf-8)
target_compile_options(glad PRIVATE /utf-8)


target_link_libraries(DZeMIKK
        PUBLIC
        glad
        opengl32
        glfw3
        assimp
        glm::glm
        freetype
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:imgui>
)
target_compile_definitions(DZeMIKK
        PUBLIC
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DZEMIKK_DEV_TOOLS=1>
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:DZEMIKK_DEV_TOOLS=0>
        DZeMIKK_VERSION_MAJOR=1
        DZeMIKK_VERSION_MINOR=0
        DZeMIKK_VERSION_REVISION=0


)
