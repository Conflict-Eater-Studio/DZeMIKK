#ifndef DZEMIKK_ASSET_HANDLE_H 
#define DZEMIKK_ASSET_HANDLE_H 

#include <memory>

namespace dzemikk {
/** 
* @brief Handle for engine-managed assets. 
* 
* Wraps std::shared_ptr to provide safe access to assets without exposing ownership. 
* 
* @tparam T Type of the asset. 
*/
template <typename T> class AssetHandle {
  public: 
    /** 
    * @brief Default constructor creating empty handle. 
    */
    AssetHandle() = default;

    /** 
    * @brief Constructs handle from shared pointer. 
    * 
    * @param ptr Shared pointer to asset. 
    */
    explicit AssetHandle(T* ptr) : ptr_(ptr) {}
    
    /** 
    * @brief Returns raw pointer to asset. 
    * 
    * @return Pointer to managed asset or nullptr if invalid. 
    */
    T* get() const {
        return ptr_;
    }
    
    /** 
    * @brief Dereference operator for direct access. 
    * 
    * @return Reference to asset. 
    */
    T& operator*() const {
        return *ptr_;
    }
    
    /** 
    * @brief Arrow operator for member access. 
    *
    * @return Pointer to asset. 
    */
    T* operator->() const {
        return ptr_;
    }
    
    /** 
    * @brief Checks if handle is valid.
    * 
    * @return True if asset exists, false otherwise. 
    */
    bool valid() const {
        return ptr_ != nullptr;
    }

  private:
    T* ptr_ = nullptr;
};
} // namespace dzemikk
#endif // DZEMIKK_ASSET_HANDLE__H