#include "assetManager/meshHandler.h"
#include "renderer/staticMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

dzemikk::MeshHandler::Result
dzemikk::MeshHandler::load(const std::string& path,
                           LoadExecutionMode loadExecutionMode) {
    auto mesh = loadMeshFromFile(path, loadExecutionMode);

    if (!mesh) {
        std::cerr << "Failed to load mesh: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {mesh, AssetError::None};
}

std::shared_ptr<dzemikk::Mesh>
dzemikk::MeshHandler::loadMeshFromFile(const std::string& path,
                                       LoadExecutionMode loadExecutionMode) {
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                    aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    const aiMesh* aiMesh = scene->mMeshes[0];

    std::vector<StaticVertex> vertices;
    vertices.reserve(aiMesh->mNumVertices);

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
        StaticVertex v{};

        v.position = {aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z};

        v.normal = aiMesh->HasNormals() ? glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y,
                                                    aiMesh->mNormals[i].z)
                                        : glm::vec3(0.0f);

        vertices.push_back(v);
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        const aiFace& face = aiMesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto mesh = std::make_shared<StaticMesh>();
    mesh->create(vertices, indices);

    if (loadExecutionMode == LoadExecutionMode::Sync) {
        mesh->uploadToGPU();
    }

    return mesh;
}

bool dzemikk::MeshHandler::reload(Handle& asset, const std::string& path) {
    if (!asset)
        return false;

    return reloadMesh(path, *asset.get());
}

bool dzemikk::MeshHandler::reloadMesh(const std::string& path, Mesh& mesh) {
    auto* staticMesh = dynamic_cast<StaticMesh*>(&mesh);
    if (!staticMesh)
        return false;

    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                    aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes())
        return false;

    const aiMesh* aiMesh = scene->mMeshes[0];

    std::vector<StaticVertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(aiMesh->mNumVertices);

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
        StaticVertex v{};

        v.position = {aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z};

        v.normal = aiMesh->HasNormals() ? glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y,
                                                    aiMesh->mNormals[i].z)
                                        : glm::vec3(0.0f);

        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        const aiFace& f = aiMesh->mFaces[i];

        for (unsigned int j = 0; j < f.mNumIndices; j++)
            indices.push_back(f.mIndices[j]);
    }

    staticMesh->recreate(vertices, indices);

    return true;
}

void dzemikk::MeshHandler::unload(Handle& asset) {
    asset = Handle{};
}
