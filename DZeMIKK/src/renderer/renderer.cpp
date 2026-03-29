#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include <iostream>

void dzemikk::Renderer::Initialize() {
    _view = glm::mat4(1.0f);
    _projection = glm::mat4(1.0f);
    _uiProjection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
}

void dzemikk::Renderer::UnInitialize() {
    _meshRenderers.clear();
    _spriteRenderers.clear();
}

void dzemikk::Renderer::registerRenderer(MeshRenderer* renderer) {
    _meshRenderers.push_back(renderer);
}

void dzemikk::Renderer::unregisterRenderer(MeshRenderer* renderer) {
    _meshRenderers.erase(std::remove(_meshRenderers.begin(), _meshRenderers.end(), renderer),
                         _meshRenderers.end());
}

void dzemikk::Renderer::registerSpriteRenderer(SpriteRenderer* renderer) {
    _spriteRenderers.push_back(renderer);
}

void dzemikk::Renderer::unregisterSpriteRenderer(SpriteRenderer* renderer) {
    _spriteRenderers.erase(std::remove(_spriteRenderers.begin(), _spriteRenderers.end(), renderer),
                           _spriteRenderers.end());
}

void dzemikk::Renderer::setCamera(const glm::mat4& view, const glm::mat4& projection) {
    _view = view;
    _projection = projection;
}

void dzemikk::Renderer::setUIProjection(const glm::mat4& ortho) {
    _uiProjection = ortho;
}

void dzemikk::Renderer::render() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); 

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_sceneCamera) {
        _view = _sceneCamera->getView();
        _projection = _sceneCamera->getProjection();
    }

    for (auto* r : _meshRenderers) {
        if (!r->mesh || !r->material || !r->transform)
            continue;
        auto* shader = r->material->shader;
        shader->bind();
        shader->setMat4("model", r->transform->getWorldMatrix());
        shader->setMat4("view", _view);
        shader->setMat4("projection", _projection);
        shader->setVec3("lightDir", glm::vec3(1.0f, -1.0f, 1.0f));
        shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.2f));
        r->mesh->draw();
    }

    if (_uiCamera) {
        _uiProjection = _uiCamera->getProjection();
    }

    for (auto* r : _spriteRenderers) {
        if (!r->mesh || !r->material || !r->transform)
            continue;
        auto* shader = r->material->shader;
        shader->bind();
        shader->setMat4("model", r->transform->getWorldMatrix());
        shader->setMat4("projection", _uiProjection);
        r->mesh->draw();
    }
}

const dzemikk::Camera* dzemikk::Renderer::getActiveSceneCamera() const {
    return _sceneCamera;
}
const dzemikk::Camera* dzemikk::Renderer::getActiveUICamera() const {
    return _uiCamera;
}

void dzemikk::Renderer::registerCamera(const dzemikk::Camera* camera) {
    if (!camera)
        return;

    if (std::find(_cameras.begin(), _cameras.end(), camera) == _cameras.end()) {
        _cameras.push_back(camera);
    }
}

void dzemikk::Renderer::unregisterCamera(const dzemikk::Camera* camera) {
    if (!camera)
        return;

    auto iter = std::find(_cameras.begin(), _cameras.end(), camera);
    if (iter != _cameras.end()) {
        if (*iter == _sceneCamera)
            _sceneCamera = nullptr;
        if (*iter == _uiCamera)
            _uiCamera = nullptr;

        _cameras.erase(iter);
    }
}

void dzemikk::Renderer::setActiveSceneCamera(const dzemikk::Camera* camera) {
    if (!camera)
        return;

    registerCamera(camera);
    _sceneCamera = camera;
}

void dzemikk::Renderer::setActiveUICamera(const dzemikk::Camera* camera) {
    if (!camera)
        return;

    registerCamera(camera);
    _uiCamera = camera;
}

void dzemikk::Renderer::setActiveSceneCameraById(int cameraId) {
    for (auto& cam : _cameras) {
        if (cam->getId() == cameraId) {
            _sceneCamera = cam;
            return;
        }
    }
    std::cerr << "[Renderer] Warning: Scene camera with ID " << cameraId << " not found.\n";
}

void dzemikk::Renderer::setActiveUICameraById(int cameraId) {
    for (auto& cam : _cameras) {
        if (cam->getId() == cameraId) {
            _uiCamera = cam;
            return;
        }
    }
    std::cerr << "[Renderer] Warning: UI camera with ID " << cameraId << " not found.\n";
}