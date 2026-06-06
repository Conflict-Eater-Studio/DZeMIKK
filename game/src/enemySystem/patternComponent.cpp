#include "enemySystem/patternComponent.h"

void game::PatternComponent::addPattern(const HexPattern& pattern, int count) {
    _patterns.push_back({pattern, count});
}

void game::PatternComponent::insertPattern(size_t index, const HexPattern& pattern, int count) {
    index = std::min(index, _patterns.size());

    _patterns.insert(_patterns.begin() + index, {pattern, count});
}

bool game::PatternComponent::removePattern(size_t index) {
    if (index >= _patterns.size())
        return false;

    _patterns.erase(_patterns.begin() + index);

    return true;
}

bool game::PatternComponent::removePattern(const HexPattern& pattern) {
    auto it = std::find_if(_patterns.begin(), _patterns.end(),
                           [&](const PatternEntry& entry) { return entry.pattern == pattern; });

    if (it == _patterns.end())
        return false;

    _patterns.erase(it);

    return true;
}

void game::PatternComponent::clearPatterns() {
    _patterns.clear();
}

bool game::PatternComponent::canUsePattern(size_t index) const {
    if (index >= _patterns.size())
        return false;

    return _patterns[index].count != 0;
}

std::string game::PatternComponent::typeName() const {
    return "PatternComponent";
}

size_t game::PatternComponent::getPatternCount() const {
    return _patterns.size();
}

game::PatternComponent::PatternEntry* game::PatternComponent::getPattern(size_t index) {
    if (index >= _patterns.size())
        return nullptr;

    return &_patterns[index];
}

const game::PatternComponent::PatternEntry* game::PatternComponent::getPattern(size_t index) const {
    if (index >= _patterns.size())
        return nullptr;

    return &_patterns[index];
}

const std::vector<game::PatternComponent::PatternEntry>&
game::PatternComponent::getPatterns() const {
    return _patterns;
}

int game::PatternComponent::findPattern(const HexPattern& pattern) const {
    for (size_t i = 0; i < _patterns.size(); ++i) {
        if (_patterns[i].pattern == pattern)
            return static_cast<int>(i);
    }

    return -1;
}

bool game::PatternComponent::hasPattern(const HexPattern& pattern) const {
    for (const auto& p : _patterns) {
        auto pat = p.pattern;

        if (pat.getEffectStrength() != pattern.getEffectStrength() ||
            pat.getType() != pattern.getType()) {
            return false;
        }

        if (pat.getHexes().size() != pattern.getHexes().size()) {
            return false;
        }

        bool valid = false;
        for (size_t i = 0; i < 6; i++) {
            // NOTE: is_permutation is good enough for <100 elemets in the array. (If any pattern
            // has more than 100 hexes then we have offically lost our minds. >_<)
            valid = std::is_permutation(pat.getHexes().begin(), pat.getHexes().end(),
                                        pattern.getHexes().begin());

            if (valid) {
                return true;
            }

            (void)pat.rotate(HexPattern::Rotation::Clockwise);
        }
    }

    return false;
}
