#include "inspectors/uiDropdownInspector.h"

#include "ecs/components/ui/uiDropdown.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::UIDropdownInspector::draw(dzemikk::UIDropdown* dropdown, const InspectorContext& ctx) {
    if (!dropdown) {
        return;
    }

    if (ImGui::CollapsingHeader("UI Dropdown", ImGuiTreeNodeFlags_DefaultOpen)) {

        auto style = dropdown->getStyle();
        bool styleChanged = false;

        styleChanged |= PropertyDrawer::drawColor("Normal Color", style.normalColor);
        styleChanged |= PropertyDrawer::drawColor("Hover Color", style.hoverColor);
        styleChanged |= PropertyDrawer::drawColor("Pressed Color", style.pressedColor);
        styleChanged |= PropertyDrawer::drawColor("Arrow Color", style.arrowColor);
        styleChanged |= PropertyDrawer::drawColor("Text Color", style.textColor);
        styleChanged |= PropertyDrawer::drawColor("Normal Option Color", style.normalOptColor);
        styleChanged |= PropertyDrawer::drawColor("Hover Option Color", style.hoverOptColor);
        styleChanged |= PropertyDrawer::drawColor("Pressed Option Color", style.pressedOptColor);
        styleChanged |=
            PropertyDrawer::drawColor("Highlight Option Color", style.highlightOptColor);

        if (styleChanged) {
            dropdown->setStyle(style);
            dropdown->applyOptionButtonColors();
        }

        auto render = dropdown->getOptionRender();
        bool renderChanged = false;

        renderChanged |= PropertyDrawer::drawFloat("Option Height", render.height);
        renderChanged |= PropertyDrawer::drawFont("Font", render.font, ctx);

        {
            int vAlign = static_cast<int>(render.textVAlign);
            const char* items[] = {"Bottom", "Middle", "Top"};

            if (ImGui::Combo("Vertical Align", &vAlign, items, IM_ARRAYSIZE(items))) {
                render.textVAlign = static_cast<dzemikk::UITextRenderer::VerticalAlign>(vAlign);
                renderChanged = true;
            }
        }

        {
            int hAlign = static_cast<int>(render.textHAlign);
            const char* items[] = {"Left", "Center", "Right"};

            if (ImGui::Combo("Horizontal Align", &hAlign, items, IM_ARRAYSIZE(items))) {
                render.textHAlign = static_cast<dzemikk::UITextRenderer::HorizontalAlign>(hAlign);
                renderChanged = true;
            }
        }

        if (renderChanged) {
            dropdown->setOptionRender(render);
            dropdown->applyOptionButtonColors();
            dropdown->applyVisualState();
            dropdown->refreshOptionLayout();
        }

        auto options = dropdown->getOptions();

        if (ImGui::TreeNode("Options")) {

            bool optionsChanged = false;
            int removeIndex = -1;

            static std::unordered_map<std::string, std::array<char, 256>> textBuffers;
            static std::unordered_map<std::string, std::array<char, 256>> valueBuffers;

            for (std::size_t i = 0; i < options.size(); ++i) {

                ImGui::PushID((int)i);

                auto& option = options[i];

                if (ImGui::TreeNode(("Option " + std::to_string(i)).c_str())) {

                    const std::string textKey = "dropdown_text_" + std::to_string(i);
                    const std::string valueKey = "dropdown_value_" + std::to_string(i);

                    auto& textBuffer = textBuffers[textKey];
                    auto& valueBuffer = valueBuffers[valueKey];

                    if (textBuffer[0] == '\0') {
                        std::snprintf(textBuffer.data(), textBuffer.size(), "%s",
                                      option.text.c_str());
                    }

                    if (valueBuffer[0] == '\0') {
                        std::snprintf(valueBuffer.data(), valueBuffer.size(), "%s",
                                      option.value.c_str());
                    }

                    if (ImGui::InputText("Text", textBuffer.data(), textBuffer.size())) {
                        option.text = textBuffer.data();
                        optionsChanged = true;
                    }

                    if (ImGui::InputText("Value", valueBuffer.data(), valueBuffer.size())) {
                        option.value = valueBuffer.data();
                        optionsChanged = true;
                    }

                    if (ImGui::Button("Remove")) {
                        removeIndex = (int)i;
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }

            if (removeIndex >= 0) {
                options.erase(options.begin() + removeIndex);
                dropdown->removeOption(removeIndex);
                optionsChanged = true;
            }

            if (ImGui::Button("Add Option")) {
                options.push_back({"Option", "value"});
                dropdown->addOption({"Option", "value"});
                optionsChanged = true;
            }

            if (optionsChanged) {

                dropdown->setOptions(options);
                dropdown->updateOptionDataOnly();
                dropdown->applyOptionButtonColors();
                dropdown->applyVisualState();
            }

            ImGui::TreePop();
        }

        dropdown->refreshOptionLayout();

        
        ImGui::Spacing();
        ImGui::Separator();

        PropertyDrawer::drawUIEvents("Events", dropdown, ctx);
    }
}