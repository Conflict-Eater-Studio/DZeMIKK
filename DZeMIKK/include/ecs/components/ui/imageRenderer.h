#ifndef DZEMIKK_IMAGERENDERER_H
#define DZEMIKK_IMAGERENDERER_H

#include "ecs/component.h"
#include "assetManager/assetHandle.h"

#include <glm/glm.hpp>

namespace dzemikk {

class RectTransform;
class Mesh;
class Material;
class ImageRenderer : public Component {
  public:
    using Base = Component;

    ImageRenderer() = default;
    ~ImageRenderer() override = default;

    ImageRenderer(const ImageRenderer& other) = delete;
    ImageRenderer(ImageRenderer&& other) noexcept = delete;
    ImageRenderer& operator=(const ImageRenderer& other) = delete;
    ImageRenderer& operator=(ImageRenderer&& other) noexcept = delete;

    [[nodiscard]] Mesh* getMesh() const {
        return _mesh.get();
    }

    [[nodiscard]] AssetHandle<Mesh> getMeshHandle() const {
        return _mesh;
    }

    [[nodiscard]] Material* getMaterial() const {
        return _material;
    }

    [[nodiscard]] RectTransform* getRectTransform() const {
        return _rectTransform;
    }

    [[nodiscard]] unsigned int getTexture() const {
        return _textureID;
    }

    [[nodiscard]] glm::vec4 getColor() const {
        return _color;
    }

    void setMesh(const AssetHandle<Mesh>& mesh) {
        _mesh = mesh;
    }

    void setMaterial(Material* material) {
        _material = material;
    }

    void setRectTransform(RectTransform* transform) {
        _rectTransform = transform;
    }

    void setTexture(unsigned int texID) {
        _textureID = texID;
    }

    void setColor(const glm::vec4& color) {
        _color = color;
    }

    [[nodiscard]] bool isValid() const {
        return _mesh && _material && _rectTransform;
    }

    [[nodiscard]] std::string typeName() const override {
        return "ImageRenderer";
    };

  private:
    AssetHandle<Mesh> _mesh;

    Material* _material = nullptr;

    RectTransform* _rectTransform = nullptr;

    unsigned int _textureID = 0;
    glm::vec4 _color = glm::vec4(1.0F);
};
} // namespace dzemikk
#endif // DZEMIKK_UISPRITERENDERER_H