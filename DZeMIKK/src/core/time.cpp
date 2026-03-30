#include "core/time.h"
#include "GLFW/glfw3.h"

namespace dzemikk {
Time::Time() {
}
Time::~Time() {
}
void Time::update() {
    time = glfwGetTime();
    deltaTime = time - _lastFrameTime;
    _lastFrameTime = time;
}
float Time::getDeltaTime() const {
    return deltaTime;
}
float Time::getFixedDeltaTime() const {
    return fixedDeltaTime;
}
float Time::getTime() const {
    return time;
}
void Time::Initialize() {
    time = 0.0f;
    deltaTime = 0.0f;
    _lastFrameTime = 0.0f;


};
void Time::UnInitialize() {

};
}

