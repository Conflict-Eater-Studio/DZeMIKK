#pragma once
#include "ecs/components/monoBehaviour.h"
#include "ecs/serialize/monoBehaviourSerializer.h"
#include "ecs/serialize/serializedRef.h"
#include "spdlog/spdlog.h"
#include "subCounter.h"

#include <nlohmann/json.hpp>

class Counter : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    [[nodiscard]] std::string typeName() const override {
        return "Counter";
    }

    int count = 0;

    dzemikk::SerializedRef<SubCounter> subCounterRef{*this};

    void update(double deltaTime) override {
        count++;
        if (count % 10 == 0) {
            subCounterRef->count++;
        }
        spdlog::info("Counter update: count={}, subCounter={}", count, subCounterRef->count);
    }
};

// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& json, const Counter& counter) {
    dzemikk::MonoBehaviourSerializer::writeBase(json, counter, "Counter");
    dzemikk::MonoBehaviourSerializer::writeRef(json, "subCounterRef", counter.subCounterRef);
    json["count"] = counter.count;
}

inline void from_json(const nlohmann::json& json, Counter& counter) {
    dzemikk::MonoBehaviourSerializer::readBase(json, counter, "Counter");
    dzemikk::MonoBehaviourSerializer::readRef(json, "subCounterRef", counter.subCounterRef);
    if (json.contains("count")) {
        counter.count = json["count"].get<int>();
    }
}
// NOLINTEND(readability-identifier-naming)