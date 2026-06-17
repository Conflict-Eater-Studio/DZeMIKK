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
     * @brief Spawns a totem on a random cell on a given chunk.
     *
     * @param chunkId Chunk ID on which the totem will be spawned.
     * @param config Config for the totem.
     */
    void addTotem(const boost::uuids::uuid& chunkId, TotemSpawnConfig config);

    /**
     * @brief Spawns a totem on a specific cell from a given chunk.
     *
     * @param chunkId Chunk ID on which the totem will be spawned.
     * @param config Config for the totem.
     * @param coord Coord of the cell to place the totem.
     */
    void addTotem(const boost::uuids::uuid& chunkId, TotemSpawnConfig config,
                  const HexCoord& coord);

    [[nodiscard]] nlohmann::json saveState() const;
    void loadState(const nlohmann::json& j);
    void clear();
    void markTotemUsed(const boost::uuids::uuid& persistantId);
    void markTotemUnused(const boost::uuids::uuid& persistantId);

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemManager";
    }

  private:
    /**
     * @brief Creates a totem on the specified cell.
     *
     * @param chunkId Chunk ID the totem belongs to.
     * @param cell Target spawn cell.
     * @param cfg Totem configuration.
     */
    void spawnTotem(const boost::uuids::uuid& chunkId, const HexChunk::HexCellPtr& cell,
                    const TotemSpawnConfig& cfg);

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

    /**
     * @brief Spawned totem entities lookup by chunk.
     */
    std::unordered_map<boost::uuids::uuid, std::vector<TotemEntity*>> _spawnedTotems;
};

} // namespace game

#endif
