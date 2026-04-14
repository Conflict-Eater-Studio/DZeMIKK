#include "assetManager/meshHandler.h"
#include "renderer/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

dzemikk::MeshHandler::Result dzemikk::MeshHandler::load(const std::string& path) {
    auto mesh = loadMeshFromFile(path);

    if (!mesh) {
        std::cerr << "Failed to load mesh: " << path << "\n";
        return {Handle(), nullptr, AssetError::LoadFailed};
    }

    return {Handle(mesh.get()), mesh, AssetError::None};
}

std::shared_ptr<dzemikk::Mesh> dzemikk::MeshHandler::loadMeshFromFile(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);
    if (!scene) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    if (!scene->HasMeshes()) {
        std::cerr << "NO MESHES IN FILE: " << path << "\n";
        return nullptr;
    }

    const aiMesh* aiMesh = scene->mMeshes[0];

    std::vector<float> vertices;
    vertices.reserve(aiMesh->mNumVertices * 6);

    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
        vertices.push_back(aiMesh->mVertices[i].x);
        vertices.push_back(aiMesh->mVertices[i].y);
        vertices.push_back(aiMesh->mVertices[i].z);

        vertices.push_back(aiMesh->mNormals[i].x);
        vertices.push_back(aiMesh->mNormals[i].y);
        vertices.push_back(aiMesh->mNormals[i].z);
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
        const aiFace& face = aiMesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->createIndexed(vertices.data(), aiMesh->mNumVertices, indices.data(), indices.size(), 6);

    return mesh;
}

bool dzemikk::MeshHandler::reload(Handle& asset, const std::string& path) {
    if (!asset.valid())
        return false;

    return reloadMesh(path, *asset);
}

bool dzemikk::MeshHandler::reloadMesh(const std::string& path, Mesh& mesh) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes())
        return false;

    const aiMesh* ai_mesh = scene->mMeshes[0];

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(ai_mesh->mNumVertices * 6);

    for (unsigned i = 0; i < ai_mesh->mNumVertices; i++) {
        vertices.push_back(ai_mesh->mVertices[i].x);
        vertices.push_back(ai_mesh->mVertices[i].y);
        vertices.push_back(ai_mesh->mVertices[i].z);

        if (ai_mesh->HasNormals()) {
            vertices.push_back(ai_mesh->mNormals[i].x);
            vertices.push_back(ai_mesh->mNormals[i].y);
            vertices.push_back(ai_mesh->mNormals[i].z);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    for (unsigned i = 0; i < ai_mesh->mNumFaces; i++) {
        const aiFace& f = ai_mesh->mFaces[i];

        for (unsigned j = 0; j < f.mNumIndices; j++)
            indices.push_back(f.mIndices[j]);
    }

    mesh.recreate(vertices.data(), indices.data(), ai_mesh->mNumVertices, indices.size(), 6);

    return true;
}

void dzemikk::MeshHandler::unload(Handle& asset) {
    asset = Handle{};
}
