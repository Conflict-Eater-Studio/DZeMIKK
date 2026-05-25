#pragma once
#include <vector>
#include <glm/ext/vector_float4.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer/renderPasses/IRenderPass.h"
#include "renderer/lightUtils.h"

#include "ecs/componentRegistry.h"
#include "ecs/gameobject.h"
#include "ecs/components/transform.h"
#include "ecs/components/light/directionalLight.h"
#include "ecs/components/light/pointLight.h"
#include "ecs/components/light/spotLight.h"

namespace dzemikk {
class LightSystem {
  public:
    std::vector<GPUDirectionalLight> _dir;
    std::vector<GPUPointLight> _point;
    std::vector<GPUSpotLight> _spot;

    std::vector<DirectionalLight*> _dirComponents;
    std::vector<PointLight*> _pointComponents;
    std::vector<SpotLight*> _spotComponents;

    static constexpr int MAX_DIR_LIGHTS = 8;
    static constexpr int MAX_POINT_LIGHTS = 16;
    static constexpr int MAX_SPOT_LIGHTS = 8;

    const GPUDirectionalLight* dirData() const {
        return _dir.data();
    }
    const GPUPointLight* pointData() const {
        return _point.data();
    }
    const GPUSpotLight* spotData() const {
        return _spot.data();
    }

    int dirCount() const {
        return (int)_dir.size();
    }

    int pointCount() const {
        return (int)_point.size();
    }

    int spotCount() const {
        return (int)_spot.size();
    }

    void update(RenderContext& ctx) {
        _dir.clear();
        _point.clear();
        _spot.clear();

        auto& reg = dzemikk::ComponentRegistry::get();

        reg.getEnabledComponents<DirectionalLight>(_dirComponents);

        int dcount = std::min((int)_dirComponents.size(), MAX_DIR_LIGHTS);

        for (int i = 0; i < dcount; i++) {
            _dir.push_back({glm::vec4(_dirComponents[i]->getDirection(), 0.0f),
                 glm::vec4(_dirComponents[i]->getColor(), _dirComponents[i]->getIntensity())});
        }

        reg.getEnabledComponents<PointLight>(_pointComponents);

        int pcount = std::min((int)_pointComponents.size(), MAX_POINT_LIGHTS);

        for (int i = 0; i < pcount; i++) {
            auto* t = _pointComponents[i]->getOwner()->transform();

            _point.push_back({glm::vec4(t->getPosition(), 1.0f),
                 glm::vec4(_pointComponents[i]->getColor(), _pointComponents[i]->getIntensity()),
                 glm::vec4(_pointComponents[i]->getRange(), 0, 0, 0)});
        }

        reg.getEnabledComponents<SpotLight>(_spotComponents);

        int scount = std::min((int)_spotComponents.size(), MAX_SPOT_LIGHTS);

        for (int i = 0; i < scount; i++) {
            auto* t = _spotComponents[i]->getOwner()->transform();

            _spot.push_back(
                {glm::vec4(t->getPosition(), 1.0f),
                 glm::vec4(_spotComponents[i]->getDirection(), 0.0f),
                 glm::vec4(_spotComponents[i]->getColor(), _spotComponents[i]->getIntensity()),
                 glm::vec4(_spotComponents[i]->getRange(), _spotComponents[i]->getInnerCutoff(),
                           _spotComponents[i]->getOuterCutoff(), 0.0f)});
        }

        ctx.directionalLights = _dir.data();
        ctx.pointLights = _point.data();
        ctx.spotLights = _spot.data();

        ctx.directionalCount = dcount;
        ctx.pointCount = pcount;
        ctx.spotCount = scount;
    }

    const std::vector<DirectionalLight*>& dirComponents() const {
        return _dirComponents;
    }

    const std::vector<PointLight*>& pointComponents() const {
        return _pointComponents;
    }

    const std::vector<SpotLight*>& spotComponents() const {
        return _spotComponents;
    }
};
} // namespace dzemikk
