#pragma once

#include "core/key_codes.h"
#include "core/mouse_codes.h"
#include <glm/glm.hpp>

struct GLFWwindow;

namespace dzemikk {

    class Input {
    public:
        // Polling methods - accessible anywhere!
        static bool IsKeyPressed(const KeyCode key);
        static bool IsMouseButtonPressed(const MouseCode button);
        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();

        // Called internally to initialize state
        static void Initialize(GLFWwindow* window);
    private:
        static GLFWwindow* s_WindowContext;
    };

}
