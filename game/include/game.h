#ifndef DZEMIKK_GAME_H
#define DZEMIKK_GAME_H
#include "core/engine.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace dzemikk {
class Material;
class GameObject;
}
class Game {
public:
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
    void newModels(std::shared_ptr<dzemikk::Material> m, dzemikk::Scene* scene);
    void spawnModel(dzemikk::Scene* scene, std::shared_ptr<dzemikk::Material> material,
                    const std::string& modelPath, const glm::vec3& position,
                    const glm::vec3& scale = glm::vec3(1.0f),
                    const glm::quat& rotation = glm::quat(glm::vec3(0.0f)));

    dzemikk::Engine* engine;
    std::shared_ptr<dzemikk::Scene> mainScene;
    dzemikk::GameObject* playerGO = nullptr;
    dzemikk::Material* materialA = nullptr;
    dzemikk::Material* materialB = nullptr;
    dzemikk::Material* quadMaterial = nullptr;
};

#endif