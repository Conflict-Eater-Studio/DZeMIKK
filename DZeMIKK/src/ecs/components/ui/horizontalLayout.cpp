#include "ecs/components/ui/horizontalLayout.h"

#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"

namespace dzemikk {
void HorizontalLayout::setSpacing(float spacing) {
    _spacing = spacing;
    rebuild();
}

float HorizontalLayout::getSpacing() const {
    return _spacing;
}

void HorizontalLayout::setChildForceExpandWidth(bool enabled) {
    _childForceExpandWidth = enabled;
    rebuild();
}

bool HorizontalLayout::getChildForceExpandWidth() const {
    return _childForceExpandWidth;
}

void HorizontalLayout::setChildForceExpandHeight(bool enabled) {
    _childForceExpandHeight = enabled;
    rebuild();
}

bool HorizontalLayout::getChildForceExpandHeight() const {
    return _childForceExpandHeight;
}

void HorizontalLayout::rebuild() {
    if (!_owner) {
        return;
    }

    auto* rect = _owner->getComponent<RectTransform>();
    if (!rect) {
        return;
    }

    const glm::vec2 rectSize = rect->getSize();
    const glm::vec2 offsetMin = rect->getOffsetMin();
    const glm::vec2 offsetMax = rect->getOffsetMax();

    const float availableWidth = rectSize[0] - offsetMin[0] - offsetMax[0];
    const float availableHeight = rectSize[1] - offsetMin[1] - offsetMax[1];

    const auto& children = _owner->getChildren();
    const size_t childCount = children.size();

    if (childCount == 0) {
        return;
    }

    float totalSpacing = static_cast<float>(childCount - 1) * _spacing;
    float totalChildWidth = availableWidth - totalSpacing;

    float childWidth =
        _childForceExpandWidth ? totalChildWidth / static_cast<float>(childCount) : 0.0F;

    float x = offsetMin[0];
    for (size_t i = 0; i < childCount; ++i) {
        auto* child = children[i];
        auto* childRect = child->rectTransform();
        if (!childRect) {
            continue;
        }

        glm::vec2 childSize = childRect->getSize();
        glm::vec2 childPivot = childRect->getPivot();

        if (_childForceExpandWidth) {
            childSize[0] = childWidth;
        }

        if (_childForceExpandHeight) {
            childSize[1] = availableHeight;
        }

        float posX = x + (childSize[0] * childPivot[0]);
        float posY = offsetMin[1] + (childSize[1] * childPivot[1]);
        childRect->setPosition(glm::vec2(posX, posY));

        childRect->setSize(childSize);

        x += childSize[0] + _spacing;
    }
}
} // namespace dzemikk
