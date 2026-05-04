#include "assetManager/StateMachineHandler.h"
#include "animation/AnimationStateMachine.h"

#include <fstream>
#include <iostream>

namespace dzemikk {
    StateMachineHandler::Result StateMachineHandler::load(const std::string& path) {
        auto stateMachine = loadStateMachineFromFile(path);

        if (!stateMachine) {
            std::cerr << "Failed to load stateMachine: " << path << "\n";
            return {nullptr, AssetError::LoadFailed};
        }

        return {stateMachine, AssetError::None};
    }

    bool StateMachineHandler::reload(Handle& asset, const std::string& path) {
        if (!asset.isValid()) {
            std::cerr << "[StateMachineHandler] Cannot reload an invalid handle: " << path << std::endl;
            return false;
        }

        // Assuming your Handle class has a get() or arrow operator to access the underlying object
        return reloadStateMachine(path, *asset.get());
    }

    void StateMachineHandler::unload(Handle& asset) {
        // Assuming your Handle class has a method to release its reference to the shared_ptr
        asset.reset();
    }

    std::shared_ptr<AnimationStateMachine> StateMachineHandler::loadStateMachineFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[StateMachineHandler] Failed to open file: " << path << std::endl;
            return nullptr;
        }

        nlohmann::json jsonData;
        try {
            file >> jsonData;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "[StateMachineHandler] JSON Parse Error in " << path << ": " << e.what() << std::endl;
            return nullptr;
        }

        auto stateMachine = std::make_shared<AnimationStateMachine>();
        try {
            // Relies on the from_json() function defined in AnimationStateMachine.h
            from_json(jsonData, *stateMachine);
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "[StateMachineHandler] JSON Deserialization Error in " << path << ": " << e.what() << std::endl;
            return nullptr;
        }

        return stateMachine;
    }

    bool StateMachineHandler::reloadStateMachine(const std::string& path, AnimationStateMachine& stateMachine) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[StateMachineHandler] Failed to open file for reload: " << path << std::endl;
            return false;
        }

        nlohmann::json jsonData;
        try {
            file >> jsonData;

            // Re-populate the existing state machine object.
            // IMPORTANT: Ensure your from_json function completely clears the old
            // states before parsing the new ones, otherwise you might get duplicate states!
            from_json(jsonData, stateMachine);

            return true;
        } catch (const std::exception& e) {
            std::cerr << "[StateMachineHandler] Failed to reload state machine " << path << ": " << e.what() << std::endl;
            return false;
        }
    }

} // namespace dzemikk