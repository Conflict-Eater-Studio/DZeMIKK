#ifndef DZEMIKK_PREFAB_HANDLER_H
#define DZEMIKK_PREFAB_HANDLER_H

#include "assetManager/iAssetHandler.h"
#include "ecs/scene.h"
#include "ecs/serialize/prefabSerializer.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace dzemikk {

class PrefabHandler : public IAssetHandler<nlohmann::json> {
  public:
    using Handle = AssetHandle<nlohmann::json>;
    using Result = AssetResult<nlohmann::json>;

    PrefabHandler(AssetManager* assetManager);

    Result load(const std::string& path, LoadExecutionMode mode = LoadExecutionMode::Sync) override;

    bool reload(Handle& asset, const std::string& path) override;

    void unload(Handle& asset) override;

    void instantiate(const Handle& prefab, Scene& scene);

  private:
    AssetManager* _assetManager;
};

} // namespace dzemikk

#endif