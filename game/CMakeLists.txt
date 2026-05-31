project(Game LANGUAGES CXX)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(GAME_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/map/HexChunk.cpp
    ${SRC_DIR}/map/HexGrid.cpp
    ${SRC_DIR}/map/PlayerEntity.cpp
    ${SRC_DIR}/map/HexPattern.cpp
    ${SRC_DIR}/scripts/world/world.cpp
    ${SRC_DIR}/game.cpp
    ${SRC_DIR}/playerMovement.cpp
    ${SRC_DIR}/camera/cameraController.cpp
    ${SRC_DIR}/stateMachine/gameStateMachine.cpp
    ${SRC_DIR}/stateMachine/explorationState.cpp
    ${SRC_DIR}/stateMachine/combatState.cpp
    ${SRC_DIR}/enemySystem/enemyEntity.cpp
    ${SRC_DIR}/enemySystem/enemyManager.cpp
    ${SRC_DIR}/enemySystem/territoryPatternRegistry.cpp
    ${SRC_DIR}/player/playerPatternComponent.cpp
    ${SRC_DIR}/ui/combatUIPanel.cpp
    ${SRC_DIR}/enemySystem/combatArenaBuilder.cpp
)

set(GAME_HEADERS
    ${INC_DIR}/map/Entity.h
    ${INC_DIR}/map/HexCell.h
    ${INC_DIR}/map/HexChunk.h
    ${INC_DIR}/map/HexCoord.h
    ${INC_DIR}/map/HexGrid.h
    ${INC_DIR}/map/HexPattern.h
    ${INC_DIR}/map/PlayerEntity.h
    ${INC_DIR}/scripts/world/world.h
    ${INC_DIR}/scripts/world/worldHex.h
    ${INC_DIR}/utils/perlin.h
    ${INC_DIR}/game.h
    ${INC_DIR}/playerMovement.h
    ${INC_DIR}/spriteupdater.h
    ${INC_DIR}/textupdater.h
    ${INC_DIR}/camera/cameraController.h
    ${INC_DIR}/stateMachine/iGameState.h
    ${INC_DIR}/stateMachine/gameStateMachine.h
    ${INC_DIR}/stateMachine/explorationState.h
    ${INC_DIR}/stateMachine/combatState.h
    ${INC_DIR}/enemySystem/enemyEntity.h
    ${INC_DIR}/enemySystem/enemyManager.h
    ${INC_DIR}/enemySystem/enemyTypes.h
    ${INC_DIR}/enemySystem/territoryPattern.h
    ${INC_DIR}/enemySystem/territoryPatternRegistry.h
    ${INC_DIR}/player/playerPatternComponent.h
    ${INC_DIR}/ui/combatUIPanel.h
    ${INC_DIR}/enemySystem/combatArenaBuilder.h
)

add_executable(game
    ${GAME_SOURCES}
    ${GAME_HEADERS}
)

target_include_directories(game
    PUBLIC
    ${INC_DIR}
)

source_group(TREE ${SRC_DIR} PREFIX "Source" FILES ${GAME_SOURCES})
source_group(TREE ${INC_DIR} PREFIX "Include" FILES ${GAME_HEADERS})

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
