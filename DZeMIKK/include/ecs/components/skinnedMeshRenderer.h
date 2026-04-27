#ifndef DZEMIKK_SKINNED_MESH_RENDERER_H
#define DZEMIKK_SKINNED_MESH_RENDERER_H
#include "../component.h"
#include "renderer/model.h"
#include "renderer/material.h"
#include "ecs/components/transform.h"

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

namespace dzemikk {
/**
 * @brief Component responsible for rendering skinned (skeletal animated) meshes.
 *
 * SkinnedMeshRenderer binds a Model containing a Skeleton and animated meshes
 * to the ECS transform system. It stores per-material overrides and computes
 * final bone transformation matrices used for GPU skinning.
 */
class SkinnedMeshRenderer : public Component {
  public:
#pragma region Getters

    /**
     * @brief Returns the model used for rendering.
     *
     * @return Model* Pointer to the assigned model (may be nullptr).
     */
    [[nodiscard]] Model* getModel() const {
        return _model;
    }

    /**
     * @brief Returns the list of materials assigned to this renderer.
     *
     * Each material corresponds to a mesh sub-part in the model.
     *
     * @return const std::vector<Material*>& Reference to material array.
     */
    [[nodiscard]] const std::vector<Material*>& getMaterials() const {
        return _materials;
    }

    /**
     * @brief Returns a material at the specified index.
     *
     * If the index is out of bounds, nullptr is returned.
     *
     * @param index Material slot index.
     * @return Material* Pointer to material or nullptr if invalid index.
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
     * This transform defines the world-space placement of the skinned mesh.
     *
     * @return Transform* Pointer to transform component.
     */
    [[nodiscard]] Transform* getTransform() const {
        return _transform;
    }

#pragma endregion

#pragma region Setters

    /**
     * @brief Assigns a model to this renderer.
     *
     * The model must contain a valid skeleton and skinned meshes if animation is used.
     *
     * @param model Pointer to model instance.
     */
    void setModel(Model* model) {
        _model = model;
    }

    /**
     * @brief Replaces the entire material list.
     *
     * Materials are mapped to mesh sub-parts in order.
     *
     * @param materials Vector of material pointers.
     */
    void setMaterials(const std::vector<Material*>& materials) {
        _materials = materials;
    }

    /**
     * @brief Sets a material at a given index.
     *
     * Expands internal storage if the index is out of range.
     *
     * @param index Material slot index.
     * @param material Pointer to material instance.
     */
    void setMaterial(size_t index, Material* material) {
        if (index >= _materials.size()) {
            _materials.resize(index + 1, nullptr);
        }
        _materials[index] = material;
    }

    /**
     * @brief Assigns a transform component to this renderer.
     *
     * @param transform Pointer to Transform component.
     */
    void setTransform(Transform* transform) {
        _transform = transform;
    }

#pragma endregion

#pragma region Animation / Skinning

    /**
     * @brief Recursively computes bone transformation matrices.
     *
     * Traverses skeleton hierarchy and computes final skinning matrices
     * relative to the given parent transform.
     *
     * @param index Current bone index in skeleton hierarchy.
     * @param parentTransform Accumulated parent transformation matrix.
     */
    void calculateBoneMatrices(int index, const glm::mat4& parentTransform);

    /**
     * @brief Returns computed final bone matrices used for GPU skinning.
     *
     * These matrices are typically uploaded to a uniform buffer or shader array.
     *
     * @return std::vector<glm::mat4>& Reference to bone matrix buffer.
     */
    [[nodiscard]] std::vector<glm::mat4>& getBoneMatrices() {
        return _finalBoneMatrices;
    }

#pragma endregion

#pragma region Type info

    /**
     * @brief Returns runtime type name of this component.
     *
     * @return std::string "SkinnedMeshRenderer"
     */
    [[nodiscard]] std::string typeName() const override {
        return "SkinnedMeshRenderer";
    };

#pragma endregion

  private:
    Model* _model = nullptr;                   
    std::vector<Material*> _materials;         
    Transform* _transform = nullptr;           
    std::vector<glm::mat4> _finalBoneMatrices;
};
}

#endif // DZEMIKK_SKINNED_MESH_RENDERER_H