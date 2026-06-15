#ifndef DZEMIKK_MATERIAL_H
#define DZEMIKK_MATERIAL_H

#include "assetManager/assetHandle.h"
#include "renderer/texture.h"

#include <glm/vec3.hpp>

namespace dzemikk {

class Shader;

/**
 * @brief PBR material used for rendering.
 */
class Material {
  public:
    Material() = default;
    ~Material() = default;

#pragma region Disable copy and move

    Material(const Material&) = delete;
    Material(Material&&) noexcept = delete;
    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&) noexcept = delete;

#pragma endregion

#pragma region Shader

    [[nodiscard]] Shader* getShader() const {
        return _shader.get();
    }

    [[nodiscard]] const AssetHandle<Shader>& getShaderHandle() const {
        return _shader;
    }

    void setShader(const AssetHandle<Shader>& shader) {
        _shader = shader;
    }

#pragma endregion

#pragma region Textures

    [[nodiscard]] Texture* getAlbedoTexture() const {
        return _albedo.get();
    }

    [[nodiscard]] Texture* getNormalTexture() const {
        return _normal.get();
    }

    [[nodiscard]] Texture* getMetallicTexture() const {
        return _metallicMap.get();
    }

    [[nodiscard]] Texture* getRoughnessTexture() const {
        return _roughnessMap.get();
    }

    [[nodiscard]] Texture* getAOTexture() const {
        return _aoMap.get();
    }

    [[nodiscard]] Texture* getEmissiveTexture() const {
        return _emissive.get();
    }

    void setAlbedoTexture(const AssetHandle<Texture>& texture) {
        _albedo = texture;
    }

    void setNormalTexture(const AssetHandle<Texture>& texture) {
        _normal = texture;
    }

    void setMetallicTexture(const AssetHandle<Texture>& texture) {
        _metallicMap = texture;
    }

    void setRoughnessTexture(const AssetHandle<Texture>& texture) {
        _roughnessMap = texture;
    }

    void setAOTexture(const AssetHandle<Texture>& texture) {
        _aoMap = texture;
    }

    void setEmissiveTexture(const AssetHandle<Texture>& texture) {
        _emissive = texture;
    }

    const AssetHandle<Texture>& getAlbedoTextureHandle() const {
        return _albedo;
    }

    const AssetHandle<Texture>& getNormalTextureHandle() const {
        return _normal;
    }

    const AssetHandle<Texture>& getMetallicTextureHandle() const {
        return _metallicMap;
    }

    const AssetHandle<Texture>& getRoughnessTextureHandle() const {
        return _roughnessMap;
    }

    const AssetHandle<Texture>& getAOTextureHandle() const {
        return _aoMap;
    }

    const AssetHandle<Texture>& getEmissiveTextureHandle() const {
        return _emissive;
    }

#pragma endregion

#pragma region Parameters

    [[nodiscard]] const glm::vec3& getAlbedoColor() const {
        return _albedoColor;
    }

    [[nodiscard]] float getMetallic() const {
        return _metallic;
    }

    [[nodiscard]] float getRoughness() const {
        return _roughness;
    }

    [[nodiscard]] float getAO() const {
        return _ao;
    }

    void setAlbedoColor(const glm::vec3& color) {
        _albedoColor = color;
    }

    void setMetallic(float value) {
        _metallic = value;
    }

    void setRoughness(float value) {
        _roughness = value;
    }

    void setAO(float value) {
        _ao = value;
    }

#pragma endregion

#pragma region Validation

    [[nodiscard]] bool isValid() const {
        return _shader.get() != nullptr;
    }

    [[nodiscard]] bool hasAlbedoTexture() const {
        return _albedo.get() != nullptr;
    }

    [[nodiscard]] bool hasNormalTexture() const {
        return _normal.get() != nullptr;
    }

    [[nodiscard]] bool hasMetallicTexture() const {
        return _metallicMap.get() != nullptr;
    }

    [[nodiscard]] bool hasRoughnessTexture() const {
        return _roughnessMap.get() != nullptr;
    }

    [[nodiscard]] bool hasAOTexture() const {
        return _aoMap.get() != nullptr;
    }

    [[nodiscard]] bool hasEmissiveTexture() const {
        return _emissive.get() != nullptr;
    }

#pragma endregion

  private:
    AssetHandle<Shader> _shader;

    // PBR textures
    AssetHandle<Texture> _albedo;
    AssetHandle<Texture> _normal;
    AssetHandle<Texture> _metallicMap;
    AssetHandle<Texture> _roughnessMap;
    AssetHandle<Texture> _aoMap;
    AssetHandle<Texture> _emissive;

    // PBR factors
    glm::vec3 _albedoColor{1.0f, 1.0f, 1.0f};

    float _metallic = 0.0f;
    float _roughness = 0.5f;
    float _ao = 1.0f;
};

} // namespace dzemikk

#endif