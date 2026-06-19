#include "camera/cameraController.h"

#include <ecs/components/transform.h>
#include <ecs/gameobject.h>

void game::CameraController::start() {
    _mode = Mode::Exploration;
}

void game::CameraController::lateUpdate(double dt) {
    switch (_mode) {
    case Mode::Exploration:
        updateExploration(dt);
        break;

    case Mode::Combat:
        updateCombat(dt);
        break;

    case Mode::Cinematic:
        updateCinematic(dt);
        break;
    }
}

void game::CameraController::setPlayerTransform(dzemikk::Transform* playerTransform) {
    _playerTransform = playerTransform;
}

void game::CameraController::setMode(Mode mode) {
    _mode = mode;

    if (_mode == Mode::Exploration || _mode == Mode::Cinematic) {
        getOwner()->transform()->setRotation(glm::vec3(glm::radians(-30.0F),
                                                       glm::radians(-90.0F), 
                                                       0.0F                  
                                                       ));
    }

    if (_mode == Mode::Combat) {
        getOwner()->transform()->setRotation(
            glm::vec3(glm::radians(-90.0F), glm::radians(-120.0F), 0.0F));
    }
}

void game::CameraController::setFollowSpeed(float followSpeed) {
    _followSpeed = followSpeed;
}

void game::CameraController::setOffsetExplorationMode(glm::vec3 offset) {
    _offsetExplorationMode = offset;
}

void game::CameraController::setOffsetCombatMode(glm::vec3 offset) {
    _offsetCombatMode = offset;
}

void game::CameraController::updateExploration(double dt) {
    if (!_playerTransform) {
        return;
    }

    auto* cameraTransform = getOwner()->transform();

    glm::vec3 playerPos = _playerTransform->getPosition();

    glm::vec3 targetPos = playerPos + _offsetExplorationMode;

    glm::vec3 currentPos = cameraTransform->getPosition();

    float t = (float)1.0F - std::exp(-_followSpeed * dt);
    glm::vec3 newPos = glm::mix(currentPos, targetPos, t);

    cameraTransform->setPosition(newPos);
}

void game::CameraController::updateCombat(double dt) {
    if (!_playerTransform) {
        return;
    }

    auto* cameraTransform = getOwner()->transform();

    glm::vec3 playerPos = _playerTransform->getPosition();

    glm::vec3 targetPos = playerPos + _offsetCombatMode;

    glm::vec3 currentPos = cameraTransform->getPosition();

    float t = (float)1.0F - std::exp(-_followSpeed * dt);
    glm::vec3 newPos = glm::mix(currentPos, targetPos, t);

    cameraTransform->setPosition(newPos);
}

void game::CameraController::updateCinematic(double dt) {
    if (!_playerTransform) {
        return;
    }

    auto* cameraTransform = getOwner()->transform();

    glm::vec3 playerPos = _playerTransform->getPosition();

    glm::vec3 targetPos = playerPos + _offsetExplorationMode;

    glm::vec3 currentPos = cameraTransform->getPosition();

    float t = (float)1.0F - std::exp(-_followSpeed * dt);
    glm::vec3 newPos = glm::mix(currentPos, targetPos, t);

    cameraTransform->setPosition(newPos);
}
