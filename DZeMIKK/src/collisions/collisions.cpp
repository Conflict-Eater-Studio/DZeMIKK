#include "collisions/collisions.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/transform.h"
#include "ecs/componentRegistry.h"
#include "ecs/gameobject.h"
#include "renderer/model.h"
#include "renderer/mesh.h"
#include "scene/octree.h"
#include "core/engine.h"
#include "core/window.h"
#include "input/input.h"
#include "renderer/renderer.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "ecs/components/ui/iUIInteractable.h"
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <limits>

namespace dzemikk {

void Collisions::initialize() {
    // Intentionally empty for now
}

void Collisions::uninitialize() {
    // Intentionally empty for now
}

void Collisions::update(Engine* engine, float deltaTime) {
    (void)deltaTime;
    
    Collider* previousHovered = _hoveredCollider;
    _hoveredCollider = nullptr;

    if (!engine) return;

    bool isMouseOverUI = false;
    std::vector<IUIInteractable*> uiElements;
    ComponentRegistry::get().getComponents<IUIInteractable>(uiElements);
    for (auto* element : uiElements) {
        if (element->isHovered()) {
            isMouseOverUI = true;
            break;
        }
    }

    if (!isMouseOverUI && engine->getRenderer()->getActiveSceneCamera()) {
        glm::vec2 mousePos = engine->getInput()->GetMousePosition();
        int width, height;
        glfwGetWindowSize(engine->getWindow()->nativeHandle(), &width, &height);

        auto activeScene = engine->getSceneManager()->getActiveScene();
        _hoveredCollider = raycast(
            engine->getRenderer()->getActiveSceneCamera(),
            activeScene ? activeScene->getOctree() : nullptr,
            mousePos,
            static_cast<float>(width),
            static_cast<float>(height)
        );
    }

    if (previousHovered != _hoveredCollider) {
        if (previousHovered && previousHovered->onMouseExit) {
            previousHovered->onMouseExit();
        }
        if (_hoveredCollider && _hoveredCollider->onMouseEnter) {
            _hoveredCollider->onMouseEnter();
        }
    }

    bool isLeftDown = engine->getInput()->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    bool pressedThisFrame = isLeftDown && !_wasLeftMouseDown;

    if (pressedThisFrame && _hoveredCollider) {
        OnColliderClicked.Invoke(_hoveredCollider);
    }

    _wasLeftMouseDown = isLeftDown;
}

Collider* Collisions::raycast(const Camera* camera, const Octree* octree, const glm::vec2& screenPos, float screenWidth, float screenHeight) {
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

    std::vector<Collider*> colliders;
    if (octree) {
        static uint32_t s_RaycastQueryId = 0;
        s_RaycastQueryId++;

        static std::vector<GameObject*> hitGOs;
        hitGOs.clear();
        octree->queryRay(rayOrigin, rayDir, hitGOs);
        
        for (auto* go : hitGOs) {
            // Remove duplicates using the query ID instead of sort and unique
            if (go->getLastRaycastQueryId() == s_RaycastQueryId) {
                continue;
            }
            go->setLastRaycastQueryId(s_RaycastQueryId);

            for (const auto& comp : go->getAllComponents()) {
                if (typeid(*comp) == typeid(Collider)) {
                    colliders.push_back(static_cast<Collider*>(comp.get()));
                    break;
                }
            }
        }
    } else {
        ComponentRegistry::get().getComponents<Collider>(colliders);
    }

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

        glm::vec3 minBox = c->getBoundsMin();
        glm::vec3 maxBox = c->getBoundsMax();
        
        float t = 0.0f;
        if (intersectRayAABB(localRayOrigin, localRayDir, minBox, maxBox, t)) {
            if (t >= 0.0f && t < closestHitModel) {
                closestHitModel = t;
                hit = true;
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
