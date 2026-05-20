#include "inspectors/animatorInspector.h"

#include "ecs/components/animator.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::AnimatorInspector::draw(dzemikk::Animator* animator, const InspectorContext& ctx) {

    auto stateMachine = animator->getStateMachine();

    ImGui::Text("State Machine:");

    if (stateMachine) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Assigned");
    } else {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "None");
    }

    // TODO:
    // drag & drop state machine asset here

    ImGui::Spacing();

    // =========================
    // CURRENT STATE
    // =========================

    auto* current = animator->getCurrentState();

    ImGui::SeparatorText("Playback");

    if (current) {
        ImGui::Text("Current State: %s", current->getName().c_str());
    } else {
        ImGui::Text("Current State: None");
    }

    ImGui::Text("Time: %.3f", animator->getCurrentTime());

    ImGui::Spacing();

    // =========================
    // FLOAT PARAMS
    // =========================

    if (ImGui::TreeNode("Float Parameters")) {

        auto params = animator->getFloatParams();

        for (auto& [name, value] : params) {

            float v = value;

            if (ImGui::DragFloat(name.c_str(), &v, 0.1f)) {
                animator->setFloat(name, v);
            }
        }

        ImGui::TreePop();
    }

    // =========================
    // BOOL PARAMS
    // =========================

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

    // =========================
    // INT PARAMS
    // =========================

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

    // =========================
    // DEBUG PLAY
    // =========================

    static char stateBuffer[128] = "";

    ImGui::SeparatorText("Debug");

    ImGui::InputText("Play State", stateBuffer, sizeof(stateBuffer));

    if (ImGui::Button("Play")) {
        animator->play(stateBuffer);
    }
}
