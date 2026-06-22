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
    ${SRC_DIR}/scripts/world/worldVisualManager.cpp
    ${SRC_DIR}/game.cpp
    ${SRC_DIR}/player/playerMovement.cpp
    ${SRC_DIR}/camera/cameraController.cpp
    ${SRC_DIR}/stateMachine/gameStateMachine.cpp
    ${SRC_DIR}/stateMachine/explorationState.cpp
    ${SRC_DIR}/stateMachine/combatState.cpp
    ${SRC_DIR}/stateMachine/combatResolver.cpp
    ${SRC_DIR}/stateMachine/cinematicState.cpp
    ${SRC_DIR}/enemySystem/enemyEntity.cpp
    ${SRC_DIR}/enemySystem/enemyManager.cpp
    ${SRC_DIR}/enemySystem/territoryPatternRegistry.cpp
    ${SRC_DIR}/enemySystem/enemyActionWeights.cpp
    ${SRC_DIR}/enemySystem/enemyPlanner.cpp
    ${SRC_DIR}/enemySystem/behaviorTree/bTNode.cpp
    ${SRC_DIR}/enemySystem/behaviorTree/healthNodes.cpp
    ${SRC_DIR}/enemySystem/behaviorTree/playerStyleNodes.cpp
    ${SRC_DIR}/player/playerPatternComponent.cpp
    ${SRC_DIR}/enemySystem/enemyPatternComponent.cpp
    ${SRC_DIR}/enemySystem/patternComponent.cpp
    ${SRC_DIR}/player/playerPatternStatsComponent.cpp
    ${SRC_DIR}/ui/combatUIPanel.cpp
    ${SRC_DIR}/ui/logoComponent.cpp
    ${SRC_DIR}/ui/uIPulseEffect.cpp
    ${SRC_DIR}/enemySystem/combatArenaBuilder.cpp
    ${SRC_DIR}/player/inventory.cpp
    ${SRC_DIR}/healthSystem.cpp
    ${SRC_DIR}/totem/totemSpawnConfig.cpp
    ${SRC_DIR}/totem/totemEntity.cpp
    ${SRC_DIR}/totem/totemManager.cpp
    ${SRC_DIR}/totem/totemDialogEntity.cpp
    ${SRC_DIR}/item/itemManager.cpp
    ${SRC_DIR}/dialog/dialogManager.cpp
    ${SRC_DIR}/dialog/dialog.cpp
    ${SRC_DIR}/visuals/depthOfFieldEffect.cpp
)

set(GAME_HEADERS
    ${INC_DIR}/map/Entity.h
    ${INC_DIR}/map/HexCell.h
    ${INC_DIR}/map/HexChunk.h
    ${INC_DIR}/map/HexCoord.h
    ${INC_DIR}/map/HexGrid.h
    ${INC_DIR}/map/HexPattern.h
    ${INC_DIR}/map/ItemEntity.h
    ${INC_DIR}/map/ItemEntityHealth.h
    ${INC_DIR}/map/ItemEntityBonusHex.h
    ${INC_DIR}/map/ItemEntityRevealHex.h
    ${INC_DIR}/map/ItemEntityRevealPatter.h
    ${INC_DIR}/map/PlayerEntity.h
    ${INC_DIR}/scripts/world/world.h
    ${INC_DIR}/scripts/world/worldHex.h
    ${INC_DIR}/scripts/world/worldVisualManager.h
    ${INC_DIR}/utils/perlin.h
    ${INC_DIR}/game.h
    ${INC_DIR}/player/playerMovement.h
    ${INC_DIR}/spriteupdater.h
    ${INC_DIR}/textupdater.h
    ${INC_DIR}/camera/cameraController.h
    ${INC_DIR}/stateMachine/iGameState.h
    ${INC_DIR}/stateMachine/gameStateMachine.h
    ${INC_DIR}/stateMachine/explorationState.h
    ${INC_DIR}/stateMachine/combatState.h
    ${INC_DIR}/stateMachine/combatTypes.h
    ${INC_DIR}/stateMachine/combatResolver.h
    ${INC_DIR}/stateMachine/cinematicState.h
    ${INC_DIR}/enemySystem/enemyEntity.h
    ${INC_DIR}/enemySystem/enemyManager.h
    ${INC_DIR}/enemySystem/enemyTypes.h
    ${INC_DIR}/enemySystem/territoryPattern.h
    ${INC_DIR}/enemySystem/territoryPatternRegistry.h
    ${INC_DIR}/enemySystem/enemyActionWeights.h
    ${INC_DIR}/enemySystem/enemyPlanner.h
    ${INC_DIR}/enemySystem/behaviorTree/bTNode.h
    ${INC_DIR}/enemySystem/behaviorTree/healthNodes.h
    ${INC_DIR}/enemySystem/behaviorTree/playerStyleNodes.h
    ${INC_DIR}/player/playerPatternComponent.h
    ${INC_DIR}/enemySystem/patternComponent.h
    ${INC_DIR}/enemySystem/enemyPatternComponent.h
    ${INC_DIR}/player/playerPatternStatsComponent.h
    ${INC_DIR}/ui/combatUIPanel.h
    ${INC_DIR}/ui/logoComponent.h
    ${INC_DIR}/ui/uIPulseEffect.h
    ${INC_DIR}/enemySystem/combatArenaBuilder.h
    ${INC_DIR}/player/inventory.h
    ${INC_DIR}/healthSystem.h
    ${INC_DIR}/totem/totemSpawnConfig.h
    ${INC_DIR}/totem/totemEntity.h
    ${INC_DIR}/totem/totemManager.h
    ${INC_DIR}/totem/totemDialogEntity.h
    ${INC_DIR}/item/itemManager.h
    ${INC_DIR}/dialog/dialogManager.h
    ${INC_DIR}/dialog/dialog.h
    ${INC_DIR}/dialog/dialogEntity.h
    ${INC_DIR}/visuals/depthOfFieldEffect.h
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "$<TARGET_FILE_DIR:game>/res"
)
