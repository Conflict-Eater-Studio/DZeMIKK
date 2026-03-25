#pragma once
#include "core/iEngineModule.h"
#include "ecs/components/transform.h"

#include <glm/glm.hpp>
#include <vector>

namespace dzemikk {

    class MeshRenderer;
    class SpriteRenderer;

    class Renderer : public IEngineModule {
      public:
        Renderer() = default;
        ~Renderer() = default;

        void Initialize() override;
        void UnInitialize() override;

        void registerRenderer(MeshRenderer* renderer);
        void unregisterRenderer(MeshRenderer* renderer);

        void registerSpriteRenderer(SpriteRenderer* renderer);
        void unregisterSpriteRenderer(SpriteRenderer* renderer);

        void setCamera(const glm::mat4& view, const glm::mat4& projection);
        void setUIProjection(const glm::mat4& ortho);

        void render();

      private:
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        std::vector<MeshRenderer*> _meshRenderers;
        std::vector<SpriteRenderer*> _spriteRenderers;

        glm::mat4 _view = glm::mat4(1.0f);
        glm::mat4 _projection = glm::mat4(1.0f);
        glm::mat4 _uiProjection = glm::mat4(1.0f);
    };

} 