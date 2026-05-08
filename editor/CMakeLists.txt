project(Editor LANGUAGES CXX)

add_executable(editor
    src/main.cpp
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

