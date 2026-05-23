#include "editor.h"

#if DZEMIKK_DEV_TOOLS

#include "assetManager/assetmanager.h"
#include "assetManagerPanel.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/scenemanager.h"
#include "ecs/serialize/sceneSerializer.h"
#include "hierarchyPanel.h"
#include "inspectorPanel.h"
#include "renderer/material.h"
#include "renderer/renderer.h"
#include <renderer/font.h>
#include <ecs/components/ui/uiBuilder.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/components/ui/uiButton.h>
#include <ecs/components/ui/uiCheckbox.h>
#include <ecs/components/ui/uiDropdown.h>
#include <ecs/components/ui/uiSlider.h>

#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#endif

#include "scenePanel.h"

editor::Editor::Editor(dzemikk::Engine* engine) : _engine(engine) {
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    _hierarchyPanel = std::make_unique<HierarchyPanel>();
    _inspectorPanel = std::make_unique<InspectorPanel>();
    _assetManagerPanel = std::make_unique<AssetManagerPanel>();
    _scenePanel = std::make_unique<ScenePanel>();
}

editor::Editor::~Editor() = default;

void editor::Editor::start() {

    setupEditor();

    _engine->SetUserUpdateCallback([this]() {

#if DZEMIKK_DEV_TOOLS
        renderDockspace();

        if (_showHierarchy) {
            _hierarchyPanel->draw(_activeScene, _selectedObject);
            _hierarchyPanel->setEditor(this);
        }

        if (_showInspector) {
            InspectorContext context;
            context.assetManager = _engine->getAssetManager();
            _inspectorPanel->draw(_selectedObject, context);
        }

        if (_showAssetManager) {
            _assetManagerPanel->draw(_engine->getAssetManager());
        }

        _scenePanel->draw(_engine->getRenderer());

        renderBottomBar();

        for (auto& op : _deferredOps) {
            op();
        }
        _deferredOps.clear();

#endif
    });

    _engine->start();
}

void editor::Editor::createEmptyObject(const std::string& name, dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, name, parent]() noexcept{
        auto* go = _activeScene->createGameObject(name);

        if (parent) {
            parent->addChild(go);
        }

        if (go->getComponent<dzemikk::Transform>()) {
            go->transform()->setPosition({0, 0, 0});
        }

        _selectedObject = go;
    });
}

void editor::Editor::createUIButton(dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, parent]() noexcept {
        auto* assetManager = _engine->getAssetManager();

        auto font = assetManager->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");

        auto quadMesh =
            assetManager->getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

        auto quadShader = assetManager->get<dzemikk::Shader>("shaders/quad");

        auto quadMat = std::make_shared<dzemikk::Material>();
        quadMat->setShader(quadShader);

        dzemikk::UIBuilder::UIButtonParams params{.name = "Checkbox",
                                                  .size = {200.0F, 60.0F},
                                                  .text = "Button",
                                                  .textFont = font,
                                                  .mesh = quadMesh,
                                                  .material = quadMat};

        auto* buttonGO = dzemikk::UIBuilder::createButton(parent, params);

        _selectedObject = buttonGO;
    });
}

void editor::Editor::createUICheckbox(dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, parent]() noexcept {
        auto* assetManager = _engine->getAssetManager();

        auto quadMesh =
            assetManager->getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

        auto quadShader = assetManager->get<dzemikk::Shader>("shaders/quad");

        auto quadMat = std::make_shared<dzemikk::Material>();
        quadMat->setShader(quadShader);

        dzemikk::UIBuilder::UICheckboxParams params{
            .name = "Checkbox",
            .size = {50.0F, 50.0F},
            .bgMesh = quadMesh,
            .checkmarkMesh = quadMesh,
            .bgMat = quadMat,
            .checkmarkMat = quadMat,
        };

        auto* checkboxGO = dzemikk::UIBuilder::createCheckbox(parent, params);

        _selectedObject = checkboxGO;
    });
}

void editor::Editor::createUISlider(dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, parent]() noexcept {
        auto* assetManager = _engine->getAssetManager();

        auto quadMesh =
            assetManager->getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

        auto quadShader = assetManager->get<dzemikk::Shader>("shaders/quad");

        auto quadMat = std::make_shared<dzemikk::Material>();
        quadMat->setShader(quadShader);

        dzemikk::UIBuilder::UISliderParams params{
            .name = "Slider",
            .size = {200.0F, 20.0F},
            .bgMesh = quadMesh,
            .fillMesh = quadMesh,
            .handleMesh = quadMesh,
            .bgMat = quadMat,
            .fillMat = quadMat,
            .handleMat = quadMat,
        };

        auto* sliderGO = dzemikk::UIBuilder::createSlider(parent, params);

        _selectedObject = sliderGO;
    });
}

void editor::Editor::createUIDropdown(dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, parent]() noexcept {
        auto* assetManager = _engine->getAssetManager();

        auto quadMesh =
            assetManager->getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

        auto quadShader = assetManager->get<dzemikk::Shader>("shaders/quad");

        auto quadMat = std::make_shared<dzemikk::Material>();
        quadMat->setShader(quadShader);

        auto font = assetManager->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");

        dzemikk::UIBuilder::UIDropdownParams params{
            .name = "Dropdown",
            .size = {400.0F, 70.0F},
            .options =
                {
                    {.text = "Option 1", .value = "opt1"},
                    {.text = "Option 2", .value = "opt2"},
                },
            .optionHeight = 70.0F,
            .text = "Select an option",
            .textFont = font,
            .bgMesh = quadMesh,
            .arrowMesh = quadMesh,
            .optionMesh = quadMesh,
            .optionsBgMesh = quadMesh,
            .bgMat = quadMat,
            .arrowMat = quadMat,
            .optionMat = quadMat,
            .optionsBgMat = quadMat,
        };

        auto* sliderGO = dzemikk::UIBuilder::createDropdown(parent, params);

        _selectedObject = sliderGO;
    });
}

void editor::Editor::deleteObject(dzemikk::GameObject* gameObject) {
    _deferredOps.emplace_back([this, gameObject]() noexcept {
        if (!gameObject) {
            return;
        }

        if (!_activeScene) {
            return;
        }

        if (_selectedObject == gameObject) {
            _selectedObject = nullptr;
        }

        _activeScene->destroyGameObject(gameObject);
    });
}

void editor::Editor::reparentObject(dzemikk::GameObject* child, dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, child, parent]() noexcept {
        if (child == parent) {
            return;
        }

        child->setParent(parent);
    });
}

void editor::Editor::instantiatePrefab(const std::string& path, dzemikk::GameObject* parent) {
    _deferredOps.emplace_back([this, path, parent]() noexcept {
        auto prefabJson = _engine->getAssetManager()->get<nlohmann::json>(path);
        auto *prefab = dzemikk::PrefabSerializer::instantiate(*_activeScene, *prefabJson.get(),
                                               _engine->getAssetManager());
        if (parent != nullptr) {
            prefab->setParent(parent);
        }
    });
}

void editor::Editor::setupEditor() {
    if (!_engine) {
        return;
    }

    auto* sceneManager = _engine->getSceneManager();
    auto scene = _engine->getAssetManager()->get<dzemikk::Scene>("scenes/s8.json");
    std::shared_ptr<dzemikk::Scene> sceneShared(scene.get(), [](dzemikk::Scene*) {});
    sceneManager->loadScene(sceneShared);
    sceneManager->setActiveScene(sceneShared);

    _activeScene = scene.get();

    for (const auto& obj : _activeScene->getObjects()) {
        if (auto *camera = obj->getComponent<dzemikk::Camera>()) {
            if (camera->getProjectionType() == dzemikk::Camera::ProjectionType::Perspective) {
                auto *camera = obj->getComponent<dzemikk::Camera>();
                _engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);
            }

            if (camera->getProjectionType() == dzemikk::Camera::ProjectionType::Orthographic) {
                auto *camera = obj->getComponent<dzemikk::Camera>();
                _engine->getRenderer()->getCameraSystem().setActiveUICamera(camera);
            }
        }
    }

    _editorInitialized = true;
}

void editor::Editor::renderDockspace() {

#if DZEMIKK_DEV_TOOLS

    setupDockspaceWindow();
    renderMainMenuBar();
    handleSaveScenePopup();
    renderDockspaceArea();
    setupInitialLayout();

    ImGui::End();

#endif
}

void editor::Editor::setupDockspaceWindow() {
    static bool dockspaceOpen = true;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                   ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);

    ImGui::Begin("DockSpaceWindow", &dockspaceOpen, windowFlags);

    ImGui::PopStyleVar(2);
}

void editor::Editor::renderMainMenuBar() {
    if (!ImGui::BeginMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save Scene As...")) {
            _showSaveScenePopup = true;
        }

        if (ImGui::MenuItem("Exit")) {
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Hierarchy", nullptr, &_showHierarchy);
        ImGui::MenuItem("Inspector", nullptr, &_showInspector);
        ImGui::MenuItem("Asset Manager", nullptr, &_showAssetManager);
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

void editor::Editor::handleSaveScenePopup() {
    if (!_showSaveScenePopup) {
        return;
    }

    ImGui::OpenPopup("Save Scene");

    if (!ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    _showSaveScenePopup = false;

    ImGui::TextUnformatted("Save current scene");
    ImGui::Separator();

    ImGui::InputText("Path", static_cast<char*>(_scenePathBuffer), sizeof(_scenePathBuffer));

    ImGui::Spacing();

    if (ImGui::Button("Save", ImVec2(120.0F, 0.0F))) {

        std::string path = openSaveFileDialog();

        if (!path.empty()) {
            try {
                nlohmann::json sceneJson = dzemikk::SceneSerializer::serialize(*_activeScene);

                std::ofstream file(path);
                if (file.is_open()) {
                    file << sceneJson.dump(4);
                }
            } catch (const std::exception& e) {
                spdlog::error("Failed to save scene: {}", e.what());
            }
        }

        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120.0F, 0.0F))) {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void editor::Editor::renderDockspaceArea() {
    ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");

    ImGui::DockSpace(dockspaceID, ImVec2(0.0F, 0.0F), ImGuiDockNodeFlags_PassthruCentralNode);
}

void editor::Editor::setupInitialLayout() {

    static bool initialized = false;
    if (initialized) {
        return;
    }

    initialized = true;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");

    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);

    ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

    ImGuiID dockMain = dockspaceID;

    ImGuiID dockLeft =
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.20F, nullptr, &dockMain);

    ImGuiID dockRight =
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.25F, nullptr, &dockMain);

    ImGuiID dockBottom =
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.25F, nullptr, &dockMain);

    ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
    ImGui::DockBuilderDockWindow("Inspector", dockRight);
    ImGui::DockBuilderDockWindow("Asset Manager", dockBottom);
    ImGui::DockBuilderDockWindow("Scene", dockMain);

    ImGui::DockBuilderFinish(dockspaceID);
}

void editor::Editor::renderBottomBar() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;

    ImVec2 size = ImVec2(viewport->Size.x, 22.0F);
    ImVec2 pos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - size.y);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12F, 0.12F, 0.12F, 1.0F));

    ImGui::Begin("BottomBar", nullptr, flags);

    ImGui::TextUnformatted(_activeScene ? "Scene: Loaded" : "Scene: None");

    ImGui::SameLine();

    const char* name = _selectedObject ? _selectedObject->getName().c_str() : "None";
    ImGui::TextUnformatted("| Selected: ");
    ImGui::SameLine();
    ImGui::TextUnformatted(name);

    ImGui::SameLine();

    ImGui::SetCursorPosX(viewport->Size.x - 200);
    ImGui::TextUnformatted("dzemikk editor");

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

std::string editor::Editor::openSaveFileDialog() {
    std::array<char, MAX_PATH> fileName{};

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = fileName.data();
    ofn.nMaxFile = MAX_PATH;

    ofn.lpstrFilter = "Scene Files (*.json)\0*.json\0"
                      "All Files (*.*)\0*.*\0";

    ofn.nFilterIndex = 1;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    ofn.lpstrDefExt = "json";

    if (GetSaveFileNameA(&ofn)) {
        return fileName.data();
    }

    return {};
}