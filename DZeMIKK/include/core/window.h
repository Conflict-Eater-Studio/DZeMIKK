//
// Created by damia on 17.03.2026.
//

#ifndef DZEMIKK_WINDOW_H
#define DZEMIKK_WINDOW_H
#include "GLFW/glfw3.h"

namespace dzemikk {
    class Window {
        private:

            GLFWwindow* window;
        public:
        Window();
        Window(int width, int height, const char* title);
        ~Window();
    };
} // dzemikk

#endif //DZEMIKK_WINDOW_H