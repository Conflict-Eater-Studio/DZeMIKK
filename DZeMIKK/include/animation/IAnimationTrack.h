#pragma once
#ifndef DZEMIKK_IANIMATIONTRACK_H
#define DZEMIKK_IANIMATIONTRACK_H
class IAnimationTrack {
public:
    IAnimationTrack() = default;
    virtual ~IAnimationTrack() = default;
    virtual void apply(float time) = 0;
};
#endif
