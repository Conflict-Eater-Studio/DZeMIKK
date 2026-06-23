#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#pragma once
#include "ecs/components/ui/uiSlider.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace game {

class ObservableFloat {
  public:
    ObservableFloat() = default;
    explicit ObservableFloat(float value) : _value(value) {}

    ObservableFloat& operator=(float value) {
        if (_value != value) {
            _value = value;
            if (_callback) {
                _callback(_value);
            }
        }
        return *this;
    }

    ObservableFloat& operator+=(float value) {
        return *this = _value + value;
    }
    ObservableFloat& operator-=(float value) {
        return *this = _value - value;
    }
    ObservableFloat& operator*=(float value) {
        return *this = _value * value;
    }
    ObservableFloat& operator/=(float value) {
        return *this = _value / value;
    }

    operator float() const {
        return _value;
    }

    [[nodiscard]] float get() const {
        return _value;
    }

    void setCallback(std::function<void(float)> callback) {
        _callback = std::move(callback);
    }

  private:
    float _value{};
    std::function<void(float)> _callback;
};

class Settings {
  public:
    struct Audio {
        ObservableFloat masterVolume{1.0F};
        ObservableFloat musicVolume{1.0F};
        ObservableFloat ambientVolume{1.0F};
        ObservableFloat sfxVolume{1.0F};
        ObservableFloat uiVolume{1.0F};
    };

    struct AudioSliders {
        dzemikk::UISlider* masterVolume{nullptr};
        dzemikk::UISlider* musicVolume{nullptr};
        dzemikk::UISlider* ambientVolume{nullptr};
        dzemikk::UISlider* sfxVolume{nullptr};
        dzemikk::UISlider* uiVolume{nullptr};
    };

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

    static Settings& get() {
        static Settings instance;
        return instance;
    }

    [[nodiscard]] Audio& audio() {
        return _audio;
    }

    [[nodiscard]] const Audio& audio() const {
        return _audio;
    }

    void setAudioSliders(const AudioSliders& sliders) {
        _audioSliders = sliders;
        updateAudioSliders();
    }

    void setOnAudioChanged(const std::function<void()>& callback) {
        _onAudioChanged = callback;
    }

    void updateAudioSliders() const {
        if (_audioSliders.masterVolume) {
            _audioSliders.masterVolume->onValueChanged(_audio.masterVolume);
        }
        if (_audioSliders.musicVolume) {
            _audioSliders.musicVolume->onValueChanged(_audio.musicVolume);
        }
        if (_audioSliders.ambientVolume) {
            _audioSliders.ambientVolume->onValueChanged(_audio.ambientVolume);
        }
        if (_audioSliders.sfxVolume) {
            _audioSliders.sfxVolume->onValueChanged(_audio.sfxVolume);
        }
        if (_audioSliders.uiVolume) {
            _audioSliders.uiVolume->onValueChanged(_audio.uiVolume);
        }
    }

    void saveDefaults() {
        _audio.masterVolume = 1.0F;
        _audio.musicVolume = 1.0F;
        _audio.ambientVolume = 1.0F;
        _audio.sfxVolume = 1.0F;
        _audio.uiVolume = 1.0F;

        updateAudioSliders();
        save();
    }

    void read() {
        if (!std::filesystem::exists("settings.json")) {
            saveDefaults();
            return;
        }

        std::ifstream in("settings.json");
        nlohmann::json s = nlohmann::json::parse(in);
        in.close();

        _audio.masterVolume = s.value("masterVolume", 1.0F);
        _audio.musicVolume = s.value("musicVolume", 1.0F);
        _audio.ambientVolume = s.value("ambientVolume", 1.0F);
        _audio.sfxVolume = s.value("sfxVolume", 1.0F);
        _audio.uiVolume = s.value("uiVolume", 1.0F);

        updateAudioSliders();
    };

    void save() const {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[Settings] Saving settings to settings.json");
#endif
        nlohmann::json s = {
            {"masterVolume", _audio.masterVolume.get()},
            {"musicVolume", _audio.musicVolume.get()},
            {"ambientVolume", _audio.ambientVolume.get()},
            {"sfxVolume", _audio.sfxVolume.get()},
            {"uiVolume", _audio.uiVolume.get()},
        };

        std::ofstream out("settings.json");
        out << s.dump(4);
        out.close();
    }

  private:
    Settings() {
        auto onChanged = [this](float) {
            updateAudioSliders();
            if (_onAudioChanged) {
                _onAudioChanged();
            }
        };
        _audio.masterVolume.setCallback(onChanged);
        _audio.musicVolume.setCallback(onChanged);
        _audio.ambientVolume.setCallback(onChanged);
        _audio.sfxVolume.setCallback(onChanged);
        _audio.uiVolume.setCallback(onChanged);

        if (!std::filesystem::exists("settings.json")) {
            saveDefaults();
            return;
        }
        read();
    }

    ~Settings() = default;

    Audio _audio;
    AudioSliders _audioSliders;
    std::function<void()> _onAudioChanged;
};
} // namespace game

#endif // GAME_SETTINGS_H
