#ifndef DZEMIKK_COLLISIONS_H
#define DZEMIKK_COLLISIONS_H

#include "core/iEngineModule.h"
#include "input/input.h"
#include <glm/glm.hpp>
#include <vector>

namespace dzemikk {

    class Camera;
    class Collider;
    class Octree;

    class Engine;

    class Collisions : public IEngineModule {
    public:
        Collisions() = default;
        ~Collisions() override = default;

        void initialize() override;
        void uninitialize() override;
        void update(Engine* engine, float deltaTime);

        /**
         * @brief Raycasts against all active Colliders in the scene and returns the closest hit.
         * 
         * @param camera The camera generating the ray.
         * @param screenPos Mouse coordinates (origin top-left, x: right, y: down).
         * @param screenWidth Width of the screen/window.
         * @param screenHeight Height of the screen/window.
         * @return Collider* Pointer to the hit Collider, or nullptr if no hit.
         */
        Collider* raycast(const Camera* camera, const Octree* octree, const glm::vec2& screenPos, float screenWidth, float screenHeight);

        Collider* getHoveredCollider() const { return _hoveredCollider; }

        Action<Collider*> OnColliderClicked;

    private:
        bool intersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& minBox, const glm::vec3& maxBox, float& tOut) const;

        Collider* _hoveredCollider = nullptr;
        bool _wasLeftMouseDown = false;
    };

} // namespace dzemikk

#endif // DZEMIKK_COLLISIONS_H
