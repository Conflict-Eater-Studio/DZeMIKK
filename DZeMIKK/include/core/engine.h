#pragma once
#ifndef DZEMIKK_APPLICATION_H
#define DZEMIKK_APPLICATION_H

#include <memory>
#include <vector>

namespace dzemikk {
class IEngineModule;
class SceneManager;
class Time;
class Window;
class Renderer;
class AnimationModule;
class Engine {
    public:
        Engine();
        ~Engine();

        void start();

        std::shared_ptr<Renderer> getRenderer();
        std::shared_ptr<Window> getWindow();
        std::shared_ptr<SceneManager> getSceneManager();
        std::shared_ptr<Time> getTime();
        std::shared_ptr<AnimationModule> getAnimationSystem();
        template <std::derived_from<IEngineModule> T>
        std::shared_ptr<T> getModule() const;
    private:
        void init();
        void shutdown();
        std::vector<std::shared_ptr<IEngineModule>> _modules;
        std::shared_ptr<Window> _mainWindow;
        std::shared_ptr<Renderer> _renderer;
        std::shared_ptr<SceneManager> _sceneManager;
        std::shared_ptr<Time> _time;
        std::shared_ptr<AnimationModule> _animationSystem;

        float _accumulator = 0.0f;

        // --- Only for test DELETE THIS
        void updateCameraWASD(float speed);
        void updateCameraArrows(float speed);
    };
}

#endif