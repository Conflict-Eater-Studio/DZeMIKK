#pragma once
#include "core/iEngineModule.h"

#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

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

    // CSV recording. Pass an empty filename to auto-generate "fps_capture_YYYYMMDD_HHMMSS.csv".
    void StartRecording(const std::string& filename = "");
    void StopRecording();
    bool IsRecording() const {
        return _isRecording;
    }

    RendererStats stats;
    float currentDeltaTime = 0.0f;

    static const int FRAME_HISTORY_COUNT = 2500;
    float frameTimeHistory[FRAME_HISTORY_COUNT] = {0.0f};
    int frameHistoryOffset = 0;

  private:
    Profiler() = default;
    ~Profiler() = default;

    struct FrameTimeStats {
        float avgMs = 0.0f;
        float minMs = 0.0f;
        float maxMs = 0.0f;
        float percentile1LowMs = 0.0f;
        float percentile01LowMs = 0.0f;
        float avgFps = 0.0f;
        float minFps = 0.0f;
        float maxFps = 0.0f;
        float onePercentLowFps = 0.0f;
        float zeroOnePercentLowFps = 0.0f;
    };

    FrameTimeStats _ComputeFrameStats() const;

    std::map<std::string, float> _cpuTimes;

    struct GPUQueryData {
        uint32_t queryID = 0;
        float timeMs = 0.0f;
        bool active = false;
    };
    std::map<std::string, GPUQueryData> _gpuQueries;

    float _rollingAverageHistory[FRAME_HISTORY_COUNT] = {0.0f};
    int _validHistorySamples = 0;
    int _rollingAverageWindow = 30;

    bool _isRecording = false;
    std::ofstream _recordStream;
    uint64_t _recordFrameIndex = 0;
    std::string _recordFilename;
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
