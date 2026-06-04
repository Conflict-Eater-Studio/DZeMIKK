#ifndef DZEMIKK_CAMERA_CONTROLLER_H
#define DZEMIKK_CAMERA_CONTROLLER_H

#include "ecs/components/monobehaviour.h"

#include <glm/glm.hpp>

namespace dzemikk {
class Transform;
}

namespace game {

class CameraController : public dzemikk::MonoBehaviour {
  public:
    enum class Mode { Exploration, Combat };

    void start() override;
    void update(double dt) override {};
    void lateUpdate(double dt) override;
    void fixedUpdate(double dt) override {};
    void onDestroy() override {};

    void setPlayerTransform(dzemikk::Transform* playerTransform);
    void setMode(Mode mode);
    void setFollowSpeed(float followSpeed);
    void setOffsetExplorationMode(glm::vec3 offset);
    void setOffsetCombatMode(glm::vec3 offset);

    [[nodiscard]] std::string typeName() const override {
        return "CameraController";
    }

  private:
    void updateExploration(double dt);
    void updateCombat(double dt);

  private:
    dzemikk::Transform* _playerTransform = nullptr;
    Mode _mode = Mode::Exploration;
    float _followSpeed = 2.0F;
    glm::vec3 _offsetExplorationMode = glm::vec3(-25.0F, 15.0F, 10.0F);
    glm::vec3 _offsetCombatMode = glm::vec3(0.0f, 20.0f, 5.0f);
};

} // namespace game

#endif