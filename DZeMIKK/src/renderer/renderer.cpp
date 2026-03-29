#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include <iostream>
#include <map>

void dzemikk::Renderer::Initialize() {
    _view = glm::mat4(1.0f);
    _projection = glm::mat4(1.0f);
    _uiProjection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    glGenBuffers(1, &uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
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

        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(_projection));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                        glm::value_ptr(_view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    using Key = std::pair<Mesh*, Material*>;
    std::map<Key, std::vector<glm::mat4>> batches;

    for (auto* r : _meshRenderers) {
        if (!r->mesh || !r->material || !r->transform)
            continue;

        Key key = {r->mesh, r->material};
        batches[key].push_back(r->transform->getWorldMatrix());
    }

    for (auto& [key, models] : batches) {
        Mesh* mesh = key.first;
        Material* material = key.second;
        Shader* shader = material->shader;

        shader->bind();
        shader->setVec3("lightDir", glm::vec3(1.0f, -1.0f, 1.0f));
        shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.2f));

        GLuint instanceVBO;
        glGenBuffers(1, &instanceVBO);
        glBindVertexArray(mesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(),
                     GL_DYNAMIC_DRAW);

        for (int i = 0; i < 4; i++) {
            glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                                  (void*)(sizeof(glm::vec4) * i));
            glEnableVertexAttribArray(2 + i);
            glVertexAttribDivisor(2 + i, 1);
        }

        glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, models.size());

        glBindVertexArray(0);
        glDeleteBuffers(1, &instanceVBO);
    }

    if (_uiCamera)
        _uiProjection = _uiCamera->getProjection();

    for (auto* r : _spriteRenderers) {
        if (!r->mesh || !r->material || !r->transform)
            continue;

        Shader* shader = r->material->shader;
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