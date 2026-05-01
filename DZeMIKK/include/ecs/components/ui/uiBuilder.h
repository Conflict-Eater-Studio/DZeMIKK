#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiSlider.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
namespace dzemikk {
class UIBuilder {
  public:
    struct UIButtonParams {
        std::string name;
        glm::vec2 position = {0.0F, 0.0F};
        glm::vec2 size = {100.0F, 50.0F};
        glm::vec2 anchorMin = {0.0F, 0.0F};
        glm::vec2 anchorMax = {0.0F, 0.0F};
        glm::vec2 pivot = {0.5F, 0.5F};
        glm::vec2 scale = {1.0F, 1.0F};
        float rotation = 0.0F;
        std::string text;
        Font* textFont = nullptr;
        UITextRenderer::VerticalAlign textVAlign = UITextRenderer::VerticalAlign::Middle;
        UITextRenderer::HorizontalAlign textHAlign = UITextRenderer::HorizontalAlign::Center;
        glm::vec4 normalColor = {1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 hoverColor = {0.8F, 0.8F, 0.8F, 1.0F};
        glm::vec4 pressedColor = {0.6F, 0.6F, 0.6F, 1.0F};
        glm::vec4 textColor = {0.0F, 0.0F, 0.0F, 1.0F};
        Mesh* mesh = nullptr;
        Material* material = nullptr;
    };

    static GameObject* createButton(GameObject* parent, const UIButtonParams& params) {
        auto* scene = parent->getScene();
        if (scene == nullptr) {
            return nullptr;
        }

        auto* btnGO = scene->createGameObject(params.name, parent);
        btnGO->rectTransform()->setPosition(params.position);
        btnGO->rectTransform()->setSize(params.size);
        btnGO->rectTransform()->setAnchorMin(params.anchorMin);
        btnGO->rectTransform()->setAnchorMax(params.anchorMax);
        btnGO->rectTransform()->setPivot(params.pivot);
        btnGO->rectTransform()->setScale(params.scale);
        btnGO->rectTransform()->setRotation(params.rotation);
        btnGO->rectTransform()->setZIndex(1);

        auto* btn = btnGO->addComponent<UIButton>();
        btn->setStyle({.normalColor = params.normalColor,
                       .hoverColor = params.hoverColor,
                       .pressedColor = params.pressedColor});

        auto* image = btnGO->addComponent<ImageRenderer>();
        image->setRectTransform(btnGO->rectTransform());
        image->setMesh(params.mesh);
        image->setMaterial(params.material);
        image->setColor(params.normalColor);

        btn->setSpriteRenderer(image);

        auto* textGO = scene->createGameObject(params.name + "_Text", btnGO);
        auto* textRect = textGO->rectTransform();
        textRect->setSize({0, 0});
        textRect->setAnchorMin({0.0F, 0.0F});
        textRect->setAnchorMax({1.0F, 1.0F});
        textRect->setPivot({0.5F, 0.5F});
        textRect->setPosition({0.0F, 0.0F});
        textRect->setZIndex(2);

        auto* text = textGO->addComponent<UITextRenderer>();
        text->color = params.textColor;
        text->text = params.text;
        text->font = params.textFont;
        text->verticalAlign = params.textVAlign;
        text->horizontalAlign = params.textHAlign;

        return btnGO;
    }
};
} // namespace dzemikk
