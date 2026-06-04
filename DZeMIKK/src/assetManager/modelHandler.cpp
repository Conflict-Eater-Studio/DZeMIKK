#include "assetManager/modelHandler.h"
#include "renderer/StaticMesh.h"
#include "renderer/model.h"

#include "animation/boneTrack.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

#include "assetManager/skeletonBuilder.h"
#include "assetManager/meshBuilder.h"
#include "assetManager/animationLoader.h"

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

bool dzemikk::ModelHandler::isAssimpHelperNode(const std::string& name) {
    return name.find("_$AssimpFbx$") != std::string::npos;
}

bool dzemikk::ModelHandler::isBoneNode(const std::string& name, const dzemikk::Skeleton& skeleton) {
    return skeleton.getBoneIndex(name) != -1;
}

void dzemikk::ModelHandler::printNodeHierarchyForMesh(aiNode* node, const aiScene* scene,
                               const dzemikk::Skeleton& skeleton, int depth) {
#if DZEMIKK_DEV_TOOLS
    if (!node) {
        return;
    }

    std::string indent(static_cast<std::size_t>(depth * 2), ' ');
    std::string name = node->mName.C_Str();

    bool isHelper = isAssimpHelperNode(name);
    bool isBone = isBoneNode(name, skeleton);
    bool hasMeshes = node->mNumMeshes > 0;

    std::string line = indent + name;

    if (isHelper) {
        line += " [ASSIMP_HELPER]";
    }
    if (isBone) {
        line += " [BONE]";
    }
    if (hasMeshes) {
        line += " [MESHES: " + std::to_string(node->mNumMeshes) + "]";
    }

    spdlog::info("[ModelHandler] {}", line);

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        printNodeHierarchyForMesh(node->mChildren[i], scene, skeleton, depth + 1);
    }
#endif
}

dzemikk::ModelHandler::Result
dzemikk::ModelHandler::load(const std::string& path,
                            LoadExecutionMode loadExecutionMode) {
    auto model = loadModelFromFile(path, loadExecutionMode);

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

    auto newModel = loadModelFromFile(path,LoadExecutionMode::Sync, LoadMode::MeshOnly);
    if (!newModel) {
        return false;
    }

    auto* model = asset.get();

    model->clear();

    for (const auto& subMesh : newModel->getSubMeshes()) {
        model->addMesh(subMesh.mesh, subMesh.materialIndex);
    }

    return true;
}

void dzemikk::ModelHandler::unload(Handle& asset) {
    asset = Handle{};
}

std::shared_ptr<dzemikk::Model>
dzemikk::ModelHandler::loadModelFromFile(const std::string& path,
                                         LoadExecutionMode loadExecutionMode, LoadMode loadMode) {

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    const bool hasAnimations = scene->mNumAnimations > 0;

    auto model = std::make_shared<Model>();

    auto *skeleton = SkeletonBuilder::build(scene);
    model->setSkeleton(std::shared_ptr<Skeleton>(skeleton));


    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    #if DZEMIKK_DEV_TOOLS
        //spdlog::info("[ModelHandler] MESH {}: {}", i, scene->mMeshes[i]->mName.C_Str());
        //printNodeHierarchyForMesh(scene->mRootNode, scene, *skeleton);
    #endif
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {

        const aiMesh* mesh = scene->mMeshes[i];
        const bool isSkinned = mesh->HasBones();

        if (loadExecutionMode == LoadExecutionMode::Async) {
            if (!isSkinned) {
                auto raw = MeshBuilder::buildStaticMeshRaw(mesh);
                model->addPending(raw);
            } else {
                auto raw = MeshBuilder::buildSkinnedMeshRaw(mesh, *skeleton);
                model->addPending(raw);
            }
        } else {
            if (!isSkinned) {
                auto staticMesh = MeshBuilder::buildStaticMesh(mesh);
                model->addMesh(staticMesh, mesh->mMaterialIndex);
            } else {
                auto skinnedMesh = MeshBuilder::buildSkinnedMesh(mesh, *skeleton);
                model->addMesh(skinnedMesh, mesh->mMaterialIndex);
            }
        }
    }

    if (scene->mNumAnimations > 0 && loadMode == LoadMode::All) {

        /*
        for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
            const aiAnimation* animation = scene->mAnimations[i];

            std::cout << "Animation [" << i << "]: " << animation->mName.C_Str() << std::endl;
        }
        */

        AnimationLoader::load(scene, *skeleton);
    }

    return model;
}


