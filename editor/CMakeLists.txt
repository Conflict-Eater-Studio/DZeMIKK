project(Editor LANGUAGES CXX)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(EDITOR_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/editor.cpp
    ${SRC_DIR}/hierarchyPanel.cpp
    ${SRC_DIR}/inspectorPanel.cpp
    ${SRC_DIR}/assetManagerPanel.cpp
    ${SRC_DIR}/inspectors/transformInspector.cpp
    ${SRC_DIR}/inspectors/inspectorRegistry.cpp
    ${SRC_DIR}/inspectors/meshRendererInspector.cpp
    ${SRC_DIR}/ui/propertyDrawer.cpp
)

set(EDITOR_HEADERS
    ${INC_DIR}/editor.h
    ${INC_DIR}/hierarchyPanel.h
    ${INC_DIR}/inspectorPanel.h
    ${INC_DIR}/assetManagerPanel.h
    ${INC_DIR}/inspectors/transformInspector.h
    ${INC_DIR}/inspectors/inspectorRegistry.h
    ${INC_DIR}/inspectors/meshRendererInspector.h
    ${INC_DIR}/ui/propertyDrawer.h
)

add_executable(editor
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
)

target_include_directories(editor
    PUBLIC
    ${INC_DIR}
)

source_group(TREE ${SRC_DIR} PREFIX "Source" FILES ${EDITOR_SOURCES})
source_group(TREE ${INC_DIR} PREFIX "Include" FILES ${EDITOR_HEADERS})

target_link_libraries(editor
    PUBLIC
    DZeMIKK
)

if (MSVC)
  target_compile_options(editor PRIVATE /utf-8)
else()
  target_compile_options(editor PRIVATE -finput-charset=UTF-8)
endif()

add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_LIBS_PATH}/fmod.dll"
    $<TARGET_FILE_DIR:editor>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_LIBS_PATH}/fmodL.dll"
    $<TARGET_FILE_DIR:editor>
)

add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "$<TARGET_FILE_DIR:editor>/res"
)
