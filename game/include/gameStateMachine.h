#ifndef DZEMIKK_GAME_STATE_MACHINE_H
#define DZEMIKK_GAME_STATE_MACHINE_H

#include "ecs/components/monobehaviour.h"
#include "stateMachine/iGameState.h"

namespace game {

class GameStateMachine : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    void start() override {};
    void lateUpdate(double dt) override {};
    void fixedUpdate(double dt) override {};
    void onDestroy() override {};

    void setState(std::unique_ptr<IGameState> newState) {
        if (_current) {
            _current->onExit();
        }

        _current = std::move(newState);

        if (_current) {
            _current->onEnter();
        }
    }

    void update(double dt) override {
        if (_current) {
            _current->onUpdate(dt);
        }
    }

    [[nodiscard]] std::string typeName() const override {
        return "GameStateMachine";
    }

    [[nodiscard]] IGameState* getCurrentState() const {
        return _current.get();
    }

    template <typename T> [[nodiscard]] T* getCurrentStateAs() const {
        return dynamic_cast<T*>(_current.get());
    }

  private:
    std::unique_ptr<IGameState> _current;
};

} // namespace game

#endif
