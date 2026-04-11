#include "assetManager/assetmanager.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <stb/stb_image.h>
#include <fstream>
#include <iterator>

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

void* dzemikk::AssetManager::LoadInternal(const std::string& id, std::type_index type) {
    if (type == std::type_index(typeid(Mesh))) {
        return loadMeshFromFile(resolvePath(id));
    }

    if (type == std::type_index(typeid(Shader))) {
        return loadShaderFromFile(id);
    }

    if (type == std::type_index(typeid(unsigned int))) {
        auto tex = new unsigned int;
        *tex = loadTextureFromFile(resolvePath(id));
        return tex;
    }

    return nullptr;
}

std::string dzemikk::AssetManager::resolvePath(const std::string& id) {
    auto it = _pathIndex.find(id);
    if (it != _pathIndex.end())
        return it->second;

    return id;
}

dzemikk::Mesh* dzemikk::AssetManager::loadMeshFromFile(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
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

GLuint dzemikk::AssetManager::loadTextureFromFile(const std::string& path, bool flipVertical) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

dzemikk::Shader* dzemikk::AssetManager::loadShaderFromFile(const std::string& basePath) {
    std::string vertPath = resolvePath(basePath + ".vert");
    std::string fragPath = resolvePath(basePath + ".frag");

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