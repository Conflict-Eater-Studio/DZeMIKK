#include "renderer/cameraSystem.h"

#include "ecs/gameobject.h"
#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include "ecs/componentRegistry.h"

#include <iostream>
#include <glad.h>

const dzemikk::Camera* dzemikk::CameraSystem::getActiveSceneCamera() const {
    return _sceneCamera;
}

const dzemikk::Camera* dzemikk::CameraSystem::getActiveUICamera() const {
    return _uiCamera;
}

void dzemikk::CameraSystem::update(dzemikk::RenderContext& ctx) {
    dzemikk::ComponentRegistry::get().getComponents<Camera>(_cameras);

    if (_sceneCamera) {
        ctx.frustum->update(_sceneCamera->getViewProjection());

        ctx.view = _sceneCamera->getView();
        ctx.projection = _sceneCamera->getProjection();

        glm::mat4 matrices[2] = {ctx.projection, ctx.view};

        glBindBuffer(GL_UNIFORM_BUFFER, ctx.uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrices), matrices);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void dzemikk::CameraSystem::setActiveSceneCamera(dzemikk::Camera* camera) {
    if (!camera)
        return;

    _sceneCamera = camera;
}

void dzemikk::CameraSystem::setActiveUICamera(dzemikk::Camera* camera) {
    if (!camera)
        return;

    _uiCamera = camera;
}

void dzemikk::CameraSystem::setActiveSceneCameraById(int cameraId) {
    for (auto& cam : _cameras) {
        if (cam->getId() == cameraId) {
            _sceneCamera = cam;
            return;
        }
    }
    std::cerr << "[Renderer] Warning: Scene camera with ID " << cameraId << " not found.\n";
}

void dzemikk::CameraSystem::setActiveUICameraById(int cameraId) {
    for (auto& cam : _cameras) {
        if (cam->getId() == cameraId) {
            _uiCamera = cam;
            return;
        }
    }
    std::cerr << "[Renderer] Warning: UI camera with ID " << cameraId << " not found.\n";
}