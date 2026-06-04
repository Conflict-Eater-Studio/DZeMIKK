#include "ecs/components/ui/verticalLayout.h"

#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"

namespace dzemikk {
void VerticalLayout::setSpacing(float spacing) {
    _spacing = spacing;
    rebuild();
}

float VerticalLayout::getSpacing() const {
    return _spacing;
}

void VerticalLayout::setChildForceExpandWidth(bool enabled) {
    _childForceExpandWidth = enabled;
    rebuild();
}

bool VerticalLayout::getChildForceExpandWidth() const {
    return _childForceExpandWidth;
}

void VerticalLayout::setChildForceExpandHeight(bool enabled) {
    _childForceExpandHeight = enabled;
    rebuild();
}

bool VerticalLayout::getChildForceExpandHeight() const {
    return _childForceExpandHeight;
}

void VerticalLayout::rebuild() {
    if (!_owner) {
        return;
    }

    auto* rect = _owner->getComponent<RectTransform>();
    if (!rect) {
        return;
    }

    const glm::vec2 rectSize = rect->getSize();
    const glm::vec2 rectPivot = rect->getPivot();
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
    float totalChildHeight = availableHeight - totalSpacing;

    float childHeight =
        _childForceExpandHeight ? totalChildHeight / static_cast<float>(childCount) : 0.0F;

    const float left = (-rectPivot[0] * rectSize[0]) + offsetMin[0];
    float startY = ((1.0F - rectPivot[1]) * rectSize[1]) - offsetMax[1];
    for (size_t i = 0; i < childCount; ++i) {
        auto* child = children[i];
        auto* childRect = child->rectTransform();
        if (!childRect) {
            continue;
        }

        glm::vec2 childSize = childRect->getSize();
        glm::vec2 childPivot = childRect->getPivot();

        if (_childForceExpandHeight) {
            childSize[1] = childHeight;
        }

        if (_childForceExpandWidth) {
            childSize[0] = availableWidth;
        }

        float posX = left + (childSize[0] * childPivot[0]);
        float posY = startY - (childSize[1] * (1.0F - childPivot[1]));
        childRect->setPosition(glm::vec2(posX, posY));

        if (_childForceExpandWidth || _childForceExpandHeight) {
            glm::vec2 stretch = childRect->getStretchSize();
            glm::vec2 newBase = childRect->getSize() - stretch;
            if (_childForceExpandWidth) {
                newBase[0] = availableWidth - stretch[0];
            }
            if (_childForceExpandHeight) {
                newBase[1] = childHeight - stretch[1];
            }
            childRect->setBaseSize(newBase);
        }

        startY -= childSize[1] + _spacing;
    }
}
} // namespace dzemikk
