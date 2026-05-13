project(Editor LANGUAGES CXX)

add_executable(editor
    src/main.cpp
    src/editor.cpp
    src/hierarchyPanel.cpp
    src/inspectorPanel.cpp
    src/inspectors/transformInspector.cpp
    src/ui/propertyDrawer.cpp
)

target_sources(editor PRIVATE
    include/editor.h
    include/hierarchyPanel.h
    include/inspectorPanel.h
    include/inspectors/transformInspector.h
    include/ui/propertyDrawer.h
)

target_include_directories(editor
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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
