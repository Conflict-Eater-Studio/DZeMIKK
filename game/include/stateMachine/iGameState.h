#ifndef DZEMIKK_IGAMESTATE_H
#define DZEMIKK_IGAMESTATE_H

namespace game {

class IGameState {
  public:
    virtual ~IGameState() = default;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void onUpdate(float dt) = 0;
};
} // namespace game

#endif