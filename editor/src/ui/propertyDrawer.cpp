#include "ui/propertyDrawer.h"

#include "renderer/material.h"

#include <imgui.h>

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

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", path.c_str());

    if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string newPath = buffer;

        auto newHandle = ctx.assetManager->get<dzemikk::Shader>(newPath);

        if (!newHandle.get()) {

            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to load shader!");

            return false;
        }

        handle = newHandle;

        return true;
    }

    return false;
}

bool editor::PropertyDrawer::drawColor(const std::string& label, glm::vec4& color) {
    return ImGui::ColorEdit4("Color", &color.x);
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

    std::string path;

    if (handle.get()) {
        path = handle.getAssetPath();
    } else {
        path = "";
    }

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
    } else {

        static std::unordered_map<std::string, std::array<char, 256>> textBuffers;

        auto& buffer = textBuffers[label];

        std::snprintf(buffer.data(), buffer.size(), "%s", path.c_str());

        if (ImGui::InputText(label.c_str(), buffer.data(), buffer.size(),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            auto newHandle = ctx.assetManager->get<dzemikk::Model>(buffer.data());

            if (!newHandle.get()) {

                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to load model!");
            } else {

                handle = newHandle;
                changed = true;
            }
        }
    }

    ImGui::PopID();

    return changed;
}