#include "core/profiler.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif

namespace dzemikk {

Profiler& Profiler::Get() {
    static Profiler instance;
    return instance;
}

void Profiler::initialize() {
    _cpuTimes.clear();
    _gpuQueries.clear();
}

void Profiler::uninitialize() {
    for (auto& pair : _gpuQueries) {
        if (pair.second.queryID != 0) {
            glDeleteQueries(1, &pair.second.queryID);
        }
    }
}

void Profiler::BeginFrame(float deltaTime) {
    currentDeltaTime = deltaTime;

    frameTimeHistory[frameHistoryOffset] = deltaTime;
    frameHistoryOffset = (frameHistoryOffset + 1) % FRAME_HISTORY_COUNT;

    stats.reset();
    _cpuTimes.clear();
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

void Profiler::DrawImGui() {
#if DZEMIKK_DEV_TOOLS
    ImGui::Begin("Performance Profiler");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", currentDeltaTime * 1000.0f,
                1.0f / currentDeltaTime);

    char overlayText[32];
    snprintf(overlayText, sizeof(overlayText), "%.1f FPS", 1.0f / currentDeltaTime);
    ImGui::PlotLines("##FrameTime", frameTimeHistory, FRAME_HISTORY_COUNT, frameHistoryOffset,
                     overlayText, 0.0f, FLT_MAX, ImVec2(0, 50));

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