#ifndef DZEMIKK_ASSET_ERROR_H
#define DZEMIKK_ASSET_ERROR_H

namespace dzemikk {
/**
 * @brief Represents possible errors that can occur during asset operations.
 *
 * This enumeration defines standard error codes used by the asset system
 * to indicate why loading, reloading, or processing of an asset has failed.

 *
 * @note AssetError::None indicates a successful operation (no error occurred).
 */
enum class AssetError {
    FileNotFound,
    LoadFailed,
    InvalidFormat,
    ReloadFailed,
    OutOfMemory,
    Unknown,
    None
};

} // namespace dzemikk

#endif // DZEMIKK_ASSET_ERROR_H