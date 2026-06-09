#ifndef GAME_WORLDHEX_H
#define GAME_WORLDHEX_H

#include "ecs/components/monoBehaviour.h"
#include "map/HexCell.h"

#include <string>
#include <utility>

namespace game {
class WorldHex : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    WorldHex() = default;

    [[nodiscard]] std::string typeName() const override {
        return "WorldHex";
    }

    void setHexCell(std::shared_ptr<HexCell> hexCell) {
        _hexCell = std::move(hexCell);
    }
    std::shared_ptr<HexCell>& getHexCell() {
        return _hexCell;
    }

  private:
    std::shared_ptr<HexCell> _hexCell{nullptr};
};
} // namespace game

#endif // GAME_WORLDHEX_H
