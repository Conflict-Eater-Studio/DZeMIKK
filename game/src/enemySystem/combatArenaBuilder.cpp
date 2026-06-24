#include "enemySystem/combatArenaBuilder.h"

#include "enemySystem/enemyEntity.h"
#include "map/HexCell.h"
#include "map/HexCoord.h"
#include "map/HexGrid.h"
#include "map/PlayerEntity.h"
#include "map/Entity.h"
#include "scripts/world/world.h"
#include <ecs/components/meshRenderer.h>

CombatArenaResult CombatArenaBuilder::build(game::EnemyEntity* enemy, game::PlayerEntity* player,
                                            game::HexGrid* grid, game::World* world) {
    CombatArenaResult result;

    if (!enemy || !player || !grid || !world) {
        return result;
    }

    auto playerT = world->getHexTransformByCell(*player->getCell().get());
    for (auto g : playerT->getOwner()->getChildren()) {
        g->enabled(false);
    }

    const auto& territorySet = enemy->getTerritory();

    if (territorySet.empty()) {
        return result;
    }

    std::vector<game::HexCell*> territory(territorySet.begin(), territorySet.end());

    auto copiedCoords = copyTerritory(territory);

    copiedCoords = shiftTerritory(copiedCoords);

    auto* chunk = getAnyChunk(world);

    if (!chunk) {
        return result;
    }

    for (const auto& coord : copiedCoords) {

        auto cell =
            ensureCellExists(coord, grid, world, chunk, game::HexCell::Type::PlayerBattleHex);

        if (!cell) {
            continue;
        }

        cell->setType(game::HexCell::Type::PlayerBattleHex);
        cell->setDirty(true);

        auto transform = world->getHexTransformByCell(*cell.get());
        auto renderer = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();
        renderer->setMaterial(0, world->getHexMaterials()[game::HexCell::Type::PlayerBattleHex]);
        renderer->getMaterial(0)->setAlbedoColor({1.0F, 1.0F, 0.0F});

        for (auto g : transform->getOwner()->getChildren()) {
            g->enabled(false);
        }

        player->addTerritoryCell(cell.get());

        result.createdCells.push_back(cell.get());
    }

    auto centerCoord = calculateCenter(copiedCoords);

    result.centerCell =
        ensureCellExists(centerCoord, grid, world, chunk, game::HexCell::Type::Normal);

    if (result.centerCell) {
        result.centerCell.get()->setDirty(true);
    }

    return result;
}

std::vector<game::HexCoord>
CombatArenaBuilder::copyTerritory(const std::vector<game::HexCell*>& territory) {

    std::vector<game::HexCoord> coords;

    coords.reserve(territory.size());

    for (auto* cell : territory) {

        if (cell) {
            coords.push_back(cell->getCoord());
        }
    }

    return coords;
}

std::vector<game::HexCoord>
CombatArenaBuilder::shiftTerritory(const std::vector<game::HexCoord>& coords) {

    auto shifted = coords;

    std::unordered_set<game::HexCoord> blocked;

    for (const auto& coord : coords) {

        blocked.insert(coord);

        for (const auto& n : game::HexCoord::getNeighbors(coord)) {
            blocked.insert(n);
        }
    }

    game::HexCoord shiftLeft(-1, 1);

    bool overlaps = true;

    while (overlaps) {

        overlaps = false;

        for (const auto& coord : shifted) {

            if (blocked.contains(coord)) {
                overlaps = true;
                break;
            }
        }

        if (overlaps) {

            for (auto& coord : shifted) {
                coord += shiftLeft;
            }
        }
    }

    return shifted;
}

game::Entity::HexCellPtr CombatArenaBuilder::ensureCellExists(const game::HexCoord& coord,
                                                    game::HexGrid* grid, game::World* world,
                                                    game::HexChunk* chunk,
                                                    game::HexCell::Type type) {

    auto cell = grid->getCell(coord);

    bool hasVisual = world->hasHexVisual(coord);

    if (!cell) {
        auto newCell = std::make_shared<game::HexCell>(
            coord, game::HexCell::State(game::HexCell::State::Empty), game::HexCell::Type(type),
            game::HexCell::GenState::Normal);
        chunk->insertCell(coord, newCell);
        world->ensureHexExists(newCell);
        return newCell;
    }

    if (!hasVisual) {
        world->ensureHexExists(cell);
    }

    return cell;
}

game::HexCoord CombatArenaBuilder::calculateCenter(const std::vector<game::HexCoord>& coords) {

    int sumQ = 0;
    int sumR = 0;

    for (const auto& coord : coords) {

        sumQ += coord.q();
        sumR += coord.r();
    }

    return {sumQ / static_cast<int>(coords.size()), sumR / static_cast<int>(coords.size())};
}

game::HexChunk* CombatArenaBuilder::getAnyChunk(game::World* world) {

    const auto& chunks = world->getGrid()->getChunks();

    if (chunks.empty()) {
        return nullptr;
    }

    return chunks.begin()->second.get();
}