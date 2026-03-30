#ifndef DZEMIKK_RENDERER_H
#define DZEMIKK_RENDERER_H

#include "core/iEngineModule.h"
#include "ecs/components/transform.h"
#include "ecs/components/camera.h"
#include "frustum.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>   

namespace dzemikk {

    class MeshRenderer;
    class SpriteRenderer;
    class Mesh;
    class Material;

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

        void render();

    private:
        struct Batch {
            Mesh* mesh;
            Material* material;
            std::vector<glm::mat4> models;
            GLuint instanceVBO = 0;
            int instanceCapacity = 0;
        };

        std::vector<Batch> _batches;

        struct pair_hash {
            template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2>& p) const {
                return std::hash<T1>{}(p.first) ^ (std::hash<T2>{}(p.second) << 1);
            }
        };

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
        unsigned int _uboMatrices;
        Frustum _frustum;
    };

}  // namespace dzemikk
#endif // DZEMIKK_RENDERER_H