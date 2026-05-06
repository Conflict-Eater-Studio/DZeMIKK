#include "assetManager/assetmanager.h"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiCheckbox.h"
#include "ecs/components/ui/uiDropdown.h"
#include "ecs/components/ui/uiSlider.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <algorithm>

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
        AssetHandle<Font> textFont;
        UITextRenderer::VerticalAlign textVAlign = UITextRenderer::VerticalAlign::Middle;
        UITextRenderer::HorizontalAlign textHAlign = UITextRenderer::HorizontalAlign::Center;
        glm::vec4 normalColor = {1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 hoverColor = {0.8F, 0.8F, 0.8F, 1.0F};
        glm::vec4 pressedColor = {0.6F, 0.6F, 0.6F, 1.0F};
        glm::vec4 textColor = {0.0F, 0.0F, 0.0F, 1.0F};
        AssetHandle<Mesh> mesh;
        std::shared_ptr<dzemikk::Material> material = nullptr;
    };

    struct UISliderParams {
        std::string name;
        glm::vec2 position = {0.0F, 0.0F};
        glm::vec2 size = {200.0F, 20.0F};
        glm::vec2 anchorMin = {0.0F, 0.0F};
        glm::vec2 anchorMax = {0.0F, 0.0F};
        glm::vec2 pivot = {0.5F, 0.5F};
        glm::vec2 scale = {1.0F, 1.0F};
        float rotation = 0.0F;
        glm::vec4 bgColor = {1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 fillColor = {0.2F, 0.6F, 1.0F, 1.0F};
        glm::vec2 handleSize = {20.0F, 20.0F};
        glm::vec4 handleColor = {1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 handleHoverColor = {0.8F, 0.8F, 0.8F, 1.0F};
        glm::vec4 hadnlePressedColor = {0.6F, 0.6F, 0.6F, 1.0F};
        AssetHandle<Mesh> bgMesh;
        AssetHandle<Mesh> fillMesh;
        AssetHandle<Mesh> handleMesh;
        std::shared_ptr<dzemikk::Material> bgMat = nullptr;
        std::shared_ptr<dzemikk::Material> fillMat = nullptr;
        std::shared_ptr<dzemikk::Material> handleMat = nullptr;
    };

    struct UICheckboxParams {
        std::string name;
        glm::vec2 position = {0.0F, 0.0F};
        glm::vec2 size = {200.0F, 20.0F};
        glm::vec2 anchorMin = {0.0F, 0.0F};
        glm::vec2 anchorMax = {0.0F, 0.0F};
        glm::vec2 pivot = {0.5F, 0.5F};
        glm::vec2 scale = {1.0F, 1.0F};
        glm::vec4 normalColor = {1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 hoverColor = {0.8F, 0.8F, 0.8F, 1.0F};
        glm::vec4 pressedColor = {0.6F, 0.6F, 0.6F, 1.0F};
        glm::vec4 checkmarkColor = {0.2F, 0.6F, 1.0F, 1.0F};
        float rotation = 0.0F;
        AssetHandle<Mesh> bgMesh;
        AssetHandle<Mesh> checkmarkMesh;
        std::shared_ptr<dzemikk::Material> bgMat = nullptr;
        std::shared_ptr<dzemikk::Material> checkmarkMat = nullptr;
    };

    struct UIDropdownParams {
        std::string name;
        glm::vec2 position = {0.0F, 0.0F};
        glm::vec2 size = {200.0F, 20.0F};
        glm::vec2 anchorMin = {0.0F, 0.0F};
        glm::vec2 anchorMax = {0.0F, 0.0F};
        glm::vec2 pivot = {0.5F, 0.5F};
        glm::vec2 scale = {1.0F, 1.0F};
        float rotation = 0.0F;
        std::vector<UIDropdown::Option> options;
        std::size_t selectedIndex = 0;
        float optionHeight = 30.0F;
        std::size_t maxVisibleOptions = 5;
        std::string text;
        AssetHandle<Font> textFont;
        glm::vec4 textColor = {0.0F, 0.0F, 0.0F, 1.0F};
        UITextRenderer::VerticalAlign textVAlign = UITextRenderer::VerticalAlign::Middle;
        UITextRenderer::HorizontalAlign textHAlign = UITextRenderer::HorizontalAlign::Center;
        glm::vec4 normalColor = {1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 hoverColor = {0.8F, 0.8F, 0.8F, 1.0F};
        glm::vec4 pressedColor = {0.6F, 0.6F, 0.6F, 1.0F};
        glm::vec4 highlightColor = {0.2F, 0.6F, 1.0F, 1.0F};
        glm::vec4 arrowColor = {1.0F, 1.0F, 1.0F, 1.0F};
        AssetHandle<Mesh> bgMesh;
        AssetHandle<Mesh> arrowMesh;
        AssetHandle<Mesh>optionMesh;
        AssetHandle<Mesh> optionsBgMesh;
        AssetHandle<Mesh> scrollbarMesh;
        std::shared_ptr<dzemikk::Material> bgMat = nullptr;
        std::shared_ptr<dzemikk::Material> arrowMat = nullptr;
        std::shared_ptr<dzemikk::Material> optionMat = nullptr;
        std::shared_ptr<dzemikk::Material> optionsBgMat = nullptr;
        std::shared_ptr<dzemikk::Material> scrollBarMat = nullptr;
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
        btn->setTextGO(textGO);
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
        text->fontAsset = params.textFont;
        text->font = params.textFont.get();
        text->verticalAlign = params.textVAlign;
        text->horizontalAlign = params.textHAlign;

        return btnGO;
    }

    static GameObject* createSlider(GameObject* parent, const UISliderParams& params) {
        auto* scene = parent->getScene();
        if (scene == nullptr) {
            return nullptr;
        }

        auto* sliderGO = scene->createGameObject(params.name, parent);
        sliderGO->rectTransform()->setSize(params.size);
        sliderGO->rectTransform()->setPosition(params.position);
        sliderGO->rectTransform()->setAnchorMin(params.anchorMin);
        sliderGO->rectTransform()->setAnchorMax(params.anchorMax);
        sliderGO->rectTransform()->setPivot(params.pivot);
        sliderGO->rectTransform()->setZIndex(1);

        auto* slider = sliderGO->addComponent<UISlider>();

        auto* background = sliderGO->addComponent<ImageRenderer>();
        background->setRectTransform(sliderGO->rectTransform());
        background->setColor(params.bgColor);
        background->setMesh(params.bgMesh);
        background->setMaterial(params.bgMat);

        auto* fillGO = scene->createGameObject(params.name + "_Fill", sliderGO);
        auto* fillRect = fillGO->rectTransform();
        fillRect->setSize({0.0F, 0.0F});
        fillRect->setAnchorMin({0.0F, 0.0F});
        fillRect->setAnchorMax({1.0F, 1.0F});
        fillRect->setPivot({0.5F, 0.5F});
        fillRect->setPosition({0.0F, 0.0F});
        fillRect->setZIndex(2);

        auto* fill = fillGO->addComponent<ImageRenderer>();
        fill->setRectTransform(fillRect);
        fill->setMesh(params.fillMesh);
        fill->setMaterial(params.fillMat);

        auto* handleGO = scene->createGameObject(params.name + "_Handle", sliderGO);
        auto* handleRect = handleGO->rectTransform();
        handleRect->setSize(params.handleSize);
        handleRect->setAnchorMin({0.5F, 0.5F});
        handleRect->setAnchorMax({0.5F, 0.5F});
        handleRect->setPivot({0.5F, 0.5F});
        handleRect->setPosition({0.0F, 0.0F});
        handleRect->setZIndex(3);

        auto* handle = handleGO->addComponent<ImageRenderer>();
        handle->setRectTransform(handleRect);
        handle->setMesh(params.handleMesh);
        handle->setMaterial(params.handleMat);

        slider->setHandleSpriteRenderer(handle);
        slider->setBackgroundSpriteRenderer(background);
        slider->setFillSpriteRenderer(fill);

        slider->onValueChanged(0.0F);

        return sliderGO;
    };

    static GameObject* createCheckbox(GameObject* parent, const UICheckboxParams& params) {
        auto* scene = parent->getScene();
        if (scene == nullptr) {
            return nullptr;
        }

        auto* go = scene->createGameObject(params.name, parent);
        auto* crect = go->rectTransform();
        crect->setSize(params.size);
        crect->setPosition(params.position);
        crect->setAnchorMin(params.anchorMin);
        crect->setAnchorMax(params.anchorMax);
        crect->setPivot(params.pivot);
        crect->setScale(params.scale);
        crect->setRotation(params.rotation);
        crect->setZIndex(1);

        auto* background = go->addComponent<ImageRenderer>();
        background->setMesh(params.bgMesh);
        background->setMaterial(params.bgMat);
        background->setRectTransform(crect);
        background->setColor(params.normalColor);

        auto* checkmarkGO = scene->createGameObject(params.name + "_Checkmark", go);
        auto* checkmarkRect = checkmarkGO->rectTransform();
        checkmarkRect->setSize({0.0F, 0.0F});
        checkmarkRect->setAnchorMin({0.1F, 0.1F});
        checkmarkRect->setAnchorMax({0.9F, 0.9F});
        checkmarkRect->setPivot({0.5F, 0.5F});
        checkmarkRect->setPosition({0.0F, 0.0F});
        checkmarkRect->setZIndex(2);

        auto* checkmark = checkmarkGO->addComponent<ImageRenderer>();
        checkmark->setMesh(params.checkmarkMesh);
        checkmark->setMaterial(params.checkmarkMat);
        checkmark->setRectTransform(checkmarkRect);
        checkmark->setColor(params.checkmarkColor);

        auto* checkbox = go->addComponent<UICheckbox>();
        checkbox->setStyle({.normalColor = params.normalColor,
                            .hoverColor = params.hoverColor,
                            .pressedColor = params.pressedColor,
                            .checkmarkColor = params.checkmarkColor});
        checkbox->setBackgroundSpriteRenderer(background);
        checkbox->setCheckmarkSpriteRenderer(checkmark);

        return go;
    }

    static GameObject* createDropdown(GameObject* parent, const UIDropdownParams& params) {
        auto* scene = parent->getScene();
        if (scene == nullptr) {
            return nullptr;
        }

        // --- Main trigger button ---
        auto* go = createButton(parent, {.name = params.name,
                                         .position = params.position,
                                         .size = params.size,
                                         .anchorMin = params.anchorMin,
                                         .anchorMax = params.anchorMax,
                                         .pivot = params.pivot,
                                         .scale = params.scale,
                                         .rotation = params.rotation,
                                         .text = params.text,
                                         .textFont = params.textFont,
                                         .textVAlign = params.textVAlign,
                                         .textHAlign = params.textHAlign,
                                         .normalColor = params.normalColor,
                                         .hoverColor = params.hoverColor,
                                         .pressedColor = params.pressedColor,
                                         .textColor = params.textColor,
                                         .mesh = params.bgMesh,
                                         .material = params.bgMat});
        auto* dtrigger = go->getComponent<UIButton>();
        auto* drect = go->rectTransform();

        auto* dropdown = go->addComponent<UIDropdown>();
        dropdown->setTriggerGO(go);

        auto dtriggerOnClick = boost::uuids::to_string(boost::uuids::random_generator()());
        UIActionRegistry::get().registerAction(
            [dropdown](const UIEvent& event) { dropdown->toggle(); }, dtriggerOnClick);
        dtrigger->addEventListener(UIEventType::Click, dtriggerOnClick);

        // --- Construct options ---
        UIDropdown::OptionRender optionRender{
            .height = params.optionHeight,
            .mesh = params.optionMesh,
            .material = params.optionMat,
            .font = params.textFont,
            .textVAlign = params.textVAlign,
            .textHAlign = params.textHAlign,
        };
dropdown->setOptions(params.options);
        dropdown->setSelectedIndex(params.selectedIndex);
        dropdown->setOptionRender(optionRender);

        // --- Construct background for options ---
        auto* optsBgGo = scene->createGameObject(params.name + "_OptionsBG", go);
        dropdown->setOptionsContainerGO(optsBgGo);
        auto* optsBgGoRect = optsBgGo->rectTransform();
        optsBgGoRect->setSize({
            params.size[0],
            params.optionHeight *
                static_cast<float>(std::min(params.maxVisibleOptions, params.options.size())),
        });
        optsBgGoRect->setAnchorMin({0.0F, 1.0F});
        optsBgGoRect->setAnchorMax({0.0F, 1.0F});
        optsBgGoRect->setPivot({0.0F, 1.0F});
        optsBgGoRect->setPosition({0.0F, -go->rectTransform()->getSize()[1]});
        optsBgGoRect->setZIndex(2);

        auto* optsBg = optsBgGo->addComponent<ImageRenderer>();
        optsBg->setMesh(params.optionsBgMesh);
        optsBg->setMaterial(params.optionsBgMat);
        optsBg->setRectTransform(optsBgGoRect);
        optsBg->setColor(params.normalColor);

        dropdown->setStyle({
            .normalColor = params.normalColor,
            .hoverColor = params.hoverColor,
            .pressedColor = params.pressedColor,
            .arrowColor = params.arrowColor,
            .textColor = params.textColor,
            .normalOptColor = params.normalColor,
            .hoverOptColor = params.hoverColor,
            .pressedOptColor = params.pressedColor,
            .highlightOptColor = params.highlightColor,
        });
        dropdown->updateOptionVisuals();

        return go;
    }
};
} // namespace dzemikk
