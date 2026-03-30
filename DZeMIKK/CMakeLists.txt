cmake_minimum_required(VERSION 3.20)
project(DZeMIKK VERSION 1.0 LANGUAGES C CXX)

set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_DIR    "${ENGINE_DIR}/src")
set(INC_DIR    "${ENGINE_DIR}/include")
set(EXTERN_DIR "${ENGINE_DIR}/extern")
set(DZEMIKK_DEV_TOOLS OFF)
set(DZeMIKK_VERSION_MAJOR 1)
set(DZeMIKK_VERSION_MINOR 0)
set(DZeMIKK_VERSION_REVISION 0)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(DZEMIKK_DEV_TOOLS ON)
endif()

add_library(DZeMIKK STATIC)
set(ENGINE_SOURCES
        "${SRC_DIR}/core/engine.cpp"
        "${SRC_DIR}/core/window.cpp"
        "${SRC_DIR}/core/time.cpp"
        "${SRC_DIR}/renderer/renderer.cpp"
    	"${SRC_DIR}/renderer/mesh.cpp"
    	"${SRC_DIR}/renderer/shader.cpp"
    	"${SRC_DIR}/renderer/material.cpp"
        "${SRC_DIR}/ecs/components/monobehaviour.cpp"
        "${SRC_DIR}/ecs/components/transform.cpp"
        "${SRC_DIR}/ecs/components/meshRenderer.cpp"
    	"${SRC_DIR}/ecs/components/spriteRenderer.cpp"
        "${SRC_DIR}/ecs/gameobject.cpp"
        "${SRC_DIR}/ecs/scene.cpp"
        "${SRC_DIR}/ecs/scenemanager.cpp"
)

set(ENGINE_HEADERS
        "${INC_DIR}/core/engine.h"
        "${INC_DIR}/core/window.h"
        "${INC_DIR}/renderer/renderer.h"
        "${INC_DIR}/ecs/components/monobehaviour.h"
        "${INC_DIR}/ecs/components/transform.h"
        "${INC_DIR}/ecs/component.h"
        "${INC_DIR}/ecs/gameobject.h"
        "${INC_DIR}/ecs/scene.h"
        "${INC_DIR}/ecs/scenemanager.h"
)

target_sources(DZeMIKK
        PRIVATE
        ${ENGINE_SOURCES}
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${INC_DIR}"
        FILES ${ENGINE_HEADERS}
)

source_group(TREE "${SRC_DIR}" PREFIX "Source" FILES ${ENGINE_SOURCES})
source_group(TREE "${INC_DIR}" PREFIX "Include" FILES ${ENGINE_HEADERS})

target_include_directories(DZeMIKK
        PUBLIC
        "${INC_DIR}"
        "${EXTERN_DIR}/glm/include"
        PRIVATE
        "${EXTERN_DIR}"
        "${EXTERN_DIR}/assimp/include"
        "${EXTERN_DIR}/freetype/include"
        "${EXTERN_DIR}/fmod/include"
)

add_library(glad STATIC
        "${EXTERN_DIR}/glad/glad.c"
)
target_include_directories(glad
        PUBLIC
        "${EXTERN_DIR}/glad/include"
        "${EXTERN_DIR}/glad/include/glad"
        "${EXTERN_DIR}/KHR/include"
)

include(FetchContent)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.4
)

FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	1.0.3
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
)
FetchContent_MakeAvailable(freetype)


add_library(fmod STATIC IMPORTED)
set_target_properties(fmod PROPERTIES
        IMPORTED_LOCATION "${FMOD_LIBS_PATH}/fmod_vc.lib"
)

if (DZEMIKK_DEV_TOOLS)
    add_library(fmod_debug STATIC IMPORTED)
    set_target_properties(fmod_debug PROPERTIES
            IMPORTED_LOCATION "${FMOD_LIBS_PATH}/fmodL_vc.lib"
    )
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

    target_link_libraries(imgui PUBLIC glfw opengl32)
    if (MSVC)
        target_compile_options(imgui PRIVATE /utf-8)
    else()
        target_compile_options(imgui PRIVATE -finput-charset=UTF-8)
    endif()

endif ()

if (MSVC)
    target_compile_options(DZeMIKK PRIVATE /utf-8)
    target_compile_options(glad PRIVATE /utf-8)
else()
    target_compile_options(DZeMIKK PRIVATE -finput-charset=UTF-8)
    target_compile_options(glad PRIVATE -finput-charset=UTF-8)
endif()


target_link_libraries(DZeMIKK
        PUBLIC
        glad
        opengl32
        glfw
        assimp
        glm::glm
        freetype
        $<$<CONFIG:Release>:fmod>
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:fmod_debug>
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
