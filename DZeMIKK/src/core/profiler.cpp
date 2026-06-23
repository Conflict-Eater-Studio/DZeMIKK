#include "core/profiler.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace dzemikk {

Profiler& Profiler::Get() {
    static Profiler instance;
    return instance;
}

void Profiler::initialize() {
    _cpuTimes.clear();
    _gpuQueries.clear();

    _validHistorySamples = 0;
    _recordFrameIndex = 0;
    _isRecording = false;

    for (int i = 0; i < MAX_FRAME_HISTORY_COUNT; ++i) {
        frameTimeHistory[i] = 0.0f;
        _rollingAverageHistory[i] = 0.0f;
    }
}

void Profiler::uninitialize() {
    StopRecording();

    for (auto& pair : _gpuQueries) {
        if (pair.second.queryID != 0) {
            glDeleteQueries(1, &pair.second.queryID);
        }
    }
}

void Profiler::BeginFrame(float deltaTime) {
    currentDeltaTime = deltaTime;

    int writeIndex = frameHistoryOffset;
    frameTimeHistory[writeIndex] = deltaTime;
    if (_validHistorySamples < MAX_FRAME_HISTORY_COUNT) {
        ++_validHistorySamples;
    }

    int window = std::min(_frameHistoryCount, _validHistorySamples);
    float sum = 0.0f;
    for (int i = 0; i < window; ++i) {
        int idx = (writeIndex - i + MAX_FRAME_HISTORY_COUNT) % MAX_FRAME_HISTORY_COUNT;
        sum += frameTimeHistory[idx];
    }
    _rollingAverageHistory[writeIndex] = (window > 0) ? (sum / static_cast<float>(window)) : deltaTime;

    frameHistoryOffset = (frameHistoryOffset + 1) % MAX_FRAME_HISTORY_COUNT;

    stats.reset();
    _cpuTimes.clear();

    if (_isRecording && _recordStream.is_open()) {
        float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
        _recordStream << _recordFrameIndex << ',' << std::fixed << std::setprecision(6) << deltaTime << ','
                      << std::fixed << std::setprecision(3) << (deltaTime * 1000.0f) << ','
                      << std::fixed << std::setprecision(2) << fps << '\n';
        ++_recordFrameIndex;
    }
}

void Profiler::EndFrame() {
    for (auto& pair : _gpuQueries) {
        if (pair.second.active && pair.second.queryID != 0) {
            GLuint available = 0;
            glGetQueryObjectuiv(pair.second.queryID, GL_QUERY_RESULT_AVAILABLE, &available);
            if (available) {
                GLuint64 timeElapsed = 0;
                glGetQueryObjectui64v(pair.second.queryID, GL_QUERY_RESULT, &timeElapsed);
                pair.second.timeMs = static_cast<float>(timeElapsed) / 1000000.0f; // ns to ms
                pair.second.active = false;
            }
        }
    }
}

void Profiler::AddCPUTime(const std::string& name, float ms) {
    _cpuTimes[name] += ms;
}

void Profiler::BeginGPUQuery(const std::string& name) {
    if (_gpuQueries.find(name) == _gpuQueries.end() || _gpuQueries[name].queryID == 0) {
        glGenQueries(1, &_gpuQueries[name].queryID);
    }

    if (!_gpuQueries[name].active) {
        glBeginQuery(GL_TIME_ELAPSED, _gpuQueries[name].queryID);
    }
}

void Profiler::EndGPUQuery(const std::string& name) {
    if (_gpuQueries.find(name) != _gpuQueries.end() && !_gpuQueries[name].active) {
        glEndQuery(GL_TIME_ELAPSED);
        _gpuQueries[name].active = true;
    }
}

void Profiler::StartRecording(const std::string& filename) {
    if (_isRecording) {
        return;
    }

    _recordFilename = filename;
    if (_recordFilename.empty()) {
        std::time_t now = std::time(nullptr);
        std::tm localTime{};
#if defined(_WIN32)
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        std::ostringstream oss;
        oss << "fps_capture_" << std::put_time(&localTime, "%Y%m%d_%H%M%S") << ".csv";
        _recordFilename = oss.str();
    }

    _recordStream.open(_recordFilename, std::ios::out | std::ios::trunc);
    if (_recordStream.is_open()) {
        _recordStream << "FrameIndex,DeltaTime(s),FrameTime(ms),FPS\n";
        _recordFrameIndex = 0;
        _isRecording = true;
    }
}

void Profiler::StopRecording() {
    if (_recordStream.is_open()) {
        _recordStream.close();
    }
    _isRecording = false;
    _recordFrameIndex = 0;
    _recordFilename.clear();
}

Profiler::FrameTimeStats Profiler::_ComputeFrameStats() const {
    FrameTimeStats result;
    if (_validHistorySamples <= 0) {
        return result;
    }

    int sampleCount = std::min(_frameHistoryCount, _validHistorySamples);
    std::vector<float> samples;
    samples.reserve(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        int idx = (frameHistoryOffset - sampleCount + i + MAX_FRAME_HISTORY_COUNT) % MAX_FRAME_HISTORY_COUNT;
        samples.push_back(frameTimeHistory[idx]);
    }

    float sum = 0.0f;
    float minDt = samples[0];
    float maxDt = samples[0];
    for (float dt : samples) {
        sum += dt;
        minDt = std::min(minDt, dt);
        maxDt = std::max(maxDt, dt);
    }
    float avgDt = sum / static_cast<float>(samples.size());

    std::vector<float> sorted = samples;
    std::sort(sorted.begin(), sorted.end());

    auto percentile = [&](float p) -> float {
        size_t idx = static_cast<size_t>(std::ceil(p * static_cast<float>(sorted.size()))) - 1;
        if (idx >= sorted.size()) {
            idx = sorted.size() - 1;
        }
        return sorted[idx];
    };

    float p99 = percentile(0.99f);
    float p999 = percentile(0.999f);

    result.avgMs = avgDt * 1000.0f;
    result.minMs = minDt * 1000.0f;
    result.maxMs = maxDt * 1000.0f;
    result.percentile1LowMs = p99 * 1000.0f;
    result.percentile01LowMs = p999 * 1000.0f;

    result.avgFps = (avgDt > 0.0f) ? (1.0f / avgDt) : 0.0f;
    result.minFps = (maxDt > 0.0f) ? (1.0f / maxDt) : 0.0f;
    result.maxFps = (minDt > 0.0f) ? (1.0f / minDt) : 0.0f;
    result.onePercentLowFps = (p99 > 0.0f) ? (1.0f / p99) : 0.0f;
    result.zeroOnePercentLowFps = (p999 > 0.0f) ? (1.0f / p999) : 0.0f;

    return result;
}

void Profiler::DrawImGui() {
#if DZEMIKK_DEV_TOOLS
    ImGui::Begin("Performance Profiler");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", currentDeltaTime * 1000.0f,
                1.0f / currentDeltaTime);

    if (_isRecording) {
        if (ImGui::Button("Stop Recording", ImVec2(150, 0))) {
            StopRecording();
        }
        ImGui::SameLine();
        ImGui::Text("Recording %llu frames to %s", static_cast<unsigned long long>(_recordFrameIndex),
                    _recordFilename.c_str());
    } else {
        if (ImGui::Button("Start Recording", ImVec2(150, 0))) {
            StartRecording();
        }
    }

    ImGui::Separator();

    FrameTimeStats frameStats = _ComputeFrameStats();
    ImGui::Text("Frame Time (ms)  Avg: %.3f  Min: %.3f  Max: %.3f  1%% Low: %.3f  0.1%% Low: %.3f",
                frameStats.avgMs, frameStats.minMs, frameStats.maxMs, frameStats.percentile1LowMs,
                frameStats.percentile01LowMs);
    ImGui::Text("FPS            Avg: %.1f  Min: %.1f  Max: %.1f  1%% Low: %.1f  0.1%% Low: %.1f",
                frameStats.avgFps, frameStats.minFps, frameStats.maxFps, frameStats.onePercentLowFps,
                frameStats.zeroOnePercentLowFps);

    ImGui::Separator();

    ImGui::SliderInt("Frame History / Rolling Avg", &_frameHistoryCount, 1, MAX_FRAME_HISTORY_COUNT, "%d frames");

    char overlayText[32];
    snprintf(overlayText, sizeof(overlayText), "%.1f FPS", 1.0f / currentDeltaTime);

    float frameTimeMs[MAX_FRAME_HISTORY_COUNT] = {0.0f};
    float rollingAvgMs[MAX_FRAME_HISTORY_COUNT] = {0.0f};

    int displayStart = (frameHistoryOffset - _frameHistoryCount + MAX_FRAME_HISTORY_COUNT) % MAX_FRAME_HISTORY_COUNT;
    for (int i = 0; i < _frameHistoryCount; ++i) {
        int idx = (displayStart + i) % MAX_FRAME_HISTORY_COUNT;
        frameTimeMs[i] = frameTimeHistory[idx] * 1000.0f;
        rollingAvgMs[i] = _rollingAverageHistory[idx] * 1000.0f;
    }

    ImGui::PlotLines("Frame Time (ms)", frameTimeMs, _frameHistoryCount, 0, overlayText, 0.0f, FLT_MAX,
                     ImVec2(0, 80));
    ImGui::PlotLines("Rolling Average (ms)", rollingAvgMs, _frameHistoryCount, 0, nullptr, 0.0f, FLT_MAX,
                     ImVec2(0, 60));

    ImGui::Separator();

    ImGui::Text("--- CPU Timers (ms) ---");
    for (const auto& pair : _cpuTimes) {
        ImGui::Text("%-30s: %.3f", pair.first.c_str(), pair.second);
    }

    ImGui::Separator();
    ImGui::Text("--- GPU Timers (ms) ---");
    for (const auto& pair : _gpuQueries) {
        ImGui::Text("%-30s: %.3f", pair.first.c_str(), pair.second.timeMs);
    }

    ImGui::Separator();
    ImGui::Text("--- Renderer Stats ---");
    ImGui::Text("Draw Calls:      %u", stats.drawCalls);
    ImGui::Text("Objects:         %u", stats.renderedObjects);
    ImGui::Text("Triangles:       %u", stats.triangleCount);
    ImGui::Text("Vertices:        %u", stats.vertexCount);
    ImGui::Text("State Changes:   %u", stats.stateChanges);

    ImGui::End();
#endif
}

ScopeCPUTimer::ScopeCPUTimer(const std::string& name) : _name(name) {
    _startTime = glfwGetTime();
}

ScopeCPUTimer::~ScopeCPUTimer() {
    float elapsedMs = static_cast<float>((glfwGetTime() - _startTime) * 1000.0);
    Profiler::Get().AddCPUTime(_name, elapsedMs);
}

ScopeGPUTimer::ScopeGPUTimer(const std::string& name) : _name(name) {
    Profiler::Get().BeginGPUQuery(_name);
}

ScopeGPUTimer::~ScopeGPUTimer() {
    Profiler::Get().EndGPUQuery(_name);
}
} // namespace dzemikk
