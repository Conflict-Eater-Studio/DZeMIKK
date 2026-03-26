#ifndef TUL_PBL_DZEMIKK_TIME_H
#define TUL_PBL_DZEMIKK_TIME_H
#include "GLFW/glfw3.h"

namespace dzemikk {

class Time {
    private:
        inline static float lastFrameTime = 0.0f;
    public:
        inline static float deltaTime = 0.0f;
        inline static float fixedDeltaTime = 1.0 / 60.0f;
        inline static float time = 0.0f;
        static void update() {
            time = glfwGetTime();
            deltaTime = time - lastFrameTime;
            lastFrameTime = time;
        }
};

} // namespace dzemikk

#endif // TUL_PBL_DZEMIKK_TIME_H
