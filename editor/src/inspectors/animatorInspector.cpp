#include "inspectors/animatorInspector.h"
#include "editor.h"

#include "ecs/components/animator.h"
#include "animation/animationstatemachine.h"
#include "ui/propertyDrawer.h"

#include <cstring>
#include <imgui.h>

void editor::AnimatorInspector::draw(dzemikk::Animator* animator, const InspectorContext& ctx) {
    if (!animator) {
        return;
    }

    drawAnimationStateMachineSection(animator, ctx);
    drawCurrentState(animator);
    drawAddParameter(animator);
    drawParameters(animator);
    drawDebug(animator);
}

void editor::AnimatorInspector::drawAnimationStateMachineSection(dzemikk::Animator* animator,
                                                                 const InspectorContext& ctx) {
    auto stateMachine = animator->getStateMachine();

    ImGui::SeparatorText("State Machine");

    if (stateMachine) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
        ImGui::TextUnformatted("Assigned");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
        ImGui::TextUnformatted("None");
        ImGui::PopStyleColor();
    }

    if (!animator->getStateMachine()) {
        if (ImGui::Button("Create State Machine")) {
            auto newSM = std::make_shared<dzemikk::AnimationStateMachine>();
            newSM->addState("Idle");
            animator->setStateMachine(newSM);
            animator->play("Idle");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Edit State Machine")) {
        ctx.editor->openAnimatorStateMachine(animator);
    }

    ImGui::Spacing();
}

void editor::AnimatorInspector::drawCurrentState(dzemikk::Animator* animator) {
    auto* current = animator->getCurrentState();

    ImGui::SeparatorText("Playback");

    if (current) {
        ImGui::TextUnformatted("Current State: ");
        ImGui::SameLine();
        ImGui::TextUnformatted(current->getName().c_str());
    } else {
        ImGui::TextUnformatted("Current State: None");
    }

    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "Time: %.3f", animator->getCurrentTime());
    ImGui::TextUnformatted(buffer);

    ImGui::Spacing();
}

void editor::AnimatorInspector::drawParameters(dzemikk::Animator* animator) {
    if (ImGui::TreeNode("Float Parameters")) {

        auto params = animator->getFloatParams();

        for (auto& [name, value] : params) {

            float v = value;

            if (ImGui::DragFloat(name.c_str(), &v, 0.1F)) {
                animator->setFloat(name, v);
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Bool Parameters")) {

        auto params = animator->getBoolParams();

        for (auto& [name, value] : params) {

            bool v = value;

            if (ImGui::Checkbox(name.c_str(), &v)) {
                animator->setBool(name, v);
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Int Parameters")) {

        auto params = animator->getIntParams();

        for (auto& [name, value] : params) {

            int v = value;

            if (ImGui::DragInt(name.c_str(), &v)) {
                animator->setInt(name, v);
            }
        }

        ImGui::TreePop();
    }
}

void editor::AnimatorInspector::drawAddParameter(dzemikk::Animator* animator) {
    ImGui::SeparatorText("Add Parameter");

    static std::array<char, 64> newName = {};
    static int newType = 0; // 0 float, 1 bool, 2 int
    static float newFloat = 0.0F;
    static bool newBool = false;
    static int newInt = 0;

    ImGui::InputText("Name", newName.data(), sizeof(newName));
    ImGui::Combo("Type", &newType, "Float\0Bool\0Int\0");

    if (newType == 0) {
        ImGui::InputFloat("Value", &newFloat);
    } else if (newType == 1) {
        ImGui::Checkbox("Value", &newBool);
    } else if (newType == 2) {
        ImGui::InputInt("Value", &newInt);
    }

    if (ImGui::Button("Add Parameter")) {
        if (std::strlen(newName.data()) > 0) {

            switch (newType) {
            case 0:
                animator->setFloat(newName.data(), newFloat);
                break;
            case 1:
                animator->setBool(newName.data(), newBool);
                break;
            case 2:
                animator->setInt(newName.data(), newInt);
                break;
            default:
                break;
            }

            newName[0] = '\0';
            newFloat = 0.0F;
            newBool = false;
            newInt = 0;
        }
    }

    ImGui::Spacing();
}

void editor::AnimatorInspector::drawDebug(dzemikk::Animator* animator) {
    static std::array<char, 128> stateBuffer = {};

    ImGui::SeparatorText("Debug");

    ImGui::InputText("Play State", stateBuffer.data(), sizeof(stateBuffer));

    if (ImGui::Button("Play")) {
        animator->play(stateBuffer.data());
    }
}
