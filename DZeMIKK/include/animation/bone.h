#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace dzemikk {

/**
 * @brief Represents a single bone in a skeleton hierarchy.
 *
 * Bone stores transformation data and parent-child relationships.
 */
class Bone {
  public:
    /**
     * @brief Constructs a bone.
     *
     * @param name Bone name.
     * @param parentIndex Index of the parent bone (-1 if root).
     */
    Bone(std::string name, int parentIndex);

#pragma region Getters

    [[nodiscard]] const std::string& getName() const;

    [[nodiscard]] int getParentIndex() const;

    [[nodiscard]] const glm::mat4& getOffsetMatrix() const;

    [[nodiscard]] const glm::mat4& getLocalTransform() const;

    [[nodiscard]] const std::vector<int>& getChildren() const;

#pragma endregion

#pragma region Setters

    void setOffsetMatrix(const glm::mat4& offset);

    void setLocalTransform(const glm::mat4& transform);

    void addChild(int childIndex);

#pragma endregion

  private:
    std::string _name;
    int _parentIndex = -1;

    glm::mat4 _offsetMatrix{1.0f};
    glm::mat4 _localTransform{1.0f};

    std::vector<int> _children;
};

} // namespace dzemikk