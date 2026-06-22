#ifndef GAME_ENEMY_MANAGER_H
#define GAME_ENEMY_MANAGER_H

#include "ecs/components/monobehaviour.h"
#include "enemySystem/enemyEntity.h"
#include "enemyTypes.h"
#include "map/HexGrid.h"

#include <random>

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;
class TerritoryPattern;

/**
 * @brief Manages enemy spawning and territory assignment.
 *
 * Responsible for creating enemies based on chunk-specific spawn rules,
 * assigning territories and providing enemy lookup utilities.
 */
class EnemyManager : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    /**
     * @brief Constructs an EnemyManager with a deterministic random seed.
     *
     * @param seed Random generator seed.
     */
    EnemyManager(unsigned int seed = 1);

#pragma region Configuration

    /**
     * @brief Sets the world used for enemy spawning.
     *
     * @param world Target world instance.
     */
    void setWorld(World* world);

    /**
     * @brief Sets the asset manager used during enemy creation.
     *
     * @param assetManager Asset manager instance.
     */
    void setAssetManager(dzemikk::AssetManager* assetManager);

    /**
     * @brief Spawns an enemy on a random cell on a given chunk
     *
     * @param chunkId Chunk ID on which the enemy will be spawned
     * @param config Config for a given enemy
     */
    void addEnemy(const boost::uuids::uuid& chunkId, EnemySpawnConfig config);

    /**
     * @brief Spawns as enemy on a given cell from a given chunk
     *
     * @param chunkId Chunk ID on which the enemy will be spawned
     * @param config Config for a given enemy
     * @param coord Coord of a cell to place the enenmy
     */
    void addEnemy(const boost::uuids::uuid& chunkId, EnemySpawnConfig config,
                  const HexCoord& coord);

    void removeEnemy(EnemyEntity* enemy);

#pragma endregion

#pragma region Enemy management

    /**
     * @brief Returns the enemy occupying a given cell.
     *
     * @param cell Target cell.
     * @return EnemyEntity* Enemy occupying the cell or nullptr.
     */
    [[nodiscard]] EnemyEntity* getEnemyByCell(HexCell* cell) const;

    [[nodiscard]] EnemyEntity* getEnemyByTerritoryCell(HexCell* cell) const;

#pragma endregion

#pragma region Save Load

    [[nodiscard]] nlohmann::json saveState() const;
    void loadState(const nlohmann::json& j);
    void clear();

    void relockBridges();

    [[nodiscard]] std::vector<boost::uuids::uuid> getAndClearRemovedEnemyIds();

#pragma endregion

    [[nodiscard]] std::string typeName() const override {
        return "EnemyManager";
    }

    std::unordered_map<boost::uuids::uuid, std::vector<EnemyEntity*>> getEnemies() {
        return _spawnedEnemies;
    }
  private:
#pragma region Spawning

    void assignTerritory(EnemyEntity* enemy, HexChunk::HexCellPtr centerCell,
                         const TerritoryPattern& pattern);

    static std::vector<HexChunk::HexCellPtr> collectAvailableCells(HexChunk* chunk);
    void spawnEnemy(const boost::uuids::uuid& chunkId, const HexChunk::HexCellPtr& cell,
                    const EnemySpawnConfig& config, bool skipValidation = false);

#pragma endregion

#pragma region Helpers

    HexChunk* findChunkForCoord(const HexCoord& coord);

    bool canPlacePattern(HexCoord center, const TerritoryPattern& pattern);

#pragma endregion

#pragma region References

    World* _world = nullptr;
    dzemikk::AssetManager* _assetManager = nullptr;

#pragma endregion

#pragma region Random

    std::mt19937 _rng;

#pragma endregion

#pragma region Spawn rules

    std::unordered_map<boost::uuids::uuid, std::vector<EnemySpawnConfig>> _spawnRules;

#pragma endregion

#pragma region Lookup

    std::unordered_map<HexCoord, EnemyEntity*> _cellToEnemy;
    std::unordered_map<boost::uuids::uuid, std::vector<EnemyEntity*>> _spawnedEnemies;
    std::vector<boost::uuids::uuid> _removedEnemyIds;

#pragma endregion
};

} // namespace game

#endif
