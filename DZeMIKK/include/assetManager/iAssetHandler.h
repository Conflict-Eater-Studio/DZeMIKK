#ifndef DZEMIKK_I_ASSET_HANDLE_H
#define DZEMIKK_I_ASSET_HANDLE_H

#include "assetHandle.h"
#include "assetError.h"

#include <string>
#include <expected>

namespace dzemikk {

template <typename T> struct AssetResult {
    std::shared_ptr<T> resource;
    AssetError error = AssetError::None;

    bool isValid() const {
        return resource && error == AssetError::None;
    }
};

struct IAssetHandlerBase {
    virtual ~IAssetHandlerBase() = default;
};

/**
 * @brief Generic interface for asset handlers.
 *
 * Defines a common API for loading, reloading, and unloading assets
 * of a given type within the engine.
 *
 * @tparam T Type of the asset managed by the handler.
 */
template <typename T> 
class IAssetHandler : public IAssetHandlerBase {
    public:
        /**
         * @brief Type of asset handled by this interface.
         */
        using AssetType = T;

        /**
         * @brief Handle type used to reference assets.
         */
        using Handle = AssetHandle<T>;

        /**
         * @brief Result type returned from load operations.
         */
        using Result = AssetResult<T>;

        virtual ~IAssetHandler() = default;

        /**
         * @brief Default constructor.
         */
        IAssetHandler() = default;

        IAssetHandler(const IAssetHandler&) = delete;
        IAssetHandler& operator=(const IAssetHandler&) = delete;

        IAssetHandler(IAssetHandler&&) = delete;
        IAssetHandler& operator=(IAssetHandler&&) = delete;

        /**
         * @brief Loads an asset from file.
         *
         * @param path Path to asset file.
         * @return Result containing asset handle or error.
         */
        virtual Result load(const std::string& path) = 0;

        /**
         * @brief Reloads an existing asset.
         *
         * @param asset Handle to asset.
         * @param path Path to asset file.
         * @return True if reload succeeded.
         */
        virtual bool reload(Handle& asset, const std::string& path) = 0;

        /**
         * @brief Unloads an asset from memory.
         *
         * @param asset Handle to asset.
         */
        virtual void unload(Handle& asset) = 0;
};
} // namespace dzemikk

#endif // DZEMIKK_I_ASSET_HANDLE_H
