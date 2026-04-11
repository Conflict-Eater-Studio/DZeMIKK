// #ifndef DZEMIKK_ANIMATIONTRACK_H
// #define DZEMIKK_ANIMATIONTRACK_H
// #pragma once
//
// #include <functional>
// #include <string>
// #include <vector>
//
// struct aiNodeAnim;
// namespace dzemikk {
//
// class AnimationTrack {
// public:
//     AnimationTrack();
//     AnimationTrack(const aiNodeAnim* track);
//     AnimationTrack(const AnimationTrack&) = delete;
//     AnimationTrack& operator=(const AnimationTrack&) = delete;
//
//     const std::string& getName() const noexcept;
//     void setName(const std::string& name);
//
//     void interpolate(float time);
//
//     void setProperty(const FloatProperty& property);
//     FloatProperty getProperty() const;
//
//     const std::vector<FloatPropertyKey>& getKeys() const noexcept;
//     void addKey(FloatPropertyKey key);
//     void setKeys(std::vector<FloatPropertyKey> keys);
//
// private:
//     std::string _name;
//     FloatProperty _property;
//     std::vector<FloatPropertyKey> _keys;
//
//     size_t findPropertyIndex(float time) const;
// };
//
// }
// #endif
