#ifndef DZEMIKK_POST_PROCESS_EFFECT_H
#define DZEMIKK_POST_PROCESS_EFFECT_H

#include "assetManager/assetHandle.h"
#include "ecs/component.h"
#include "renderer/shader.h"

namespace dzemikk {

class PostProcessEffect : public Component {
  public:
    PostProcessEffect() = default;

    virtual ~PostProcessEffect() = default;

    virtual void bindShaderUniforms(Shader& shader) {}

    virtual int getPriority() const {
        return 0;
    }

    void setShader(AssetHandle<Shader> shader) {
        _shader = shader;
    }

    [[nodiscard]]
    const AssetHandle<Shader>& getShader() const {
        return _shader;
    }

    void setEnabled(bool enabled) {
        _enabled = enabled;
    }

    [[nodiscard]]
    bool isEnabled() const {
        return _enabled;
    }

    std::string typeName() const override;

  private:
    AssetHandle<Shader> _shader;
    bool _enabled = true;
};

} // namespace dzemikk

#endif