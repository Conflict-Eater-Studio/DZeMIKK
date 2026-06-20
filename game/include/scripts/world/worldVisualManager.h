#pragma once

#include "ecs/components/monobehaviour.h"
#include "scripts/world/world.h"

class Game;

namespace dzemikk {
class AssetManager;
}

namespace game {

class WorldVisualManager : public dzemikk::MonoBehaviour {
  public:
    struct SpawnPrefabDef {
        std::string key;
        float weight;
        float minScale;
        float maxScale;
        bool allowRotation;
    };

    using Base = dzemikk::MonoBehaviour;

    void start() override;

    void setWorld(World* world) {
        _world = world;
    }

    void setAssetManager(dzemikk::AssetManager* assetManager) {
        _assetManager = assetManager;
    }

    void setGame(Game* game) {
        _game = game;
    }

    void spawnPrefabOnChunk(const std::string& chunkName, const std::string& prefabKey);

    [[nodiscard]]
    std::string typeName() const override {
        return "WorldVisualManager";
    }

    void init();

    void spawnForestChunk(const std::string& chunkName);
  private:
    struct ForestCluster {
        glm::vec3 center;
        float radius;
    };

    bool isHexFree(HexCell* hex) const;

    void spawnClusterObject(dzemikk::Scene* scene, const std::string& prefabKey,
                            const glm::vec3& pos, float minScale, float maxScale);
    void spawnRockCluster(dzemikk::Scene* scene, const std::vector<std::string>& rocks,
                          const glm::vec3& basePos, int minCount, int maxCount, float spread,
                          float minScale, float maxScale);

    void generateForestPath(const std::vector<std::shared_ptr<HexCell>>& hexes);

    World* _world = nullptr;
    Game* _game = nullptr;

    dzemikk::AssetManager* _assetManager = nullptr;

    std::unordered_map < std::string, dzemikk::AssetHandle <nlohmann::json>> _cache;

    float rand01() {
        return (float)rand() / (float)RAND_MAX;
    }
};

} // namespace game