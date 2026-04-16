#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "renderer/texture.h"
#include "renderer/model.h"
#include "ecs/componentRegistry.h"

#include "core/profiler.h"
#include "ecs/component.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/textRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>

void dzemikk::Renderer::initialize() {
    _view = glm::mat4(1.0f);
    _projection = glm::mat4(1.0f);
    _uiProjection = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f);

    glGenBuffers(1, &_uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _uboMatrices, 0, 2 * sizeof(glm::mat4));

    glEnable(GL_MULTISAMPLE);

    _skybox = new Skybox();

    const char* vertexSrc = R"(
    #version 330 core
    layout (location = 0) in vec4 vertex; // pos.xy, uv.xy

    out vec2 TexCoords;

    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
    )";

    const char* fragmentSrc = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 color;

    uniform sampler2D text;
    uniform vec3 textColor;

    void main() {
        float alpha = texture(text, TexCoords).r;
        color = vec4(textColor, alpha);
    }
    )";

    _textShader = new Shader(vertexSrc, fragmentSrc);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void dzemikk::Renderer::uninitialize() {
    for (auto& batch : _batches) {
        if (batch.instanceVBO != 0) {
            glDeleteBuffers(1, &batch.instanceVBO);
            batch.instanceVBO = 0;
        }
    }

    if (_uboMatrices != 0) {
        glDeleteBuffers(1, &_uboMatrices);
        _uboMatrices = 0;
    }

    _batches.clear();
}

void dzemikk::Renderer::render() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_skybox && _sceneCamera) {

        float time = glfwGetTime();

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * 0.1f, glm::vec3(0, 1, 0));
        glm::mat4 viewNoTrans = _sceneCamera->getView() * rotation;

        _skybox->render(viewNoTrans, _sceneCamera->getProjection());

        Profiler::Get().stats.drawCalls++;
        Profiler::Get().stats.renderedObjects++;
        Profiler::Get().stats.vertexCount += 36;
        Profiler::Get().stats.triangleCount += 12;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    dzemikk::ComponentRegistry::get().getComponents<Camera>(_cameras);

    if (_sceneCamera) {
        if (_sceneCamera->isDirty()) {
            _frustum.update(_sceneCamera->getViewProjection());
        }

        _view = _sceneCamera->getView();
        _projection = _sceneCamera->getProjection();

        glm::mat4 matrices[2] = {_projection, _view};

        glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrices), matrices);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    for (auto& batch : _batches) {
        batch.models.clear();
    }

    dzemikk::ComponentRegistry::get().getComponents<MeshRenderer>(_meshRenderers);

    {
        DZ_PROFILE_CPU("Frustum Culling & Batching");
        for (auto* r : _meshRenderers) {
            if (!r->isValid())
                continue;

            Model* model = r->getModel();
            glm::mat4 transform = r->getTransform()->getWorldMatrix();

            float radius = r->getCullingRadius();

            if (!_frustum.isSphereVisible(r->getTransform()->getPosition(), radius))
                continue;

            for (size_t i = 0; i < model->getSubMeshes().size(); i++) {
                const auto* sub = model->getSubMesh(i);
                if (!sub)
                    continue;

                Mesh* mesh = sub->mesh.get();

                Material* material = nullptr;

                if (i < r->getMaterials().size())
                    material = r->getMaterial(i);

                if (!material && !r->getMaterials().empty())
                    material = r->getMaterial(0);

                if (!material)
                    continue;

                Batch* batch = nullptr;

                for (auto& b : _batches) {
                    if (b.mesh == mesh && b.material == material) {
                        batch = &b;
                        break;
                    }
                }

                if (!batch) {
                    _batches.push_back({});
                    batch = &_batches.back();

                    batch->mesh = mesh;
                    batch->material = material;

                    glGenBuffers(1, &batch->instanceVBO);
                }

                batch->models.push_back(transform);
                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += batch->mesh->getVertexCount();
                Profiler::Get().stats.triangleCount += batch->mesh->getVertexCount() / 3;
            }
        }
    }

    {
        DZ_PROFILE_GPU("Opaque Rendering (Batches)");
        for (auto& batch : _batches) {
            if (batch.models.empty())
                continue;

            Mesh* mesh = batch.mesh;
            Material* material = batch.material;
            Shader* shader = material->getShader();

            shader->bind();

            shader->setVec3("lightDir", glm::vec3(1.0f, -1.0f, -1.0f));
            shader->setVec3("lightColor", glm::vec3(1.0f));
            shader->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.2f));

            mesh->drawInstanced(batch.models, batch.instanceVBO);
            Profiler::Get().stats.drawCalls++;
        }
    }

    if (_uiCamera)
        _uiProjection = _uiCamera->getProjection();
    glDisable(GL_DEPTH_TEST);

    dzemikk::ComponentRegistry::get().getComponents<SpriteRenderer>(_spriteRenderers);

    {
        DZ_PROFILE_GPU("Transparent Rendering (Sprites)");
        for (auto* r : _spriteRenderers) {
            if (!r->isValid())
                continue;

            Shader* shader = r->getMaterial()->getShader();
            shader->bind();

            shader->setMat4("model", r->getTransform()->getWorldMatrix());
            shader->setMat4("projection", _uiProjection);

            if (r->hasTexture()) {
                r->getTexture()->bind(0);
                shader->setInt("spriteTexture", 0);
                shader->setBool("useTexture", true);
            } else {
                shader->setBool("useTexture", false);
            }

            shader->setVec4("spriteColor", r->getColor());

            r->getMesh()->draw();

            Profiler::Get().stats.drawCalls++;

            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += r->getMesh()->getVertexCount();
            Profiler::Get().stats.triangleCount += r->getMesh()->getVertexCount() / 3;
        }
    }

    std::vector<ImageRenderer*> uiSprites;
    ComponentRegistry::get().getEnabledComponents<ImageRenderer>(uiSprites);

    {
        DZ_PROFILE_GPU("Image Rendering");
        std::ranges::sort(uiSprites, [](ImageRenderer* a, ImageRenderer* b) {
            unsigned int az = a->getOwner()->rectTransform()->getZIndex();
            unsigned int bz = a->getOwner()->rectTransform()->getZIndex();
            return az < bz;
        });
        for (auto* r : uiSprites) {
            if (!r->isValid()) {
                continue;
            }

            Shader* shader = r->getMaterial()->getShader();
            shader->bind();

            shader->setMat4("model", r->getRectTransform()->getWorldMatrix());
            shader->setMat4("projection", _uiProjection);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r->getTexture());
            shader->setInt("spriteTexture", 0);
            shader->setVec4("spriteColor", r->getColor());

            r->getMesh()->draw();
            Profiler::Get().stats.drawCalls++;

            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += r->getMesh()->getVertexCount();
            Profiler::Get().stats.triangleCount += r->getMesh()->getVertexCount() / 3;
        }
    }

    std::vector<TextRenderer*> texts;
    ComponentRegistry::get().getComponents<TextRenderer>(texts);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        DZ_PROFILE_GPU("Text Rendering");
        for (auto* t : texts) {
            if (!t->isValid())
                continue;

            Shader* shader = _textShader;
            shader->bind();

            shader->setMat4("projection", _uiProjection);
            shader->setVec3("textColor", t->color);

            float x = t->getOwner()->transform()->getPosition().x;
            float y = t->getOwner()->transform()->getPosition().y;

            glBindVertexArray(textVAO);
            for (char c : t->text) {
                Character ch = t->font->characters[c];

                float xpos = x + ch.bearing.x * t->scale;
                float ypos = y - (ch.size.y - ch.bearing.y) * t->scale;

                float w = ch.size.x * t->scale;
                float h = ch.size.y * t->scale;

                float vertices[6][4] = {
                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                    {xpos + w, ypos, 1.0f, 1.0f},

                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                    {xpos + w, ypos + h, 1.0f, 0.0f}};

                glBindTexture(GL_TEXTURE_2D, ch.textureID);

                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                Profiler::Get().stats.drawCalls++;

                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += 6;
                Profiler::Get().stats.triangleCount += 2;

                x += (ch.advance >> 6) * t->scale;
            }
            glBindVertexArray(0);
        }
    }

    std::vector<UITextRenderer*> uiTexts;
    ComponentRegistry::get().getComponents<UITextRenderer>(uiTexts);

    {
        DZ_PROFILE_GPU("UI Text Rendering");
        for (auto* t : uiTexts) {
            if (!t->isValid()) {
                continue;
            }

            auto* owner = t->getOwner();
            if (!owner) {
                continue;
            }

            auto* rect = owner->getComponent<RectTransform>();
            if (!rect) {
                continue;
            }

            const glm::mat4 world = rect->getWorldMatrix();
            const glm::vec4 p0 = world * glm::vec4(0.0F, 0.0F, 0.0F, 1.0F);
            const float originX = p0[0];
            const float originY = p0[1];
            const glm::vec2 rectSize = rect->getSize();

            float penX = 0.0F;
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();

            for (char c : t->text) {
                auto glyphIt = t->font->characters.find(c);
                if (glyphIt == t->font->characters.end()) {
                    continue;
                }

                const Character& ch = glyphIt->second;
                const float ypos = -(ch.size.y - ch.bearing.y) * t->scale;
                const float h = ch.size.y * t->scale;

                minY = std::min(minY, ypos);
                maxY = std::max(maxY, ypos + h);

                penX += (ch.advance >> 6) * t->scale;
            }

            if (minY > maxY) {
                minY = 0.0F;
                maxY = 0.0F;
            }

            const float textWidth = penX;
            const float textHeight = maxY - minY;

            float offsetX = 0.0F;
            switch (t->horizontalAlign) {
            case UITextRenderer::HorizontalAlign::Left:
                offsetX = 0.0F;
                break;
            case UITextRenderer::HorizontalAlign::Center:
                offsetX = (rectSize.x - textWidth) * 0.5F;
                break;
            case UITextRenderer::HorizontalAlign::Right:
                offsetX = rectSize.x - textWidth;
                break;
            }

            float offsetY = 0.0F;
            switch (t->verticalAlign) {
            case UITextRenderer::VerticalAlign::Bottom:
                offsetY = -minY;
                break;
            case UITextRenderer::VerticalAlign::Middle:
                offsetY = (rectSize.y - textHeight) * 0.5F - minY;
                break;
            case UITextRenderer::VerticalAlign::Top:
                offsetY = rectSize.y - textHeight - minY;
                break;
            }

            Shader* shader = _textShader;
            shader->bind();

            shader->setMat4("projection", _uiProjection);
            shader->setVec3("textColor", t->color);

            float x = originX + offsetX;
            float y = originY + offsetY;

            glBindVertexArray(textVAO);
            for (char c : t->text) {
                auto glyphIt = t->font->characters.find(c);
                if (glyphIt == t->font->characters.end()) {
                    continue;
                }

                const Character& ch = glyphIt->second;

                float xpos = x + ch.bearing.x * t->scale;
                float ypos = y - (ch.size.y - ch.bearing.y) * t->scale;

                float w = ch.size.x * t->scale;
                float h = ch.size.y * t->scale;

                float vertices[6][4] = {
                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                    {xpos + w, ypos, 1.0f, 1.0f},

                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                    {xpos + w, ypos + h, 1.0f, 0.0f}};

                glBindTexture(GL_TEXTURE_2D, ch.textureID);

                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                Profiler::Get().stats.drawCalls++;

                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += 6;
                Profiler::Get().stats.triangleCount += 2;

                x += (ch.advance >> 6) * t->scale;
            }
            glBindVertexArray(0);
        }
    }
}

const dzemikk::Camera* dzemikk::Renderer::getActiveSceneCamera() const {
    return _sceneCamera;
}
const dzemikk::Camera* dzemikk::Renderer::getActiveUICamera() const {
    return _uiCamera;
}

void dzemikk::Renderer::setActiveSceneCamera(dzemikk::Camera* camera) {
    if (!camera)
        return;

    _sceneCamera = camera;
}

void dzemikk::Renderer::setActiveUICamera(dzemikk::Camera* camera) {
    if (!camera)
        return;

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

void dzemikk::Renderer::setSkybox(Skybox* skybox) {
    if (!skybox) {
        _skybox = nullptr;
        return;
    }
    _skybox = skybox;
}

const dzemikk::Skybox* dzemikk::Renderer::getSkybox() const {
    return _skybox;
}