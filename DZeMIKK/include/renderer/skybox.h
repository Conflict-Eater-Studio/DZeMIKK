#ifndef DZEMIKK_SKYBOX_H
#define DZEMIKK_SKYBOX_H

#include "renderer/StaticMesh.h"
#include "renderer/shader.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace dzemikk {

    /**
     * @brief Represents a skybox used for rendering environment background.
     *
     * Supports rendering using either a solid color or a cubemap texture.
     */
    class Skybox {
    public:
        using MeshPtr = std::unique_ptr<StaticMesh>;

        /**
         * @brief Defines skybox rendering mode.
         */
        enum class Mode { Color, Cubemap };

        /**
         * @brief Constructs a skybox with default color mode.
         */
        Skybox();
        ~Skybox();

        #pragma region Disable copy

        Skybox(const Skybox&) = delete;
        Skybox& operator=(const Skybox&) = delete;

        #pragma endregion

        #pragma region Enable move

        Skybox(Skybox&&) noexcept = default;
        Skybox& operator=(Skybox&&) noexcept = default;

        #pragma endregion

        #pragma region Setup

        /**
         * @brief Loads a cubemap from 6 texture paths.
         *
         * @param faces Paths in order: +X, -X, +Y, -Y, +Z, -Z.
         */
        void loadCubemap(const std::vector<std::string>& faces);

        /**
         * @brief Sets solid color skybox.
         */
        void setColor(const glm::vec3& color);

        /**
         * @brief Sets rendering mode explicitly.
         */
        void setMode(Mode mode);

        #pragma endregion

        #pragma region Rendering

        /**
         * @brief Renders the skybox.
         *
         * @param view Camera view matrix.
         * @param projection Camera projection matrix.
         */
        void render(const glm::mat4& view, const glm::mat4& projection) const;

        #pragma endregion

        #pragma region Getters

        [[nodiscard]] Mode getMode() const {
            return _mode;
        }
        [[nodiscard]] const glm::vec3& getColor() const {
            return _color;
        }

        void setShader(Shader* shader);

        #pragma endregion

    private:
        Mode _mode = Mode::Color;

        MeshPtr _cubeMesh;
        Shader* _shader = nullptr;

        GLuint _cubemapTex = 0;
        glm::vec3 _color = glm::vec3(0.5f, 0.7f, 1.0f);

        #pragma region Initialization

        void initCube();

        #pragma endregion
    };

} // namespace dzemikk

#endif // DZEMIKK_SKYBOX_H