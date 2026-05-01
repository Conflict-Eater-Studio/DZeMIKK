#pragma once
#ifndef DZEMIKK_STATEMACHINE_HANDLER_H
#define DZEMIKK_STATEMACHINE_HANDLER_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class AnimationStateMachine;

    /**
     * @brief Handles loading, reloading, and unloading of AnimationStateMachine assets.
     *
     * StateMachineHandler parses JSON definitions from disk to construct the graph
     * of AnimationStates and Transitions. Loaded state machines are wrapped in
     * AnimationStateMachine objects and managed by the engine's asset system.
     *
     * Supports hot-reloading (useful for tweaking animation graphs at runtime)
     * and safe lifetime management via AssetHandle.
     */
    class StateMachineHandler : public IAssetHandler<AnimationStateMachine> {
      public:
        using Handle = AssetHandle<AnimationStateMachine>;
        using Result = AssetResult<AnimationStateMachine>;

        /**
         * @brief Loads an animation state machine from a JSON file.
         *
         * @param path Path to the state machine JSON file.
         * @return AssetResult containing a valid AnimationStateMachine handle or error.
         */
        Result load(const std::string& path) override;

        /**
         * @brief Reloads an existing state machine.
         *
         * Re-parses the JSON file and reconstructs the underlying states and transitions.
         *
         * @param asset Reference to the state machine handle.
         * @param path Path to the JSON file.
         * @return True if reload succeeded.
         */
        bool reload(Handle& asset, const std::string& path) override;

        /**
         * @brief Unloads a state machine from memory.
         *
         * @param asset State machine handle to unload.
         */
        void unload(Handle& asset) override;

      private:
        /**
         * @brief Loads state machine data from a JSON file.
         *
         * Reads the file and deserializes the states, clips, and transitions.
         *
         * @param path Path to the JSON file.
         * @return Shared pointer to AnimationStateMachine or nullptr on failure.
         */
        static std::shared_ptr<AnimationStateMachine> loadStateMachineFromFile(const std::string& path);

        /**
         * @brief Reloads state machine data into an existing instance.
         *
         * Clears the current states and rebuilds them from the updated JSON file.
         *
         * @param path Path to the JSON file.
         * @param stateMachine Reference to the AnimationStateMachine instance.
         * @return True if reload succeeded.
         */
        static bool reloadStateMachine(const std::string& path, AnimationStateMachine& stateMachine);
    };

} // namespace dzemikk

#endif // DZEMIKK_STATEMACHINE_HANDLER_H