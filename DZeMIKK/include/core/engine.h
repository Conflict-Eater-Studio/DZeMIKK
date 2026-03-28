#ifndef DZEMIKK_APPLICATION_H
#define DZEMIKK_APPLICATION_H
#include <memory>
#include "window.h"
#include "renderer/renderer.h"

namespace dzemikk {

class ComponentRegistry {
//     private:
//     ComponentRegistry() = default;
//     ~ComponentRegistry() = default;
//
// public:
//     ComponentRegistry instance;
//     std::vector<std::shared_ptr<Component>> components;
//     void add(std::shared_ptr<Component> component);
//     void remove(std::shared_ptr<Component> component);
//     template<T>
//     const std::vector<std::shared_ptr<Component>> getComponents<T>();
};
class Engine {
    private:
        std::vector<std::shared_ptr<IEngineModule>> _modules;
        std::shared_ptr<Window> _mainWindow;
        std::shared_ptr<Renderer> _renderer;
        void init();
    public:
        Engine();
        ~Engine();
        void start() const;
        void update() const;
        void fixedUpdate() const;
        void lateUpdate() const;

        template <std::derived_from<dzemikk::IEngineModule> T>
        std::shared_ptr<T> getModule() const;

        std::shared_ptr<Renderer> getRenderer() {
            return _renderer;
        }
        std::shared_ptr<Window> GetWindow() {
            return _mainWindow;
        }
    };
} // namespace dzemikk

#endif //DZEMIKK_APPLICATION_H