#include "ui/combatUIPanel.h"

#include "enemySystem/enemyPatternComponent.h"
#include "player/playerPatternComponent.h"

#include <GLFW/glfw3.h>
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
#include <chrono>
#include <iostream>
#include <renderer/renderer.h>
#include <renderer/texture.h>

void game::CombatUIPanel::start() {
    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");
    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");
    _patternsContainer = _canvas->findDescendantByName("Patterns");
    _scrollListenerId = _engine->getInput()->OnMouseScrolled.addListener(
        [this](dzemikk::MouseScrolledEvent& e) { onMouseScrolled(e); });
    _scrollHandle = getOwner()->getScene()->findGameObjectByName("Scroll");
    refresh();
}

void game::CombatUIPanel::update(double deltaTime) {}

void game::CombatUIPanel::onDestroy() {
    _engine->getInput()->OnMouseScrolled.removeListener(_scrollListenerId);
}

void game::CombatUIPanel::refresh(bool enableChildren) {
#if DZEMIKK_DEV_TOOLS
    auto tRefreshStart = std::chrono::steady_clock::now();
#endif

    clear();

#if DZEMIKK_DEV_TOOLS
    auto tAfterClear = std::chrono::steady_clock::now();
    auto clearMs = std::chrono::duration<double, std::milli>(tAfterClear - tRefreshStart).count();
    spdlog::info("[CombatUIPanel::refresh] clear() took {:.2f} ms", clearMs);
#endif

    if (!_patterns) {
#if DZEMIKK_DEV_TOOLS
        auto totalMs = std::chrono::duration<double, std::milli>(
                           std::chrono::steady_clock::now() - tRefreshStart)
                           .count();
        spdlog::info("[CombatUIPanel::refresh] early return (no patterns), total {:.2f} ms",
                     totalMs);
#endif
        return;
    }

    buildUI();

#if DZEMIKK_DEV_TOOLS
    auto tAfterBuild = std::chrono::steady_clock::now();
    auto buildMs = std::chrono::duration<double, std::milli>(tAfterBuild - tAfterClear).count();
    spdlog::info("[CombatUIPanel::refresh] buildUI() took {:.2f} ms", buildMs);
#endif

    refreshCounts();

#if DZEMIKK_DEV_TOOLS
    auto tAfterCounts = std::chrono::steady_clock::now();
    auto countsMs = std::chrono::duration<double, std::milli>(tAfterCounts - tAfterBuild).count();
    spdlog::info("[CombatUIPanel::refresh] refreshCounts() took {:.2f} ms", countsMs);
#endif

    for (auto* child : getOwner()->getChildren()) {
        if (enableChildren) {
            child->enabled(true);
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto tEnd = std::chrono::steady_clock::now();
    auto enableMs = std::chrono::duration<double, std::milli>(tEnd - tAfterCounts).count();
    auto totalMs = std::chrono::duration<double, std::milli>(tEnd - tRefreshStart).count();
    spdlog::info("[CombatUIPanel::refresh] enableChildren loop took {:.2f} ms", enableMs);
    spdlog::info("[CombatUIPanel::refresh] TOTAL: {:.2f} ms (clear={:.2f}, build={:.2f}, "
                 "counts={:.2f}, enable={:.2f})",
                 totalMs, clearMs, buildMs, countsMs, enableMs);
#endif
}

void game::CombatUIPanel::clear() {
    if (_uiEntries.empty()) {
        return;
    }

#if DZEMIKK_DEV_TOOLS
    auto tStart = std::chrono::steady_clock::now();
    spdlog::info("[CombatUIPanel::clear] entries to clear: {}", _uiEntries.size());
#endif

    _uiEntries.at(0).root->getParent()->detachChildren();

#if DZEMIKK_DEV_TOOLS
    auto tAfterDetach = std::chrono::steady_clock::now();
    auto detachMs = std::chrono::duration<double, std::milli>(tAfterDetach - tStart).count();
    spdlog::info("[CombatUIPanel::clear] detachChildren() took {:.2f} ms", detachMs);
#endif

    for (auto& entry : _uiEntries) {
        if (entry.root) {
#if DZEMIKK_DEV_TOOLS
            auto tBeforeDestroy = std::chrono::steady_clock::now();
#endif
            entry.root->getScene()->destroyGameObject(entry.root);
#if DZEMIKK_DEV_TOOLS
            auto destroyMs = std::chrono::duration<double, std::milli>(
                                 std::chrono::steady_clock::now() - tBeforeDestroy)
                                 .count();
            if (destroyMs > 0.05) {
                spdlog::warn("[CombatUIPanel::clear] destroyGameObject(root idx={}) took {:.2f} ms",
                             &entry - _uiEntries.data(), destroyMs);
            }
#endif
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto tAfterDestroyLoop = std::chrono::steady_clock::now();
    auto destroyLoopMs =
        std::chrono::duration<double, std::milli>(tAfterDestroyLoop - tAfterDetach).count();
    spdlog::info("[CombatUIPanel::clear] destroy loop total took {:.2f} ms", destroyLoopMs);
#endif

    _uiEntries.clear();

#if DZEMIKK_DEV_TOOLS
    auto tEnd = std::chrono::steady_clock::now();
    auto clearVecMs = std::chrono::duration<double, std::milli>(tEnd - tAfterDestroyLoop).count();
    auto totalMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    spdlog::info("[CombatUIPanel::clear] _uiEntries.clear() took {:.2f} ms", clearVecMs);
    spdlog::info("[CombatUIPanel::clear] TOTAL: {:.2f} ms (detach={:.2f}, destroyLoop={:.2f}, "
                 "vecClear={:.2f})",
                 totalMs, detachMs, destroyLoopMs, clearVecMs);
#endif
}

void game::CombatUIPanel::buildUI() {
    if (!_patterns || !_patternsContainer) {
        return;
    }

#if DZEMIKK_DEV_TOOLS
    auto tBuildStart = std::chrono::steady_clock::now();
#endif

    if (_hideEmptyPatterns) {
        const auto indices = getAvailablePatternIndices();

#if DZEMIKK_DEV_TOOLS
        auto tAfterFilter = std::chrono::steady_clock::now();
        auto filterMs = std::chrono::duration<double, std::milli>(tAfterFilter - tBuildStart).count();
        spdlog::info("[CombatUIPanel::buildUI] getAvailablePatternIndices() took {:.2f} ms ({} indices)",
                     filterMs, indices.size());
#endif

        for (size_t patternIndex : indices) {
#if DZEMIKK_DEV_TOOLS
            auto tSlotStart = std::chrono::steady_clock::now();
#endif
            createPatternSlot(_patterns->getPatterns()[patternIndex], patternIndex);
#if DZEMIKK_DEV_TOOLS
            auto slotMs = std::chrono::duration<double, std::milli>(
                              std::chrono::steady_clock::now() - tSlotStart)
                              .count();
            if (slotMs > 0.5) {
                spdlog::warn("[CombatUIPanel::buildUI] createPatternSlot(index={}) took {:.2f} ms",
                             patternIndex, slotMs);
            }
#endif
        }
    } else {
        const auto& patterns = _patterns->getPatterns();
        const size_t start = _firstVisiblePatternIndex;
        const size_t end = std::min(start + MAX_VISIBLE_PATTERNS, patterns.size());

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[CombatUIPanel::buildUI] creating slots [{}, {}) of {} total patterns",
                     start, end, patterns.size());
#endif

        for (size_t i = start; i < end; ++i) {
#if DZEMIKK_DEV_TOOLS
            auto tSlotStart = std::chrono::steady_clock::now();
#endif
            createPatternSlot(patterns[i], i);
#if DZEMIKK_DEV_TOOLS
            auto slotMs = std::chrono::duration<double, std::milli>(
                              std::chrono::steady_clock::now() - tSlotStart)
                              .count();
            if (slotMs > 0.5) {
                spdlog::warn("[CombatUIPanel::buildUI] createPatternSlot(index={}) took {:.2f} ms",
                             i, slotMs);
            }
#endif
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto tAfterSlots = std::chrono::steady_clock::now();
    auto slotsMs = std::chrono::duration<double, std::milli>(tAfterSlots - tBuildStart).count();
    spdlog::info("[CombatUIPanel::buildUI] all createPatternSlot() calls took {:.2f} ms", slotsMs);
#endif

    auto* grid = _patternsContainer->getComponent<dzemikk::GridLayout>();

    if (grid) {
#if DZEMIKK_DEV_TOOLS
        auto tGridStart = std::chrono::steady_clock::now();
#endif
        grid->rebuild();
#if DZEMIKK_DEV_TOOLS
        auto gridMs = std::chrono::duration<double, std::milli>(
                          std::chrono::steady_clock::now() - tGridStart)
                          .count();
        spdlog::info("[CombatUIPanel::buildUI] grid->rebuild() took {:.2f} ms", gridMs);
#endif
    }

#if DZEMIKK_DEV_TOOLS
    auto tEnd = std::chrono::steady_clock::now();
    auto totalMs = std::chrono::duration<double, std::milli>(tEnd - tBuildStart).count();
    spdlog::info("[CombatUIPanel::buildUI] TOTAL: {:.2f} ms", totalMs);
#endif
}

void game::CombatUIPanel::createPatternSlot(const PatternComponent::PatternEntry& entry,
                                            size_t index) {
#if DZEMIKK_DEV_TOOLS
    auto tStart = std::chrono::steady_clock::now();
#endif

    auto* patternGO = createPatternSlotObject();

#if DZEMIKK_DEV_TOOLS
    auto tAfterInstantiate = std::chrono::steady_clock::now();
    auto instantiateMs =
        std::chrono::duration<double, std::milli>(tAfterInstantiate - tStart).count();
#endif

    PatternUIEntry uiEntry;
    uiEntry.patternIndex = index;
    uiEntry.root = patternGO;
    uiEntry.button = patternGO->getComponent<dzemikk::UIButton>();

    const uint32_t usageCount = getPatternCount(entry);
    const auto color = applyUsageTint(getPatternBaseColor(entry.pattern.getType()), usageCount);
    const std::string actionId = "pattern_" + std::to_string(index);
    setupButton(uiEntry.button, index, actionId, color);

#if DZEMIKK_DEV_TOOLS
    auto tAfterSetup = std::chrono::steady_clock::now();
    auto setupButtonMs =
        std::chrono::duration<double, std::milli>(tAfterSetup - tAfterInstantiate).count();
#endif

    setupPatternSlotContent(patternGO, uiEntry, entry.pattern, usageCount);

#if DZEMIKK_DEV_TOOLS
    auto tAfterContent = std::chrono::steady_clock::now();
    auto contentMs = std::chrono::duration<double, std::milli>(tAfterContent - tAfterSetup).count();
#endif

    _uiEntries.push_back(uiEntry);

#if DZEMIKK_DEV_TOOLS
    auto totalMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tStart).count();
    spdlog::info(
        "[CombatUIPanel::createPatternSlot] index={} TOTAL: {:.2f} ms (instantiate={:.2f}, "
        "setupButton={:.2f}, setupContent={:.2f})",
        index, totalMs, instantiateMs, setupButtonMs, contentMs);
#endif
}

void game::CombatUIPanel::createPatternPreview(dzemikk::GameObject* parent,
                                               const HexPattern& pattern) {
#if DZEMIKK_DEV_TOOLS
    auto tStart = std::chrono::steady_clock::now();
#endif

    constexpr float HEX_SIZE = 15.0F;

    std::vector<glm::vec2> positions;

    for (const auto& hex : pattern.getHexes()) {
        float x =
            HEX_SIZE * std::numbers::sqrt3_v<float> * ((float)hex.q() + (float)hex.r() * 0.5F);
        float y = HEX_SIZE * 1.5F * (float)hex.r();
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

    std::string texturePath = "";
    if (pattern.getType() == HexPattern::Type::ATK) {
        texturePath = "textures/ui grafiki/ui patterns/atak.png";
    } else if (pattern.getType() == HexPattern::Type::DEF) {
        texturePath = "textures/ui grafiki/ui patterns/tarcza.png";
    } else if (pattern.getType() == HexPattern::Type::HEAL) {
        texturePath = "textures/ui grafiki/ui patterns/leczenie.png";
    } else if (pattern.getType() == HexPattern::Type::BONUSHEX) {
        texturePath = "textures/ui grafiki/ui patterns/dodatkowy_hex.png";
    }

#if DZEMIKK_DEV_TOOLS
    auto tBeforeLoop = std::chrono::steady_clock::now();
    auto prepMs = std::chrono::duration<double, std::milli>(tBeforeLoop - tStart).count();
#endif

    for (size_t i = 0; i < positions.size(); ++i) {

#if DZEMIKK_DEV_TOOLS
        auto tHexStart = std::chrono::steady_clock::now();
#endif

        auto* hexGO = dzemikk::PrefabSerializer::instantiate(*getOwner()->getScene(),
                                                             *_hexUIPrefab.get(), _assetManager);
        hexGO->setParent(parent);
        hexGO->enabled(false);

        auto* transform = hexGO->rectTransform();
        transform->setPosition(glm::vec3(positions[i] - center, 0.0F));

        auto* renderer = hexGO->getComponent<dzemikk::ImageRenderer>();
        glm::vec4 color = getPatternBaseColor(pattern.getType());
        renderer->setColor({color.r, color.g, color.b, 1.0F});

        auto* hexChild = hexGO->findChildByName("Empty");
        auto* hexChildRenderer = hexChild->getComponent<dzemikk::ImageRenderer>();

        if (texturePath != "") {
            hexChildRenderer->setTexture(_assetManager->get<dzemikk::Texture>(texturePath));
        } else {
            hexChildRenderer->enabled(false);
        }

#if DZEMIKK_DEV_TOOLS
        auto hexMs = std::chrono::duration<double, std::milli>(
                         std::chrono::steady_clock::now() - tHexStart)
                         .count();
        if (hexMs > 0.5) {
            spdlog::warn("[CombatUIPanel::createPatternPreview] hex {} instantiate+setup took "
                         "{:.2f} ms",
                         i, hexMs);
        }
#endif
    }

#if DZEMIKK_DEV_TOOLS
    auto totalMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tStart).count();
    auto loopMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tBeforeLoop)
            .count();
    spdlog::info("[CombatUIPanel::createPatternPreview] TOTAL: {:.2f} ms (prep={:.2f}, "
                 "hexLoop={:.2f}, hexes={})",
                 totalMs, prepMs, loopMs, positions.size());
#endif
}

void game::CombatUIPanel::refreshCounts() {
#if DZEMIKK_DEV_TOOLS
    auto tStart = std::chrono::steady_clock::now();
#endif

    for (auto& entry : _uiEntries) {
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

#if DZEMIKK_DEV_TOOLS
    auto totalMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tStart).count();
    spdlog::info("[CombatUIPanel::refreshCounts] TOTAL: {:.2f} ms ({} entries)", totalMs,
                 _uiEntries.size());
#endif
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

void game::CombatUIPanel::refreshVisuals() {
    if (!_patterns) {
        return;
    }

    const bool isEnemy = (dynamic_cast<EnemyPatternComponent*>(_patterns) != nullptr);

    for (auto& ui : _uiEntries) {
        if (ui.patternIndex >= _patterns->getPatterns().size()) {
            continue;
        }

        const auto& entry = _patterns->getPatterns()[ui.patternIndex];

        uint32_t count = 0;

        if (isEnemy) {
            auto* enemy = dynamic_cast<EnemyPatternComponent*>(_patterns);

            const auto& usage = enemy->getPatternUsage();

            auto it = usage.find(&entry.pattern);
            if (it != usage.end()) {
                count = it->second;
            }
        } else {
            count = entry.count;
        }

        glm::vec4 baseColor = getPatternBaseColor(entry.pattern.getType());
        glm::vec4 color = applyUsageTint(baseColor, count);

        if (ui.button) {
            if (_isClickable) {
                ui.button->setStyle({color, color * 0.75F, color * 0.5F});
            } else {
                ui.button->setStyle({color, color, color});
            }

            ui.button->applyVisualState();
        }

        if (ui.countText) {
            ui.countText->text = "0/" + std::to_string(count);
        }
    }
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

void game::CombatUIPanel::setupButton(dzemikk::UIButton* button, size_t index,
                                      const std::string& actionId, const glm::vec4& color) {
    if (!button) {
        return;
    }

    if (_isClickable) {

        dzemikk::UIActionRegistry::get().registerAction(
            [this, index, actionId](const dzemikk::UIEvent&) {
                if (!_patterns) {
                    return;
                }

                _patterns->usePattern(index);

                refreshCounts();
            },
            actionId);

        const std::string hoverAction = actionId + "_hover";
        const std::string unhoverAction = actionId + "_unhover";

        dzemikk::UIActionRegistry::get().registerAction(
            [this, index](const dzemikk::UIEvent&) {
                auto tooltipsGO = getOwner()->getScene()->findGameObjectByName("Tooltips_Panel");
                auto patternTooltip = tooltipsGO->findDescendantByName("Pattern");
                auto bonusTooltip = tooltipsGO->findDescendantByName("BonusHex");

                auto iconGO = patternTooltip->findChildByName("Icon");
                auto iconRenderer = iconGO->getComponent<dzemikk::ImageRenderer>();

                auto name = patternTooltip->findChildByName("Name")
                                ->getComponent<dzemikk::UITextRenderer>();

                auto leftHexGO = patternTooltip->findChildByName("Left")->findChildByName("Hex_UI");
                auto leftHexIconGO = leftHexGO->findChildByName("Empty");

                auto leftTextGo = patternTooltip->findChildByName("Left")->findChildByName("T1");
                auto leftTextRenderer = leftTextGo->getComponent<dzemikk::UITextRenderer>();

                auto rightHexGO =
                    patternTooltip->findChildByName("Right")->findChildByName("Hex_UI");
                auto rightHexIconGO = rightHexGO->findChildByName("Empty");

                auto rightTextGo = patternTooltip->findChildByName("Right")->findChildByName("T1");
                auto rightTextRenderer = rightTextGo->getComponent<dzemikk::UITextRenderer>();

                auto bonusTooltipTextGO = bonusTooltip->findChildByName("Text");
                auto bonusTooltipTextRenderer =
                    bonusTooltipTextGO->getComponent<dzemikk::UITextRenderer>();

                switch (_patterns->getPattern(index)->pattern.getType()) {
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

                    leftTextRenderer->text =
                        std::format("Deals {:.1f} Damage",
                                    _patterns->getPattern(index)->pattern.getEffectStrength());

                    rightHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                        {1.0F, 0.0F, 0.0F, 1.0F});
                    rightHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                        _assetManager->get<dzemikk::Texture>(
                            "textures/ui grafiki/ui patterns/atak.png"));

                    rightTextRenderer->text =
                        std::format("{:.1f} Damage total",
                                    _patterns->getPattern(index)->pattern.getEffectStrength() *
                                        _patterns->getPattern(index)->pattern.getHexes().size());
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

                    leftTextRenderer->text =
                        std::format("Grants {:.1f} Armor",
                                    _patterns->getPattern(index)->pattern.getEffectStrength());

                    rightHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                        {0.0F, 0.0F, 1.0F, 1.0F});
                    rightHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                        _assetManager->get<dzemikk::Texture>(
                            "textures/ui grafiki/ui patterns/tarcza.png"));

                    rightTextRenderer->text =
                        std::format("{:.1f} Armor total",
                                    _patterns->getPattern(index)->pattern.getEffectStrength() *
                                        _patterns->getPattern(index)->pattern.getHexes().size());
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

                    leftTextRenderer->text =
                        std::format("Restores {:.1f} Health",
                                    _patterns->getPattern(index)->pattern.getEffectStrength());

                    rightHexGO->getComponent<dzemikk::ImageRenderer>()->setColor(
                        {0.0F, 1.0F, 0.0F, 1.0F});
                    rightHexIconGO->getComponent<dzemikk::ImageRenderer>()->setTexture(
                        _assetManager->get<dzemikk::Texture>(
                            "textures/ui grafiki/ui patterns/leczenie.png"));

                    rightTextRenderer->text =
                        std::format("{:.1f} Health total",
                                    _patterns->getPattern(index)->pattern.getEffectStrength() *
                                        _patterns->getPattern(index)->pattern.getHexes().size());
                    patternTooltip->enabled(true);
                    break;
                case HexPattern::Type::BONUSHEX:
                    bonusTooltipTextRenderer->text =
                        std::format("Expands your territory by {} tile",
                                    _patterns->getPattern(index)->pattern.getHexes().size());

                    bonusTooltip->enabled(true);
                    break;
                default:
                    break;
                }
            },
            hoverAction);

        dzemikk::UIActionRegistry::get().registerAction(
            [this, index](const dzemikk::UIEvent&) {
                auto tooltipsGO = getOwner()->getScene()->findGameObjectByName("Tooltips_Panel");
                auto patternTooltip = tooltipsGO->findDescendantByName("Pattern");
                patternTooltip->enabled(false);

                auto bonusTooltip = tooltipsGO->findDescendantByName("BonusHex");
                bonusTooltip->enabled(false);
            },
            unhoverAction);

        button->addEventListener(dzemikk::UIEventType::Click, actionId);
        button->addEventListener(dzemikk::UIEventType::Enter, hoverAction);
        button->addEventListener(dzemikk::UIEventType::Exit, unhoverAction);

        button->setStyle({color, color * 0.75F, color * 0.5F});
    } else {
        button->setStyle({color, color, color});
    }

    button->applyVisualState();
}

void game::CombatUIPanel::setupPatternSlotContent(dzemikk::GameObject* patternGO,
                                                  PatternUIEntry& uiEntry,
                                                  const HexPattern& pattern, uint32_t usageCount) {
    const std::string patternName = buildPatternName(pattern);

    for (auto* child : patternGO->getChildren()) {

        if (child->getName() == "Button_Text") {
            setupPatternName(child, patternName);
        } else if (child->getName() == "Count") {
            setupCountText(child, usageCount, uiEntry);
        } else if (child->getName() == "HexHolder") {
            createPatternPreview(child, pattern);
        } else if (child->getName() == "Empty") {
            auto* renderer = child->getComponent<dzemikk::ImageRenderer>();
            glm::vec4 color = getPatternBaseColor(pattern.getType());
            renderer->setColor({color.r, color.g, color.b, 1.0F});
        }
    }
}

void game::CombatUIPanel::setupPatternName(dzemikk::GameObject* object,
                                           const std::string& patternName) {
    auto* text = object->getComponent<dzemikk::UITextRenderer>();

    if (text) {
        text->text = patternName;
    }
}

void game::CombatUIPanel::setupCountText(dzemikk::GameObject* countRoot, uint32_t usageCount,
                                         PatternUIEntry& uiEntry) {
    for (auto* child : countRoot->getChildren()) {

        if (child->getName() != "Text_Count") {
            continue;
        }

        auto* text = child->getComponent<dzemikk::UITextRenderer>();

        if (!text) {
            continue;
        }

        text->text = std::to_string(usageCount);
        uiEntry.countText = text;
    }
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

void game::CombatUIPanel::addPatternSlot(const PatternComponent::PatternEntry& entry) {
    const size_t index = _uiEntries.size();
    createPatternSlot(entry, index);

    if (auto* grid = _patternsContainer->getComponent<dzemikk::GridLayout>()) {
        grid->rebuild();
    }
}

void game::CombatUIPanel::setEngine(dzemikk::Engine* engine) {
    _engine = engine;
}

void game::CombatUIPanel::onMouseScrolled(dzemikk::MouseScrolledEvent& e) {
    if (_mode != Mode::AvailablePatterns)
        return;

    if (!isMouseOverPanel())
        return;

    if (!_patterns)
        return;

    const auto& patterns = _patterns->getPatterns();
    const size_t total = patterns.size();

    if (total <= MAX_VISIBLE_PATTERNS)
        return;

    constexpr size_t SCROLL_STEP = 2;

    if (e.GetYOffset() < 0) {
        _firstVisiblePatternIndex += SCROLL_STEP;
    } else if (e.GetYOffset() > 0) {
        if (_firstVisiblePatternIndex >= SCROLL_STEP)
            _firstVisiblePatternIndex -= SCROLL_STEP;
        else
            _firstVisiblePatternIndex = 0;
    }

    // ?? KLUCZ: clamp tylko do realnego ko�ca listy
    const size_t maxStartIndex = total > 0 ? total - 1 : 0;

    _firstVisiblePatternIndex = std::clamp(_firstVisiblePatternIndex, size_t(0), maxStartIndex);

    refresh(true);
    updateScrollHandle();
}

float game::CombatUIPanel::calculateScrollHandleY() const {
    const auto& patterns = _patterns->getPatterns();

    const int total = static_cast<int>(patterns.size());
    const int visible = MAX_VISIBLE_PATTERNS;

    if (total <= visible)
        return _scrollHandleMaxY;

    const int maxStart = total - visible;

    float t = static_cast<float>(_firstVisiblePatternIndex) / static_cast<float>(maxStart);

    t = std::clamp(t, 0.0f, 1.0f);

    return _scrollHandleMaxY + t * (_scrollHandleMinY - _scrollHandleMaxY);
}

void game::CombatUIPanel::updateScrollHandle() {
    if (!_scrollHandle)
        return;

    auto* rt = _scrollHandle->rectTransform();
    auto pos = rt->getPosition();

    pos.y = calculateScrollHandleY();
    rt->setPosition(pos);
}

bool game::CombatUIPanel::isMouseOverPanel() const {
    const glm::vec2 mouse = _engine->getInput()->GetMousePosition();

    constexpr float minX = 20.0f;
    constexpr float maxX = 320.0f;

    constexpr float minY = 250.0f;
    constexpr float maxY = 900.0f;

    return (mouse.x >= minX && mouse.x <= maxX && mouse.y >= minY && mouse.y <= maxY);
}
