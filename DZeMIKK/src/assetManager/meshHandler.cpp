#include "assetManager/meshHandler.h"
#include "renderer/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

void* dzemikk::MeshHandler::load(const std::string& path) {
    return loadMeshFromFile(path);
}

dzemikk::Mesh* dzemikk::MeshHandler::loadMeshFromFile(const std::string& path) {
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

void dzemikk::MeshHandler::reload(void* asset, const std::string& path) {
    reloadMesh(path, static_cast<Mesh*>(asset));
}

void dzemikk::MeshHandler::reloadMesh(const std::string& path, Mesh* mesh) {
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

void dzemikk::MeshHandler::unload(void* asset) {
    delete static_cast<Mesh*>(asset);
}
