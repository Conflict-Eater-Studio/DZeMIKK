#ifndef DZEMIKK_MESH_RENDERER_H
#define DZEMIKK_MESH_RENDERER_H

#include "../component.h"

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>


namespace dzemikk {
class Transform;
class Model;
class Material;

/**
 * @brief Component responsible for rendering a mesh with a given material.
 *
 * MeshRenderer links together geometry (Mesh), appearance (Material),
 * and spatial data (Transform) to be used by the rendering system.
 */
class MeshRenderer : public Component {
  public:
    using Base = Component;

    /**
     * @brief Constructs an empty MeshRenderer component.
     */
    MeshRenderer() = default;
    ~MeshRenderer() = default;

#pragma region Disable copy and move

    MeshRenderer(const MeshRenderer& other) = delete;
    MeshRenderer(MeshRenderer&& other) noexcept = delete;
    MeshRenderer& operator=(const MeshRenderer& other) = delete;
    MeshRenderer& operator=(MeshRenderer&& other) noexcept = delete;

#pragma endregion

#pragma region Getters

    /**
     * @brief Returns the mesh used for rendering.
     *
     * @return Model* Pointer to the mesh.
     */
    [[nodiscard]] Model* getModel() const {
        return _model;
    }

    
    /**
     * @brief Returns all materials assigned to this renderer.
     *
     * @return const std::vector<Material*>& Reference to material list.
     */
    [[nodiscard]] const std::vector<Material*>& getMaterials() const {
        return _materials;
    }

    /**
     * @brief Returns a material at a given index.
     *
     * @param index Material index.
     * @return Material* Pointer to material or nullptr if out of range.
     */
    [[nodiscard]] Material* getMaterial(size_t index) const {
        if (index >= _materials.size()) {
            return nullptr;
        }

        return _materials[index];
    }

    /**
     * @brief Returns the transform associated with this renderer.
     *
     * @return Transform* Pointer to the transform.
     */
    [[nodiscard]] Transform* getTransform() const {
        return _transform;
    }

#pragma endregion

#pragma region Setters

    /**
     * @brief Sets the model used for rendering.
     *
     * @param mesh Pointer to the mesh.
     */
    void setModel(Model* model) {
        _model = model;
        calculateCullingRadius(model);
    }

    /**
     * @brief Sets the full material list.
     *
     * @param materials Vector of material pointers.
     */
    void setMaterials(const std::vector<Material*>& materials) {
        _materials = materials;
    }

    /**
     * @brief Sets a material at a specific index.
     *
     * If the index exceeds current size, the internal array is expanded.
     *
     * @param index Material slot index.
     * @param material Pointer to material.
     */
    void setMaterial(size_t index, Material* material) {
        if (index >= _materials.size()) {
            _materials.resize(index + 1, nullptr);
        }

        _materials[index] = material;
    }

    /**
     * @brief Sets the transform associated with this renderer.
     *
     * @param transform Pointer to the transform.
     */
    void setTransform(Transform* transform) {
        _transform = transform;
    }

    /**
     * @brief Sets the color multiplier for the mesh.
     *
     * @param color The RGBA color vector.
     */
    void setColor(const glm::vec4& color) {
        _color = color;
    }

    /**
     * @brief Returns the color multiplier.
     *
     * @return const glm::vec4& The color.
     */
    [[nodiscard]] const glm::vec4& getColor() const {
        return _color;
    }

#pragma endregion

#pragma region Validation

    /**
     * @brief Checks if the renderer has all required data to render.
     *
     * @return true If mesh, material, and transform are set.
     * @return false Otherwise.
     */
    [[nodiscard]] bool isValid() const {
        return _model && !_materials.empty() && _transform;
    }

#pragma endregion

    /**
     * @brief Recalculates bounding radius used for frustum culling.
     *
     * @param model Model used for computation.
     */
    void calculateCullingRadius(Model* model);

    /**
     * @brief Returns precomputed culling radius.
     *
     * @return float Radius used for visibility checks.
     */
    float getCullingRadius() const;

    /**
     * @brief Returns runtime type name of this component.
     *
     * @return std::string Name of the component type.
     */
    [[nodiscard]] std::string typeName() const override {
        return "MeshRenderer";
    };

  private:
#pragma region References

    Model* _model = nullptr;
    std::vector<Material*> _materials;
    Transform* _transform = nullptr;
    glm::vec4 _color = glm::vec4(1.0f);

    float _cullingRadius = 1.0F;

#pragma endregion
};

} // namespace dzemikk
#endif // DZEMIKK_MESH_RENDERER_H