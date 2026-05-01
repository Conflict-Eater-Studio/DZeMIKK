#pragma once
#ifndef DZEMIKK_ANIMATIONSTATEMACHINE_H
#define DZEMIKK_ANIMATIONSTATEMACHINE_H

#include "nlohmann/json.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace dzemikk {

class AnimationClip;
class AnimationState;

    /**
     * @brief A data-driven asset that defines animation states and transitions.
     * 
     * This class acts purely as a blueprint. It holds the graph of states 
     * but does NOT track runtime execution (like current time or current state).
     * Runtime playback is handled by the Animator component.
     */
    class AnimationStateMachine {
    public:
        AnimationStateMachine() = default;
        ~AnimationStateMachine() = default;
        
        // Prevent copying to ensure this remains a unique asset in memory
        AnimationStateMachine(const AnimationStateMachine&) = delete;
        AnimationStateMachine& operator=(const AnimationStateMachine&) = delete;

        /**
         * @brief Adds a new, unnamed state to the machine.
         */
        AnimationState* addState();
        
        /**
         * @brief Adds a new named state to the machine.
         * @param name The unique identifier for the state.
         */
        AnimationState* addState(const std::string& name);

        /**
         * @brief Retrieves a state by its name. Used by the Animator component.
         * @param stateName The name of the state to look up.
         * @return Pointer to the state, or nullptr if it doesn't exist.
         */
        [[nodiscard]] AnimationState* getState(const std::string& stateName) const;

        /**
         * @brief Read-only access to all states (useful for serialization or UI).
         */
        [[nodiscard]] const std::unordered_map<std::string, std::unique_ptr<AnimationState>>& getStates() const;

    private:
        std::unordered_map<std::string, std::unique_ptr<AnimationState>> _states;
    };

    inline void to_json(nlohmann::json& json, const AnimationStateMachine& animationStateMachine) {
        json["states"] = nlohmann::json::object();
        
        for (const auto& [name, statePtr] : animationStateMachine.getStates()) {
            if (statePtr) {
                json["states"][name] = *statePtr;
            }
        }
    }

    inline void from_json(const nlohmann::json& json, AnimationStateMachine& animationStateMachine) {
        // To be implemented: 
        // 1. Iterate over json["states"]
        // 2. Call animationStateMachine.addState(name)
        // 3. Deserialize the JSON data into that new state
    }

} // namespace dzemikk

#endif