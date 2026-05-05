#ifndef DZEMIKK_I_GPU_UPLOADABLE_H
#define DZEMIKK_I_GPU_UPLOADABLE_H

namespace dzemikk {
/**
 * @brief Interface for resources that require GPU upload.
 *
 * Used for assets that need to transfer data from CPU memory
 * to GPU memory (e.g. textures, meshes, buffers).
 */
class IGpuUploadable {
  public:
    /**
     * @brief Uploads resource data to GPU.
     *
     * @note Must be called on render/main thread with valid GPU context.
     */
    virtual void uploadToGPU() = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~IGpuUploadable() = default;

    IGpuUploadable() = default;

    IGpuUploadable(const IGpuUploadable&) = delete;
    IGpuUploadable& operator=(const IGpuUploadable&) = delete;

    IGpuUploadable(IGpuUploadable&&) = delete;
    IGpuUploadable& operator=(IGpuUploadable&&) = delete;
};
} // namespace dzemikk
#endif // DZEMIKK_I_GPU_UPLOADABLE_H