#ifndef DZEMIKK_MODEL_HANDLER_H
#define DZEMIKK_MODEL_HANDLER_H
#include "iAssetHandler.h"
#include "animation/skeleton.h"
#include <assimp/matrix4x4.h>
#include <assimp/mesh.h>
#include "renderer/SkinnedMesh.h"

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

    static glm::mat4 aiToGlm(const aiMatrix4x4& m);
    static void buildSkeleton(aiNode* node, dzemikk::Skeleton& skeleton, int parent);
    static void extractBoneWeights(const aiMesh* mesh,
                                   std::vector<SkinnedVertex>& vertices,
                                   const dzemikk::Skeleton& skeleton);

};
}

#endif //DZEMIKK_MODEL_HANDLER_H

