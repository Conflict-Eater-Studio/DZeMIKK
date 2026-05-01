#ifndef DZEMIKK_MODEL_HANDLER_H
#define DZEMIKK_MODEL_HANDLER_H
#include "iAssetHandler.h"
#include "animation/skeleton.h"
#include <assimp/matrix4x4.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include "renderer/SkinnedMesh.h"
#include "renderer/StaticMesh.h"

namespace dzemikk {
class Model;

/**
 * @brief Asset handler responsible for loading, reloading and unloading 3D models.
 *
 * ModelHandler implements the IAssetHandler interface for Model assets.
 * It provides functionality to load models from disk, reload existing assets
 * (hot-reload support), and properly release model resources.
 */
class ModelHandler : public IAssetHandler<Model> {
  public:
    using Handle = AssetHandle<Model>;
    using Result = AssetResult<Model>;
    
    /**
     * @brief Loads a model asset from a file path.
     *
     * @param path Path to the model file.
     * @return Result containing the loaded Model or an error state.
     */
    Result load(const std::string& path) override;

    /**
     * @brief Reloads an existing model asset from a file path.
     *
     * Used for hot-reloading assets without recreating handles.
     *
     * @param asset Reference to the existing asset handle.
     * @param path Path to the model file.
     * @return true if reload succeeded, false otherwise.
     */
    bool reload(Handle& asset, const std::string& path) override;

    /**
     * @brief Unloads a model asset and releases its resources.
     *
     * @param asset Reference to the asset handle to unload.
     */
    void unload(Handle& asset) override;

  private:
    /**
     * @brief Internal helper function for loading a model from file.
     *
     * Performs actual parsing and construction of the Model object.
     *
     * @param path Path to the model file.
     * @return Shared pointer to the loaded Model, or nullptr on failure.
     */
    static std::shared_ptr<Model> loadModelFromFile(const std::string& path);

    static void loadAnimations(const aiScene* scene, Skeleton& skeleton);

#pragma region Assimp conversion utilities

    /**
     * @brief Converts Assimp matrix format to GLM matrix format.
     *
     * @param m Input Assimp transformation matrix.
     * @return glm::mat4 Converted GLM-compatible matrix.
     */
    static glm::mat4 aiToGlm(const aiMatrix4x4& m);

    /**
     * @brief Recursively builds a skeleton hierarchy from an Assimp scene node.
     *
     * Traverses the node tree and constructs a skeleton structure while accumulating
     * hierarchical transforms. Each node's transformation is combined with its parent
     * to produce a final bone transform in model space.
     *
     * @param node Current Assimp scene node.
     * @param skeleton Output skeleton structure being constructed.
     * @param parent Index of the parent bone (-1 for root).
     * @param accumulatedTransform Transformation matrix accumulated from parent nodes.
     */
    static void buildSkeleton(aiNode* node, Skeleton& skeleton, int parent,
                              glm::mat4 accumulatedTransform);
    static void applyBoneOffsets(const aiScene* scene, Skeleton& skeleton);

    /**
     * @brief Extracts bone weights from an Assimp mesh and applies them to vertices.
     *
     * Parses bone influence data from the mesh and assigns up to 4 bone IDs and
     * weights per vertex. Also updates skeleton mapping with bone indices.
     *
     * @param mesh Input Assimp mesh containing bone data.
     * @param vertices Output vertex array to be filled with skinning data.
     * @param skeleton Skeleton structure used to map bone names to indices.
     */
    static void extractBoneWeights(const aiMesh* mesh, std::vector<SkinnedVertex>& vertices,
                                   dzemikk::Skeleton& skeleton);

    /**
     * @brief Builds a static (non-animated) mesh from Assimp mesh data.
     *
     * Converts vertex positions, normals, and index data from an aiMesh into a
     * GPU-ready StaticMesh. This function is used for geometry that does not
     * participate in skeletal animation.
     *
     * @param mesh Input Assimp mesh containing static geometry data.
     * @return std::shared_ptr<StaticMesh> Fully constructed and GPU-uploaded static mesh.
     */
    static std::shared_ptr<StaticMesh> buildStaticMesh(const aiMesh* mesh);

    /**
     * @brief Builds a skinned (skeletally animated) mesh from Assimp mesh data.
     *
     * Converts vertex attributes (position, normal, bone IDs, weights) and index
     * data into a GPU-ready SkinnedMesh. Also binds vertex bone influences to the
     * provided skeleton structure.
     *
     * @param mesh Input Assimp mesh containing bone and skinning data.
     * @param skeleton Skeleton used to resolve bone IDs and bind pose transforms.
     * @return std::shared_ptr<SkinnedMesh> Fully constructed skinned mesh ready for animation.
     */
    static std::shared_ptr<SkinnedMesh> buildSkinnedMesh(const aiMesh* mesh, Skeleton& skeleton);

#pragma endregion

};
}

#endif //DZEMIKK_MODEL_HANDLER_H

