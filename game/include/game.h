#ifndef DZEMIKK_GAME_H
#define DZEMIKK_GAME_H
#include "core/engine.h"
#include "glad/glad.h"
#include "map/PlayerEntity.h"
#include "scripts/world/world.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace dzemikk {
class Material;
class GameObject;
} // namespace dzemikk
namespace game {
class PlayerMovement;
}
class Game {
  public:
    Game(const Game&) = default;
    Game(Game&&) = delete;
    Game& operator=(const Game&) = delete;
    Game& operator=(Game&&) = delete;
    explicit Game(dzemikk::Engine* engine);
    ~Game() = default;

    void start();

  private:
    void setupSkybox();
    void setupMainCamera();
    void setupWorld();
    void setupUICamera();
    void setupInputCallbacks();
    void setupPlayer();

    dzemikk::Engine* _engine;
    dzemikk::AssetHandle<dzemikk::Scene> _mainScene;
    game::HexGrid* _hexGrid = nullptr;
    dzemikk::GameObject* _worldGO = nullptr;
    dzemikk::GameObject* _playerGO = nullptr;
    dzemikk::Material* _materialA = nullptr;
    dzemikk::Material* _materialB = nullptr;
    dzemikk::Material* _quadMaterial = nullptr;
    game::PlayerEntity* _playerEntity = nullptr;
    game::PlayerMovement* _playerMovement = nullptr;
};

#endif
