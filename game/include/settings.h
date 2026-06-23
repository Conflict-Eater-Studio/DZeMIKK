#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#pragma once
#include "ecs/components/colorGradingEffect.h"
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

    struct ColorGrading {
        ObservableFloat exposure{0.0F};
        ObservableFloat contrast{1.0F};
        ObservableFloat saturation{1.0F};
        ObservableFloat temperature{0.0F};
        ObservableFloat tint{0.0F};
        glm::vec3 colorFilter{1.0F};
    };

    struct ColorGradingSliders {
        dzemikk::UISlider* exposure{nullptr};
        dzemikk::UISlider* contrast{nullptr};
        dzemikk::UISlider* saturation{nullptr};
        dzemikk::UISlider* temperature{nullptr};
        dzemikk::UISlider* tint{nullptr};
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

    [[nodiscard]] ColorGrading& colorGrading() {
        return _colorGrading;
    }

    [[nodiscard]] const ColorGrading& colorGrading() const {
        return _colorGrading;
    }

    void setAudioSliders(const AudioSliders& sliders) {
        _audioSliders = sliders;
        updateAudioSliders();
    }

    void setOnAudioChanged(const std::function<void()>& callback) {
        _onAudioChanged = callback;
    }

    void setColorGradingEffect(dzemikk::ColorGradingEffect* effect) {
        _colorGradingEffect = effect;
        updateColorGrading();
    }

    void setColorGradingSliders(const ColorGradingSliders& sliders) {
        _colorGradingSliders = sliders;
        updateColorGradingSliders();
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

    void updateColorGrading() const {
        if (_colorGradingEffect == nullptr) {
            return;
        }
        _colorGradingEffect->setExposure(_colorGrading.exposure);
        _colorGradingEffect->setContrast(_colorGrading.contrast);
        _colorGradingEffect->setSaturation(_colorGrading.saturation);
        _colorGradingEffect->setTemperature(_colorGrading.temperature);
        _colorGradingEffect->setTint(_colorGrading.tint);
        _colorGradingEffect->setColorFilter(_colorGrading.colorFilter);
    }

    void updateColorGradingSliders() const {
        if (_colorGradingSliders.exposure) {
            _colorGradingSliders.exposure->onValueChanged(_colorGrading.exposure);
        }
        if (_colorGradingSliders.contrast) {
            _colorGradingSliders.contrast->onValueChanged(_colorGrading.contrast);
        }
        if (_colorGradingSliders.saturation) {
            _colorGradingSliders.saturation->onValueChanged(_colorGrading.saturation);
        }
        if (_colorGradingSliders.temperature) {
            _colorGradingSliders.temperature->onValueChanged(_colorGrading.temperature);
        }
        if (_colorGradingSliders.tint) {
            _colorGradingSliders.tint->onValueChanged(_colorGrading.tint);
        }
    }

    void saveDefaults() {
        _audio.masterVolume = 1.0F;
        _audio.musicVolume = 1.0F;
        _audio.ambientVolume = 1.0F;
        _audio.sfxVolume = 1.0F;
        _audio.uiVolume = 1.0F;

        _colorGrading.exposure = 0.0F;
        _colorGrading.contrast = 1.0F;
        _colorGrading.saturation = 1.0F;
        _colorGrading.temperature = 0.0F;
        _colorGrading.tint = 0.0F;
        _colorGrading.colorFilter = glm::vec3(1.0F);

        updateAudioSliders();
        updateColorGrading();
        updateColorGradingSliders();
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

        if (s.contains("colorGrading")) {
            const auto& cg = s["colorGrading"];
            _colorGrading.exposure = cg.value("exposure", 0.0F);
            _colorGrading.contrast = cg.value("contrast", 1.0F);
            _colorGrading.saturation = cg.value("saturation", 1.0F);
            _colorGrading.temperature = cg.value("temperature", 0.0F);
            _colorGrading.tint = cg.value("tint", 0.0F);
            if (cg.contains("colorFilter")) {
                _colorGrading.colorFilter =
                    glm::vec3(cg["colorFilter"][0].get<float>(), cg["colorFilter"][1].get<float>(),
                              cg["colorFilter"][2].get<float>());
            }
        }

        updateAudioSliders();
        updateColorGrading();
        updateColorGradingSliders();
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
            {"colorGrading",
             {
                 {"exposure", _colorGrading.exposure.get()},
                 {"contrast", _colorGrading.contrast.get()},
                 {"saturation", _colorGrading.saturation.get()},
                 {"temperature", _colorGrading.temperature.get()},
                 {"tint", _colorGrading.tint.get()},
                 {"colorFilter",
                  {_colorGrading.colorFilter.x, _colorGrading.colorFilter.y,
                   _colorGrading.colorFilter.z}},
             }},
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

        auto onColorGradingChanged = [this](float) {
            updateColorGrading();
            updateColorGradingSliders();
        };
        _colorGrading.exposure.setCallback(onColorGradingChanged);
        _colorGrading.contrast.setCallback(onColorGradingChanged);
        _colorGrading.saturation.setCallback(onColorGradingChanged);
        _colorGrading.temperature.setCallback(onColorGradingChanged);
        _colorGrading.tint.setCallback(onColorGradingChanged);

        if (!std::filesystem::exists("settings.json")) {
            saveDefaults();
            return;
        }
        read();
    }

    ~Settings() = default;

    Audio _audio;
    AudioSliders _audioSliders;
    ColorGrading _colorGrading;
    ColorGradingSliders _colorGradingSliders;
    dzemikk::ColorGradingEffect* _colorGradingEffect{nullptr};
    std::function<void()> _onAudioChanged;
};
} // namespace game

#endif // GAME_SETTINGS_H
