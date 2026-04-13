#include "ecs/scenemanager.h"

#include "ecs/gameobject.h"

namespace dzemikk {
void SceneManager::Initialize() {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    loadScene(scene);
    setActiveScene(scene);
}
void SceneManager::UnInitialize() {}
void SceneManager::loadScene(const std::shared_ptr<Scene>& scene) {
    _scenes.push_back(scene);
}
void SceneManager::unloadScene(const std::shared_ptr<Scene>& scene) {
    _scenes.erase(std::remove(_scenes.begin(), _scenes.end(), scene), _scenes.end());
}
void SceneManager::setActiveScene(const std::shared_ptr<Scene>& scene) {
    _activeScene = scene;
}
void SceneManager::update(float deltaTime) const {
    _activeScene->update(deltaTime);
}
void SceneManager::fixedUpdate(float fixeDeltaTime) const {
    _activeScene->fixedUpdate(fixeDeltaTime);
}
} // namespace dzemikk
