#pragma once

#include "ecs/components/monoBehaviour.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/monoBehaviourSerializer.h"

#include <nlohmann/json.hpp>

class SubCounter : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    [[nodiscard]] std::string typeName() const override {
        return "SubCounter";
    }

    int count = 0;
};

// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& json, const SubCounter& counter) {
    dzemikk::MonoBehaviourSerializer::writeBase(json, counter, "SubCounter");
    json["count"] = counter.count;
}

inline void from_json(const nlohmann::json& json, SubCounter& counter) {
    dzemikk::MonoBehaviourSerializer::readBase(json, counter, "SubCounter");

    if (json.contains("count") && json["count"].is_number_integer()) {
        counter.count = json["count"].get<int>();
    }
}
// NOLINTEND(readability-identifier-naming)