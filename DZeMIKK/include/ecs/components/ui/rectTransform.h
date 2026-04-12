#ifndef DZEMIKK_RECTTRANSFORM_H
#define DZEMIKK_RECTTRANSFORM_H

#include "ecs/component.h"
#include "glm/fwd.hpp"

#include <glm/glm.hpp>
#include <string>

namespace dzemikk {
/**
 * @brief Parameters for initializing a RectTransform
 */
struct RectTransformParams {
    glm::vec2 position = glm::vec2(0.0F);
    glm::vec2 size = glm::vec2(0.0F);
    glm::vec2 scale = glm::vec2(1.0F);
    float rotation = 0.0F; // [degrees]
    glm::vec2 pivot = glm::vec2(0.5F);
    glm::vec2 anchorMin = glm::vec2(0.5F);
    glm::vec2 anchorMax = glm::vec2(0.5F);
};

class RectTransform : public Component {
  public:
    using Base = Component;

    /**
     * @brief Constructs a RectTransform with the given parameters
     * @param params Initialization parameters
     */
    RectTransform(RectTransformParams params = RectTransformParams());
    RectTransform(const RectTransform& other) = delete;
    RectTransform& operator=(const RectTransform& other) = delete;
    RectTransform(RectTransform&& other) noexcept = delete;
    RectTransform operator=(RectTransform&& other) noexcept = delete;
    ~RectTransform() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "RectTransform";
    }

    /**
     * @brief Sets the position
     * @param position New position
     */
    void setPosition(const glm::vec2& position);

    /**
     * @brief Sets the size
     * @param size New size
     */
    void setSize(const glm::vec2& size);

    /**
     * @brief Sets the scale
     * @param scale New scale
     */
    void setScale(const glm::vec2& scale);

    /**
     * @brief Sets the rotation
     * @param rotation New rotation (in degrees)
     */
    void setRotation(float rotation);

    /**
     * @brief Sets the pivot
     * @param pivot New pivot
     */
    void setPivot(const glm::vec2& pivot);

    /**
     * @brief Sets the anchor minimum
     * @param anchorMin New anchor minimum
     */
    void setAnchorMin(const glm::vec2& anchorMin);

    /**
     * @brief Sets the anchor maximum
     * @param anchorMax New anchor maximum
     */
    void setAnchorMax(const glm::vec2& anchorMax);

    /**
     * @brief Sets the z-index
     * @param zIndex New z-index
     */
    void setZIndex(unsigned int zIndex);

    /**
     * @brief Translates the transform
     * @param delta Translation vector
     */
    void translate(const glm::vec2& delta);

    /**
     * @brief Rotates the transform
     * @param deltaDegrees Delta rotation (in degrees)
     */
    void rotate(float deltaDegrees);

    /**
     * @brief Scales the transform
     * @param delta Scale vector
     */
    void scale(const glm::vec2& delta);

    /**
     * @brief Scales the transform uniformly
     * @param uniform New uniform scale
     */
    void scale(float uniform);

    /**
     * @brief Gets the position
     * @return Current position
     */
    [[nodiscard]] glm::vec2 getPosition() const;

    /**
     * @brief Gets the size of the sprite to render
     * If anchor max - min is not (0, 0), this will return stretch size + size, otherwise just size.
     * @return Current size
     */
    [[nodiscard]] glm::vec2 getSize() const;

    /**
     * @brief Gets the scale
     * @return Current scale
     */
    [[nodiscard]] glm::vec2 getScale() const;

    /**
     * @brief Gets the rotation
     * @return Current rotation (in degrees)
     */
    [[nodiscard]] float getRotation() const;

    /**
     * @brief Gets the pivot
     * @return Current pivot
     */
    [[nodiscard]] glm::vec2 getPivot() const;

    /**
     * @brief Gets the anchor minimum
     * @return Current anchor minimum
     */
    [[nodiscard]] glm::vec2 getAnchorMin() const;

    /**
     * @brief Gets the anchor maximum
     * @return Current anchor maximum
     */
    [[nodiscard]] glm::vec2 getAnchorMax() const;

    /**
     * @brief Gets the z-index
     * @return Current z-index
     */
    [[nodiscard]] unsigned int getZIndex() const;

    /**
     * @brief Gets the local transformation matrix
     * @return Local transformation matrix
     */
    [[nodiscard]] glm::mat4 getLocalMatrix() const;

    /**
     * @brief Gets the world transformation matrix
     * @return World transformation matrix
     */
    [[nodiscard]] glm::mat4 getWorldMatrix() const;

    /**
     * @brief Gets the stretch size
     * @return Current stretch size
     */
    [[nodiscard]] glm::vec2 getStretchSize() const;

    void markDirty();
    void markSizeDirty();

  private:
    [[nodiscard]] glm::mat4 getLocalNoSizeMatrix() const;
    [[nodiscard]] glm::mat4 getLocalSizeMatrix() const;
    [[nodiscard]] glm::mat4 getWorldNoSizeMatrix() const;

    glm::vec2 _position{0.0F, 0.0F};
    glm::vec2 _size{100.0F, 100.0F};
    glm::vec2 _scale{1.0F, 1.0F};
    float _rotation{0.0F}; // [degrees]
    glm::vec2 _pivot{0.5F, 0.5F};
    glm::vec2 _anchorMin{0.5F, 0.5F};
    glm::vec2 _anchorMax{0.5F, 0.5F};
    unsigned int _zIndex{0};

    mutable glm::mat4 _cachedLocalNoSizeMatrix = glm::mat4(1.0F);
    mutable glm::mat4 _cachedLocalSizeMatrix = glm::mat4(1.0F);
    mutable glm::mat4 _cachedLocalMatrix = glm::mat4(1.0F);
    mutable glm::mat4 _cachedWorldNoSizeMatrix = glm::mat4(1.0F);
    mutable glm::mat4 _cachedWorldMatrix = glm::mat4(1.0F);
    mutable glm::vec2 _cachedSize = glm::vec2(0.0F);
    mutable bool _localDirty = true;
    mutable bool _worldDirty = true;
    mutable bool _sizeDirty = true;
};
} // namespace dzemikk

#endif // DZEMIKK_RECTTRANSFORM_H