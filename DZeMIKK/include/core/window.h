#pragma once
#ifndef DZEMIKK_WINDOW_H
#define DZEMIKK_WINDOW_H
#include "iEngineModule.h"

#include "events/event.h"
#include <functional>

struct GLFWwindow;

namespace dzemikk {
    class Window : public IEngineModule {
    public:
        using EventCallbackFn = std::function<void(Event&)>;
    
    private:
        GLFWwindow* window_;
        
        struct WindowData {
            std::string Title;
            unsigned int Width, Height;
            EventCallbackFn EventCallback;
        };

        WindowData data_;
    public:
        Window();
        Window(int width, int height, const char* title);
        ~Window();
        bool shouldClose() const;
        void pollEvents() const;
        void swapBuffers() const;
        void clear(float red, float green, float blue, float alpha) const;
        GLFWwindow* nativeHandle() const;
        void initialize() override;
        void uninitialize() override;

        void setEventCallback(const EventCallbackFn& callback) {
            data_.EventCallback = callback;
        }
    };
}

#endif