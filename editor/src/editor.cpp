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

#endif

namespace editor {

Editor::Editor(dzemikk::Engine* engine) : _engine(engine) {}

void Editor::start() {

    setupEditor();
    registerCallbacks();

    _engine->SetUserUpdateCallback([this]() {

#if DZEMIKK_DEV_TOOLS
        renderDockspace();

        if (_showHierarchy) {
            drawHierarchyPanel();
        }

        if (_showInspector) {
            drawInspectorPanel();
        }

#endif
    });

    _engine->start();
}

void Editor::setupEditor() {

    if (!_engine) {
        return;
    }

    auto* sceneManager = _engine->getSceneManager();

    // Create default editor scene
    auto editorScene = std::make_shared<dzemikk::Scene>();

    sceneManager->loadScene(editorScene);
    sceneManager->setActiveScene(editorScene);

    _activeScene = editorScene.get();

    // ================= CAMERA =================

    auto* cameraGO = _activeScene->createGameObject("Editor Camera");

    cameraGO->transform()->setPosition({0.0F, 3.0F, 8.0F});

    auto* camera = cameraGO->addComponent<dzemikk::Camera>();

    camera->lookAt({0.0F, 0.0F, 0.0F});

    _engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);

    // ================= TEST OBJECT =================

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

    // ustaw parent-child
    childGO->setParent(cubeGO);

    // lokalna pozycja wzglêdem rodzica
    childGO->transform()->setPosition({2.0F, 0.0F, 0.0F});
    childGO->transform()->setScale({0.5F, 0.5F, 0.5F});
    auto* meshRendererC = childGO->addComponent<dzemikk::MeshRenderer>();

    auto model2 = _engine->getAssetManager()->getPrimitiveModel(
        dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Sphere);
    meshRendererC->setModel(model2);
    meshRendererC->setMaterial(0, material);
    meshRendererC->setTransform(childGO->transform());

    // ================= LIGHT =================

    auto* lightGO = _activeScene->createGameObject("Directional Light");

    auto* light = lightGO->addComponent<dzemikk::DirectionalLight>();

    light->direction = glm::normalize(glm::vec3(-0.5F, -1.0F, -0.3F));
    light->color = glm::vec3(1.0F);
    light->intensity = 1.0F;

    _editorInitialized = true;
}

void Editor::registerCallbacks() {
    // future:
    // shortcuts
    // gizmos
    // drag & drop
    // selection
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

            if (ImGui::MenuItem("Exit")) {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {

            ImGui::MenuItem("Hierarchy", nullptr, &_showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &_showInspector);

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
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

        // left panel
        ImGuiID dockLeft =
            ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.20F, nullptr, &dockMain);

        // right panel
        ImGuiID dockRight =
            ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.25F, nullptr, &dockMain);

        // dock windows
        ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
        ImGui::DockBuilderDockWindow("Inspector", dockRight);

        ImGui::DockBuilderFinish(dockspaceID);
    }

    ImGui::End();

#endif
}

void Editor::drawHierarchyPanel() {

#if DZEMIKK_DEV_TOOLS

    ImGui::Begin("Hierarchy");

    // ===== ADD BUTTON =====
    if (ImGui::Button("+ Add Cube")) {
        auto* cube = createCube("Runtime Cube");

        if (_selectedObject) {
            cube->setParent(_selectedObject);
        }

        _selectedObject = cube;
    }

    if (!_activeScene) {
        ImGui::Text("No active scene");
        ImGui::End();
        return;
    }

    const auto& objects = _activeScene->getObjects();
    for (const auto& go : objects) {
        if (go->getParent() == nullptr) {
            drawGameObjectNode(go.get());
        }
    }

    ImGui::End();

#endif
}

void Editor::drawInspectorPanel() {

#if DZEMIKK_DEV_TOOLS

    ImGui::Begin("Inspector");

    if (!_selectedObject) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    ImGui::Text("Name: %s", _selectedObject->getName().c_str());

    auto* transform = _selectedObject->transform();

    if (transform) {

        glm::vec3 position = transform->getPosition();
        glm::vec3 scale = transform->getScale();

        if (ImGui::DragFloat3("Position", &position.x, 0.1F)) {
            transform->setPosition(position);
        }

        if (ImGui::DragFloat3("Scale", &scale.x, 0.1F)) {
            transform->setScale(scale);
        }
    }

    ImGui::End();

#endif
}

void Editor::drawGameObjectNode(dzemikk::GameObject* gameObject) {

#if DZEMIKK_DEV_TOOLS

    if (!gameObject) {
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (_selectedObject == gameObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const auto& children = gameObject->getChildren();

    if (children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool opened = ImGui::TreeNodeEx((void*)gameObject, flags, "%s", gameObject->getName().c_str());

    if (ImGui::IsItemClicked()) {
        _selectedObject = gameObject;
    }

    if (opened) {

        for (auto* child : children) {
            drawGameObjectNode(child);
        }

        ImGui::TreePop();
    }

#endif
}

dzemikk::GameObject* Editor::createCube(const std::string& name) {
    if (!_activeScene) {
        return nullptr;
    }

    auto* go = _activeScene->createGameObject(name);

    auto* meshRenderer = go->addComponent<dzemikk::MeshRenderer>();

    auto model = _engine->getAssetManager()->getPrimitiveModel(
        dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Cube);

    auto shader = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");

    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);

    meshRenderer->setModel(model);
    meshRenderer->setMaterial(0, material);
    meshRenderer->setTransform(go->transform());

    return go;
}

} // namespace editor