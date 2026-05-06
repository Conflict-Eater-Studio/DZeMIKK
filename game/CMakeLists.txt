project(Game LANGUAGES CXX)

add_executable(game
                src/main.cpp
                src/map/HexChunk.cpp
                src/map/grid.cpp
                src/scripts/world/world.cpp
                src/main.cpp
                src/game.cpp
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

add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FMOD_LIBS_PATH}/fmod.dll"
        $<TARGET_FILE_DIR:game>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FMOD_LIBS_PATH}/fmodL.dll"
        $<TARGET_FILE_DIR:game>
)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "$<TARGET_FILE_DIR:game>/res"
)
