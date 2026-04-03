#pragma once
#include <cstdint>

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

    class Profiler {
      public:
        inline static RendererStats rendererStats;

        static void resetFrame() {
            rendererStats.reset();
        }
    };
}
