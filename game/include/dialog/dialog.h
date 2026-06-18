#ifndef GAME_DIALOG_H
#define GAME_DIALOG_H
#pragma once

#include "input/input.h"

#include <nlohmann/json.hpp>
#include <queue>
#include <string>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

class Game;

namespace dzemikk {
class UITextRenderer;
}

namespace game {
class Dialog {
  public:
    struct Entry {
        std::string speaker;
        std::string text;
    };

    Dialog(Game* game);
    Dialog(const Dialog&) = delete;
    Dialog& operator=(const Dialog&) = delete;
    Dialog(Dialog&&) = delete;
    Dialog& operator=(Dialog&&) = delete;
    ~Dialog();

    void enqueuePlan(const std::string& speaker, const std::string& text) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[Dialog]: Enqueuing plan");
        _entriesPlan.push({.speaker = speaker, .text = text});
#endif
    }
    void clearPlan() {
        _entriesPlan = std::queue<Entry>();
    }

    void start();

    [[nodiscard]] bool isTriggered() const {
        return _triggered;
    }
    void setTriggered(bool triggered) {
        _triggered = triggered;
    }
    void update(float dt);
    void advance();

  private:
    Game* _game;
    dzemikk::UITextRenderer* _speakerTextRenderer{nullptr};
    dzemikk::UITextRenderer* _dialogTextRenderer{nullptr};

    std::queue<Entry> _entriesPlan;
    std::queue<Entry> _entries;
    float _textSpeed = 0.025F; // [s/character]
    float _currentTime = 0.0F;
    std::string _currentText;
    std::string _currentSpeaker;
    std::string _displayedText;

    bool _inDialog = false;
    bool _triggered = false;

    dzemikk::ListenerID _mouseButtonPressedListenerId;
};

// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const Dialog::Entry& entry) {
    j = nlohmann::json{{"speaker", entry.speaker}, {"text", entry.text}};
}

inline void from_json(const nlohmann::json& j, Dialog::Entry& entry) {
    if (!j.contains("speaker") || !j.contains("text")) {
        throw std::runtime_error("Missing required fields in Dialog::Entry JSON");
    }
    entry.speaker = j.at("speaker").get<std::string>();
    entry.text = j.at("text").get<std::string>();
}
// NOLINTEND(readability-identifier-naming)

} // namespace game

#endif // GAME_DIALOG_H
