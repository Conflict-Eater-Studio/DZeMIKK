#pragma once
#ifndef DZEMIKK_WINDOW_H
#define DZEMIKK_WINDOW_H
#include "iEngineModule.h"

struct GLFWwindow;

namespace dzemikk {
    class Window : public IEngineModule {
    private:
        GLFWwindow* window_;
    public:
        Window();
        Window(int width, int height, const char* title);
        ~Window();
        bool shouldClose() const;
        void pollEvents() const;
        void swapBuffers() const;
        void clear(float red, float green, float blue, float alpha) const;
        GLFWwindow* nativeHandle() const;
        void Initialize() override;
        void UnInitialize() override;
    };
}

#endif