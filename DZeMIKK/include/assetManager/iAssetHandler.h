#ifndef DZEMIKK_I_ASSET_HANDLE_H
#define DZEMIKK_I_ASSET_HANDLE_H

#include "assetHandle.h"
#include "assetError.h"

#include <string>
#include <expected>

namespace dzemikk {

/**
 * @brief Result wrapper for asset loading operations.
 *
 * Encapsulates both the loaded resource and potential error state,
 * allowing consistent handling of success/failure cases in the asset pipeline.
 */
template <typename T> struct AssetResult {
    /** @brief Loaded asset resource (if successful). */
    std::shared_ptr<T> resource;

    /** @brief Error state of the loading operation. */
    AssetError error = AssetError::None;

    /**
     * @brief Checks whether the result is valid.
     *
     * @return true if resource is valid and no error occurred.
     */
    [[nodiscard]] bool isValid() const noexcept {
        return resource && error == AssetError::None;
    }
};

/**
 * @brief Base interface for all asset handlers.
 *
 * Provides type-erased operations required for generic asset management,
 * such as unloading assets without knowing their concrete type.
 */
struct IAssetHandlerBase {
    enum class LoadExecutionMode : std::uint8_t { Sync, Async };
    virtual ~IAssetHandlerBase() = default;
    IAssetHandlerBase() = default;

    IAssetHandlerBase(const IAssetHandlerBase&) = delete;
    IAssetHandlerBase& operator=(const IAssetHandlerBase&) = delete;

    IAssetHandlerBase(IAssetHandlerBase&&) = delete;
    IAssetHandlerBase& operator=(IAssetHandlerBase&&) = delete;

    /**
     * @brief Unloads a type-erased asset instance.
     *
     * Called by the asset system when an asset should be released
     * without knowing its concrete type.
     *
     * @param asset Type-erased shared pointer to the asset.
     */
    virtual void unloadUntyped(std::shared_ptr<void> asset, const std::string& path) = 0;
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

        ~IAssetHandler() override = default;

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
        virtual Result load(const std::string& path, LoadExecutionMode loadExecutionMode = LoadExecutionMode::Sync) = 0;

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

        /**
         * @brief Unloads a type-erased asset.
         *
         * Converts void pointer back to typed asset and forwards
         * it to typed unload implementation.
         */
        void unloadUntyped(std::shared_ptr<void> asset, const std::string& path) override {
            auto typed = std::static_pointer_cast<T>(asset);
            Handle handle(typed, path);
            unload(handle);
        }
};
} // namespace dzemikk

#endif // DZEMIKK_I_ASSET_HANDLE_H
