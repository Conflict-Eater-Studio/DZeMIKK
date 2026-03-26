#pragma once

namespace dzemikk {
    class IEngineModule {
      public:
        virtual ~IEngineModule() = default;
        virtual void Initialize() = 0;
        virtual void UnInitialize() = 0;
    };
} 