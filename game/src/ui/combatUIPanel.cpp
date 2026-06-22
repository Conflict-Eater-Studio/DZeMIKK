#include "ui/combatUIPanel.h"

#include "ecs/components/ui/uiTextRenderer.h"
#include "enemySystem/enemyPatternComponent.h"
#include "player/playerPatternComponent.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <collisions/collisions.h>
#include <core/engine.h>
#include <core/window.h>
#include <ecs/components/collider.h>
#include <ecs/components/ui/gridLayout.h>
#include <ecs/components/ui/imageRenderer.h>
#include <ecs/components/ui/uiActionRegistry.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/serialize/prefabSerializer.h>
#include <renderer/renderer.h>
#include <renderer/texture.h>

void game::CombatUIPanel::start() {
    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");
    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");
    _patternsContainer = _canvas->findDescendantByName("Patterns");
    _scrollListenerId = _engine->getInput()->OnMouseScrolled.addListener(
        [this](dzemikk::MouseScrolledEvent& e) { onMouseScrolled(e); });
    _scrollHandle = getOwner()->getScene()->findGameObjectByName("Scroll");
    _patternPool.reserve(kMaxVisiblePatterns);

    for (int i = 0; i < kMaxVisiblePatterns; i++) {
        auto* obj = createPatternSlotObject();
        PatternPoolObject poolObj{
            .button = obj->getComponent<dzemikk::UIButton>(),
            .root = obj,
            .countText =
                obj->findDescendantByName("Text_Count")->getComponent<dzemikk::UITextRenderer>(),
            .nameText =
                obj->findDescendantByName("Button_Text")->getComponent<dzemikk::UITextRenderer>(),
            .previewContainer = obj->findDescendantByName("HexHolder"),
            .borderRenderer =
                obj->findDescendantByName("Empty")->getComponent<dzemikk::ImageRenderer>(),
        };

        for (int j = 0; j < 8; j++) {
            auto* hexGO = dzemikk::PrefabSerializer::instantiate(
                *getOwner()->getScene(), *_hexUIPrefab.get(), _assetManager);
            hexGO->enabled(false);
            hexGO->setParent(poolObj.previewContainer);
            poolObj.hexPool.emplace_back(hexGO, false);
        }

        obj->enabled(false);
        _patternPool.emplace_back(poolObj);
        setupButtonActions(_patternPool.back(), i);
    }

    refresh();
}

void game::CombatUIPanel::update(double deltaTime) {
    std::ranges::for_each(_patternPool, [](PatternPoolObject& obj) {
        if (!obj.used && obj.root->isEnabled()) {
            obj.root->enabled(false);
        }
        std::ranges::for_each(obj.hexPool, [](std::pair<dzemikk::GameObject*, bool>& hexPair) {
            if (!hexPair.second && hexPair.first->isEnabled()) {
                hexPair.first->enabled(false);
            }
        });
    });
}

void game::CombatUIPanel::onDestroy() {
    _engine->getInput()->OnMouseScrolled.removeListener(_scrollListenerId);
}

void game::CombatUIPanel::refresh(bool enableChildren) {
    if (!_patterns) {
        return;
    }

    buildUI();
    refreshCounts();

    for (auto* child : getOwner()->getChildren()) {
        if (enableChildren) {
            child->enabled(true);
        }
    }
}

void game::CombatUIPanel::buildUI() {
    if (!_patterns || !_patternsContainer) {
        return;
    }

    std::ranges::for_each(_patternPool, [](PatternPoolObject& obj) {
        obj.root->enabled(false);
        obj.used = false;
        std::ranges::for_each(obj.hexPool, [](std::pair<dzemikk::GameObject*, bool>& hexPair) {
            hexPair.second = false;
        });
    });

    if (_hideEmptyPatterns) {
        const auto indices = getAvailablePatternIndices();
        for (size_t patternIndex : indices) {
            createPatternSlot(_patterns->getPatterns()[patternIndex], patternIndex);
        }
    } else {
        const auto& patterns = _patterns->getPatterns();
        const size_t start = _firstVisiblePatternIndex;
        const size_t end = std::min(start + kMaxVisiblePatterns, patterns.size());
        for (size_t i = start; i < end; ++i) {
            createPatternSlot(patterns[i], i);
        }
    }

    auto* grid = _patternsContainer->getComponent<dzemikk::GridLayout>();
    if (grid) {
        grid->rebuild();
    }
}

void game::CombatUIPanel::createPatternSlot(const PatternComponent::PatternEntry& entry,
                                            size_t index) {
#if DZEMIKK_DEV_TOOLS
    auto tStart = std::chrono::steady_clock::now();
    auto tPhaseStart = tStart;
#endif

    auto poolObjIt =
        std::ranges::find_if(_patternPool, [](const PatternPoolObject& obj) { return !obj.used; });
    if (poolObjIt == _patternPool.end()) {
        return;
    }
    poolObjIt->used = true;
    poolObjIt->patternIndex = index;

    const uint32_t usageCount = getPatternCount(entry);
    const auto color = applyUsageTint(getPatternBaseColor(entry.pattern.getType()), usageCount);

    setupButton(*poolObjIt, color);
    setupPatternSlotContent(*poolObjIt, entry.pattern, usageCount);
}

void game::CombatUIPanel::createPatternPreview(game::CombatUIPanel::PatternPoolObject& obj,
                                               const HexPattern& pattern) {
    constexpr float hexSize = 15.0F;

    std::vector<glm::vec2> positions;

    for (const auto& hex : pattern.getHexes()) {
        float x =
            hexSize * std::numbers::sqrt3_v<float> * ((float)hex.q() + ((float)hex.r() * 0.5F));
        float y = hexSize * 1.5F * (float)hex.r();
        positions.emplace_back(-y, x);
    }

    glm::vec2 minPos(FLT_MAX);
    glm::vec2 maxPos(-FLT_MAX);

    for (auto& p : positions) {

        minPos.x = std::min(minPos.x, p.x);
        minPos.y = std::min(minPos.y, p.y);

        maxPos.x = std::max(maxPos.x, p.x);
        maxPos.y = std::max(maxPos.y, p.y);
    }

    glm::vec2 center = (minPos + maxPos) * 0.5F;

    std::string texturePath;
    if (pattern.getType() == HexPattern::Type::ATK) {
        texturePath = "textures/ui grafiki/ui patterns/atak.png";
    } else if (pattern.getType() == HexPattern::Type::DEF) {
        texturePath = "textures/ui grafiki/ui patterns/tarcza.png";
    } else if (pattern.getType() == HexPattern::Type::HEAL) {
        texturePath = "textures/ui grafiki/ui patterns/leczenie.png";
    } else if (pattern.getType() == HexPattern::Type::BONUSHEX) {
        texturePath = "textures/ui grafiki/ui patterns/dodatkowy_hex.png";
    }

    for (size_t i = 0; i < obj.hexPool.size(); ++i) {
        if (i >= positions.size()) {
            obj.hexPool[i].second = false;
            continue;
        }
        obj.hexPool[i].second = true;

        auto* transform = obj.hexPool[i].first->rectTransform();
        transform->setPosition(glm::vec3(positions[i] - center, 0.0F));

        auto* renderer = obj.hexPool[i].first->getComponent<dzemikk::ImageRenderer>();
        glm::vec4 color = getPatternBaseColor(pattern.getType());
        renderer->setColor({color.r, color.g, color.b, 1.0F});

        auto* hexChild = obj.hexPool[i].first->findChildByName("Empty");
        auto* hexChildRenderer = hexChild->getComponent<dzemikk::ImageRenderer>();

        if (texturePath != "") {
            hexChildRenderer->setTexture(_assetManager->get<dzemikk::Texture>(texturePath));
        } else {
            hexChildRenderer->enabled(false);
        }
    }
}

void game::CombatUIPanel::refreshCounts() {
    auto active = _patternPool |
                  std::ranges::views::filter([](const PatternPoolObject& obj) { return obj.used; });

    for (auto& entry : active) {
        if (!entry.countText) {
            continue;
        }

        const auto& patternEntry = _patterns->getPatterns()[entry.patternIndex];
        int32_t count = getPatternCount(patternEntry);
        entry.countText->text = "0/" + std::to_string(count);

        if (_mode == Mode::AvailablePatterns) {
            int32_t maxCount = patternEntry.maxCount;
            entry.countText->text = std::to_string(count) + "/" + std::to_string(maxCount);
        }
    }
}

void game::CombatUIPanel::setHideEmptyPatterns(bool value) {
    _hideEmptyPatterns = value;
}

std::string game::CombatUIPanel::buildPatternName(const HexPattern& pattern) {

    switch (pattern.getType()) {

    case HexPattern::Type::ATK:
        return "Attack";

    case HexPattern::Type::DEF:
        return "Defense";

    case HexPattern::Type::HEAL:
        return "Heal";

    case HexPattern::Type::BONUSHEX:
        return "Bonus";

    default:
        return "Pattern";
    }
}

glm::vec4 game::CombatUIPanel::getPatternBaseColor(HexPattern::Type type) {
    switch (type) {
    case HexPattern::Type::ATK:
        return {1.F, 0.F, 0.F, 0.55F};
    case HexPattern::Type::DEF:
        return {0.F, 0.F, 1.F, 0.55F};
    case HexPattern::Type::HEAL:
        return {0.F, 1.F, 0.F, 0.55F};
    case HexPattern::Type::BONUSHEX:
        return {1.0F, 0.84F, 0.0F, 0.55F};
    default:
        return {0.3F, 0.3F, 0.3F, 0.55F};
    }
}

glm::vec4 game::CombatUIPanel::applyUsageTint(glm::vec4 base, uint32_t count) {
    // float intensity = std::min(1.0F, (float)count * 0.2F);

    // base.r += intensity * 0.3F;
    // base.g += intensity * 0.3F;
    // base.b += intensity * 0.3F;

    return base;
}

std::string game::CombatUIPanel::typeName() const {
    return "CombatUIPanel";
}

void game::CombatUIPanel::setPatternsComponent(PatternComponent* patterns) {
    _patterns = patterns;
}

void game::CombatUIPanel::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::CombatUIPanel::setCanvas(dzemikk::GameObject* canvas) {
    _canvas = canvas;
}

void game::CombatUIPanel::setMode(Mode mode) {
    _mode = mode;
    refresh();
}

int32_t game::CombatUIPanel::getPatternCount(const PatternComponent::PatternEntry& entry) const {
    if (!_patterns) {
        return 0;
    }

    if (_mode == Mode::AvailablePatterns) {
        return entry.count;
    }

    const auto* enemyPatterns = dynamic_cast<const EnemyPatternComponent*>(_patterns);

    if (!enemyPatterns) {
        return 0;
    }

    const auto& usage = enemyPatterns->getPatternUsage();

    auto it = usage.find(&entry.pattern);

    if (it == usage.end()) {
        return 0;
    }

    return (int32_t)it->second;
}

dzemikk::GameObject* game::CombatUIPanel::createPatternSlotObject() {
    auto* patternGO = dzemikk::PrefabSerializer::instantiate(
        *getOwner()->getScene(), *_patternSlotPrefab.get(), _assetManager);

    patternGO->setParent(_patternsContainer);
    patternGO->enabled(false);

    return patternGO;
}

uint32_t game::CombatUIPanel::getUsageCount(const PatternComponent::PatternEntry& entry) const {
    if (!_patterns) {
        return 0;
    }

    auto* enemyPatterns = dynamic_cast<game::EnemyPatternComponent*>(_patterns);

    if (!enemyPatterns) {
        return 0;
    }

    const auto& usage = enemyPatterns->getPatternUsage();

    auto it = usage.find(&entry.pattern);

    return it != usage.end() ? it->second : 0;
}

void game::CombatUIPanel::setupButtonActions(game::CombatUIPanel::PatternPoolObject& obj,
                                             size_t poolIndex) {
    if (!obj.button || !_isClickable) {
        return;
    }

    const std::string actionId = "pattern_pool_" + std::to_string(poolIndex);
    const std::string hoverAction = actionId + "_hover";
    const std::string unhoverAction = actionId + "_unhover";

    dzemikk::UIActionRegistry::get().registerAction(
        [this, &obj](const dzemikk::UIEvent&) {
            if (!_patterns) {
                return;
            }
            _patterns->usePattern(obj.patternIndex);
            refreshCounts();
        },
        actionId);

    dzemikk::UIActionRegistry::get().registerAction(
        [this, &obj](const dzemikk::UIEvent&) {
            if (!_patterns) {
                return;
            }

            auto* tooltipsGO = getOwner()->getScene()->findGameObjectByName("Tooltips_Panel");
            auto* patternTooltip = tooltipsGO->findDescendantByName("Pattern");
            auto* bonusTooltip = tooltipsGO->findDescendantByName("BonusHex");

            auto* iconGO = patternTooltip->findChildByName("Icon");
            auto* iconRenderer = iconGO->getComponent<dzemikk::ImageRenderer>();

            auto* name =
                patternTooltip->findChildByName("Name")->getComponent<dzemikk::UITextRenderer>();

            auto* leftHexGO = patternTooltip->findChildByName("Left")->findChildByName("Hex_UI");
            auto* leftHexIconGO = leftHexGO->findChildByName("Empty");

            auto* leftTextGo = patternTooltip->findChildByName("Left")->findChildByName("T1");
            auto* leftTextRenderer = leftTextGo->getComponent<dzemikk::UITextRenderer>();

            auto* rightHexGO = patternTooltip->findChildByName("Right")->findChildByName("Hex_UI");
            auto* rightHexIconGO = rightHexGO->findChildByName("Empty");

            auto* rightTextGo = patternTooltip->findChildByName("Right")->findChildByName("T1");
            auto* rightTextRenderer = rightTextGo->getComponent<dzemikk::UITextRenderer>();

            auto* bonusTooltipTextGO = bonusTooltip->findChildByName("Text");
            auto* bonusTooltipTextRenderer =
                bonusTooltipTextGO->getComponent<dzemikk::UITextRenderer>();

            switch (_patterns->getPattern(obj.patternIndex)->pattern.getType()) {
            case HexPattern::Type::ATK:
                iconRenderer->setTexture(_assetManager->get<dzemikk::Texture>(
                    "textures/ui grafiki/ui patterns/atak.png"));

                name->text = "ATTACK";
                name->color = glm::vec3(1.0F, 0.0F, 0.0F);

                leftHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                    {1.0F, 0.0F, 0.0F, 1.0F});
                leftHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                    _assetManager->get<dzemikk::Texture>(
                        "textures/ui grafiki/ui patterns/atak.png"));

                leftTextRenderer->text = std::format(
                    "Deals {:.1f} Damage",
                    _patterns->getPattern(obj.patternIndex)->pattern.getEffectStrength());

                rightHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                    {1.0F, 0.0F, 0.0F, 1.0F});
                rightHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                    _assetManager->get<dzemikk::Texture>(
                        "textures/ui grafiki/ui patterns/atak.png"));

                rightTextRenderer->text = std::format(
                    "{:.1f} Damage total",
                    _patterns->getPattern(obj.patternIndex)->pattern.getEffectStrength() *
                        _patterns->getPattern(obj.patternIndex)->pattern.getHexes().size());
                patternTooltip->enabled(true);
                break;
            case HexPattern::Type::DEF:
                iconRenderer->setTexture(_assetManager->get<dzemikk::Texture>(
                    "textures/ui grafiki/ui patterns/tarcza.png"));

                name->text = "DEFENSE";
                name->color = glm::vec3(0.0F, 0.0F, 1.0F);

                leftHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                    {0.0F, 0.0F, 1.0F, 1.0F});
                leftHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                    _assetManager->get<dzemikk::Texture>(
                        "textures/ui grafiki/ui patterns/tarcza.png"));

                leftTextRenderer->text = std::format(
                    "Grants {:.1f} Armor",
                    _patterns->getPattern(obj.patternIndex)->pattern.getEffectStrength());

                rightHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                    {0.0F, 0.0F, 1.0F, 1.0F});
                rightHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                    _assetManager->get<dzemikk::Texture>(
                        "textures/ui grafiki/ui patterns/tarcza.png"));

                rightTextRenderer->text = std::format(
                    "{:.1f} Armor total",
                    _patterns->getPattern(obj.patternIndex)->pattern.getEffectStrength() *
                        _patterns->getPattern(obj.patternIndex)->pattern.getHexes().size());
                patternTooltip->enabled(true);
                break;
            case HexPattern::Type::HEAL:
                iconRenderer->setTexture(_assetManager->get<dzemikk::Texture>(
                    "textures/ui grafiki/ui patterns/leczenie.png"));

                name->text = "HEAL";
                name->color = glm::vec3(0.0F, 1.0F, 0.0F);

                leftHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                    {0.0F, 1.0F, 0.0F, 1.0F});
                leftHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                    _assetManager->get<dzemikk::Texture>(
                        "textures/ui grafiki/ui patterns/leczenie.png"));

                leftTextRenderer->text = std::format(
                    "Restores {:.1f} Health",
                    _patterns->getPattern(obj.patternIndex)->pattern.getEffectStrength());

                rightHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                    {0.0F, 1.0F, 0.0F, 1.0F});
                rightHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                    _assetManager->get<dzemikk::Texture>(
                        "textures/ui grafiki/ui patterns/leczenie.png"));

                rightTextRenderer->text = std::format(
                    "{:.1f} Health total",
                    _patterns->getPattern(obj.patternIndex)->pattern.getEffectStrength() *
                        _patterns->getPattern(obj.patternIndex)->pattern.getHexes().size());
                patternTooltip->enabled(true);
                break;
            case HexPattern::Type::BONUSHEX:
                bonusTooltipTextRenderer->text =
                    std::format("Expands your territory by {} tile",
                                _patterns->getPattern(obj.patternIndex)->pattern.getHexes().size());

                bonusTooltip->enabled(true);
                break;
            default:
                break;
            }
        },
        hoverAction);

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            auto* tooltipsGO = getOwner()->getScene()->findGameObjectByName("Tooltips_Panel");
            auto* patternTooltip = tooltipsGO->findDescendantByName("Pattern");
            patternTooltip->enabled(false);

            auto* bonusTooltip = tooltipsGO->findDescendantByName("BonusHex");
            bonusTooltip->enabled(false);
        },
        unhoverAction);

    obj.button->addEventListener(dzemikk::UIEventType::Click, actionId);
    obj.button->addEventListener(dzemikk::UIEventType::Enter, hoverAction);
    obj.button->addEventListener(dzemikk::UIEventType::Exit, unhoverAction);
}

void game::CombatUIPanel::setupButton(game::CombatUIPanel::PatternPoolObject& obj,
                                      const glm::vec4& color) {
    if (!obj.button) {
        return;
    }

    if (_isClickable) {
        obj.button->setStyle({
            .normalColor = color,
            .hoverColor = color * 0.75F,
            .pressedColor = color * 0.5F,
        });
    } else {
        obj.button->setStyle({
            .normalColor = color,
            .hoverColor = color,
            .pressedColor = color,
        });
    }

    obj.button->applyVisualState();
}

void game::CombatUIPanel::setupPatternSlotContent(game::CombatUIPanel::PatternPoolObject& obj,
                                                  const HexPattern& pattern, uint32_t usageCount) {
    const std::string patternName = buildPatternName(pattern);
    obj.nameText->text = patternName;
    obj.countText->text = std::to_string(usageCount);
    createPatternPreview(obj, pattern);
    glm::vec4 color = getPatternBaseColor(pattern.getType());
    obj.borderRenderer->setColor({color.r, color.g, color.b, 1.0F});
}

std::vector<size_t> game::CombatUIPanel::getAvailablePatternIndices() const {
    std::vector<size_t> result;

    for (size_t i = 0; i < _patterns->getPatterns().size(); ++i) {
        if (getPatternCount(*_patterns->getPattern(i)) > 0) {
            result.push_back(i);
        }
    }

    return result;
}

void game::CombatUIPanel::setEngine(dzemikk::Engine* engine) {
    _engine = engine;
}

void game::CombatUIPanel::onMouseScrolled(dzemikk::MouseScrolledEvent& e) {
    if (_mode != Mode::AvailablePatterns) {
        return;
    }

    if (!isMouseOverPanel()) {
        return;
    }

    if (!_patterns) {
        return;
    }

    const auto& patterns = _patterns->getPatterns();
    const size_t total = patterns.size();

    if (total <= kMaxVisiblePatterns) {
        return;
    }

    constexpr size_t scrollStep = 2;

    if (e.GetYOffset() < 0) {
        _firstVisiblePatternIndex += scrollStep;
    } else if (e.GetYOffset() > 0) {
        if (_firstVisiblePatternIndex >= scrollStep) {
            _firstVisiblePatternIndex -= scrollStep;
        } else {
            _firstVisiblePatternIndex = 0;
        }
    }

    // ?? KLUCZ: clamp tylko do realnego ko�ca listy
    const size_t maxStartIndex = total > 0 ? total - kMaxVisiblePatterns : 0;

    _firstVisiblePatternIndex = std::clamp(_firstVisiblePatternIndex, size_t(0), maxStartIndex);

    refresh(true);
    updateScrollHandle();
}

float game::CombatUIPanel::calculateScrollHandleY() const {
    const auto& patterns = _patterns->getPatterns();

    const int total = static_cast<int>(patterns.size());
    const int visible = kMaxVisiblePatterns;

    if (total <= visible) {
        return _scrollHandleMaxY;
    }

    const int maxStart = total - visible;

    float t = static_cast<float>(_firstVisiblePatternIndex) / static_cast<float>(maxStart);

    t = std::clamp(t, 0.0F, 1.0F);

    return _scrollHandleMaxY + (t * (_scrollHandleMinY - _scrollHandleMaxY));
}

void game::CombatUIPanel::updateScrollHandle() {
    if (!_scrollHandle) {
        return;
    }

    auto* rt = _scrollHandle->rectTransform();
    auto pos = rt->getPosition();

    pos.y = calculateScrollHandleY();
    rt->setPosition(pos);
}

bool game::CombatUIPanel::isMouseOverPanel() const {
    const glm::vec2 mouse = _engine->getInput()->GetMousePosition();

    constexpr float minX = 20.0F;
    constexpr float maxX = 320.0F;

    constexpr float minY = 250.0F;
    constexpr float maxY = 900.0F;

    return (mouse.x >= minX && mouse.x <= maxX && mouse.y >= minY && mouse.y <= maxY);
}
