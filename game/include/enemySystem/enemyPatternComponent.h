#include "enemySystem/patternComponent.h"
#include "map/HexPattern.h"

namespace game {
class EnemyPatternComponent : public PatternComponent{
  public:
    struct PatternUsage {
        const HexPattern* pattern = nullptr;
        uint32_t count = 0;
    };

    void start() override;
    void update(double deltaTime) override;
    void onDestroy() override;

    std::string typeName() const override;

    bool usePattern(size_t index) override;

    void clearUsage();
    void registerPatternUsage(const HexPattern* pattern);
    uint32_t getUsageCount(const HexPattern* pattern) const;
    const std::unordered_map<const HexPattern*, uint32_t>& getPatternUsage() const;

  private:
    std::unordered_map<const HexPattern*, uint32_t> _patternUsage;
};
}