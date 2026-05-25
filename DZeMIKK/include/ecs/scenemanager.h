#ifndef TUL_PBL_DZEMIKK_SCENEMANAGER_H
#define TUL_PBL_DZEMIKK_SCENEMANAGER_H
#include "../core/iEngineModule.h"
#include "scene.h"

#include <memory>
#include <vector>

namespace dzemikk {
    class SceneManager : public IEngineModule {
    private:
        std::vector<std::shared_ptr<Scene>> _scenes;
        std::shared_ptr<Scene> _activeScene;
    public:
        SceneManager() = default;
        ~SceneManager() = default;
        void initialize() override;
        void uninitialize() override;
        void loadScene(const std::shared_ptr<Scene>& scene);
        void unloadScene(const std::shared_ptr<Scene>& scene);
        void setActiveScene(const std::shared_ptr<Scene>& scene);
        std::shared_ptr<Scene> getActiveScene() const;
        void update(float deltaTime) const;
        void fixedUpdate(float deltaTime) const;


    };
}

#endif // TUL_PBL_DZEMIKK_SCENEMANAGER_H
