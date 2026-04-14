#ifndef DZEMIKK_ASSET_HANDLE_H 
#define DZEMIKK_ASSET_HANDLE_H 

#include <memory>

namespace dzemikk {
template <typename T> class AssetHandle {
    std::weak_ptr<T> ptr_;

  public:
    AssetHandle() = default;
    AssetHandle(const std::shared_ptr<T>& ptr) : ptr_(ptr) {}

    T* get() const {
        auto sp = ptr_.lock();
        return sp ? sp.get() : nullptr;
    }

    explicit operator bool() const {
        return !ptr_.expired();
    }
};
} // namespace dzemikk
#endif // DZEMIKK_ASSET_HANDLE__H