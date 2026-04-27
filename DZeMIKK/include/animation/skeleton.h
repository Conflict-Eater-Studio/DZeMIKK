#ifndef DZEMIKK_SKELETON_H
#define DZEMIKK_SKELETON_H

#include "bone.h"

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace dzemikk {

/**
 * @brief Represents a skeleton consisting of a hierarchy of bones.
 *
 * Provides fast lookup by name and indexed access.
 */
class Skeleton {
  public:
#pragma region Bone Management

    /**
     * @brief Adds a new bone to the skeleton.
     *
     * @param name Bone name.
     * @param parentIndex Parent bone index (-1 if root).
     * @return Index of the newly added bone.
     */
    int addBone(const std::string& name, int parentIndex);

#pragma endregion

#pragma region Getters

    /**
     * @brief Returns bone index by name.
     *
     * @param name Bone name.
     * @return Index or -1 if not found.
     */
    [[nodiscard]] int getBoneIndex(const std::string& name) const;

    /**
     * @brief Returns bone by index (const).
     *
     * @param index Bone index.
     * @return Pointer to bone or nullptr if invalid index.
     */
    [[nodiscard]] const Bone* getBone(std::size_t index) const;
    Bone* getBone(int index);

    /**
     * @brief Returns total number of bones.
     */
    [[nodiscard]] std::size_t getBoneCount() const;

    /**
     * @brief Returns all bones (read-only).
     */
    [[nodiscard]] const std::vector<Bone>& getBones() const;

#pragma endregion

#pragma region Global Transform

    /**
     * @brief Returns the global inverse transform matrix of the skeleton.
     *
     * This matrix is used to convert from model space into root bone space,
     * ensuring correct alignment of animated meshes.
     *
     * @return const glm::mat4& Global inverse transform matrix.
     */
    [[nodiscard]] const glm::mat4& getGlobalInverseTransform() const;

    
    /**
     * @brief Sets the global inverse transform matrix of the skeleton.
     *
     * Typically computed as the inverse of the root node transform from the
     * imported scene (e.g. Assimp root node).
     *
     * @param transform Global inverse transform matrix.
     */
    void setGlobalInverseTransform(const glm::mat4& transform);

#pragma endregion

  private:
    std::vector<Bone> _bones;
    std::unordered_map<std::string, int> _boneMap;

    glm::mat4 _globalInverseTransform{1.0F};
};

} // namespace dzemikk
#endif // DZEMIKK_SKELETON_H