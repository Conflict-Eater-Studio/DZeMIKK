#ifndef DZEMIKK_ASSET_HANDLE_H 
#define DZEMIKK_ASSET_HANDLE_H 

#include <memory>

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
    AssetHandle() = default;
    ~AssetHandle() = default;

    AssetHandle(const AssetHandle&) = default;
    AssetHandle& operator=(const AssetHandle&) = default;
    AssetHandle(AssetHandle&&) noexcept = default;
    AssetHandle& operator=(AssetHandle&&) noexcept = default;

   /**
    * @brief Constructs handle from shared pointer.
    */
    explicit AssetHandle(const std::shared_ptr<T>& ptr) : _ptr(ptr) {}

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

  private:
    /**
     * @brief Weak reference to the asset.
     *
     * Does not affect asset lifetime.
     */
    std::weak_ptr<T> _ptr;
};
} // namespace dzemikk
#endif // DZEMIKK_ASSET_HANDLE__H