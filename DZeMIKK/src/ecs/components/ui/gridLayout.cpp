#include "ecs/components/ui/gridLayout.h"

#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"

namespace dzemikk {
void GridLayout::setCellSize(const glm::vec2& cellSize) {
    _cellSize = cellSize;
    rebuild();
}

glm::vec2 GridLayout::getCellSize() const {
    return _cellSize;
}

void GridLayout::setSpacing(const glm::vec2& spacing) {
    _spacing = spacing;
    rebuild();
}

glm::vec2 GridLayout::getSpacing() const {
    return _spacing;
}

void GridLayout::setColumns(int columns) {
    _columns = columns;
    rebuild();
}

int GridLayout::getColumns() const {
    return _columns;
}

void GridLayout::setStartCorner(LayoutStartCorner corner) {
    _startCorner = corner;
    rebuild();
}

LayoutStartCorner GridLayout::getStartCorner() const {
    return _startCorner;
}

void GridLayout::rebuild() {
    if (!_owner) {
        return;
    }

    auto* rect = _owner->getComponent<RectTransform>();
    if (!rect) {
        return;
    }

    const auto& children = _owner->getChildren();
    const size_t childCount = children.size();

    if (childCount == 0) {
        return;
    }

    const glm::vec2 rectSize = rect->getSize();
    const glm::vec2 offsetMin = rect->getOffsetMin();
    const glm::vec2 offsetMax = rect->getOffsetMax();

    const float availableWidth = rectSize[0] - offsetMin[0] - offsetMax[0];
    const float availableHeight = rectSize[1] - offsetMin[1] - offsetMax[1];

    const int columns = _columns > 0 ? _columns : 1;
    const int rows = static_cast<int>((childCount + columns - 1) / columns);

    float cellWidth = _cellSize[0];
    float cellHeight = _cellSize[1];

    if (cellWidth <= 0.0F) {
        float totalSpacingX = static_cast<float>(columns - 1) * _spacing[0];
        cellWidth = (availableWidth - totalSpacingX) / static_cast<float>(columns);
    }
    if (cellHeight <= 0.0F) {
        float totalSpacingY = static_cast<float>(rows - 1) * _spacing[1];
        cellHeight = (availableHeight - totalSpacingY) / static_cast<float>(rows);
    }

    const glm::vec2 effectiveCellSize{cellWidth, cellHeight};
    const glm::vec2 cellFull{cellWidth + _spacing[0], cellHeight + _spacing[1]};

    if (columns == 0) {
        return;
    }

    float startX = offsetMin[0];
    float startY = offsetMin[1];

    for (size_t i = 0; i < childCount; ++i) {
        auto* child = children[i];
        auto* childRect = child->rectTransform();
        if (!childRect) {
            continue;
        }

        auto childPivot = childRect->getPivot();

        int row = static_cast<int>(i) / columns;
        int col = static_cast<int>(i) % columns;

        int effectiveCol = col;
        int effectiveRow = row;

        if (_startCorner == LayoutStartCorner::UpperRight ||
            _startCorner == LayoutStartCorner::LowerRight) {
            effectiveCol = columns - 1 - col;
        }
        if (_startCorner == LayoutStartCorner::UpperLeft ||
            _startCorner == LayoutStartCorner::UpperRight) {
            effectiveRow = rows - 1 - row;
        }

        float posX = startX + (static_cast<float>(effectiveCol) * cellFull[0]) +
                     (effectiveCellSize[0] * childPivot[0]);
        float posY = startY + (static_cast<float>(effectiveRow) * cellFull[1]) +
                     (effectiveCellSize[1] * childPivot[1]);

        childRect->setPosition(glm::vec2(posX, posY));
        childRect->setSize(effectiveCellSize);
    }
}
} // namespace dzemikk
