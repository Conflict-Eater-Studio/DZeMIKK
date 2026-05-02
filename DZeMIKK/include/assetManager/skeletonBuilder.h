#ifndef DZEMIKK_SKELETON_BUILDER_H
#define DZEMIKK_SKELETON_BUILDER_H

#include <animation/skeleton.h>
#include <assimp/scene.h>
#include <memory>

namespace dzemikk {

/**
 * @brief Builds a runtime Skeleton from an Assimp scene.
 *
 * Traverses the Assimp node hierarchy and constructs a corresponding
 * Skeleton structure, including hierarchy and bind pose transforms.
 * Also applies bone offset matrices extracted from mesh data.
 */
class SkeletonBuilder {
  public:
    /**
     * @brief Creates a Skeleton from the given Assimp scene.
     *
     * @param scene Pointer to a loaded Assimp scene.
     * @return std::shared_ptr<Skeleton> Constructed skeleton instance.
     */
    static Skeleton* build(const aiScene* scene);

  private:
#pragma region Conversion

    /**
     * @brief Converts an Assimp matrix to a GLM matrix.
     *
     * @param m Assimp matrix.
     * @return glm::mat4 Equivalent GLM matrix.
     */
    static glm::mat4 aiToGlm(const aiMatrix4x4& m);

#pragma endregion

#pragma region Hierarchy construction

    /**
     * @brief Recursively builds the skeleton hierarchy from Assimp nodes.
     *
     * @param node Current Assimp node.
     * @param skeleton Target skeleton being constructed.
     * @param parent Index of the parent bone (-1 if root).
     */
    static void buildSkeleton(aiNode* node, Skeleton& skeleton, int parent);

#pragma endregion

#pragma region Bone data

    /**
     * @brief Applies bone offset (inverse bind pose) matrices from meshes.
     *
     * Extracts bone offset matrices from Assimp mesh data and assigns them
     * to corresponding bones in the skeleton.
     *
     * @param scene Assimp scene containing mesh data.
     * @param skeleton Skeleton to update.
     */
    static void applyBoneOffsets(const aiScene* scene, Skeleton& skeleton);

#pragma endregion
};

} // namespace dzemikk
#endif // DZEMIKK_SKELETON_BUILDER_H