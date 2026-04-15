#ifndef DZEMIKK_APPLICATION_H
#define DZEMIKK_APPLICATION_H
#include <memory>
#include "window.h"
#include "renderer/renderer.h"
#include "events/event.h"
#include "input/input.h"
#include <functional>

namespace dzemikk {
    class Engine {
    private:
        std::shared_ptr<Window> mainWindow;
        std::shared_ptr<Renderer> _renderer;
        std::shared_ptr<Input> _input;
        std::function<void()> m_UserUpdateCallback;
        void init();
    public:
        Engine();
        ~Engine();
        void update() const;
        void OnEvent(Event& e);

        void SetUserUpdateCallback(const std::function<void()>& callback) { m_UserUpdateCallback = callback; }

        std::shared_ptr<Renderer> GetRenderer() {
            return _renderer;
        }

        std::shared_ptr<Input> GetInput() {
            return _input;
        }
    };
}

#endif //DZEMIKK_APPLICATION_H