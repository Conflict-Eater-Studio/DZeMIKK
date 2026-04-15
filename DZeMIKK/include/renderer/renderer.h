#ifndef DZEMIKK_RENDERER_H
#define DZEMIKK_RENDERER_H

#include "core/iEngineModule.h"
#include "frustum.h"
#include "skybox.h"

#include <memory>   

namespace dzemikk {

    class MeshRenderer;
    class SpriteRenderer;
    class Mesh;
    class Material;
    class Camera;
    
    /**
     * @brief The main rendering module.
     *
     * Handles rendering of 3D meshes and 2D sprites. Supports frustum culling,
     * batching, instanced rendering, and UI overlay rendering.
     */
    class Renderer : public IEngineModule {
    public:
        Renderer() = default;
        ~Renderer() = default;

        #pragma region Disable copy/move
        
        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept = delete;
        
        #pragma endregion


        #pragma region Initialization / Uninitialization

        /**
         * @brief Initializes renderer resources (UBO, OpenGL states).
         */
        void Initialize() override;

        /**
         * @brief Releases GPU resources and clears batches.
         */
        void UnInitialize() override;

        #pragma endregion

        #pragma region Camera Management

        /**
         * @brief Sets the active scene camera for 3D rendering.
         */
        void setActiveSceneCamera(Camera* camera);

        /**
         * @brief Sets the active UI camera for overlay rendering.
         */
        void setActiveUICamera(Camera* camera);

        /**
         * @brief Finds and sets the active scene camera by ID.
         */
        void setActiveSceneCameraById(int cameraId);

        /**
         * @brief Finds and sets the active UI camera by ID.
         */
        void setActiveUICameraById(int cameraId);

        /**
         * @brief Returns the currently active scene camera.
         */
        [[nodiscard]] const Camera* getActiveSceneCamera() const;

        /**
         * @brief Returns the currently active UI camera.
         */
        [[nodiscard]] const Camera* getActiveUICamera() const;

        #pragma endregion

        
        #pragma region Rendering

        /**
         * @brief Renders all 3D meshes and 2D sprites.
         *
         * Performs frustum culling, batching, instanced rendering, and
         * UI overlay rendering using the UI camera.
         */
        void render();
        
        #pragma endregion

        #pragma region Skybox Management

        /**
         * @brief Sets a custom skybox using a cubemap texture.
         */
        void setSkybox(Skybox* skybox);

        /**
         * @brief Returns the currently active skybox.
         */
        [[nodiscard]] const Skybox* getSkybox() const;

        #pragma endregion

    private:
        #pragma region Batch structure

        /**
         * @brief Represents a batch of meshes sharing the same material.
         *
         * Used for instanced rendering to minimize draw calls.
         */
        struct Batch {
            Mesh* mesh = nullptr;
            Material* material = nullptr;
            std::vector<glm::mat4> models; 
            GLuint instanceVBO = 0;          
        };

        #pragma endregion

        #pragma region Internal Data
        std::vector<Batch> _batches;

        std::vector<MeshRenderer*> _meshRenderers;
        std::vector<SpriteRenderer*> _spriteRenderers;

        glm::mat4 _view = glm::mat4(1.0f);
        glm::mat4 _projection = glm::mat4(1.0f);
        glm::mat4 _uiProjection = glm::mat4(1.0f);

        std::vector<Camera*> _cameras; 
        Camera* _sceneCamera;        
        Camera* _uiCamera;   

        unsigned int _uboMatrices;
        Frustum _frustum;

        Skybox* _skybox;

        Shader* _textShader = nullptr;
        GLuint textVAO = 0;
        GLuint textVBO = 0;

        #pragma endregion
    };

}  // namespace dzemikk
#endif // DZEMIKK_RENDERER_H