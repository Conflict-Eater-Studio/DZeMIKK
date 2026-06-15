#include "ui/propertyDrawer.h"

#include <renderer/material.h>

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

void editor::PropertyDrawer::drawVec2ReadOnly(const std::string& label, const glm::vec2& value) {
    ImGui::BeginDisabled();
    ImGui::DragFloat2(label.c_str(), const_cast<float*>(&value.x), 0.0F);
    ImGui::EndDisabled();
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
            const char* droppedPath = static_cast<const char*>(payload->Data);

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

bool editor::PropertyDrawer::drawMaterials(const std::string& label,
                                           dzemikk::MeshRenderer* renderer,
                                           const InspectorContext& ctx) {

    if (!renderer) {
        return false;
    }

    const auto& materials = renderer->getMaterials();

    bool changed = false;

    ImGui::Text("Materials: %d", (int)materials.size());

    for (size_t i = 0; i < materials.size(); i++) {
        ImGui::PushID((int)i);

        auto material = materials[i];

        if (!material) {
            ImGui::Text("Material %d: null", (int)i);
            ImGui::SameLine();

            if (ImGui::Button("Create")) {
                renderer->setMaterial(i, std::make_shared<dzemikk::Material>());
                changed = true;
            }

            ImGui::PopID();
            continue;
        }

        std::string header = "Material " + std::to_string(i);

        if (ImGui::TreeNode(header.c_str())) {

            auto shader = material->getShaderHandle();

            if (drawShader("Shader", shader, ctx)) {
                material->setShader(shader);
                changed = true;
            }


            if (ImGui::TreeNode("Textures")) {

                auto albedo = material->getAlbedoTexture()
                                  ? material->getAlbedoTextureHandle()
                                  : dzemikk::AssetHandle<dzemikk::Texture>();

                auto normal = material->getNormalTexture()
                                  ? material->getNormalTextureHandle()
                                  : dzemikk::AssetHandle<dzemikk::Texture>();

                auto metallic = material->getMetallicTexture()
                                    ? material->getMetallicTextureHandle()
                                    : dzemikk::AssetHandle<dzemikk::Texture>();

                auto roughness = material->getRoughnessTexture()
                                     ? material->getRoughnessTextureHandle()
                                     : dzemikk::AssetHandle<dzemikk::Texture>();

                auto ao = material->getAOTexture() ? material->getAOTextureHandle()
                                                   : dzemikk::AssetHandle<dzemikk::Texture>();

                auto emissive = material->getEmissiveTexture()
                                    ? material->getEmissiveTextureHandle()
                                    : dzemikk::AssetHandle<dzemikk::Texture>();

                if (drawTexture("Albedo", albedo, ctx)) {
                    material->setAlbedoTexture(albedo);
                    changed = true;
                }

                if (drawTexture("Normal", normal, ctx)) {
                    material->setNormalTexture(normal);
                    changed = true;
                }

                if (drawTexture("Metallic", metallic, ctx)) {
                    material->setMetallicTexture(metallic);
                    changed = true;
                }

                if (drawTexture("Roughness", roughness, ctx)) {
                    material->setRoughnessTexture(roughness);
                    changed = true;
                }

                if (drawTexture("AO", ao, ctx)) {
                    material->setAOTexture(ao);
                    changed = true;
                }

                if (drawTexture("Emissive", emissive, ctx)) {
                    material->setEmissiveTexture(emissive);
                    changed = true;
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("PBR")) {

                glm::vec3 albedoColor = material->getAlbedoColor();

                if (ImGui::ColorEdit3("Albedo Color", &albedoColor.x)) {
                    material->setAlbedoColor(albedoColor);
                    changed = true;
                }

                float metallic = material->getMetallic();

                if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f)) {
                    material->setMetallic(metallic);
                    changed = true;
                }

                float roughness = material->getRoughness();

                if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) {
                    material->setRoughness(roughness);
                    changed = true;
                }

                float ao = material->getAO();

                if (ImGui::SliderFloat("AO", &ao, 0.0f, 1.0f)) {
                    material->setAO(ao);
                    changed = true;
                }

                ImGui::TreePop();
            }


            if (ImGui::Button("Remove Material")) {
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
        renderer->setMaterial(materials.size(), std::make_shared<dzemikk::Material>());
        changed = true;
    }

    return changed;
}

bool editor::PropertyDrawer::drawModel(const std::string& label,
                                       dzemikk::AssetHandle<dzemikk::Model>& handle,
                                       const InspectorContext& ctx) {
    enum class ModelSource : std::uint8_t { Primitive, Custom };

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

    const std::array<const char*, 2> sourceItems = {"Primitive", "Custom"};

    int current = static_cast<int>(source);
    if (ImGui::Combo("Source", &current, sourceItems.data(), static_cast<int>(sourceItems.size()))) {
        source = static_cast<ModelSource>(current);
    }

    bool changed = false;

    if (source == ModelSource::Primitive) {
        const std::array<const char*, 4> primitives = {"Cube", "Quad", "Sphere", "Capsule"};

        if (ImGui::Combo("Primitive", &primitiveIndex, primitives.data(), static_cast<int>(primitives.size()))) {
            auto primitive =
                static_cast<dzemikk::PrimitiveMeshLibrary::PrimitiveMesh>(primitiveIndex);

            handle = ctx.assetManager->getPrimitiveModel(primitive);
            changed = true;
        }
    } else {
        static std::unordered_map<std::string, std::array<char, 256>> buffers;
        auto& buffer = buffers[label];

        std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());

        ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                         ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL")) {
                const char* droppedPath = static_cast<const char*>(payload->Data);

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

    const auto length = std::min(path.size(), buffer.size() - 1);
    std::memcpy(buffer.data(), path.data(), length);
    buffer[length] = '\0';

    bool changed = false;

    ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                     ImGuiInputTextFlags_EnterReturnsTrue);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE")) {
            const char* droppedPath = static_cast<const char*>(payload->Data);

            auto newHandle = ctx.assetManager->get<dzemikk::Texture>(droppedPath);

            if (newHandle.get()) {
                handle = newHandle;
                changed = true;

                const std::string_view path = droppedPath;
                const auto length = std::min(path.size(), buffer.size() - 1);
                std::memcpy(buffer.data(), path.data(), length);
                buffer[length] = '\0';
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

            const char* droppedPath = static_cast<const char*>(payload->Data);

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
    if (!interactable) {
        return false;
    }

    bool changed = false;

    ImGui::TextUnformatted(label.c_str());
    ImGui::Separator();

    auto& eventMap = interactable->getEventActionsRef();

    for (auto& [eventType, actions] : eventMap) {
        ImGui::PushID(static_cast<int>(eventType));

        if (ImGui::TreeNode(toString(eventType))) {
            for (int i = 0; i < (int)actions.size(); ++i) {
                ImGui::PushID(i);

                std::array<char, 128> buffer;
                std::snprintf(buffer.data(), sizeof(buffer), "%s", actions[i].c_str());

                if (ImGui::InputText("Action ID", buffer.data(), sizeof(buffer))) {
                    std::string old = actions[i];
                    std::string now = buffer.data();

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

            static std::array<char, 128> newAction{};

            ImGui::InputText("New Action", newAction.data(), sizeof(newAction));

            if (ImGui::Button("Add")) {
                if (newAction[0] != '\0') {
                    interactable->addEventListener(eventType, newAction.data());
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
    const std::array<const char*, 4> items = {"Click", "Enter", "Exit", "ValueChanged"};

    ImGui::Combo("Add Event Type", &selectedEvent, items.data(), static_cast<int>(items.size()));

    static std::array<char, 128> newEventAction{};
    ImGui::InputText("Action ID (new event)", newEventAction.data(), newEventAction.size());

    if (ImGui::Button("Add Event Binding")) {
        if (newEventAction[0] != '\0') {
            interactable->addEventListener(fromIndex(selectedEvent), newEventAction.data());
            newEventAction[0] = '\0';
            changed = true;
        }
    }

    return changed;
}