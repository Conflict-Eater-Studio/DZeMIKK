#pragma once

namespace dzemikk {
    class IEngineModule {
      public:
        IEngineModule() = default;
        virtual ~IEngineModule() = default;

        IEngineModule(const IEngineModule&) = delete;
        IEngineModule& operator=(const IEngineModule&) = delete;

        IEngineModule(IEngineModule&&) = delete;
        IEngineModule& operator=(IEngineModule&&) = delete;

        virtual void Initialize() = 0;
        virtual void UnInitialize() = 0;
    };
} 