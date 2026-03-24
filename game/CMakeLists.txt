project(Game LANGUAGES CXX)

add_executable(game
                src/main.cpp
)
target_include_directories(game
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game
        PUBLIC
        DZeMIKK
)
if (MSVC)
    target_compile_options(game PRIVATE /utf-8)
else()
    target_compile_options(game PRIVATE -finput-charset=UTF-8)
endif()
