#ifndef DZEMIKK_BONE_H
#define DZEMIKK_BONE_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace dzemikk {

/**
 * @brief Represents a single bone in a skeletal hierarchy.
 *
 * A Bone stores both bind-pose and runtime transformation data, along with
 * hierarchical relationships (parent-child) used for skeletal animation.
 *
 * It is a logical node in a skeleton.
 */
class Bone {
  public:
    /**
     * @brief Constructs a bone with a name and parent relationship.
     *
     * @param name Human-readable bone identifier.
     * @param parentIndex Index of parent bone in skeleton hierarchy (-1 if root).
     */
    Bone(std::string name, int parentIndex);

#pragma region Getters

    /**
     * @brief Returns the bone name.
     *
     * @return const std::string& Bone identifier.
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * @brief Returns index of parent bone.
     *
     * @return int Parent bone index, or -1 if this is the root bone.
     */
    [[nodiscard]] int getParentIndex() const;

    /**
     * @brief Returns the offset matrix (bind pose inverse transform).
     *
     * This matrix transforms vertices from model space into bone space.
     *
     * @return const glm::mat4& Offset matrix.
     */
    [[nodiscard]] const glm::mat4& getOffsetMatrix() const;

    /**
     * @brief Returns the local animation transform of the bone.
     *
     * This represents the current animated transform relative to parent bone.
     *
     * @return const glm::mat4& Local transform.
     */
    [[nodiscard]] const glm::mat4& getLocalTransform() const;

    /**
     * @brief Returns the bind-pose local transform of the bone.
     *
     * This is the original transform of the bone at rest pose.
     *
     * @return const glm::mat4& Bind pose local transform.
     */
    [[nodiscard]] const glm::mat4& getBindLocalTransform() const;

    /**
     * @brief Returns indices of all child bones.
     *
     * @return const std::vector<int>& List of child bone indices.
     */
    [[nodiscard]] const std::vector<int>& getChildren() const;

#pragma endregion

#pragma region Setters

    /**
     * @brief Sets the offset matrix (inverse bind pose matrix).
     *
     * @param offset Offset matrix transforming model space - bone space.
     */
    void setOffsetMatrix(const glm::mat4& offset);

    /**
     * @brief Sets the parent bone index.
     *
     * @param parent Parent bone index (-1 if root).
     */
    void setParent(int parent);

    /**
     * @brief Sets the current animated local transform.
     *
     * @param transform Local transformation matrix.
     */
    void setLocalTransform(const glm::mat4& transform);

    /**
     * @brief Sets the bind-pose local transform.
     *
     * @param transform Bind pose transformation matrix.
     */
    void setBindLocalTransform(const glm::mat4& transform);

    /**
     * @brief Adds a child bone index to this bone.
     *
     * @param childIndex Index of child bone in skeleton.
     */
    void addChild(int childIndex);

#pragma endregion

  private:
    std::string _name;     
    int _parentIndex = -1;

    glm::mat4 _offsetMatrix{1.0F};      
    glm::mat4 _localTransform{1.0F};     
    glm::mat4 _bindLocalTransform{1.0F}; 

    std::vector<int> _children; 
};
} // namespace dzemikk
#endif // DZEMIKK_BONE_H