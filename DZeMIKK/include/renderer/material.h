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

    [[nodiscard]] const glm::vec3& getEmissiveColor() const {
        return _emissiveColor;
    }

    [[nodiscard]] float getEmissiveStrength() const {
        return _emissiveStrength;
    }

    void setEmissiveColor(const glm::vec3& color) {
        _emissiveColor = color;
    }

    void setEmissiveStrength(float value) {
        _emissiveStrength = value;
    }
#pragma endregion

    std::shared_ptr<Material> clone() const {
        auto m = std::make_shared<Material>();

        m->_shader = _shader;

        m->_albedo = _albedo;
        m->_normal = _normal;
        m->_metallicMap = _metallicMap;
        m->_roughnessMap = _roughnessMap;
        m->_aoMap = _aoMap;
        m->_emissive = _emissive;

        m->_albedoColor = _albedoColor;
        m->_metallic = _metallic;
        m->_roughness = _roughness;
        m->_ao = _ao;

        m->_emissiveColor = _emissiveColor;
        m->_emissiveStrength = _emissiveStrength;

        return m;
    }

    bool operator==(const Material& other) const {
        return _shader.getAssetPath() == other._shader.getAssetPath() &&

               _albedo.get() == other._albedo.get() && _normal.get() == other._normal.get() &&
               _metallicMap.get() == other._metallicMap.get() && _roughnessMap.get() == other._roughnessMap.get() &&
               _aoMap.get() == other._aoMap.get() && _emissive.get() == other._emissive.get() &&
               
               _albedoColor == other._albedoColor && _emissiveColor == other._emissiveColor &&

               _metallic == other._metallic && _roughness == other._roughness && _ao == other._ao &&
               _emissiveStrength == other._emissiveStrength;
    }

    bool operator!=(const Material& other) const {
        return !(*this == other);
    }

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

    glm::vec3 _emissiveColor{0.0f};
    float _emissiveStrength = 0.0f;
};

} // namespace dzemikk

#endif