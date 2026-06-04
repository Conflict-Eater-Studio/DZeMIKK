#pragma once
#ifndef DZEMIKK_PROPERTYKEY_H
#define DZEMIKK_PROPERTYKEY_H
#define GLM_ENABLE_EXPERIMENTAL

template<typename T>
struct PropertyKey {
    float time;
    T value;
};

#endif