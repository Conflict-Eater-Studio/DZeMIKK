#include "player/playerMovement.h"

#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "game.h"
#include "gameStateMachine.h"
#include "player/playerPatternComponent.h"
#include "scripts/world/world.h"
#include "stateMachine/combatState.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace game {

void PlayerMovement::start() {
    MonoBehaviour::start();
}

void PlayerMovement::update(double deltaTime) {
    MonoBehaviour::update(deltaTime);
    if (_path.empty() || _step >= _path.size()) {
        return;
    }

    _duration += static_cast<float>(deltaTime);

    if (_duration > _speed && _step < _path.size()) {
        HexGrid::HexCellPtr ptr = _path[_step % _path.size()];
        _playerEntity->tryMove(ptr);
        _duration = 0.0F;
        _step++;
    }

    if (auto cell = _playerEntity->getCell()) {
        if (cell->getType() == HexCell::Type::EnemyBattleHex && _game) {
            _game->getStateMachine()->setState(std::make_unique<game::CombatState>(_game));
        }
    }
}
void PlayerMovement::setSpeed(float speed) {
    _speed = speed;
}
float PlayerMovement::getSpeed() const {
    return _speed;
}
void PlayerMovement::setPlayerEntity(PlayerEntity* playerEntity) {
    _playerEntity = playerEntity;
}
void PlayerMovement::setHexGrid(HexGrid* hexGrid) {
    _hexGrid = hexGrid;
}
void PlayerMovement::moveTo(HexGrid::HexCellPtr cell) {
    _path = findPath(_playerEntity->getCell(), cell);
    _step = 1;
}
void PlayerMovement::setGame(Game* game) {
    _game = game;
}

void PlayerMovement::stopMovement() {
    _path.clear();
    _step = 0;
    _duration = 0.0;
}

bool PlayerMovement::isWalkableCell(const HexGrid::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() != HexCell::GenState::Blocked &&
           cell->getType() != HexCell::Type::BlockingPattern &&
           cell->getType() != HexCell::Type::BlockingBridge;
}

void PlayerMovement::tryUnlockBlockingPattern(const HexGrid::HexCellPtr& cell) {
    if (!cell || cell->getType() != HexCell::Type::BlockingPattern) {
        return;
    }

    const auto* blockingInfo = _hexGrid->findBlockingPatternByCoord(cell->getCoord());
    if (!blockingInfo || blockingInfo->unlocked) {
        return;
    }

    auto* playerGO = getOwner()->getScene()->findGameObjectByTag("Player");
    if (!playerGO) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[PlayerMovement] Player GameObject not found in scene.");
#endif
        return;
    }

    auto* patternComponent = playerGO->getComponent<PlayerPatternComponent>();
    if (!patternComponent) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[PlayerMovement] PlayerPatternComponent not found on Player GameObject.");
#endif
        return;
    }

    if (patternComponent->hasPattern(blockingInfo->pattern)) {
        _hexGrid->unlockChunk(blockingInfo->blockedChunkId);
    }
}

std::vector<HexGrid::HexCellPtr> PlayerMovement::findPath(const HexGrid::HexCellPtr& startCell,
                                                          const HexGrid::HexCellPtr& targetCell) {
    if (startCell == nullptr || targetCell == nullptr) {
        return {};
    }

    const HexCoord start = startCell->getCoord();
    const HexCoord target = targetCell->getCoord();

    if (!isWalkableCell(startCell) || !isWalkableCell(targetCell)) {
        return {};
    }

    struct PathNode {
        HexCoord coord;
        int fScore;

        bool operator<(const PathNode& other) const {
            return fScore > other.fScore;
        }
    };

    std::priority_queue<PathNode> frontier;
    frontier.push({start, HexCoord::distance(start, target)});
    std::unordered_map<HexCoord, HexCoord> cameFrom;

    std::unordered_map<HexCoord, int> gScore;
    std::unordered_set<HexCoord> closedSet;

    gScore[start] = 0;

    while (!frontier.empty()) {
        const HexCoord current = frontier.top().coord;
        frontier.pop();

        if (closedSet.contains(current)) {
            continue;
        }

        if (current == target) {
            std::vector<HexGrid::HexCellPtr> path;
            HexCoord pathCoord = target;

            while (pathCoord != start) {
                auto cell = _hexGrid->getCell(pathCoord);
                if (cell == nullptr) {
                    return {};
                }

                path.push_back(cell);
                pathCoord = cameFrom.at(pathCoord);
            }

            path.push_back(startCell);
            std::ranges::reverse(path.begin(), path.end());
            return path;
        }

        closedSet.insert(current);

        for (const auto& neighbor : HexCoord::getNeighbors(current)) {
            if (closedSet.contains(neighbor)) {
                continue;
            }

            auto neighborCell = _hexGrid->getCell(neighbor);

            if (neighborCell && neighborCell->getType() == HexCell::Type::BlockingPattern) {
                tryUnlockBlockingPattern(neighborCell);
                neighborCell = _hexGrid->getCell(neighbor);
            }

            if (!isWalkableCell(neighborCell)) {
                continue;
            }
            if (targetCell->getType() != HexCell::Type::EnemyBattleHex) {
                if (neighborCell->getType() == HexCell::Type::EnemyBattleHex) {
                    continue;
                }
            }
            if (neighborCell->getState() == HexCell::State::Prop) {
                continue;
            }
            if (neighborCell->getEntity() != nullptr && neighbor != target) {
                continue;
            }

            const int neighbourGScore = gScore.at(current) + 1;
            auto neighborGScoreIt = gScore.find(neighbor);

            if (neighborGScoreIt != gScore.end() && neighbourGScore >= neighborGScoreIt->second) {
                continue;
            }

            cameFrom[neighbor] = current;
            gScore[neighbor] = neighbourGScore;

            const int fScore = neighbourGScore + HexCoord::distance(neighbor, target);
            frontier.push({neighbor, fScore});
        }
    }

    return {};
}

} // namespace game
