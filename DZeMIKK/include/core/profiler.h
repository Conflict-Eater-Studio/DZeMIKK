#pragma once
#include "core/iEngineModule.h"

#include <cstdint>
#include <map>
#include <string>

namespace dzemikk {
struct RendererStats {
    uint32_t drawCalls = 0;
    uint32_t renderedObjects = 0;
    uint32_t triangleCount = 0;
    uint32_t vertexCount = 0;
    uint32_t stateChanges = 0;

    void reset() {
        drawCalls = 0;
        renderedObjects = 0;
        triangleCount = 0;
        vertexCount = 0;
        stateChanges = 0;
    }
};

class Profiler : public IEngineModule {
  public:
    void initialize() override;
    void uninitialize() override;

    static Profiler& Get();

    void BeginFrame(float deltaTime);
    void EndFrame();
    void DrawImGui();

    void AddCPUTime(const std::string& name, float ms);

    // GPU Queries handling
    void BeginGPUQuery(const std::string& name);
    void EndGPUQuery(const std::string& name);

    RendererStats stats;
    float currentDeltaTime = 0.0f;

    bool enableFrustumCulling = true;
    bool enableInstancing = true;

    static const int FRAME_HISTORY_COUNT = 150;
    float frameTimeHistory[FRAME_HISTORY_COUNT] = {0.0f};
    int frameHistoryOffset = 0;

  private:
    Profiler() = default;
    ~Profiler() = default;

    std::map<std::string, float> _cpuTimes;

    struct GPUQueryData {
        uint32_t queryID = 0;
        float timeMs = 0.0f;
        bool active = false;
    };
    std::map<std::string, GPUQueryData> _gpuQueries;
};

class ScopeCPUTimer {
  public:
    ScopeCPUTimer(const std::string& name);
    ~ScopeCPUTimer();

  private:
    std::string _name;
    double _startTime;
};

class ScopeGPUTimer {
  public:
    ScopeGPUTimer(const std::string& name);
    ~ScopeGPUTimer();

  private:
    std::string _name;
};
} // namespace dzemikk

#define PROFILE_CONCAT_(a, b) a##b
#define PROFILE_CONCAT(a, b) PROFILE_CONCAT_(a, b)

#define DZ_PROFILE_CPU(name) dzemikk::ScopeCPUTimer PROFILE_CONCAT(__cpu_timer_, __LINE__)(name)
#define DZ_PROFILE_GPU(name) dzemikk::ScopeGPUTimer PROFILE_CONCAT(__gpu_timer_, __LINE__)(name)