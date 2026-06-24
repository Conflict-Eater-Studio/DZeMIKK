#ifndef DZEMIKK_ENEMYTOOLTIP_H
#define DZEMIKK_ENEMYTOOLTIP_H
#include "ecs/components/monobehaviour.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiSlider.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "enemyTypes.h"

class Game;

namespace game {

    class EnemyTooltip : public dzemikk::MonoBehaviour {
    public:
          void showTooltip(game::EnemySpawnConfig config);
          void hideTooltip() const;
          void setNameText(dzemikk::UITextRenderer* text);
          void setHealthText(dzemikk::UITextRenderer* text);
          void setPersonalityText(dzemikk::UITextRenderer* text);
          void setTooltipGO(dzemikk::GameObject* game_object);
          void setImageRenderer(dzemikk::ImageRenderer* imageRenderer);
          void setGame(Game* game);
            void setSlider(dzemikk::UISlider* slider);
          std::string typeName() const override;

          private:
            dzemikk::GameObject* _tooltipGO = nullptr;
            dzemikk::UITextRenderer* _name = nullptr;
            dzemikk::UITextRenderer* _hp = nullptr;
            dzemikk::UITextRenderer* _personality = nullptr;
            dzemikk::ImageRenderer* _imageRenderer = nullptr;
            dzemikk::UISlider* _slider = nullptr;
            Game* _game = nullptr;

    };
}
#endif
