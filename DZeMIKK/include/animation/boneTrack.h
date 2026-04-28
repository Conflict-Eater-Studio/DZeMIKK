#include "animation/bone.h"
#include "animation/quaterniontrack.h"
#include "animation/vectortrack.h"

namespace dzemikk {
class QuaternionTrack;
class VectorTrack;

/**
 * @brief Represents animation tracks for a single bone (rotation, position, scale).
 *        NOTE: This class does NOT own the pointers (no memory management).
 */
class BoneTrack {
  public:
    BoneTrack() = default;
    ~BoneTrack() = default;

    void setRotation(QuaternionTrack* rotation) {
        m_rotation = rotation;
    }

    void setPosition(VectorTrack* position) {
        m_position = position;
    }

    void setScale(VectorTrack* scale) {
        m_scale = scale;
    }

    QuaternionTrack* getRotation() const {
        return m_rotation;
    }

    VectorTrack* getPosition() const {
        return m_position;
    }

    VectorTrack* getScale() const {
        return m_scale;
    }

    bool hasRotation() const {
        return m_rotation != nullptr;
    }
    bool hasPosition() const {
        return m_position != nullptr;
    }
    bool hasScale() const {
        return m_scale != nullptr;
    }

    void clearRotation() {
        m_rotation = nullptr;
    }
    void clearPosition() {
        m_position = nullptr;
    }
    void clearScale() {
        m_scale = nullptr;
    }

  private:
    QuaternionTrack* m_rotation = nullptr;
    VectorTrack* m_position = nullptr;
    VectorTrack* m_scale = nullptr;
};
} // namespace dzemikk