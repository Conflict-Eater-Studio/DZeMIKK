#ifndef TUL_PBL_DZEMIKK_ANIMATIONSTATESERIALIZER_H
#define TUL_PBL_DZEMIKK_ANIMATIONSTATESERIALIZER_H
#include "animation/animationstate.h"
#include "ecs/serialize/uuid.h"

namespace dzemikk {
    inline void to_json(nlohmann::json& json, const AnimationState& state) {
        json["name"] = state.getName();
        json["animation"] = state.getClip();
        json["transitions"] = state.getTransitions();
    }

    void from_json(const nlohmann::json& j, AnimationState& s) {
    }
}
#endif //TUL_PBL_DZEMIKK_ANIMATIONSTATESERIALIZER_H
