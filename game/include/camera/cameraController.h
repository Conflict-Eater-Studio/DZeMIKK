#ifndef DZEMIKK_CAMERA_CONTROLLER_H
#define DZEMIKK_CAMERA_CONTROLLER_H

#include "ecs/components/monobehaviour.h"

#include <glm/glm.hpp>

namespace dzemikk {
class Transform;
}

namespace game {

/**
 * @brief Controls camera movement and positioning relative to the player.
 *
 * Supports multiple camera modes and smoothly follows the target transform
 * using configurable offsets and follow speed.
 */
class CameraController : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    /**
     * @brief Available camera behavior modes.
     */
    enum class Mode { Exploration, Combat };

#pragma region Lifecycle

    void start() override;
    void update(double dt) override {};
    void lateUpdate(double dt) override;
    void fixedUpdate(double dt) override {};
    void onDestroy() override {};

#pragma endregion

#pragma region Configuration

    /**
     * @brief Sets the transform that the camera should follow.
     *
     * @param playerTransform Target player transform.
     */
    void setPlayerTransform(dzemikk::Transform* playerTransform);

    /**
     * @brief Changes the active camera mode.
     *
     * @param mode Desired camera mode.
     */
    void setMode(Mode mode);

    /**
     * @brief Sets the camera follow interpolation speed.
     *
     * @param followSpeed Follow speed factor.
     */
    void setFollowSpeed(float followSpeed);

    /**
     * @brief Sets the exploration mode camera offset.
     *
     * @param offset Offset relative to the player.
     */
    void setOffsetExplorationMode(glm::vec3 offset);

    /**
     * @brief Sets the combat mode camera offset.
     *
     * @param offset Offset relative to the player.
     */
    void setOffsetCombatMode(glm::vec3 offset);

#pragma endregion

    [[nodiscard]] std::string typeName() const override {
        return "CameraController";
    }

  private:
#pragma region Update methods

    /**
     * @brief Updates camera position in exploration mode.
     *
     * @param dt Delta time in seconds.
     */
    void updateExploration(double dt);

    /**
     * @brief Updates camera position in combat mode.
     *
     * @param dt Delta time in seconds.
     */
    void updateCombat(double dt);

#pragma endregion

#pragma region References

    dzemikk::Transform* _playerTransform = nullptr;

#pragma endregion

#pragma region State

    Mode _mode = Mode::Exploration;

#pragma endregion

#pragma region Configuration

    float _followSpeed = 2.0F;
    glm::vec3 _offsetExplorationMode = glm::vec3(-25.0F, 14.0F, 10.0F);
    glm::vec3 _offsetCombatMode = glm::vec3(-2.0f, 20.0f, 5.0f);

#pragma endregion

    bool _useModeRotation = true;

    glm::vec3 _manualRotation = glm::vec3(0.0f);
    float _manualRotationSpeed = 90.0f;
};

} // namespace game

#endif
