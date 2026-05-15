#ifndef DZEMIKK_COLLIDER_H
#define DZEMIKK_COLLIDER_H

#include "../component.h"
#include <glm/glm.hpp>
#include <string>

namespace dzemikk {

class Transform;
class Model;

/**
 * @brief Component used to give a GameObject collision bounds.
 * By decoupling this from MeshRenderer, we drastically accelerate raycasting.
 */
class Collider : public Component {
public:
    using Base = Component;

    Collider() = default;
    ~Collider() override = default;

    Collider(const Collider&) = delete;
    Collider& operator=(const Collider&) = delete;
    Collider(Collider&&) noexcept = delete;
    Collider& operator=(Collider&&) noexcept = delete;

    void setModel(Model* model);
    Model* getModel() const;

    void setTransform(Transform* transform);
    Transform* getTransform() const;

    [[nodiscard]] bool isValid() const {
        return _model && _transform;
    }

    float getCullingRadius() const;

    [[nodiscard]] const glm::vec3& getBoundsMin() const { return _boundsMin; }
    [[nodiscard]] const glm::vec3& getBoundsMax() const { return _boundsMax; }

    [[nodiscard]] std::string typeName() const override {
        return "Collider";
    }

private:
    void calculateCullingRadius(Model* model);

    Model* _model = nullptr;
    Transform* _transform = nullptr;
    float _cullingRadius = 1.0f;
    glm::vec3 _boundsMin{0.0f};
    glm::vec3 _boundsMax{0.0f};
};

} // namespace dzemikk

#endif // DZEMIKK_COLLIDER_H
