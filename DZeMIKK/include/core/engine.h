#ifndef DZEMIKK_APPLICATION_H
#define DZEMIKK_APPLICATION_H
#include <memory>
#include "window.h"

namespace dzemikk {
    class Engine {
    private:
        std::shared_ptr<Window> mainWindow;
        void init();
    public:
        Engine();
        ~Engine();
        void update() const;
    };
}



#endif //DZEMIKK_APPLICATION_H