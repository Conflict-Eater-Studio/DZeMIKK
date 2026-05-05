#include "assetManager/modelHandler.h"
#include "renderer/mesh.h"
#include "renderer/model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

dzemikk::ModelHandler::Result dzemikk::ModelHandler::load(const std::string& path) {
    auto model = loadModelFromFile(path);

    if (!model) {
        std::cerr << "Failed to load mesh: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {model, AssetError::None};
}

bool dzemikk::ModelHandler::reload(Handle& asset, const std::string& path) {
    if (!asset) {
        return false;
    }

    auto newModel = loadModelFromFile(path);
    if (!newModel) {
        return false;
    }

    auto* model = asset.get();

    model->clear();

    const auto& newSubMeshes = newModel->getSubMeshes();
    for (const auto& subMesh : newSubMeshes) {
        model->addMesh(subMesh.mesh, subMesh.materialIndex);
    }

    return true;
}

void dzemikk::ModelHandler::unload(Handle& asset) {
    asset = Handle{};
}

std::shared_ptr<dzemikk::Model> dzemikk::ModelHandler::loadModelFromFile(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        return nullptr;
    }

    auto model = std::make_shared<Model>();

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* aiMesh = scene->mMeshes[i];

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        vertices.reserve(aiMesh->mNumVertices * 6);

        for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
            vertices.push_back(aiMesh->mVertices[v].x);
            vertices.push_back(aiMesh->mVertices[v].y);
            vertices.push_back(aiMesh->mVertices[v].z);

            if (aiMesh->HasNormals()) {
                vertices.push_back(aiMesh->mNormals[v].x);
                vertices.push_back(aiMesh->mNormals[v].y);
                vertices.push_back(aiMesh->mNormals[v].z);
            } else {
                vertices.insert(vertices.end(), {0, 0, 0});
            }
        }

        for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
            const aiFace& face = aiMesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        auto mesh = std::make_shared<Mesh>();
        mesh->createIndexed(vertices.data(), aiMesh->mNumVertices, indices.data(), indices.size(),
                            6);

        model->addMesh(mesh, aiMesh->mMaterialIndex);
    }

    return model;
}
