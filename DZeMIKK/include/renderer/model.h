#ifndef DZEMIKK_MODEL_H
#define DZEMIKK_MODEL_H

#include "renderer/mesh.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace dzemikk {
/**
 * @brief Represents a 3D model composed of multiple sub-meshes.
 *
 * A Model is a container for multiple SubMesh objects, each of which
 * references a Mesh and an associated material index. This allows
 * rendering complex objects composed of multiple parts with different materials.
 */
class Model {
  public:
    /**
     * @brief Represents a single sub-mesh within a model.
     *
     * Each SubMesh contains a reference to a Mesh and an index pointing
     * to a material
     */
    struct SubMesh {
        std::shared_ptr<Mesh> mesh;
        std::uint32_t materialIndex = 0;

        SubMesh(std::shared_ptr<Mesh> m, std::uint32_t matIdx)
            : mesh(std::move(m)), materialIndex(matIdx) {}
    };

#pragma region Constructors / Destructor

    /**
     * @brief Default constructor.
     */
    Model() = default;

    /**
     * @brief Default destructor.
     */
    ~Model() = default;

#pragma endregion

#pragma region Copy / Move semantics

    Model(const Model&) = default;
    Model(Model&&) noexcept = default;
    Model& operator=(const Model&) = default;
    Model& operator=(Model&&) noexcept = default;

#pragma endregion

#pragma region Rendering

    /**
     * @brief Draws all sub-meshes of the model.
     *
     * Iterates through all SubMesh objects and calls their draw method.
     * Material binding should be handled externally or extended here.
     */
    void draw() const;

#pragma endregion

#pragma region Mesh Management

    /**
     * @brief Adds a new sub-mesh to the model.
     *
     * @param mesh Shared pointer to the mesh.
     * @param materialIndex Index of the associated material.
     */
    void addMesh(const std::shared_ptr<Mesh>& mesh, std::uint32_t materialIndex);

    /**
     * @brief Removes all sub-meshes from the model.
     */
    void clear();

#pragma endregion

#pragma region Accessors

    /**
     * @brief Returns all sub-meshes.
     *
     * @return const std::vector<SubMesh>& Reference to sub-mesh container.
     */
    [[nodiscard]] const std::vector<SubMesh>& getSubMeshes() const;

    /**
     * @brief Returns the number of sub-meshes.
     *
     * @return std::size_t Number of sub-meshes.
     */
    [[nodiscard]] std::size_t getMeshCount() const;

    /**
     * @brief Returns a pointer to a sub-mesh at a given index.
     *
     * @param index Index of the sub-mesh.
     * @return const SubMesh* Pointer to the sub-mesh or nullptr if index is out of range.
     */
    [[nodiscard]] const SubMesh* getSubMesh(std::size_t index) const;

#pragma endregion

  private:
    /**
     *Container of all sub-meshes in the model
     */
    std::vector<SubMesh> _subMeshes;
};

} // namespace dzemikk

#endif //#ifndef DZEMIKK_MODEL_H