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
        "${SRC_DIR}/core/profiler.cpp"
        "${SRC_DIR}/input/input.cpp"
        "${SRC_DIR}/collisions/collisions.cpp"
        "${SRC_DIR}/renderer/renderer.cpp"
    	"${SRC_DIR}/renderer/mesh.cpp"
    	"${SRC_DIR}/renderer/shader.cpp"
    	"${SRC_DIR}/renderer/material.cpp"
    	"${SRC_DIR}/renderer/frustum.cpp"
    	"${SRC_DIR}/renderer/skybox.cpp"
    	"${SRC_DIR}/renderer/font.cpp"
    	"${SRC_DIR}/renderer/texture.cpp"
    	"${SRC_DIR}/renderer/model.cpp"
        "${SRC_DIR}/ecs/components/monobehaviour.cpp"
        "${SRC_DIR}/ecs/components/transform.cpp"
        "${SRC_DIR}/ecs/components/meshRenderer.cpp"
        "${SRC_DIR}/ecs/components/collider.cpp"
        "${SRC_DIR}/ecs/components/spriteRenderer.cpp"
        "${SRC_DIR}/ecs/components/camera.cpp"
        "${SRC_DIR}/ecs/components/textRenderer.cpp"
        "${SRC_DIR}/ecs/gameobject.cpp"
        "${SRC_DIR}/ecs/scene.cpp"
        "${SRC_DIR}/scene/octree.cpp"
        "${SRC_DIR}/ecs/scenemanager.cpp"
        "${SRC_DIR}/animation/animationmodule.cpp"
        "${SRC_DIR}/ecs/components/animator.cpp"
        "${SRC_DIR}/animation/animationstatemachine.cpp"
        "${SRC_DIR}/ecs/component.cpp"
        "${SRC_DIR}/animation/animationclip.cpp"
        "${SRC_DIR}/animation/animationstate.cpp"
        "${SRC_DIR}/animation/animationtrack.cpp"
        "${SRC_DIR}/ecs/scenemanager.cpp"
        "${SRC_DIR}/ecs/serialize/componentSerializerRegistry.cpp"
        "${SRC_DIR}/ecs/serialize/gameobjectSerializer.cpp"
        "${SRC_DIR}/ecs/serialize/sceneSerializer.cpp"
        "${SRC_DIR}/ecs/serialize/prefabSerializer.cpp"
        "${SRC_DIR}/assetManager/assetManager.cpp"
        "${SRC_DIR}/assetManager/iAssetHandler.cpp"
        "${SRC_DIR}/assetManager/fontHandler.cpp"
        "${SRC_DIR}/assetManager/meshHandler.cpp"
        "${SRC_DIR}/assetManager/shaderHandler.cpp"
        "${SRC_DIR}/assetManager/skyboxHandler.cpp"
        "${SRC_DIR}/assetManager/soundHandler.cpp"
        "${SRC_DIR}/assetManager/textureHandler.cpp"
        "${SRC_DIR}/assetManager/modelHandler.cpp"
        "${SRC_DIR}/assetManager/primitiveFactory.cpp"
        "${SRC_DIR}/assetManager/assetDatabase.cpp"
        "${SRC_DIR}/assetManager/assetLoaderRegistry.cpp"
        "${SRC_DIR}/assetManager/primitiveMeshLibrary.cpp"
        "${SRC_DIR}/assetManager/resourceIndex.cpp"
        "${SRC_DIR}/audio/sound.cpp"
        "${SRC_DIR}/ecs/components/ui/rectTransform.cpp"
        "${SRC_DIR}/ecs/components/ui/uiButton.cpp"
        "${SRC_DIR}/ecs/components/ui/uiSlider.cpp"
        "${SRC_DIR}/ecs/components/ui/uiCheckbox.cpp"
)

set(ENGINE_HEADERS
        "${INC_DIR}/core/engine.h"
        "${INC_DIR}/core/window.h"
        "${INC_DIR}/core/profiler.h"
        "${INC_DIR}/core/iEngineModule.h"
        "${INC_DIR}/input/input.h"
        "${INC_DIR}/collisions/collisions.h"
        "${INC_DIR}/events/event.h"
        "${INC_DIR}/events/key_event.h"
        "${INC_DIR}/events/mouse_event.h"
        "${INC_DIR}/events/application_event.h"
        "${INC_DIR}/renderer/renderer.h"
        "${INC_DIR}/renderer/mesh.h"
        "${INC_DIR}/renderer/shader.h"
        "${INC_DIR}/renderer/material.h"
        "${INC_DIR}/renderer/frustum.h"
        "${INC_DIR}/renderer/skybox.h"
        "${INC_DIR}/renderer/font.h"
        "${INC_DIR}/renderer/texture.h"
        "${INC_DIR}/renderer/model.h"
        "${INC_DIR}/ecs/components/monobehaviour.h"
        "${INC_DIR}/ecs/components/transform.h"
	    "${INC_DIR}/ecs/components/meshRenderer.h"
	    "${INC_DIR}/ecs/components/collider.h"
    	"${INC_DIR}/ecs/components/spriteRenderer.h"
    	"${INC_DIR}/ecs/components/textRenderer.h"
        "${INC_DIR}/ecs/components/camera.h"
        "${INC_DIR}/ecs/component.h"
        "${INC_DIR}/ecs/gameobject.h"
        "${INC_DIR}/ecs/scene.h"
        "${INC_DIR}/scene/octree.h"
        "${INC_DIR}/ecs/scenemanager.h"
        "${INC_DIR}/ecs/componentRegistry.h"
        "${INC_DIR}/ecs/scenemanager.h"
        "${INC_DIR}/ecs/components/animator.h"
        "${INC_DIR}/animation/animationmodule.h"
        "${INC_DIR}/animation/animationclip.h"
        "${INC_DIR}/animation/AnimationStateMachine.h"
        "${INC_DIR}/animation/animationstate.h"
        "${INC_DIR}/animation/transition.h"
        "${INC_DIR}/animation/animationtrack.h"
        "${INC_DIR}/animation/IAnimationTrack.h"
        "${INC_DIR}/animation/floattrack.h"
        "${INC_DIR}/animation/vectortrack.h"
        "${INC_DIR}/animation/quaterniontrack.h"
        "${INC_DIR}/animation/property.h"
        "${INC_DIR}/animation/propertykey.h"
        "${INC_DIR}/ecs/serialize/componentSerializerRegistry.h"
        "${INC_DIR}/ecs/serialize/gameobjectSerializer.h"
        "${INC_DIR}/ecs/serialize/sceneSerializer.h"
        "${INC_DIR}/ecs/serialize/prefabSerializer.h"
        "${INC_DIR}/ecs/serialize/serializedRef.h"
        "${INC_DIR}/ecs/serialize/monoBehaviourSerializer.h"
        "${INC_DIR}/ecs/serialize/transformSerializer.h"
        "${INC_DIR}/ecs/serialize/uiButtonSerializer.h"
        "${INC_DIR}/ecs/serialize/uuid.h"
        "${INC_DIR}/assetManager/assetManager.h"
        "${INC_DIR}/assetManager/assetError.h"
        "${INC_DIR}/assetManager/fontHandler.h"
        "${INC_DIR}/assetManager/meshHandler.h"
        "${INC_DIR}/assetManager/shaderHandler.h"
        "${INC_DIR}/assetManager/skyboxHandler.h"
        "${INC_DIR}/assetManager/soundHandler.h"
        "${INC_DIR}/assetManager/textureHandler.h"
        "${INC_DIR}/assetManager/modelHandler.h"
        "${INC_DIR}/assetManager/iAssetHandler.h"
        "${INC_DIR}/assetManager/primitiveFactory.h"
        "${INC_DIR}/assetManager/assetHandle.h"
        "${INC_DIR}/assetManager/assetDatabase.h"
        "${INC_DIR}/assetManager/assetLoaderRegistry.h"
        "${INC_DIR}/assetManager/primitiveMeshLibrary.h"
        "${INC_DIR}/assetManager/resourceIndex.h"
        "${INC_DIR}/audio/sound.h"
        "${INC_DIR}/ecs/components/ui/imageRenderer.h"
        "${INC_DIR}/ecs/components/ui/uiTextRenderer.h"
        "${INC_DIR}/ecs/components/ui/canvas.h"
        "${INC_DIR}/ecs/components/ui/rectTransform.h"
        "${INC_DIR}/ecs/components/ui/iUIInteractable.h"
        "${INC_DIR}/ecs/components/ui/uiButton.h"
        "${INC_DIR}/ecs/components/ui/uiButtonActionRegistry.h"
        "${INC_DIR}/ecs/components/ui/uiSlider.h"
        "${INC_DIR}/ecs/components/ui/uiSliderActionRegistry.h"
        "${INC_DIR}/ecs/components/ui/colors.h"
        "${INC_DIR}/ecs/components/ui/uiCheckbox.h"
        "${INC_DIR}/ecs/components/ui/uiCheckboxActionRegistry.h"
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
        "${EXTERN_DIR}/boost/include"
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
