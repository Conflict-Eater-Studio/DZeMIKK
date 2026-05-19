#ifndef DZEMIKK_SCENE_HANDLER_H
#define DZEMIKK_SCENE_HANDLER_H

#include "ecs/scene.h"
#include "iAssetHandler.h"

namespace dzemikk {

class AssetManager;

class SceneHandler : public IAssetHandler<Scene> {
  public:
    using Handle = AssetHandle<Scene>;
    using Result = AssetResult<Scene>;

    explicit SceneHandler(AssetManager* assetManager);

    Result load(const std::string& path,
                LoadExecutionMode loadExecutionMode = LoadExecutionMode::Sync) override;

    bool reload(Handle& asset, const std::string& path) override;

    void unload(Handle& asset) override;

  private:
    AssetManager* _assetManager = nullptr;
};

} // namespace dzemikk

#endif