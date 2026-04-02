#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"

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
    _stats.reset();
    dzemikk::Shader* currentShader = nullptr;

    for (auto* r : _meshRenderers) {
        if (!r->mesh || !r->material || !r->transform)
            continue;
        
        auto* shader = r->material->shader;
        if (currentShader != shader) {
            shader->bind();
            currentShader = shader;
            _stats.stateChanges++;
        }

        shader->setMat4("model", r->transform->getWorldMatrix());
        shader->setMat4("view", _view);
        shader->setMat4("projection", _projection);
        
        r->mesh->draw();
        
        _stats.drawCalls++;
        _stats.renderedObjects++;
        _stats.vertexCount += r->mesh->vertexCount;
        _stats.triangleCount += r->mesh->vertexCount / 3;
    }

    for (auto* r : _spriteRenderers) {
        if (!r->mesh || !r->material || !r->transform)
            continue;
            
        auto* shader = r->material->shader;
        if (currentShader != shader) {
            shader->bind();
            currentShader = shader;
            _stats.stateChanges++;
        }

        shader->setMat4("model", r->transform->getWorldMatrix());
        shader->setMat4("view", glm::mat4(1.0f));
        shader->setMat4("projection", _uiProjection);
        
        r->mesh->draw();
        
        _stats.drawCalls++;
        _stats.renderedObjects++;
        _stats.vertexCount += r->mesh->vertexCount;
        _stats.triangleCount += r->mesh->vertexCount / 3;
    }
}