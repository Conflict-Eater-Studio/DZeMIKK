#ifndef DZEMIKK_GRIDLAYOUT_H
#define DZEMIKK_GRIDLAYOUT_H

#include "ecs/component.h"

#include <glm/glm.hpp>
#include <string>

namespace dzemikk {

enum class LayoutStartCorner : uint8_t {
    UpperLeft = 0,
    UpperRight = 1,
    LowerLeft = 2,
    LowerRight = 3
};

class GridLayout : public Component {
  public:
    using Base = Component;

    GridLayout() = default;
    GridLayout(const GridLayout& other) = delete;
    GridLayout& operator=(const GridLayout& other) = delete;
    GridLayout(GridLayout&& other) noexcept = delete;
    GridLayout& operator=(GridLayout&& other) noexcept = delete;
    ~GridLayout() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "GridLayout";
    }

    void setCellSize(const glm::vec2& cellSize);
    [[nodiscard]] glm::vec2 getCellSize() const;

    void setSpacing(const glm::vec2& spacing);
    [[nodiscard]] glm::vec2 getSpacing() const;

    void setColumns(int columns);
    [[nodiscard]] int getColumns() const;

    void setStartCorner(LayoutStartCorner corner);
    [[nodiscard]] LayoutStartCorner getStartCorner() const;

    void rebuild();

  private:
    glm::vec2 _cellSize{100.0F, 100.0F};
    glm::vec2 _spacing{0.0F, 0.0F};
    int _columns = 1;
    LayoutStartCorner _startCorner = LayoutStartCorner::UpperLeft;
};

} // namespace dzemikk

#endif // DZEMIKK_GRIDLAYOUT_H
