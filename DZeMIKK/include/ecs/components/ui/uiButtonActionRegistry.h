#ifndef DZEMIKK_UIBUTTONACTIONREGISTRY_H
#define DZEMIKK_UIBUTTONACTIONREGISTRY_H

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace dzemikk {
class UIButton;

class UIButtonActionRegistry {
  public:
    using ActionFactory = std::function<std::function<void()>(UIButton&)>;

    static UIButtonActionRegistry& get() {
        static UIButtonActionRegistry kRegistry;
        return kRegistry;
    }

    void registerAction(const std::string& actionId, ActionFactory actionFactory) {
        std::scoped_lock lock(_mutex);
        _factories[actionId] = std::move(actionFactory);
    }

    [[nodiscard]] bool hasAction(const std::string& actionId) const {
        std::scoped_lock lock(_mutex);
        return _factories.contains(actionId);
    }

    [[nodiscard]] std::function<void()> bind(const std::string& actionId, UIButton& button) const {
        ActionFactory actionFactory;

        {
            std::scoped_lock lock(_mutex);
            const auto iter = _factories.find(actionId);
            if (iter == _factories.end()) {
                return {};
            }

            actionFactory = iter->second;
        }

        return actionFactory(button);
    }

  private:
    UIButtonActionRegistry() = default;

    mutable std::mutex _mutex;
    std::unordered_map<std::string, ActionFactory> _factories;
};
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTONACTIONREGISTRY_H
