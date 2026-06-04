#ifndef DZEMIKK_IMAGERENDERER_H
#define DZEMIKK_IMAGERENDERER_H

#include "ecs/component.h"
#include "assetManager/assetHandle.h"

#include <glm/glm.hpp>

namespace dzemikk {

class RectTransform;
class Mesh;
class Material;
class Texture;

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

    [[nodiscard]] std::shared_ptr<dzemikk::Material> getMaterial() const {
        return _material;
    }

    [[nodiscard]] RectTransform* getRectTransform() const {
        return _rectTransform;
    }

    [[nodiscard]] Texture* getTexture() const {
        return _texture.get();
    }
    
    [[nodiscard]] AssetHandle<Texture> getTextureHandle() const {
        return _texture;
    }

    [[nodiscard]] glm::vec4 getColor() const {
        return _color;
    }

    void setMesh(AssetHandle<Mesh> mesh) {
        _mesh = mesh;
    }

    void setMaterial(std::shared_ptr<dzemikk::Material> material) {
        _material = material;
    }

    void setRectTransform(RectTransform* transform) {
        _rectTransform = transform;
    }

    bool useTexture() {
        return _useTexture;
    }

    void setUseTexture(bool useTexture) {
        _useTexture = useTexture;
    }

    void setTexture(AssetHandle<Texture> texture) {
        _texture = texture;
    }

    void setColor(const glm::vec4& color) {
        _color = color;
    }

    [[nodiscard]] bool isValid() const {
        return _mesh && _material && _rectTransform;
    }

    [[nodiscard]] bool hasTexture() const {
        return _texture.get() != nullptr;
    }

    [[nodiscard]] std::string typeName() const override {
        return "ImageRenderer";
    };

  private:
    AssetHandle<Mesh> _mesh;

    std::shared_ptr<dzemikk::Material> _material = nullptr;

    RectTransform* _rectTransform = nullptr;

    bool _useTexture = false;

    AssetHandle<Texture> _texture;
    glm::vec4 _color = glm::vec4(1.0F);
};
} // namespace dzemikk
#endif // DZEMIKK_UISPRITERENDERER_H
