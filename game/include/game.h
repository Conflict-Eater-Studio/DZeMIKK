#ifndef DZEMIKK_GAME_H
#define DZEMIKK_GAME_H
#include "core/engine.h"

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

    dzemikk::Engine* engine;
    std::shared_ptr<dzemikk::Scene> mainScene;
    dzemikk::GameObject* playerGO = nullptr;
    dzemikk::Material* materialA = nullptr;
    dzemikk::Material* materialB = nullptr;
    dzemikk::Material* quadMaterial = nullptr;
};

#endif