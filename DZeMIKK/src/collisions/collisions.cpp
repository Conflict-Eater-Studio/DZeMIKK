#include "collisions/collisions.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/transform.h"
#include "ecs/componentRegistry.h"
#include "renderer/model.h"
#include "renderer/mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <limits>

namespace dzemikk {

void Collisions::initialize() {
    // Intentionally empty for now
}

void Collisions::uninitialize() {
    // Intentionally empty for now
}

Collider* Collisions::raycast(const Camera* camera, const glm::vec2& screenPos, float screenWidth, float screenHeight) {
    if (!camera) return nullptr;

    // Convert screen pos to NDC
    float x = (2.0f * screenPos.x) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y) / screenHeight; // Y is bottom-to-top in NDC
    
    glm::vec4 rayStartNDC(x, y, -1.0f, 1.0f);
    glm::vec4 rayEndNDC(x, y, 0.0f, 1.0f);

    glm::mat4 invVP = glm::inverse(camera->getViewProjection());

    glm::vec4 rayStartWorld = invVP * rayStartNDC;
    rayStartWorld /= rayStartWorld.w;
    
    glm::vec4 rayEndWorld = invVP * rayEndNDC;
    rayEndWorld /= rayEndWorld.w;

    glm::vec3 rayDir = glm::normalize(glm::vec3(rayEndWorld) - glm::vec3(rayStartWorld));
    glm::vec3 rayOrigin = glm::vec3(rayStartWorld);

    static std::vector<Collider*> colliders;
    ComponentRegistry::get().getComponents<Collider>(colliders);

    Collider* closestHit = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    for (auto* c : colliders) {
        if (!c->isValid()) continue;

        const glm::mat4& worldMat = c->getTransform()->getWorldMatrix();
        glm::vec3 pos(worldMat[3]);
        float radius = c->getCullingRadius();
        
        glm::vec3 L = pos - rayOrigin;
        float sqDist = glm::dot(L, L);
        float radiusSq = radius * radius;
        
        float tca = glm::dot(L, rayDir);
        if (tca < 0.0f && sqDist > radiusSq) continue;
        
        float d2 = sqDist - tca * tca;
        if (d2 > radiusSq) continue;

        glm::mat4 invModel = glm::inverse(worldMat);
        glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
        glm::vec3 localRayDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));

        Model* model = c->getModel();
        if (!model) continue;

        bool hit = false;
        float closestHitModel = std::numeric_limits<float>::max();

        for (const auto& subMesh : model->getSubMeshes()) {
            if (!subMesh.mesh) continue;

            glm::vec3 minBox = subMesh.mesh->getBoundsMin();
            glm::vec3 maxBox = subMesh.mesh->getBoundsMax();
            
            float t = 0.0f;
            if (intersectRayAABB(localRayOrigin, localRayDir, minBox, maxBox, t)) {
                if (t >= 0.0f && t < closestHitModel) {
                    closestHitModel = t;
                    hit = true;
                }
            }
        }

        if (hit) {
            glm::vec3 hitLocal = localRayOrigin + localRayDir * closestHitModel;
            glm::vec3 hitWorld = glm::vec3(worldMat * glm::vec4(hitLocal, 1.0f));
            float dist = glm::distance(rayOrigin, hitWorld);

            if (dist < closestDist) {
                closestDist = dist;
                closestHit = c;
            }
        }
    }

    return closestHit;
}

bool Collisions::intersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& minBox, const glm::vec3& maxBox, float& tOut) const {
    glm::vec3 invDir = 1.0f / rayDir;
    
    glm::vec3 t0 = (minBox - rayOrigin) * invDir;
    glm::vec3 t1 = (maxBox - rayOrigin) * invDir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (tNear > tFar || tFar < 0.0f) {
        return false;
    }

    tOut = tNear;
    return true;
}

} // namespace dzemikk
