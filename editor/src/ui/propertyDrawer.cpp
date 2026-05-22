#include "ui/propertyDrawer.h"

#include "renderer/material.h"


namespace {

const char* toString(dzemikk::UIEventType type) {
    switch (type) {
    case dzemikk::UIEventType::Click:
        return "Click";
    case dzemikk::UIEventType::Enter:
        return "Enter";
    case dzemikk::UIEventType::Exit:
        return "Exit";
    default:
        return "Unknown";
    }
}

dzemikk::UIEventType fromIndex(int i) {
    using namespace dzemikk;
    switch (i) {
    case 0:
        return UIEventType::Click;
    case 1:
        return UIEventType::Enter;
    case 2:
        return UIEventType::Exit;
    default:
        return UIEventType::Click;
    }
}

} // namespace

bool editor::PropertyDrawer::drawFloat(const std::string& label, float& value, float speed) {

    return ImGui::DragFloat(label.c_str(), &value, speed);
}

bool editor::PropertyDrawer::drawFloat(const std::string& label, float& value, float min, float max,
                                       float speed) {
    return ImGui::SliderFloat(label.c_str(), &value, min, max);
}

bool editor::PropertyDrawer::drawInt(const std::string& label, int& value, int speed) {

    return ImGui::DragInt(label.c_str(), &value, speed);
}

bool editor::PropertyDrawer::drawInt(const std::string& label, int& value, int min, int max) {

    return ImGui::SliderInt(label.c_str(), &value, min, max);
}

bool editor::PropertyDrawer::drawVec2(const std::string& label, glm::vec2& value,
                                      float resetValue) {

    return ImGui::DragFloat2(label.c_str(), &value.x, 0.1F);
}

bool editor::PropertyDrawer::drawVec3(const std::string& label, glm::vec3& value,
                                      float resetValue) {

    return ImGui::DragFloat3(label.c_str(), &value.x, 0.1F);
}

bool editor::PropertyDrawer::drawBool(const std::string& label, bool& value) {
    return ImGui::Checkbox(label.c_str(), &value);
}

bool editor::PropertyDrawer::drawShader(const std::string& label,
                                        dzemikk::AssetHandle<dzemikk::Shader>& handle,
                                        const InspectorContext& ctx) {
    std::string path = handle.get() ? handle.getAssetPath() : "null";

    static std::unordered_map<std::string, std::array<char, 256>> buffers;
    auto& buffer = buffers[label];

    std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());

    bool changed = false;

    ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                     ImGuiInputTextFlags_EnterReturnsTrue);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_SHADER")) {
            const char* droppedPath = (const char*)payload->Data;

            auto newHandle = ctx.assetManager->get<dzemikk::Shader>(droppedPath);

            if (newHandle.get()) {
                handle = newHandle;
                changed = true;

                std::snprintf(buffer.data(), buffer.size(), "%s", droppedPath);
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        auto newHandle = ctx.assetManager->get<dzemikk::Shader>(buffer.data());

        if (newHandle.get()) {
            handle = newHandle;
            changed = true;
        }
    }

    return changed;
}

bool editor::PropertyDrawer::drawColor(const std::string& label, glm::vec4& color) {
    return ImGui::ColorEdit4(label.c_str(), &color.x);
}

bool editor::PropertyDrawer::drawMaterials(const std::string& label, dzemikk::MeshRenderer* renderer,
                                           const InspectorContext& ctx) {
    if (!renderer) {
        return false;
    }

    auto& materials = renderer->getMaterials();

    bool changed = false;

    ImGui::Text("Materials: %zu", materials.size());

    for (size_t i = 0; i < materials.size(); i++) {

        ImGui::PushID(static_cast<int>(i));

        auto material = materials[i];

        std::string header = "Material " + std::to_string(i);

        if (!material) {

            ImGui::Text("Material %zu: null", i);

            ImGui::SameLine();

            std::string createButton = "Create##" + std::to_string(i);

            if (ImGui::Button(createButton.c_str())) {

                auto newMaterial = std::make_shared<dzemikk::Material>();
                renderer->setMaterial(i, newMaterial);

                changed = true;
            }

            ImGui::PopID();
            continue;
        }

        if (ImGui::TreeNode(header.c_str())) {

            auto shaderHandle = material->getShaderHandle();

            if (PropertyDrawer::drawShader("Shader", shaderHandle, ctx)) {
                material->setShader(shaderHandle);
                changed = true;
            }

            std::string removeButton = "Remove##" + std::to_string(i);

            if (ImGui::Button(removeButton.c_str())) {

                renderer->setMaterial(i, nullptr);

                ImGui::TreePop();
                ImGui::PopID();

                return true;
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (ImGui::Button("Add Material")) {

        auto material = std::make_shared<dzemikk::Material>();
        renderer->setMaterial(materials.size(), material);

        changed = true;
    }

    return changed;
}

bool editor::PropertyDrawer::drawModel(const std::string& label,
                                       dzemikk::AssetHandle<dzemikk::Model>& handle,
                                       const InspectorContext& ctx) {
    enum class ModelSource { Primitive, Custom };

    constexpr std::string_view prefix = "primitive/";

    ImGui::PushID(label.c_str());

    std::string path = handle.get() ? handle.getAssetPath() : "";

    static std::unordered_map<std::string, ModelSource> sourceStates;
    static std::unordered_map<std::string, int> primitiveStates;

    if (!sourceStates.contains(label)) {
        sourceStates[label] = ModelSource::Custom;
        primitiveStates[label] = 0;

        if (path.starts_with(prefix)) {
            sourceStates[label] = ModelSource::Primitive;
            primitiveStates[label] = std::stoi(path.substr(prefix.size()));
        }
    }

    auto& source = sourceStates[label];
    auto& primitiveIndex = primitiveStates[label];

    const char* sourceItems[] = {"Primitive", "Custom"};

    int current = static_cast<int>(source);
    if (ImGui::Combo("Source", &current, sourceItems, IM_ARRAYSIZE(sourceItems))) {
        source = static_cast<ModelSource>(current);
    }

    bool changed = false;

    if (source == ModelSource::Primitive) {
        const char* primitives[] = {"Cube", "Quad", "Sphere", "Capsule"};

        if (ImGui::Combo("Primitive", &primitiveIndex, primitives, IM_ARRAYSIZE(primitives))) {
            auto primitive =
                static_cast<dzemikk::PrimitiveMeshLibrary::PrimitiveMesh>(primitiveIndex);

            handle = ctx.assetManager->getPrimitiveModel(primitive);
            changed = true;
        }
    }

    else {
        static std::unordered_map<std::string, std::array<char, 256>> buffers;
        auto& buffer = buffers[label];

        std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());

        ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                         ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL")) {
                const char* droppedPath = (const char*)payload->Data;

                auto newHandle = ctx.assetManager->get<dzemikk::Model>(droppedPath);

                if (newHandle.get()) {
                    handle = newHandle;
                    changed = true;

                    std::snprintf(buffer.data(), buffer.size(), "%s", droppedPath);
                }
            }

            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemDeactivatedAfterEdit()) {
            auto newHandle = ctx.assetManager->get<dzemikk::Model>(buffer.data());

            if (newHandle.get()) {
                handle = newHandle;
                changed = true;
            }
        }
    }

    ImGui::PopID();
    return changed;
}

bool editor::PropertyDrawer::drawTexture(const std::string& label,
                                         dzemikk::AssetHandle<dzemikk::Texture>& handle,
                                         const InspectorContext& ctx) {
    std::string path = handle.get() ? handle.getAssetPath() : "";

    static std::unordered_map<std::string, std::array<char, 256>> buffers;
    auto& buffer = buffers[label];

    std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());

    bool changed = false;

    ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                     ImGuiInputTextFlags_EnterReturnsTrue);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE")) {
            const char* droppedPath = (const char*)payload->Data;

            auto newHandle = ctx.assetManager->get<dzemikk::Texture>(droppedPath);

            if (newHandle.get()) {
                handle = newHandle;
                changed = true;

                std::snprintf(buffer.data(), buffer.size(), "%s", droppedPath);
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        auto newHandle = ctx.assetManager->get<dzemikk::Texture>(buffer.data());

        if (newHandle.get()) {
            handle = newHandle;
            changed = true;
        }
    }

    return changed;
}

bool editor::PropertyDrawer::drawFont(const std::string& label,
                                      dzemikk::AssetHandle<dzemikk::Font>& handle,
                                      const InspectorContext& ctx) {

    std::string path = handle.get() ? handle.getAssetPath() : "";

    static std::unordered_map<std::string, std::array<char, 256>> textBuffers;
    auto& buffer = textBuffers[label];

    std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());

    bool changed = false;

    ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                     ImGuiInputTextFlags_EnterReturnsTrue);

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_FONT")) {

            const char* droppedPath = (const char*)payload->Data;

            auto newHandle = ctx.assetManager->get<dzemikk::Font>(droppedPath);

            if (newHandle.get()) {
                handle = newHandle;
                changed = true;
                std::snprintf(buffer.data(), buffer.size(), "%s", droppedPath);
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {

        auto newHandle = ctx.assetManager->get<dzemikk::Font>(buffer.data());

        if (newHandle.get()) {
            handle = newHandle;
            changed = true;
        }
    }

    return changed;
}

bool editor::PropertyDrawer::drawUIEvents(const std::string& label,
                                          dzemikk::IUIInteractable* interactable,
                                          const InspectorContext&) {
    if (!interactable)
        return false;

    bool changed = false;

    ImGui::Text("%s", label.c_str());
    ImGui::Separator();

    auto& eventMap = interactable->getEventActionsRef();

    for (auto& [eventType, actions] : eventMap) {

        ImGui::PushID(static_cast<int>(eventType));

        if (ImGui::TreeNode(toString(eventType))) {

            for (int i = 0; i < (int)actions.size(); ++i) {

                ImGui::PushID(i);

                char buffer[128];
                std::snprintf(buffer, sizeof(buffer), "%s", actions[i].c_str());

                if (ImGui::InputText("Action ID", buffer, sizeof(buffer))) {

                    std::string old = actions[i];
                    std::string now = buffer;

                    interactable->removeEventListener(eventType, old);
                    interactable->addEventListener(eventType, now);

                    actions[i] = now;
                    changed = true;
                }

                if (ImGui::Button("Remove")) {
                    interactable->removeEventListener(eventType, actions[i]);
                    ImGui::PopID();
                    changed = true;
                    break;
                }

                ImGui::PopID();
            }

            static char newAction[128] = "";

            ImGui::InputText("New Action", newAction, sizeof(newAction));

            if (ImGui::Button("Add")) {
                if (newAction[0] != '\0') {
                    interactable->addEventListener(eventType, newAction);
                    newAction[0] = '\0';
                    changed = true;
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::Spacing();
    ImGui::Separator();

    static int selectedEvent = 0;
    const char* items[] = {"Click", "Enter", "Exit", "ValueChanged"};

    ImGui::Combo("Add Event Type", &selectedEvent, items, IM_ARRAYSIZE(items));

    static char newEventAction[128] = "";
    ImGui::InputText("Action ID (new event)", newEventAction, sizeof(newEventAction));

    if (ImGui::Button("Add Event Binding")) {
        if (newEventAction[0] != '\0') {
            interactable->addEventListener(fromIndex(selectedEvent), newEventAction);
            newEventAction[0] = '\0';
            changed = true;
        }
    }

    return changed;
}