#ifndef DZEMIKK_SPRITE_RENDERER_H
#define DZEMIKK_SPRITE_RENDERER_H

#include "../component.h"
#include "assetManager/assetHandle.h"

#include <glm/glm.hpp>

namespace dzemikk {

class Transform;
class Mesh;
class Material;
class Texture;

/**
 * @brief Component responsible for rendering 2D sprites.
 *
 * SpriteRenderer connects a mesh (usually a quad), material (shader),
 * and transform to render 2D objects in the scene.
 */
class SpriteRenderer : public Component {
  public:
    using Base = Component;

    /**
     * @brief Constructs an empty SpriteRenderer component.
     */
    SpriteRenderer() = default;
    ~SpriteRenderer() = default;

#pragma region Disable copy and move

    SpriteRenderer(const SpriteRenderer& other) = delete;
    SpriteRenderer(SpriteRenderer&& other) noexcept = delete;
    SpriteRenderer& operator=(const SpriteRenderer& other) = delete;
    SpriteRenderer& operator=(SpriteRenderer&& other) noexcept = delete;

#pragma endregion

#pragma region Getters

    /**
     * @brief Returns the mesh used for rendering (usually a quad).
     *
     * @return Mesh* Pointer to the mesh.
     */
    [[nodiscard]] Mesh* getMesh() const {
        return _mesh;
    }

    /**
     * @brief Returns the material used for rendering.
     *
     * @return Material* Pointer to the material.
     */
    [[nodiscard]] Material* getMaterial() const {
        return _material;
    }

    /**
     * @brief Returns the transform associated with this sprite.
     *
     * @return Transform* Pointer to the transform.
     */
    [[nodiscard]] Transform* getTransform() const {
        return _transform;
    }

    [[nodiscard]] Texture* getTexture() const {
        return _texture.get();
    }
    [[nodiscard]] AssetHandle<Texture> getTextureHandle() const {
        return _texture;
    }
    glm::vec4 getColor() const {
        return _color;
    }

#pragma endregion

#pragma region Setters

    /**
     * @brief Sets the mesh used for rendering.
     *
     * @param mesh Pointer to the mesh.
     */
    void setMesh(Mesh* mesh) {
        _mesh = mesh;
    }

    /**
     * @brief Sets the material used for rendering.
     *
     * @param material Pointer to the material.
     */
    void setMaterial(Material* material) {
        _material = material;
    }

    /**
     * @brief Sets the transform associated with this sprite.
     *
     * @param transform Pointer to the transform.
     */
    void setTransform(Transform* transform) {
        _transform = transform;
    }

    /**
     * @brief Sets sprite texture (albedo).
     */
    void setTexture(AssetHandle<Texture> texture) {
        _texture = texture;
    }

    void setColor(const glm::vec4& color) {
        _color = color;
    }

    bool useTexture() {
        return _useTexture;
    }

    void setUseTexture(bool useTexture) {
        _useTexture = useTexture;
    }

#pragma endregion

#pragma region State

    /**
     * @brief Checks if the renderer has all required data to render.
     *
     * @return true If mesh, material, and transform are set.
     * @return false Otherwise.
     */
    [[nodiscard]] bool isValid() const {
        return _mesh && _material && _transform;
    }

    /**
     * @brief Checks if sprite has texture.
     */
    [[nodiscard]] bool hasTexture() const {
        return _texture.get() != nullptr;
    }

#pragma endregion

    [[nodiscard]] std::string typeName() const override {
        return "SpriteRenderer";
    };

  private:
#pragma region References

    /**
     * @brief Non-owning pointer to mesh resource (usually a quad).
     */
    Mesh* _mesh;

    /**
     * @brief Non-owning pointer to material resource.
     */
    Material* _material;

    /**
     * @brief Non-owning pointer to transform component.
     */
    Transform* _transform = nullptr;

    bool _useTexture = false;

    AssetHandle<Texture> _texture;
    glm::vec4 _color = glm::vec4(1.0f);

#pragma endregion
};

} // namespace dzemikk

#endif // DZEMIKK_SPRITE_RENDERER_H