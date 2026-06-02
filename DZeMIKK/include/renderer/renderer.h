#ifndef DZEMIKK_RENDERER_H
#define DZEMIKK_RENDERER_H

#include "core/iEngineModule.h"
#include "core/engine.h"
#include "frustum.h"
#include "skybox.h"
#include <memory>   

#include "renderer/renderPasses/IRenderPass.h"
#include "renderer/cameraSystem.h"
#include "renderer/lightSystem.h"

#include "renderer/framebuffer.h"
#include "renderer/renderPasses/postProcessRenderPass.h"

namespace dzemikk {
    class Camera;
    
    /**
     * @brief The main rendering module.
     *
     * Central entry point for the rendering pipeline. Responsible for:
     * - managing render passes
     * - maintaining frame context (matrices, cameras, frustum)
     *
     * Does NOT directly render geometry — delegates work to IRenderPass instances.
     */
    class Renderer : public IEngineModule {
    public:
        Renderer(EngineMode engineMode, Engine* engine) : _engineMode(engineMode), _engine(engine) {};
        ~Renderer() = default;

#pragma region Disable copy/move
        
        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept = delete;
        
#pragma endregion

#pragma region Initialization / Uninitialization

        /**
         * @brief Initializes renderer resources.
         *
         * Creates GPU buffers (UBO), initializes render passes,
         * and sets up default OpenGL state.
         */
        void initialize() override;

        /**
         * @brief Releases GPU resources.
         *
         * Cleans up buffers and any persistent rendering data.
         */
        void uninitialize() override;

#pragma endregion

#pragma region Rendering

        /**
         * @brief Executes full rendering pipeline.
         *
         * Steps:
         * 1. Prepare frame (clear buffers, setup GL state)
         * 2. Update camera system
         * 3. Execute all registered render passes in order
         */
        void render();
        
#pragma endregion

#pragma region Skybox Management

        /**
         * @brief Sets a custom skybox using a cubemap texture.
         *
         * Delegates to SkyboxRenderPass if available.
         */
        void setSkybox(AssetHandle<Skybox> skybox);

        /**
         * @brief Returns the currently active skybox.
         *
         * Returns empty handle if no skybox pass exists.
         */
        [[nodiscard]] const AssetHandle<Skybox> getSkybox() const;

#pragma endregion

        /**
         * @brief Access to camera system.
         *
         * Used to control active cameras externally.
         */
        CameraSystem& getCameraSystem() {
            return _cameraSystem;
        }

        const CameraSystem& getCameraSystem() const {
            return _cameraSystem;
        }

        void setViewportSize(uint32_t width, uint32_t height);

        [[nodiscard]] Framebuffer* getSceneFramebuffer() const {
            return _sceneFramebuffer.get();
        }

    private:
#pragma region Internal Data
      /**
       * @brief Per-frame rendering context shared across all passes.
       *
       * Contains camera data, matrices, frustum, and GPU bindings.
       * Updated every frame before executing render passes.
       */
        RenderContext _context;

        
        /**
         * @brief Ordered list of render passes.
         *
         * Execution order defines the rendering pipeline.
         * Ownership is maintained via unique_ptr.
         */
        std::vector<std::unique_ptr<IRenderPass>> _passes;
        PostProcessRenderPass _postProcessingPass;

        /**
         * @brief Fast lookup map for render passes by type.
         *
         * Allows O(1) access to specific pass (e.g. SkyboxRenderPass).
         */
        std::unordered_map<std::type_index, IRenderPass*> _passMap;

        /**
         * @brief Uniform Buffer Object storing view/projection matrices.
         */
        unsigned int _uboMatrices = 0;

        /**
         * @brief View frustum used for culling.
         */
        Frustum _frustum;

        /**
         * @brief Manages active scene/UI cameras.
         */
        CameraSystem _cameraSystem;

        LightSystem _lightSystem;

        std::unique_ptr<Framebuffer> _sceneFramebuffer;

        uint32_t _viewportWidth = 1920;
        uint32_t _viewportHeight = 1080;

        EngineMode _engineMode = EngineMode::Game;

        AssetHandle<Shader> _presentShader;
        GLuint _fullscreenVAO = 0;

        Engine* _engine = nullptr;

        void initFullscreenQuad();

        /**
         * @brief Prepares frame before rendering.
         *
         * Clears buffers and sets default OpenGL state.
         */
        void setupFrame();

        /**
         * @brief Retrieves a render pass by type.
         *
         * @tparam T Render pass type
         * @return Pointer to pass or nullptr if not found
         */
        template <typename T> T* getPass();

        /**
         * @brief Const version of getPass.
         */
        template <typename T> const T* getPass() const;

        /**
         * @brief Adds a new render pass to the pipeline.
         *
         * @tparam T Render pass type
         * @param args Constructor arguments for the pass
         * @return Raw pointer to the created pass
         *
         * @note Ownership is kept internally (unique_ptr).
         * @warning Adding multiple passes of the same type will overwrite lookup map entry.
         */
        template <typename T, typename... Args> T* addPass(Args&&... args);

#pragma endregion
    };

// ================================== IMPLEMENTATION ==================================
template <typename T> T* Renderer::getPass() {
    auto it = _passMap.find(typeid(T));
    if (it != _passMap.end())
        return static_cast<T*>(it->second);
    return nullptr;
}

template <typename T> const T* Renderer::getPass() const {
    auto it = _passMap.find(typeid(T));
    if (it != _passMap.end())
        return static_cast<T*>(it->second);
    return nullptr;
}

template <typename T, typename... Args> T* Renderer::addPass(Args&&... args) {
    auto pass = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = pass.get();

    _passMap[typeid(T)] = ptr;
    _passes.push_back(std::move(pass));

    return ptr;
}

}  // namespace dzemikk
#endif // DZEMIKK_RENDERER_H