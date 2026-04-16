#ifndef DZEMIKK_ASSET_LOADER_REGISTRY_H
#define DZEMIKK_ASSET_LOADER_REGISTRY_H

#include "iAssetHandler.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace dzemikk {

/**
 * @brief Registry for asset handlers.
 *
 * Maps asset types to their corresponding handlers, enabling
 * type-safe retrieval and centralized management of asset loading logic.
 */
class AssetLoaderRegistry {
  public:
    AssetLoaderRegistry() = default;
    ~AssetLoaderRegistry() = default;

    AssetLoaderRegistry(const AssetLoaderRegistry&) = delete;
    AssetLoaderRegistry& operator=(const AssetLoaderRegistry&) = delete;
    AssetLoaderRegistry(AssetLoaderRegistry&&) noexcept = delete;
    AssetLoaderRegistry& operator=(AssetLoaderRegistry&&) noexcept = delete;

    /**
     * @brief Retrieves a handler for a given asset type.
     *
     * @tparam T Asset type.
     * @return IAssetHandler<T>* Pointer to handler or nullptr if not registered.
     */
    template <typename T> 
    IAssetHandler<T>* get() const{
        auto it = _handlers.find(typeid(T));
        if (it == _handlers.end()) {
            return nullptr;
        }

        return static_cast<IAssetHandler<T>*>(it->second.get());
    }
    
    /**
     * @brief Registers a handler for a given asset type.
     *
     * @tparam T Asset type.
     * @param handler Unique pointer to handler instance.
     */
    template <typename T> 
    void registerHandler(std::unique_ptr<IAssetHandler<T>> handler) {
        if (!handler) {
            return;
        }

        _handlers[typeid(T)] = std::move(handler);
    }

    /**
     * @brief Removes all registered handlers.
     */
    void clear();

    /**
     * @brief Retrieves a handler using runtime type information.
     *
     * @param type Type index of the asset.
     * @return IAssetHandlerBase* Pointer to handler or nullptr if not found.
     */
    [[nodiscard]] IAssetHandlerBase* getByType(std::type_index type) const;

    /**
     * @brief Checks whether a handler exists for a given type.
     */
    [[nodiscard]] bool contains(std::type_index type) const;

  private:
    /**
     * @brief Internal storage of handlers.
     *
     * Uses type_index as key to associate each asset type
     * with its corresponding handler instance.
     */
    std::unordered_map<std::type_index, std::unique_ptr<IAssetHandlerBase>> _handlers;
};

} // namespace dzemikk

#endif //DZEMIKK_ASSET_LOADER_REGISTRY_H