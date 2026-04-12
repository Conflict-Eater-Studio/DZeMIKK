#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "assetManager/assetmanager.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/font.h"
#include "renderer/skybox.h"
#include "renderer/texture.h"
#include "audio/sound.h"

#include "assetManager/fontHandler.h"
#include "assetManager/meshHandler.h"
#include "assetManager/shaderHandler.h"
#include "assetManager/skyboxHandler.h"
#include "assetManager/soundHandler.h"
#include "assetManager/textureHandler.h"

#include <iostream>
#include <stb/stb_image.h>
#include <fstream>
#include <iterator>
#include <corecrt_math_defines.h>
#include <assimp/version.h>

void dzemikk::AssetManager::Initialize() {
#if DZEMIKK_DEV_TOOLS
    auto t0 = std::chrono::high_resolution_clock::now();
    spdlog::info("[AssetManager] Initialization started");

    spdlog::info("Assimp version: {}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(),
                 aiGetVersionRevision());
#endif

    _pathIndex.clear();

    auto rootOpt = findResRoot();
    if (!rootOpt) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[AssetManager] Cannot find 'res' folder!");
#else
        std::cerr << "[AssetManager] ERROR: cannot find 'res' folder!\n";
#endif
        return;
    }

    _rootPath = rootOpt->string();
    std::replace(_rootPath.begin(), _rootPath.end(), '\\', '/');

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Resource root: {}", _rootPath);
#endif

    size_t fileCount = 0;

    for (auto& p : std::filesystem::recursive_directory_iterator(_rootPath)) {
        if (!p.is_regular_file())
            continue;

        std::string fullPath = p.path().string();
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

        std::string relative = std::filesystem::relative(p.path(), _rootPath).string();
        std::replace(relative.begin(), relative.end(), '\\', '/');

        _pathIndex[relative] = fullPath;
        ++fileCount;
    }
    RegisterHandlers();
    initPrimitiveMeshes();

#if DZEMIKK_DEV_TOOLS
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    spdlog::info("[AssetManager] Initialization finished in {} ms", ms);
#endif
}

std::optional<std::filesystem::path> dzemikk::AssetManager::findResRoot() {
    namespace fs = std::filesystem;

    fs::path start = fs::current_path();

    for (auto& p : fs::recursive_directory_iterator(start)) {
        if (p.is_directory() && p.path().filename() == "res") {
            return fs::absolute(p.path());
        }
    }

    return std::nullopt;
}

void dzemikk::AssetManager::UnInitialize() {
    for (auto& [id, entry] : _assets) {
        if (entry.type == std::type_index(typeid(Mesh))) {
            delete static_cast<Mesh*>(entry.data);
        }
        else if (entry.type == std::type_index(typeid(unsigned int))) {
            delete static_cast<unsigned int*>(entry.data);
        }
    }

    _assets.clear();
}

void* dzemikk::AssetManager::loadInternal(const std::string& id, std::type_index type) {
    auto it = _handlers.find(type);
    if (it == _handlers.end())
        return nullptr;

    std::string path = resolvePath(id); 

    return it->second->load(path);
}

std::string dzemikk::AssetManager::resolvePath(const std::string& id) {
    auto it = _pathIndex.find(id);
    if (it != _pathIndex.end())
        return it->second;

    return _rootPath + "/" + id;
}

void dzemikk::AssetManager::Unload(const std::string& id) {
    auto it = _assets.find(id);
    if (it == _assets.end())
        return;

    AssetEntry& entry = it->second;

    auto handlerIt = _handlers.find(entry.type);
    if (handlerIt != _handlers.end()) {
        handlerIt->second->unload(entry.data);
    }

    _assets.erase(it);
}

void dzemikk::AssetManager::initPrimitiveMeshes() {
    _builtinMeshes[PrimitiveMesh::Cube] = createCubeMesh();
    _builtinMeshes[PrimitiveMesh::Quad] = createQuadMesh();
    _builtinMeshes[PrimitiveMesh::Sphere] = createSphereMesh();
    _builtinMeshes[PrimitiveMesh::Capsule] = createCapsuleMesh();
}

dzemikk::Mesh* dzemikk::AssetManager::createCubeMesh() {
    dzemikk::Mesh* mesh = new dzemikk::Mesh();

    float vertices[] = {// --- Front face
                        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
                        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

                        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
                        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

                        // --- Back face
                        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
                        -1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

                        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
                        -1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

                        // --- Left face
                        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, -1.0f, 0.0f,
                        0.0f, -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

                        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
                        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,

                        // --- Right face
                        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
                        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

                        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
                        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

                        // --- Top face
                        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
                        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,

                        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

                        // --- Bottom face
                        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f,
                        0.0f, -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,

                        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,
                        0.0f, 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f};

    mesh->create(vertices, 36, 6);
    return mesh;
}

dzemikk::Mesh* dzemikk::AssetManager::GetPrimitive(PrimitiveMesh type) {
    auto it = _builtinMeshes.find(type);
    if (it != _builtinMeshes.end())
        return it->second;

    return nullptr;
}

void dzemikk::AssetManager::setFMODSystem(FMOD::System* system) {
    this->system = system;

    auto it = _handlers.find(typeid(Sound));
    auto* handler = static_cast<SoundHandler*>(it->second.get());
    handler->system = system;
}

dzemikk::Mesh* dzemikk::AssetManager::createQuadMesh() {
    dzemikk::Mesh* mesh = new dzemikk::Mesh();
    float vertices[] = {// pos        // uv
                        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,

                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    mesh->create2D(vertices, 6);
    return mesh;
}

dzemikk::Mesh* dzemikk::AssetManager::createSphereMesh() {
    const int stacks = 16;
    const int slices = 16;
    const float radius = 0.5f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= stacks; ++i) {
        float V = (float)i / stacks;
        float phi = V * glm::pi<float>();

        for (int j = 0; j <= slices; ++j) {
            float U = (float)j / slices;
            float theta = U * glm::two_pi<float>();

            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);

            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(pos);

            vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z});
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    auto mesh = new dzemikk::Mesh();
    mesh->createIndexed(vertices.data(), vertices.size() / 6, indices.data(), indices.size(), 6);

    return mesh;
}

dzemikk::Mesh* dzemikk::AssetManager::createCapsuleMesh() {
    auto* mesh = new dzemikk::Mesh();

    const int segments = 24;
    const int rings = 12;

    const float radius = 0.5f;
    const float height = 1.0f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    auto addVertex = [&](const glm::vec3& pos) {
        glm::vec3 normal = glm::normalize(pos);

        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
    };

    auto indexOf = [&](int ring, int seg) { return ring * (segments + 1) + seg; };

    float halfH = height * 0.5f;

    for (int i = 0; i <= 1; i++) {
        float y = -halfH + i * height;

        for (int j = 0; j <= segments; j++) {
            float a = (float)j / segments * glm::two_pi<float>();

            glm::vec3 pos(cos(a) * radius, y, sin(a) * radius);
            addVertex(pos);
        }
    }

    for (int j = 0; j < segments; j++) {
        int i0 = j;
        int i1 = j + 1;
        int i2 = j + segments + 1;
        int i3 = j + segments + 2;

        indices.insert(indices.end(), {(unsigned)i0, (unsigned)i2, (unsigned)i1, (unsigned)i1,
                                       (unsigned)i2, (unsigned)i3});
    }

    int baseTop = vertices.size() / 6;

    for (int i = 0; i <= rings; i++) {
        float v = (float)i / rings;
        float phi = v * (glm::half_pi<float>());

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos(cos(theta) * cos(phi) * radius, sin(phi) * radius + halfH,
                          sin(theta) * cos(phi) * radius);

            addVertex(pos);
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < segments; j++) {
            int a = baseTop + i * (segments + 1) + j;
            int b = a + segments + 1;

            indices.insert(indices.end(), {(unsigned)a, (unsigned)b, (unsigned)a + 1,
                                           (unsigned)a + 1, (unsigned)b, (unsigned)b + 1});
        }
    }

    int baseBottom = vertices.size() / 6;

    for (int i = 0; i <= rings; i++) {
        float v = (float)i / rings;
        float phi = v * (glm::half_pi<float>());

        for (int j = 0; j <= segments; j++) {
            float u = (float)j / segments;
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos(cos(theta) * cos(phi) * radius, -sin(phi) * radius - halfH,
                          sin(theta) * cos(phi) * radius);

            addVertex(pos);
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < segments; j++) {
            int a = baseBottom + i * (segments + 1) + j;
            int b = a + segments + 1;

            indices.insert(indices.end(), {(unsigned)a, (unsigned)a + 1, (unsigned)b,
                                           (unsigned)a + 1, (unsigned)b + 1, (unsigned)b});
        }
    }

    mesh->createIndexed(vertices.data(), vertices.size() / 6, indices.data(), indices.size(), 6);

    return mesh;
}

void dzemikk::AssetManager::RegisterHandlers() {
    _handlers[typeid(Mesh)] = std::make_unique<MeshHandler>();
    _handlers[typeid(Shader)] = std::make_unique<ShaderHandler>();
    _handlers[typeid(Texture)] = std::make_unique<TextureHandler>();
    _handlers[typeid(Skybox)] = std::make_unique<SkyboxHandler>();
    _handlers[typeid(Font)] = std::make_unique<FontHandler>();
    _handlers[typeid(Sound)] = std::make_unique<SoundHandler>();
}

void dzemikk::AssetManager::reloadInternal(const std::string& path, void* data, std::type_index type) {
    auto it = _handlers.find(type);
    if (it == _handlers.end())
        return;

    it->second->reload(data, resolvePath(path));
}