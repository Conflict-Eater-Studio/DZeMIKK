#pragma once

#include <glm/glm.hpp>
#include <string>

namespace editor {

class PropertyDrawer {
  public:
    static bool drawFloat(const std::string& label, float& value, float speed = 0.1F);

    static bool drawVec3(const std::string& label, glm::vec3& value, float resetValue = 0.0F);
};

} // namespace editor