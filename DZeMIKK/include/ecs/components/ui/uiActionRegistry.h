#ifndef DZEMIKK_UIACTIONREGISTRY_H
#define DZEMIKK_UIACTIONREGISTRY_H

#include "ecs/components/ui/uiEvent.h"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace dzemikk {
class UIActionRegistry {
  public:
    using Action = std::function<void(const UIEvent&)>;

    static UIActionRegistry& get() {
        static UIActionRegistry instance;
        return instance;
    }

    void registerAction(Action action, const std::string& actionId) {
        std::scoped_lock lock(_mutex);
        _actions[actionId] = std::move(action);
    }

    [[nodiscard]] bool hasAction(const std::string& actionId) const {
        std::scoped_lock lock(_mutex);
        return _actions.contains(actionId);
    }

    [[nodiscard]] bool invoke(const std::string& actionId, const UIEvent& event) const {
        Action action;
        {
            std::scoped_lock lock(_mutex);
            const auto iter = _actions.find(actionId);
            if (iter == _actions.end()) {
                return false;
            }

            action = iter->second;
        }

        if (action == nullptr) {
            return false;
        }

        action(event);
        return true;
    }

    bool unregisterAction(const std::string& actionId) {
        std::scoped_lock lock(_mutex);
        return _actions.erase(actionId) > 0;
    }

    void clear() {
        std::scoped_lock lock(_mutex);
        _actions.clear();
    }

  private:
    UIActionRegistry() = default;

    mutable std::mutex _mutex;
    std::unordered_map<std::string, Action> _actions;
};
} // namespace dzemikk

#endif // DZEMIKK_UIACTIONREGISTRY_H
