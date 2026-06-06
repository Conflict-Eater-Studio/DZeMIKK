#ifndef GAME_ENEMY_MANAGER_H
#define GAME_ENEMY_MANAGER_H

#include "ecs/components/monobehaviour.h"
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
     * @brief Sets enemy spawn rules for a chunk.
     *
     * @param chunkId Chunk identifier.
     * @param config Spawn configuration list.
     */
    void setSpawnConfig(const boost::uuids::uuid& chunkId,
                        const std::vector<EnemySpawnConfig>& config);

#pragma endregion

#pragma region Enemy management

    /**
     * @brief Spawns enemies according to configured chunk rules.
     */
    void spawnEnemiesPerChunk();

    /**
     * @brief Returns the enemy occupying a given cell.
     *
     * @param cell Target cell.
     * @return EnemyEntity* Enemy occupying the cell or nullptr.
     */
    [[nodiscard]] EnemyEntity* getEnemyByCell(HexCell* cell) const;

#pragma endregion

    [[nodiscard]] std::string typeName() const override {
        return "EnemyManager";
    }

  private:
#pragma region Spawning

    void spawnEnemy(HexChunk::HexCellPtr cell, const EnemySpawnConfig& cfg,
                    const boost::uuids::uuid& spawnChunkId);
    void assignTerritory(EnemyEntity* enemy, HexChunk::HexCellPtr centerCell,
                         const TerritoryPattern& pattern);

    static std::vector<HexChunk::HexCellPtr> collectAvailableCells(HexChunk* chunk);
    void spawnFromConfig(const EnemySpawnConfig& cfg,
                         std::vector<HexChunk::HexCellPtr>& availableCells, size_t& cursor,
                         const boost::uuids::uuid& spawnChunkId);
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

#pragma endregion
};

} // namespace game

#endif
