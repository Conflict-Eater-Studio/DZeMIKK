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
    void setupScene();
    void setupSkybox();
    void setupMainCamera();
    void setupMaterials();
    void setupWorld();
    void setupPlayer();
    void setupChest();
    void setupEnemy();
    void setupUICamera();
    void setupUI();
    void setupButton(dzemikk::GameObject* canvasGo);
    void setupSlider(dzemikk::GameObject* canvasGo);
    void setupCheckbox(dzemikk::GameObject* canvasGo);
    void setupAudio();
    void setupInputCallbacks();
    void newModels(const std::shared_ptr<dzemikk::Material>& m, dzemikk::Scene* scene);
    void spawnModel(dzemikk::Scene* scene, std::shared_ptr<dzemikk::Material> material,
                    const std::string& modelPath, const glm::vec3& position,
                    const glm::vec3& scale = glm::vec3(1.0F),
                    const glm::quat& rotation = glm::quat(glm::vec3(0.0F)));

    dzemikk::Engine* _engine;
    std::shared_ptr<dzemikk::Scene> _mainScene;
    game::HexGrid* _hexGrid = nullptr;
    dzemikk::GameObject* _worldGO = nullptr;
    dzemikk::GameObject* _playerGO = nullptr;
    dzemikk::GameObject* _enemyGO = nullptr;
    dzemikk::Material* _materialA = nullptr;
    dzemikk::Material* _materialB = nullptr;
    dzemikk::Material* _quadMaterial = nullptr;
    game::PlayerEntity* _playerEntity = nullptr;
};

#endif
