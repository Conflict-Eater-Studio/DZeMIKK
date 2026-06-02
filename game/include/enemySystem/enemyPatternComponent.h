#include "enemySystem/patternComponent.h"
#include "map/HexPattern.h"

namespace game {
class EnemyPatternComponent : public PatternComponent{
  public:
    void start() override;
    void update(double deltaTime) override;
    void onDestroy() override;

    std::string typeName() const override;

    bool usePattern(size_t index) override;
};
}