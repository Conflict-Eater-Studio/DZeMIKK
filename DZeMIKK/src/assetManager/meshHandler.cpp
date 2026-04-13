#include "assetManager/meshHandler.h"
#include "renderer/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

void* dzemikk::MeshHandler::load(const std::string& path) {
    return loadMeshFromFile(path).release();
}

std::unique_ptr<dzemikk::Mesh> dzemikk::MeshHandler::loadMeshFromFile(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                     aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
    }

    if (!scene->HasMeshes()) {
        std::cerr << "NO MESHES IN FILE" << "\n";
    }

    auto* aiMesh = scene->mMeshes[0];

    std::vector<float> vertices;

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
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            auto* indicesPtr = face.mIndices;
            indices.push_back(indicesPtr[j]);
        }
    }

    auto mesh = std::make_unique<dzemikk::Mesh>();
    mesh->createIndexed(vertices.data(), aiMesh->mNumVertices, indices.data(), indices.size(), 6);

    return mesh;
}

void dzemikk::MeshHandler::reload(void* asset, const std::string& path) {
    reloadMesh(path, static_cast<Mesh*>(asset));
}

void dzemikk::MeshHandler::reloadMesh(const std::string& path, Mesh* mesh) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
    }

    if (!scene->HasMeshes()) {
        std::cerr << "NO MESHES IN FILE" << "\n";
    }

    auto* aiMesh = scene->mMeshes[0];

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned i = 0; i < aiMesh->mNumVertices; i++) {
        vertices.insert(vertices.end(),
                        {aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z,
                         aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z});
    }

    for (unsigned faceIndex = 0; faceIndex < aiMesh->mNumFaces; faceIndex++) {
        auto& face = aiMesh->mFaces[faceIndex];

        for (unsigned indexIndex = 0; indexIndex < face.mNumIndices; indexIndex++) {
            indices.push_back(face.mIndices[indexIndex]);
        }
    }

    mesh->recreate(vertices.data(), indices.data(), aiMesh->mNumVertices, indices.size(), 6);
}

void dzemikk::MeshHandler::unload(void* asset) {
    delete static_cast<Mesh*>(asset);
}
