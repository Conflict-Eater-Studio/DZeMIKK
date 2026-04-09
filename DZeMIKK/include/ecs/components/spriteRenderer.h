#ifndef DZEMIKK_SPRITE_RENDERER_H
#define DZEMIKK_SPRITE_RENDERER_H

#include "../component.h"

#include <glm/glm.hpp>

namespace dzemikk {

class Transform;
class Mesh;
class Material;

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

    unsigned int getTexture() const {
        return _textureID;
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

    void setTexture(unsigned int texID) {
        _textureID = texID;
    }

    void setColor(const glm::vec4& color) {
        _color = color;
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

#pragma endregion

    [[nodiscard]] std::string typeName() const override {
        return "SpriteRenderer";
    };

  private:
#pragma region References

    /**
     * @brief Non-owning pointer to mesh resource (usually a quad).
     */
    Mesh* _mesh = nullptr;

    /**
     * @brief Non-owning pointer to material resource.
     */
    Material* _material = nullptr;

    /**
     * @brief Non-owning pointer to transform component.
     */
    Transform* _transform = nullptr;

    unsigned int _textureID = 0;
    glm::vec4 _color = glm::vec4(1.0f);

#pragma endregion
};

} // namespace dzemikk

#endif // DZEMIKK_SPRITE_RENDERER_H