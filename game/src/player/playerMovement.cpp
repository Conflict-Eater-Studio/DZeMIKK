#include "player/playerMovement.h"

#include "ecs/components/animator.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "game.h"
#include "gameStateMachine.h"
#include "player/playerPatternComponent.h"
#include "scripts/world/world.h"
#include "stateMachine/combatState.h"

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "animation/animationclip.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"

#include <algorithm>
#include <assetManager/assetHandle.h>
#include <assetManager/assetmanager.h>
#include <assetManager/soundHandler.h>
#include <audio/audioManager.h>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace walkSound {

struct SoundInitContext {
    dzemikk::AudioManager* audioManager;
};

void onMoveSFXLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX, false);
}
} // namespace walkSound

namespace game {

void PlayerMovement::start() {}

void PlayerMovement::update(double deltaTime) {
    if (!_animator)
        return;
    if (_path.empty()) {
        _animator->setInt("isMoving", 0);
        return;
    }

    updateCellLerps(deltaTime);

    if (_step >= _path.size()) {
        _animator->setInt("isMoving", 0);
        HexGrid::HexCellPtr currentCell = _path[_step - 1];

        return;
    }

    HexGrid::HexCellPtr currentTargetCell = _path[_step % _path.size()];

    if (currentTargetCell->getState() == HexCell::State::Item) {
        spdlog::info("[PlayerMovement] Player stepped on item");
        if (_animator->getCurrentState()->getName() != "Pickup") {
            _animator->play("Pickup");
        }
    };

    if (_animator->getCurrentState()->getName() == "Pickup" &&
        !_animator->getCurrentState()->getClip()->isFinished()) {
        return;
    }

    _animator->setInt("isMoving", 1);

    if (isFallingFinished) {
        lerpCellTo(currentTargetCell, _playerEntity->getCell()->getHeight(),
                   [this](float progress) { isFallingFinished = false; });
    }

    dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*currentTargetCell);

    if (_animator->getCurrentState()->getClip()->isFinished()) {
        if (cellTransform) {
            lerpCellTo(currentTargetCell, currentTargetCell->getHeight(),
                       [this](float progress) { isFallingFinished = true; });

            if (!_cachedPath.empty()) {
                _path = _cachedPath;
                _cachedPath.clear();
                _step = 1;
            }
        }

        walkSound::SoundInitContext sCtx{_game->getEngine()->getAudioManager()};
        dzemikk::AssetManager::AssetTask<dzemikk::Sound, walkSound::SoundInitContext> taskS;
        taskS.context = sCtx;
        taskS.onLoad = walkSound::onMoveSFXLoad;
        _game->getEngine()->getAssetManager()->getAsync("audio/chod_v1_krotki.wav", taskS);

        rotateToDirection(_playerDir);
        _playerEntity->tryMove(currentTargetCell);
        _animator->setInt("direction", -1);
        _animator->play("Idle");
        _step++;
    }

    if (_step < _path.size()) {
        auto dir =
            HexCoord::dir(currentTargetCell->getCoord() - _playerEntity->getCell()->getCoord());
        if (dir.has_value()) {
            int hexDir = static_cast<int>(dir.value());
            int relativeDir = 3;
            if (hexDir == _playerDir) {
                _animator->setInt("direction", relativeDir);
            } else {
                int offset = hexDir - _playerDir;
                int anim = (relativeDir + offset + 12) % 12;
                _animator->setInt("direction", anim);
            }

            _playerDir = hexDir;
        }
    }

    if (auto cell = _playerEntity->getCell()) {
        if (cell->getType() == HexCell::Type::EnemyBattleHex && _game) {
            _game->getStateMachine()->setState(std::make_unique<game::CombatState>(_game));
        }
    }
}
void PlayerMovement::setSpeed(float speed) {
    _lerpSpeed = speed;
}
float PlayerMovement::getSpeed() const {
    return _lerpSpeed;
}
void PlayerMovement::setPlayerEntity(PlayerEntity* playerEntity) {
    _playerEntity = playerEntity;
}
void PlayerMovement::setHexGrid(HexGrid* hexGrid) {
    _hexGrid = hexGrid;
}
void PlayerMovement::moveTo(HexGrid::HexCellPtr cell) {
    std::vector<HexGrid::HexCellPtr> path = findPath(_playerEntity->getCell(), cell);
    // if (!_animator->getCurrentState()->getClip()->isFinished()){
    //     _cachedPath = path;
    // }
    if (_animator->getCurrentState()->getName() == "Idle") {
        _path = path;
        _step = 1;
    };
}
void PlayerMovement::setGame(Game* game) {
    _game = game;
}

void PlayerMovement::stopMovement() {
    _path.clear();
    _step = 0;
}

void PlayerMovement::setAnimator(dzemikk::Animator* animator) {
    _animator = animator;
}

dzemikk::Animator* PlayerMovement::getAnimator() const {
    return _animator;
}
void PlayerMovement::setWorld(World* world) {
    _world = world;
}
void PlayerMovement::lerpCellTo(const HexGrid::HexCellPtr& cell, float targetY,
                                LerpCallback callback) {
    for (auto& lerp : _cellLerps) {
        if (lerp.cell == cell) {
            if (lerp.targetY == targetY) {
                return;
            }
            float t = std::min(lerp.progress, 1.0f);
            lerp.targetY = targetY;
            lerp.progress = t;
            lerp.onComplete = callback;
            return;
        }
    }

    dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*cell);
    if (!cellTransform) {
        return;
    }
    _cellLerps.push_back({cell, cellTransform->getPosition().y, targetY, 0.0f, callback});
}

void PlayerMovement::updateCellLerps(double deltaTime) {
    for (auto it = _cellLerps.begin(); it != _cellLerps.end();) {
        it->progress += _lerpSpeed;

        float t = std::min(it->progress, 1.0f);

        dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*it->cell);
        if (cellTransform) {
            glm::vec3 pos = cellTransform->getPosition();
            pos.y = it->startY + (it->targetY - it->startY) * t;
            cellTransform->setPosition(pos);
        }
        if (it->progress >= 1.0f) {
            LerpCallback callback = it->onComplete;
            it = _cellLerps.erase(it);
            callback(1.0f);
        } else {
            ++it;
        }
    }
}
void PlayerMovement::rotateToDirection(int direction) {
    float angle = directionToAngle(direction);
    _playerEntity->getOwner()->transform()->setRotation(
        glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)));
}

float PlayerMovement::directionToAngle(int direction) {
    float angle = -direction * 30.0f;
    return angle;
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
            if (neighborCell->getState() == HexCell::State::Prop || neighborCell->getVisualState() == HexCell::VisualState::Signpost) {
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
