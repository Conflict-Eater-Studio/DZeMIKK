#pragma once

namespace dzemikk {
class IGpuUploadable {
  public:
    virtual void uploadToGPU() = 0;
    virtual ~IGpuUploadable() = default;
};
}
