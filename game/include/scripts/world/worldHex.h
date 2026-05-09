#ifndef GAME_WORLDHEX_H
#define GAME_WORLDHEX_H

#include "ecs/components/monoBehaviour.h"
#include "map/HexCell.h"

#include <string>

namespace game {
class WorldHex : public dzemikk::MonoBehaviour {
  public:
    WorldHex() = default;

    [[nodiscard]] std::string typeName() const override {
        return "WorldHex";
    }

    void setHexCell(HexCell* hexCell) {
        _hexCell = hexCell;
    }
    HexCell* getHexCell() {
        return _hexCell;
    }

  private:
    HexCell* _hexCell{nullptr};
};
} // namespace game

#endif // GAME_WORLDHEX_H
