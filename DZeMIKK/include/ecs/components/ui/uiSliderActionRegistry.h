#ifndef DZEMIKK_UISLIDERACTIONREGISTRY_H
#define DZEMIKK_UISLIDERACTIONREGISTRY_H

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace dzemikk {
class UISlider;

class UISliderActionRegistry {
  public:
    using ActionFactory = std::function<std::function<void()>(UISlider&)>;
    using ValueChangedActionFactory = std::function<std::function<void(float)>(UISlider&)>;

    static UISliderActionRegistry& get() {
        static UISliderActionRegistry kRegistry;
        return kRegistry;
    }

    void registerAction(const std::string& actionId, ActionFactory actionFactory) {
        std::scoped_lock lock(_mutex);
        _factories[actionId] = std::move(actionFactory);
    }

    void registerValueChangedAction(const std::string& actionId,
                                    ValueChangedActionFactory actionFactory) {
        std::scoped_lock lock(_mutex);
        _valueChangedFactories[actionId] = std::move(actionFactory);
    }

    [[nodiscard]] bool hasAction(const std::string& actionId) const {
        std::scoped_lock lock(_mutex);
        return _factories.contains(actionId);
    }

    [[nodiscard]] bool hasValueChangedAction(const std::string& actionId) const {
        std::scoped_lock lock(_mutex);
        return _valueChangedFactories.contains(actionId);
    }

    [[nodiscard]] std::function<void()> bind(const std::string& actionId, UISlider& slider) const {
        ActionFactory actionFactory;

        {
            std::scoped_lock lock(_mutex);
            const auto iter = _factories.find(actionId);
            if (iter == _factories.end()) {
                return {};
            }

            actionFactory = iter->second;
        }

        return actionFactory(slider);
    }

    [[nodiscard]] std::function<void(float)> bindValueChanged(const std::string& actionId,
                                                              UISlider& slider) const {
        ValueChangedActionFactory actionFactory;

        {
            std::scoped_lock lock(_mutex);
            const auto iter = _valueChangedFactories.find(actionId);
            if (iter == _valueChangedFactories.end()) {
                return {};
            }

            actionFactory = iter->second;
        }

        return actionFactory(slider);
    }

  private:
    UISliderActionRegistry() = default;

    mutable std::mutex _mutex;
    std::unordered_map<std::string, ActionFactory> _factories;
    std::unordered_map<std::string, ValueChangedActionFactory> _valueChangedFactories;
};
} // namespace dzemikk

#endif // DZEMIKK_UISLIDERACTIONREGISTRY_H
