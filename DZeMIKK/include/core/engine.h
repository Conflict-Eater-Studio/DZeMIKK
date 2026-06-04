#pragma once
#ifndef DZEMIKK_ENGINE_H
#define DZEMIKK_ENGINE_H

#include <memory>
#include <vector>
#include <concepts>

#include "events/event.h"
#include "input/input.h"

namespace dzemikk {

class Scene;
class IEngineModule;
class SceneManager;
class Time;
class Window;
class Renderer;
class AssetManager;
class AnimationModule;
class Input;
class Collisions;
class AudioManager;

enum class EngineMode { Game, Editor };

/**
 * @brief The core application class managing the game loop and all subsystems.
 */
class Engine {
public:
    Engine(EngineMode mode = EngineMode::Game);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void start();

    [[nodiscard]] Renderer* getRenderer() const;
    [[nodiscard]] Window* getWindow() const;
    [[nodiscard]] SceneManager* getSceneManager() const;
    [[nodiscard]] Time* getTime() const;
    [[nodiscard]] AnimationModule* getAnimationModule() const;
    [[nodiscard]] AssetManager* getAssetManager() const;
    [[nodiscard]] Input* getInput() const;
    [[nodiscard]] Collisions* getCollisions() const;
    [[nodiscard]] AudioManager* getAudioManager() const;

    
    [[nodiscard]] EngineMode getMode() const {
        return _mode;
    }
    [[nodiscard]] bool isEditorMode() const {
        return _mode == EngineMode::Editor;
    }


    void SetUserUpdateCallback(const std::function<void()>& callback) {
        m_UserUpdateCallback = callback;
    }
    void OnEvent(Event& e);

    // --- Only for test DELETE THIS ---
    void updateCameraWASD(float speed);
    void updateCameraArrows(float speed);
	void updateMouseUI(float deltaTime);

private:
    void init();
    void shutdown();

    std::unique_ptr<Window> _mainWindow;
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<SceneManager> _sceneManager;
    std::unique_ptr<Time> _time;
    std::unique_ptr<AssetManager> _assetManager;
    std::unique_ptr<AnimationModule> _animationModule;
    std::unique_ptr<Input> _input;
    std::unique_ptr<Collisions> _collisions;
    std::unique_ptr<AudioManager> _audioManager;

    std::vector<std::unique_ptr<IEngineModule>> _modules;

    std::function<void()> m_UserUpdateCallback;

    float _accumulator = 0.0F;
    bool _wasLeftMouseDown = false;
    double _scrollDelta = 0.0F;

    EngineMode _mode;
};
} // namespace dzemikk

#endif // DZEMIKK_ENGINE_H