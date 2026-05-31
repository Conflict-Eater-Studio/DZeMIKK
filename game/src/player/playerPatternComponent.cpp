#include "player/playerPatternComponent.h"
#include <iostream>

void game::PlayerPatternComponent::start() {
    // atk1
    addPattern(HexPattern({{0, 0}, {1, -1}, {2, -2}}, HexPattern::Type::ATK), 1);

    // atk2
    addPattern(HexPattern({{0, 0}, {1, 0}, {2, -1}}, HexPattern::Type::ATK), -1);

    // atk3
    addPattern(HexPattern({{0, 0}, {1, -1}, {-1, 0}, {0, 1}}, HexPattern::Type::ATK), -1);

    // def1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::DEF), -1);

    // def2
    addPattern(HexPattern({{0, 0}, {1, -1}, {2, -2}}, HexPattern::Type::DEF), -1);

    // hp1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::HEAL), -1);
}

void game::PlayerPatternComponent::update(double deltaTime) {}

void game::PlayerPatternComponent::addPattern(const HexPattern& pattern, int count) {
    _patterns.push_back({pattern, count});
}

void game::PlayerPatternComponent::insertPattern(size_t index, const HexPattern& pattern,
                                                 int count) {
    index = std::min(index, _patterns.size());

    _patterns.insert(_patterns.begin() + index, {pattern, count});
}

bool game::PlayerPatternComponent::removePattern(size_t index) {
    if (index >= _patterns.size())
        return false;

    _patterns.erase(_patterns.begin() + index);

    return true;
}

bool game::PlayerPatternComponent::removePattern(const HexPattern& pattern) {
    auto it = std::find_if(_patterns.begin(), _patterns.end(),
                           [&](const PatternEntry& entry) { return entry.pattern == pattern; });

    if (it == _patterns.end())
        return false;

    _patterns.erase(it);

    return true;
}

void game::PlayerPatternComponent::clearPatterns() {
    _patterns.clear();
}

bool game::PlayerPatternComponent::addCount(size_t index, int amount) {
    if (index >= _patterns.size())
        return false;

    _patterns[index].count += amount;

    return true;
}

bool game::PlayerPatternComponent::removeCount(size_t index, int amount) {
    if (index >= _patterns.size())
        return false;

    if (_patterns[index].count < amount)
        return false;

    _patterns[index].count -= amount;

    return true;
}

bool game::PlayerPatternComponent::setCount(size_t index, int count) {
    if (index >= _patterns.size())
        return false;

    _patterns[index].count = count;

    return true;
}

bool game::PlayerPatternComponent::canUsePattern(size_t index) const {
    if (index >= _patterns.size())
        return false;

    return _patterns[index].count != 0;
}

bool game::PlayerPatternComponent::usePattern(size_t index) {
    if (!canUsePattern(index))
        return false;

    auto& entry = _patterns[index];

    if (entry.count > 0) {
        entry.count--;
    }

    //TO DO: do something

    return true;
}

size_t game::PlayerPatternComponent::getPatternCount() const {
    return _patterns.size();
}

game::PlayerPatternComponent::PatternEntry* game::PlayerPatternComponent::getPattern(size_t index) {
    if (index >= _patterns.size())
        return nullptr;

    return &_patterns[index];
}

const game::PlayerPatternComponent::PatternEntry* game::PlayerPatternComponent::getPattern(size_t index) const {
    if (index >= _patterns.size())
        return nullptr;

    return &_patterns[index];
}

const std::vector<game::PlayerPatternComponent::PatternEntry>&
game::PlayerPatternComponent::getPatterns() const {
    return _patterns;
}

int game::PlayerPatternComponent::findPattern(const HexPattern& pattern) const {
    for (size_t i = 0; i < _patterns.size(); ++i) {
        if (_patterns[i].pattern == pattern)
            return static_cast<int>(i);
    }

    return -1;
}

std::string game::PlayerPatternComponent::typeName() const {
    return "PlayerPatternComponent";
}
