#ifndef DZEMIKK_APPLICATION_H
#define DZEMIKK_APPLICATION_H
#include <memory>
#include "window.h"
#include "renderer/renderer.h"

namespace dzemikk {
    class Engine {
    private:
        std::shared_ptr<Window> mainWindow;
        std::shared_ptr<Renderer> _renderer;
        void init();
    public:
        Engine();
        ~Engine();
        void update();

        std::shared_ptr<Renderer> GetRenderer() {
            return _renderer;
        }

        // --- Only for test DELETE THIS
        void updateCameraWASD(float speed);
        void updateCameraArrows(float speed);
    };
}

#endif //DZEMIKK_APPLICATION_H