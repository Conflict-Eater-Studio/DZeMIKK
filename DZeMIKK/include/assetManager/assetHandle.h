#ifndef DZEMIKK_ASSET_HANDLE_H 
#define DZEMIKK_ASSET_HANDLE_H 

#include <memory>
#include <string>

namespace dzemikk {
/**
 * @brief Non-owning handle to an asset.
 *
 * Wraps a std::weak_ptr to avoid extending the lifetime of the asset
 * while still allowing safe access if the asset is still alive.
 *
 * @tparam T Asset type.
 */
template <typename T> class AssetHandle {
  public:
    using AssetPath = std::string;

    AssetHandle() = default;
    ~AssetHandle() = default;

    AssetHandle(const AssetHandle&) = default;
    AssetHandle& operator=(const AssetHandle&) = default;
    AssetHandle(AssetHandle&&) noexcept = default;
    AssetHandle& operator=(AssetHandle&&) noexcept = default;

   /**
    * @brief Constructs handle from shared pointer.
    */
    explicit AssetHandle(const std::shared_ptr<T>& ptr, const std::string& path)
        : _ptr(ptr), _path(std::make_shared<AssetPath>(path)) {}

    /**
     * @brief Attempts to get raw pointer to the asset.
     *
     * @return T* Pointer if asset is still alive, otherwise nullptr.
     */
    [[nodiscard]] T* get() const {
        if (auto sp = _ptr.lock()) {
            return sp.get();
        }
        return nullptr;
    }

    /**
     * @brief Checks whether the asset is still valid.
     */
    [[nodiscard]] bool isValid() const {
        return !_ptr.expired();
    }

    /**
     * @brief Boolean conversion operator.
     */
    explicit operator bool() const {
        return isValid();
    }

    /**
     * @brief Returns asset path associated with this handle.
     *
     * @return Path string.
     */
    [[nodiscard]] std::string getAssetPath() const {
        return *_path;
    }

  private:
    /**
     * @brief Weak reference to the asset.
     *
     * Does not affect asset lifetime.
     */
    std::weak_ptr<T> _ptr;

    /**
     * @brief Shared storage of asset path.
     *
     * Shared to allow cheap copying of handles.
     */
    std::shared_ptr<AssetPath> _path;
};
} // namespace dzemikk
#endif // DZEMIKK_ASSET_HANDLE__H