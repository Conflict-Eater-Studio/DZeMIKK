#include "assetManager/assetmanager.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/font.h"
#include "renderer/skybox.h"
#include "renderer/texture.h"
#include "audio/sound.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <stb/stb_image.h>
#include <fstream>
#include <iterator>
#include <corecrt_math_defines.h>

void dzemikk::AssetManager::Initialize() {
    _pathIndex.clear();

    auto rootOpt = findResRoot();

    if (!rootOpt) {
        std::cerr << "[AssetManager] ERROR: cannot find 'res' folder!\n";
        return;
    }

    _rootPath = rootOpt->string();
    std::replace(_rootPath.begin(), _rootPath.end(), '\\', '/');

    for (auto& p : std::filesystem::recursive_directory_iterator(_rootPath)) {
        if (!p.is_regular_file())
            continue;

        std::string fullPath = p.path().string();

        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

        std::string relative = std::filesystem::relative(p.path(), _rootPath).string();
        std::replace(relative.begin(), relative.end(), '\\', '/');

        _pathIndex[relative] = fullPath;
    }

    initPrimitiveMeshes();
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
    if (type == std::type_index(typeid(Mesh))) {
        return loadMeshFromFile(resolvePath(id));
    }

    if (type == std::type_index(typeid(Shader))) {
        return loadShaderFromFile(id);
    }

    if (type == std::type_index(typeid(Font))) {
        return loadFontFromFile(resolvePath(id));
    }

    if (type == std::type_index(typeid(Skybox))) {
        return loadSkyboxFromFile(id);
    }

    if (type == std::type_index(typeid(Texture))) {
        return loadTextureFromFile(resolvePath(id));
    }

    if (type == std::type_index(typeid(Sound))) {
        return loadSoundFromFile(resolvePath(id));
    }

    return nullptr;
}

std::string dzemikk::AssetManager::resolvePath(const std::string& id) {
    auto it = _pathIndex.find(id);
    if (it != _pathIndex.end())
        return it->second;

    return id;
}

dzemikk::Mesh* dzemikk::AssetManager::loadMeshFromFile(const std::string& id) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(id, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
    }

    if (!scene->HasMeshes()) {
        std::cerr << "NO MESHES IN FILE" << std::endl;
    }

    aiMesh* ai_mesh = scene->mMeshes[0];

    std::vector<float> vertices;

    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
        vertices.push_back(ai_mesh->mVertices[i].x);
        vertices.push_back(ai_mesh->mVertices[i].y);
        vertices.push_back(ai_mesh->mVertices[i].z);

        vertices.push_back(ai_mesh->mNormals[i].x);
        vertices.push_back(ai_mesh->mNormals[i].y);
        vertices.push_back(ai_mesh->mNormals[i].z);
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
        aiFace face = ai_mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto mesh = new dzemikk::Mesh();
    mesh->createIndexed(vertices.data(), ai_mesh->mNumVertices, indices.data(), indices.size(), 6);

    return mesh;
}

dzemikk::Texture* dzemikk::AssetManager::loadTextureFromFile(const std::string& id, bool flipVertical) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

    unsigned char* data = stbi_load(id.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << id << std::endl;
        return nullptr;
    }

    auto texture = new Texture();
    texture->initFromData(data, width, height, channels);

    stbi_image_free(data);

    return texture;
}

dzemikk::Shader* dzemikk::AssetManager::loadShaderFromFile(const std::string& id) {
    std::string vertPath = resolvePath(id + ".vert");
    std::string fragPath = resolvePath(id + ".frag");

    std::ifstream vFile(vertPath);
    std::ifstream fFile(fragPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to open shader:\n" << vertPath << "\n" << fragPath << "\n";
        return nullptr;
    }

    std::string vertSrc((std::istreambuf_iterator<char>(vFile)), std::istreambuf_iterator<char>());

    std::string fragSrc((std::istreambuf_iterator<char>(fFile)), std::istreambuf_iterator<char>());

    return new dzemikk::Shader(vertSrc.c_str(), fragSrc.c_str());
}

dzemikk::Font* dzemikk::AssetManager::loadFontFromFile(const std::string& path) {
    auto font = new dzemikk::Font();

    if (!font->load(path)) {
        std::cerr << "Failed to load font: " << path << std::endl;
        delete font;
        return nullptr;
    }

    return font;
}

dzemikk::Skybox* dzemikk::AssetManager::loadSkyboxFromFile(const std::string& id) {
    std::vector<std::string> faces = {
        resolvePath(id + "/right.png"), resolvePath(id + "/left.png"),
        resolvePath(id + "/top.png"),   resolvePath(id + "/bottom.png"),
        resolvePath(id + "/front.png"), resolvePath(id + "/back.png")};

    auto skybox = new dzemikk::Skybox();

    try {
        skybox->loadCubemap(faces);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Skybox load failed: " << e.what() << std::endl;
        delete skybox;
        return nullptr;
    }

    return skybox;
}

void dzemikk::AssetManager::Unload(const std::string& id) {
    auto it = _assets.find(id);
    if (it == _assets.end())
        return;

    AssetEntry& entry = it->second;

    if (entry.type == std::type_index(typeid(Mesh))) {
        delete static_cast<Mesh*>(entry.data);
    } else if (entry.type == std::type_index(typeid(Shader))) {
        delete static_cast<Shader*>(entry.data);
    } else if (entry.type == std::type_index(typeid(Font))) {
        delete static_cast<Font*>(entry.data);
    } else if (entry.type == std::type_index(typeid(Skybox))) {
        delete static_cast<Skybox*>(entry.data);
    } else if (entry.type == std::type_index(typeid(Texture))) {
        delete static_cast<Texture*>(entry.data);
    } else if (entry.type == std::type_index(typeid(Sound))) {
        delete static_cast<Sound*>(entry.data);
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

dzemikk::Sound* dzemikk::AssetManager::loadSoundFromFile(const std::string& id) {
    FMOD::Sound* fmodSound = nullptr;

    system->createSound(id.c_str(), FMOD_DEFAULT, nullptr, &fmodSound);

    auto sound = new Sound();
    sound->init(fmodSound);

    return sound;
}

void dzemikk::AssetManager::reloadInternal(const std::string& id, void* data, std::type_index type) {
    if (type == std::type_index(typeid(Mesh))) {
        reloadMesh(id, static_cast<Mesh*>(data));
    }

    if (type == std::type_index(typeid(Shader))) {
        std::cout << "Recompile shader";
        reloadShader(id, static_cast<Shader*>(data));
    }

    if (type == std::type_index(typeid(Font))) {
        reloadFont(id, static_cast<Font*>(data));
    }

    if (type == std::type_index(typeid(Skybox))) {
        reloadSkybox(id, static_cast<Skybox*>(data));
    }

    if (type == std::type_index(typeid(Texture))) {
        reloadTexture(id, static_cast<Texture*>(data));
    }

    if (type == std::type_index(typeid(Sound))) {
        reloadSound(id, static_cast<Sound*>(data));
    }
}

void dzemikk::AssetManager::reloadShader(const std::string& id, dzemikk::Shader* shader) {
    std::string vertPath = resolvePath(id + ".vert");
    std::string fragPath = resolvePath(id + ".frag");

    std::ifstream vFile(vertPath);
    std::ifstream fFile(fragPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to reload shader\n";
        return;
    }

    std::string vertSrc((std::istreambuf_iterator<char>(vFile)), {});
    std::string fragSrc((std::istreambuf_iterator<char>(fFile)), {});

    shader->recompile(vertSrc.c_str(), fragSrc.c_str());
}

void dzemikk::AssetManager::reloadMesh(const std::string& id, dzemikk::Mesh* mesh) {
    auto path = resolvePath(id);

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes())
        return;

    auto* ai_mesh = scene->mMeshes[0];

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned i = 0; i < ai_mesh->mNumVertices; i++) {
        vertices.insert(vertices.end(),
                        {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z,
                         ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z});
    }

    for (unsigned i = 0; i < ai_mesh->mNumFaces; i++) {
        auto& f = ai_mesh->mFaces[i];
        for (unsigned j = 0; j < f.mNumIndices; j++)
            indices.push_back(f.mIndices[j]);
    }

    mesh->recreate(vertices.data(), indices.data(), ai_mesh->mNumVertices, indices.size(), 6);
}

void dzemikk::AssetManager::reloadFont(const std::string& id, dzemikk::Font* font) {
    std::string path = resolvePath(id);

    if (!font->load(path)) {
        std::cerr << "Failed to reload font: " << path << "\n";
    }
}

void dzemikk::AssetManager::reloadSkybox(const std::string& id, dzemikk::Skybox* skybox) {
    std::vector<std::string> faces = {
        resolvePath(id + "/right.png"), resolvePath(id + "/left.png"),
        resolvePath(id + "/top.png"),   resolvePath(id + "/bottom.png"),
        resolvePath(id + "/front.png"), resolvePath(id + "/back.png")};

    try {
        skybox->loadCubemap(faces);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Skybox reload failed: " << e.what() << "\n";
    }
}

void dzemikk::AssetManager::reloadTexture(const std::string& id, dzemikk::Texture* texture) {
    std::string path = resolvePath(id);

    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to reload texture: " << path << "\n";
        return;
    }

    GLuint newTex;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);

    GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE,
                 data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    texture->replaceTexture(newTex, width, height, channels);
}

void dzemikk::AssetManager::reloadSound(const std::string& id, dzemikk::Sound* sound) {
    std::string path = resolvePath(id);

    FMOD::Sound* newSound = nullptr;

    FMOD_RESULT result = system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &newSound);

    if (result != FMOD_OK || !newSound) {
        std::cerr << "Failed to reload sound: " << path << "\n";
        return;
    }

    sound->replaceSound(newSound);
}