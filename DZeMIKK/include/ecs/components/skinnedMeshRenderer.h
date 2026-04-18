#include "../component.h"
#include "renderer/model.h"
#include "renderer/material.h"
#include "ecs/components/transform.h"

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

namespace dzemikk {
class SkinnedMeshRenderer : public Component {
  public:
    using Base = Component;
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

    
    void calculateBoneMatrices(int index, const glm::mat4& parentTransform);

    [[nodiscard]] std::string typeName() const override {
        return "SkinnedMeshRenderer";
    };

    std::vector<glm::mat4>& getBoneMatrices() {
        return _finalBoneMatrices;
    }

#pragma endregion

  private:
    Model* _model = nullptr;
    std::vector<Material*> _materials;
    Transform* _transform = nullptr;

    std::vector<glm::mat4> _finalBoneMatrices; 
};
}