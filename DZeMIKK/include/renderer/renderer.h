#pragma once
#include "core/iEngineModule.h"
#include "ecs/components/transform.h"
#include "ecs/components/camera.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>   

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

        void registerCamera(const Camera* camera);
        void unregisterCamera(const Camera* camera);

        void setActiveSceneCamera(const Camera* camera);
        void setActiveUICamera(const Camera* camera);

        void setActiveSceneCameraById(int cameraId);
        void setActiveUICameraById(int cameraId);

        const Camera* getActiveSceneCamera() const;
        const Camera* getActiveUICamera() const;

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

        std::vector<const Camera*> _cameras; 
        const Camera* _sceneCamera;        
        const Camera* _uiCamera;           
    };

} 