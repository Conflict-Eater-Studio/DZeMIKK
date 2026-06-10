#ifndef GAME_TOTEM_MANAGER_H
#define GAME_TOTEM_MANAGER_H

#include "ecs/components/monobehaviour.h"
#include "map/HexChunk.h"
#include "totemSpawnConfig.h"

#include <random>

class Game;

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;
class TotemEntity;

/**
 * @brief Manages spawning and placement of totems across world chunks.
 *
 * Responsible for configuring chunk-specific spawn rules, selecting
 * valid spawn locations, and creating totem entities in the world.
 */
class TotemManager : public dzemikk::MonoBehaviour {
  public:
    /**
     * @brief Creates a new TotemManager.
     *
     * @param seed Random seed used for totem placement.
     */
    TotemManager(unsigned int seed = 1);

    /**
     * @brief Sets the world used for spawning totems.
     *
     * @param world World instance.
     */
    void setWorld(World* world);

    /**
     * @brief Sets the asset manager used for prefab loading.
     *
     * @param assetManager Asset manager instance.
     */
    void setAssetManager(dzemikk::AssetManager* assetManager);

    /**
     * @brief Sets the owning game instance.
     *
     * @param game Game instance.
     */
    void setGame(Game* game);

    /**
     * @brief Assigns spawn rules for a specific chunk.
     *
     * @param chunkId Target chunk identifier.
     * @param config Totem spawn configurations for the chunk.
     */
    void setSpawnConfig(const boost::uuids::uuid& chunkId,
                        const std::vector<TotemSpawnConfig>& config);

    /**
     * @brief Spawns all configured totems in their respective chunks.
     */
    void spawnTotemsPerChunk();

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemManager";
    }

  private:
    /**
     * @brief Creates a totem on the specified cell.
     *
     * @param cell Target spawn cell.
     * @param cfg Totem configuration.
     */
    void spawnTotem(HexChunk::HexCellPtr cell, const TotemSpawnConfig& cfg);

    /**
     * @brief Collects cells that are valid candidates for totem spawning.
     *
     * @param chunk Chunk being evaluated.
     *
     * @return List of available cells.
     */
    static std::vector<HexChunk::HexCellPtr> collectAvailableCells(HexChunk* chunk);

    /**
     * @brief World containing spawned totems.
     */
    World* _world = nullptr;

    /**
     * @brief Asset manager used for prefab loading.
     */
    dzemikk::AssetManager* _assetManager = nullptr;

    /**
     * @brief Owning game instance.
     */
    Game* _game = nullptr;

    /**
     * @brief Random number generator used during spawning.
     */
    std::mt19937 _rng;

    /**
     * @brief Spawn configurations assigned per chunk.
     */
    std::unordered_map<boost::uuids::uuid, std::vector<TotemSpawnConfig>> _spawnRules;
};

} // namespace game

#endif