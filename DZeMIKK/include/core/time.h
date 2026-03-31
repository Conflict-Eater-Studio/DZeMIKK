#pragma once
#ifndef DZEMIKK_TIME_H
#define DZEMIKK_TIME_H
#include "iEngineModule.h"

namespace dzemikk {
    class Time : public IEngineModule{
    public:
        Time();
        ~Time();
        void update();

        float deltaTime;
        float fixedDeltaTime;
        float time;

        void update() const;
        float getDeltaTime() const;
        float getFixedDeltaTime() const;
        float getTime() const;
        void Initialize() override;
        void UnInitialize() override;
    private:
        float _lastFrameTime{};
    };
}
#endif
