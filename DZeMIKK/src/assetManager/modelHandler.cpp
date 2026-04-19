#include "assetManager/modelHandler.h"
#include "renderer/StaticMesh.h"
#include "renderer/model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void printNode(aiNode* node, int depth = 0) {
    for (int i = 0; i < depth; i++)
        std::cout << "  ";

    std::cout << node->mName.C_Str() << " (Meshes: " << node->mNumMeshes << ")\n";

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        printNode(node->mChildren[i], depth + 1);
    }
}

dzemikk::ModelHandler::Result dzemikk::ModelHandler::load(const std::string& path) {
    auto model = loadModelFromFile(path);

    if (!model) {
        std::cerr << "Failed to load mesh: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {model, AssetError::None};
}

bool dzemikk::ModelHandler::reload(Handle& asset, const std::string& path) {
    if (!asset)
        return false;

    auto newModel = loadModelFromFile(path);
    if (!newModel)
        return false;

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

std::shared_ptr<dzemikk::Model> dzemikk::ModelHandler::loadModelFromFile(const std::string& path) {

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

    //printNode(scene->mRootNode);

    auto model = std::make_shared<Model>();

    auto skeleton = std::make_shared<dzemikk::Skeleton>();
    buildSkeleton(scene->mRootNode, *skeleton, -1);
    skeleton->setGlobalInverseTransform(glm::inverse(aiToGlm(scene->mRootNode->mTransformation)));

    model->setSkeleton(skeleton);

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {

        const aiMesh* aiMesh = scene->mMeshes[i];

        bool isSkinned = aiMesh->HasBones();

        if (!isSkinned) {

            std::vector<StaticVertex> vertices;
            std::vector<unsigned int> indices;

            vertices.reserve(aiMesh->mNumVertices);

            for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
                StaticVertex vertex{};

                vertex.position = {aiMesh->mVertices[v].x, aiMesh->mVertices[v].y,
                                   aiMesh->mVertices[v].z};

                vertex.normal = aiMesh->HasNormals()
                                    ? glm::vec3(aiMesh->mNormals[v].x, aiMesh->mNormals[v].y,
                                                aiMesh->mNormals[v].z)
                                    : glm::vec3(0.0f);

                vertices.push_back(vertex);
            }

            for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
                const aiFace& face = aiMesh->mFaces[f];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            auto mesh = std::make_shared<StaticMesh>();
            mesh->create(vertices, indices);

            model->addMesh(mesh, aiMesh->mMaterialIndex);
        }

        else {

            std::vector<SkinnedVertex> vertices;
            std::vector<unsigned int> indices;

            vertices.resize(aiMesh->mNumVertices);

            for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
                auto& vertex = vertices[v];

                vertex.position = {aiMesh->mVertices[v].x, aiMesh->mVertices[v].y,
                                   aiMesh->mVertices[v].z};

                vertex.normal = aiMesh->HasNormals()
                                    ? glm::vec3(aiMesh->mNormals[v].x, aiMesh->mNormals[v].y,
                                                aiMesh->mNormals[v].z)
                                    : glm::vec3(0.0f);
            }

            for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
                const aiFace& face = aiMesh->mFaces[f];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            extractBoneWeights(aiMesh, vertices, *skeleton);

            auto mesh = std::make_shared<SkinnedMesh>();
            mesh->create(vertices, indices);

            model->addMesh(mesh, aiMesh->mMaterialIndex);
        }
    }

    return model;
}

glm::mat4 dzemikk::ModelHandler::aiToGlm(const aiMatrix4x4& m) {
    glm::mat4 result;
    result[0][0] = m.a1;
    result[1][0] = m.a2;
    result[2][0] = m.a3;
    result[3][0] = m.a4;
    result[0][1] = m.b1;
    result[1][1] = m.b2;
    result[2][1] = m.b3;
    result[3][1] = m.b4;
    result[0][2] = m.c1;
    result[1][2] = m.c2;
    result[2][2] = m.c3;
    result[3][2] = m.c4;
    result[0][3] = m.d1;
    result[1][3] = m.d2;
    result[2][3] = m.d3;
    result[3][3] = m.d4;
    return result;
}

void dzemikk::ModelHandler::buildSkeleton(aiNode* node, dzemikk::Skeleton& skeleton, int parent) {
    int index = skeleton.addBone(node->mName.C_Str(), parent);

    auto* bone = const_cast<Bone*>(skeleton.getBone(index));
    if (bone) {
        bone->setLocalTransform(aiToGlm(node->mTransformation));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        buildSkeleton(node->mChildren[i], skeleton, index);
    }
}

void dzemikk::ModelHandler::extractBoneWeights(const aiMesh* mesh, std::vector<dzemikk::SkinnedVertex>& vertices,
                               dzemikk::Skeleton& skeleton) {
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone* bone = mesh->mBones[i];

        int boneID = skeleton.getBoneIndex(bone->mName.C_Str());

        std::cout << "Bone: " << i << " name: " << bone->mName.C_Str() << "\n";

        if (boneID == -1)
            continue;
        
        auto* skelBone = const_cast<Bone*>(skeleton.getBone(boneID));
        if (skelBone) {
            skelBone->setOffsetMatrix(aiToGlm(bone->mOffsetMatrix));
        }

        for (unsigned int w = 0; w < bone->mNumWeights; w++) {
            const aiVertexWeight& vw = bone->mWeights[w];
            auto& v = vertices[vw.mVertexId];

            for (int k = 0; k < 4; k++) {
                if (v.weights[k] == 0.0f) {
                    v.boneIDs[k] = boneID;
                    v.weights[k] = vw.mWeight;
                    break;
                }
            }
        }
    }

    std::cout << "\n\n";
}