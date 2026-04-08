#ifndef DZEMIKK_APPLICATION_H
#define DZEMIKK_APPLICATION_H
#include <memory>
#include "window.h"
#include "ecs/scene.h"
#include "renderer/renderer.h"
#include "ecs/scenemanager.h"
#include "time.h"
#include "window.h"

#include <memory>

namespace dzemikk {
class Engine {
    public:
        Engine();
        ~Engine();
        // --- Only for test DELETE THIS
        void updateCameraWASD(float speed);
        void updateCameraArrows(float speed);

        void start();

        std::shared_ptr<Renderer> getRenderer();
        std::shared_ptr<Window> getWindow();
        std::shared_ptr<SceneManager> getSceneManager();
        std::shared_ptr<Time> getTime();

        template <std::derived_from<IEngineModule> T>
        std::shared_ptr<T> getModule() const;
         Scene* scene;
    private:
        void init();
        void shutdown();
        std::vector<std::shared_ptr<IEngineModule>> _modules;
        std::shared_ptr<Window> _mainWindow;
        std::shared_ptr<Renderer> _renderer;
        std::shared_ptr<SceneManager> _sceneManager;
        std::shared_ptr<Time> _time;

        float _accumulator = 0.0f;
    };
} // namespace dzemikk

#endif //DZEMIKK_APPLICATION_H