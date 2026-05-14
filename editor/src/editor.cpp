#include "editor.h"

#if DZEMIKK_DEV_TOOLS

#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/scenemanager.h"

#include "renderer/renderer.h"
#include "renderer/material.h"

#include "assetManager/assetmanager.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "hierarchyPanel.h"
#include "inspectorPanel.h"
#include "assetManagerPanel.h"

#include "ecs/serialize/sceneSerializer.h"

#include <fstream>
#include <filesystem>

#endif
#include <iostream>

namespace editor {

Editor::Editor(dzemikk::Engine* engine) : _engine(engine) {
    _hierarchyPanel = std::make_unique<HierarchyPanel>();
    _inspectorPanel = std::make_unique<InspectorPanel>();
    _assetManagerPanel = std::make_unique<AssetManagerPanel>();
}

Editor::~Editor() = default;

void Editor::start() {

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

        renderBottomBar();

        for (auto& op : _deferredOps) {
            op();
        }
        _deferredOps.clear();

#endif
    });

    _engine->start();
}

void Editor::createEmptyObject(const std::string& name, dzemikk::GameObject* parent) {
    _deferredOps.push_back([=]() {
        auto* go = _activeScene->createGameObject(name);

        if (parent) {
            parent->addChild(go);
        }

        go->transform()->setPosition({0, 0, 0});
        _selectedObject = go;
    });
}

void Editor::deleteObject(dzemikk::GameObject* gameObject) {
    _deferredOps.push_back([=]() {
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

void Editor::reparentObject(dzemikk::GameObject* child, dzemikk::GameObject* parent) {
    _deferredOps.push_back([=]() {

        if (child == parent) {
            return;
        }

        child->setParent(parent);
    });
}

void Editor::setupEditor() {

    if (!_engine) {
        return;
    }

    auto* sceneManager = _engine->getSceneManager();

    auto editorScene = std::make_shared<dzemikk::Scene>();

    sceneManager->loadScene(editorScene);
    sceneManager->setActiveScene(editorScene);

    _activeScene = editorScene.get();

    auto* cameraGO = _activeScene->createGameObject("Editor Camera");
    cameraGO->transform()->setPosition({0.0F, 3.0F, 8.0F});
    auto* camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt({0.0F, 0.0F, 0.0F});
    _engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);


    auto* cubeGO = _activeScene->createGameObject("Cube");
    cubeGO->transform()->setPosition({0.0F, 0.0F, 0.0F});
    auto* meshRenderer = cubeGO->addComponent<dzemikk::MeshRenderer>();
    auto model = _engine->getAssetManager()->getPrimitiveModel(
        dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Cube);
    auto shader = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);
    meshRenderer->setModel(model);
    meshRenderer->setMaterial(0, material);
    meshRenderer->setTransform(cubeGO->transform());
    auto* childGO = _activeScene->createGameObject("Cube Child");
    cubeGO->addChild(childGO);
    childGO->transform()->setPosition({2.0F, 0.0F, 0.0F});
    childGO->transform()->setScale({0.5F, 0.5F, 0.5F});
    
    auto* lightGO = _activeScene->createGameObject("Directional Light");
    auto* light = lightGO->addComponent<dzemikk::DirectionalLight>();
    light->setDirection(glm::vec3(-0.5f, -1.0f, -0.3f));
    light->setColor(glm::vec3(1.0f));
    light->setIntensity(1.0f);
    
    _editorInitialized = true;
}

void Editor::renderDockspace() {

#if DZEMIKK_DEV_TOOLS

    static bool dockspaceOpen = true;
    static bool initialized = false;

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

    // ===== MENU BAR =====

    if (ImGui::BeginMenuBar()) {

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

    // ===== OPEN POPUP =====

    if (_showSaveScenePopup) {
        ImGui::OpenPopup("Save Scene");
    }

    // ===== DOCKSPACE =====

    ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");

    ImGui::DockSpace(dockspaceID, ImVec2(0.0F, 0.0F), ImGuiDockNodeFlags_PassthruCentralNode);

    // ===== INITIAL LAYOUT =====

    if (!initialized) {

        initialized = true;

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

        ImGui::DockBuilderFinish(dockspaceID);
    }

    // ===== SAVE SCENE POPUP =====

    if (ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        _showSaveScenePopup = false;

        ImGui::Text("Save current scene");

        ImGui::Separator();

        ImGui::InputText("Path", _scenePathBuffer, sizeof(_scenePathBuffer));

        ImGui::Spacing();

        if (ImGui::Button("Save", ImVec2(120.0F, 0.0F))) {

            try {

                nlohmann::json sceneJson = dzemikk::SceneSerializer::serialize(*_activeScene);


                std::cout << std::filesystem::current_path() << std::endl;
                std::ofstream file(_scenePathBuffer);

                if (file.is_open()) {
                    file << sceneJson.dump(4);
                    file.close();
                }

            } catch (const std::exception& e) {

                // optional logging
                // std::cerr << e.what() << std::endl;
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120.0F, 0.0F))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();

#endif
}

void Editor::renderBottomBar() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;

    ImVec2 size = ImVec2(viewport->Size.x, 22.0f);
    ImVec2 pos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - size.y);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));

    ImGui::Begin("BottomBar", nullptr, flags);

    ImGui::Text("Scene: %s", _activeScene ? "Loaded" : "None");

    ImGui::SameLine();

    ImGui::Text("| Selected: %s", _selectedObject ? _selectedObject->getName().c_str() : "None");

    ImGui::SameLine();

    ImGui::SetCursorPosX(viewport->Size.x - 200);
    ImGui::Text("dzemikk editor");

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace editor