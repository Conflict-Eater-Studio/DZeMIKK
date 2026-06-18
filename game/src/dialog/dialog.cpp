#include "dialog/dialog.h"

#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "events/mouse_event.h"
#include "game.h"

namespace game {
Dialog::Dialog(Game* game) : _game(game) {
    if (_game == nullptr) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[Dialog]: game pointer is null");
#endif
        throw std::runtime_error("[Dialog]: game pointer is null");
    }

    auto* scene = _game->getCurrentScene().get();
    _speakerTextRenderer = scene->findGameObjectByName("Dialogs")
                               ->findDescendantByName("SpeakerText")
                               ->getComponent<dzemikk::UITextRenderer>();
    _dialogTextRenderer = scene->findGameObjectByName("Dialogs")
                              ->findDescendantByName("DialogText")
                              ->getComponent<dzemikk::UITextRenderer>();

    if (_speakerTextRenderer == nullptr || _dialogTextRenderer == nullptr) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[Dialog]: Could not find UITextRenderer components for dialog");
#endif
        throw std::runtime_error("[Dialog]: Could not find UITextRenderer components for dialog");
    }

    _mouseButtonPressedListenerId =
        _game->getEngine()->getInput()->OnMouseButtonPressed.addListener(
            [this](dzemikk::MouseButtonPressedEvent& event) {
                if (_inDialog && event.GetMouseButton() == GLFW_MOUSE_BUTTON_LEFT) {
                    advance();
                }
            });
}

Dialog::~Dialog() {
    _game->getEngine()->getInput()->OnMouseButtonPressed.removeListener(
        _mouseButtonPressedListenerId);
}

void Dialog::update(float dt) {
    if (!_inDialog || _currentText.empty()) {
        return;
    }

    if (_currentTime >= _textSpeed && _displayedText.size() < _currentText.size()) {
        _displayedText += _currentText[_displayedText.size()];
        _dialogTextRenderer->text = _displayedText;
        _currentTime -= _textSpeed;
    } else if (_displayedText.size() < _currentText.size()) {
        _currentTime += dt;
    } else {
        _currentTime = 0.0F;
    }
}

void Dialog::start() {
    if (_entriesPlan.empty()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[Dialog]: No entries to start dialog");
#endif
        return;
    }
    _entries = _entriesPlan;
    _inDialog = true;
    _triggered = true;
    _currentTime = 0.0F;

    _currentText = _entries.front().text;
    _currentSpeaker = _entries.front().speaker;
    _entries.pop();
    _displayedText.clear();

    _speakerTextRenderer->text = _currentSpeaker;
    _dialogTextRenderer->text = "";

    _game->getCurrentScene().get()->findGameObjectByName("Dialogs")->enabled(true);
}

void Dialog::advance() {
    if (_entries.empty() && _currentText.size() == _displayedText.size()) {
        _displayedText.clear();
        _currentText.clear();
        _currentSpeaker.clear();
        _dialogTextRenderer->text = "";
        _speakerTextRenderer->text = "";

        _game->getCurrentScene().get()->findGameObjectByName("Dialogs")->enabled(false);
    } else {
        if (_currentText.size() != _displayedText.size()) {
            _displayedText = _currentText;
            _dialogTextRenderer->text = _displayedText;
            return;
        }

        _displayedText.clear();
        _currentText = _entries.front().text;
        _currentSpeaker = _entries.front().speaker;
        _entries.pop();
        _currentTime = 0.0F;

        _speakerTextRenderer->text = _currentSpeaker;
        _dialogTextRenderer->text = "";
    }
}
} // namespace game
