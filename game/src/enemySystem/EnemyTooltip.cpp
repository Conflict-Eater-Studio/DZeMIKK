#include "enemySystem/EnemyTooltip.h"

#include "assetManager/assetmanager.h"
#include "ecs/gameobject.h"
#include "game.h"

#if DZEMIKK_DEV_TOOLS
#include "spdlog/spdlog.h"
#endif
void game::EnemyTooltip::showTooltip(EnemySpawnConfig config) {
    _tooltipGO->enabled(true);
    _hp->text = std::to_string(config.hp) + "/" + std::to_string(config.hp);
    _slider->onValueChanged(1.0f);
    switch (config.personality) {
    case EnemyPersonality::Aggressive:
        _personality->text = "AGGRESSIVE";
        _personality->color = glm::vec3(1.0F, 0.0F, 0.0F);
        _imageRenderer->setTexture(
            _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/avatar4.png"));
        break;
    case EnemyPersonality::Balanced:
        _personality->text = "BALANCED";
        _personality->color = glm::vec3(0.48F, 0.48F, 0.48F);
        _imageRenderer->setTexture(
            _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/avatar3.png"));
        break;
    case EnemyPersonality::Defensive:
        _personality->text = "DEFENSIVE";
        _personality->color = glm::vec3(0.0F, 0.0F, 1.0F);
        _imageRenderer->setTexture(
        _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
        "textures/ui grafiki/avatary/avatar2.png"));
        break;
        break;
    }
}
void game::EnemyTooltip::hideTooltip() const {
    _tooltipGO->enabled(false);
}
void game::EnemyTooltip::setNameText(dzemikk::UITextRenderer* text) {
    _name = text;
}
void game::EnemyTooltip::setHealthText(dzemikk::UITextRenderer* text) {
    _hp = text;
}
void game::EnemyTooltip::setPersonalityText(dzemikk::UITextRenderer* text) {
    _personality = text;
}
void game::EnemyTooltip::setTooltipGO(dzemikk::GameObject* gameObject) {
    _tooltipGO = gameObject;
}
void game::EnemyTooltip::setImageRenderer(dzemikk::ImageRenderer* imageRenderer) {
    _imageRenderer = imageRenderer;
}
void game::EnemyTooltip::setGame(Game* game) {
    _game = game;
}
void game::EnemyTooltip::setSlider(dzemikk::UISlider* slider) {
    _slider = slider;
}
std::string game::EnemyTooltip::typeName() const {
    return "EnemyTooltip";
}