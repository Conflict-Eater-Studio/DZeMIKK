#ifndef DZEMIKK_MATERIAL_H
#define DZEMIKK_MATERIAL_H

#include "assetManager/assetHandle.h"
#include "renderer/texture.h"

namespace dzemikk {

    class Shader;

    /**
     * @brief Represents a material used for rendering.
     *
     * Material defines how an object is rendered by storing
     * a reference to a shader and (in the future)
     * rendering parameters like textures, colors, and uniforms.
     */
    class Material {
      public:
        /**
         * @brief Constructs an empty material.
         */
        Material() = default;
        ~Material() = default;

        #pragma region Disable copy and move

        Material(const Material& other) = delete;
        Material(Material&& other) noexcept = delete;
        Material& operator=(const Material& other) = delete;
        Material& operator=(Material&& other) noexcept = delete;

        #pragma endregion

        #pragma region Getters

        /**
         * @brief Returns the shader used by this material.
         *
         * @return Shader* Pointer to shader.
         */

        [[nodiscard]] Shader* getShader() const {
            return _shader.get();
        }
        [[nodiscard]] const AssetHandle<Shader>& getShaderHandle() const {
            return _shader;
        }
        #pragma endregion

        #pragma region Setters

        /**
         * @brief Sets the shader used by this material.
         *
         * @param shader Pointer to shader.
         */
        void setShader(AssetHandle<Shader> shader) {
            _shader = shader;
        }

        #pragma endregion

        #pragma region Validation

        /**
         * @brief Checks if the material is valid for rendering.
         *
         * @return true If shader is set.
         */
        [[nodiscard]] bool isValid() const {
            return _shader.get() != nullptr;
        }

        #pragma endregion

        [[nodiscard]] const AssetHandle<Texture>& getTextureHandle() const {
            return _texture;
        }

        [[nodiscard]] Texture* getTexture() const {
            return _texture.get();
        }

        void setTexture(AssetHandle<Texture> texture) {
            _texture = texture;
        }

    private:
        #pragma region Data

        /**
         * @brief Non-owning pointer to shader resource.
         */
        AssetHandle<Shader> _shader;

        AssetHandle<Texture> _texture;

        #pragma endregion
    };

} // namespace dzemikk

#endif // DZEMIKK_MATERIAL_H