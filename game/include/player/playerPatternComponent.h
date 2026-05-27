#include "ecs/components/monobehaviour.h"
#include <assetManager/assetHandle.h>
#include "map/HexPattern.h"
#include "map/HexCell.h"

#include "nlohmann/json.hpp"

using namespace dzemikk;

namespace dzemikk {
class GameObject;
class AssetManager;
}

namespace game {
class PlayerEntity;
class HexGrid;

class PlayerPatternComponent : public MonoBehaviour {
  public:
    struct PatternEntry {
        HexPattern pattern;
        int count = 0;
    };

    void start() override;
    void update(double deltaTime) override;

    void addPattern(const HexPattern& pattern, int count = 1);

    bool canUsePattern(size_t index) const;

    std::vector<HexCell*> getAffectedCells(size_t index, HexPattern::Rotation rotation) const;

    void usePattern(size_t index);

    void setPlayer(PlayerEntity* player);
    [[nodiscard]] PlayerEntity* getPlayer() const;

    void setGrid(HexGrid* grid);
    [[nodiscard]] HexGrid* getGrid() const;

    void setPlayerPatternsCanvas(GameObject* playerPatternsCanvas);
    [[nodiscard]] GameObject* getPlayerPatternsCanvas() const;

    void setAssetManager(AssetManager* assetManager);

    std::string typeName() const override;

    void setupUI();

  private:
    std::vector<PatternEntry> _patterns;
    PlayerEntity* _player = nullptr;
    HexGrid* _grid = nullptr;
    GameObject* _playerPatternsCanvas = nullptr;
    dzemikk::AssetHandle<nlohmann::json> _patternSlotPrefab;
    dzemikk::AssetHandle<nlohmann::json> _hexUIPrefab;
    AssetManager* _assetManager = nullptr;
};

} // namespace game