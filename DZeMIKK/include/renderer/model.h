#ifndef DZEMIKK_MODEL_H
#define DZEMIKK_MODEL_H

#include "renderer/mesh.h"
#include "animation/skeleton.h"
#include "assetManager/meshBuilder.h"
#include "assetManager/iGpuUploadable.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <variant>

namespace dzemikk {
/**
 * @brief Represents a 3D model composed of multiple sub-meshes.
 *
 * A Model is a container for multiple SubMesh objects, each of which
 * references a Mesh and an associated material index. This allows
 * rendering complex objects composed of multiple parts with different materials.
 */
class Model : public IGpuUploadable {
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

    /**
     * @brief Sets skeleton used by the model (for skinned meshes).
     */
    void setSkeleton(std::shared_ptr<Skeleton> skeleton);

    /**
     * @brief Returns model skeleton if available.
     */
    std::shared_ptr<Skeleton> getSkeleton() const;

    /**
     * @brief Adds raw static mesh to pending GPU upload queue.
     */
    void addPending(MeshBuilder::RawStaticMesh& mesh);

    /**
     * @brief Adds raw skinned mesh to pending GPU upload queue.
     */
    void addPending(MeshBuilder::RawSkinnedMesh& mesh);

#pragma endregion

    /**
     * @brief Uploads resource data to the GPU.
     *
     * Transfers CPU-side asset data into GPU memory so it can be used
     * for rendering. Called after the asset has been fully loaded.
     */
    void uploadToGPU() override;

  private:
    /**
     * @brief Container of all sub-meshes in the model.
     */
    std::vector<SubMesh> _subMeshes;

    /**
     * @brief Skeleton used for skinned animation (if present).
     */
    std::shared_ptr<Skeleton> _skeleton;

    /**
     * @brief Pending mesh data waiting for GPU upload.
     */
    struct PendingMesh {
        std::variant<MeshBuilder::RawStaticMesh, MeshBuilder::RawSkinnedMesh> data;
    };

    /**
     * @brief Queue of meshes awaiting GPU upload.
     */
    std::vector<PendingMesh> _pendingMeshes;

    /**
     * @brief Indicates whether GPU resources have been initialized.
     */
    bool _gpuReady = false;
};

} // namespace dzemikk

#endif //#ifndef DZEMIKK_MODEL_H