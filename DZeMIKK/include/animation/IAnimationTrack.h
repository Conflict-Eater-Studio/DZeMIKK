#pragma once
#ifndef DZEMIKK_IANIMATIONTRACK_H
#define DZEMIKK_IANIMATIONTRACK_H
#include "ecs/serialize/uuid.h"
class IAnimationTrack {
public:
    IAnimationTrack() = default;
    virtual ~IAnimationTrack() = default;
    virtual void apply(float time) = 0;
    virtual std::string getType() const = 0;
    virtual nlohmann::json serialize() const = 0;
};
#endif
