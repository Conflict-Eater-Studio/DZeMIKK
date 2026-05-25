#include "animatorStateMachinePanel.h"

#include "animation/animationstatemachine.h"
#include "ecs/components/animator.h"

#include <cstring>
#include <imgui.h>

namespace editor {

void AnimatorStateMachinePanel::open(dzemikk::Animator* animator) {
    _animator = animator;
    _open = true;
}

void AnimatorStateMachinePanel::draw(dzemikk::Animator* animator) {
    if (!_open) {
        return;
    }

    ImGui::Begin("Animator State Machine", &_open);

    if (!_animator) {
        ImGui::TextUnformatted("No animator selected.");
        ImGui::End();
        return;
    }

    auto sm = _animator->getStateMachine();

    if (!sm) {
        ImGui::TextUnformatted("Animator has no state machine.");
        ImGui::End();
        return;
    }

    ImGui::Text("State Machine Editor");
    ImGui::Separator();

    drawStates(sm.get());
    drawAddState(sm.get());

    ImGui::Separator();

    drawSelectedState();

    ImGui::End();
}

void AnimatorStateMachinePanel::drawStates(dzemikk::AnimationStateMachine* sm) {

    ImGui::Text("States:");
    ImGui::Separator();

    const auto& states = sm->getStates();

    for (auto& [name, statePtr] : states) {

        ImGui::BulletText("%s", name.c_str());
        ImGui::SameLine();

        if (ImGui::SmallButton(("Select##" + name).c_str())) {
            _selectedState = statePtr.get();
        }
    }

    ImGui::Spacing();
}

void AnimatorStateMachinePanel::drawAddState(dzemikk::AnimationStateMachine* sm) {

    ImGui::Text("Add State:");
    ImGui::InputText("Name", _newStateName.data(), _newStateName.size());

    if (ImGui::Button("Add State")) {

        if (std::strlen(_newStateName.data()) > 0) {

            auto* newState = sm->addState(_newStateName.data());

            _selectedState = newState;

            _newStateName.fill('\0');
        }
    }

    ImGui::Spacing();
}

void AnimatorStateMachinePanel::drawSelectedState() {

    if (!_selectedState) {
        ImGui::TextUnformatted("No state selected.");
        return;
    }

    ImGui::Text("Selected State: %s", _selectedState->getName().c_str());

    ImGui::Separator();

    drawTransitions();
}

void AnimatorStateMachinePanel::drawTransitions() {

    cacheStateNames(_animator->getStateMachine().get());

    ImGui::Text("Transitions:");
    ImGui::Separator();

    auto& transitions = _selectedState->getTransitions();

    // ===== LISTA TRANSITION =====
    for (size_t i = 0; i < transitions.size(); i++) {

        auto& t = transitions[i];

        ImGui::BulletText("%s", t.targetState.c_str());
        ImGui::SameLine();
        ImGui::Text("(%.2f)", t.duration);

        ImGui::SameLine();
        const char* typeStr = (t.condition.type == dzemikk::ParamType::Float) ? "Float"
                              : (t.condition.type == dzemikk::ParamType::Int) ? "Int"
                                                                              : "Bool";

        const char* opStr = (t.condition.op == dzemikk::Greater)    ? ">"
                            : (t.condition.op == dzemikk::Less)     ? "<"
                            : (t.condition.op == dzemikk::Equal)    ? "=="
                            : (t.condition.op == dzemikk::NotEqual) ? "!="
                                                                    : "?";

        ImGui::Text("[%s %s %s %.2f]", t.condition.parameter.c_str(), typeStr, opStr,
                    t.condition.value);

        ImGui::SameLine();

        if (ImGui::SmallButton(("X##" + std::to_string(i)).c_str())) {

            auto copy = transitions;
            copy.erase(copy.begin() + i);
            _selectedState->setTransitions(copy);
            break;
        }
    }

    ImGui::Spacing();

    // ===== ADD TRANSITION =====

    ImGui::Text("Add Transition:");
    ImGui::Separator();

    // --- TARGET STATE ---
    ImGui::Text("Target State:");

    if (!_cachedStateNames.empty()) {

        std::vector<const char*> items;
        items.reserve(_cachedStateNames.size());

        for (auto& s : _cachedStateNames) {
            items.push_back(s.c_str());
        }

        ImGui::Combo("##targetState", &_targetStateIndex, items.data(), (int)items.size());
    } else {
        ImGui::TextUnformatted("No states available");
    }

    ImGui::InputFloat("Duration", &_transitionDuration);

    // ===== CONDITION UI (typed) =====

    const char* types[] = {"Float", "Int", "Bool"};
    const char* ops[] = {"Greater", "Less", "Equal", "NotEqual"};

    ImGui::Text("Condition:");

    ImGui::Combo("Type", &_condType, types, IM_ARRAYSIZE(types));
    ImGui::InputText("Parameter", _condParam.data(), _condParam.size());

    ImGui::Combo("Operator", &_condOp, ops, IM_ARRAYSIZE(ops));

    ImGui::InputFloat("Value", &_condValue);

    if (ImGui::Button("Add Transition")) {

        if (!_selectedState || _cachedStateNames.empty()) {
            return;
        }

        if (_targetStateIndex < 0 || _targetStateIndex >= (int)_cachedStateNames.size()) {
            return;
        }

        if (std::strlen(_condParam.data()) == 0) {
            return;
        }

        dzemikk::Transition t;
        t.targetState = _cachedStateNames[_targetStateIndex];
        t.duration = _transitionDuration;

        t.condition.parameter = _condParam.data();
        t.condition.op = static_cast<dzemikk::Operator>(_condOp);
        t.condition.value = _condValue;
        t.condition.type = static_cast<dzemikk::ParamType>(_condType);

        auto copy = _selectedState->getTransitions();
        copy.push_back(t);

        _selectedState->setTransitions(copy);

        // reset UI
        _targetStateIndex = 0;
        _condParam.fill('\0');

        _transitionDuration = 0.25f;
        _condValue = 0.0f;
        _condOp = 0;
        _condType = 0;
    }
}

void AnimatorStateMachinePanel::cacheStateNames(dzemikk::AnimationStateMachine* sm) {

    _cachedStateNames.clear();

    const auto& states = sm->getStates();
    for (auto& [name, _] : states) {
        _cachedStateNames.push_back(name);
    }

    if (_targetStateIndex >= (int)_cachedStateNames.size()) {
        _targetStateIndex = 0;
    }
}

} // namespace editor