#ifndef DZEMIKK_GAME_H
#define DZEMIKK_GAME_H
#include "core/engine.h"
#include "enemySystem/EnemyTooltip.h"
#include "glad/glad.h"
#include "map/PlayerEntity.h"
#include "scripts/world/world.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

namespace dzemikk {
class Material;
class GameObject;
class Camera;
class MeshRenderer;
} // namespace dzemikk
namespace game {
class PlayerMovement;
class CameraController;
class GameStateMachine;

} // namespace game
class Game {
  public:
    Game(const Game&) = default;
    Game(Game&&) = delete;
    Game& operator=(const Game&) = delete;
    Game& operator=(Game&&) = delete;
    explicit Game(dzemikk::Engine* engine);
    ~Game() = default;

    void start();

    game::CameraController* getCameraController();
    void enableCombatUI(bool enable);
    game::GameStateMachine* getStateMachine() {
        return _stateMachine;
    }

    dzemikk::AssetHandle<dzemikk::Scene> getCurrentScene();

    game::HexGrid* getHexGrid();

    dzemikk::Engine* getEngine() {
        return _engine;
    }
    void setExplorationState();
    void setCinematicState();

    void setupEnemiesTooltip();
    void startGame();
    void restartGame();
    [[nodiscard]] bool isPendingRestart() const;
    void markPendingRestart();
    void resetExplorationInputState();

  private:
    void setupSkybox();
    void setupMainCamera();
    void setupWorld();
    void setupUICamera();
    void setupInputCallbacks();
    void setupPlayer();
    void setupEnemies();
    void setupItems();
    void registerDefaultTerritories();
    void setupTotems();
    void setupDialogs();
    void setupWorldVisuals();
    void spawnRandomPointLight();
    void setupSaveAnim();

    dzemikk::Engine* _engine;
    dzemikk::AssetHandle<dzemikk::Scene> _mainScene;
    dzemikk::AssetHandle<dzemikk::Scene> _menuScene;
    dzemikk::AssetHandle<dzemikk::Scene> _creditsScene;
    dzemikk::AssetHandle<dzemikk::Scene> _settingsScene;
    game::HexGrid* _hexGrid = nullptr;
    dzemikk::GameObject* _worldGO = nullptr;
    dzemikk::GameObject* _playerGO = nullptr;
    dzemikk::Camera* _mainCamera = nullptr;
    dzemikk::Material* _materialA = nullptr;
    dzemikk::Material* _materialB = nullptr;
    dzemikk::Material* _quadMaterial = nullptr;
    game::PlayerEntity* _playerEntity = nullptr;
    game::PlayerMovement* _playerMovement = nullptr;
    game::CameraController* _cameraController = nullptr;
    game::GameStateMachine* _stateMachine = nullptr;
    game::EnemyTooltip* _enemyTooltip = nullptr;
    dzemikk::MeshRenderer* _lastHitRenderer = nullptr;
    std::unordered_map<dzemikk::MeshRenderer*, glm::vec4> _lastHitBaseColors;

    bool _gameStarted = false;
    bool _pendingRestart = false;

    dzemikk::MeshRenderer* lastHitRenderer = nullptr;

    std::unordered_map<dzemikk::MeshRenderer*, std::shared_ptr<dzemikk::Material>> baseMaterials;

    game::EnemyEntity* lastHighlightedEnemy = nullptr;
    game::EnemyEntity* animEnemy = nullptr;
    game::EnemyEntity* requestedEnemy = nullptr;

    std::vector<game::HexCell*> animCells;
    size_t animIndex = 0;
    int animFrameCounter = 0;
    static constexpr int animFrameDelay = 7;
};

#endif
